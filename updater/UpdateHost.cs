using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Net;
using System.Net.Security;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Web.Script.Serialization;

namespace ManakaMultiplayer.UpdateHost
{
    internal static class Program
    {
        private const int MaxManifestBytes = 2 * 1024 * 1024;
        private const string InstalledUpdateHostPath =
            "EscapeFromManaka/UpdateHost.exe";
        private const string StagedUpdateHostPath =
            "EscapeFromManaka/UpdateHost.new.exe";
        private static readonly JavaScriptSerializer JsonSerializer =
            new JavaScriptSerializer();
        private static string logPath;
        private static string statusPath;
        private static string currentPhase = "error";
        private static string currentVersion = "";
        private static string latestVersion = "";
        private static long pendingFiles;
        private static long totalDownloadBytes;
        private static long downloadedBytes;
        private static string currentFile = "";
        private static string statusMessage = "";
        private static string channel = "stable";
        private static string devAccessToken = "";
        private static bool channelSwitch = false;
        private static bool noLaunch = false;

        [STAThread]
        private static int Main(string[] args)
        {
            try
            {
                string gameRoot = GetArgumentValue(args, "--game-root");
                if (String.IsNullOrWhiteSpace(gameRoot))
                {
                    gameRoot = ResolveGameRoot(args);
                }
                gameRoot = Path.GetFullPath(gameRoot);
                channel = GetArgumentValue(args, "--channel");
                if (String.IsNullOrWhiteSpace(channel))
                {
                    channel = "stable";
                }
                channel = channel.Trim().ToLowerInvariant();
                if (channel != "stable" && channel != "dev")
                {
                    throw new ArgumentException(
                        "Unsupported update channel: " + channel);
                }
                if (channel == "dev")
                {
                    devAccessToken = LoadDevAccessToken(gameRoot);
                }
                noLaunch = HasArgument(args, "--no-launch");
                statusPath = GetArgumentValue(args, "--state");
                if (String.IsNullOrWhiteSpace(statusPath))
                {
                    statusPath = Path.Combine(
                        gameRoot,
                        "EscapeFromManaka",
                        "Update",
                        "update-state.json");
                }
                statusPath = Path.GetFullPath(statusPath);
                PrepareLog(gameRoot);
                Log("UpdateHost version " + UpdateHostKeys.UpdateHostVersion +
                    " starting. gameRoot=" + gameRoot +
                    " channel=" + channel +
                    " statusPath=" + statusPath +
                    " args=" + String.Join(" ", args));

                RequireGameFiles(gameRoot);
                RecoverIncompleteTransaction(gameRoot);

                string mode = GetArgumentValue(args, "--mode");
                if (String.Equals(mode, "check", StringComparison.OrdinalIgnoreCase))
                {
                    return CheckForUpdates(gameRoot);
                }
                if (String.Equals(mode, "update", StringComparison.OrdinalIgnoreCase))
                {
                    int parentPid = ParseIntArgument(args, "--parent-pid", 0);
                    return DownloadAndInstall(gameRoot, parentPid);
                }

                throw new ArgumentException("Missing or unsupported --mode.");
            }
            catch (Exception exception)
            {
                Log("ERROR: " + exception);
                WriteStatus("error", exception.Message);
                return 2;
            }
        }

        private static int CheckForUpdates(string gameRoot)
        {
            ConfigureTlsValidation();
            ManifestInfo manifest = FetchSignedManifest(gameRoot);
            string stateFile = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "LauncherState.json");
            UpdateState state = UpdateState.Load(stateFile);
            string localVersion = ResolveLocalVersion(state);
            currentVersion = localVersion;
            latestVersion = manifest.Version;
            channelSwitch =
                !String.IsNullOrWhiteSpace(state.Channel) &&
                !String.Equals(
                    state.Channel,
                    manifest.Channel,
                    StringComparison.Ordinal);

            if (!channelSwitch &&
                CompareVersions(manifest.Version, localVersion) < 0)
            {
                Log("Signed release " + manifest.Version +
                    " is older than the installed client version " +
                    localVersion + "; update will not be offered.");
                pendingFiles = 0;
                totalDownloadBytes = 0;
                WriteStatus("current", "已安装版本高于服务器版本，保持当前版本。");
                return 0;
            }
            if (!channelSwitch)
            {
                ValidateReleaseOrdering(manifest, state);
            }

            List<ManifestFile> pending = FindPendingFiles(
                gameRoot,
                manifest.Files);
            pendingFiles = pending.Count;
            totalDownloadBytes = SumBytes(pending);
            downloadedBytes = 0;
            currentFile = "";
            Log("Signed release " + manifest.Version +
                " sequence=" + manifest.ReleaseSequence +
                " build=" + manifest.BuildId +
                "; pendingFiles=" + pending.Count +
                "; pendingBytes=" + totalDownloadBytes + ".");

            if (pending.Count > 0 &&
                CompareVersions(manifest.Version, localVersion) == 0 &&
                ContainsOnlyMetadata(pending))
            {
                string updateRoot = Path.Combine(
                    gameRoot,
                    "EscapeFromManaka",
                    "Update");
                string stagingRoot = Path.Combine(
                    updateRoot,
                    "staging-" + manifest.BuildId);
                DownloadPendingFiles(
                    manifest,
                    pending,
                    stagingRoot);
                ApplyPendingFilesTransaction(
                    gameRoot,
                    manifest,
                    pending,
                    stagingRoot);
                state.Channel = manifest.Channel;
                state.Version = manifest.Version;
                state.BuildId = manifest.BuildId;
                state.ReleaseSequence = manifest.ReleaseSequence;
                state.Save(stateFile);
                CompleteTransaction(gameRoot, manifest.BuildId);
                TryDeleteDirectory(stagingRoot);
                pendingFiles = 0;
                totalDownloadBytes = 0;
                downloadedBytes = 0;
                currentFile = "";
                Log("Synchronized same-version metadata without prompting.");
                WriteStatus("current", "当前已经是最新版本。");
                return 0;
            }

