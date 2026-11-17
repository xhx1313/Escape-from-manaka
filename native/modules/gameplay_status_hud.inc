// Native.cpp module: gameplay_status_hud.inc
// Compact, state-driven gameplay status list displayed in the upper-left.

constexpr int kGameplayStatusHudMaxRows = 13;
constexpr float kGameplayStatusHudFadeSeconds = 0.22f;
constexpr int kGameplayControlsHintCount = 9;

static SceneUiElement g_gameplayStatusHudPanel{};
static SceneUiElement g_gameplayControlsHintPanel{};
static SceneUiElement g_gameplayControlsHintRows[kGameplayControlsHintCount]{};
static Il2CppObject* g_gameplayControlsHintTexts[kGameplayControlsHintCount]{};
struct GameplayStatusHudRowState
{
    SceneUiElement background;
    SceneUiElement element;
    std::string text;
    std::string renderedText;
    float alpha = 0.0f;
    float targetAlpha = 0.0f;
    float renderedAlpha = -1.0f;
};
static std::vector<GameplayStatusHudRowState> g_gameplayStatusHudRows;
static unsigned long long g_gameplayStatusHudLastUpdateTick = 0;

Il2CppObject* GameplayStatusHud_GetPlayerClassAccessor()
{
    return g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
}

Il2CppObject* GameplayStatusHud_GetPlayerState()
{
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    return playerClassAccessor != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x60)
        : nullptr;
}

struct GameplayActionSnapshot
{
    Il2CppObject* actionRecord = nullptr;
    int actionType = -1;
    bool active = false;
};

GameplayActionSnapshot GameplayAction_ReadCurrent()
{
    GameplayActionSnapshot snapshot{};
    auto* playerState = GameplayStatusHud_GetPlayerState();
    if (playerState == nullptr)
    {
        return snapshot;
    }
    snapshot.actionRecord = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerState) + 0x48);
    if (snapshot.actionRecord == nullptr)
    {
        return snapshot;
    }
    snapshot.actionType = *reinterpret_cast<int*>(
        reinterpret_cast<char*>(snapshot.actionRecord) + 0x18);
    snapshot.active = snapshot.actionType >= 0;
    return snapshot;
}

struct GameplayStatusHudMovementState
{
    bool valid = false;
    float moveSpeed = 0.0f;
    bool isDash = false;
    bool isCrouch = false;
};

bool GameplayStatusHud_ReadMovementState(
    GameplayStatusHudMovementState& output)
{
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    auto* playerState = GameplayStatusHud_GetPlayerState();
    if (playerClassAccessor == nullptr || playerState == nullptr)
    {
        return false;
    }

    auto* locomotion = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerClassAccessor) + 0x58);
    auto* moveSpeed = locomotion != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(locomotion) + 0x28)
        : nullptr;
    output.moveSpeed = moveSpeed != nullptr
        ? *reinterpret_cast<float*>(
            reinterpret_cast<char*>(moveSpeed) + 0x10)
        : 0.0f;
    output.isDash = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x19);
    output.isCrouch = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x1B);
    output.valid = true;
    return true;
}

bool GameplayStatusHud_IsAdultGoodsEquipped(int adultGoodsType)
{
    static const Il2CppMethod* isEquipped = nullptr;
    if (isEquipped == nullptr)
    {
        isEquipped = FindMethod(
            FindClass("ExposureUnnoticed2.Scripts.InGame", "GameStateFacade"),
            "IsEquipAdultGoods",
            1);
    }
    if (isEquipped == nullptr)
    {
        return false;
    }
    int type = adultGoodsType;
    void* parameters[] = { &type };
    auto* value = Invoke(isEquipped, nullptr, parameters);
    return value != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(value));
}

bool GameplayStatusHud_IsVibratorStrong()
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData == nullptr ||
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1B8) != 2)
    {
        return false;
    }
    return
        GameplayStatusHud_IsAdultGoodsEquipped(1) ||
        GameplayStatusHud_IsAdultGoodsEquipped(6) ||
        GameplayStatusHud_IsAdultGoodsEquipped(7);
}

