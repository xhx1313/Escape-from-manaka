// Native.cpp module: mod_guide_ui.inc
// Independent in-game guide for Escape from Manaka. The original tutorial
// remains untouched; this panel only uses the existing UI, cursor and
// TimeScaleManager facilities.

constexpr int kModGuidePageCount = 5;
constexpr int kModGuidePageImage = 3;

struct ModGuidePageDefinition
{
    const char* navigationLabel;
    const char* title;
    const char* body;
    bool showMapImage;
};

static const ModGuidePageDefinition kModGuidePages[kModGuidePageCount] = {
    {
        "玩法概览",
        "Escape from Manaka / 玩法概览",
        "这是基于原版角色、场景和任务系统制作的撤离玩法。\n"
        "\n"
        "基本流程：\n"
        "黑房准备 -> 选择场景与时间 -> 局内搜索和完成任务\n"
        "-> 在撤离点按 F 返回黑房。\n"
        "\n"
        "进入前必须先在原版游戏中创建并使用手动存档槽 0。\n"
        "顶部的联机大厅、跳蚤市场和展览柜目前仍为占位页面。",
        false,
    },
    {
        "黑房准备",
        "黑房仓库 / 出发前准备",
        "装备页可以在装备槽、背包、安全箱和仓库之间整理物品。\n"
        "\n"
        "- 拖动物品到分类槽即可穿戴，拖回背包或仓库即可卸下。\n"
        "- 拖动时按 R 可以旋转物品。\n"
        "- 装备和背包变化会在放下物品后立即保存。\n"
        "- 底部 1 / 2 / 3 快捷格只绑定背包或安全箱中的成人用品。\n"
        "- 快捷格右键可以解除绑定，实际物品不会丢失。\n"
        "\n"
        "准备好后点击右上角“进入游戏”，选择场景和白天/夜晚。",
        false,
    },
    {
        "操作按键",
        "局内操作 / CONTROLS",
        "WASD + 鼠标    移动和视角\n"
        "按住 E         打开动作菜单，松开关闭\n"
        "Tab            打开或关闭局内物资栏\n"
        "F              原版交互、搜索物资点、确认任务、撤离\n"
        "M              打开或关闭当前楼层大地图\n"
        "T              搭讪准星附近的 NPC；再次按下结束\n"
        "Y              放弃当前任务\n"
        "Q              切换大衣 / 裸露状态\n"
        "1 / 2 / 3      使用对应成人用品快捷格\n"
        "Esc            先关闭 MOD 界面；没有界面时打开原版菜单\n"
        "F1             打开或关闭本指南",
        false,
    },
    {
        "资源与撤离",
        "资源点、任务与撤离",
        "小地图只显示本局已经生成的资源点、有效撤离/转移点，\n"
        "以及当前任务需要显示的目标点。\n"
        "\n"
        "圆形 = 物资点\n"
        "方形 = 撤离点或转移点\n"
        "星形 = 当前任务目标\n"
        "\n"
        "靠近物资圆环后按 F 查看任务并接取。\n"
        "接取后右上角显示追踪；完成任务会出现奖励物资。\n"
        "在方形撤离点按 F 返回黑房，按 Y 可放弃当前任务。\n"
        "M 大地图支持滚轮缩放、拖动、楼层切换和单击追踪。",
        true,
    },
    {
        "风险与进阶",
        "风险、NPC 与进阶功能",
        "准星附近 2 米内有 NPC 时会显示“搭讪”，按 T 开始对话。\n"
        "佩戴强振动器时，对话会启用耐心、兴奋和 QTE 机制。\n"
        "\n"
        "按住 E 选择暴露动作时，看到玩家的 NPC 可能主动靠近。\n"
        "NPC 发现玩家后仍会执行原版发现与失败流程。\n"
        "\n"
        "Esc 只关闭当前模态界面，不会穿透打开系统菜单。\n"
        "N / B 地图点编辑属于高级工具，建议先熟悉正常玩法。\n"
        "F5 / F6 / F9 是开发或截图工具，不属于普通操作流程。",
        false,
    },
};

struct ModGuideUiState
{
    SceneUiElement root{};
    SceneUiElement mainPanel{};
    SceneUiElement leftPanel{};
    SceneUiElement rightPanel{};
    SceneUiElement closeButton{};
    SceneUiElement navigationButtons[kModGuidePageCount]{};
    SceneUiElement imagePanel{};
    SceneUiElement pageTitleElement{};
    SceneUiElement bodyElement{};
    Il2CppObject* pageTitle = nullptr;
    Il2CppObject* bodyText = nullptr;
    Il2CppObject* footerText = nullptr;
    Il2CppObject* closeText = nullptr;
    Il2CppObject* navigationText[kModGuidePageCount]{};
    std::wstring imagePath;
};