            if (!channelSwitch &&
                pending.Count == 0 &&
                CompareVersions(manifest.Version, localVersion) <= 0)
            {
                WriteStatus("current", "当前已经是最新版本。");
                return 0;
            }

            WriteStatus(
                "available",
                channelSwitch
                    ? "切换更新频道。"
                    : (pending.Count == 0
                    ? "文件已经一致，可以继续当前版本。"
                    : "发现可安装的更新。"));
            return 0;
        }

        private static bool ContainsOnlyMetadata(List<ManifestFile> files)
        {
            foreach (ManifestFile file in files)
            {
                if (!String.Equals(
                        file.Path,
                        "EscapeFromManaka/ReleaseInfo.json",
                        StringComparison.OrdinalIgnoreCase) &&
                    !String.Equals(
                        file.Path,
                        "EscapeFromManaka/README.md",
                        StringComparison.OrdinalIgnoreCase))
                {
                    return false;
                }
            }
            return true;
        }

        private static int DownloadAndInstall(string gameRoot, int parentPid)
        {
            ConfigureTlsValidation();
            ManifestInfo manifest = FetchSignedManifest(gameRoot);
            string stateFile = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "LauncherState.json");
            UpdateState state = UpdateState.Load(stateFile);
            string localVersion = ResolveLocalVersion(state);
            currentVersion = localVersion;
            latestVersion = manifest.Version;
            channelSwitch =
                !String.IsNullOrWhiteSpace(state.Channel) &&
                !String.Equals(
                    state.Channel,
                    manifest.Channel,
                    StringComparison.Ordinal);
            if (!channelSwitch &&
                CompareVersions(manifest.Version, localVersion) < 0)
            {
                WriteStatus("current", "服务器版本较低，取消更新。");
                LaunchGame(gameRoot);
                return 0;
            }
            if (!channelSwitch)
            {
                ValidateReleaseOrdering(manifest, state);
            }

            List<ManifestFile> pending = FindPendingFiles(
                gameRoot,
                manifest.Files);
            pendingFiles = pending.Count;
            totalDownloadBytes = SumBytes(pending);
            downloadedBytes = 0;
            currentFile = "";

            string updateRoot = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Update");
            Directory.CreateDirectory(updateRoot);
            string stagingRoot = Path.Combine(
                updateRoot,
                "staging-" + manifest.BuildId);

            if (pending.Count > 0)
            {
                WriteStatus("downloading", "正在下载更新。");
                DownloadPendingFiles(
                    manifest,
                    pending,
                    stagingRoot);
            }

            WriteStatus("ready", "下载完成，等待游戏退出后安装。");
            if (parentPid > 0)
            {
                WaitForParentExit(parentPid, gameRoot);
            }
            else
            {
                EnsureGameNotRunning();
            }
            RecoverIncompleteTransaction(gameRoot);

