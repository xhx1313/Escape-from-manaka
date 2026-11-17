// Native.cpp module: update_ui.inc
// MOD-entry update prompt. The game remains at the title scene while the
// hidden UpdateHost process checks, downloads, replaces files and restarts.
constexpr UINT kUpdateStartCheckMessage = WM_APP + 90;
constexpr UINT_PTR kUpdatePollTimer = 4;
constexpr int kUpdateLaterButtonId = 501;
constexpr int kUpdatePrimaryButtonId = 502;
constexpr int kUpdateClientWidth = 570;
constexpr int kUpdateClientHeight = 255;
constexpr int kUpdateButtonWidth = 150;
constexpr int kUpdateButtonHeight = 36;
constexpr int kUpdateButtonY = 195;

enum UpdateUiState
{
    UpdateUiState_Unchecked = 0,
    UpdateUiState_Checking,
    UpdateUiState_Available,
    UpdateUiState_Downloading,
    UpdateUiState_Postponed,
    UpdateUiState_Error,
    UpdateUiState_Restarting,
};

struct UpdateHostStatus
{
    std::string phase;
    std::string currentVersion;
    std::string latestVersion;
    long long pendingFiles = 0;
    long long totalBytes = 0;
    long long downloadedBytes = 0;
    std::string currentFile;
    std::string message;
    std::string channel;
    bool channelSwitch = false;
};

std::atomic<int> g_updateUiState{UpdateUiState_Unchecked};
std::atomic<bool> g_updateEntryPermit{false};
std::atomic<bool> g_updateStartRequested{false};
std::atomic<bool> g_updateRestartRequested{false};
std::atomic<bool> g_updateOriginalExitAttempted{false};
std::atomic<bool> g_updateExitFallbackAttempted{false};
std::atomic<unsigned long long> g_updateExitReadyTick{0};
std::atomic<bool> g_updateWindowOpen{false};
std::atomic<unsigned long long> g_updateLastChangeTick{0};
HANDLE g_updateHostProcess = nullptr;
HWND g_updateWindow = nullptr;
HWND g_updateTitleLabel = nullptr;
HWND g_updateBodyLabel = nullptr;
HWND g_updateProgressBar = nullptr;
HWND g_updateLaterButton = nullptr;
HWND g_updatePrimaryButton = nullptr;
HFONT g_updateFont = nullptr;
UpdateHostStatus g_updateStatus;
std::wstring g_updateStatePath;

std::wstring UpdateUi_StateDirectory()
{
    return GetEscapeFromManakaSubdirectory(L"Update");
}

std::wstring UpdateUi_StatePath()
{
    if (g_updateStatePath.empty())
    {
        g_updateStatePath =
            UpdateUi_StateDirectory() + L"\\update-state.json";
    }
    return g_updateStatePath;
}

void UpdateUi_ReconcileInstalledVersion()
{
    const std::wstring path =
        GetEscapeFromManakaDirectory() + L"\\installed.json";
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input)
    {
        return;
    }
    try
    {
        std::string text(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());
        auto value = nlohmann::json::parse(text);
        const std::string version = WideToUtf8(MANAKA_MOD_VERSION_STRING);
        if (value.value("version", "") == version)
        {
            return;
        }
        value["version"] = version;
        value["runtimeVersion"] = version;
        const std::wstring temporary = path + L".new";
        {
            std::ofstream output(
                temporary.c_str(),
                std::ios::binary | std::ios::trunc);
            if (!output)
            {
                return;
            }
            output << value.dump(2) << '\n';
        }
        if (!MoveFileExW(
                temporary.c_str(),
                path.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
            DeleteFileW(temporary.c_str());
            LogInfo(
                LogCategory_Persistence,
                L"Update",
                L"Unable to update installed.json runtime version: error=%lu.",
                GetLastError());
        }
    }
    catch (...)
    {
        LogInfo(
            LogCategory_Persistence,
            L"Update",
            L"Unable to reconcile installed.json version.");
    }
}