bool GameplayStatusHud_IsPlayerInsideElevator()
{
    auto* elevator = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.ScenePlops.Elevator",
        "ElevatorController",
        "Instance");
    if (elevator == nullptr)
    {
        return false;
    }
    auto* isInside = FindMethod(
        FindClass(
            "ExposureUnnoticed2.Object3D.ScenePlops.Elevator",
            "ElevatorController"),
        "IsInsidePlayer",
        0);
    auto* value = isInside != nullptr
        ? Invoke(isInside, elevator, nullptr)
        : nullptr;
    return value != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(value));
}

const char* GameplayStatusHud_GetActionLabel(int actionType)
{
    for (int index = 0; index < kExtendedPoseMenuOptionCount; ++index)
    {
        if (kExtendedPoseMenuOptions[index].actionType == actionType)
        {
            return kExtendedPoseMenuOptions[index].label;
        }
    }
    switch (actionType)
    {
    case 10000: return "捡大衣";
    case 10001: return "拾取";
    case 10002: return "丢弃";
    case 10003: return "换衣";
    case 10006: return "插入肛塞";
    case 10007: return "取出肛塞";
    case 10008: return "通用装备";
    case 10011: return "使用售货机";
    case 10012: return "喝水";
    case 10019: return "戴手铐";
    case 10020: return "把手铐放到面前";
    case 10021: return "被锁在面前物体";
    case 10022: return "解锁面前手铐";
    case 10023: return "戴眼罩";
    case 10024: return "切换振动器";
    case 10025: return "拾取物品";
    case 10026: return "坐在长椅上";
    case 10027: return "从长椅起身";
    default: return nullptr;
    }
}

const char* const kGameplayControlsHintLabels[kGameplayControlsHintCount] = {
    "<color=#8CE7FF>W/A/S/D</color>  移动",
    "<color=#8CE7FF>Q</color>  切换露出模式",
    "<color=#8CE7FF>E</color>  使用动作",
    "<color=#8CE7FF>F</color>  交互",
    "<color=#8CE7FF>T</color>  搭讪",
    "<color=#8CE7FF>R</color>  切换视角",
    "<color=#8CE7FF>Tab</color>  装备栏",
    "<color=#8CE7FF>M</color>  小地图",
    "<color=#8CE7FF>C</color>  蹲下",
};

void GameplayControlsHint_Build()
{
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    if (ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        SetWorldGameObjectActive(
            g_gameplayControlsHintPanel.gameObject,
            true);
        return;
    }

    g_gameplayControlsHintPanel = SceneUiElement{};
    for (auto& row : g_gameplayControlsHintRows)
    {
        row = SceneUiElement{};
    }
    for (auto*& text : g_gameplayControlsHintTexts)
    {
        text = nullptr;
    }
    if (g_fullScUnityFont == nullptr)
    {
        ExtendedMode_CacheFont();
    }
    auto* rootTransform = GetWorldTransform(
        g_extendedGameplayHudCanvasObject);
    if (rootTransform == nullptr)
    {
        return;
    }

    g_gameplayControlsHintPanel = CreateScenePanel(
        "EscapeFromManakaControlsHint",
        rootTransform,
        0.016f,
        0.020f,
        0.178f,
        0.106f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        return;
    }
    for (int index = 0; index < kGameplayControlsHintCount; ++index)
    {
        const int column = index % 3;
        const int row = index / 3;
        constexpr float cellWidth = 0.309f;
        constexpr float columnStep = 0.327f;
        constexpr float cellHeight = 0.302f;
        constexpr float rowStep = 0.337f;
        const float minX = 0.015f + column * columnStep;
        const float maxX = minX + cellWidth;
        const float maxY = 0.985f - static_cast<float>(row) * rowStep;
        const float minY = maxY - cellHeight;
        char backgroundName[64]{};
        char textName[64]{};
        std::snprintf(
            backgroundName,
            sizeof(backgroundName),
            "ControlsHintRow%02d",
            index);
        std::snprintf(
            textName,
            sizeof(textName),
            "ControlsHintText%02d",
            index);
        g_gameplayControlsHintRows[index] = CreateScenePanel(
            backgroundName,
            g_gameplayControlsHintPanel.rect,
            minX,
            minY,
            maxX,
            maxY,
            0.030f,
            0.043f,
            0.060f,
            0.78f);
        g_gameplayControlsHintTexts[index] = CreateSceneTextElement(
            textName,
            g_gameplayControlsHintRows[index].rect,
            kGameplayControlsHintLabels[index],
            0.035f,
            0.04f,
            0.965f,
            0.96f,
            14.0f,
            257,
            0.90f,
            0.95f,
            1.0f,
            1.0f).text;
    }
    SetWorldGameObjectActive(
        g_gameplayControlsHintPanel.gameObject,
        true);
    LogInfo(LogCategory_Ui, L"Ui", L"Gameplay controls hint created.");
}

