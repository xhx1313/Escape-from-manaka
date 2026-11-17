using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using System.Windows.Forms;

namespace ManakaMultiplayer.Setup
{
    internal static class Program
    {
        private static bool IsUninstall
        {
            get
            {
                string name = Path.GetFileNameWithoutExtension(Application.ExecutablePath);
                return string.Equals(name, "uninstall", StringComparison.OrdinalIgnoreCase) ||
                       Array.Exists(Environment.GetCommandLineArgs(), value =>
                           string.Equals(value, "--uninstall", StringComparison.OrdinalIgnoreCase));
            }
        }

        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            string[] arguments = Environment.GetCommandLineArgs();
            int installArgument = Array.IndexOf(arguments, "--install-silent");
            if (installArgument >= 0 && installArgument + 1 < arguments.Length)
            {
                string errorLog = Path.Combine(
                    Application.StartupPath,
                    "ManakaMultiplayer.Setup.error.log");
                try
                {
                    if (File.Exists(errorLog))
                    {
                        File.Delete(errorLog);
                    }
                    InstallerEngine.Install(arguments[installArgument + 1]);
                    Environment.ExitCode = 0;
                }
                catch (Exception exception)
                {
                    try
                    {
                        File.WriteAllText(
                            errorLog,
                            exception.ToString(),
                            new UTF8Encoding(false));
                    }
                    catch
                    {
                    }
                    Environment.ExitCode = 2;
                }
                return;
            }

            int uninstallArgument = Array.IndexOf(arguments, "--uninstall-silent");
            if (uninstallArgument >= 0)
            {
                try
                {
                    string root = uninstallArgument + 1 < arguments.Length &&
                                  !arguments[uninstallArgument + 1].StartsWith("--")
                        ? arguments[uninstallArgument + 1]
                        : Application.StartupPath;
                    InstallerEngine.UninstallCore(root);
                    Environment.ExitCode = 0;
                }
                catch
                {
                    Environment.ExitCode = 3;
                }
                return;
            }

            if (IsUninstall)
            {
                InstallerEngine.Uninstall(
                    InstallerEngine.ResolveUninstallGameRoot(
                        Application.StartupPath));
                return;
            }