void UpdateUi_PromoteStagedUpdateHost()
{
    const std::wstring directory = GetEscapeFromManakaDirectory();
    const std::wstring staged =
        directory + L"\\UpdateHost.new.exe";
    const std::wstring target =
        directory + L"\\UpdateHost.exe";
    if (GetFileAttributesW(staged.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        return;
    }
    if (MoveFileExW(
            staged.c_str(),
            target.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        LogInfo(
            LogCategory_Persistence,
            L"Update",
            L"Promoted staged UpdateHost.exe for the next update.");
    }
    else
    {
        LogInfo(
            LogCategory_Persistence,
            L"Update",
            L"Unable to promote staged UpdateHost.exe: error=%lu.",
            GetLastError());
    }
}

std::wstring UpdateUi_QuoteArgument(const std::wstring& value)
{
    std::wstring quoted = L"\"";
    for (wchar_t character : value)
    {
        if (character == L'"')
        {
            quoted += L'\\';
        }
        quoted += character;
    }
    quoted += L'"';
    return quoted;
}

void UpdateUi_CloseHostProcess(bool terminate)
{
    if (g_updateHostProcess == nullptr)
    {
        return;
    }
    if (terminate &&
        WaitForSingleObject(g_updateHostProcess, 0) == WAIT_TIMEOUT)
    {
        TerminateProcess(g_updateHostProcess, 1);
        WaitForSingleObject(g_updateHostProcess, 2000);
    }
    CloseHandle(g_updateHostProcess);
    g_updateHostProcess = nullptr;
}

bool UpdateUi_StartHost(const wchar_t* mode)
{
    const std::wstring root = GetModuleDirectory();
    const std::wstring hostPath =
        root + L"\\EscapeFromManaka\\UpdateHost.exe";
    if (GetFileAttributesW(hostPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        g_updateStatus.message = "UpdateHost.exe is missing.";
        g_updateUiState = UpdateUiState_Error;
        LogInfo(
            LogCategory_Ui,
            L"Update",
            L"Update host is missing: %s.",
            hostPath.c_str());
        return false;
    }

    DeleteFileW(UpdateUi_StatePath().c_str());
    UpdateUi_CloseHostProcess(true);
    wchar_t channelValue[16]{};
    GetEnvironmentVariableW(
        L"MANAKA_EFM_CHANNEL",
        channelValue,
        static_cast<DWORD>(
            sizeof(channelValue) / sizeof(channelValue[0])));
    const std::wstring updateChannel =
        _wcsicmp(channelValue, L"dev") == 0
            ? L"dev"
            : L"stable";
    std::wstring commandLine =
        UpdateUi_QuoteArgument(hostPath) +
        L" --mode " + mode +
        L" --channel " + updateChannel +
        L" --game-root " + UpdateUi_QuoteArgument(root) +
        L" --state " + UpdateUi_QuoteArgument(UpdateUi_StatePath());
    if (_wcsicmp(mode, L"update") == 0)
    {
        commandLine +=
            L" --parent-pid " +
            std::to_wstring(GetCurrentProcessId());
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION process{};
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');
    const BOOL started = CreateProcessW(
        hostPath.c_str(),
        mutableCommand.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        root.c_str(),
        &startup,
        &process);
    if (!started)
    {
        g_updateStatus.message =
            "Unable to start UpdateHost.exe (error " +
            std::to_string(GetLastError()) + ").";
        g_updateUiState = UpdateUiState_Error;
        LogInfo(
            LogCategory_Ui,
            L"Update",
            L"Unable to start update host: error=%lu.",
            GetLastError());
        return false;
    }

    g_updateHostProcess = process.hProcess;
    CloseHandle(process.hThread);
    g_updateLastChangeTick = GetTickCount64();
    LogInfo(
        LogCategory_Ui,
        L"Update",
        L"Update host started in mode %s channel=%s. pid=%lu.",
        mode,
        updateChannel.c_str(),
        process.dwProcessId);
    return true;
}

bool UpdateUi_ReadStatus(UpdateHostStatus& status)
{
    const std::wstring path = UpdateUi_StatePath();
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input)
    {
        return false;
    }
    std::string text(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    if (text.empty())
    {
        return false;
    }

    try
    {
        const auto value = nlohmann::json::parse(text);
        status.phase = value.value("phase", "");
        status.currentVersion = value.value("currentVersion", "");
        status.latestVersion = value.value("latestVersion", "");
        status.pendingFiles = value.value("pendingFiles", 0LL);
        status.totalBytes = value.value("totalDownloadBytes", 0LL);
        status.downloadedBytes = value.value("downloadedBytes", 0LL);
        status.currentFile = value.value("currentFile", "");
        status.message = value.value("message", "");
        status.channel = value.value("channel", "");
        status.channelSwitch = value.value("channelSwitch", false);
        return !status.phase.empty();
    }
    catch (...)
    {
        return false;
    }
}

std::wstring UpdateUi_FormatBytes(long long bytes)
{
    if (bytes >= 1024LL * 1024LL * 1024LL)
    {
        wchar_t value[64]{};
        swprintf_s(
            value,
            L"%.2f GB",
            static_cast<double>(bytes) /
                (1024.0 * 1024.0 * 1024.0));
        return value;
    }
    if (bytes >= 1024LL * 1024LL)
    {
        wchar_t value[64]{};
        swprintf_s(
            value,
            L"%.2f MB",
            static_cast<double>(bytes) / (1024.0 * 1024.0));
        return value;
    }
    if (bytes >= 1024LL)
    {
        wchar_t value[64]{};
        swprintf_s(
            value,
            L"%.1f KB",
            static_cast<double>(bytes) / 1024.0);
        return value;
    }
    return std::to_wstring(bytes) + L" B";
}

void UpdateUi_SetText(HWND control, const std::wstring& text)
{
    if (control != nullptr && IsWindow(control))
    {
        SetWindowTextW(control, text.c_str());
    }
}

void UpdateUi_UpdateWindowControls()
{
    if (g_updateWindow == nullptr || !IsWindow(g_updateWindow))
    {
        return;
    }

    if (g_updateUiState.load() == UpdateUiState_Checking)
    {
        UpdateUi_SetText(g_updateTitleLabel, L"正在检查更新");
        UpdateUi_SetText(
            g_updateBodyLabel,
            L"正在验证服务器版本和更新签名，请稍候。");
        ShowWindow(g_updateProgressBar, SW_HIDE);
        ShowWindow(g_updatePrimaryButton, SW_HIDE);
        ShowWindow(g_updateLaterButton, SW_SHOW);
        MoveWindow(
            g_updateLaterButton,
            (kUpdateClientWidth - kUpdateButtonWidth) / 2,
            kUpdateButtonY,
            kUpdateButtonWidth,
            kUpdateButtonHeight,
            TRUE);
        UpdateUi_SetText(g_updateLaterButton, L"跳过检查");
        return;
    }

    if (g_updateUiState.load() == UpdateUiState_Available)
    {
        const std::wstring current =
            Utf8ToWide(g_updateStatus.currentVersion);
        const std::wstring latest =
            Utf8ToWide(g_updateStatus.latestVersion);
        std::wstring body =
            g_updateStatus.channelSwitch
                ? std::wstring(L"即将切换更新频道。\r\n")
                : std::wstring();
        body +=
            L"当前版本：" + current +
            L"\r\n最新版本：" + latest +
            L"\r\n需要下载：" +
            std::to_wstring(g_updateStatus.pendingFiles) +
            L" 个文件，" +
            UpdateUi_FormatBytes(g_updateStatus.totalBytes);
        UpdateUi_SetText(g_updateTitleLabel, L"发现新版本");
        UpdateUi_SetText(g_updateBodyLabel, body);
        ShowWindow(g_updateProgressBar, SW_HIDE);
        ShowWindow(g_updateLaterButton, SW_SHOW);
        MoveWindow(
            g_updateLaterButton,
            234,
            kUpdateButtonY,
            kUpdateButtonWidth,
            kUpdateButtonHeight,
            TRUE);
        UpdateUi_SetText(g_updateLaterButton, L"稍后进入");
        ShowWindow(g_updatePrimaryButton, SW_SHOW);
        MoveWindow(
            g_updatePrimaryButton,
            396,
            kUpdateButtonY,
            kUpdateButtonWidth,
            kUpdateButtonHeight,
            TRUE);
        EnableWindow(g_updatePrimaryButton, TRUE);
        UpdateUi_SetText(g_updatePrimaryButton, L"立即更新");
        return;
    }

    if (g_updateUiState.load() == UpdateUiState_Downloading)
    {
        const int percent =
            g_updateStatus.totalBytes > 0
                ? static_cast<int>(
                    g_updateStatus.downloadedBytes * 100 /
                    g_updateStatus.totalBytes)
                : 0;
        std::wstring body =
            L"正在下载更新：" +
            std::to_wstring(percent) +
            L"%  (" +
            UpdateUi_FormatBytes(g_updateStatus.downloadedBytes) +
            L" / " +
            UpdateUi_FormatBytes(g_updateStatus.totalBytes) +
            L")\r\n" +
            Utf8ToWide(g_updateStatus.currentFile);
        UpdateUi_SetText(g_updateTitleLabel, L"更新下载中");
        UpdateUi_SetText(g_updateBodyLabel, body);
        ShowWindow(g_updateProgressBar, SW_SHOW);
        SendMessageW(
            g_updateProgressBar,
            PBM_SETRANGE,
            0,
            MAKELPARAM(0, 100));
        SendMessageW(
            g_updateProgressBar,
            PBM_SETPOS,
            static_cast<WPARAM>(std::clamp(percent, 0, 100)),
            0);
        ShowWindow(g_updateLaterButton, SW_HIDE);
        ShowWindow(g_updatePrimaryButton, SW_SHOW);
        MoveWindow(
            g_updatePrimaryButton,
            (kUpdateClientWidth - kUpdateButtonWidth) / 2,
            kUpdateButtonY,
            kUpdateButtonWidth,
            kUpdateButtonHeight,
            TRUE);
        UpdateUi_SetText(g_updatePrimaryButton, L"取消下载");
        return;
    }

    if (g_updateUiState.load() == UpdateUiState_Restarting)
    {
        UpdateUi_SetText(g_updateTitleLabel, L"更新准备完成");
        UpdateUi_SetText(
            g_updateBodyLabel,
            L"下载和校验已经完成，游戏将自动关闭并安装更新。");
        ShowWindow(g_updateProgressBar, SW_SHOW);
        SendMessageW(g_updateProgressBar, PBM_SETPOS, 100, 0);
        ShowWindow(g_updateLaterButton, SW_HIDE);
        ShowWindow(g_updatePrimaryButton, SW_HIDE);
        return;
    }

    if (g_updateUiState.load() == UpdateUiState_Error)
    {
        UpdateUi_SetText(g_updateTitleLabel, L"更新检查失败");
        UpdateUi_SetText(
            g_updateBodyLabel,
            Utf8ToWide(g_updateStatus.message));
        ShowWindow(g_updateProgressBar, SW_HIDE);
        ShowWindow(g_updatePrimaryButton, SW_HIDE);
        ShowWindow(g_updateLaterButton, SW_SHOW);
        MoveWindow(
            g_updateLaterButton,
            (kUpdateClientWidth - kUpdateButtonWidth) / 2,
            kUpdateButtonY,
            kUpdateButtonWidth,
            kUpdateButtonHeight,
            TRUE);
        UpdateUi_SetText(g_updateLaterButton, L"继续进入");
    }
}

void UpdateUi_CloseWindow(bool permitEntry)
{
    g_updateWindowOpen = false;
    if (permitEntry)
    {
        g_updateEntryPermit = true;
    }
    if (g_updateWindow != nullptr && IsWindow(g_updateWindow))
    {
        DestroyWindow(g_updateWindow);
    }
}

void UpdateUi_HandleLater()
{
    if (g_updateUiState.load() == UpdateUiState_Downloading)
    {
        if (MessageBoxW(
                g_updateWindow,
                L"确定取消更新吗？当前版本不会被修改。",
                L"取消更新",
                MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
        {
            return;
        }
        UpdateUi_CloseHostProcess(true);
        g_updateUiState = UpdateUiState_Postponed;
        UpdateUi_CloseWindow(true);
        return;
    }

    if (g_updateUiState.load() == UpdateUiState_Checking)
    {
        UpdateUi_CloseHostProcess(true);
    }
    g_updateUiState = UpdateUiState_Postponed;
    UpdateUi_CloseWindow(true);
}

void UpdateUi_HandlePrimary()
{
    if (g_updateUiState.load() == UpdateUiState_Available)
    {
        g_updateUiState = UpdateUiState_Downloading;
        g_updateStatus.downloadedBytes = 0;
        g_updateStatus.currentFile = "";
        if (!UpdateUi_StartHost(L"update"))
        {
            UpdateUi_UpdateWindowControls();
        }
        return;
    }

    if (g_updateUiState.load() == UpdateUiState_Downloading)
    {
        UpdateUi_HandleLater();
    }
}

LRESULT CALLBACK UpdateWindowProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        g_updateFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        g_updateTitleLabel = CreateWindowW(
            L"STATIC",
            L"正在检查更新",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            24,
            18,
            kUpdateClientWidth - 48,
            30,
            window,
            nullptr,
            g_instance,
            nullptr);
        g_updateBodyLabel = CreateWindowW(
            L"STATIC",
            L"",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            24,
            58,
            kUpdateClientWidth - 48,
            72,
            window,
            nullptr,
            g_instance,
            nullptr);
        g_updateProgressBar = CreateWindowExW(
            0,
            PROGRESS_CLASSW,
            L"",
            WS_CHILD | PBS_SMOOTH,
            24,
            137,
            kUpdateClientWidth - 48,
            18,
            window,
            nullptr,
            g_instance,
            nullptr);
        g_updateLaterButton = CreateWindowW(
            L"BUTTON",
            L"跳过检查",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            234,
            kUpdateButtonY,
            kUpdateButtonWidth,
            kUpdateButtonHeight,
            window,
            reinterpret_cast<HMENU>(kUpdateLaterButtonId),
            g_instance,
            nullptr);
        g_updatePrimaryButton = CreateWindowW(
            L"BUTTON",
            L"立即更新",
            WS_CHILD | BS_PUSHBUTTON,
            396,
            kUpdateButtonY,
            kUpdateButtonWidth,
            kUpdateButtonHeight,
            window,
            reinterpret_cast<HMENU>(kUpdatePrimaryButtonId),
            g_instance,
            nullptr);
        for (HWND control : {
                 g_updateTitleLabel,
                 g_updateBodyLabel,
                 g_updateProgressBar,
                 g_updateLaterButton,
                 g_updatePrimaryButton})
        {
            SendMessageW(
                control,
                WM_SETFONT,
                reinterpret_cast<WPARAM>(g_updateFont),
                TRUE);
        }
        SetTimer(window, kUpdatePollTimer, 100, nullptr);
        UpdateUi_UpdateWindowControls();
        return 0;
    }
    case WM_TIMER:
        if (wParam != kUpdatePollTimer)
        {
            break;
        }
        {
            UpdateHostStatus status;
            if (!UpdateUi_ReadStatus(status))
            {
                if (g_updateHostProcess != nullptr &&
                    WaitForSingleObject(g_updateHostProcess, 0) ==
                        WAIT_OBJECT_0 &&
                    GetTickCount64() - g_updateLastChangeTick > 1500)
                {
                    g_updateStatus.message =
                        "更新程序没有返回状态。";
                    g_updateUiState = UpdateUiState_Error;
                    UpdateUi_UpdateWindowControls();
                }
                return 0;
            }

            g_updateStatus = status;
            if (status.phase == "available")
            {
                g_updateUiState = UpdateUiState_Available;
            }
            else if (status.phase == "downloading")
            {
                g_updateUiState = UpdateUiState_Downloading;
            }
            else if (status.phase == "ready" ||
                     status.phase == "waiting" ||
                     status.phase == "completed")
            {
                if (g_updateUiState.exchange(UpdateUiState_Restarting) !=
                    UpdateUiState_Restarting)
                {
                    g_updateRestartRequested = true;
                    g_updateExitReadyTick = GetTickCount64();
                    LogInfo(
                        LogCategory_Ui,
                        L"Update",
                        L"Update download is ready; restarting the game.");
                }
            }
            else if (status.phase == "current")
            {
                UpdateUi_CloseHostProcess(false);
                g_updateUiState = UpdateUiState_Postponed;
                UpdateUi_CloseWindow(true);
                return 0;
            }
            else if (status.phase == "error")
            {
                UpdateUi_CloseHostProcess(false);
                g_updateUiState = UpdateUiState_Error;
            }
            UpdateUi_UpdateWindowControls();
            return 0;
        }
    case WM_COMMAND:
        if (LOWORD(wParam) == kUpdateLaterButtonId)
        {
            UpdateUi_HandleLater();
            return 0;
        }
        if (LOWORD(wParam) == kUpdatePrimaryButtonId)
        {
            UpdateUi_HandlePrimary();
            return 0;
        }
        break;
    case WM_CLOSE:
        UpdateUi_HandleLater();
        return 0;
    case WM_DESTROY:
        KillTimer(window, kUpdatePollTimer);
        if (window == g_updateWindow)
        {
            g_updateWindow = nullptr;
            g_updateTitleLabel = nullptr;
            g_updateBodyLabel = nullptr;
            g_updateProgressBar = nullptr;
            g_updateLaterButton = nullptr;
            g_updatePrimaryButton = nullptr;
            g_updateWindowOpen = false;
        }
        return 0;
    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

void UpdateUi_OpenWindow()
{
    if (g_updateWindow != nullptr && IsWindow(g_updateWindow))
    {
        ShowWindow(g_updateWindow, SW_SHOW);
        SetForegroundWindow(g_updateWindow);
        return;
    }

    INITCOMMONCONTROLSEX controls{};
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&controls);

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = UpdateWindowProc;
    windowClass.hInstance = g_instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground =
        reinterpret_cast<HBRUSH>(
            static_cast<INT_PTR>(COLOR_WINDOW + 1));
    windowClass.lpszClassName = L"ManakaUpdateWindow";
    RegisterClassExW(&windowClass);

    RECT gameRect{};
    if (g_gameWindow == nullptr ||
        !GetWindowRect(g_gameWindow, &gameRect))
    {
        gameRect = RECT{
            0,
            0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN)};
    }
    const DWORD windowStyle =
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    const DWORD extendedStyle =
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    RECT windowRect{
        0,
        0,
        kUpdateClientWidth,
        kUpdateClientHeight};
    AdjustWindowRectEx(
        &windowRect,
        windowStyle,
        FALSE,
        extendedStyle);
    const int windowWidth = windowRect.right - windowRect.left;
    const int windowHeight = windowRect.bottom - windowRect.top;
    const int x =
        gameRect.left +
        ((gameRect.right - gameRect.left) - windowWidth) / 2;
    const int y =
        gameRect.top +
        ((gameRect.bottom - gameRect.top) - windowHeight) / 2;
    g_updateWindow = CreateWindowExW(
        extendedStyle,
        L"ManakaUpdateWindow",
        L"Escape from Manaka 更新",
        windowStyle,
        x,
        y,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        g_instance,
        nullptr);
    if (g_updateWindow != nullptr)
    {
        g_updateWindowOpen = true;
        ShowWindow(g_updateWindow, SW_SHOW);
        UpdateWindow(g_updateWindow);
        SetForegroundWindow(g_updateWindow);
    }
}

bool UpdateUi_HandleRuntimeMessage(
    UINT message,
    WPARAM,
    LPARAM)
{
    if (message != kUpdateStartCheckMessage)
    {
        return false;
    }
    g_updateStatus = UpdateHostStatus{};
    g_updateUiState = UpdateUiState_Checking;
    UpdateUi_OpenWindow();
    UpdateUi_StartHost(L"check");
    UpdateUi_UpdateWindowControls();
    return true;
}

void UpdateUi_TickRuntimeThread()
{
    if (g_updateStartRequested.load() &&
        g_updateUiState.load() == UpdateUiState_Checking &&
        !g_updateWindowOpen.load() &&
        g_messageWindow != nullptr &&
        g_updateHostProcess == nullptr)
    {
        UpdateUi_OpenWindow();
        UpdateUi_StartHost(L"check");
        UpdateUi_UpdateWindowControls();
    }

}

void UpdateUi_TickMainThread()
{
    if (!g_updateRestartRequested.load())
    {
        return;
    }

    if (!g_updateOriginalExitAttempted.exchange(true))
    {
        auto* exitMethod =
            g_titleClass != nullptr
                ? FindMethod(g_titleClass, "OnClickExitGame", 0)
                : nullptr;
        if (exitMethod != nullptr &&
            g_titleInstance != nullptr &&
            ExtendedMode_IsUnityObjectAlive(g_titleInstance))
        {
            LogInfo(
                LogCategory_Ui,
                L"Update",
                L"Invoking the original TitleSceneView.OnClickExitGame path.");
            Invoke(exitMethod, g_titleInstance, nullptr);
            return;
        }

        LogInfo(
            LogCategory_Ui,
            L"Update",
            L"Original title exit handler is unavailable; using fallback.");
        g_updateExitReadyTick = 0;
    }

    const unsigned long long readyTick = g_updateExitReadyTick.load();
    const unsigned long long now = GetTickCount64();
    if (readyTick != 0 &&
        now - readyTick < 2000)
    {
        return;
    }
    if (g_updateExitFallbackAttempted.exchange(true))
    {
        return;
    }

    auto* applicationClass = FindClass("UnityEngine", "Application");
    auto* quitMethod =
        applicationClass != nullptr
            ? FindMethod(applicationClass, "Quit", 0)
            : nullptr;
    if (quitMethod != nullptr)
    {
        LogInfo(
            LogCategory_Ui,
            L"Update",
            L"Invoking UnityEngine.Application.Quit fallback.");
        Invoke(quitMethod, nullptr, nullptr);
    }

    const HWND gameWindow = FindGameWindow();
    const BOOL posted =
        gameWindow != nullptr
            ? PostMessageW(gameWindow, WM_CLOSE, 0, 0)
            : FALSE;
    LogInfo(
        LogCategory_Ui,
        L"Update",
        L"Update close fallback: hwnd=%p posted=%d error=%lu.",
        gameWindow,
        posted ? 1 : 0,
        posted ? 0 : GetLastError());
}

void UpdateUi_RequestModEntry()
{
    if (!g_updateStartRequested.load())
    {
        if (!g_updateStartRequested.exchange(true))
        {
            g_updateUiState = UpdateUiState_Checking;
            if (g_messageWindow != nullptr)
            {
                PostMessageW(
                    g_messageWindow,
                    kUpdateStartCheckMessage,
                    0,
                    0);
            }
            return;
        }
    }

    const int state = g_updateUiState.load();
    if (state == UpdateUiState_Available ||
        state == UpdateUiState_Downloading)
    {
        if (g_updateWindow != nullptr)
        {
            ShowWindow(g_updateWindow, SW_SHOW);
            SetForegroundWindow(g_updateWindow);
        }
        return;
    }
    if (state != UpdateUiState_Checking)
    {
        g_updateEntryPermit = true;
    }
}

bool UpdateUi_ConsumeModEntryPermit()
{
    return g_updateEntryPermit.exchange(false);
}