void GameplayControlsHint_Reset()
{
    g_gameplayControlsHintPanel = SceneUiElement{};
    for (auto& row : g_gameplayControlsHintRows)
    {
        row = SceneUiElement{};
    }
    for (auto*& text : g_gameplayControlsHintTexts)
    {
        text = nullptr;
    }
}

void GameplayControlsHint_Update()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (ExtendedMode_IsUnityObjectAlive(
                g_gameplayControlsHintPanel.gameObject))
        {
            SetWorldGameObjectActive(
                g_gameplayControlsHintPanel.gameObject,
                false);
        }
        return;
    }
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        GameplayControlsHint_Build();
    }
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        return;
    }
    const bool hiddenByModal =
        GameplayInput_IsCustomModalUiOpen() ||
        g_extendedOriginalTutorialActive.load() ||
        g_extendedOriginalSystemMenuVisible.load() ||
        g_extendedOriginalInteractionMenuActive.load() ||
        g_extendedOriginalHandcuffsPanelActive.load();
    SetWorldGameObjectActive(
        g_gameplayControlsHintPanel.gameObject,
        !hiddenByModal);
}

void GameplayStatusHud_Build()
{
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayStatusHudPanel.gameObject) &&
        static_cast<int>(g_gameplayStatusHudRows.size()) ==
            kGameplayStatusHudMaxRows)
    {
        SetWorldGameObjectActive(g_gameplayStatusHudPanel.gameObject, true);
        return;
    }

    g_gameplayStatusHudPanel = SceneUiElement{};
    g_gameplayStatusHudRows.clear();
    g_gameplayStatusHudLastUpdateTick = 0;
    auto* rootTransform = GetWorldTransform(g_extendedGameplayHudCanvasObject);
    g_gameplayStatusHudPanel = CreateScenePanel(
        "EscapeFromManakaStatusHud",
        rootTransform,
        0.016f,
        0.515f,
        0.305f,
        0.965f,
        0.012f,
        0.018f,
        0.026f,
        0.0f);
    if (g_gameplayStatusHudPanel.gameObject == nullptr)
    {
        return;
    }
    g_gameplayStatusHudRows.reserve(kGameplayStatusHudMaxRows);
    for (int index = 0; index < kGameplayStatusHudMaxRows; ++index)
    {
        char backgroundName[64]{};
        char textName[64]{};
        std::snprintf(
            backgroundName,
            sizeof(backgroundName),
            "StatusRowBackground%02d",
            index);
        std::snprintf(
            textName,
            sizeof(textName),
            "StatusRowText%02d",
            index);
        const float maxY = 0.955f - static_cast<float>(index) * 0.071f;
        const float minY = maxY - 0.064f;
        auto background = CreateScenePanel(
            backgroundName,
            g_gameplayStatusHudPanel.rect,
            0.012f,
            minY,
            0.585f,
            maxY,
            0.034f,
            0.048f,
            0.066f,
            0.76f);
        auto row = CreateSceneTextElement(
            textName,
            background.rect,
            "",
            0.055f,
            0.09f,
            0.95f,
            0.91f,
            15.0f,
            257,
            0.88f,
            0.93f,
            0.98f,
            1.0f);
        SetWorldGameObjectActive(background.gameObject, false);
        GameplayStatusHudRowState rowState{};
        rowState.background = background;
        rowState.element = row;
        g_gameplayStatusHudRows.push_back(rowState);
    }
    SetWorldGameObjectActive(g_gameplayStatusHudPanel.gameObject, true);
    LogInfo(LogCategory_Ui, L"Ui", L"Gameplay status listener HUD created.");
}