static ModGuideUiState g_modGuideUi;
static std::atomic<bool> g_modGuideOpen{false};
static std::atomic<bool> g_modGuideToggleRequested{false};
static std::atomic<bool> g_modGuideOpenRequested{false};
static std::atomic<bool> g_modGuideAwaitSystemMenuClose{false};
static std::atomic<bool> g_modGuideCloseRequested{false};
static std::atomic<bool> g_modGuidePhysicalDown{false};
static std::atomic<bool> g_modGuideClickRequested{false};
static std::atomic<LONG> g_modGuideClickX{0};
static std::atomic<LONG> g_modGuideClickY{0};
static std::atomic<int> g_modGuidePage{0};
static std::atomic<int> g_modGuidePageRequested{-1};
static bool g_modGuideTimeScaleActive = false;
static unsigned long long g_modGuideSystemMenuCloseRequestedAt = 0;

bool ModGuideUi_IsOpen()
{
    return g_modGuideOpen.load();
}

void ModGuideUi_RequestClose()
{
    g_modGuideCloseRequested = true;
}

void ModGuideUi_RequestOpenFromTutorial()
{
    g_modGuideCloseRequested = false;
    g_modGuideOpenRequested = true;
}

void ModGuideUi_SetTimeScale(bool active)
{
    auto* managerClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "TimeScaleManager");
    auto* manager = managerClass != nullptr
        ? ExtendedMode_GetStaticObject(
              "ExposureUnnoticed2.Scripts.Base",
              "TimeScaleManager",
              "Instance")
        : nullptr;
    auto* switchMode = managerClass != nullptr
        ? FindMethod(managerClass, "SwitchMode", 2)
        : nullptr;
    if (manager == nullptr || switchMode == nullptr)
    {
        return;
    }
    int mode = 1; // TimeScaleManager.TimeScaleMode.OpenTutorial
    bool enabled = active;
    void* parameters[] = { &mode, &enabled };
    Invoke(switchMode, manager, parameters);
    g_modGuideTimeScaleActive = active;
}

bool ModGuideUi_IsCanvasUsable()
{
    auto* canvas = g_extendedModeState.load() == ExtendedMode_Gameplay
        ? g_extendedGameplayHudCanvasObject
        : g_extendedCanvas;
    return ExtendedMode_IsUnityObjectAlive(canvas);
}

void ModGuideUi_ReleaseReferences()
{
    g_modGuideUi = ModGuideUiState{};
}

