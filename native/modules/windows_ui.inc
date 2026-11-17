// Native.cpp module: windows_ui.inc
// Included inside the shared implementation namespace by Native.cpp.
HHOOK g_gameplayKeyboardHook = nullptr;
HHOOK g_extendedMouseHook = nullptr;
WNDPROC g_originalGameplayWindowProc = nullptr;
HWND g_gameplayWindowSubclassTarget = nullptr;
std::atomic<bool> g_gameplayInventoryTogglePhysicalDown{false};
std::atomic<bool> g_gameplayInventoryClosePendingUntilRelease{false};
std::atomic<bool> g_eyeMaskVisionAdjustmentKeyDown[2]{
    false,
    false
};
std::atomic<bool> g_eyeMaskVisionAdjustmentKeyConsumed[2]{
    false,
    false
};

bool GameplayInput_ProcessEyeMaskVisionAdjustmentKey(
    int virtualKey,
    bool keyDown,
    bool keyUp,
    const wchar_t* source)
{
    const int direction =
        virtualKey == VK_UP
            ? 1
            : (virtualKey == VK_DOWN ? -1 : 0);
    if (direction == 0)
    {
        return false;
    }
    const int index = direction > 0 ? 0 : 1;
    if (keyDown)
    {
        const bool wasDown =
            g_eyeMaskVisionAdjustmentKeyDown[index].exchange(true);
        if (wasDown)
        {
            return g_eyeMaskVisionAdjustmentKeyConsumed[index].load();
        }
        const bool available =
            g_extendedModeState.load() == ExtendedMode_Gameplay &&
            g_eyeMaskVisionAdjustmentAvailable.load() &&
            !g_extendedOriginalSystemMenuVisible.load() &&
            !g_extendedOriginalInteractionMenuActive.load() &&
            !g_extendedOriginalHandcuffsPanelActive.load() &&
            !GameplayInput_IsCustomModalUiOpen();
        if (available &&
            EyeMaskVision_RequestAdjustment(direction, source))
        {
            g_eyeMaskVisionAdjustmentKeyConsumed[index] = true;
            return true;
        }
        return false;
    }
    if (keyUp)
    {
        g_eyeMaskVisionAdjustmentKeyDown[index] = false;
        return g_eyeMaskVisionAdjustmentKeyConsumed[index].exchange(false);
    }
    return false;
}

void GameplayInput_ResetEyeMaskVisionAdjustmentKeys()
{
    for (int index = 0; index < 2; ++index)
    {
        g_eyeMaskVisionAdjustmentKeyDown[index] = false;
        g_eyeMaskVisionAdjustmentKeyConsumed[index] = false;
    }
}

bool GameplayInput_QueueInventoryToggle(const wchar_t* source)
{
    if (g_extendedToggleGameplayInventoryRequested.exchange(true))
    {
        return false;
    }
    LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
        L"Gameplay inventory toggle queued by %s.",
        source != nullptr ? source : L"input");
    return true;
}

void Runtime_RequestShutdown(const wchar_t* source)
{
    bool expected = false;
    if (!g_runtimeShutdownRequested.compare_exchange_strong(
            expected,
            true))
    {
        return;
    }
    g_stopRequested = true;
    g_networkStop = true;
    LogInfo(
        LogCategory_Lifecycle,
        L"ShutdownRequest",
        L"Escape runtime shutdown requested by %s.",
        source != nullptr ? source : L"the host");
}

LRESULT CALLBACK GameplayWindowProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (message == WM_KILLFOCUS)
    {
        g_gameplayInventoryTogglePhysicalDown = false;
        GameplayInput_ResetEyeMaskVisionAdjustmentKeys();
        g_gameplayInventoryClosePendingUntilRelease = false;
        GameplayWorldMap_ResetPhysicalInput();
        CheckpointSystem_ResetMapEditorPhysicalKeys();
        GameplayInput_ResetEscapeDismissState();
    }
    if (message == WM_CLOSE ||
        message == WM_DESTROY ||
        message == WM_NCDESTROY ||
        (message == WM_SYSCOMMAND &&
         (wParam & 0xfff0u) == SC_CLOSE) ||
        (message == WM_ENDSESSION && wParam != 0))
    {
        Runtime_RequestShutdown(L"the game window");
    }
    const bool keyboardMessage =
        message == WM_KEYDOWN ||
        message == WM_KEYUP ||
        message == WM_SYSKEYDOWN ||
        message == WM_SYSKEYUP;
    const int state = g_extendedModeState.load();
    if (keyboardMessage &&
        (state == ExtendedMode_Active ||
         state == ExtendedMode_Gameplay))
    {
        const int virtualKey = static_cast<int>(wParam);
        const bool keyDown =
            message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
        const bool keyUp =
            message == WM_KEYUP || message == WM_SYSKEYUP;
        if (g_extendedOriginalTutorialActive.load())
        {
            if (virtualKey == VK_ESCAPE)
            {
                g_extendedOpenSystemMenuRequested = false;
                g_extendedEscapeDismissBlockedUntil.store(
                    GetTickCount64() + 1000);
            }
            return g_originalGameplayWindowProc != nullptr
                ? CallWindowProcW(
                    g_originalGameplayWindowProc,
                    window,
                    message,
                    wParam,
                    lParam)
                : DefWindowProcW(window, message, wParam, lParam);
        }
        if (ModGuideUi_ProcessPhysicalKey(
                virtualKey,
                keyDown,
                L"the game-window message path"))
        {
            return 0;
        }
        if (ModGuideUi_IsOpen())
        {
            GameplayInput_ProcessEscapeDismissKey(
                virtualKey,
                keyDown,
                keyUp,
                L"the game-window message path while the MOD guide is open");
            return 0;
        }
    }
    if (keyboardMessage &&
        (state == ExtendedMode_Active ||
         state == ExtendedMode_Gameplay) &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        const int virtualKey = static_cast<int>(wParam);
        const bool keyDown =
            message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
        const bool keyUp =
            message == WM_KEYUP || message == WM_SYSKEYUP;
        const bool systemShortcut =
            (GameplayInput_ReadRawAsyncKeyState(VK_MENU) & 0x8000) != 0 ||
            (GameplayInput_ReadRawAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ||
            (GameplayInput_ReadRawAsyncKeyState(VK_LWIN) & 0x8000) != 0 ||
            (GameplayInput_ReadRawAsyncKeyState(VK_RWIN) & 0x8000) != 0;
        if (!systemShortcut)
        {
            if (GameplayInput_ProcessEscapeDismissKey(
                    virtualKey,
                    keyDown,
                    keyUp,
                    L"the game-window message path"))
            {
                return 0;
            }
            if (GameplayInput_ProcessEyeMaskVisionAdjustmentKey(
                    virtualKey,
                    keyDown,
                    keyUp,
                    L"the game-window message path"))
            {
                return 0;
            }
            if (GameplayWorldMap_ProcessPhysicalKey(
                    virtualKey,
                    keyDown,
                    L"the game-window message path"))
            {
                return 0;
            }
            if (g_gameplayWorldMapOpen.load())
            {
                return 0;
            }
            if (CheckpointSystem_ProcessMapEditorPhysicalKey(
                    virtualKey,
                    keyDown,
                    L"the game-window message path"))
            {
                return 0;
            }
            if (CheckpointSystem_IsMapEditorUiOpen())
            {
                if (!CheckpointSystem_IsMapEditorTextInputFocused())
                {
                    return 0;
                }
                return g_originalGameplayWindowProc != nullptr
                    ? CallWindowProcW(
                        g_originalGameplayWindowProc,
                        window,
                        message,
                        wParam,
                        lParam)
                    : DefWindowProcW(
                        window,
                        message,
                        wParam,
                        lParam);
            }
            if (virtualKey == VK_TAB)
            {
                if (keyDown)
                {
                    const bool wasDown =
                        g_gameplayInventoryTogglePhysicalDown.exchange(
                            true);
                    if (!wasDown)
                    {
                        if (g_gameplayInventoryOpen.load())
                        {
                            g_gameplayInventoryClosePendingUntilRelease =
                                true;
                        }
                        else
                        {
                            const bool allowed =
                                !g_extendedPoseMenuVisible.load() &&
                                !g_extendedOriginalInteractionMenuActive.load() &&
                                !g_extendedOriginalHandcuffsPanelActive.load() &&
                                !CheckpointSystem_IsTaskUiOpen();
                            if (allowed)
                            {
                                GameplayInput_QueueInventoryToggle(
                                    L"the game-window Tab press");
                            }
                        }
                    }
                }
                else if (keyUp)
                {
                    g_gameplayInventoryTogglePhysicalDown = false;
                    if (g_gameplayInventoryClosePendingUntilRelease.exchange(
                            false) &&
                        g_gameplayInventoryOpen.load())
                    {
                        GameplayInput_QueueInventoryToggle(
                            L"the game-window Tab release");
                    }
                }
                return 0;
            }
            if (virtualKey == 'E' &&
                !g_extendedOriginalInteractionMenuActive.load() &&
                !g_extendedOriginalHandcuffsPanelActive.load() &&
                !g_gameplayInventoryOpen.load() &&
                !CheckpointSystem_IsTaskUiOpen())
            {
                g_extendedPoseMenuHeld = keyDown;
                return 0;
            }
            if (virtualKey == 'F')
            {
                if (keyUp)
                {
                    g_checkpointInteractActionDown = false;
                    if (g_checkpointInteractionKeyConsumed.exchange(false))
                    {
                        return 0;
                    }
                }
                if (keyDown &&
                    !g_extendedOriginalInteractionMenuActive.load() &&
                    !g_extendedOriginalHandcuffsPanelActive.load() &&
                    !g_gameplayInventoryOpen.load() &&
                    !CheckpointSystem_IsTaskUiOpen() &&
                    CheckpointSystem_HasNearbyInteraction())
                {
                    bool expected = false;
                    if (g_checkpointInteractActionDown.compare_exchange_strong(
                            expected,
                            true))
                    {
                        g_checkpointInteractionKeyConsumed = true;
                        g_checkpointSearchRequested = true;
                        LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                            L"Checkpoint interaction window message consumed for a nearby Escape point.");
                    }
                    return 0;
                }
            }
            if (virtualKey == VK_TAB ||
                GameplayInput_IsCustomModalUiOpen() ||
                g_extendedOriginalHandcuffsPanelActive.load())
            {
                static bool loggedWindowBlock[256]{};
                if (keyDown &&
                    virtualKey >= 0 &&
                    virtualKey < 256 &&
                    !loggedWindowBlock[virtualKey])
                {
                    loggedWindowBlock[virtualKey] = true;
                    LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                        L"Gameplay window input blocked vk=%d.",
                        virtualKey);
                }
                return 0;
            }
        }
    }
    return g_originalGameplayWindowProc != nullptr
        ? CallWindowProcW(
            g_originalGameplayWindowProc,
            window,
            message,
            wParam,
            lParam)
        : DefWindowProcW(window, message, wParam, lParam);
}

