using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

internal static class WindowProbe
{
    private const uint WM_LBUTTONUP = 0x0202;
    private const uint WM_COMMAND = 0x0111;
    private const int HostCommand = 100;

    [DllImport("user32.dll")]
    private static extern bool EnumWindows(EnumProc callback, IntPtr parameter);

    [DllImport("user32.dll")]
    private static extern bool EnumChildWindows(IntPtr parent, EnumProc callback, IntPtr parameter);

    [DllImport("user32.dll")]
    private static extern uint GetWindowThreadProcessId(IntPtr window, out uint processId);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern int GetWindowTextW(IntPtr window, StringBuilder text, int count);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern int GetClassNameW(IntPtr window, StringBuilder text, int count);

    [DllImport("user32.dll")]
    private static extern bool PostMessageW(IntPtr window, uint message, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll")]
    private static extern IntPtr SendMessageW(IntPtr window, uint message, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll")]
    private static extern IntPtr GetDlgItem(IntPtr dialog, int id);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern IntPtr FindWindowExW(
        IntPtr parent,
        IntPtr childAfter,
        string className,
        string windowName);

    private delegate bool EnumProc(IntPtr window, IntPtr parameter);

    private sealed class Entry
    {
        public IntPtr Handle;
        public string ClassName;
        public string Text;
    }

    private static int Main(string[] args)
    {
        if (args.Length == 1)
        {
            IntPtr messageWindow = FindWindowExW(
                new IntPtr(-3),
                IntPtr.Zero,
                "ManakaMultiplayerTimer",
                null);
            if (messageWindow == IntPtr.Zero)
            {
                Console.Error.WriteLine("native-message-window-not-found");
                return 10;
            }

            const uint wmApp = 0x8000;
            string command = args[0].ToLowerInvariant();
            uint message;
            IntPtr value = IntPtr.Zero;
            switch (command)
            {
                case "ui-open":
                    message = wmApp + 50;
                    break;
                case "ui-close":
                    message = wmApp + 51;
                    break;
                case "ui-equipment":
                    message = wmApp + 52;
                    value = IntPtr.Zero;
                    break;
                case "ui-lobby":
                    message = wmApp + 52;
                    value = new IntPtr(1);
                    break;
                case "ui-host":
                    message = wmApp + 53;
                    value = IntPtr.Zero;
                    break;
                case "ui-join":
                    message = wmApp + 53;
                    value = new IntPtr(1);
                    break;
                case "ui-probe-player":
                    message = wmApp + 54;
                    break;
                case "action-1":
                case "action-2":
                case "action-3":
                case "action-4":
                case "action-5":
                    message = wmApp + 55;
                    value = new IntPtr(command[command.Length - 1] - '1');
                    break;
                default:
                    Console.Error.WriteLine("unknown-test-command");
                    return 11;
            }

            SendMessageW(messageWindow, message, value, IntPtr.Zero);
            Console.WriteLine("command-sent=" + command);
            return 0;
        }

        uint targetProcess;
        if (args.Length < 2 || !uint.TryParse(args[0], out targetProcess))
        {
            Console.Error.WriteLine("usage: WindowProbe <pid> list|host");
            return 1;
        }

        List<Entry> entries = new List<Entry>();
        EnumWindows(delegate(IntPtr window, IntPtr parameter)
        {
            uint processId;
            GetWindowThreadProcessId(window, out processId);
            if (processId == targetProcess)
            {
                Add(window, entries);
                EnumChildWindows(window, delegate(IntPtr child, IntPtr ignored)
                {
                    Add(child, entries);
                    return true;
                }, IntPtr.Zero);
            }
            return true;
        }, IntPtr.Zero);

        foreach (Entry entry in entries)
        {
            Console.WriteLine(entry.Handle.ToInt64().ToString("x") + " " + entry.ClassName + " | " + entry.Text);
        }

        if (args[1].Equals("host", StringComparison.OrdinalIgnoreCase))
        {
            Entry overlay = entries.Find(value => value.ClassName == "ManakaMultiplayerOverlay");
            if (overlay == null)
            {
                Console.Error.WriteLine("overlay-not-found");
                return 2;
            }
            PostMessageW(overlay.Handle, WM_LBUTTONUP, IntPtr.Zero, IntPtr.Zero);
            Thread.Sleep(700);

            entries.Clear();
            EnumWindows(delegate(IntPtr window, IntPtr parameter)
            {
                uint processId;
                GetWindowThreadProcessId(window, out processId);
                if (processId == targetProcess)
                {
                    Add(window, entries);
                    EnumChildWindows(window, delegate(IntPtr child, IntPtr ignored)
                    {
                        Add(child, entries);
                        return true;
                    }, IntPtr.Zero);
                }
                return true;
            }, IntPtr.Zero);

            Entry menu = entries.Find(value => value.ClassName == "ManakaMultiplayerMenu");
            if (menu == null)
            {
                Console.Error.WriteLine("menu-not-found");
                return 3;
            }

            SendMessageW(menu.Handle, WM_COMMAND, new IntPtr(HostCommand), IntPtr.Zero);
            Thread.Sleep(700);
            IntPtr status = GetDlgItem(menu.Handle, 300);
            string text = ReadText(status);
            Console.WriteLine("host-status=" + text);
            return text.StartsWith("ROOM CREATED", StringComparison.Ordinal) ? 0 : 4;
        }

        return 0;
    }

    private static void Add(IntPtr window, List<Entry> entries)
    {
        StringBuilder className = new StringBuilder(256);
        StringBuilder text = new StringBuilder(1024);
        GetClassNameW(window, className, className.Capacity);
        GetWindowTextW(window, text, text.Capacity);
        entries.Add(new Entry { Handle = window, ClassName = className.ToString(), Text = text.ToString() });
    }

    private static string ReadText(IntPtr window)
    {
        StringBuilder text = new StringBuilder(2048);
        GetWindowTextW(window, text, text.Capacity);
        return text.ToString();
    }
}