void ModGuideUi_Build()
{
    if (ExtendedMode_IsUnityObjectAlive(g_modGuideUi.root.gameObject))
    {
        return;
    }
    ModGuideUi_ReleaseReferences();
    const int state = g_extendedModeState.load();
    if (state != ExtendedMode_Active &&
        state != ExtendedMode_Gameplay)
    {
        return;
    }
    auto* canvas = state == ExtendedMode_Gameplay
        ? g_extendedGameplayHudCanvasObject
        : g_extendedCanvas;
    if (!ExtendedMode_IsUnityObjectAlive(canvas))
    {
        return;
    }
    if (g_fullScUnityFont == nullptr)
    {
        ExtendedMode_CacheFont();
    }
    auto* rootTransform = GetWorldTransform(canvas);
    if (rootTransform == nullptr)
    {
        return;
    }

    g_modGuideUi.root = CreateScenePanel(
        "EscapeFromManakaModGuide",
        rootTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.01f,
        0.015f,
        0.025f,
        0.84f);
    BringSceneObjectToFront(g_modGuideUi.root.gameObject);

    g_modGuideUi.mainPanel = CreateScenePanel(
        "ModGuideMainPanel",
        g_modGuideUi.root.rect,
        0.075f,
        0.070f,
        0.925f,
        0.930f,
        0.025f,
        0.035f,
        0.052f,
        0.985f);
    CreateScenePanel(
        "ModGuideHeader",
        g_modGuideUi.mainPanel.rect,
        0.0f,
        0.905f,
        1.0f,
        1.0f,
        0.10f,
        0.25f,
        0.33f,
        0.96f);
    CreateSceneTextElement(
        "ModGuideHeaderTitle",
        g_modGuideUi.mainPanel.rect,
        "ESCAPE FROM MANAKA  /  玩法指南",
        0.025f,
        0.915f,
        0.670f,
        0.985f,
        23.0f,
        257,
        0.96f,
        0.98f,
        1.0f,
        1.0f);

    g_modGuideUi.closeButton = CreateScenePanel(
        "ModGuideCloseButton",
        g_modGuideUi.mainPanel.rect,
        0.785f,
        0.918f,
        0.970f,
        0.988f,
        0.24f,
        0.07f,
        0.07f,
        0.98f);
    g_modGuideUi.closeText = CreateSceneTextElement(
        "ModGuideCloseText",
        g_modGuideUi.closeButton.rect,
        "关闭  ESC",
        0.04f,
        0.08f,
        0.96f,
        0.92f,
        15.0f,
        514,
        1.0f,
        0.94f,
        0.94f,
        1.0f).text;

    g_modGuideUi.leftPanel = CreateScenePanel(
        "ModGuideNavigation",
        g_modGuideUi.mainPanel.rect,
        0.025f,
        0.050f,
        0.255f,
        0.890f,
        0.025f,
        0.040f,
        0.060f,
        0.96f);
    for (int index = 0; index < kModGuidePageCount; ++index)
    {
        const float maxY = 0.820f - static_cast<float>(index) * 0.132f;
        const float minY = maxY - 0.092f;
        char name[64]{};
        std::snprintf(name, sizeof(name), "ModGuidePageButton%d", index);
        g_modGuideUi.navigationButtons[index] = CreateScenePanel(
            name,
            g_modGuideUi.leftPanel.rect,
            0.035f,
            minY,
            0.965f,
            maxY,
            0.035f,
            0.055f,
            0.082f,
            0.94f);
        g_modGuideUi.navigationText[index] = CreateSceneTextElement(
            "Label",
            g_modGuideUi.navigationButtons[index].rect,
            kModGuidePages[index].navigationLabel,
            0.045f,
            0.08f,
            0.955f,
            0.92f,
            17.0f,
            257,
            0.92f,
            0.96f,
            1.0f,
            1.0f).text;
    }

    g_modGuideUi.rightPanel = CreateScenePanel(
        "ModGuideContent",
        g_modGuideUi.mainPanel.rect,
        0.275f,
        0.050f,
        0.975f,
        0.890f,
        0.035f,
        0.050f,
        0.072f,
        0.97f);
    g_modGuideUi.pageTitleElement = CreateSceneTextElement(
        "ModGuidePageTitle",
        g_modGuideUi.rightPanel.rect,
        "",
        0.035f,
        0.855f,
        0.965f,
        0.955f,
        23.0f,
        257,
        0.96f,
        0.98f,
        1.0f,
        1.0f);
    g_modGuideUi.pageTitle = g_modGuideUi.pageTitleElement.text;
    g_modGuideUi.bodyElement = CreateSceneTextElement(
        "ModGuideBody",
        g_modGuideUi.rightPanel.rect,
        "",
        0.035f,
        0.145f,
        0.965f,
        0.825f,
        17.0f,
        257,
        0.88f,
        0.93f,
        0.98f,
        1.0f);
    g_modGuideUi.bodyText = g_modGuideUi.bodyElement.text;
    g_modGuideUi.imagePanel = CreateSceneRawImageElement(
        "ModGuideMapImage",
        g_modGuideUi.rightPanel.rect,
        L"",
        0.280f,
        0.115f,
        0.755f,
        0.505f,
        1.0f,
        1.0f,
        1.0f,
        0.96f);
    g_modGuideUi.footerText = CreateSceneTextElement(
        "ModGuideFooter",
        g_modGuideUi.rightPanel.rect,
        "F1 关闭指南  ·  鼠标点击左侧分类切换页面",
        0.035f,
        0.025f,
        0.965f,
        0.100f,
        13.0f,
        257,
        0.62f,
        0.72f,
        0.82f,
        1.0f).text;

    SetWorldGameObjectActive(g_modGuideUi.imagePanel.gameObject, false);
    SetWorldGameObjectActive(g_modGuideUi.root.gameObject, false);
    LogInfo(LogCategory_Ui, L"Ui", L"Independent MOD guide UI built with %d pages.", kModGuidePageCount);
}