void GameplayStatusHud_Reset()
{
    g_gameplayStatusHudPanel = SceneUiElement{};
    g_gameplayStatusHudRows.clear();
    g_gameplayStatusHudLastUpdateTick = 0;
    GameplayControlsHint_Reset();
}

void GameplayStatusHud_Update()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (ExtendedMode_IsUnityObjectAlive(
                g_gameplayStatusHudPanel.gameObject))
        {
            SetWorldGameObjectActive(
                g_gameplayStatusHudPanel.gameObject,
                false);
        }
        return;
    }
    GameplayControlsHint_Update();
    if (!ExtendedMode_IsUnityObjectAlive(g_gameplayStatusHudPanel.gameObject))
    {
        GameplayStatusHud_Build();
    }
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    auto* playerState = GameplayStatusHud_GetPlayerState();
    if (playerClassAccessor == nullptr || playerState == nullptr)
    {
        LogOnce(
            "gameplay-status-hud-player-missing",
            LogLevel_Debug,
            LogCategory_Ui,
            L"HudRead",
            L"Gameplay status HUD skipped because player state references are unavailable.");
        return;
    }

    GameplayStatusHudMovementState movementState{};
    if (!GameplayStatusHud_ReadMovementState(movementState))
    {
        LogOnce(
            "gameplay-status-hud-movement-missing",
            LogLevel_Debug,
            LogCategory_Ui,
            L"HudRead",
            L"Gameplay status HUD could not read the locomotion movement state.");
        return;
    }
    const float currentMoveSpeed = movementState.moveSpeed;
    const bool isDash = movementState.isDash;
    const bool isCrouch = movementState.isCrouch;
    const bool nearNpc = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x88);
    const bool ecstasy = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x50);
    const float streetLightBareyasusa = *reinterpret_cast<float*>(
        reinterpret_cast<char*>(playerState) + 0x9C);
    auto* touchedLights = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerState) + 0x138);
    const int touchedLightCount = touchedLights != nullptr
        ? *reinterpret_cast<int*>(
            reinterpret_cast<char*>(touchedLights) + 0x18)
        : 0;
    const auto currentAction = GameplayAction_ReadCurrent();
    const int currentActionType = currentAction.actionType;
    auto* lastChair = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerState) + 0x1A8);
    const bool onChair =
        lastChair != nullptr &&
        ExtendedMode_IsUnityObjectAlive(lastChair) &&
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(lastChair) + 0x78) > 0;

    auto* gameStateData = ExtendedMode_GetGameStateData();
    const bool handsLocked =
        gameStateData != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A8);
    const bool lockedToObject =
        gameStateData != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A9);
    const int vibrationMode =
        gameStateData != nullptr
            ? *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1B8)
            : 0;

    std::vector<std::string> lines;
    lines.reserve(kGameplayStatusHudMaxRows);
    if (isCrouch)
    {
        lines.emplace_back(
            currentMoveSpeed > 0.02f ? "移动：爬行" : "移动：蹲伏");
    }
    else if (currentMoveSpeed > 0.02f)
    {
        lines.emplace_back(isDash ? "移动：奔跑" : "移动：行走");
    }
    else
    {
        lines.emplace_back("移动：静止");
    }

    lines.emplace_back(
        g_extendedSpecialCoatMode.load() ? "大衣状态" : "裸露状态");
    if (GameplayStatusHud_IsAdultGoodsEquipped(2))
    {
        lines.emplace_back("佩戴眼罩");
    }
    const bool vibrationActive =
        vibrationMode > 0 &&
        (GameplayStatusHud_IsAdultGoodsEquipped(1) ||
         GameplayStatusHud_IsAdultGoodsEquipped(6) ||
         GameplayStatusHud_IsAdultGoodsEquipped(7));
    if (vibrationActive)
    {
        const char* mode = vibrationMode == 1
            ? "弱"
            : (vibrationMode == 2
                ? "强"
                : (vibrationMode == 3 ? "随机" : "未知"));
        char line[96]{};
        std::snprintf(line, sizeof(line), "振动器：%s", mode);
        lines.emplace_back(line);
    }
    if (currentActionType >= 0)
    {
        const char* label = GameplayStatusHud_GetActionLabel(
            currentActionType);
        char line[160]{};
        if (label != nullptr)
        {
            std::snprintf(line, sizeof(line), "动作：%s", label);
        }
        else
        {
            std::snprintf(line, sizeof(line), "动作：#%d", currentActionType);
        }
        lines.emplace_back(line);
    }
    if (handsLocked)
    {
        lines.emplace_back("双手被锁住");
    }
    if (lockedToObject)
    {
        lines.emplace_back("被锁在面前物体");
    }
    if (nearNpc)
    {
        lines.emplace_back("路人身旁");
    }
    if (ecstasy)
    {
        lines.emplace_back("高潮状态");
    }
    if (streetLightBareyasusa > 0.0f || touchedLightCount > 0)
    {
        lines.emplace_back("在光源下");
    }
    if (GameplayStatusHud_IsPlayerInsideElevator())
    {
        lines.emplace_back("在电梯内");
    }
    if (onChair)
    {
        lines.emplace_back("在长椅上");
    }
    if (currentActionType == 10011)
    {
        lines.emplace_back("在使用售货机");
    }

    if (lines.size() > static_cast<size_t>(kGameplayStatusHudMaxRows))
    {
        lines.resize(kGameplayStatusHudMaxRows);
    }
    SetWorldGameObjectActive(g_gameplayStatusHudPanel.gameObject, true);
    const unsigned long long now = GetTickCount64();
    float deltaSeconds = g_gameplayStatusHudLastUpdateTick == 0
        ? 0.0f
        : static_cast<float>(
            now - g_gameplayStatusHudLastUpdateTick) / 1000.0f;
    g_gameplayStatusHudLastUpdateTick = now;
    deltaSeconds = std::clamp(deltaSeconds, 0.0f, 0.1f);
    const float alphaStep =
        deltaSeconds / kGameplayStatusHudFadeSeconds;

    for (int index = 0; index < kGameplayStatusHudMaxRows; ++index)
    {
        auto& row = g_gameplayStatusHudRows[static_cast<size_t>(index)];
        const std::string desiredText =
            index < static_cast<int>(lines.size())
                ? lines[static_cast<size_t>(index)]
                : std::string();

        if (index == 0)
        {
            row.text = desiredText;
            row.alpha = 1.0f;
            row.targetAlpha = 1.0f;
        }
        else if (row.text != desiredText)
        {
            if (row.alpha > 0.0f)
            {
                row.targetAlpha = 0.0f;
            }
            else
            {
                row.text = desiredText;
                row.alpha = 0.0f;
                row.targetAlpha = desiredText.empty() ? 0.0f : 1.0f;
            }
        }
        else
        {
            row.targetAlpha = desiredText.empty() ? 0.0f : 1.0f;
        }

        if (row.alpha < row.targetAlpha)
        {
            row.alpha = std::min(row.targetAlpha, row.alpha + alphaStep);
        }
        else if (row.alpha > row.targetAlpha)
        {
            row.alpha = std::max(row.targetAlpha, row.alpha - alphaStep);
        }

        if (row.alpha <= 0.0f && row.targetAlpha <= 0.0f &&
            row.text != desiredText)
        {
            row.text = desiredText;
            row.targetAlpha = desiredText.empty() ? 0.0f : 1.0f;
        }

        const bool visible = !row.text.empty();
        SetWorldGameObjectActive(row.background.gameObject, visible);
        if (!visible)
        {
            row.renderedText.clear();
            row.renderedAlpha = -1.0f;
            continue;
        }
        if (row.renderedText != row.text ||
            row.renderedAlpha != row.alpha)
        {
            SetSceneGraphicColor(
                row.background.image,
                index == 0 ? 0.045f : 0.055f,
                index == 0 ? 0.220f : 0.260f,
                index == 0 ? 0.085f : 0.105f,
                row.alpha * 0.76f);
            SetSceneText(
                row.element.text,
                row.text.c_str(),
                index == 0 ? 16.0f : 15.0f,
                257,
                index == 0 ? 0.48f : 0.88f,
                index == 0 ? 0.92f : 0.93f,
                index == 0 ? 1.0f : 0.98f,
                row.alpha);
            row.renderedText = row.text;
            row.renderedAlpha = row.alpha;
        }
    }
}
