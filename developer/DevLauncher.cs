using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Windows.Forms;

namespace ManakaMultiplayer.DevLauncher
{
    internal static class Program
    {
        [STAThread]
        private static int Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            string gameRoot = ResolveGameRoot(args);
            if (gameRoot == null)
            {
                MessageBox.Show(
                    "未找到 SecretFlasherManaka.exe。请将 Start-Dev.exe 放到游戏目录后运行。",
                    "逃离赛雷卡开发频道",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
                return 1;
            }

            using (DevLauncherForm form = new DevLauncherForm(gameRoot))
            {
                if (form.ShowDialog() != DialogResult.OK || !form.StartRequested)
                {
                    return 0;
                }
                return form.LaunchGame();
            }
        }

        private static string ResolveGameRoot(string[] args)
        {
            if (args.Length >= 2 &&
                String.Equals(args[0], "--game-root", StringComparison.OrdinalIgnoreCase))
            {
                string explicitRoot = Path.GetFullPath(args[1]);
                return File.Exists(Path.Combine(
                    explicitRoot,
                    "SecretFlasherManaka.exe"))
                    ? explicitRoot
                    : null;
            }

            string directory = Path.GetFullPath(AppDomain.CurrentDomain.BaseDirectory);
            if (File.Exists(Path.Combine(directory, "SecretFlasherManaka.exe")))
            {
                return directory;
            }
            DirectoryInfo parent = Directory.GetParent(directory);
            if (parent != null &&
                File.Exists(Path.Combine(parent.FullName, "SecretFlasherManaka.exe")))
            {
                return parent.FullName;
            }
            return null;
        }
    }

    internal sealed class DevLauncherForm : Form
    {
        private readonly string gameRoot;
        private readonly string channelRoot;
        private readonly string keyPath;
        private readonly string sourcePath;
        private readonly Label titleLabel;
        private readonly Label statusLabel;
        private readonly Label sourceLabel;
        private readonly Button selectButton;
        private readonly Button startButton;
        private readonly Button closeButton;

        public bool StartRequested { get; private set; }

        public DevLauncherForm(string root)
        {
            gameRoot = root;
            channelRoot = Path.Combine(
                root,
                "EscapeFromManaka",
                "Channel");
            keyPath = Path.Combine(channelRoot, "dev.key");
            sourcePath = Path.Combine(channelRoot, "dev-source.txt");

            Text = "逃离赛雷卡开发频道";
            StartPosition = FormStartPosition.CenterScreen;
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            ClientSize = new Size(500, 245);
            Font = new Font("Microsoft YaHei UI", 9F);

            titleLabel = new Label();
            titleLabel.Text = "开发频道启动器";
            titleLabel.Font = new Font(
                "Microsoft YaHei UI",
                14F,
                FontStyle.Bold);
            titleLabel.AutoSize = true;
            titleLabel.Location = new Point(22, 18);

            statusLabel = new Label();
            statusLabel.Location = new Point(24, 62);
            statusLabel.Size = new Size(450, 54);
            statusLabel.ForeColor = Color.DimGray;

            sourceLabel = new Label();
            sourceLabel.Location = new Point(24, 122);
            sourceLabel.Size = new Size(450, 36);
            sourceLabel.AutoEllipsis = true;
            sourceLabel.ForeColor = Color.Gray;

            selectButton = new Button();
            selectButton.Text = "选择 Token 文件";
            selectButton.Location = new Point(24, 181);
            selectButton.Size = new Size(150, 36);
            selectButton.Click += SelectButtonClicked;

            startButton = new Button();
            startButton.Text = "启动开发频道";
            startButton.Location = new Point(186, 181);
            startButton.Size = new Size(170, 36);
            startButton.Click += StartButtonClicked;

            closeButton = new Button();
            closeButton.Text = "取消";
            closeButton.Location = new Point(368, 181);
            closeButton.Size = new Size(110, 36);
            closeButton.Click += delegate { DialogResult = DialogResult.Cancel; };

            Controls.Add(titleLabel);
            Controls.Add(statusLabel);
            Controls.Add(sourceLabel);
            Controls.Add(selectButton);
            Controls.Add(startButton);
            Controls.Add(closeButton);

            Shown += delegate
            {
                if (File.Exists(keyPath))
                {
                    UpdateConfiguredStatus("已检测到开发频道访问密钥。");
                    return;
                }
                if (TryLoadSavedSource())
                {
                    UpdateConfiguredStatus("已从上次选择的 Token 文件恢复配置。");
                    return;
                }
                BeginInvoke(new Action(SelectTokenFile));
            };
        }

        private void SelectButtonClicked(object sender, EventArgs e)
        {
            SelectTokenFile();
        }