void ModGuideUi_RefreshPage(int page)
{
    page = std::clamp(page, 0, kModGuidePageCount - 1);
    g_modGuidePage = page;
    const auto& definition = kModGuidePages[page];
    SetSceneText(
        g_modGuideUi.pageTitle,
        definition.title,
        23.0f,
        257,
        0.96f,
        0.98f,
        1.0f,
        1.0f);
    SetSceneText(
        g_modGuideUi.bodyText,
        definition.body,
        17.0f,
        257,
        0.88f,
        0.93f,
        0.98f,
        1.0f);
    for (int index = 0; index < kModGuidePageCount; ++index)
    {
        const bool selected = index == page;
        SetSceneGraphicColor(
            g_modGuideUi.navigationButtons[index].image,
            selected ? 0.12f : 0.035f,
            selected ? 0.38f : 0.055f,
            selected ? 0.58f : 0.082f,
            0.96f);
    }

    if (definition.showMapImage)
    {
        SetSceneRect(
            g_modGuideUi.bodyElement.rect,
            0.035f,
            0.535f,
            0.965f,
            0.825f);
        const std::wstring path =
            GetEscapeFromManakaSubdirectory(L"Maps") +
            L"\\fashion_shop\\floor_01_512.png";
        if (path != g_modGuideUi.imagePath)
        {
            auto* texture = LoadScenePngTexture(path);
            SetSceneRawImageTexture(
                g_modGuideUi.imagePanel.rawImage,
                texture);
            g_modGuideUi.imagePath = path;
        }
        SetWorldGameObjectActive(g_modGuideUi.imagePanel.gameObject, true);
        BringSceneObjectToFront(g_modGuideUi.imagePanel.gameObject);
    }
    else
    {
        SetWorldGameObjectActive(g_modGuideUi.imagePanel.gameObject, false);
        SetSceneRect(
            g_modGuideUi.bodyElement.rect,
            0.035f,
            0.145f,
            0.965f,
            0.825f);
    }
}

void ModGuideUi_Show(bool show)
{
    if (show)
    {
        ModGuideUi_Build();
        if (!ExtendedMode_IsUnityObjectAlive(g_modGuideUi.root.gameObject))
        {
            LogInfo(LogCategory_Ui, L"Ui", L"Independent MOD guide could not open because its canvas is unavailable.");
            return;
        }
        ModGuideUi_RefreshPage(g_modGuidePage.load());
        SetWorldGameObjectActive(g_modGuideUi.root.gameObject, true);
        BringSceneObjectToFront(g_modGuideUi.root.gameObject);
        g_modGuideOpen = true;
        ModGuideUi_SetTimeScale(true);
        ExtendedMode_SetGameplayHudRaycastsEnabled(false);
        ExtendedMode_ForceCursor();
        LogInfo(LogCategory_Ui, L"Ui", L"Independent MOD guide opened with mouse cursor ownership.");
        return;
    }

    if (g_modGuideOpen.exchange(false))
    {
        if (ExtendedMode_IsUnityObjectAlive(g_modGuideUi.root.gameObject))
        {
            SetWorldGameObjectActive(g_modGuideUi.root.gameObject, false);
        }
        if (g_modGuideTimeScaleActive)
        {
            ModGuideUi_SetTimeScale(false);
        }
        ExtendedMode_SetGameplayHudRaycastsEnabled(true);
        if (g_extendedModeState.load() == ExtendedMode_Gameplay)
        {
            ExtendedMode_ForceGameplayCursor();
        }
        else
        {
            ExtendedMode_ForceCursor();
        }
        LogInfo(LogCategory_Ui, L"Ui", L"Independent MOD guide closed and gameplay cursor restored.");
    }
}