            Application.Run(new SetupForm());
        }
    }

    internal sealed class SetupForm : Form
    {
        private readonly TextBox pathBox = new TextBox();
        private readonly Label statusLabel = new Label();
        private readonly Button installButton = new Button();

        public SetupForm()
        {
            Text = "SecretFlasherManaka - Escape from Manaka Setup " + ModVersion.Value;
            StartPosition = FormStartPosition.CenterScreen;
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            ClientSize = new Size(640, 235);
            Font = new Font("Segoe UI", 9F);

            Label title = new Label();
            title.Text = "Install Escape from Manaka " + ModVersion.Value;
            title.Font = new Font("Segoe UI", 14F, FontStyle.Bold);
            title.AutoSize = true;
            title.Location = new Point(20, 18);

            Label description = new Label();
            description.Text = "Select the folder containing SecretFlasherManaka.exe and UnityPlayer.dll.";
            description.AutoSize = true;
            description.Location = new Point(22, 57);

            pathBox.Location = new Point(22, 88);
            pathBox.Size = new Size(470, 27);

            Button browseButton = new Button();
            browseButton.Text = "Browse...";
            browseButton.Location = new Point(504, 86);
            browseButton.Size = new Size(110, 31);
            browseButton.Click += BrowseClicked;

            installButton.Text = "Install / Update";
            installButton.Location = new Point(400, 144);
            installButton.Size = new Size(214, 40);
            installButton.Click += InstallClicked;

            statusLabel.Location = new Point(22, 140);
            statusLabel.Size = new Size(365, 58);
            statusLabel.ForeColor = Color.DimGray;

            Controls.Add(title);
            Controls.Add(description);
            Controls.Add(pathBox);
            Controls.Add(browseButton);
            Controls.Add(installButton);
            Controls.Add(statusLabel);
        }

        private void BrowseClicked(object sender, EventArgs e)
        {
            using (FolderBrowserDialog dialog = new FolderBrowserDialog())
            {
                dialog.Description = "Select the SecretFlasherManaka game folder";
                dialog.ShowNewFolderButton = false;
                if (dialog.ShowDialog(this) == DialogResult.OK)
                {
                    pathBox.Text = dialog.SelectedPath;
                }
            }
        }

        private void InstallClicked(object sender, EventArgs e)
        {
            installButton.Enabled = false;
            statusLabel.Text = "Installing native proxy...";
            try
            {
                string result = InstallerEngine.Install(pathBox.Text);
                statusLabel.ForeColor = Color.ForestGreen;
                statusLabel.Text = "Installed successfully.";
                MessageBox.Show(
                    this,
                    result + Environment.NewLine + Environment.NewLine +
                    "Launch SecretFlasherManaka.exe normally. An ESCAPE FROM MANAKA button will appear over the title screen." +
                    Environment.NewLine +
                    "On the host, allow Windows Firewall access for TCP port 12121.",
                    "Installation complete",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Information);
            }
            catch (Exception exception)
            {
                statusLabel.ForeColor = Color.Firebrick;
                statusLabel.Text = "Installation failed.";
                MessageBox.Show(this, exception.Message, "Installation failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                installButton.Enabled = true;
            }
        }
    }

    internal static class InstallerEngine
    {
        private const string ProxyResourceName = "winhttp.dll";
        private const string UpdateHostResourceName = "UpdateHost.exe";
        private const string RuntimeAssetsResourceName = "runtime-assets.zip";
        private const string PackageDirectoryName = "EscapeFromManaka";
        private const string ProxyFileName = "winhttp.dll";
        private const string UpdateHostFileName = "UpdateHost.exe";
        private const string LegacyLauncherFileName = "Escape from Manaka.exe";
        private const string VeryLegacyLauncherFileName = "ManakaLauncher.exe";
        private const string ProxyOriginalFileName = "winhttp_orig.dll";
        private const string BackupFileName = "winhttp.dll.manaka-backup";
        private const string ManifestFileName = "installed.json";
        private const string UninstallerFileName = "uninstall.exe";
        private const string AssetsDirectoryName = "Assets";
        private const string CheckpointDirectoryName = "Checkpoints";
        private const string AudioDirectoryName = "Audio";
        private const string MapDirectoryName = "Maps";
        private const string MapDevelopmentDirectoryName = "MapsDev";
        private const string DataDirectoryName = "Data";
        private const string FontsDirectoryName = "Fonts";
        private const string LogsDirectoryName = "Logs";
        private const string BackupDirectoryName = "Backup";
        private const string LegacyDirectoryName = "Legacy";
        private const string EquipmentCapturesDirectoryName = "EquipmentCaptures";
        private const string EquipmentColorCapturesDirectoryName = "EquipmentColorCaptures";
        private const string EquipmentPreviewsDirectoryName = "EquipmentPreviews";

        public static string Install(string selectedDirectory)
        {
            string root = ValidateGameDirectory(selectedDirectory);
            StopIfGameIsRunning();

            string packageRoot = Path.Combine(root, PackageDirectoryName);
            Directory.CreateDirectory(packageRoot);
            MigrateLegacyLayout(root, packageRoot);

            string proxyPath = Path.Combine(root, ProxyFileName);
            string originalPath = Path.Combine(root, ProxyOriginalFileName);
            string backupPath = Path.Combine(
                packageRoot,
                BackupDirectoryName,
                BackupFileName);
            byte[] proxyBytes = ReadResource(ProxyResourceName);
            byte[] updateHostBytes = ReadResource(UpdateHostResourceName);

            if (File.Exists(proxyPath) && !HashesEqual(proxyPath, proxyBytes))
            {
                if (!File.Exists(backupPath))
                {
                    File.Move(proxyPath, backupPath);
                }
            }

            string systemWinHttp = Path.Combine(Environment.SystemDirectory, "winhttp.dll");
            if (!File.Exists(systemWinHttp))
            {
                throw new FileNotFoundException("The Windows system winhttp.dll could not be found.", systemWinHttp);
            }

            File.Copy(systemWinHttp, originalPath, true);
            WriteBytesAtomically(proxyPath, proxyBytes);
            WriteBytesAtomically(
                Path.Combine(packageRoot, UpdateHostFileName),
                updateHostBytes);
            DeleteInside(root, Path.Combine(root, LegacyLauncherFileName));
            DeleteInside(root, Path.Combine(root, VeryLegacyLauncherFileName));
            DeleteDesktopShortcut();
            foreach (string managedDirectory in new[]
                     {
                         AssetsDirectoryName,
                         CheckpointDirectoryName,
                         AudioDirectoryName,
                         MapDirectoryName,
                         FontsDirectoryName
                     })
            {
                DeleteDirectoryInside(
                    root,
                    Path.Combine(packageRoot, managedDirectory));
            }
            ExtractArchive(RuntimeAssetsResourceName, packageRoot);
            string equipmentPath = Path.Combine(
                packageRoot,
                AssetsDirectoryName);
            string checkpointPath = Path.Combine(
                packageRoot,
                CheckpointDirectoryName);
            string audioPath = Path.Combine(
                packageRoot,
                AudioDirectoryName);
            string mapPath = Path.Combine(
                packageRoot,
                MapDirectoryName);
            int audioFileCount = Directory.Exists(audioPath)
                ? Directory.GetFiles(
                    audioPath,
                    "*.wav",
                    SearchOption.AllDirectories).Length
                : 0;
            string uninstaller = Path.Combine(packageRoot, UninstallerFileName);
            if (!string.Equals(
                    Path.GetFullPath(Application.ExecutablePath),
                    Path.GetFullPath(uninstaller),
                    StringComparison.OrdinalIgnoreCase))
            {
                InstallUninstaller(uninstaller);
            }
            string legacyUninstaller = Path.Combine(root, UninstallerFileName);
            if (!string.Equals(
                    Path.GetFullPath(Application.ExecutablePath),
                    Path.GetFullPath(legacyUninstaller),
                    StringComparison.OrdinalIgnoreCase))
            {
                DeleteInside(root, legacyUninstaller);
            }

            string manifest =
                "{" + Environment.NewLine +
                "  \"name\": \"SecretFlasherManaka Escape from Manaka\"," + Environment.NewLine +
                "  \"version\": \"" + ModVersion.Value + "\"," + Environment.NewLine +
                "  \"installedAtUtc\": \"" + DateTime.UtcNow.ToString("O") + "\"," + Environment.NewLine +
                "  \"proxySha256\": \"" + ToHex(SHA256.Create().ComputeHash(proxyBytes)) + "\"," + Environment.NewLine +
                "  \"updateHostSha256\": \"" + ToHex(SHA256.Create().ComputeHash(updateHostBytes)) + "\"," + Environment.NewLine +
                "  \"equipmentAssets\": " + EscapeJson(equipmentPath) + "," + Environment.NewLine +
                "  \"checkpointAssets\": " + EscapeJson(checkpointPath) + "," + Environment.NewLine +
                "  \"moaning3Assets\": " + EscapeJson(audioPath) + "," + Environment.NewLine +
                "  \"mapAssets\": " + EscapeJson(mapPath) + "," + Environment.NewLine +
                "  \"moaning3FileCount\": " + audioFileCount + "," + Environment.NewLine +
                "  \"hadOriginalProxy\": " + (File.Exists(backupPath) ? "true" : "false") + Environment.NewLine +
                "}" + Environment.NewLine;
            File.WriteAllText(
                Path.Combine(packageRoot, ManifestFileName),
                manifest,
                new UTF8Encoding(false));

            return "Installed:" + Environment.NewLine +
                   proxyPath + Environment.NewLine +
                   originalPath + Environment.NewLine +
                   equipmentPath + Environment.NewLine +
                   checkpointPath + Environment.NewLine +
                   audioPath + Environment.NewLine +
                   mapPath + Environment.NewLine +
                   uninstaller;
        }

        public static void Uninstall(string selectedDirectory)
        {
            try
            {
                string root = ResolveUninstallGameRoot(selectedDirectory);
                DialogResult confirmation = MessageBox.Show(
                    "Remove the pure-native multiplayer proxy from this game folder?" +
                    Environment.NewLine + Environment.NewLine + root,
                    "Uninstall Multiplayer",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question);
                if (confirmation != DialogResult.Yes)
                {
                    return;
                }

                UninstallCore(root);

                MessageBox.Show(
                    "The native multiplayer proxy was removed. Original files were restored.",
                    "Uninstall complete",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Information);

                ScheduleSelfDelete(Application.ExecutablePath);
            }
            catch (Exception exception)
            {
                MessageBox.Show(exception.Message, "Uninstall failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public static void UninstallCore(string selectedDirectory)
        {
            string root = ResolveUninstallGameRoot(selectedDirectory);
            StopIfGameIsRunning();
            string packageRoot = Path.Combine(root, PackageDirectoryName);

            DeleteInside(root, Path.Combine(root, ProxyFileName));
            DeleteInside(root, Path.Combine(root, ProxyOriginalFileName));
            DeleteInside(root, Path.Combine(root, LegacyLauncherFileName));
            DeleteInside(root, Path.Combine(root, VeryLegacyLauncherFileName));
            DeleteInside(
                root,
                Path.Combine(packageRoot, UpdateHostFileName));
            DeleteDesktopShortcut();
            DeleteInside(root, Path.Combine(packageRoot, ManifestFileName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, AssetsDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, CheckpointDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, AudioDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, MapDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, MapDevelopmentDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, DataDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, FontsDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, LogsDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, EquipmentCapturesDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, EquipmentColorCapturesDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, EquipmentPreviewsDirectoryName));
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, LegacyDirectoryName));

            string backupPath = Path.Combine(
                packageRoot,
                BackupDirectoryName,
                BackupFileName);
            if (File.Exists(backupPath))
            {
                File.Move(backupPath, Path.Combine(root, ProxyFileName));
            }
            DeleteDirectoryInside(
                root,
                Path.Combine(packageRoot, BackupDirectoryName));

            if (!IsPathInside(
                    packageRoot,
                    Application.ExecutablePath))
            {
                DeleteDirectoryInside(root, packageRoot);
            }
        }

        public static string ResolveUninstallGameRoot(string selectedDirectory)
        {
            string selected = Path.GetFullPath(selectedDirectory);
            if (LooksLikeGameDirectory(selected))
            {
                return selected;
            }
            DirectoryInfo parent = Directory.GetParent(selected);
            if (parent != null &&
                string.Equals(
                    Path.GetFileName(selected),
                    PackageDirectoryName,
                    StringComparison.OrdinalIgnoreCase) &&
                LooksLikeGameDirectory(parent.FullName))
            {
                return parent.FullName;
            }
            return selected;
        }

        private static string ValidateGameDirectory(string selectedDirectory)
        {
            if (string.IsNullOrWhiteSpace(selectedDirectory))
            {
                throw new InvalidOperationException("Select the game directory.");
            }

            string root = Path.GetFullPath(selectedDirectory.Trim().Trim('"'));
            string gameExecutable = Path.Combine(root, "SecretFlasherManaka.exe");
            string unityPlayer = Path.Combine(root, "UnityPlayer.dll");
            string gameAssembly = Path.Combine(root, "GameAssembly.dll");
            string dataDirectory = Path.Combine(root, "SecretFlasherManaka_Data");

            if (!File.Exists(gameExecutable) || !File.Exists(unityPlayer) ||
                !File.Exists(gameAssembly) || !Directory.Exists(dataDirectory))
            {
                throw new InvalidOperationException(
                    "The selected folder is not a supported SecretFlasherManaka game directory.");
            }
            return root;
        }

        private static bool LooksLikeGameDirectory(string directory)
        {
            return File.Exists(Path.Combine(directory, "SecretFlasherManaka.exe")) &&
                   File.Exists(Path.Combine(directory, "UnityPlayer.dll")) &&
                   File.Exists(Path.Combine(directory, "GameAssembly.dll")) &&
                   Directory.Exists(Path.Combine(directory, "SecretFlasherManaka_Data"));
        }

        private static void MigrateLegacyLayout(string root, string packageRoot)
        {
            string logsRoot = Path.Combine(packageRoot, LogsDirectoryName);
            string backupRoot = Path.Combine(packageRoot, BackupDirectoryName);
            string dataRoot = Path.Combine(packageRoot, DataDirectoryName);
            string legacyRoot = Path.Combine(packageRoot, LegacyDirectoryName);
            Directory.CreateDirectory(logsRoot);
            Directory.CreateDirectory(backupRoot);
            Directory.CreateDirectory(dataRoot);
            Directory.CreateDirectory(legacyRoot);

            MoveLegacyDirectory(root, packageRoot, "EscapeFromManakaAssets", AssetsDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "EscapeFromManakaAudio", AudioDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "EscapeFromManakaCheckpoints", CheckpointDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "EscapeFromManakaMaps", MapDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "EscapeFromManakaMapsDev", MapDevelopmentDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "EscapeFromManakaFonts", FontsDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "equipment-catalog-captures", EquipmentCapturesDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "equipment-catalog-color-captures", EquipmentColorCapturesDirectoryName);
            MoveLegacyDirectory(root, packageRoot, "equipment-previews", EquipmentPreviewsDirectoryName);

            foreach (string legacyDirectory in Directory.GetDirectories(root))
            {
                string name = Path.GetFileName(legacyDirectory);
                if (name.StartsWith(
                        "equipment-catalog-color-captures.pre-",
                        StringComparison.OrdinalIgnoreCase))
                {
                    MoveDirectory(
                        root,
                        legacyDirectory,
                        Path.Combine(legacyRoot, name));
                }
            }

            MoveMatchingFiles(
                root,
                "ManakaMultiplayer.Native.log*",
                logsRoot);
            MoveMatchingFiles(
                root,
                "ManakaMultiplayer.Setup.error.log*",
                logsRoot);
            MoveMatchingFiles(
                root,
                "EscapeFromManakaInventory_*.json",
                dataRoot);

            MoveFile(
                root,
                Path.Combine(root, "ManakaMultiplayer.installed.json"),
                Path.Combine(packageRoot, ManifestFileName));
            MoveFile(
                root,
                Path.Combine(root, BackupFileName),
                Path.Combine(backupRoot, BackupFileName));
        }

        private static void MoveLegacyDirectory(
            string root,
            string packageRoot,
            string legacyName,
            string destinationName)
        {
            MoveDirectory(
                root,
                Path.Combine(root, legacyName),
                Path.Combine(packageRoot, destinationName));
        }

        private static void MoveDirectory(
            string root,
            string source,
            string destination)
        {
            if (!Directory.Exists(source))
            {
                return;
            }
            string fullRoot = Path.GetFullPath(root)
                .TrimEnd(Path.DirectorySeparatorChar) +
                Path.DirectorySeparatorChar;
            string fullSource = Path.GetFullPath(source);
            string fullDestination = Path.GetFullPath(destination);
            if (!fullSource.StartsWith(fullRoot, StringComparison.OrdinalIgnoreCase) ||
                !fullDestination.StartsWith(fullRoot, StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidOperationException(
                    "Refusing to move an installation directory outside the game folder.");
            }
            Directory.CreateDirectory(fullDestination);
            foreach (string file in Directory.GetFiles(
                         fullSource,
                         "*",
                         SearchOption.AllDirectories))
            {
                string relative = file.Substring(fullSource.Length)
                    .TrimStart(Path.DirectorySeparatorChar);
                string target = Path.Combine(fullDestination, relative);
                Directory.CreateDirectory(Path.GetDirectoryName(target));
                if (File.Exists(target))
                {
                    File.SetAttributes(target, FileAttributes.Normal);
                    File.Delete(target);
                }
                File.Move(file, target);
            }
            Directory.Delete(fullSource, true);
        }

        private static void MoveMatchingFiles(
            string root,
            string pattern,
            string destinationDirectory)
        {
            foreach (string file in Directory.GetFiles(
                         root,
                         pattern,
                         SearchOption.TopDirectoryOnly))
            {
                MoveFile(
                    root,
                    file,
                    Path.Combine(destinationDirectory, Path.GetFileName(file)));
            }
        }

        private static void MoveFile(
            string root,
            string source,
            string destination)
        {
            if (!File.Exists(source))
            {
                return;
            }
            string fullRoot = Path.GetFullPath(root)
                .TrimEnd(Path.DirectorySeparatorChar) +
                Path.DirectorySeparatorChar;
            string fullSource = Path.GetFullPath(source);
            string fullDestination = Path.GetFullPath(destination);
            if (!fullSource.StartsWith(fullRoot, StringComparison.OrdinalIgnoreCase) ||
                !fullDestination.StartsWith(fullRoot, StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidOperationException(
                    "Refusing to move an installation file outside the game folder.");
            }
            Directory.CreateDirectory(Path.GetDirectoryName(fullDestination));
            if (File.Exists(fullDestination))
            {
                File.SetAttributes(fullDestination, FileAttributes.Normal);
                File.Delete(fullDestination);
            }
            File.Move(fullSource, fullDestination);
        }

        private static byte[] ReadResource(string resourceName)
        {
            using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
            {
                if (stream == null)
                {
                    throw new InvalidOperationException("Embedded proxy resource was not found: " + resourceName);
                }
                using (MemoryStream memory = new MemoryStream())
                {
                    stream.CopyTo(memory);
                    return memory.ToArray();
                }
            }
        }

        private static void DeleteDesktopShortcut()
        {
            string shortcutPath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory),
                "Escape from Manaka.lnk");
            if (File.Exists(shortcutPath))
            {
                File.Delete(shortcutPath);
            }
        }

        private static string ExtractArchive(
            string resourceName,
            string targetRoot)
        {
            Directory.CreateDirectory(targetRoot);
            string safeRoot = Path.GetFullPath(targetRoot)
                .TrimEnd(Path.DirectorySeparatorChar) +
                Path.DirectorySeparatorChar;
            byte[] archiveBytes = ReadResource(resourceName);
            using (MemoryStream memory = new MemoryStream(archiveBytes, false))
            using (ZipArchive archive = new ZipArchive(memory, ZipArchiveMode.Read, false))
            {
                foreach (ZipArchiveEntry entry in archive.Entries)
                {
                    string normalized = entry.FullName
                        .Replace('/', Path.DirectorySeparatorChar);
                    string destination = Path.GetFullPath(
                        Path.Combine(targetRoot, normalized));
                    if (!destination.StartsWith(
                            safeRoot,
                            StringComparison.OrdinalIgnoreCase))
                    {
                        throw new InvalidOperationException(
                            "Embedded archive contains an unsafe path: " +
                            entry.FullName);
                    }
                    if (string.IsNullOrEmpty(entry.Name))
                    {
                        Directory.CreateDirectory(destination);
                        continue;
                    }
                    Directory.CreateDirectory(Path.GetDirectoryName(destination));
                    using (Stream source = entry.Open())
                    using (FileStream output = File.Create(destination))
                    {
                        source.CopyTo(output);
                    }
                }
            }
            return targetRoot;
        }

        private static void WriteBytesAtomically(string path, byte[] bytes)
        {
            string temporary = path + ".manaka-new";
            File.WriteAllBytes(temporary, bytes);
            File.Copy(temporary, path, true);
            File.Delete(temporary);
        }

        private static void InstallUninstaller(string destination)
        {
            string temporary = destination + ".manaka-new";
            Exception lastException = null;
            for (int attempt = 0; attempt < 10; ++attempt)
            {
                try
                {
                    File.Copy(Application.ExecutablePath, temporary, true);
                    File.Copy(temporary, destination, true);
                    File.Delete(temporary);
                    return;
                }
                catch (IOException exception)
                {
                    lastException = exception;
                }
                catch (UnauthorizedAccessException exception)
                {
                    lastException = exception;
                }
                System.Threading.Thread.Sleep(300);
            }
            try
            {
                if (File.Exists(temporary))
                {
                    File.Delete(temporary);
                }
            }
            catch
            {
            }
            if (lastException != null)
            {
                throw new IOException(
                    "The new uninstaller could not replace the existing one. " +
                    "Close any running uninstall process and retry.",
                    lastException);
            }
        }

        private static bool HashesEqual(string path, byte[] bytes)
        {
            using (SHA256 sha = SHA256.Create())
            using (FileStream stream = File.OpenRead(path))
            {
                return string.Equals(ToHex(sha.ComputeHash(stream)), ToHex(sha.ComputeHash(bytes)), StringComparison.OrdinalIgnoreCase);
            }
        }

        private static string ToHex(byte[] bytes)
        {
            StringBuilder builder = new StringBuilder(bytes.Length * 2);
            foreach (byte value in bytes)
            {
                builder.Append(value.ToString("x2"));
            }
            return builder.ToString();
        }

        private static void DeleteInside(string root, string target)
        {
            string fullRoot = Path.GetFullPath(root).TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
            string fullTarget = Path.GetFullPath(target);
            if (!fullTarget.StartsWith(fullRoot, StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidOperationException("Refusing to delete a file outside the selected game directory.");
            }
            if (File.Exists(fullTarget))
            {
                File.SetAttributes(fullTarget, FileAttributes.Normal);
                File.Delete(fullTarget);
            }
        }

        private static void DeleteDirectoryInside(string root, string target)
        {
            string fullRoot = Path.GetFullPath(root)
                .TrimEnd(Path.DirectorySeparatorChar) +
                Path.DirectorySeparatorChar;
            string fullTarget = Path.GetFullPath(target)
                .TrimEnd(Path.DirectorySeparatorChar);
            if (!fullTarget.StartsWith(
                    fullRoot,
                    StringComparison.OrdinalIgnoreCase) ||
                string.Equals(
                    fullTarget,
                    fullRoot.TrimEnd(Path.DirectorySeparatorChar),
                    StringComparison.OrdinalIgnoreCase))
            {
                throw new InvalidOperationException(
                    "Refusing to delete a directory outside the selected game directory.");
            }
            if (Directory.Exists(fullTarget))
            {
                Directory.Delete(fullTarget, true);
            }
        }

        private static string EscapeJson(string value)
        {
            if (value == null)
            {
                return "null";
            }
            return "\"" + value
                .Replace("\\", "\\\\")
                .Replace("\"", "\\\"") + "\"";
        }

        private static void StopIfGameIsRunning()
        {
            Process[] processes = Process.GetProcessesByName("SecretFlasherManaka");
            if (processes.Length > 0)
            {
                throw new InvalidOperationException("Close SecretFlasherManaka before installing or uninstalling.");
            }
        }

        private static bool IsPathInside(string directory, string path)
        {
            string fullDirectory = Path.GetFullPath(directory)
                .TrimEnd(Path.DirectorySeparatorChar) +
                Path.DirectorySeparatorChar;
            string fullPath = Path.GetFullPath(path);
            return fullPath.StartsWith(
                fullDirectory,
                StringComparison.OrdinalIgnoreCase);
        }

        private static void ScheduleSelfDelete(string executablePath)
        {
            if (!string.Equals(Path.GetFileName(executablePath), UninstallerFileName, StringComparison.OrdinalIgnoreCase))
            {
                return;
            }

            string scriptPath = Path.Combine(Path.GetTempPath(), "manaka-mp-" + Guid.NewGuid().ToString("N") + ".cmd");
            string parentDirectory = Path.GetDirectoryName(executablePath);
            bool removePackageDirectory = string.Equals(
                Path.GetFileName(parentDirectory),
                PackageDirectoryName,
                StringComparison.OrdinalIgnoreCase);
            string[] commands = removePackageDirectory
                ? new string[]
                {
                    "@echo off",
                    "ping 127.0.0.1 -n 3 > nul",
                    "del /f /q \"" + executablePath + "\"",
                    "rmdir /s /q \"" + parentDirectory + "\"",
                    "del /f /q \"" + scriptPath + "\""
                }
                : new string[]
                {
                    "@echo off",
                    "ping 127.0.0.1 -n 3 > nul",
                    "del /f /q \"" + executablePath + "\"",
                    "del /f /q \"" + scriptPath + "\""
                };
            File.WriteAllLines(
                scriptPath,
                commands,
                Encoding.ASCII);

            ProcessStartInfo start = new ProcessStartInfo();
            start.FileName = "cmd.exe";
            start.Arguments = "/d /c \"" + scriptPath + "\"";
            start.WindowStyle = ProcessWindowStyle.Hidden;
            start.UseShellExecute = true;
            Process.Start(start);
        }
    }
}