void GameplayInput_EnsureWindowSubclass()
{
    const HWND gameWindow = FindGameWindow();
    if (gameWindow == nullptr ||
        gameWindow == g_gameplayWindowSubclassTarget)
    {
        return;
    }
    if (g_gameplayWindowSubclassTarget != nullptr &&
        IsWindow(g_gameplayWindowSubclassTarget) &&
        reinterpret_cast<WNDPROC>(
            GetWindowLongPtrW(
                g_gameplayWindowSubclassTarget,
                GWLP_WNDPROC)) == GameplayWindowProc)
    {
        SetWindowLongPtrW(
            g_gameplayWindowSubclassTarget,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(g_originalGameplayWindowProc));
    }
    const WNDPROC previous = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(
            gameWindow,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(GameplayWindowProc)));
    if (previous == nullptr)
    {
        LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Unable to subclass the gameplay window input path.");
        return;
    }
    g_originalGameplayWindowProc = previous;
    g_gameplayWindowSubclassTarget = gameWindow;
    LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
        L"Gameplay window input subclass installed on hwnd=%p.",
        gameWindow);
}

LRESULT CALLBACK ExtendedMouseHook(
    int code,
    WPARAM wParam,
    LPARAM lParam)
{
    if (code == HC_ACTION)
    {
        const auto* mouse =
            reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
        if (mouse != nullptr &&
            ModGuideUi_ProcessMouseMessage(
                static_cast<UINT>(wParam),
                mouse->pt.x,
                mouse->pt.y))
        {
            return 1;
        }
        if (g_extendedOriginalTutorialActive.load())
        {
            g_extendedManualPointerDown = false;
            g_extendedManualPointerConsumed = false;
            return CallNextHookEx(
                g_extendedMouseHook,
                code,
                wParam,
                lParam);
        }
    }
    if (code == HC_ACTION &&
        g_gameplayWorldMapOpen.load())
    {
        const HWND gameWindow = FindGameWindow();
        const auto* mouse =
            reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            mouse != nullptr)
        {
            const int wheelDelta = static_cast<short>(
                HIWORD(mouse->mouseData));
            if (GameplayWorldMap_ProcessMouseMessage(
                    static_cast<UINT>(wParam),
                    mouse->pt.x,
                    mouse->pt.y,
                    wheelDelta))
            {
                return 1;
            }
            return CallNextHookEx(
                g_extendedMouseHook,
                code,
                wParam,
                lParam);
        }
    }
    if (code == HC_ACTION &&
        g_extendedPoseMenuVisible.load())
    {
        const auto* mouse =
            reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
        if (mouse != nullptr)
        {
            if (wParam == WM_MOUSEMOVE)
            {
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                return CallNextHookEx(
                    g_extendedMouseHook,
                    code,
                    wParam,
                    lParam);
            }
            if (wParam == WM_LBUTTONDOWN ||
                wParam == WM_LBUTTONDBLCLK)
            {
                g_extendedManualPointerPressX = mouse->pt.x;
                g_extendedManualPointerPressY = mouse->pt.y;
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                g_extendedManualPointerDown = true;
                g_extendedPoseMenuClickRequested = true;
                return 1;
            }
            if (wParam == WM_LBUTTONUP)
            {
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                g_extendedManualPointerDown = false;
                return 1;
            }
            return 1;
        }
    }
    if (code == HC_ACTION &&
        NpcDialogue_IsActive())
    {
        const HWND gameWindow = FindGameWindow();
        const auto* mouse =
            reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            mouse != nullptr)
        {
            if (wParam == WM_MOUSEMOVE)
            {
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                return CallNextHookEx(
                    g_extendedMouseHook,
                    code,
                    wParam,
                    lParam);
            }
            if (wParam == WM_LBUTTONDOWN ||
                wParam == WM_LBUTTONDBLCLK)
            {
                g_extendedManualPointerPressX = mouse->pt.x;
                g_extendedManualPointerPressY = mouse->pt.y;
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                g_extendedManualPointerDown = true;
                ++g_extendedManualPointerPressSerial;
                LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                    L"NPC dialogue mouse down captured at (%ld,%ld).",
                    mouse->pt.x,
                    mouse->pt.y);
                return 1;
            }
            if (wParam == WM_LBUTTONUP)
            {
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                g_extendedManualPointerDown = false;
                return 1;
            }
            return 1;
        }
    }
    if (code == HC_ACTION &&
        g_extendedOriginalInteractionMenuActive.load())
    {
        g_extendedManualPointerDown = false;
        g_extendedManualPointerConsumed = false;
        if (wParam == WM_LBUTTONDOWN)
        {
            g_extendedOriginalInteractionMenuClickRequested = true;
            LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Original interaction menu received mouse down; mod mouse hook passed it through.");
        }
        else if (wParam == WM_LBUTTONUP)
        {
            LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Original interaction menu received mouse up; mod mouse hook passed it through.");
        }
        return CallNextHookEx(g_extendedMouseHook, code, wParam, lParam);
    }
    if (code == HC_ACTION &&
        (g_extendedOriginalHandcuffsPanelActive.load() ||
         DildoInventory_IsPanelActive()))
    {
        const auto* mouse =
            reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_MOUSEMOVE && mouse != nullptr)
        {
            g_extendedManualPointerCurrentX = mouse->pt.x;
            g_extendedManualPointerCurrentY = mouse->pt.y;
            g_extendedManualPointerPositionValid = true;
            return CallNextHookEx(
                g_extendedMouseHook,
                code,
                wParam,
                lParam);
        }
        if ((wParam == WM_LBUTTONDOWN ||
             wParam == WM_LBUTTONDBLCLK) &&
            mouse != nullptr)
        {
            g_extendedManualPointerPressX = mouse->pt.x;
            g_extendedManualPointerPressY = mouse->pt.y;
            g_extendedManualPointerCurrentX = mouse->pt.x;
            g_extendedManualPointerCurrentY = mouse->pt.y;
            g_extendedManualPointerPositionValid = true;
            g_extendedManualPointerDown = true;
            g_extendedOriginalHandcuffsPanelClickRequested = true;
            return 1;
        }
        if (wParam == WM_LBUTTONUP)
        {
            g_extendedManualPointerDown = false;
            return 1;
        }
        return CallNextHookEx(
            g_extendedMouseHook,
            code,
            wParam,
            lParam);
    }
    if (code == HC_ACTION &&
        CheckpointSystem_IsMapEditorUiOpen())
    {
        const HWND gameWindow = FindGameWindow();
        const auto* mouse =
            reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            mouse != nullptr)
        {
            if (wParam == WM_MOUSEMOVE)
            {
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
            }
            else if (wParam == WM_LBUTTONDOWN ||
                     wParam == WM_LBUTTONDBLCLK)
            {
                g_extendedManualPointerPressX = mouse->pt.x;
                g_extendedManualPointerPressY = mouse->pt.y;
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                g_extendedManualPointerDown = true;
                g_extendedManualPointerConsumed = false;
                g_extendedManualPointerInventoryEligible = false;
                ++g_extendedManualPointerPressSerial;
            }
            else if (wParam == WM_LBUTTONUP)
            {
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                g_extendedManualPointerDown = false;
                ++g_extendedManualPointerReleaseSerial;
            }
        }
        return CallNextHookEx(
            g_extendedMouseHook,
            code,
            wParam,
            lParam);
    }
    if (code == HC_ACTION &&
        (g_gameplayInventoryOpen.load() ||
         CheckpointSystem_IsTaskUiOpen()))
    {
        const HWND gameWindow = FindGameWindow();
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow)
        {
            if (wParam == WM_LBUTTONDOWN)
            {
                const auto* mouse =
                    reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
                if (mouse != nullptr)
                {
                    g_extendedManualPointerPressX = mouse->pt.x;
                    g_extendedManualPointerPressY = mouse->pt.y;
                    g_extendedManualPointerCurrentX = mouse->pt.x;
                    g_extendedManualPointerCurrentY = mouse->pt.y;
                    g_extendedManualPointerPositionValid = true;
                    g_extendedManualPointerConsumed = false;
                    g_extendedManualPointerInventoryEligible = true;
                    g_extendedManualPointerDown = true;
                    ++g_extendedManualPointerPressSerial;
                }
                return 1;
            }
            if (wParam == WM_MOUSEMOVE)
            {
                const auto* mouse =
                    reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
                if (mouse != nullptr)
                {
                    g_extendedManualPointerCurrentX = mouse->pt.x;
                    g_extendedManualPointerCurrentY = mouse->pt.y;
                    g_extendedManualPointerPositionValid = true;
                }
            }
            if (wParam == WM_RBUTTONDOWN)
            {
                const auto* mouse =
                    reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
                if (mouse != nullptr)
                {
                    g_extendedManualPointerPressX = mouse->pt.x;
                    g_extendedManualPointerPressY = mouse->pt.y;
                    g_extendedManualPointerCurrentX = mouse->pt.x;
                    g_extendedManualPointerCurrentY = mouse->pt.y;
                    g_extendedManualPointerPositionValid = true;
                    ++g_extendedManualRightPressSerial;
                }
                return 1;
            }
            if (wParam == WM_LBUTTONUP ||
                wParam == WM_RBUTTONUP)
            {
                if (wParam == WM_LBUTTONUP)
                {
                    const auto* mouse =
                        reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
                    if (mouse != nullptr)
                    {
                        g_extendedManualPointerCurrentX = mouse->pt.x;
                        g_extendedManualPointerCurrentY = mouse->pt.y;
                        g_extendedManualPointerPositionValid = true;
                    }
                    g_extendedManualPointerDown = false;
                    ++g_extendedManualPointerReleaseSerial;
                }
                return 1;
            }
        }
    }
    if (code == HC_ACTION &&
        g_extendedModeState.load() == ExtendedMode_Active)
    {
        if (g_extendedOriginalSystemMenuOpen.load())
        {
            g_extendedManualPointerDown = false;
            g_extendedManualPointerConsumed = false;
            return CallNextHookEx(g_extendedMouseHook, code, wParam, lParam);
        }
        const HWND gameWindow = FindGameWindow();
        if (wParam == WM_MOUSEWHEEL &&
            gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            g_inventoryActiveRoute == kInventoryRouteCabinet &&
            ExhibitionUi_IsPickerOpen())
        {
            const auto* mouse =
                reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
            if (mouse != nullptr)
            {
                ExhibitionUi_QueuePickerWheel(
                    static_cast<short>(HIWORD(mouse->mouseData)));
            }
            return 1;
        }
        if (wParam == WM_MOUSEWHEEL &&
            gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            g_inventoryActiveRoute == kInventoryRouteEquipment &&
            !g_extendedActionListOpen)
        {
            const auto* mouse =
                reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
            if (mouse != nullptr)
            {
                InventoryUi_QueueStashWheel(
                    static_cast<short>(HIWORD(mouse->mouseData)));
            }
            return 1;
        }
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            wParam == WM_RBUTTONDOWN)
        {
            const auto* mouse =
                reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
            if (mouse != nullptr)
            {
                g_extendedManualPointerPressX = mouse->pt.x;
                g_extendedManualPointerPressY = mouse->pt.y;
                g_extendedManualPointerCurrentX = mouse->pt.x;
                g_extendedManualPointerCurrentY = mouse->pt.y;
                g_extendedManualPointerPositionValid = true;
                ++g_extendedManualRightPressSerial;
            }
            return 1;
        }
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            wParam == WM_RBUTTONUP)
        {
            return 1;
        }
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            wParam == WM_LBUTTONDOWN)
        {
            const auto* mouse =
                reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
            if (mouse != nullptr)
            {
                g_extendedManualPointerPressX = mouse->pt.x;
                g_extendedManualPointerPressY = mouse->pt.y;
                g_extendedManualPointerConsumed = false;
                g_extendedManualPointerInventoryEligible = false;
                g_extendedManualPointerDown = true;
                ++g_extendedManualPointerPressSerial;
            }
            return 1;
        }
        if (gameWindow != nullptr &&
            GetForegroundWindow() == gameWindow &&
            (wParam == WM_LBUTTONUP ||
             wParam == WM_LBUTTONDBLCLK))
        {
            g_extendedManualPointerDown = false;
            g_extendedManualPointerConsumed = false;
            return 1;
        }
    }
    return CallNextHookEx(g_extendedMouseHook, code, wParam, lParam);
}

