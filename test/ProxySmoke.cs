using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

internal static class ProxySmoke
{
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern IntPtr LoadLibraryW(string fileName);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr GetProcAddress(IntPtr module, string name);

    [DllImport("kernel32.dll")]
    private static extern bool FreeLibrary(IntPtr module);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate uint VersionDelegate();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate int StartHostDelegate();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private delegate int JoinDelegate(string host);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private delegate int StatusDelegate(StringBuilder buffer, int capacity);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void StopDelegate();

    [UnmanagedFunctionPointer(CallingConvention.Winapi)]
    private delegate bool CheckPlatformDelegate();

    private static T Bind<T>(IntPtr module, string name) where T : class
    {
        IntPtr address = GetProcAddress(module, name);
        if (address == IntPtr.Zero)
        {
            throw new MissingMethodException(name);
        }
        return (T)(object)Marshal.GetDelegateForFunctionPointer(address, typeof(T));
    }

    private static int Main(string[] args)
    {
        Environment.SetEnvironmentVariable("MANAKA_MP_DISABLE_OVERLAY", "1");
        if (args.Length == 0)
        {
            Console.Error.WriteLine("usage: ProxySmoke status|host|client <dll> [host]");
            return 1;
        }

        string mode = args[0];
        string dll = args.Length > 1 ? args[1] : "winhttp.dll";
        IntPtr module = LoadLibraryW(dll);
        if (module == IntPtr.Zero)
        {
            Console.Error.WriteLine("LoadLibrary failed: " + new Win32Exception(Marshal.GetLastWin32Error()).Message);
            return 2;
        }

        try
        {
            VersionDelegate version = Bind<VersionDelegate>(module, "ManakaMultiplayerVersion");
            StatusDelegate status = Bind<StatusDelegate>(module, "ManakaTestStatus");
            Console.WriteLine("proxy-version=" + version().ToString("x8"));

            if (mode == "status")
            {
                CheckPlatformDelegate checkPlatform = Bind<CheckPlatformDelegate>(module, "WinHttpCheckPlatform");
                Console.WriteLine("forwarded-winhttp-call=" + checkPlatform());
                Console.WriteLine("status=" + ReadStatus(status));
                return 0;
            }

            StartHostDelegate startHost = Bind<StartHostDelegate>(module, "ManakaTestStartHost");
            JoinDelegate join = Bind<JoinDelegate>(module, "ManakaTestJoin");
            StopDelegate stop = Bind<StopDelegate>(module, "ManakaTestStop");

            if (mode == "host")
            {
                startHost();
            }
            else if (mode == "client" && args.Length > 2)
            {
                join(args[2]);
            }
            else
            {
                Console.Error.WriteLine("invalid mode or missing host");
                return 3;
            }

            for (int index = 0; index < 50; ++index)
            {
                Thread.Sleep(100);
                string current = ReadStatus(status);
                if (current.StartsWith("CONNECTED", StringComparison.Ordinal))
                {
                    Console.WriteLine("status=" + current);
                    stop();
                    return 0;
                }
            }

            Console.Error.WriteLine("status=" + ReadStatus(status));
            stop();
            return 4;
        }
        finally
        {
            FreeLibrary(module);
        }
    }

    private static string ReadStatus(StatusDelegate status)
    {
        StringBuilder builder = new StringBuilder(2048);
        status(builder, builder.Capacity);
        return builder.ToString();
    }
}