void ModGuideUi_ApplyClick(LONG screenX, LONG screenY)
{
    auto* gameWindow = FindGameWindow();
    RECT clientRect{};
    POINT point{ screenX, screenY };
    if (gameWindow == nullptr ||
        !GetClientRect(gameWindow, &clientRect) ||
        !ScreenToClient(gameWindow, &point) ||
        clientRect.right <= clientRect.left ||
        clientRect.bottom <= clientRect.top)
    {
        return;
    }
    const float width = static_cast<float>(
        clientRect.right - clientRect.left);
    const float height = static_cast<float>(
        clientRect.bottom - clientRect.top);
    const float x = static_cast<float>(point.x) / width;
    const float y = 1.0f - static_cast<float>(point.y) / height;

    // Convert root-panel coordinates to main-panel local coordinates.
    constexpr float rootMinX = 0.075f;
    constexpr float rootMinY = 0.070f;
    constexpr float rootMaxX = 0.925f;
    constexpr float rootMaxY = 0.930f;
    if (!ExtendedMode_PointInRect(
            x, y, rootMinX, rootMinY, rootMaxX, rootMaxY))
    {
        return;
    }
    const float localX =
        (x - rootMinX) / (rootMaxX - rootMinX);
    const float localY =
        (y - rootMinY) / (rootMaxY - rootMinY);
    if (ExtendedMode_PointInRect(
            localX, localY, 0.785f, 0.918f, 0.970f, 0.988f))
    {
        ModGuideUi_RequestClose();
        return;
    }
    constexpr float leftMinX = 0.025f;
    constexpr float leftMinY = 0.050f;
    constexpr float leftMaxX = 0.255f;
    constexpr float leftMaxY = 0.890f;
    const float leftLocalX =
        (localX - leftMinX) / (leftMaxX - leftMinX);
    const float leftLocalY =
        (localY - leftMinY) / (leftMaxY - leftMinY);
    for (int index = 0; index < kModGuidePageCount; ++index)
    {
        const float maxY = 0.820f - static_cast<float>(index) * 0.132f;
        const float minY = maxY - 0.092f;
        if (ExtendedMode_PointInRect(
                leftLocalX,
                leftLocalY,
                0.035f,
                minY,
                0.965f,
                maxY))
        {
            g_modGuidePageRequested = index;
            return;
        }
    }
}

void ModGuideUi_Tick()
{
    const int state = g_extendedModeState.load();
    if (state != ExtendedMode_Active &&
        state != ExtendedMode_Gameplay)
    {
        ModGuideUi_Reset();
        return;
    }
    if (g_modGuideCloseRequested.exchange(false))
    {
        ModGuideUi_Show(false);
    }
    const bool tutorialRequested =
        g_modGuideOpenRequested.exchange(false);
    if (tutorialRequested &&
        !g_modGuideAwaitSystemMenuClose.load())
    {
        if (ExtendedMode_IsOriginalSystemMenuActive())
        {
            auto* systemMenuClass = FindClass(
                "ExposureUnnoticed2.ObjectUI.SystemMenu",
                "SystemMenuView");
            auto* systemMenu = systemMenuClass != nullptr
                ? Invoke(
                      FindMethod(systemMenuClass, "get_Instance", 0),
                      nullptr,
                      nullptr)
                : nullptr;
            auto* resume = systemMenuClass != nullptr
                ? FindMethod(systemMenuClass, "OnClickResume", 0)
                : nullptr;
            if (systemMenu != nullptr && resume != nullptr)
            {
                g_modGuideAwaitSystemMenuClose = true;
                g_modGuideSystemMenuCloseRequestedAt =
                    GetTickCount64();
                Invoke(resume, systemMenu, nullptr);
                g_extendedOriginalSystemMenuOpen = false;
                g_extendedOriginalSystemMenuVisible = false;
                ExtendedMode_SetCustomUiBehindSystemMenu(false);
                LogInfo(LogCategory_Ui, L"Ui", L"MOD tutorial request closed the original system menu before opening the MOD guide.");
                return;
            }
        }
        if (!g_modGuideOpen.load())
        {
            ModGuideUi_Show(true);
        }
    }
    else if (g_modGuideAwaitSystemMenuClose.load())
    {
        const unsigned long long now = GetTickCount64();
        if (ExtendedMode_IsOriginalSystemMenuActive() &&
            now - g_modGuideSystemMenuCloseRequestedAt < 750)
        {
            return;
        }
        g_modGuideAwaitSystemMenuClose = false;
        g_modGuideSystemMenuCloseRequestedAt = 0;
        if (!g_modGuideOpen.load())
        {
            ModGuideUi_Show(true);
        }
    }
    if (g_modGuideToggleRequested.exchange(false))
    {
        if (g_modGuideOpen.load())
        {
            ModGuideUi_Show(false);
        }
        else if (!g_extendedOriginalSystemMenuVisible.load() &&
                 !g_extendedOriginalTutorialActive.load() &&
                 !g_extendedOriginalInteractionMenuActive.load() &&
                 !g_extendedOriginalHandcuffsPanelActive.load() &&
                 !g_extendedSceneSelectorOpen &&
                 !g_extendedActionListOpen &&
                 !EquipmentPreviewUi_IsOpen() &&
                 !ItemContextMenu_IsOpen() &&
                 !GameplayInput_IsCustomModalUiOpen())
        {
            ModGuideUi_Show(true);
        }
        else
        {
            LogInfo(LogCategory_Ui, L"Ui", L"Independent MOD guide ignored the open request while another modal UI was active.");
        }
    }
    if (!g_modGuideOpen.load())
    {
        return;
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_modGuideUi.root.gameObject))
    {
        ModGuideUi_ReleaseReferences();
        ModGuideUi_Build();
        if (!ExtendedMode_IsUnityObjectAlive(g_modGuideUi.root.gameObject))
        {
            ModGuideUi_Show(false);
            return;
        }
        SetWorldGameObjectActive(g_modGuideUi.root.gameObject, true);
        ModGuideUi_RefreshPage(g_modGuidePage.load());
    }
    const int requestedPage = g_modGuidePageRequested.exchange(-1);
    if (requestedPage >= 0)
    {
        ModGuideUi_RefreshPage(requestedPage);
        LogInfo(LogCategory_Ui, L"Ui", L"Independent MOD guide selected page %d.", requestedPage + 1);
    }
    if (g_modGuideClickRequested.exchange(false))
    {
        ModGuideUi_ApplyClick(
            g_modGuideClickX.load(),
            g_modGuideClickY.load());
    }
    ExtendedMode_ForceCursor();
}