LRESULT CALLBACK GameplayKeyboardHook(
    int code,
    WPARAM wParam,
    LPARAM lParam)
{
    if (code == HC_ACTION && lParam != 0)
    {
        const auto* keyState =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyState != nullptr && keyState->vkCode == 'Q')
        {
            const bool keyDown =
                wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
            const int modeState = g_extendedModeState.load();
            const bool hotkeyAvailable =
                (modeState == ExtendedMode_Active ||
                 modeState == ExtendedMode_Gameplay) &&
                !g_extendedOriginalSystemMenuVisible.load() &&
                !g_extendedOriginalInteractionMenuActive.load() &&
                !g_extendedOriginalHandcuffsPanelActive.load() &&
                !g_gameplayInventoryOpen.load() &&
                !CheckpointSystem_IsTaskUiOpen() &&
                !CheckpointSystem_IsMapEditorUiOpen() &&
                !CheckpointSystem_IsCoatToggleLocked();
            const bool wasDown =
                g_gameplaySpecialCoatPhysicalDown.exchange(keyDown);
            if (keyDown && !wasDown && hotkeyAvailable)
            {
                g_extendedToggleSpecialCoatRequested = true;
                LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                    L"Escape special coat hook key queued: state=%d menu=%d inventory=%d taskUi=%d.",
                    modeState,
                    g_extendedOriginalSystemMenuVisible.load() ? 1 : 0,
                    g_gameplayInventoryOpen.load() ? 1 : 0,
                    CheckpointSystem_IsTaskUiOpen() ? 1 : 0);
            }
        }
    }
    if (code == HC_ACTION)
    {
        const auto* diagnosticKeyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (diagnosticKeyboard != nullptr &&
            (diagnosticKeyboard->vkCode == 'J' ||
             diagnosticKeyboard->vkCode == 'K' ||
             diagnosticKeyboard->vkCode == 'L' ||
             diagnosticKeyboard->vkCode == VK_TAB))
        {
            static bool diagnosticLogged[256]{};
            const auto key = diagnosticKeyboard->vkCode;
            if (key < 256 && !diagnosticLogged[key])
            {
                diagnosticLogged[key] = true;
                const HWND diagnosticForeground = GetForegroundWindow();
                DWORD diagnosticProcessId = 0;
                if (diagnosticForeground != nullptr)
                {
                    GetWindowThreadProcessId(
                        diagnosticForeground,
                        &diagnosticProcessId);
                }
                LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                    L"Gameplay filter saw key vk=%lu state=%d foregroundPid=%lu currentPid=%lu menu=%d.",
                    static_cast<unsigned long>(key),
                    g_extendedModeState.load(),
                    static_cast<unsigned long>(diagnosticProcessId),
                    static_cast<unsigned long>(GetCurrentProcessId()),
                    g_extendedOriginalSystemMenuVisible.load() ? 1 : 0);
            }
        }
    }
    if (code == HC_ACTION &&
        g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            (keyboard->vkCode == 'W' || keyboard->vkCode == VK_ESCAPE))
        {
            static bool loggedDiagnostic = false;
            if (!loggedDiagnostic)
            {
                loggedDiagnostic = true;
                LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                    L"Gameplay keyboard diagnostic: message=%llu vk=%lu gameWindow=%p foreground=%p menu=%d.",
                    static_cast<unsigned long long>(wParam),
                    keyboard->vkCode,
                    g_gameWindow,
                    GetForegroundWindow(),
                    g_extendedOriginalSystemMenuVisible.load() ? 1 : 0);
            }
        }
    }
    const HWND foregroundWindow = GetForegroundWindow();
    DWORD foregroundProcessId = 0;
    if (foregroundWindow != nullptr)
    {
        GetWindowThreadProcessId(
            foregroundWindow,
            &foregroundProcessId);
    }
    const bool gameplayForeground =
        foregroundWindow != nullptr &&
        foregroundProcessId == GetCurrentProcessId();
    if (code == HC_ACTION &&
        gameplayForeground &&
        g_extendedOriginalTutorialActive.load())
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            keyboard->vkCode == VK_ESCAPE)
        {
            g_extendedOpenSystemMenuRequested = false;
            g_extendedEscapeDismissBlockedUntil.store(
                GetTickCount64() + 1000);
        }
        return CallNextHookEx(
            g_gameplayKeyboardHook,
            code,
            wParam,
            lParam);
    }
    if (code == HC_ACTION &&
        gameplayForeground &&
        (g_extendedModeState.load() == ExtendedMode_Active ||
         g_extendedModeState.load() == ExtendedMode_Gameplay))
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            !g_extendedOriginalSystemMenuVisible.load())
        {
            const bool keyDown =
                wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
            const bool keyUp =
                wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
            const bool systemShortcut =
                (GameplayInput_ReadRawAsyncKeyState(VK_MENU) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_LWIN) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_RWIN) & 0x8000) != 0;
            if (!systemShortcut &&
                ModGuideUi_ProcessPhysicalKey(
                    keyboard->vkCode,
                    keyDown,
                    L"the low-level gameplay keyboard hook"))
            {
                return 1;
            }
            if (!systemShortcut &&
                GameplayInput_ProcessEscapeDismissKey(
                    keyboard->vkCode,
                    keyDown,
                    keyUp,
                    L"the low-level gameplay keyboard hook"))
            {
                return 1;
            }
            if (!systemShortcut &&
                ModGuideUi_IsOpen())
            {
                return 1;
            }
            if (!systemShortcut &&
                GameplayWorldMap_ProcessPhysicalKey(
                    keyboard->vkCode,
                    keyDown,
                    L"the low-level gameplay keyboard hook"))
            {
                return 1;
            }
            if (!systemShortcut &&
                g_gameplayWorldMapOpen.load())
            {
                return 1;
            }
            if (!systemShortcut &&
                CheckpointSystem_ProcessMapEditorPhysicalKey(
                    keyboard->vkCode,
                    keyDown,
                    L"the low-level gameplay keyboard hook"))
            {
                return 1;
            }
            if (CheckpointSystem_IsMapEditorUiOpen() &&
                !systemShortcut)
            {
                return CheckpointSystem_IsMapEditorTextInputFocused()
                    ? CallNextHookEx(
                        g_gameplayKeyboardHook,
                        code,
                        wParam,
                        lParam)
                    : 1;
            }
        }
    }
    if (code == HC_ACTION &&
        g_extendedOriginalHandcuffsPanelActive.load() &&
        gameplayForeground)
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            !g_extendedOriginalSystemMenuVisible.load())
        {
            const bool keyDown =
                wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
            const bool systemShortcut =
                (GameplayInput_ReadRawAsyncKeyState(VK_MENU) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_LWIN) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_RWIN) & 0x8000) != 0;
            if (systemShortcut ||
                keyboard->vkCode == VK_ESCAPE)
            {
                return CallNextHookEx(
                    g_gameplayKeyboardHook,
                    code,
                    wParam,
                    lParam);
            }
            if (keyboard->vkCode >= '1' &&
                keyboard->vkCode <= '3')
            {
                g_gameplayAdultQuickSlotPhysicalDown[
                    keyboard->vkCode - '1'] = keyDown;
            }
            return 1;
        }
    }
    if (code == HC_ACTION &&
        g_gameplayInventoryOpen.load())
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            !g_extendedOriginalSystemMenuVisible.load())
        {
            const bool keyDown =
                wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
            const bool systemShortcut =
                (GameplayInput_ReadRawAsyncKeyState(VK_MENU) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_LWIN) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_RWIN) & 0x8000) != 0;
            if (systemShortcut)
            {
                return CallNextHookEx(
                    g_gameplayKeyboardHook,
                    code,
                    wParam,
                    lParam);
            }
            if (keyboard->vkCode == VK_ESCAPE &&
                keyDown &&
                CheckpointSystem_IsRewardInventoryOpen())
            {
                CheckpointSystem_RequestRewardInventoryClose();
            }
            if (keyboard->vkCode == VK_TAB)
            {
                return CallNextHookEx(
                    g_gameplayKeyboardHook,
                    code,
                    wParam,
                    lParam);
            }
            if (!gameplayForeground)
            {
                return CallNextHookEx(
                    g_gameplayKeyboardHook,
                    code,
                    wParam,
                    lParam);
            }
            // The inventory owns the keyboard while it is visible. This is
            // deliberately below the system-shortcut check so Alt+F4 remains
            // available, while J/K/L and every other gameplay key is consumed.
            return 1;
        }
    }
    if (code == HC_ACTION &&
        g_extendedPoseMenuVisible.load() &&
        gameplayForeground)
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            !g_extendedOriginalSystemMenuVisible.load())
        {
            const bool keyUp =
                wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
            const bool systemShortcut =
                (GameplayInput_ReadRawAsyncKeyState(VK_MENU) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_LWIN) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_RWIN) & 0x8000) != 0;
            if (systemShortcut)
            {
                return CallNextHookEx(
                    g_gameplayKeyboardHook,
                    code,
                    wParam,
                    lParam);
            }
            if (keyboard->vkCode == 'E' && keyUp)
            {
                g_extendedPoseMenuHeld = false;
            }
            // Match the Tab inventory: the custom panel owns every gameplay
            // key until E and its mouse selection complete.
            return 1;
        }
    }
    if (code == HC_ACTION &&
        CheckpointSystem_IsTaskUiOpen() &&
        gameplayForeground)
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            !g_extendedOriginalSystemMenuVisible.load())
        {
            const bool systemShortcut =
                (GameplayInput_ReadRawAsyncKeyState(VK_MENU) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_LWIN) & 0x8000) != 0 ||
                (GameplayInput_ReadRawAsyncKeyState(VK_RWIN) & 0x8000) != 0;
            if (systemShortcut)
            {
                return CallNextHookEx(
                    g_gameplayKeyboardHook,
                    code,
                    wParam,
                    lParam);
            }
            return 1;
        }
    }
    const int gameplayInputState = g_extendedModeState.load();
    if (code == HC_ACTION &&
        (gameplayInputState == ExtendedMode_Active ||
         gameplayInputState == ExtendedMode_Gameplay) &&
        gameplayForeground)
    {
        const auto* keyboard =
            reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (keyboard != nullptr &&
            !g_extendedOriginalSystemMenuVisible.load())
        {
            const bool keyDown =
                wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
            const bool keyUp =
                wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
            if (keyboard->vkCode == 'E' && keyUp)
            {
                g_extendedPoseMenuHeld = false;
            }
            const bool systemShortcut =
                (GetAsyncKeyState(VK_MENU) & 0x8000) != 0 ||
                (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ||
                (GetAsyncKeyState(VK_LWIN) & 0x8000) != 0 ||
                (GetAsyncKeyState(VK_RWIN) & 0x8000) != 0;
            if (systemShortcut)
            {
                return CallNextHookEx(
                    g_gameplayKeyboardHook,
                    code,
                    wParam,
                    lParam);
            }
            if (GameplayInput_ProcessEyeMaskVisionAdjustmentKey(
                    keyboard->vkCode,
                    keyDown,
                    keyUp,
                    L"the low-level gameplay keyboard hook"))
            {
                return 1;
            }
            switch (keyboard->vkCode)
            {
            case 'E':
            {
                if (!g_extendedOriginalInteractionMenuActive.load() &&
                    !g_extendedOriginalHandcuffsPanelActive.load() &&
                    !g_gameplayInventoryOpen.load() &&
                    !CheckpointSystem_IsTaskUiOpen())
                {
                    g_extendedPoseMenuHeld = keyDown && !keyUp;
                    return 1;
                }
                break;
            }
            case 'Q':
            {
                // Q is reserved for entering/leaving the special coat state.
                return 1;
            }
            case '1':
            case '2':
            case '3':
            {
                const int quickSlot = keyboard->vkCode - '1';
                const bool wasDown =
                    g_gameplayAdultQuickSlotPhysicalDown[
                        quickSlot].exchange(keyDown);
                if (keyDown &&
                    !wasDown &&
                    !g_extendedOriginalInteractionMenuActive.load() &&
                    !g_extendedOriginalHandcuffsPanelActive.load())
                {
                    GameplayInput_QueueAdultQuickSlot(
                        quickSlot,
                        L"the gameplay keyboard hook");
                }
                return 1;
            }
            case 'T':
            {
                static bool talkKeyDown = false;
                NpcDialogue_ReserveTalkKey();
                if (keyDown && !talkKeyDown)
                {
                    talkKeyDown = true;
                    if (NpcDialogue_HandleTalkKeyDown())
                    {
                        LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"NPC dialogue consumed the T key.");
                    }
                }
                else if (keyUp)
                {
                    talkKeyDown = false;
                }
                return 1;
            }
            case VK_TAB:
            {
                break;
            }
            case 'Y':
            {
                if (keyDown &&
                    CheckpointSystem_HasAcceptedTask() &&
                    !g_checkpointAbandonActionDown.exchange(true))
                {
                    g_checkpointAbandonTaskRequested = true;
                    LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Checkpoint task abandon key queued.");
                }
                else if (keyUp)
                {
                    g_checkpointAbandonActionDown = false;
                }
                return 1;
            }
            case 'W':
                if (keyDown)
                {
                    g_extendedForwardKeyDown = true;
                    g_extendedForwardKeyPulse = true;
                }
                else if (keyUp)
                {
                    g_extendedForwardKeyDown = false;
                }
                break;
            case 'S':
                if (keyDown)
                {
                    g_extendedBackwardKeyDown = true;
                    g_extendedBackwardKeyPulse = true;
                }
                else if (keyUp)
                {
                    g_extendedBackwardKeyDown = false;
                }
                break;
            case 'A':
                if (keyDown)
                {
                    g_extendedLeftKeyDown = true;
                    g_extendedLeftKeyPulse = true;
                }
                else if (keyUp)
                {
                    g_extendedLeftKeyDown = false;
                }
                break;
            case 'D':
                if (keyDown)
                {
                    g_extendedRightKeyDown = true;
                    g_extendedRightKeyPulse = true;
                }
                else if (keyUp)
                {
                    g_extendedRightKeyDown = false;
                }
                break;
            case VK_ESCAPE:
                if (CheckpointSystem_IsFailureUiActive())
                {
                    if (keyDown)
                    {
                        CheckpointSystem_RequestFailureReturn(
                            L"Escape during MissionFailedPanel");
                    }
                    return 1;
                }
                if (NpcDialogue_IsActive())
                {
                    if (keyDown)
                    {
                        NpcDialogue_RequestClose();
                        g_extendedOpenSystemMenuRequested = false;
                        LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"NPC dialogue consumed Escape.");
                    }
                    return 1;
                }
                // The original interaction panel owns Esc while it is open.
                // Let the original UI close it without opening the system menu.
                if (g_extendedOriginalInteractionMenuActive.load() ||
                    g_extendedOriginalHandcuffsPanelActive.load() ||
                    DildoInventory_IsPanelActive())
                {
                    break;
                }
                if (keyDown)
                {
                    GameplayInput_RequestSystemMenu(
                        L"the low-level gameplay keyboard hook");
                    return 1;
                }
                break;
            case 'F':
            {
                static bool interactKeyDown = false;
                bool handled = false;
                if (keyDown && !interactKeyDown)
                {
                    interactKeyDown = true;
                    const bool nearbyCheckpoint =
                        CheckpointSystem_HasNearbyInteraction();
                    LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                        L"Gameplay F key down: nearbyCheckpoint=%d.",
                        nearbyCheckpoint ? 1 : 0);
                    if (!g_extendedOriginalInteractionMenuActive.load() &&
                        !g_extendedOriginalHandcuffsPanelActive.load() &&
                        !g_gameplayInventoryOpen.load() &&
                        !CheckpointSystem_IsTaskUiOpen() &&
                        nearbyCheckpoint)
                    {
                        g_checkpointInteractActionDown = true;
                        g_checkpointInteractionKeyConsumed = true;
                        g_checkpointSearchRequested = true;
                        LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Checkpoint interaction key consumed for a nearby Escape point.");
                        handled = true;
                    }
                }
                else if (keyDown &&
                         g_checkpointInteractionKeyConsumed.load())
                {
                    handled = true;
                }
                else if (keyUp)
                {
                    interactKeyDown = false;
                    g_checkpointInteractActionDown = false;
                    handled =
                        g_checkpointInteractionKeyConsumed.exchange(false);
                }
                if (handled)
                {
                    return 1;
                }
                break;
            }
            case 'R':
            case 'C':
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_LCONTROL:
            case VK_RCONTROL:
            case VK_LMENU:
            case VK_RMENU:
            case VK_LWIN:
            case VK_RWIN:
                break;
            default:
            {
                static bool blockedKeyLogged[256]{};
                const auto key = keyboard->vkCode;
                if (keyDown && key < 256 && !blockedKeyLogged[key])
                {
                    blockedKeyLogged[key] = true;
                    LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                        L"Escape gameplay input blocked original key vk=%lu.",
                        static_cast<unsigned long>(key));
                }
                return 1;
            }
            }
        }
    }
    // Esc belongs to the original gameplay input path while the player is in
    // the world. Once its panel is open, the shipped SystemMenuView owns the
    // close path and all menu button handling.
    return CallNextHookEx(g_gameplayKeyboardHook, code, wParam, lParam);
}

