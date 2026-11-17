// Native.cpp module: escape_mode_ui.inc
// Included inside the shared implementation namespace by Native.cpp.
void NpcDialogue_ResetForSceneChange();

void HideTitleCanvas(bool hidden)
{
    if (g_titleCanvasComponent == nullptr)
    {
        return;
    }
    bool enabled = !hidden;
    void* parameters[] = { &enabled };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Behaviour"), "set_enabled", 1),
        g_titleCanvasComponent,
        parameters);
}

bool SetActiveSceneValue(Il2CppObject* scene)
{
    if (scene == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    auto* sceneManager = FindClass("UnityEngine.SceneManagement", "SceneManager");
    void* parameters[] = { il2cpp_object_unbox(scene) };
    auto* result = Invoke(
        FindMethod(sceneManager, "SetActiveScene", 1),
        nullptr,
        parameters);
    return result != nullptr &&
           il2cpp_object_unbox != nullptr &&
           *static_cast<bool*>(il2cpp_object_unbox(result));
}

constexpr int kEscapeFromManakaActionsPerPage = 8;

void RefreshEscapeFromManakaActionPage()
{
    MoaningAudio_RefreshDebugPanel();
}

void EscapeFromManaka_UpdateDebugActionClicks()
{
    if (!g_efmActionListOpen || !g_lobbyUiOpen)
    {
        return;
    }
    auto* debugPanelGameObject =
        GetComponentGameObject(g_efmDebugActionPanel);
    if (debugPanelGameObject == nullptr)
    {
        return;
    }
    auto* activeSelfObject = Invoke(
        FindMethod(
            FindClass("UnityEngine", "GameObject"),
            "get_activeSelf",
            0),
        debugPanelGameObject,
        nullptr);
    if (activeSelfObject == nullptr ||
        !*static_cast<bool*>(il2cpp_object_unbox(activeSelfObject)))
    {
        return;
    }

    static bool mouseWasDown = false;
    const bool mouseDown =
        (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (!mouseDown)
    {
        mouseWasDown = false;
        return;
    }
    if (mouseWasDown)
    {
        return;
    }
    mouseWasDown = true;

    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    POINT cursor{};
    if (gameWindow == nullptr ||
        !GetClientRect(gameWindow, &clientRect) ||
        !GetCursorPos(&cursor) ||
        !ScreenToClient(gameWindow, &cursor))
    {
        return;
    }

    const float width =
        static_cast<float>(clientRect.right - clientRect.left);
    const float height =
        static_cast<float>(clientRect.bottom - clientRect.top);
    if (width <= 0.0f || height <= 0.0f)
    {
        return;
    }

    const float screenX =
        static_cast<float>(cursor.x - clientRect.left) / width;
    const float screenY =
        1.0f -
        static_cast<float>(cursor.y - clientRect.top) / height;
    constexpr float debugMinX = 0.36504f;
    constexpr float debugMinY = 0.18620f;
    constexpr float debugWidth = 0.48200f;
    constexpr float debugHeight = 0.52080f;
    const float localX = (screenX - debugMinX) / debugWidth;
    const float localY = (screenY - debugMinY) / debugHeight;
    if (localX < 0.0f || localX > 1.0f ||
        localY < 0.0f || localY > 1.0f)
    {
        return;
    }

    if (localY >= 0.07f && localY <= 0.15f)
    {
        if (localX >= 0.08f && localX <= 0.24f)
        {
            MoaningAudio_ChangePage(-1);
            return;
        }
        if (localX >= 0.76f && localX <= 0.92f)
        {
            MoaningAudio_ChangePage(1);
            return;
        }
    }

    for (int slot = 0; slot < kEscapeFromManakaActionsPerPage; ++slot)
    {
        const int column = slot % 2;
        const int row = slot / 2;
        const float minX = column == 0 ? 0.06f : 0.53f;
        const float minY = 0.64f - row * 0.10f;
        if (localX >= minX && localX <= minX + 0.41f &&
            localY >= minY && localY <= minY + 0.075f)
        {
            MoaningAudio_PlayDebugSlot(slot);
            return;
        }
    }
}

void BuildEscapeFromManakaUi()
{
    if (g_multiplayerCanvasObject != nullptr)
    {
        return;
    }

    auto* titleText = FindFirstTmpTextComponent(g_nativeButton);
    if (titleText != nullptr)
    {
        g_tmpFontAsset = Invoke(
            FindMethod(FindClass("TMPro", "TMP_Text"), "get_font", 0),
            titleText,
            nullptr);
    }
    LogInfo(LogCategory_Ui, L"Ui", L"TMP font asset for multiplayer scene: %p", g_tmpFontAsset);

    auto* buttonTransform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        g_nativeButton,
        nullptr);
    auto* titleCanvasTransform = FindCanvasTransform(buttonTransform);
    if (titleCanvasTransform != nullptr)
    {
        auto* titleCanvasGameObject = Invoke(
            FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
            titleCanvasTransform,
            nullptr);
        g_titleCanvasComponent = GetSceneComponentByClass(
            titleCanvasGameObject,
            "UnityEngine",
            "Canvas");
    }

    auto* sceneManager = FindClass("UnityEngine.SceneManagement", "SceneManager");
    const auto getActiveScene = FindMethod(sceneManager, "GetActiveScene", 0);
    const auto createScene = FindMethod(sceneManager, "CreateScene", 1);
    LogInfo(LogCategory_Ui, L"Ui", 
        L"SceneManager=%p GetActiveScene=%p CreateScene=%p",
        sceneManager,
        getActiveScene,
        createScene);
    g_titleScene = Invoke(
        getActiveScene,
        nullptr,
        nullptr);
    auto* sceneName = il2cpp_string_new("EscapeFromManaka_preparing");
    void* createParameters[] = { sceneName };
    g_multiplayerScene = Invoke(
        createScene,
        nullptr,
        createParameters);
    if (g_multiplayerScene == nullptr)
    {
        LogInfo(LogCategory_Ui, L"Ui", L"Unity SceneManager.CreateScene failed; building Escape from Manaka in the active scene.");
    }
    else if (!SetActiveSceneValue(g_multiplayerScene))
    {
        LogInfo(LogCategory_Ui, L"Ui", L"Unity could not activate the multiplayer scene.");
    }

    g_multiplayerCanvasObject = CreateSceneGameObject("EscapeFromManakaCanvas", nullptr);
    auto* canvasRect = GetSceneComponentByClass(
        g_multiplayerCanvasObject,
        "UnityEngine",
        "RectTransform");
    auto* canvas = AddSceneComponentToGameObject(
        g_multiplayerCanvasObject,
        "UnityEngine",
        "Canvas");
    auto* canvasScaler = AddSceneComponentToGameObject(
        g_multiplayerCanvasObject,
        "UnityEngine.UI",
        "CanvasScaler");
    AddSceneComponentToGameObject(
        g_multiplayerCanvasObject,
        "UnityEngine.UI",
        "GraphicRaycaster");

    int renderMode = 0;
    int sortingOrder = 32000;
    void* renderModeParameters[] = { &renderMode };
    void* sortingParameters[] = { &sortingOrder };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Canvas"), "set_renderMode", 1),
        canvas,
        renderModeParameters);
    Invoke(
        FindMethod(FindClass("UnityEngine", "Canvas"), "set_sortingOrder", 1),
        canvas,
        sortingParameters);
    SetSceneRect(canvasRect, 0.0f, 0.0f, 1.0f, 1.0f);

    auto* unityScreenClass = FindClass("UnityEngine", "Screen");
    auto* unityScreenWidthObject = Invoke(
        FindMethod(unityScreenClass, "get_width", 0),
        nullptr,
        nullptr);
    const int unityScreenWidth = unityScreenWidthObject != nullptr
        ? *static_cast<int*>(il2cpp_object_unbox(unityScreenWidthObject))
        : 0;
    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    const int clientWidth = gameWindow != nullptr && GetClientRect(gameWindow, &clientRect)
        ? clientRect.right - clientRect.left
        : 0;
    if (canvasScaler != nullptr && unityScreenWidth > 0 && clientWidth > 0)
    {
        float scaleFactor = static_cast<float>(clientWidth) / static_cast<float>(unityScreenWidth);
        void* scaleParameters[] = { &scaleFactor };
        Invoke(
            FindMethod(FindClass("UnityEngine.UI", "CanvasScaler"), "set_scaleFactor", 1),
            canvasScaler,
            scaleParameters);
        LogInfo(LogCategory_Ui, L"Ui", 
            L"Canvas scale factor: %.4f (client=%d unity=%d)",
            scaleFactor,
            clientWidth,
            unityScreenWidth);
    }

    auto* rootTransform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        g_multiplayerCanvasObject,
        nullptr);
    auto background = CreateScenePanel(
        "SceneBackdrop",
        rootTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.012f,
        0.015f,
        0.018f,
        1.0f);

    auto header = CreateScenePanel(
        "RouteBar",
        rootTransform,
        0.018f,
        0.892f,
        0.982f,
        0.977f,
        0.045f,
        0.050f,
        0.055f,
        0.99f);
    CreateSceneTextElement(
        "Title",
        header.rect,
        "ESCAPE FROM MANAKA",
        0.018f,
        0.18f,
        0.30f,
        0.88f,
        27.0f,
        257,
        0.94f,
        0.96f,
        0.98f,
        1.0f);

    auto equipmentTab = CreateSceneButton(
        "EquipmentTab",
        header.rect,
        "装备",
        0.31f,
        0.20f,
        0.405f,
        0.82f,
        0.62f,
        0.36f,
        0.18f,
        1.0f,
        20.0f,
        514,
        true);
    auto lobbyTab = CreateSceneButton(
        "LobbyTab",
        header.rect,
        "联机 / ONLINE",
        0.412f,
        0.20f,
        0.515f,
        0.82f,
        0.12f,
        0.14f,
        0.15f,
        1.0f,
        20.0f,
        514,
        true);
    auto fleaButton = CreateSceneButton(
        "FleaMarket",
        header.rect,
        "跳蚤市场",
        0.665f,
        0.20f,
        0.775f,
        0.82f,
        0.12f,
        0.14f,
        0.15f,
        1.0f,
        19.0f,
        514,
        true);
    auto startButton = CreateSceneButton(
        "StartGame",
        header.rect,
        "开始游戏",
        0.782f,
        0.20f,
        0.892f,
        0.82f,
        0.12f,
        0.14f,
        0.15f,
        1.0f,
        19.0f,
        514,
        true);
    auto closeButton = CreateSceneButton(
        "Close",
        header.rect,
        "关闭",
        0.905f,
        0.20f,
        0.982f,
        0.82f,
        0.34f,
        0.11f,
        0.10f,
        1.0f,
        18.0f,
        514,
        true);
    g_equipmentTabButton = equipmentTab.button;
    g_lobbyTabButton = lobbyTab.button;
    g_fleaMarketButton = fleaButton.button;
    g_startGameButton = startButton.button;
    g_closeUiButton = closeButton.button;

    auto content = CreateScenePanel(
        "PageHost",
        rootTransform,
        0.018f,
        0.035f,
        0.982f,
        0.875f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);

    auto equipmentPage = CreateScenePanel(
        "EquipmentPage",
        content.rect,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    g_equipmentPage = equipmentPage.rect;

    auto left = CreateScenePanel(
        "LoadoutColumn",
        equipmentPage.rect,
        0.0f,
        0.0f,
        0.315f,
        1.0f,
        0.02f,
        0.025f,
        0.03f,
        0.12f);
    CreateSceneTextElement(
        "LoadoutTitle",
        left.rect,
        "装备 / LOADOUT",
        0.035f,
        0.90f,
        0.95f,
        0.98f,
        24.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    CreateSceneTextElement(
        "CharacterHint",
        left.rect,
        "实时角色视图\n按住左键左右拖动旋转",
        0.05f,
        0.42f,
        0.95f,
        0.72f,
        18.0f,
        514,
        0.76f,
        0.80f,
        0.84f,
        0.88f);
    g_efmLoadoutPanelTransform = left.rect;
    for (int index = 0; index < 8; ++index)
    {
        const int column = index % 2;
        const int row = index / 2;
        char label[24]{};
        std::snprintf(label, sizeof(label), "装备槽 %02d", index + 1);
        auto slot = CreateSceneButton(
            label,
            left.rect,
            label,
            0.045f + column * 0.465f,
            0.04f + row * 0.075f,
            0.485f + column * 0.465f,
            0.105f + row * 0.075f,
            0.10f,
            0.12f,
            0.13f,
            0.92f,
            15.0f,
            514,
            false);
        (void)slot;
    }

    auto middle = CreateScenePanel(
        "BackpackColumn",
        equipmentPage.rect,
        0.32f,
        0.0f,
        0.655f,
        1.0f,
        0.055f,
        0.062f,
        0.068f,
        0.98f);
    CreateSceneTextElement(
        "BackpackTitle",
        middle.rect,
        "背包 / BACKPACK",
        0.035f,
        0.90f,
        0.95f,
        0.98f,
        24.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    for (int index = 0; index < 25; ++index)
    {
        const int column = index % 5;
        const int row = index / 5;
        char label[16]{};
        std::snprintf(label, sizeof(label), "%02d", index + 1);
        auto slot = CreateSceneButton(
            label,
            middle.rect,
            label,
            0.04f + column * 0.184f,
            0.035f + row * 0.166f,
            0.216f + column * 0.184f,
            0.191f + row * 0.166f,
            0.10f,
            0.12f,
            0.13f,
            0.96f,
            15.0f,
            514,
            false);
        (void)slot;
    }

    auto right = CreateScenePanel(
        "StashColumn",
        equipmentPage.rect,
        0.66f,
        0.0f,
        1.0f,
        1.0f,
        0.050f,
        0.056f,
        0.062f,
        0.98f);
    CreateSceneTextElement(
        "StashTitle",
        right.rect,
        "STASH",
        0.035f,
        0.91f,
        0.95f,
        0.98f,
        24.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    for (int index = 0; index < 36; ++index)
    {
        const int column = index % 6;
        const int row = index / 6;
        char label[16]{};
        std::snprintf(label, sizeof(label), "%02d", index + 1);
        auto slot = CreateSceneButton(
            label,
            right.rect,
            label,
            0.035f + column * 0.158f,
            0.035f + row * 0.143f,
            0.183f + column * 0.158f,
            0.166f + row * 0.143f,
            0.10f,
            0.12f,
            0.13f,
            0.96f,
            14.0f,
            514,
            false);
        (void)slot;
    }

    auto lobbyPage = CreateScenePanel(
        "LobbyPage",
        content.rect,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    g_lobbyPage = lobbyPage.rect;
    auto lobbyActionsPanel = CreateScenePanel(
        "LobbyActionsPanel",
        lobbyPage.rect,
        0.02f,
        0.02f,
        0.36f,
        0.97f,
        0.055f,
        0.062f,
        0.068f,
        0.98f);
    CreateSceneTextElement(
        "LobbyTitle",
        lobbyActionsPanel.rect,
        "联机 / ONLINE LOBBY",
        0.06f,
        0.86f,
        0.94f,
        0.97f,
        28.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    auto hostButton = CreateSceneButton(
        "HostRoom",
        lobbyActionsPanel.rect,
        "主机 / HOST ROOM",
        0.10f,
        0.58f,
        0.90f,
        0.72f,
        0.19f,
        0.42f,
        0.26f,
        1.0f,
        23.0f,
        514,
        true);
    auto joinButton = CreateSceneButton(
        "JoinRoom",
        lobbyActionsPanel.rect,
        "加入 / JOIN ROOM",
        0.10f,
        0.40f,
        0.90f,
        0.54f,
        0.16f,
        0.28f,
        0.50f,
        1.0f,
        23.0f,
        514,
        true);
    auto lobbyStatus = CreateSceneTextElement(
        "LobbyStatus",
        lobbyActionsPanel.rect,
        "WAITING FOR ROOM SERVICE",
        0.08f,
        0.10f,
        0.92f,
        0.30f,
        19.0f,
        514,
        0.92f,
        0.94f,
        0.96f,
        1.0f);

    auto roomListPanel = CreateScenePanel(
        "RoomListPanel",
        lobbyPage.rect,
        0.38f,
        0.02f,
        0.98f,
        0.97f,
        0.028f,
        0.034f,
        0.040f,
        0.98f);
    CreateSceneTextElement(
        "RoomListTitle",
        roomListPanel.rect,
        "ROOM LIST / 房间列表",
        0.035f,
        0.88f,
        0.96f,
        0.97f,
        28.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    auto roomListSummary = CreateSceneTextElement(
        "RoomListSummary",
        roomListPanel.rect,
        "NO LAN ROOMS DISCOVERED",
        0.035f,
        0.80f,
        0.96f,
        0.87f,
        18.0f,
        257,
        0.72f,
        0.78f,
        0.84f,
        1.0f);
    for (int index = 0; index < 6; ++index)
    {
        const char* rowNames[] = {
            "RoomRow01", "RoomRow02", "RoomRow03",
            "RoomRow04", "RoomRow05", "RoomRow06"
        };
        const char* rowLabels[] = {
            "ROOM SLOT 01   —   EMPTY",
            "ROOM SLOT 02   —   EMPTY",
            "ROOM SLOT 03   —   EMPTY",
            "ROOM SLOT 04   —   EMPTY",
            "ROOM SLOT 05   —   EMPTY",
            "ROOM SLOT 06   —   EMPTY"
        };
        const float maxY = 0.73f - index * 0.105f;
        auto row = CreateScenePanel(
            rowNames[index],
            roomListPanel.rect,
            0.035f,
            maxY - 0.082f,
            0.965f,
            maxY,
            0.070f,
            0.078f,
            0.086f,
            0.96f);
        auto rowLabel = CreateSceneTextElement(
            "RoomRowLabel",
            row.rect,
            rowLabels[index],
            0.035f,
            0.12f,
            0.96f,
            0.88f,
            18.0f,
            257,
            0.80f,
            0.84f,
            0.88f,
            1.0f);
        g_roomRowLabels.push_back(rowLabel.text);
    }

    auto debugActionPanel = CreateScenePanel(
        "EscapeFromManakaDebugAudioList",
        content.rect,
        0.36f,
        0.18f,
        0.86f,
        0.80f,
        0.045f,
        0.050f,
        0.055f,
        0.985f);
    CreateSceneTextElement(
        "DebugAudioListText",
        debugActionPanel.rect,
        "MOANING3 AUDIO TEST  ·  ~ CLOSE\n"
        "Click an individual WAV file to play it.",
        0.05f,
        0.84f,
        0.95f,
        0.97f,
        21.0f,
        257,
        0.92f,
        0.94f,
        0.96f,
        1.0f);
    for (int index = 0;
         index < kMoaningAudioFilesPerPage;
         ++index)
    {
        const int column = index % 2;
        const int row = index / 2;
        const float minX = column == 0 ? 0.06f : 0.53f;
        const float minY = 0.64f - row * 0.10f;
        char objectName[48]{};
        std::snprintf(
            objectName,
            sizeof(objectName),
            "EscapeFromManakaAudioButton%d",
            index);
        auto actionButton = CreateSceneButton(
            objectName,
            debugActionPanel.rect,
            "FILE",
            minX,
            minY,
            minX + 0.41f,
            minY + 0.075f,
            0.12f,
            0.14f,
            0.17f,
            1.0f,
            14.0f,
            514,
            true);
        g_efmActionButtons.push_back(actionButton.button);
        g_efmActionLabels.push_back(actionButton.text);
    }
    auto previousPage = CreateSceneButton(
        "EscapeFromManakaPreviousAudioPage",
        debugActionPanel.rect,
        "PREV",
        0.08f,
        0.07f,
        0.24f,
        0.15f,
        0.12f,
        0.14f,
        0.17f,
        1.0f,
        16.0f,
        514,
        true);
    auto nextPage = CreateSceneButton(
        "EscapeFromManakaNextAudioPage",
        debugActionPanel.rect,
        "NEXT",
        0.76f,
        0.07f,
        0.92f,
        0.15f,
        0.12f,
        0.14f,
        0.17f,
        1.0f,
        16.0f,
        514,
        true);
    auto pageLabel = CreateSceneTextElement(
        "EscapeFromManakaAudioStatus",
        debugActionPanel.rect,
        "",
        0.25f,
        0.07f,
        0.75f,
        0.15f,
        14.0f,
        514,
        0.90f,
        0.92f,
        0.95f,
        1.0f);
    g_efmPrevActionPageButton = previousPage.button;
    g_efmNextActionPageButton = nextPage.button;
    g_efmActionPageLabel = pageLabel.text;
    g_efmDebugActionPanel = debugActionPanel.rect;
    RefreshEscapeFromManakaActionPage();

    g_hostButtonComponent = hostButton.button;
    g_joinButtonComponent = joinButton.button;
    g_lobbyStatusLabel = lobbyStatus.text;
    g_roomListLabel = roomListSummary.text;
    SetGameObjectActive(g_efmDebugActionPanel, false);
    g_uiRoot = canvas;

    auto* screenClass = FindClass("UnityEngine", "Screen");
    auto* screenWidth = Invoke(FindMethod(screenClass, "get_width", 0), nullptr, nullptr);
    auto* screenHeight = Invoke(FindMethod(screenClass, "get_height", 0), nullptr, nullptr);
    LogInfo(LogCategory_Ui, L"Ui", 
        L"Screen: %d x %d",
        screenWidth != nullptr ? *static_cast<int*>(il2cpp_object_unbox(screenWidth)) : -1,
        screenHeight != nullptr ? *static_cast<int*>(il2cpp_object_unbox(screenHeight)) : -1);
    LogSceneRect("Canvas", canvasRect);
    LogSceneRect("Content", content.rect);
    LogSceneRect("Left", left.rect);
    LogSceneRect("Middle", middle.rect);
    LogSceneRect("Right", right.rect);

    SwitchEscapeFromManakaTab(0);
    SetGameObjectActive(g_lobbyPage, false);
    SetGameObjectActive(g_equipmentPage, true);
    SetGameObjectActive(canvas, false);
    LogInfo(LogCategory_Ui, L"Ui", L"Created and activated the standalone Escape from Manaka interface.");
}

void UpdateEscapeFromManakaUiText()
{
    static unsigned long long renderedVersion = 0;
    static int renderedMode = -1;
    const auto version = g_statusVersion.load();
    const int mode = g_networkMode.load();
    if (version == renderedVersion && mode == renderedMode)
    {
        return;
    }
    renderedVersion = version;
    renderedMode = mode;

    const auto status = GetStatus();
    if (g_lobbyStatusLabel != nullptr)
    {
        const char* displayStatus =
            mode == 1
                ? (status.rfind(L"CONNECTED", 0) == 0 ? "HOST ONLINE" : "ROOM CREATED")
                : mode == 2
                    ? "SEARCHING LOBBY"
                    : mode == 3
                        ? (status.rfind(L"CONNECTED", 0) == 0
                            ? "CONNECTED TO ROOM"
                            : "JOINING ROOM")
                        : "READY";
        SetButtonVisual(
            g_lobbyStatusLabel,
            displayStatus,
            19.0f,
            514,
            0.92f,
            0.94f,
            0.96f,
            1.0f);
    }
    if (g_roomListLabel != nullptr)
    {
        const char* roomText =
            mode == 1 ? "LOCAL ROOM: HOST" :
            mode == 2 ? "SEARCHING LOBBY..." :
            mode == 3 ? "HOST FOUND  ·  CONNECTED" :
                        "NO ROOMS DISCOVERED";
        SetSceneText(
            g_roomListLabel,
            roomText,
            18.0f,
            257,
            0.72f,
            0.78f,
            0.84f,
            1.0f);
    }
    if (!g_roomRowLabels.empty() && g_roomRowLabels[0] != nullptr)
    {
        const char* firstRoom =
            mode == 1
                ? (status.rfind(L"CONNECTED", 0) == 0
                    ? "ROOM 01   LOCAL HOST   ·   1/2 CONNECTED"
                    : "ROOM 01   LOCAL HOST   ·   WAITING")
                : mode == 3
                    ? "ROOM 01   REMOTE HOST   ·   CONNECTED"
                    : "ROOM SLOT 01   —   EMPTY";
        SetSceneText(
            g_roomRowLabels[0],
            firstRoom,
            18.0f,
            257,
            mode == 0 ? 0.80f : 0.92f,
            0.84f,
            mode == 0 ? 0.88f : 0.96f,
            1.0f);
    }
    if (g_hostButtonComponent != nullptr)
    {
        const bool connected = mode == 1 && status.rfind(L"CONNECTED", 0) == 0;
        SetButtonVisual(
            g_hostButtonComponent,
            connected ? "主机已连接 / HOST CONNECTED" : "主机 / HOST ROOM",
            23.0f,
            514,
            0.92f,
            0.94f,
            0.96f,
            1.0f);
    }
    if (g_joinButtonComponent != nullptr)
    {
        const bool searching = mode == 2;
        const bool connected = mode == 3 && status.rfind(L"CONNECTED", 0) == 0;
        SetButtonVisual(
            g_joinButtonComponent,
            connected
                ? "已加入主机 / JOINED"
                : searching ? "正在搜索主机..." : "加入 / JOIN ROOM",
            23.0f,
            514,
            0.92f,
            0.94f,
            0.96f,
            1.0f);
    }
}

void EscapeFromManaka_ResetRuntimeState()
{
    MoaningAudio_Release();
    NpcDialogue_ResetForSceneChange();
    g_nativeButtonInjected = false;
    g_nativeButton = nullptr;
    g_nativeButtonComponent = nullptr;
    g_nativeButtonRect = nullptr;
    g_continueButtonComponent = nullptr;
    g_startButtonComponent = nullptr;
    g_titleMenuButtons.clear();
    g_uiRoot = nullptr;
    g_uiBackdrop = nullptr;
    g_equipmentPage = nullptr;
    g_lobbyPage = nullptr;
    g_equipmentTabButton = nullptr;
    g_lobbyTabButton = nullptr;
    g_closeUiButton = nullptr;
    g_fleaMarketButton = nullptr;
    g_startGameButton = nullptr;
    g_hostButtonComponent = nullptr;
    g_joinButtonComponent = nullptr;
    g_lobbyStatusLabel = nullptr;
    g_roomListLabel = nullptr;
    g_roomRowLabels.clear();
    g_multiplayerCanvasObject = nullptr;
    g_multiplayerScene = nullptr;
    g_titleCanvasComponent = nullptr;
    g_previewStageComponent = nullptr;
    g_previewCameraComponent = nullptr;
    g_previewRenderTexture = nullptr;
    g_previewModel = nullptr;
    g_previewAnimator = nullptr;
    g_efmLoadoutPanelTransform = nullptr;
    g_efmPreparingScene = nullptr;
    g_efmUiPlayerGameObject = nullptr;
    g_efmUiPlayerController = nullptr;
    g_efmPreviewController = nullptr;
    g_efmPreviewIdleClip = nullptr;
    g_efmDebugActionPanel = nullptr;
    g_efmPreparingState = 0;
    g_efmPreparingFrames = 0;
    g_efmInitializeInvoked = false;
    g_efmPoseResetInvoked = false;
    g_efmToggleActionListRequested = false;
    g_efmActionListOpen = false;
    g_efmAudioSilenced = false;
    g_efmModelDragging = false;
    g_efmModelYaw = 0.0f;
    g_efmSelectedAction = 0;
    g_efmActionPage = 0;
    g_efmPendingAction = -1;
    g_efmActionButtons.clear();
    g_efmActionLabels.clear();
    g_efmActionPageLabel = nullptr;
    g_efmPrevActionPageButton = nullptr;
    g_efmNextActionPageButton = nullptr;
}

void EscapeFromManaka_ReturnToTitle()
{
    EscapeFromManaka_ResetRuntimeState();
    auto* sceneManager = FindClass(
        "UnityEngine.SceneManagement",
        "SceneManager");
    auto* titleName = il2cpp_string_new("Title");
    int loadMode = 0;
    void* parameters[] = { titleName, &loadMode };
    Invoke(
        FindMethod(sceneManager, "LoadScene", 2),
        nullptr,
        parameters);
    LogInfo(LogCategory_Ui, L"Ui", L"Escape from Manaka returned to a fresh Title scene.");
}

void SetEscapeFromManakaOpen(bool open)
{
    if (g_nativeButtonComponent == nullptr)
    {
        return;
    }
    if (open && g_uiRoot == nullptr)
    {
        BuildEscapeFromManakaUi();
    }
    if (g_uiRoot == nullptr)
    {
        return;
    }

    g_lobbyUiOpen = open;
    SetGameObjectActive(g_uiRoot, open);
    if (open)
    {
        if (g_networkMode == 0)
        {
            SetStatus(L"WAITING FOR ROOM SERVICE");
        }
        SetWorldGameObjectActive(g_previewStageComponent, true);
        HideTitleCanvas(true);
        SetActiveSceneValue(g_multiplayerScene);
        EscapeFromManaka_preparing();
        SwitchEscapeFromManakaTab(g_activeUiTab);
        UpdateEscapeFromManakaUiText();
        LogInfo(LogCategory_Ui, L"Ui", L"Opened Escape from Manaka preparing interface.");
    }
    else
    {
        SetWorldGameObjectActive(g_previewStageComponent, false);
        HideTitleCanvas(false);
        EscapeFromManaka_ReturnToTitle();
    }
}