            try
            {
                ApplyPendingFilesTransaction(
                    gameRoot,
                    manifest,
                    pending,
                    stagingRoot);

                state.Channel = manifest.Channel;
                state.Version = manifest.Version;
                state.BuildId = manifest.BuildId;
                state.ReleaseSequence = manifest.ReleaseSequence;
                state.Save(stateFile);
                CompleteTransaction(gameRoot, manifest.BuildId);
                TryDeleteDirectory(stagingRoot);
                WriteStatus("completed", "更新安装完成，正在启动游戏。");
                if (!noLaunch)
                {
                    LaunchGame(gameRoot);
                }
                return 0;
            }
            catch
            {
                WriteStatus("error", "更新安装失败，正在恢复当前版本。");
                TryLaunchAfterFailure(gameRoot);
                throw;
            }
        }

        private static string ResolveGameRoot(string[] args)
        {
            string explicitRoot = GetArgumentValue(args, "--install");
            if (!String.IsNullOrWhiteSpace(explicitRoot))
            {
                return Path.GetFullPath(explicitRoot);
            }

            string baseDirectory = AppDomain.CurrentDomain.BaseDirectory;
            if (File.Exists(Path.Combine(
                    baseDirectory,
                    "SecretFlasherManaka.exe")))
            {
                return Path.GetFullPath(baseDirectory);
            }

            DirectoryInfo parent = Directory.GetParent(
                baseDirectory.TrimEnd(
                    Path.DirectorySeparatorChar,
                    Path.AltDirectorySeparatorChar));
            if (parent != null &&
                File.Exists(Path.Combine(
                    parent.FullName,
                    "SecretFlasherManaka.exe")))
            {
                return Path.GetFullPath(parent.FullName);
            }

            return Path.GetFullPath(baseDirectory);
        }

        private static void PrepareLog(string gameRoot)
        {
            string logDirectory = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Logs");
            Directory.CreateDirectory(logDirectory);
            logPath = Path.Combine(logDirectory, "UpdateHost.log");
        }

        private static void Log(string message)
        {
            if (String.IsNullOrEmpty(logPath))
            {
                return;
            }
            string line = DateTime.UtcNow.ToString("O", CultureInfo.InvariantCulture) +
                " " + message + Environment.NewLine;
            File.AppendAllText(logPath, line, new UTF8Encoding(false));
        }

        private static int CompareVersions(string left, string right)
        {
            string[] leftParts = (left ?? "").Split('.');
            string[] rightParts = (right ?? "").Split('.');
            if (leftParts.Length != 3 || rightParts.Length != 3)
            {
                throw new InvalidDataException(
                    "Version values must use LEAD.MAJOR.MINOR format.");
            }

            for (int index = 0; index < 3; ++index)
            {
                int leftValue;
                int rightValue;
                if (!Int32.TryParse(
                        leftParts[index],
                        NumberStyles.None,
                        CultureInfo.InvariantCulture,
                        out leftValue) ||
                    !Int32.TryParse(
                        rightParts[index],
                        NumberStyles.None,
                        CultureInfo.InvariantCulture,
                        out rightValue))
                {
                    throw new InvalidDataException(
                        "Version values must use LEAD.MAJOR.MINOR format.");
                }
                int comparison = leftValue.CompareTo(rightValue);
                if (comparison != 0)
                {
                    return comparison;
                }
            }
            return 0;
        }

        private static bool HasArgument(string[] args, string expected)
        {
            foreach (string argument in args)
            {
                if (String.Equals(
                        argument,
                        expected,
                        StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }
            return false;
        }

        private static ManifestInfo FetchSignedManifest(string gameRoot)
        {
            byte[] envelopeBytes = DownloadBytes(
                GetChannelBaseUrl() + "channels/" + channel + ".json",
                MaxManifestBytes);
            Dictionary<string, object> envelope = ParseObject(
                Encoding.UTF8.GetString(envelopeBytes));
            ManifestInfo manifest = VerifyEnvelope(envelope, channel);
            ValidateManifest(manifest, gameRoot, false);
            return manifest;
        }

        private static string GetChannelBaseUrl()
        {
            return channel == "dev"
                ? UpdateHostKeys.DevBaseUrl
                : UpdateHostKeys.StableBaseUrl;
        }

        private static string LoadDevAccessToken(string gameRoot)
        {
            string path = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Channel",
                "dev.key");
            if (!File.Exists(path))
            {
                throw new FileNotFoundException(
                    "Development channel access key was not found.",
                    path);
            }

            byte[] stored = File.ReadAllBytes(path);
            byte[] plain;
            try
            {
                plain = ProtectedData.Unprotect(
                    stored,
                    null,
                    DataProtectionScope.CurrentUser);
            }
            catch
            {
                plain = stored;
            }
            string token = Encoding.UTF8.GetString(plain).Trim();
            if (String.IsNullOrWhiteSpace(token))
            {
                throw new InvalidDataException(
                    "Development channel access key is empty.");
            }
            return token;
        }

        private static string ResolveLocalVersion(UpdateState state)
        {
            if (!String.IsNullOrWhiteSpace(state.Version))
            {
                return state.Version;
            }
            return UpdateHostKeys.UpdateHostVersion;
        }

        private static void ValidateReleaseOrdering(
            ManifestInfo manifest,
            UpdateState state)
        {
            if (state.ReleaseSequence > manifest.ReleaseSequence)
            {
                throw new InvalidDataException(
                    "The signed update sequence is older than the installed state.");
            }
            if (state.ReleaseSequence == manifest.ReleaseSequence &&
                state.ReleaseSequence > 0 &&
                !String.Equals(
                    state.BuildId,
                    manifest.BuildId,
                    StringComparison.Ordinal))
            {
                throw new InvalidDataException(
                    "The signed update sequence was replayed with another build.");
            }
        }

        private static long SumBytes(List<ManifestFile> files)
        {
            long total = 0;
            foreach (ManifestFile file in files)
            {
                total += file.Size;
            }
            return total;
        }

        private static int ParseIntArgument(
            string[] args,
            string name,
            int defaultValue)
        {
            string value = GetArgumentValue(args, name);
            int parsed;
            return !String.IsNullOrWhiteSpace(value) &&
                   Int32.TryParse(
                       value,
                       NumberStyles.Integer,
                       CultureInfo.InvariantCulture,
                       out parsed)
                ? parsed
                : defaultValue;
        }

        private static void WaitForParentExit(int parentPid, string gameRoot)
        {
            if (parentPid <= 0)
            {
                throw new InvalidDataException(
                    "A valid parent game process id is required.");
            }

            try
            {
                using (Process parent = Process.GetProcessById(parentPid))
                {
                    WriteStatus("waiting", "等待游戏正常退出。");
                    if (!parent.WaitForExit(300000))
                    {
                        throw new TimeoutException(
                            "The game did not exit within five minutes.");
                    }
                }
            }
            catch (ArgumentException)
            {
                Log("Parent game process already exited.");
            }
            catch
            {
                WriteStatus("error", "等待游戏退出超时，更新已取消。");
                TryLaunchAfterFailure(gameRoot);
                throw;
            }
        }

        private static void TryLaunchAfterFailure(string gameRoot)
        {
            try
            {
                if (noLaunch)
                {
                    return;
                }
                Process[] processes =
                    Process.GetProcessesByName("SecretFlasherManaka");
                if (processes.Length == 0)
                {
                    LaunchGame(gameRoot);
                }
                foreach (Process process in processes)
                {
                    process.Dispose();
                }
            }
            catch (Exception exception)
            {
                Log("Unable to restart the game after update failure: " +
                    exception);
            }
        }

        private static void WriteStatus(string phase, string message)
        {
            currentPhase = phase;
            statusMessage = message ?? "";
            if (String.IsNullOrWhiteSpace(statusPath))
            {
                return;
            }

            Dictionary<string, object> value =
                new Dictionary<string, object>();
            value["schemaVersion"] = 1;
            value["phase"] = currentPhase;
            value["currentVersion"] = currentVersion ?? "";
            value["latestVersion"] = latestVersion ?? "";
            value["pendingFiles"] = pendingFiles;
            value["totalDownloadBytes"] = totalDownloadBytes;
            value["downloadedBytes"] = downloadedBytes;
            value["currentFile"] = currentFile ?? "";
            value["message"] = statusMessage;
            value["channel"] = channel;
            value["channelSwitch"] = channelSwitch;
            value["updatedAtUtc"] = DateTime.UtcNow.ToString(
                "O",
                CultureInfo.InvariantCulture);

            Directory.CreateDirectory(Path.GetDirectoryName(statusPath));
            string temporary = statusPath + ".new";
            File.WriteAllText(
                temporary,
                JsonSerializer.Serialize(value) + Environment.NewLine,
                new UTF8Encoding(false));
            if (File.Exists(statusPath))
            {
                File.Replace(temporary, statusPath, null, true);
            }
            else
            {
                File.Move(temporary, statusPath);
            }
        }

        private static string GetArgumentValue(
            string[] args,
            string name)
        {
            for (int index = 0; index < args.Length - 1; ++index)
            {
                if (String.Equals(
                        args[index],
                        name,
                        StringComparison.OrdinalIgnoreCase))
                {
                    return args[index + 1];
                }
            }
            return null;
        }

        private static void RequireGameFiles(string gameRoot)
        {
            string[] required =
            {
                "SecretFlasherManaka.exe",
                "UnityPlayer.dll",
                "GameAssembly.dll"
            };
            foreach (string fileName in required)
            {
                string path = Path.Combine(gameRoot, fileName);
                if (!File.Exists(path))
                {
                    throw new FileNotFoundException(
                        "Required game file was not found: " + path,
                        path);
                }
            }
        }

        private static void ConfigureTlsValidation()
        {
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;
            ServicePointManager.ServerCertificateValidationCallback =
                ValidateServerCertificate;
        }

        private static bool ValidateServerCertificate(
            object sender,
            X509Certificate certificate,
            X509Chain chain,
            SslPolicyErrors errors)
        {
            if (certificate == null)
            {
                return false;
            }

            X509Certificate2 certificate2;
            try
            {
                certificate2 = new X509Certificate2(certificate);
            }
            catch
            {
                return false;
            }

            DateTime now = DateTime.UtcNow;
            if (now < certificate2.NotBefore.ToUniversalTime() ||
                now > certificate2.NotAfter.ToUniversalTime())
            {
                return false;
            }

            byte[] expected;
            try
            {
                expected = Convert.FromBase64String(
                    UpdateHostKeys.TlsSpkiSha256Base64);
            }
            catch
            {
                return false;
            }

            byte[] actual;
            using (SHA256 sha256 = SHA256.Create())
            {
                actual = sha256.ComputeHash(
                    GetSubjectPublicKeyInfo(certificate2));
            }
            bool matches = ConstantTimeEquals(actual, expected);
            if (!matches)
            {
                Log("TLS SPKI mismatch. expected=" +
                    UpdateHostKeys.TlsSpkiSha256Base64 +
                    " actual=" + Convert.ToBase64String(actual));
            }
            return matches;
        }

        private static byte[] GetSubjectPublicKeyInfo(
            X509Certificate2 certificate)
        {
            if (!String.Equals(
                    certificate.PublicKey.Oid.Value,
                    "1.2.840.113549.1.1.1",
                    StringComparison.Ordinal))
            {
                throw new CryptographicException(
                    "The pinned TLS certificate must use an RSA key.");
            }

            byte[] algorithmIdentifier =
            {
                0x30, 0x0d,
                0x06, 0x09,
                0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
                0x05, 0x00
            };
            byte[] publicKey =
                certificate.PublicKey.EncodedKeyValue.RawData;
            byte[] bitStringValue = new byte[publicKey.Length + 1];
            bitStringValue[0] = 0;
            Buffer.BlockCopy(
                publicKey,
                0,
                bitStringValue,
                1,
                publicKey.Length);
            byte[] bitString = EncodeDerValue(0x03, bitStringValue);
            byte[] body = new byte[
                algorithmIdentifier.Length + bitString.Length];
            Buffer.BlockCopy(
                algorithmIdentifier,
                0,
                body,
                0,
                algorithmIdentifier.Length);
            Buffer.BlockCopy(
                bitString,
                0,
                body,
                algorithmIdentifier.Length,
                bitString.Length);
            return EncodeDerValue(0x30, body);
        }

        private static byte[] EncodeDerValue(byte tag, byte[] value)
        {
            List<byte> length = new List<byte>();
            int remaining = value.Length;
            do
            {
                length.Insert(0, (byte)(remaining & 0xff));
                remaining >>= 8;
            }
            while (remaining != 0);

            if (length.Count > 1 || length[0] >= 0x80)
            {
                length.Insert(0, (byte)(0x80 | length.Count));
            }

            byte[] encoded = new byte[
                1 + length.Count + value.Length];
            encoded[0] = tag;
            length.CopyTo(encoded, 1);
            Buffer.BlockCopy(
                value,
                0,
                encoded,
                1 + length.Count,
                value.Length);
            return encoded;
        }

        private static bool ConstantTimeEquals(
            byte[] left,
            byte[] right)
        {
            if (left == null || right == null || left.Length != right.Length)
            {
                return false;
            }

            int difference = 0;
            for (int index = 0; index < left.Length; ++index)
            {
                difference |= left[index] ^ right[index];
            }
            return difference == 0;
        }

        private static byte[] DownloadBytes(
            string url,
            int maximumBytes)
        {
            HttpWebRequest request = CreateRequest(url);
            using (HttpWebResponse response =
                (HttpWebResponse)request.GetResponse())
            using (Stream stream = response.GetResponseStream())
            using (MemoryStream memory = new MemoryStream())
            {
                if (response.ContentLength > maximumBytes)
                {
                    throw new InvalidDataException(
                        "HTTP response exceeds the allowed size.");
                }

                byte[] buffer = new byte[32768];
                int read;
                while ((read = stream.Read(buffer, 0, buffer.Length)) > 0)
                {
                    if (memory.Length + read > maximumBytes)
                    {
                        throw new InvalidDataException(
                            "HTTP response exceeds the allowed size.");
                    }
                    memory.Write(buffer, 0, read);
                }
                return memory.ToArray();
            }
        }

        private static HttpWebRequest CreateRequest(string url)
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
            request.AllowAutoRedirect = false;
            request.AutomaticDecompression =
                DecompressionMethods.GZip | DecompressionMethods.Deflate;
            request.UserAgent = "EscapeFromManakaUpdateHost/" +
                UpdateHostKeys.UpdateHostVersion;
            if (channel == "dev")
            {
                if (String.IsNullOrWhiteSpace(devAccessToken))
                {
                    throw new InvalidOperationException(
                        "Development channel access key is unavailable.");
                }
                request.Headers["X-EFM-Dev-Key"] = devAccessToken;
            }
            request.Timeout = 20000;
            request.ReadWriteTimeout = 60000;
            return request;
        }

        private static Dictionary<string, object> ParseObject(string json)
        {
            Dictionary<string, object> value =
                JsonSerializer.DeserializeObject(json) as
                    Dictionary<string, object>;
            if (value == null)
            {
                throw new InvalidDataException("JSON root must be an object.");
            }
            return value;
        }

        private static ManifestInfo VerifyEnvelope(
            Dictionary<string, object> envelope,
            string requestedChannel)
        {
            int schemaVersion = GetInt32(envelope, "schemaVersion");
            string algorithm = GetString(envelope, "algorithm");
            string keyId = GetString(envelope, "keyId");
            if (schemaVersion != 1 ||
                !String.Equals(
                    algorithm,
                    "ES256",
                    StringComparison.Ordinal) ||
                !String.Equals(
                    keyId,
                    ExpectedManifestKeyId(requestedChannel),
                    StringComparison.Ordinal))
            {
                throw new InvalidDataException(
                    "Unsupported signed channel envelope.");
            }

            byte[] manifestBytes = Convert.FromBase64String(
                GetString(envelope, "manifest"));
            byte[] signatureBytes = Convert.FromBase64String(
                GetString(envelope, "signature"));
            VerifySignature(
                manifestBytes,
                signatureBytes,
                requestedChannel);
            return ManifestInfo.Parse(manifestBytes);
        }

        private static void VerifySignature(
            byte[] manifestBytes,
            byte[] signatureBytes,
            string requestedChannel)
        {
            X509Certificate2 certificate = new X509Certificate2(
                Convert.FromBase64String(
                    ExpectedManifestCertificate(requestedChannel)));
            DateTime now = DateTime.UtcNow;
            if (now < certificate.NotBefore.ToUniversalTime() ||
                now > certificate.NotAfter.ToUniversalTime())
            {
                throw new CryptographicException(
                    "The manifest signing certificate is outside its validity period.");
            }
            using (ECDsa publicKey = certificate.GetECDsaPublicKey())
            {
                if (publicKey == null)
                {
                    throw new CryptographicException(
                        "The update manifest signature is invalid.");
                }

                bool valid;
                try
                {
                    valid = publicKey.VerifyData(
                        manifestBytes,
                        ConvertDerSignatureToIeeeP1363(signatureBytes),
                        HashAlgorithmName.SHA256);
                }
                catch (CryptographicException)
                {
                    valid = false;
                }
                if (!valid)
                {
                    throw new CryptographicException(
                        "The update manifest signature is invalid.");
                }
            }
        }

        private static string ExpectedManifestKeyId(string requestedChannel)
        {
            return requestedChannel == "dev"
                ? UpdateHostKeys.DevManifestKeyId
                : UpdateHostKeys.StableManifestKeyId;
        }

        private static string ExpectedManifestCertificate(
            string requestedChannel)
        {
            return requestedChannel == "dev"
                ? UpdateHostKeys.DevManifestSigningCertificateBase64
                : UpdateHostKeys.StableManifestSigningCertificateBase64;
        }

        private static byte[] ConvertDerSignatureToIeeeP1363(
            byte[] signature)
        {
            const int componentSize = 32;
            if (signature.Length == componentSize * 2)
            {
                return signature;
            }
            if (signature.Length < 8 || signature[0] != 0x30)
            {
                throw new CryptographicException(
                    "Invalid ECDSA signature encoding.");
            }

            int offset = 1;
            int sequenceLength = ReadDerLength(signature, ref offset);
            if (sequenceLength != signature.Length - offset)
            {
                throw new CryptographicException(
                    "Invalid ECDSA signature sequence length.");
            }

            byte[] r = ReadDerInteger(signature, ref offset);
            byte[] s = ReadDerInteger(signature, ref offset);
            if (offset != signature.Length ||
                r.Length > componentSize ||
                s.Length > componentSize)
            {
                throw new CryptographicException(
                    "Invalid ECDSA signature component.");
            }

            byte[] result = new byte[componentSize * 2];
            Buffer.BlockCopy(
                r,
                0,
                result,
                componentSize - r.Length,
                r.Length);
            Buffer.BlockCopy(
                s,
                0,
                result,
                componentSize * 2 - s.Length,
                s.Length);
            return result;
        }

        private static int ReadDerLength(
            byte[] value,
            ref int offset)
        {
            if (offset >= value.Length)
            {
                throw new CryptographicException(
                    "Invalid ECDSA signature length.");
            }
            int first = value[offset++];
            if ((first & 0x80) == 0)
            {
                return first;
            }

            int count = first & 0x7f;
            if (count == 0 || count > 2 ||
                offset + count > value.Length)
            {
                throw new CryptographicException(
                    "Invalid ECDSA signature length.");
            }
            int length = 0;
            for (int index = 0; index < count; ++index)
            {
                length = (length << 8) | value[offset++];
            }
            return length;
        }

        private static byte[] ReadDerInteger(
            byte[] value,
            ref int offset)
        {
            if (offset >= value.Length || value[offset++] != 0x02)
            {
                throw new CryptographicException(
                    "Invalid ECDSA signature integer.");
            }
            int length = ReadDerLength(value, ref offset);
            if (length <= 0 || offset + length > value.Length)
            {
                throw new CryptographicException(
                    "Invalid ECDSA signature integer.");
            }

            int start = offset;
            int end = offset + length;
            while (start < end - 1 && value[start] == 0)
            {
                ++start;
            }
            byte[] result = new byte[end - start];
            Buffer.BlockCopy(value, start, result, 0, result.Length);
            offset = end;
            return result;
        }

        private static void ValidateManifest(
            ManifestInfo manifest,
            string gameRoot,
            bool skipGameCheck)
        {
            if (manifest.SchemaVersion != 1 ||
                manifest.ReleaseSequence < 1 ||
                !String.Equals(
                    manifest.Channel,
                    channel,
                    StringComparison.Ordinal) ||
                !String.Equals(
                    manifest.SigningKeyId,
                    ExpectedManifestKeyId(channel),
                    StringComparison.Ordinal))
            {
                throw new InvalidDataException(
                    "The signed manifest does not match this update host.");
            }
            if (manifest.Files.Count == 0)
            {
                throw new InvalidDataException(
                    "The signed manifest contains no files.");
            }

            if (!skipGameCheck &&
                !String.IsNullOrWhiteSpace(manifest.GameAssemblySha256))
            {
                string actual = HashFile(Path.Combine(
                    gameRoot,
                    "GameAssembly.dll")).ToLowerInvariant();
                if (!String.Equals(
                        actual,
                        manifest.GameAssemblySha256.ToLowerInvariant(),
                        StringComparison.Ordinal))
                {
                    throw new InvalidDataException(
                        "GameAssembly.dll does not match the signed update target.");
                }
            }
        }

        private static List<ManifestFile> FindPendingFiles(
            string gameRoot,
            List<ManifestFile> files)
        {
            List<ManifestFile> pending = new List<ManifestFile>();
            foreach (ManifestFile file in files)
            {
                bool stagedUpdateHost = String.Equals(
                    file.Path,
                    StagedUpdateHostPath,
                    StringComparison.OrdinalIgnoreCase);
                string target = ResolveSafeTarget(
                    gameRoot,
                    stagedUpdateHost
                        ? InstalledUpdateHostPath
                        : file.Path);
                FileInfo info = new FileInfo(target);
                if (!info.Exists ||
                    info.Length != file.Size ||
                    !String.Equals(
                        HashFile(target),
                        file.Sha256,
                        StringComparison.OrdinalIgnoreCase))
                {
                    pending.Add(file);
                }
            }
            return pending;
        }

        private static void DownloadPendingFiles(
            ManifestInfo manifest,
            List<ManifestFile> pending,
            string stagingRoot)
        {
            downloadedBytes = 0;
            foreach (ManifestFile file in pending)
            {
                string destination = ResolveSafeTarget(
                    stagingRoot,
                    file.Path);
                Directory.CreateDirectory(Path.GetDirectoryName(destination));
                string url = GetChannelBaseUrl() + file.ObjectPath;
                Log("Downloading " + file.Path + ".");
                currentFile = file.Path;
                WriteStatus("downloading", "正在下载更新。");
                DownloadFile(url, destination, file);
            }
            currentFile = "";
        }

        private static void DownloadFile(
            string url,
            string destination,
            ManifestFile expected)
        {
            string temporary = destination + ".download";
            if (File.Exists(temporary))
            {
                File.Delete(temporary);
            }
            HttpWebRequest request = CreateRequest(url);
            using (HttpWebResponse response =
                (HttpWebResponse)request.GetResponse())
            using (Stream input = response.GetResponseStream())
            using (FileStream output = new FileStream(
                temporary,
                FileMode.Create,
                FileAccess.Write,
                FileShare.None))
            {
                byte[] buffer = new byte[65536];
                DateTime lastStatus = DateTime.MinValue;
                int read;
                while ((read = input.Read(buffer, 0, buffer.Length)) > 0)
                {
                    output.Write(buffer, 0, read);
                    downloadedBytes += read;
                    DateTime now = DateTime.UtcNow;
                    if ((now - lastStatus).TotalMilliseconds >= 200)
                    {
                        lastStatus = now;
                        WriteStatus("downloading", "正在下载更新。");
                    }
                }
            }

            FileInfo info = new FileInfo(temporary);
            if (info.Length != expected.Size ||
                !String.Equals(
                    HashFile(temporary),
                    expected.Sha256,
                    StringComparison.OrdinalIgnoreCase))
            {
                File.Delete(temporary);
                throw new InvalidDataException(
                    "Downloaded file failed verification: " + expected.Path);
            }
            if (File.Exists(destination))
            {
                File.Delete(destination);
            }
            File.Move(temporary, destination);
            WriteStatus("downloading", "正在下载更新。");
        }

        private static void ApplyPendingFilesTransaction(
            string gameRoot,
            ManifestInfo manifest,
            List<ManifestFile> pending,
            string stagingRoot)
        {
            string updateRoot = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Update");
            string backupRoot = Path.Combine(
                updateRoot,
                "backup-" + manifest.BuildId);
            Directory.CreateDirectory(backupRoot);
            List<Dictionary<string, object>> journalFiles =
                new List<Dictionary<string, object>>();

            foreach (ManifestFile file in pending)
            {
                string target = ResolveSafeTarget(gameRoot, file.Path);
                bool existed = File.Exists(target);
                if (existed)
                {
                    string backup = ResolveSafeTarget(
                        backupRoot,
                        file.Path);
                    Directory.CreateDirectory(
                        Path.GetDirectoryName(backup));
                    File.Copy(target, backup, true);
                }
                Dictionary<string, object> journalFile =
                    new Dictionary<string, object>();
                journalFile["path"] = file.Path;
                journalFile["existed"] = existed;
                journalFiles.Add(journalFile);
            }

            WriteTransactionJournal(
                gameRoot,
                manifest.BuildId,
                journalFiles);

            try
            {
                foreach (ManifestFile file in pending)
                {
                    string source = ResolveSafeTarget(
                        stagingRoot,
                        file.Path);
                    string target = ResolveSafeTarget(
                        gameRoot,
                        file.Path);
                    Directory.CreateDirectory(
                        Path.GetDirectoryName(target));
                    string temporary = target + ".manaka-new";
                    File.Copy(source, temporary, true);
                    if (File.Exists(target))
                    {
                        File.Replace(temporary, target, null, true);
                    }
                    else
                    {
                        File.Move(temporary, target);
                    }
                    Log("Applied " + file.Path + " for build " +
                        manifest.BuildId + ".");
                }
            }
            catch
            {
                RestoreTransaction(gameRoot);
                throw;
            }
        }

        private static void WriteTransactionJournal(
            string gameRoot,
            string buildId,
            List<Dictionary<string, object>> files)
        {
            string updateRoot = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Update");
            Directory.CreateDirectory(updateRoot);
            Dictionary<string, object> value =
                new Dictionary<string, object>();
            value["schemaVersion"] = 1;
            value["buildId"] = buildId;
            value["files"] = files;
            string journalPath = Path.Combine(
                updateRoot,
                "transaction.json");
            string temporary = journalPath + ".new";
            File.WriteAllText(
                temporary,
                JsonSerializer.Serialize(value) + Environment.NewLine,
                new UTF8Encoding(false));
            if (File.Exists(journalPath))
            {
                File.Replace(temporary, journalPath, null, true);
            }
            else
            {
                File.Move(temporary, journalPath);
            }
        }

        private static void RecoverIncompleteTransaction(string gameRoot)
        {
            string transaction = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Update",
                "transaction.json");
            if (!File.Exists(transaction))
            {
                return;
            }

            try
            {
                Process[] processes =
                    Process.GetProcessesByName("SecretFlasherManaka");
                bool gameRunning = processes.Length > 0;
                foreach (Process process in processes)
                {
                    process.Dispose();
                }
                if (gameRunning)
                {
                    Log("Incomplete update transaction detected while the game is running; recovery is deferred.");
                    return;
                }
                RestoreTransaction(gameRoot);
            }
            catch (Exception exception)
            {
                Log("Incomplete transaction recovery failed: " + exception);
                throw;
            }
        }

        private static void RestoreTransaction(string gameRoot)
        {
            string updateRoot = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Update");
            string transaction = Path.Combine(
                updateRoot,
                "transaction.json");
            if (!File.Exists(transaction))
            {
                return;
            }

            Dictionary<string, object> value = ParseObject(
                File.ReadAllText(transaction, Encoding.UTF8));
            string buildId = Convert.ToString(
                value.ContainsKey("buildId") ? value["buildId"] : "",
                CultureInfo.InvariantCulture);
            object[] files = GetRequired(value, "files") as object[];
            if (files == null)
            {
                throw new InvalidDataException(
                    "Invalid transaction file list.");
            }

            string backupRoot = Path.Combine(
                updateRoot,
                "backup-" + buildId);
            for (int index = files.Length - 1; index >= 0; --index)
            {
                Dictionary<string, object> entry =
                    files[index] as Dictionary<string, object>;
                if (entry == null)
                {
                    throw new InvalidDataException(
                        "Invalid transaction entry.");
                }
                string relativePath = Convert.ToString(
                    GetRequired(entry, "path"),
                    CultureInfo.InvariantCulture);
                bool existed = Convert.ToBoolean(
                    GetRequired(entry, "existed"),
                    CultureInfo.InvariantCulture);
                string target = ResolveSafeTarget(
                    gameRoot,
                    relativePath);
                if (existed)
                {
                    string backup = ResolveSafeTarget(
                        backupRoot,
                        relativePath);
                    if (!File.Exists(backup))
                    {
                        throw new FileNotFoundException(
                            "Update rollback file is missing: " + backup,
                            backup);
                    }
                    Directory.CreateDirectory(
                        Path.GetDirectoryName(target));
                    File.Copy(backup, target, true);
                }
                else if (File.Exists(target))
                {
                    File.Delete(target);
                }
            }

            File.Delete(transaction);
            TryDeleteDirectory(backupRoot);
            Log("Recovered incomplete update transaction " + buildId + ".");
        }

        private static void CompleteTransaction(string gameRoot, string buildId)
        {
            string updateRoot = Path.Combine(
                gameRoot,
                "EscapeFromManaka",
                "Update");
            string transaction = Path.Combine(
                updateRoot,
                "transaction.json");
            if (File.Exists(transaction))
            {
                File.Delete(transaction);
            }
            TryDeleteDirectory(Path.Combine(
                updateRoot,
                "backup-" + buildId));
        }

        private static string ResolveSafeTarget(
            string root,
            string relativePath)
        {
            string normalized = relativePath.Replace('/', '\\');
            if (String.IsNullOrWhiteSpace(normalized) ||
                Path.IsPathRooted(normalized) ||
                normalized.Contains("..") ||
                normalized.Contains(":"))
            {
                throw new InvalidDataException(
                    "Unsafe update path: " + relativePath);
            }

            if (!String.Equals(
                    relativePath,
                    "winhttp.dll",
                    StringComparison.OrdinalIgnoreCase) &&
                !relativePath.StartsWith(
                    "EscapeFromManaka/",
                    StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidDataException(
                    "Update path is outside the approved runtime tree: " +
                    relativePath);
            }

            string fullRoot = Path.GetFullPath(root).TrimEnd(
                Path.DirectorySeparatorChar,
                Path.AltDirectorySeparatorChar) +
                Path.DirectorySeparatorChar;
            string target = Path.GetFullPath(Path.Combine(root, normalized));
            if (!target.StartsWith(
                    fullRoot,
                    StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidDataException(
                    "Update path escaped the target root: " + relativePath);
            }
            return target;
        }

        private static string HashFile(string path)
        {
            using (SHA256 sha256 = SHA256.Create())
            using (FileStream stream = File.OpenRead(path))
            {
                return ToHex(sha256.ComputeHash(stream));
            }
        }

        private static string ToHex(byte[] bytes)
        {
            StringBuilder builder = new StringBuilder(bytes.Length * 2);
            foreach (byte value in bytes)
            {
                builder.Append(value.ToString("x2", CultureInfo.InvariantCulture));
            }
            return builder.ToString();
        }

        private static void LaunchGame(string gameRoot)
        {
            ProcessStartInfo startInfo = new ProcessStartInfo(
                Path.Combine(gameRoot, "SecretFlasherManaka.exe"));
            startInfo.WorkingDirectory = gameRoot;
            startInfo.UseShellExecute = false;
            if (channel == "dev")
            {
                startInfo.EnvironmentVariables["MANAKA_EFM_CHANNEL"] = "dev";
            }
            Log("Launching SecretFlasherManaka.exe.");
            Process.Start(startInfo);
        }

        private static void EnsureGameNotRunning()
        {
            Process[] processes =
                Process.GetProcessesByName("SecretFlasherManaka");
            if (processes.Length == 0)
            {
                return;
            }
            foreach (Process process in processes)
            {
                process.Dispose();
            }
            throw new InvalidOperationException(
                "SecretFlasherManaka is running; close it before updating.");
        }

        private static int GetInt32(
            Dictionary<string, object> value,
            string name)
        {
            return Convert.ToInt32(
                GetRequired(value, name),
                CultureInfo.InvariantCulture);
        }

        private static long GetInt64(
            Dictionary<string, object> value,
            string name)
        {
            return Convert.ToInt64(
                GetRequired(value, name),
                CultureInfo.InvariantCulture);
        }

        private static string GetString(
            Dictionary<string, object> value,
            string name)
        {
            return Convert.ToString(
                GetRequired(value, name),
                CultureInfo.InvariantCulture);
        }

        private static object GetRequired(
            Dictionary<string, object> value,
            string name)
        {
            object result;
            if (!value.TryGetValue(name, out result))
            {
                throw new InvalidDataException(
                    "Missing JSON property: " + name);
            }
            return result;
        }

        private static void TryDeleteDirectory(string path)
        {
            try
            {
                if (Directory.Exists(path))
                {
                    Directory.Delete(path, true);
                }
            }
            catch (Exception exception)
            {
                Log("Unable to remove staging directory: " + exception.Message);
            }
        }

        private sealed class ManifestInfo
        {
            public int SchemaVersion;
            public long ReleaseSequence;
            public string Version;
            public string Channel;
            public string BuildId;
            public string ProtocolVersion;
            public string SigningKeyId;
            public string GameAssemblySha256;
            public List<ManifestFile> Files;

            public static ManifestInfo Parse(byte[] bytes)
            {
                Dictionary<string, object> value = ParseObject(
                    Encoding.UTF8.GetString(bytes));
                ManifestInfo manifest = new ManifestInfo();
                manifest.SchemaVersion = GetInt32(value, "schemaVersion");
                manifest.ReleaseSequence = GetInt64(
                    value,
                    "releaseSequence");
                manifest.Version = GetString(value, "version");
                manifest.Channel = GetString(value, "channel");
                manifest.BuildId = GetString(value, "buildId");
                manifest.ProtocolVersion = GetString(
                    value,
                    "protocolVersion");
                manifest.SigningKeyId = GetString(value, "signingKeyId");
                manifest.GameAssemblySha256 = Convert.ToString(
                    value.ContainsKey("gameAssemblySha256")
                        ? value["gameAssemblySha256"]
                        : null,
                    CultureInfo.InvariantCulture);
                manifest.Files = new List<ManifestFile>();

                object[] files = GetRequired(value, "files") as object[];
                if (files == null)
                {
                    throw new InvalidDataException(
                        "Manifest files must be an array.");
                }
                foreach (object item in files)
                {
                    Dictionary<string, object> fileValue =
                        item as Dictionary<string, object>;
                    if (fileValue == null)
                    {
                        throw new InvalidDataException(
                            "Manifest file entry is invalid.");
                    }
                    ManifestFile file = new ManifestFile();
                    file.Path = GetString(fileValue, "path");
                    file.Sha256 = GetString(fileValue, "sha256")
                        .ToLowerInvariant();
                    file.Size = GetInt64(fileValue, "size");
                    file.ObjectPath = GetString(fileValue, "object");
                    if (file.Size < 0 ||
                        file.Sha256.Length != 64 ||
                        !String.Equals(
                            file.ObjectPath,
                            "objects/" + file.Sha256,
                            StringComparison.Ordinal))
                    {
                        throw new InvalidDataException(
                            "Manifest file hash or object path is invalid.");
                    }
                    manifest.Files.Add(file);
                }
                return manifest;
            }
        }

        private sealed class ManifestFile
        {
            public string Path;
            public string Sha256;
            public long Size;
            public string ObjectPath;
        }

        private sealed class UpdateState
        {
            public string Channel;
            public string Version;
            public string BuildId;
            public long ReleaseSequence;

            public static UpdateState Load(string path)
            {
                if (!File.Exists(path))
                {
                    return new UpdateState();
                }
                Dictionary<string, object> value = ParseObject(
                    File.ReadAllText(path, Encoding.UTF8));
                UpdateState state = new UpdateState();
                state.Channel = Convert.ToString(
                    value.ContainsKey("channel") ? value["channel"] : "",
                    CultureInfo.InvariantCulture);
                state.Version = Convert.ToString(
                    value.ContainsKey("version") ? value["version"] : "",
                    CultureInfo.InvariantCulture);
                state.BuildId = Convert.ToString(
                    value.ContainsKey("buildId") ? value["buildId"] : "",
                    CultureInfo.InvariantCulture);
                state.ReleaseSequence = value.ContainsKey("releaseSequence")
                    ? Convert.ToInt64(
                        value["releaseSequence"],
                        CultureInfo.InvariantCulture)
                    : 0;
                return state;
            }

            public void Save(string path)
            {
                Directory.CreateDirectory(Path.GetDirectoryName(path));
                Dictionary<string, object> value =
                    new Dictionary<string, object>();
                value["channel"] = Channel;
                value["version"] = Version;
                value["buildId"] = BuildId;
                value["releaseSequence"] = ReleaseSequence;
                string json = JsonSerializer.Serialize(value);
                string temporary = path + ".new";
                File.WriteAllText(
                    temporary,
                    json + Environment.NewLine,
                    new UTF8Encoding(false));
                if (File.Exists(path))
                {
                    File.Replace(temporary, path, null, true);
                }
                else
                {
                    File.Move(temporary, path);
                }
            }
        }
    }

}