void UpdateMenuText()
{
    if (g_statusLabel != nullptr)
    {
        const auto status = GetStatus();
        SetWindowTextW(g_statusLabel, status.c_str());
    }
    if (g_infoLabel != nullptr)
    {
        const auto info = GetLocalIpv4Hint() + L"\nDirect TCP test, no authentication. Use only with a trusted host.";
        SetWindowTextW(g_infoLabel, info.c_str());
    }
}

LRESULT CALLBACK MenuWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        g_uiFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        const auto create = [&](const wchar_t* type, const wchar_t* text, DWORD style, int x, int y, int width, int height, int id)
        {
            HWND control = CreateWindowExW(
                0, type, text, WS_CHILD | WS_VISIBLE | style,
                x, y, width, height, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g_instance, nullptr);
            if (control != nullptr)
            {
                SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(g_uiFont), TRUE);
            }
            return control;
        };

        create(L"STATIC", L"Escape from Manaka Native Test", SS_LEFT, 20, 18, 430, 24, 0);
        create(L"BUTTON", L"CREATE ROOM (HOST)", BS_PUSHBUTTON, 20, 54, 205, 38, kHostButtonId);
        create(L"EDIT", L"127.0.0.1", WS_BORDER | ES_AUTOHSCROLL, 20, 112, 270, 28, kAddressEditId);
        create(L"BUTTON", L"JOIN ROOM (CLIENT)", BS_PUSHBUTTON, 300, 108, 160, 36, kJoinButtonId);
        g_statusLabel = create(L"STATIC", L"Idle", SS_LEFT, 20, 156, 430, 44, kStatusLabelId);
        g_infoLabel = create(L"STATIC", L"", SS_LEFT, 20, 208, 430, 50, kInfoLabelId);
        create(L"BUTTON", L"STOP / LEAVE", BS_PUSHBUTTON, 20, 276, 210, 34, kStopButtonId);
        create(L"BUTTON", L"CLOSE", BS_PUSHBUTTON, 340, 276, 120, 34, kCloseButtonId);
        g_addressEdit = GetDlgItem(window, kAddressEditId);
        SetTimer(window, kMenuPollTimer, 400, nullptr);
        UpdateMenuText();
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case kHostButtonId:
            StartHost();
            break;
        case kJoinButtonId:
        {
            wchar_t address[256]{};
            GetWindowTextW(g_addressEdit, address, 256);
            StartClient(address);
            break;
        }
        case kStopButtonId:
            StopNetwork();
            SetStatus(L"Stopped.");
            break;
        case kCloseButtonId:
            ShowWindow(window, SW_HIDE);
            break;
        default:
            break;
        }
        UpdateMenuText();
        return 0;
    case WM_TIMER:
        UpdateMenuText();
        return 0;
    case kRefreshMenuTextMessage:
        UpdateMenuText();
        return 0;
    case WM_CLOSE:
        ShowWindow(window, SW_HIDE);
        return 0;
    case WM_DESTROY:
        KillTimer(window, kMenuPollTimer);
        if (window == g_menuWindow)
        {
            g_menuWindow = nullptr;
            g_statusLabel = nullptr;
            g_infoLabel = nullptr;
            g_addressEdit = nullptr;
        }
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
    return 0;
}