void ModGuideUi_Reset()
{
    if (g_modGuideOpen.exchange(false))
    {
        if (ExtendedMode_IsUnityObjectAlive(g_modGuideUi.root.gameObject))
        {
            SetWorldGameObjectActive(g_modGuideUi.root.gameObject, false);
        }
        if (g_modGuideTimeScaleActive)
        {
            ModGuideUi_SetTimeScale(false);
        }
        ExtendedMode_SetGameplayHudRaycastsEnabled(true);
    }
    else if (g_modGuideTimeScaleActive)
    {
        ModGuideUi_SetTimeScale(false);
    }
    g_modGuideToggleRequested = false;
    g_modGuideOpenRequested = false;
    g_modGuideAwaitSystemMenuClose = false;
    g_modGuideSystemMenuCloseRequestedAt = 0;
    g_modGuideCloseRequested = false;
    g_modGuidePhysicalDown = false;
    g_modGuideClickRequested = false;
    g_modGuidePageRequested = -1;
    g_modGuidePage = 0;
    ModGuideUi_ReleaseReferences();
}

bool ModGuideUi_ProcessPhysicalKey(
    int virtualKey,
    bool down,
    const wchar_t* source)
{
    if (virtualKey != VK_F1)
    {
        return false;
    }
    const bool wasDown = g_modGuidePhysicalDown.exchange(down);
    if (down && !wasDown)
    {
        const int state = g_extendedModeState.load();
        if ((state == ExtendedMode_Active ||
             state == ExtendedMode_Gameplay) &&
            !g_extendedOriginalSystemMenuVisible.load() &&
            !g_extendedOriginalTutorialActive.load())
        {
            g_modGuideCloseRequested = false;
            g_modGuideToggleRequested = true;
            LogInfo(LogCategory_Ui, L"Ui", 
                L"Independent MOD guide toggle queued by %s.",
                source != nullptr ? source : L"input");
        }
    }
    return true;
}

bool ModGuideUi_ProcessMouseMessage(
    UINT message,
    LONG screenX,
    LONG screenY)
{
    if (!g_modGuideOpen.load())
    {
        return false;
    }
    auto* gameWindow = FindGameWindow();
    if (gameWindow == nullptr ||
        GetForegroundWindow() != gameWindow)
    {
        return false;
    }
    if (message == WM_MOUSEMOVE)
    {
        return false;
    }
    if (message == WM_LBUTTONDOWN ||
        message == WM_LBUTTONDBLCLK)
    {
        g_modGuideClickX = screenX;
        g_modGuideClickY = screenY;
        g_modGuideClickRequested = true;
        return true;
    }
    if (message == WM_LBUTTONUP)
    {
        return true;
    }
    if (message == WM_RBUTTONDOWN ||
        message == WM_RBUTTONUP ||
        message == WM_RBUTTONDBLCLK ||
        message == WM_MBUTTONDOWN ||
        message == WM_MBUTTONUP ||
        message == WM_MBUTTONDBLCLK ||
        message == WM_MOUSEWHEEL ||
        message == WM_MOUSEHWHEEL)
    {
        return true;
    }
    return false;
}