        private void SelectTokenFile()
        {
            using (OpenFileDialog dialog = new OpenFileDialog())
            {
                dialog.Title = "选择开发频道 Token 文件";
                dialog.Filter =
                    "Token 文件 (*.txt;*.key)|*.txt;*.key|所有文件 (*.*)|*.*";
                dialog.CheckFileExists = true;
                if (File.Exists(sourcePath))
                {
                    string lastPath = File.ReadAllText(
                        sourcePath,
                        Encoding.UTF8).Trim();
                    if (File.Exists(lastPath))
                    {
                        dialog.InitialDirectory =
                            Path.GetDirectoryName(lastPath);
                        dialog.FileName = Path.GetFileName(lastPath);
                    }
                }
                if (dialog.ShowDialog(this) != DialogResult.OK)
                {
                    UpdateStatus(
                        File.Exists(keyPath)
                            ? "保留现有开发频道配置。"
                            : "尚未配置开发频道访问密钥。",
                        File.Exists(keyPath)
                            ? Color.DimGray
                            : Color.Firebrick);
                    return;
                }
                ConfigureFromToken(dialog.FileName);
            }
        }

        private bool TryLoadSavedSource()
        {
            if (!File.Exists(sourcePath))
            {
                return false;
            }
            try
            {
                string source = File.ReadAllText(
                    sourcePath,
                    Encoding.UTF8).Trim();
                return File.Exists(source) && ConfigureFromToken(source);
            }
            catch
            {
                return false;
            }
        }

        private bool ConfigureFromToken(string tokenPath)
        {
            try
            {
                string token = File.ReadAllText(
                    tokenPath,
                    Encoding.UTF8).Trim();
                if (String.IsNullOrWhiteSpace(token))
                {
                    throw new InvalidDataException("Token 文件内容为空。");
                }

                Directory.CreateDirectory(channelRoot);
                byte[] protectedBytes = ProtectedData.Protect(
                    Encoding.UTF8.GetBytes(token),
                    null,
                    DataProtectionScope.CurrentUser);
                string temporary = keyPath + ".new";
                File.WriteAllBytes(temporary, protectedBytes);
                if (File.Exists(keyPath))
                {
                    File.Replace(temporary, keyPath, null, true);
                }
                else
                {
                    File.Move(temporary, keyPath);
                }
                File.WriteAllText(
                    sourcePath,
                    Path.GetFullPath(tokenPath),
                    new UTF8Encoding(false));
                UpdateConfiguredStatus("Token 配置成功。");
                return true;
            }
            catch (Exception exception)
            {
                UpdateStatus(
                    "Token 配置失败：" + exception.Message,
                    Color.Firebrick);
                return false;
            }
        }

        private void UpdateConfiguredStatus(string message)
        {
            UpdateStatus(
                message + "\r\n点击“启动开发频道”打开游戏。",
                Color.ForestGreen);
            sourceLabel.Text = "Token 来源：" + ReadSavedSource();
            startButton.Enabled = true;
        }

        private void UpdateStatus(string message, Color color)
        {
            statusLabel.Text = message;
            statusLabel.ForeColor = color;
            startButton.Enabled = File.Exists(keyPath);
            sourceLabel.Text = File.Exists(sourcePath)
                ? "Token 来源：" + ReadSavedSource()
                : "";
        }

        private string ReadSavedSource()
        {
            try
            {
                return File.Exists(sourcePath)
                    ? File.ReadAllText(sourcePath, Encoding.UTF8).Trim()
                    : "已加密保存";
            }
            catch
            {
                return "已加密保存";
            }
        }

        private void StartButtonClicked(object sender, EventArgs e)
        {
            if (!File.Exists(keyPath))
            {
                SelectTokenFile();
                if (!File.Exists(keyPath))
                {
                    return;
                }
            }
            StartRequested = true;
            DialogResult = DialogResult.OK;
        }

        public int LaunchGame()
        {
            try
            {
                if (Process.GetProcessesByName("SecretFlasherManaka").Length > 0)
                {
                    MessageBox.Show(
                        "游戏已经在运行。请先关闭游戏，再使用开发启动器。",
                        "逃离赛雷卡开发频道",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Information);
                    return 2;
                }

                ProcessStartInfo startInfo = new ProcessStartInfo(
                    Path.Combine(gameRoot, "SecretFlasherManaka.exe"));
                startInfo.WorkingDirectory = gameRoot;
                startInfo.UseShellExecute = false;
                startInfo.EnvironmentVariables["MANAKA_EFM_CHANNEL"] = "dev";
                Process.Start(startInfo);
                return 0;
            }
            catch (Exception exception)
            {
                MessageBox.Show(
                    "启动开发频道失败：" + exception.Message,
                    "逃离赛雷卡开发频道",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
                return 3;
            }
        }
    }
}