void OpenMenu()
{
    if (g_menuWindow != nullptr && IsWindow(g_menuWindow))
    {
        ShowWindow(g_menuWindow, SW_SHOW);
        SetForegroundWindow(g_menuWindow);
        return;
    }

    RECT gameRect{};
    if (g_gameWindow != nullptr && !GetWindowRect(g_gameWindow, &gameRect))
    {
        gameRect = RECT{0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    }
    const int width = 490;
    const int height = 370;
    const int x = gameRect.left + ((gameRect.right - gameRect.left) - width) / 2;
    const int y = gameRect.top + ((gameRect.bottom - gameRect.top) - height) / 2;

    g_menuWindow = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"ManakaMultiplayerMenu",
        L"Escape from Manaka",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, width, height,
        nullptr, nullptr, g_instance, nullptr);

    if (g_menuWindow != nullptr)
    {
        ShowWindow(g_menuWindow, SW_SHOW);
        UpdateWindow(g_menuWindow);
    }
}

LRESULT CALLBACK OverlayWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT paint{};
        const auto device = BeginPaint(window, &paint);
        RECT rectangle{};
        GetClientRect(window, &rectangle);
        const auto brush = CreateSolidBrush(RGB(24, 24, 24));
        FillRect(device, &rectangle, brush);
        DeleteObject(brush);
        FrameRect(device, &rectangle, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
        SetBkMode(device, TRANSPARENT);
        SetTextColor(device, RGB(255, 255, 255));
        wchar_t text[] = L"MULTIPLAYER";
        DrawTextW(device, text, -1, &rectangle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_SETCURSOR:
        SetCursor(LoadCursorW(nullptr, IDC_HAND));
        return TRUE;
    case WM_LBUTTONUP:
        OpenMenu();
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}

void UpdateOverlay()
{
    RefreshTitleState();
    if (g_overlayWindow != nullptr && IsWindow(g_overlayWindow))
    {
        ShowWindow(g_overlayWindow, SW_HIDE);
    }
}

LRESULT CALLBACK TimerWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (UpdateUi_HandleRuntimeMessage(message, wParam, lParam))
    {
        return 0;
    }
    if (message == WM_TIMER && wParam == kOverlayPollTimer)
    {
        UpdateOverlay();
        return 0;
    }
    if (message == kTestUiOpenMessage)
    {
        g_pendingUiCommand = 1;
        return 0;
    }
    if (message == kTestUiCloseMessage)
    {
        g_pendingUiCommand = 2;
        return 0;
    }
    if (message == kTestUiTabMessage)
    {
        g_pendingUiCommand = wParam == 1 ? 11 : 10;
        return 0;
    }
    if (message == kTestUiLobbyActionMessage)
    {
        g_pendingUiCommand = wParam == 1 ? 21 : 20;
        return 0;
    }
    if (message == kTestUiPlayerProbeMessage)
    {
        g_pendingUiCommand = 30;
        return 0;
    }
    if (message == kTestEfmActionMessage)
    {
        g_efmPendingAction = static_cast<int>(wParam);
        g_extendedPendingAction = static_cast<int>(wParam);
        return 0;
    }
    if (message == kTestEfmSystemMenuMessage)
    {
        if (g_extendedModeState.load() == ExtendedMode_Gameplay)
        {
            g_extendedOpenSystemMenuRequested = true;
        }
        return 0;
    }
    if (message == kTestEfmMoveMessage)
    {
        if (g_extendedModeState.load() == ExtendedMode_Gameplay)
        {
            switch (wParam)
            {
            case 1:
                g_extendedBackwardKeyDown = true;
                g_extendedBackwardKeyPulse = true;
                break;
            case 2:
                g_extendedLeftKeyDown = true;
                g_extendedLeftKeyPulse = true;
                break;
            case 3:
                g_extendedRightKeyDown = true;
                g_extendedRightKeyPulse = true;
                break;
            case 9:
                g_extendedForwardKeyDown = false;
                g_extendedBackwardKeyDown = false;
                g_extendedLeftKeyDown = false;
                g_extendedRightKeyDown = false;
                g_extendedForwardKeyPulse = false;
                g_extendedBackwardKeyPulse = false;
                g_extendedLeftKeyPulse = false;
                g_extendedRightKeyPulse = false;
                break;
            default:
                g_extendedForwardKeyDown = true;
                g_extendedForwardKeyPulse = true;
                break;
            }
            static bool loggedMoveRequest = false;
            if (!loggedMoveRequest)
            {
                loggedMoveRequest = true;
                LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Native test move request queued for gameplay.");
            }
        }
        return 0;
    }
    if (message == kTestEfmExitMessage)
    {
        if (g_extendedModeState.load() == ExtendedMode_Gameplay)
        {
            g_extendedExitRequested = true;
        }
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

void RuntimeThread()
{
    WSADATA winsockData{};
    WSAStartup(MAKEWORD(2, 2), &winsockData);
    Log_StartSession();
    UpdateUi_PromoteStagedUpdateHost();
    UpdateUi_ReconcileInstalledVersion();
    LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Escape from Manaka version: %s.", MANAKA_MOD_VERSION_STRING);
    SetStatus(L"Injected. Waiting for the Unity IL2CPP runtime...");

    WNDCLASSEXW timerClass{};
    timerClass.cbSize = sizeof(timerClass);
    timerClass.lpfnWndProc = TimerWindowProc;
    timerClass.hInstance = g_instance;
    timerClass.lpszClassName = L"ManakaMultiplayerTimer";
    RegisterClassExW(&timerClass);

    WNDCLASSEXW overlayClass{};
    overlayClass.cbSize = sizeof(overlayClass);
    overlayClass.lpfnWndProc = OverlayWindowProc;
    overlayClass.hInstance = g_instance;
    overlayClass.hCursor = LoadCursorW(nullptr, IDC_HAND);
    overlayClass.lpszClassName = L"ManakaMultiplayerOverlay";
    RegisterClassExW(&overlayClass);

    WNDCLASSEXW menuClass{};
    menuClass.cbSize = sizeof(menuClass);
    menuClass.lpfnWndProc = MenuWindowProc;
    menuClass.hInstance = g_instance;
    menuClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    menuClass.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<INT_PTR>(COLOR_WINDOW + 1));
    menuClass.lpszClassName = L"ManakaMultiplayerMenu";
    RegisterClassExW(&menuClass);

    g_messageWindow = CreateWindowExW(
        0, L"ManakaMultiplayerTimer", L"", 0, 0, 0, 0, 0,
        HWND_MESSAGE, nullptr, g_instance, nullptr);
    if (g_messageWindow == nullptr)
    {
        LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Unable to create the native message window.");
        return;
    }
    g_gameplayKeyboardHook = SetWindowsHookExW(
        WH_KEYBOARD_LL,
        GameplayKeyboardHook,
        g_instance,
        0);
    g_extendedMouseHook = SetWindowsHookExW(
        WH_MOUSE_LL,
        ExtendedMouseHook,
        g_instance,
        0);
    if (g_gameplayKeyboardHook == nullptr)
    {
        LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Unable to install the gameplay keyboard hook.");
    }
    if (g_extendedMouseHook == nullptr)
    {
        LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Unable to install the extended mouse hook.");
    }

    SetTimer(g_messageWindow, kOverlayPollTimer, 200, nullptr);
    GameplayInput_EnsureWindowSubclass();
    const auto started = std::chrono::steady_clock::now();
    bool fallbackLogged = false;
    bool actionListKeyDown = false;
    bool equipmentPreviewKeyDown = false;
    bool equipmentCatalogBatchKeyDown = false;
    bool escapeKeyDown = false;
    bool npcDialogueKeyDown = false;
    bool checkpointInteractPollKeyDown = false;
    bool checkpointAbandonPollKeyDown = false;

    MSG message{};
    while (!g_stopRequested)
    {
        if (g_runtimeShutdownRequested.load())
        {
            g_stopRequested = true;
            break;
        }
        GameplayInput_EnsureWindowSubclass();
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                g_stopRequested = true;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (g_runtimeShutdownRequested.load())
        {
            g_stopRequested = true;
            break;
        }
        UpdateUi_TickRuntimeThread();
        if (!g_titleApiReady &&
            std::chrono::steady_clock::now() - started > std::chrono::seconds(25) &&
            !fallbackLogged)
        {
            g_fallbackVisible = true;
            fallbackLogged = true;
            SetStatus(L"Title-scene detection unavailable. Showing the test button as a fallback.");
        }

        const bool actionKeyPressed =
            (GetAsyncKeyState(VK_OEM_3) & 0x8000) != 0;
        if (actionKeyPressed && !actionListKeyDown)
        {
            g_extendedToggleActionListRequested = true;
        }
        actionListKeyDown = actionKeyPressed;

        const bool equipmentPreviewKeyPressed =
            (GetAsyncKeyState(VK_F5) & 0x8000) != 0;
        if (equipmentPreviewKeyPressed && !equipmentPreviewKeyDown &&
            (g_extendedModeState.load() == ExtendedMode_PreparingGameplay ||
             g_extendedModeState.load() == ExtendedMode_Active))
        {
            g_extendedToggleEquipmentPreviewRequested = true;
        }
        equipmentPreviewKeyDown = equipmentPreviewKeyPressed;

        const bool equipmentCatalogBatchKeyPressed =
            (GetAsyncKeyState(VK_F6) & 0x8000) != 0;
        if (equipmentCatalogBatchKeyPressed &&
            !equipmentCatalogBatchKeyDown &&
            g_extendedModeState.load() == ExtendedMode_Active)
        {
            g_extendedToggleEquipmentCatalogBatchRequested = true;
        }
        equipmentCatalogBatchKeyDown = equipmentCatalogBatchKeyPressed;

        const bool guideKeyPressed =
            GetForegroundWindow() == g_gameWindow &&
            (GameplayInput_ReadRawAsyncKeyState(VK_F1) & 0x8000) != 0;
        ModGuideUi_ProcessPhysicalKey(
            VK_F1,
            guideKeyPressed,
            L"foreground polling fallback");

        const bool escapePressed =
            (GameplayInput_ReadRawAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        const bool escapeDownEdge =
            escapePressed && !escapeKeyDown;
        const bool escapeUpEdge =
            !escapePressed && escapeKeyDown;
        const bool escapeDismissConsumed =
            GameplayInput_ProcessEscapeDismissKey(
                VK_ESCAPE,
                escapeDownEdge,
                escapeUpEdge,
                L"runtime polling fallback");
        if (!escapeDismissConsumed && escapeDownEdge)
        {
            if (CheckpointSystem_IsFailureUiActive())
            {
                CheckpointSystem_RequestFailureReturn(
                    L"Escape polling during MissionFailedPanel");
            }
            else if (NpcDialogue_IsActive())
            {
                NpcDialogue_RequestClose();
            }
            else if (g_extendedModeState.load() == ExtendedMode_PreparingGameplay ||
                (g_extendedModeState.load() == ExtendedMode_Active &&
                 !g_extendedOriginalInteractionMenuActive.load()))
            {
                GameplayInput_RequestSystemMenu(
                    L"runtime polling fallback");
            }
        }
        escapeKeyDown = escapePressed;

        HWND foregroundWindow = GetForegroundWindow();
        DWORD foregroundProcessId = 0;
        if (foregroundWindow != nullptr)
        {
            GetWindowThreadProcessId(
                foregroundWindow,
                &foregroundProcessId);
        }
        const bool npcDialogueKeyPressed =
            foregroundProcessId == GetCurrentProcessId() &&
            (GameplayInput_ReadRawAsyncKeyState('T') & 0x8000) != 0;
        bool npcDialogueKeyConsumed = false;
        if (npcDialogueKeyPressed && !npcDialogueKeyDown)
        {
            NpcDialogue_ReserveTalkKey();
            if (NpcDialogue_IsActive())
            {
                NpcDialogue_RequestClose();
                npcDialogueKeyConsumed = true;
            }
            else if (NpcDialogue_IsTalkPromptVisible())
            {
                NpcDialogue_RequestTalk();
                npcDialogueKeyConsumed = true;
            }
        }
        npcDialogueKeyDown = npcDialogueKeyPressed;
        if (g_extendedModeState.load() == ExtendedMode_Gameplay)
        {
            const bool worldMapKeyPressed =
                foregroundProcessId == GetCurrentProcessId() &&
                (GameplayInput_ReadRawAsyncKeyState('M') & 0x8000) != 0;
            GameplayWorldMap_ProcessPhysicalKey(
                'M',
                worldMapKeyPressed,
                L"foreground polling fallback");
        }
        else
        {
            GameplayWorldMap_ResetPhysicalInput();
        }
        const bool poseMenuPressed =
            (GameplayInput_ReadRawAsyncKeyState('E') & 0x8000) != 0;
        const bool poseMenuAllowed =
            g_extendedModeState.load() == ExtendedMode_Gameplay &&
            !g_extendedOriginalTutorialActive.load() &&
            !ModGuideUi_IsOpen() &&
            !g_extendedOriginalSystemMenuVisible.load() &&
            !g_extendedOriginalInteractionMenuActive.load() &&
            !g_extendedOriginalHandcuffsPanelActive.load() &&
            !NpcDialogue_IsActive() &&
            !g_gameplayInventoryOpen.load() &&
            !CheckpointSystem_IsTaskUiOpen();
        g_extendedPoseMenuHeld =
            poseMenuPressed && poseMenuAllowed;
        const bool specialCoatPressed =
            (GetAsyncKeyState('Q') & 0x8000) != 0;
        const bool specialCoatWasDown =
            g_gameplaySpecialCoatPhysicalDown.exchange(
                specialCoatPressed);
        if (specialCoatPressed &&
            !specialCoatWasDown &&
            (g_extendedModeState.load() == ExtendedMode_Gameplay ||
             g_extendedModeState.load() == ExtendedMode_Active) &&
            !g_extendedOriginalSystemMenuVisible.load() &&
            !g_extendedOriginalInteractionMenuActive.load() &&
            !g_extendedOriginalHandcuffsPanelActive.load() &&
            !GameplayInput_IsCustomModalUiOpen() &&
            !CheckpointSystem_IsCoatToggleLocked())
        {
            g_extendedToggleSpecialCoatRequested = true;
            LogInfo(LogCategory_Lifecycle, L"Lifecycle", 
                L"Escape special coat poll key queued: state=%d inventory=%d taskUi=%d.",
                g_extendedModeState.load(),
                g_gameplayInventoryOpen.load() ? 1 : 0,
                CheckpointSystem_IsTaskUiOpen() ? 1 : 0);
        }

        const bool foregroundGameplay =
            foregroundProcessId == GetCurrentProcessId() &&
            (g_extendedModeState.load() == ExtendedMode_Active ||
             g_extendedModeState.load() == ExtendedMode_Gameplay) &&
            !g_extendedOriginalTutorialActive.load() &&
            !ModGuideUi_IsOpen() &&
            !g_extendedOriginalSystemMenuVisible.load() &&
            !g_extendedOriginalInteractionMenuActive.load() &&
            !g_extendedOriginalHandcuffsPanelActive.load() &&
            !NpcDialogue_IsActive() &&
            !g_gameplayInventoryOpen.load() &&
            !CheckpointSystem_IsTaskUiOpen();
        if (g_gameplayKeyboardHook == nullptr &&
            foregroundGameplay)
        {
            const bool upPressed =
                (GameplayInput_ReadRawAsyncKeyState(VK_UP) & 0x8000) != 0;
            const bool downPressed =
                (GameplayInput_ReadRawAsyncKeyState(VK_DOWN) & 0x8000) != 0;
            GameplayInput_ProcessEyeMaskVisionAdjustmentKey(
                VK_UP,
                upPressed,
                !upPressed,
                L"foreground polling fallback");
            GameplayInput_ProcessEyeMaskVisionAdjustmentKey(
                VK_DOWN,
                downPressed,
                !downPressed,
                L"foreground polling fallback");
        }
        for (int quickSlot = 0;
             quickSlot < kInventoryQuickSlotCount;
             ++quickSlot)
        {
            const bool pressed =
                (GameplayInput_ReadRawAsyncKeyState(
                    '1' + quickSlot) &
                 0x8000) != 0;
            const bool wasDown =
                g_gameplayAdultQuickSlotPhysicalDown[
                    quickSlot].exchange(pressed);
            if (pressed &&
                !wasDown &&
                foregroundGameplay)
            {
                GameplayInput_QueueAdultQuickSlot(
                    quickSlot,
                    L"foreground polling");
            }
        }
        const bool checkpointInteractPressed =
            foregroundGameplay &&
            (GameplayInput_ReadRawAsyncKeyState('F') & 0x8000) != 0;
        if (g_gameplayKeyboardHook == nullptr &&
            checkpointInteractPressed &&
            !checkpointInteractPollKeyDown)
        {
            bool expected = false;
            if (g_checkpointInteractActionDown.compare_exchange_strong(
                    expected,
                    true))
            {
                if (CheckpointSystem_HasNearbyInteraction())
                {
                    g_checkpointInteractionKeyConsumed = true;
                    g_checkpointSearchRequested = true;
                    LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Checkpoint interaction consumed by foreground polling.");
                }
            }
        }
        else if (!checkpointInteractPressed)
        {
            g_checkpointInteractActionDown = false;
            g_checkpointInteractionKeyConsumed = false;
        }
        checkpointInteractPollKeyDown = checkpointInteractPressed;

        const bool checkpointAbandonPressed =
            foregroundGameplay &&
            (GameplayInput_ReadRawAsyncKeyState('Y') & 0x8000) != 0;
        if (checkpointAbandonPressed &&
            !checkpointAbandonPollKeyDown)
        {
            bool expected = false;
            if (CheckpointSystem_HasAcceptedTask() &&
                g_checkpointAbandonActionDown.compare_exchange_strong(
                    expected,
                    true))
            {
                g_checkpointAbandonTaskRequested = true;
                LogInfo(LogCategory_Lifecycle, L"Lifecycle", L"Checkpoint task abandon queued by foreground polling.");
            }
        }
        else if (!checkpointAbandonPressed &&
                 !CheckpointSystem_IsTaskUiOpen())
        {
            g_checkpointAbandonActionDown = false;
        }
        checkpointAbandonPollKeyDown = checkpointAbandonPressed;

        MsgWaitForMultipleObjects(0, nullptr, FALSE, 16, QS_ALLINPUT);
    }

    if (g_gameplayKeyboardHook != nullptr)
    {
        UnhookWindowsHookEx(g_gameplayKeyboardHook);
        g_gameplayKeyboardHook = nullptr;
    }
    if (g_extendedMouseHook != nullptr)
    {
        UnhookWindowsHookEx(g_extendedMouseHook);
        g_extendedMouseHook = nullptr;
    }
    KillTimer(g_messageWindow, kOverlayPollTimer);
    StopNetwork();
    Log_FlushAndStop();
    WSACleanup();
    g_runtimeThreadStopped = true;
}
