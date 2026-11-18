// Native.cpp module: extended_mode.inc
// Native.cpp module: extended_mode.inc
// Full-gameplay Escape from Manaka mode.
//
// This module intentionally does not reuse the legacy title-scene preview
// lifecycle, PlayerClassAccessor construction, PlayerBodyCustomizeAdjuster
// calls, or the old equipment/lobby canvas.

void GameplayMinimap_Build(Il2CppObject* parentTransform);
void GameplayMinimap_Update();
void GameplayMinimap_Reset();
void GameplayWorldMap_Build(Il2CppObject* parentTransform);
void GameplayWorldMap_Reset();
bool GameplayWorldMap_ProcessPhysicalKey(
    int virtualKey,
    bool down,
    const wchar_t* source);
bool GameplayWorldMap_ShouldBlockNativeInput();

enum ExtendedModeState
{
    ExtendedMode_Inactive = 0,
    ExtendedMode_OpeningSavePanel = 1,
    ExtendedMode_LoadingGameplay = 2,
    ExtendedMode_PreparingGameplay = 3,
    ExtendedMode_Active = 4,
    ExtendedMode_EnteringGameplay = 5,
    ExtendedMode_Gameplay = 6,
    ExtendedMode_Exiting = 7,
};

constexpr int kExtendedRoomLayer = 31;
constexpr int kExtendedCanvasSortingOrder = 32000;
constexpr int kExtendedGameplayHudSortingOrder = 32100;
constexpr int kExtendedCanvasOrderBehindSystemMenu = -100;

struct ExtendedStageOption
{
    const char* label;
    int stageType;
    const char* sceneName;
};

static constexpr ExtendedStageOption kExtendedStageOptions[] = {
    { "便利店",   6,  "Konbini" },
    { "服装店",   7,  "Fashion_shop" },
    { "住宅区",  10, "Night_Demo 1" },
    { "购物中心", 11, "Mall" },
    { "繁华街",  12, "StationFront" },
    { "综合公园", 14, "MyPark" },
    { "公寓前",  15, "Mansion" },
};

static constexpr ExtendedStageOption kExtendedWarehouseStageOption = {
    "自宅",
    0,
    "Apartment",
};

constexpr int kEscapeFromManakaStageCount =
    sizeof(kExtendedStageOptions) / sizeof(kExtendedStageOptions[0]);

const ExtendedStageOption* ExtendedMode_FindStageOption(int stageType)
{
    if (stageType == kExtendedWarehouseStageOption.stageType)
    {
        return &kExtendedWarehouseStageOption;
    }
    for (const auto& option : kExtendedStageOptions)
    {
        if (option.stageType == stageType)
        {
            return &option;
        }
    }
    return nullptr;
}

struct ExtendedLayerSnapshot
{
    Il2CppObject* gameObject;
    int layer;
};

struct ExtendedAnimatorStateSnapshot
{
    bool valid = false;
    Il2CppObject* animator = nullptr;
    float layerWeights[12]{};
    float moveSpeed = 0.0f;
    float strafeX = 0.0f;
    float strafeY = 0.0f;
    bool isStrafe = false;
    bool isCrouch = false;
};

SceneUiElement g_extendedEnterGameButton{};
SceneUiElement g_extendedSceneSelectorPanel{};
SceneUiElement g_extendedSceneButtons[kEscapeFromManakaStageCount]{};
SceneUiElement g_extendedDayButton{};
SceneUiElement g_extendedNightButton{};
SceneUiElement g_extendedConfirmEnterButton{};
SceneUiElement g_extendedCancelEnterButton{};
Il2CppObject* g_extendedSceneSelectionHint = nullptr;
Il2CppObject* g_extendedGameplayHudCanvasObject = nullptr;
Il2CppObject* g_extendedCoordinateLabel = nullptr;
Il2CppObject* g_extendedInteractionPromptPanel = nullptr;
Il2CppObject* g_extendedInteractionPromptPanelRect = nullptr;
Il2CppObject* g_extendedInteractionPromptActionRect = nullptr;
Il2CppObject* g_extendedInteractionPromptActionLabel = nullptr;
Il2CppObject* g_extendedInteractionPromptKeyLabel = nullptr;
SceneUiElement g_extendedInteractionPromptPrefix{};
SceneUiElement g_extendedInteractionPromptKeyBadge{};
struct ExtendedInteractionPromptState
{
    bool visible = false;
    char key = '\0';
    std::string action;
};
ExtendedInteractionPromptState
    g_extendedInteractionPromptStates[2]{};
SceneUiElement g_extendedCoatModeButton{};
Il2CppObject* g_extendedPoseMenuPanel = nullptr;
SceneUiElement g_extendedPoseMenuButtons[24]{};
std::atomic<bool> g_extendedPoseMenuHeld{false};
std::atomic<bool> g_extendedPoseMenuVisible{false};
std::atomic<bool> g_extendedPoseMenuClickRequested{false};
std::atomic<int> g_extendedPoseMenuRequestedAction{-1};
bool g_extendedPoseMenuSuppressedUntilRelease = false;
int g_extendedPoseMenuHoverIndex = -1;
std::vector<ExtendedLayerSnapshot> g_extendedLayerSnapshots;
ExtendedAnimatorStateSnapshot g_extendedAnimatorStateSnapshot;
int g_extendedSelectedStageIndex = 0;
bool g_extendedSelectedDaytime = true;
bool g_extendedSceneSelectorOpen = false;
bool g_extendedTransitionInvoked = false;
bool g_extendedDirectWarehouseTransition = false;
bool g_extendedTransferLoadingOverlayActive = false;
bool g_extendedReturnToWarehouseAfterExit = false;
std::atomic<bool> g_extendedWarehouseReturnInProgress{false};
int g_extendedWarehouseCoverHideDelayFrames = 0;
int g_extendedWarehouseReentryDelay = 0;
int g_extendedPendingDeploymentStageType = -1;
bool g_extendedPendingDeploymentDaytime = true;
int g_extendedPendingDeploymentDelayFrames = 0;
int g_extendedReloadAfterMenuOpenFrames = 0;
bool g_extendedSceneLoadStarted = false;
bool g_extendedTargetAlreadyActive = false;
bool g_extendedTimeApplied = false;
int g_extendedSettledStageFrames = 0;
int g_extendedGameplayHudFrame = 0;
int g_extendedSourceStageType = 0;
bool g_extendedSpawnPositionApplied = false;
int g_extendedGameplayStableFrames = -1;
unsigned int g_extendedPlayerUpdatePumpCount = 0;
unsigned int g_extendedInputResolverCallCount = 0;
int g_extendedLastGameplayFrame = -1;
Il2CppObject* g_extendedLastResolvedInputData = nullptr;
HWND g_extendedTransferLoadingWindow = nullptr;
bool g_extendedLoadingWindowClassRegistered = false;
std::atomic<int> g_extendedTargetStageType{0};
std::atomic<bool> g_extendedTargetDaytime{true};
std::atomic<bool> g_extendedGameplayActive{false};
std::atomic<bool> g_extendedOpenSceneSelectorRequested{false};
std::atomic<bool> g_extendedEnterGameRequested{false};
std::atomic<bool> g_extendedSceneTransitionQueued{false};
std::atomic<int> g_extendedQueuedSceneTargetStage{-1};
std::atomic<bool> g_extendedQueuedTransitionPreserveDeployment{false};
std::atomic<bool> g_extendedQueuedTransitionFromSelection{false};
std::atomic<bool> g_extendedAwaitingFreshSceneManager{false};
std::atomic<unsigned long long> g_extendedSceneGeneration{0};
std::atomic<int> g_extendedExitPreviousState{ExtendedMode_Inactive};
std::atomic<bool> g_extendedGameplayUpdateObserved{false};
std::atomic<bool> g_extendedDefaultSavePending{false};
int g_extendedDefaultSaveWriteAttempts = 0;
std::atomic<bool> g_extendedOriginalInteractionMenuActive{false};
std::atomic<bool> g_extendedOriginalInteractionMenuClickRequested{false};
std::atomic<bool> g_extendedOriginalHandcuffsPanelActive{false};
std::atomic<bool> g_extendedOriginalHandcuffsPanelClickRequested{false};
std::atomic<unsigned long long> g_gameplayEyeMaskModelApplyAt{0};
std::atomic<bool> g_gameplayConfiguredEyeMaskRequested{false};
std::atomic<bool> g_gameplayEyeMaskForceThirdPersonRequested{false};
std::atomic<unsigned long long> g_gameplayVibratorModelApplyAt{0};
std::atomic<int> g_gameplayVibratorModelType{-1};
std::atomic<int> g_gameplayVibratorPreviousMode{-1};
std::atomic<int> g_gameplayVibratorNextMode{-1};
std::atomic<int> g_gameplayVibratorLogicalMode{0};
std::atomic<int> g_gameplayAdultQuickSlotRequested{-1};
bool g_gameplayVibratorSaveStateCaptured = false;
int g_gameplayOriginalVibrationMode = 0;
bool g_gameplayOriginalVibratorPresent[3]{};
bool g_gameplayOriginalVibratorActive[3]{};
std::atomic<bool> g_gameplayAdultQuickSlotPhysicalDown[
    kInventoryQuickSlotCount]{{false}, {false}, {false}};
Il2CppObject* g_extendedHandcuffsPanelInstance = nullptr;
unsigned long long g_extendedHandcuffsPanelNextDiscoveryAt = 0;
Il2CppObject* g_extendedBindingCandidate = nullptr;
Il2CppObject* g_extendedBindingManagerCandidate = nullptr;
Il2CppObject* g_extendedPlayerBeforeSceneLoad = nullptr;
int g_extendedBindingStableFrames = 0;
int g_extendedOutfitRefreshStage = 0;
int g_extendedLastMainThreadTickFrame = -1;
int g_extendedLastSafeTransitionPumpFrame = -1;
int g_extendedSceneCleanupFramesRemaining = 0;
int g_extendedSceneBindingStableFrames = 0;
int g_extendedEventManagerUpdateFrames = 0;
Il2CppObject* g_extendedActiveEventMethodManager = nullptr;
Il2CppObject* g_extendedSceneBindingEventManagerCandidate = nullptr;
std::wstring g_extendedQueuedTransitionReason;
std::atomic<unsigned long long> g_gameplayForbiddenKeyMask0{0};
std::atomic<unsigned long long> g_gameplayForbiddenKeyMask1{0};
std::atomic<unsigned long long> g_gameplayForbiddenKeyMask2{0};
std::atomic<unsigned long long> g_gameplayForbiddenKeyMask3{0};
std::atomic<unsigned long long> g_gameplayOriginalActionBlockedUntil{0};
std::unordered_set<Il2CppObject*> g_gameplayDisabledInputActions;
Il2CppObject* g_mapEditorBlockedEyeMaskInputAction = nullptr;

float g_extendedRoomCenterX = 0.0f;
float g_extendedRoomFloorY = 0.0f;
float g_extendedRoomCenterZ = 0.0f;
float g_extendedModelCenterY = 0.0f;
float g_extendedModelCenterX = 0.0f;
float g_extendedModelCenterZ = 0.0f;
float g_extendedModelForwardX = 0.0f;
float g_extendedModelForwardZ = 1.0f;
float g_extendedModelBaseLocalEulerX = 0.0f;
float g_extendedModelBaseLocalEulerY = 0.0f;
float g_extendedModelBaseLocalEulerZ = 0.0f;
float g_extendedOriginalLocalEulerX = 0.0f;
float g_extendedOriginalLocalEulerY = 0.0f;
float g_extendedOriginalLocalEulerZ = 0.0f;

struct ExtendedClothingStateSnapshot
{
    bool valid = false;
    int currentClothes = -1;
    int currentHadObject = -1;
    int currentHoldClothes = -1;
    int clothesAState = -1;
    int clothesBState = -1;
    bool playerStateValid = false;
    Il2CppObject* playerCurrentClothes = nullptr;
    int playerCurrentClothesBState = -1;
    int playerExposedBodyParts = -1;
    bool playerSkipDropClothes = false;
};

ExtendedClothingStateSnapshot g_extendedClothingStateSnapshot;
Il2CppObject* g_extendedCoatlessManager = nullptr;
Il2CppObject* g_extendedCoatlessCoatController = nullptr;
unsigned int g_extendedCoatlessDroppedCoatFrame = 0;
bool g_extendedCoatlessApplied = false;
bool g_extendedCoatlessDropSuppressedLogged = false;
bool g_extendedCoatlessCoatVisualEquipped = false;
std::atomic<bool> g_extendedSpecialCoatMode{false};
std::atomic<bool> g_extendedToggleSpecialCoatRequested{false};
bool g_extendedSpecialCoatApplied = false;

struct ExtendedSpecialCoatSnapshot
{
    bool valid = false;
    Il2CppObject* currentClothes = nullptr;
    int currentClothesBState = 10;
    int exposedBodyParts = 1;
    int gameStateClothes = -1;
    int gameStateHoldClothes = -1;
    int gameStateClothesAState = 0;
    int gameStateClothesBState = 10;
};

ExtendedSpecialCoatSnapshot g_extendedSpecialCoatSnapshot;

Il2CppObject* ExtendedMode_GetGameStateData();
void ExtendedMode_ApplyCoatlessLogic(bool force);
void ExtendedMode_CacheFont();
void ExtendedMode_ResetLegAnimationState();
void ExtendedMode_SetPlayerSkipDropClothes(
    Il2CppObject* playerState,
    bool skip);
bool ExtendedMode_GetPlayerSkipDropClothes(
    Il2CppObject* playerState);
bool ExtendedMode_GetGameObjectActiveSelf(Il2CppObject* gameObject);
bool ExtendedMode_GetBehaviourEnabled(Il2CppObject* component);
void ExtendedMode_HideDroppedCoat(bool force);
void ExtendedMode_UpdateCoatModeIndicator();
bool ExtendedMode_IsOriginalHandcuffsPanelActive();
void ExtendedMode_RefreshOriginalHandcuffsPanelActive();
void EquipmentPreviewUi_ApplyAdultGoodsAttachment(
    int adultGoodsType,
    bool active);
Il2CppObject* EquipmentPreviewUi_GetArrayElement(
    Il2CppObject* array,
    int index);
void GameplayInput_ApplyVibratorModeToOriginalState(
    int adultGoodsType,
    int mode,
    bool keepModelVisible);
void GameplayStatusHud_Build();
void GameplayStatusHud_Reset();
void GameplayStatusHud_Update();
void ExtendedMode_ClearCrossSceneUiCaches();
void ExtendedMode_SetCustomUiBehindSystemMenu(bool behind);
void ExtendedMode_CloseOriginalSystemMenuForSceneChange();
bool ExtendedMode_IsOriginalSystemMenuActive();
bool NpcDialogue_IsActive();
bool NpcDialogue_IsTalkPromptVisible();
bool NpcDialogue_ShouldBlockSystemMenuOpen();
bool NpcDialogue_HandleTalkKeyDown();
void NpcDialogue_RequestTalk();
void NpcDialogue_RequestClose();
void NpcDialogue_ReserveTalkKey();
bool NpcDialogue_IsTalkKeyReserved();
void NpcDialogue_Tick();
void NpcDialogue_ResetForSceneChange();
int ExtendedMode_GetUnityFrameCount();
bool ExtendedMode_IsGameplayManagerHealthy();
void ExtendedMode_ProcessSafeSceneTransitionPump();

bool ExtendedMode_IsRunning()
{
    const int state = g_extendedModeState.load();
    return state == ExtendedMode_OpeningSavePanel ||
           state == ExtendedMode_LoadingGameplay ||
           state == ExtendedMode_PreparingGameplay ||
           state == ExtendedMode_Active ||
           state == ExtendedMode_EnteringGameplay ||
           state == ExtendedMode_Gameplay ||
           state == ExtendedMode_Exiting;
}

void ExtendedMode_SetOriginalTutorialSuppressed(
    bool suppressed,
    const wchar_t* reason)
{
    const bool previous =
        g_modOriginalTutorialSuppressed.exchange(suppressed);
    if (previous == suppressed)
    {
        return;
    }
    LogInfo(LogCategory_Action, L"Action", 
        suppressed
            ? L"MOD session suppresses original tutorial triggers (%s)."
            : L"MOD original-tutorial suppression cleared (%s).",
        reason != nullptr ? reason : L"unspecified");
}

bool ExtendedMode_ShouldSuppressPlayerUpdate()
{
    return g_extendedSuppressPlayerUpdate.load() ||
           g_extendedSceneTransitionQueued.load();
}

bool GameplayInput_IsCustomModalUiOpen()
{
    return NpcDialogue_IsActive() ||
           ModGuideUi_IsOpen() ||
           g_checkpointFailureUiActive.load() ||
           g_extendedPoseMenuVisible.load() ||
           g_gameplayInventoryOpen.load() ||
           g_gameplayWorldMapOpen.load() ||
           CheckpointSystem_IsTaskUiOpen() ||
           CheckpointSystem_IsMapEditorUiOpen();
}

bool GameplayInput_ShouldBlockSystemMenuAfterDismiss()
{
    return g_extendedModeState.load() == ExtendedMode_Gameplay &&
           (ModGuideUi_IsOpen() ||
            g_extendedOriginalTutorialActive.load() ||
            g_extendedEscapeDismissConsumed.load() ||
            GetTickCount64() <
                g_extendedEscapeDismissBlockedUntil.load());
}

bool GameplayInput_ProcessEscapeDismissKey(
    int virtualKey,
    bool keyDown,
    bool keyUp,
    const wchar_t* source)
{
    if (virtualKey != VK_ESCAPE)
    {
        return false;
    }
    if (keyUp)
    {
        const bool consumed =
            g_extendedEscapeDismissConsumed.exchange(false);
        g_extendedEscapeDismissBlockedUntil.store(
            GetTickCount64() + 250);
        return consumed;
    }
    const int state = g_extendedModeState.load();
    const bool modGuideOpen = ModGuideUi_IsOpen();
    if (!keyDown ||
        (state != ExtendedMode_Gameplay &&
         !(state == ExtendedMode_Active && modGuideOpen)) ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (g_extendedEscapeDismissConsumed.load())
    {
        return true;
    }
    if (GetTickCount64() <
        g_extendedEscapeDismissBlockedUntil.load())
    {
        return true;
    }

    const bool inventoryOpen =
        state == ExtendedMode_Gameplay &&
        g_gameplayInventoryOpen.load();
    const bool worldMapOpen =
        state == ExtendedMode_Gameplay &&
        g_gameplayWorldMapOpen.load();
    const bool mapEditorOpen =
        state == ExtendedMode_Gameplay &&
        CheckpointSystem_IsMapEditorUiOpen();
    if (!inventoryOpen && !worldMapOpen && !mapEditorOpen && !modGuideOpen)
    {
        return false;
    }

    bool expected = false;
    if (!g_extendedEscapeDismissConsumed.compare_exchange_strong(
            expected,
            true))
    {
        return true;
    }
    g_extendedEscapeDismissRequested = true;
    g_extendedEscapeDismissBlockedUntil.store(
        GetTickCount64() + 1000);
    g_extendedOpenSystemMenuRequested = false;
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape dismiss queued by %s: inventory=%d worldMap=%d mapEditor=%d.",
        source != nullptr ? source : L"input",
        inventoryOpen ? 1 : 0,
        worldMapOpen ? 1 : 0,
        mapEditorOpen ? 1 : 0);
    if (modGuideOpen)
    {
        LogInfo(LogCategory_Action, L"Action", L"Escape dismiss will close the independent MOD guide.");
    }
    return true;
}

void GameplayInput_ResetEscapeDismissState()
{
    g_extendedEscapeDismissRequested = false;
    g_extendedEscapeDismissConsumed = false;
    g_extendedEscapeDismissBlockedUntil.store(
        GetTickCount64() + 500);
}

bool GameplayInput_RequestSystemMenu(const wchar_t* source)
{
    if (ModGuideUi_IsOpen() ||
        g_extendedOriginalTutorialActive.load() ||
        g_extendedEscapeDismissConsumed.load() ||
        GetTickCount64() <
            g_extendedEscapeDismissBlockedUntil.load())
    {
        g_extendedOpenSystemMenuRequested = false;
        LogInfo(LogCategory_Action, L"Action", 
            L"System menu request suppressed after modal Escape dismissal by %s.",
            source != nullptr ? source : L"input");
        return false;
    }
    if (g_extendedOpenSystemMenuRequested.exchange(true))
    {
        return false;
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"System menu request queued by %s.",
        source != nullptr ? source : L"input");
    return true;
}

bool GameplayInput_QueueAdultQuickSlot(
    int quickSlot,
    const wchar_t* source)
{
    if (quickSlot < 0 ||
        quickSlot >= kInventoryQuickSlotCount ||
        g_extendedModeState.load() != ExtendedMode_Gameplay ||
        g_extendedOriginalSystemMenuVisible.load() ||
        g_extendedOriginalInteractionMenuActive.load() ||
        g_extendedOriginalHandcuffsPanelActive.load() ||
        GameplayInput_IsCustomModalUiOpen())
    {
        return false;
    }

    int expected = -1;
    if (!g_gameplayAdultQuickSlotRequested.compare_exchange_strong(
            expected,
            quickSlot))
    {
        return false;
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods quick slot %d queued by %s.",
        quickSlot + 1,
        source != nullptr ? source : L"input");
    return true;
}

bool ExtendedMode_ShouldSuppressGameplayCamera()
{
    return g_extendedSuppressPlayerUpdate.load() ||
           ModGuideUi_IsOpen() ||
           g_extendedOriginalTutorialActive.load() ||
           g_checkpointFailureUiActive.load() ||
           NpcDialogue_IsActive() ||
           g_extendedPoseMenuVisible.load() ||
           g_gameplayInventoryOpen.load() ||
           g_gameplayWorldMapOpen.load() ||
           ExtendedMode_IsOriginalHandcuffsPanelActive() ||
           CheckpointSystem_IsTaskUiOpen();
}

bool GameplayInput_ShouldSuppressNativeKeyboardKey(int virtualKey)
{
    if (virtualKey == VK_ESCAPE &&
        (g_extendedEscapeDismissConsumed.load() ||
         GetTickCount64() <
             g_extendedEscapeDismissBlockedUntil.load()))
    {
        return true;
    }
    if ((virtualKey == 'N' || virtualKey == 'n') &&
        CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        return true;
    }
    if ((virtualKey == 'M' || virtualKey == 'm') &&
        GameplayWorldMap_ShouldBlockNativeInput())
    {
        return true;
    }
    if ((!NpcDialogue_IsActive() &&
         !ModGuideUi_IsOpen() &&
         !g_checkpointFailureUiActive.load() &&
         !g_extendedPoseMenuVisible.load() &&
         !g_gameplayInventoryOpen.load() &&
         !g_gameplayWorldMapOpen.load() &&
         !CheckpointSystem_IsTaskUiOpen()) ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    switch (virtualKey)
    {
    case VK_LBUTTON:
    case VK_RBUTTON:
    case VK_MBUTTON:
    case VK_XBUTTON1:
    case VK_XBUTTON2:
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:
    case VK_LWIN:
    case VK_RWIN:
        return false;
    default:
        return true;
    }
}

SHORT WINAPI GameplayInput_GetAsyncKeyStateHook(int virtualKey)
{
    using OriginalFn = SHORT (WINAPI*)(int);
    const SHORT state = g_originalGetAsyncKeyState != nullptr
        ? reinterpret_cast<OriginalFn>(
            g_originalGetAsyncKeyState)(virtualKey)
        : 0;
    return GameplayInput_ShouldSuppressNativeKeyboardKey(virtualKey)
        ? 0
        : state;
}

SHORT WINAPI GameplayInput_GetKeyStateHook(int virtualKey)
{
    using OriginalFn = SHORT (WINAPI*)(int);
    const SHORT state = g_originalGetKeyState != nullptr
        ? reinterpret_cast<OriginalFn>(
            g_originalGetKeyState)(virtualKey)
        : 0;
    return GameplayInput_ShouldSuppressNativeKeyboardKey(virtualKey)
        ? 0
        : state;
}

SHORT GameplayInput_ReadRawAsyncKeyState(int virtualKey)
{
    using OriginalFn = SHORT (WINAPI*)(int);
    return g_originalGetAsyncKeyState != nullptr
        ? reinterpret_cast<OriginalFn>(
            g_originalGetAsyncKeyState)(virtualKey)
        : GetAsyncKeyState(virtualKey);
}

bool GameplayInput_KeyMayReachOriginalActions(int virtualKey)
{
    return virtualKey == VK_ESCAPE ||
           virtualKey == 'W' ||
           virtualKey == 'A' ||
           virtualKey == 'S' ||
           virtualKey == 'D' ||
           virtualKey == 'F' ||
           virtualKey == 'R' ||
           virtualKey == 'C' ||
           virtualKey == VK_SHIFT ||
           virtualKey == VK_LSHIFT ||
           virtualKey == VK_RSHIFT ||
           virtualKey == VK_CONTROL ||
           virtualKey == VK_LCONTROL ||
           virtualKey == VK_RCONTROL ||
           virtualKey == VK_MENU ||
           virtualKey == VK_LMENU ||
           virtualKey == VK_RMENU ||
           virtualKey == VK_LWIN ||
           virtualKey == VK_RWIN ||
           virtualKey == VK_CAPITAL ||
           virtualKey == VK_LBUTTON ||
           virtualKey == VK_RBUTTON ||
           virtualKey == VK_MBUTTON ||
           virtualKey == VK_XBUTTON1 ||
           virtualKey == VK_XBUTTON2;
}

void GameplayInput_SetForbiddenKeyState(int virtualKey, bool down)
{
    if (virtualKey < 0 || virtualKey > 255)
    {
        return;
    }
    const int word = virtualKey / 64;
    const unsigned long long bit = 1ull << (virtualKey % 64);
    std::atomic<unsigned long long>* masks[] = {
        &g_gameplayForbiddenKeyMask0,
        &g_gameplayForbiddenKeyMask1,
        &g_gameplayForbiddenKeyMask2,
        &g_gameplayForbiddenKeyMask3,
    };
    if (down)
    {
        masks[word]->fetch_or(bit);
        g_gameplayOriginalActionBlockedUntil.store(
            GetTickCount64() + 150);
        static std::atomic<bool> loggedForbiddenKey[256]{};
        if (!loggedForbiddenKey[virtualKey].exchange(true))
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Gameplay original-action whitelist armed by vk=%d.",
                virtualKey);
        }
    }
    else
    {
        masks[word]->fetch_and(~bit);
    }
}

void GameplayInput_NotePhysicalKey(int virtualKey, bool down)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        return;
    }
    if (!down)
    {
        if (virtualKey == 'F')
        {
            g_checkpointInteractActionDown = false;
            g_checkpointInteractionKeyConsumed = false;
        }
        else if (virtualKey == 'Y')
        {
            g_checkpointAbandonActionDown = false;
        }
    }
    if (!GameplayInput_KeyMayReachOriginalActions(virtualKey))
    {
        GameplayInput_SetForbiddenKeyState(virtualKey, down);
    }
}

void GameplayInput_UpdateActionWhitelist()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        g_gameplayForbiddenKeyMask0.store(0);
        g_gameplayForbiddenKeyMask1.store(0);
        g_gameplayForbiddenKeyMask2.store(0);
        g_gameplayForbiddenKeyMask3.store(0);
        g_gameplayOriginalActionBlockedUntil.store(0);
        return;
    }

    unsigned long long masks[4]{};
    for (int virtualKey = 8; virtualKey < 256; ++virtualKey)
    {
        if (GameplayInput_KeyMayReachOriginalActions(virtualKey))
        {
            continue;
        }
        if ((GetAsyncKeyState(virtualKey) & 0x8000) != 0)
        {
            masks[virtualKey / 64] |= 1ull << (virtualKey % 64);
            g_gameplayOriginalActionBlockedUntil.store(
                GetTickCount64() + 150);
        }
    }
    g_gameplayForbiddenKeyMask0.store(masks[0]);
    g_gameplayForbiddenKeyMask1.store(masks[1]);
    g_gameplayForbiddenKeyMask2.store(masks[2]);
    g_gameplayForbiddenKeyMask3.store(masks[3]);
}

void GameplayInput_ClearForbiddenKeyState()
{
    g_gameplayForbiddenKeyMask0.store(0);
    g_gameplayForbiddenKeyMask1.store(0);
    g_gameplayForbiddenKeyMask2.store(0);
    g_gameplayForbiddenKeyMask3.store(0);
    g_gameplayOriginalActionBlockedUntil.store(0);
}

bool GameplayInput_IsForbiddenActionInputActive()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }

    static std::atomic<unsigned long long> lastKeyboardSample{0};
    const unsigned long long now = GetTickCount64();
    unsigned long long previous = lastKeyboardSample.load();
    while (previous != now &&
           !lastKeyboardSample.compare_exchange_weak(previous, now))
    {
    }
    if (previous != now)
    {
        GameplayInput_UpdateActionWhitelist();
    }

    return
        g_gameplayForbiddenKeyMask0.load() != 0 ||
        g_gameplayForbiddenKeyMask1.load() != 0 ||
        g_gameplayForbiddenKeyMask2.load() != 0 ||
        g_gameplayForbiddenKeyMask3.load() != 0 ||
        now < g_gameplayOriginalActionBlockedUntil.load();
}

bool GameplayInput_ShouldBlockOriginalAction(int actionType)
{
    if (g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (g_extendedEscapeDismissConsumed.load() ||
        GetTickCount64() <
            g_extendedEscapeDismissBlockedUntil.load())
    {
        return true;
    }
    if (GameplayWorldMap_ShouldBlockNativeInput())
    {
        return true;
    }
    if (actionType == 1016 &&
        CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        return true;
    }
    if (g_checkpointInteractionKeyConsumed.load() &&
        (actionType == 8 ||
         actionType == 26))
    {
        return true;
    }
    const bool dialogueKeyOwned =
        NpcDialogue_IsActive() ||
        NpcDialogue_IsTalkPromptVisible() ||
        NpcDialogue_IsTalkKeyReserved() ||
        (GameplayInput_ReadRawAsyncKeyState('T') & 0x8000) != 0;
    if (dialogueKeyOwned &&
        (actionType == 22 ||
         actionType == 23 ||
         actionType == 27 ||
         actionType == 28 ||
         (actionType >= 1018 && actionType <= 1021)))
    {
        return true;
    }
    if (NpcDialogue_IsActive() ||
        g_checkpointFailureUiActive.load() ||
        g_extendedPoseMenuVisible.load() ||
        g_gameplayInventoryOpen.load() ||
        g_gameplayWorldMapOpen.load() ||
        CheckpointSystem_IsTaskUiOpen() ||
        CheckpointSystem_IsMapEditorUiOpen())
    {
        return true;
    }
    return false;
}

bool ExtendedMode_ShouldBlockSave()
{
    return ExtendedMode_IsRunning();
}

Il2CppObject* ExtendedMode_GetStaticObject(
    const char* namespaceName,
    const char* className,
    const char* fieldName)
{
    if (il2cpp_class_get_field_from_name == nullptr ||
        il2cpp_field_static_get_value == nullptr)
    {
        return nullptr;
    }
    auto* klass = FindClass(namespaceName, className);
    if (klass == nullptr)
    {
        return nullptr;
    }
    auto* field = il2cpp_class_get_field_from_name(klass, fieldName);
    if (field == nullptr)
    {
        return nullptr;
    }
    Il2CppObject* value = nullptr;
    il2cpp_field_static_get_value(field, &value);
    return value;
}

void ExtendedMode_SetComponentEnabled(Il2CppObject* component, bool enabled)
{
    if (component == nullptr)
    {
        return;
    }
    void* parameters[] = { &enabled };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Behaviour"), "set_enabled", 1),
        component,
        parameters);
}

bool ExtendedMode_IsUnityObjectAlive(Il2CppObject* object)
{
    if (object == nullptr)
    {
        return false;
    }
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* inequality = FindMethod(objectClass, "op_Inequality", 2);
    Il2CppObject* nullObject = nullptr;
    void* parameters[] = { object, nullObject };
    auto* result = Invoke(inequality, nullptr, parameters);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

void GameplayInput_ClearOriginalAdultGoodsActions(
    Il2CppObject* inputData)
{
    if (inputData == nullptr)
    {
        return;
    }
    auto* flags = reinterpret_cast<unsigned char*>(inputData);
    // These are the original InputData fields for adult-goods actions.
    // Keep them clear so the shipped hotkeys cannot execute on their own.
    flags[0x3D] = 0; // JustEyeMask
    flags[0x3E] = 0; // JustSwitchVibrator
    flags[0x3F] = 0; // JustSwitchPistonMachine
    flags[0x40] = 0; // JustRingDildo
    flags[0x41] = 0; // JustRingHandcuff
    flags[0x42] = 0; // JustRingEyeMask
}

void GameplayInput_SuppressReservedNativeActions(
    Il2CppObject* inputData)
{
    if (inputData == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }

    auto* flags = reinterpret_cast<unsigned char*>(inputData);
    const bool qDown =
        (GameplayInput_ReadRawAsyncKeyState('Q') & 0x8000) != 0;
    const bool eDown =
        (GameplayInput_ReadRawAsyncKeyState('E') & 0x8000) != 0;
    const bool fDown =
        (GameplayInput_ReadRawAsyncKeyState('F') & 0x8000) != 0;
    const bool tDown =
        (GameplayInput_ReadRawAsyncKeyState('T') & 0x8000) != 0;
    const bool checkpointInteractionConsumed =
        g_checkpointInteractionKeyConsumed.load();

    // Q and E are mod-owned gameplay shortcuts. Unity Input System can still
    // expose their device state even when the Win32 keyboard hook consumes the
    // window message, so remove the resulting native action flags after the
    // shipped InputResolver has populated InputData.
    if (qDown)
    {
        flags[0x29] = 0; // JustOpenClothesA
        flags[0x2A] = 0; // JustCloseClothesA
        flags[0x2B] = 0; // JustOpenClothesB
        flags[0x2C] = 0; // JustCloseClothesB
    }
    if (eDown && !fDown)
    {
        flags[0x2F] = 0; // Interact
        flags[0x30] = 0; // JustInteract
        flags[0x43] = 0; // HoldCommonRingMenu
        flags[0x44] = 0; // HoldSkillRingMenu
    }
    if (checkpointInteractionConsumed)
    {
        flags[0x2F] = 0; // Interact
        flags[0x30] = 0; // JustInteract
        flags[0x43] = 0; // HoldCommonRingMenu
        flags[0x44] = 0; // HoldSkillRingMenu
    }
    if (tDown ||
        NpcDialogue_IsTalkKeyReserved() ||
        NpcDialogue_IsTalkPromptVisible() ||
        NpcDialogue_IsActive())
    {
        flags[0x36] = 0; // JustTimeStop
        flags[0x44] = 0; // HoldSkillRingMenu
        flags[0x48] = 0; // JustSlowSkill
    }
}

bool GameplayInput_CanUseHandForAdultGoods()
{
    auto* playerFacadeClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerFacade");
    auto* playerFacade = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerFacade",
        "Instance");
    auto* canUseHand = FindMethod(playerFacadeClass, "CanUseHand", 5);
    if (playerFacade == nullptr || canUseHand == nullptr)
    {
        return false;
    }
    bool sendMessage = true;
    bool skipHandcuff = false;
    bool skipBasket = false;
    bool skipPlayingAction = false;
    bool skipCrouch = false;
    void* parameters[] = {
        &sendMessage,
        &skipHandcuff,
        &skipBasket,
        &skipPlayingAction,
        &skipCrouch,
    };
    auto* result = Invoke(
        canUseHand,
        playerFacade,
        parameters);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

int GameplayInput_GetCurrentActionType()
{
    auto* playerClassAccessor =
        g_extendedPlayerController != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedPlayerController) +
                0x40)
            : nullptr;
    auto* playerState =
        playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) + 0x60)
            : nullptr;
    auto* currentAction =
        playerState != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerState) + 0x48)
            : nullptr;
    return currentAction != nullptr
        ? *reinterpret_cast<int*>(
            reinterpret_cast<char*>(currentAction) + 0x18)
        : -1;
}

bool ExtendedMode_ShouldPreserveModalPoseAnimation()
{
    if (!GameplayInput_IsCustomModalUiOpen())
    {
        return false;
    }
    const int actionType = GameplayInput_GetCurrentActionType();
    return actionType >= 50000 && actionType < 60000;
}

int GameplayInput_GetOriginalVibrationMode()
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    const int mode =
        gameStateData != nullptr
            ? *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1B8)
            : 0;
    return std::clamp(mode, 0, 3);
}

Il2CppObject* ExtendedMode_GetAnimationManager();

int GameplayAnimationDiagnostic_GetAnimatorActionId(
    Il2CppObject* animationManager)
{
    auto* animator =
        animationManager != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(animationManager) + 0x30)
            : nullptr;
    if (animator == nullptr)
    {
        return -1;
    }
    static const Il2CppMethod* getInteger = nullptr;
    if (getInteger == nullptr)
    {
        getInteger = FindMethod(
            FindClass("UnityEngine", "Animator"),
            "GetInteger",
            1);
    }
    constexpr int kActionIdHash = 1957839355;
    int actionId = -1;
    void* parameters[] = {
        const_cast<int*>(&kActionIdHash),
        &actionId,
    };
    auto* value = Invoke(getInteger, animator, parameters);
    return value != nullptr && il2cpp_object_unbox != nullptr
        ? *static_cast<int*>(il2cpp_object_unbox(value))
        : -1;
}

void GameplayAnimationDiagnostic_Log(
    const wchar_t* stage,
    Il2CppObject* animationManager,
    Il2CppObject* inputData,
    bool includeAnimatorStates)
{
    if (animationManager == nullptr)
    {
        LogDebug(
            LogCategory_Animation,
            L"ManagerSnapshot",
            L"AnimDiag[%s] PlayerAnimationManager=<null>.",
            stage);
        return;
    }

    const auto readFloat = [animationManager](int offset)
    {
        return *reinterpret_cast<float*>(
            reinterpret_cast<char*>(animationManager) + offset);
    };
    const auto readSmooth = [animationManager](
        int offset,
        bool target)
    {
        auto* smoothFloat = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(animationManager) + offset);
        return smoothFloat != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(smoothFloat) +
                (target ? 0x1C : 0x10))
            : 0.0f;
    };
    const auto readInputBool = [inputData](int offset)
    {
        return inputData != nullptr &&
            *reinterpret_cast<unsigned char*>(
                reinterpret_cast<char*>(inputData) + offset) != 0;
    };

    auto* animator = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(animationManager) + 0x30);
    const float moveX =
        inputData != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(inputData) + 0x10)
            : 0.0f;
    const float moveY =
        inputData != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(inputData) + 0x14)
            : 0.0f;
    const float moveZ =
        inputData != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(inputData) + 0x18)
            : 0.0f;
    const bool gamanBaibu =
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(animationManager) + 0x2C);
    const int actionType = GameplayInput_GetCurrentActionType();
    const int animatorActionId =
        GameplayAnimationDiagnostic_GetAnimatorActionId(
            animationManager);
    LogDebug(
        LogCategory_Animation,
        L"ManagerSnapshot",
        L"AnimDiag[%s] mgr=%p anim=%p actionType=%d oldActionId=%d animatorActionId=%d gaman=%d inputSwitch=%d originalMode=%d logicalMode=%d move=(%.2f,%.2f,%.2f) dash=%d targetMove=%.3f baseMove=%.3f arm=%.3f/%.3f rightArm=%.3f/%.3f moveBlend=%.3f/%.3f locSpeed=%.3f/%.3f another=%.3f/%.3f.",
        stage,
        animationManager,
        animator,
        actionType,
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(animationManager) + 0xC0),
        animatorActionId,
        gamanBaibu ? 1 : 0,
        readInputBool(0x3E) ? 1 : 0,
        GameplayInput_GetOriginalVibrationMode(),
        g_gameplayVibratorLogicalMode.load(),
        moveX,
        moveY,
        moveZ,
        readInputBool(0x26) ? 1 : 0,
        readFloat(0x10),
        readFloat(0x24),
        readSmooth(0x68, false),
        readSmooth(0x68, true),
        readSmooth(0x78, false),
        readSmooth(0x78, true),
        readSmooth(0xA0, false),
        readSmooth(0xA0, true),
        readSmooth(0xB8, false),
        readSmooth(0xB8, true),
        readSmooth(0xB0, false),
        readSmooth(0xB0, true));

    if (!includeAnimatorStates || animator == nullptr)
    {
        return;
    }
    static const Il2CppMethod* getCurrentState = nullptr;
    if (getCurrentState == nullptr)
    {
        getCurrentState = FindMethod(
            FindClass("UnityEngine", "Animator"),
            "GetCurrentAnimatorStateInfo",
            1);
    }
    const int layers[] = { 0, 1, 3, 7 };
    int stateHashes[4] = { -1, -1, -1, -1 };
    float normalizedTimes[4]{};
    float lengths[4]{};
    for (int index = 0; index < 4; ++index)
    {
        int layer = layers[index];
        void* parameters[] = { &layer };
        auto* stateObject = Invoke(
            getCurrentState,
            animator,
            parameters);
        if (stateObject == nullptr || il2cpp_object_unbox == nullptr)
        {
            continue;
        }
        const auto* state = static_cast<const int*>(
            il2cpp_object_unbox(stateObject));
        const auto* stateFloats =
            reinterpret_cast<const float*>(state);
        stateHashes[index] = state[0];
        normalizedTimes[index] = stateFloats[3];
        lengths[index] = stateFloats[4];
    }
    LogTrace(
        LogCategory_Animation,
        L"AnimatorLayers",
        L"AnimDiag[%s] state0=%d/%.3f/%.3f state1=%d/%.3f/%.3f state3=%d/%.3f/%.3f state7=%d/%.3f/%.3f.",
        stage,
        stateHashes[0],
        normalizedTimes[0],
        lengths[0],
        stateHashes[1],
        normalizedTimes[1],
        lengths[1],
        stateHashes[2],
        normalizedTimes[2],
        lengths[2],
        stateHashes[3],
        normalizedTimes[3],
        lengths[3]);
}

void GameplayInput_ScheduleEyeMaskModel()
{
    g_gameplayEyeMaskModelApplyAt =
        GetTickCount64() + 2000;
    LogInfo(LogCategory_Action, L"Action", L"Adult goods eye-mask model scheduled in 2000 ms.");
}

void GameplayInput_RequestConfiguredEyeMaskAction()
{
    const int frame = ExtendedMode_GetUnityFrameCount();
    static int lastRequestFrame = -1;
    if (frame >= 0 && frame == lastRequestFrame)
    {
        return;
    }
    lastRequestFrame = frame;
    g_gameplayConfiguredEyeMaskRequested = true;
    LogInfo(LogCategory_Action, L"Action", L"Configured EyeMask input queued.");
}

void GameplayInput_RequestEyeMaskForceThirdPerson()
{
    const int frame = ExtendedMode_GetUnityFrameCount();
    static int lastRequestFrame = -1;
    if (frame >= 0 && frame == lastRequestFrame)
    {
        return;
    }
    lastRequestFrame = frame;
    g_gameplayEyeMaskForceThirdPersonRequested = true;
    g_gameplayConfiguredEyeMaskRequested = true;
    EyeMaskVision_ForceThirdPersonAfterRemoval();
    LogInfo(LogCategory_Action, L"Action", 
        L"ViewChange during eye mask queued eye-mask removal and forced third-person restoration.");
}

void GameplayInput_TickEyeMaskModelAttachment()
{
    const unsigned long long applyAt =
        g_gameplayEyeMaskModelApplyAt.load();
    if (applyAt == 0 ||
        GetTickCount64() < applyAt)
    {
        return;
    }
    unsigned long long expected = applyAt;
    if (!g_gameplayEyeMaskModelApplyAt.compare_exchange_strong(
            expected,
            0))
    {
        return;
    }
    auto* gameStateFacadeClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameStateFacade");
    auto* isEquipped = FindMethod(
        gameStateFacadeClass,
        "IsEquipAdultGoods",
        1);
    int type = 2;
    void* parameters[] = { &type };
    auto* activeValue = Invoke(
        isEquipped,
        nullptr,
        parameters);
    const bool active =
        activeValue != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(
            il2cpp_object_unbox(activeValue));
    EquipmentPreviewUi_ApplyAdultGoodsAttachment(
        2,
        active);
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods eye-mask model rendered after delay (active=%d).",
        active ? 1 : 0);
}

int GameplayInput_ScheduleVibratorModel(
    int adultGoodsType)
{
    const int previousMode =
        GameplayInput_GetOriginalVibrationMode();
    const int nextMode = (previousMode + 1) % 4;
    g_gameplayVibratorLogicalMode = nextMode;
    g_gameplayVibratorModelType = adultGoodsType;
    g_gameplayVibratorPreviousMode = previousMode;
    g_gameplayVibratorNextMode = nextMode;
    g_gameplayVibratorModelApplyAt =
        GetTickCount64() + 2000;
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods vibrator model scheduled in 2000 ms (type=%d, mode=%d->%d).",
        adultGoodsType,
        previousMode,
        nextMode);
    return previousMode;
}

void GameplayInput_SetVibrationMode(int mode)
{
    auto* vibratorClass = FindClass(
        "ExposureUnnoticed2.Object3D.AdultGoods",
        "CommonVibratorController");
    auto* forceSetMode = FindMethod(
        vibratorClass,
        "ForceSetVibrationMode",
        1);
    if (forceSetMode == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Adult goods vibrator ForceSetVibrationMode is unavailable.");
        return;
    }
    int vibrationMode = mode;
    void* parameters[] = { &vibrationMode };
    Invoke(
        forceSetMode,
        nullptr,
        parameters);
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods vibrator original mode forced to %d.",
        mode);
}

void GameplayInput_SetAdultGoodsActiveState(
    int adultGoodsType,
    bool active)
{
    auto* gameStateFacadeClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameStateFacade");
    auto* updateState = FindMethod(
        gameStateFacadeClass,
        "UpdateAdultGoodsState",
        2);
    int type = adultGoodsType;
    bool state = active;
    if (updateState != nullptr)
    {
        void* stateParameters[] = { &type, &state };
        Invoke(
            updateState,
            nullptr,
            stateParameters);
    }

    auto* playerClassAccessor =
        g_extendedPlayerController != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedPlayerController) +
                0x40)
            : nullptr;
    auto* adultGoodsController =
        playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) +
                0x100)
            : nullptr;
    auto* adultGoodsControllerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAdultGoodsController");
    auto* changeAdultGoods = FindMethod(
        adultGoodsControllerClass,
        "ChangeAdultGoods",
        2);
    if (adultGoodsController != nullptr &&
        changeAdultGoods != nullptr)
    {
        void* changeParameters[] = { &type, &state };
        Invoke(
            changeAdultGoods,
            adultGoodsController,
            changeParameters);
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods type=%d original active state set to %d.",
        adultGoodsType,
        active ? 1 : 0);
}

Il2CppObject* GameplayInput_GetOriginalAdultGoodsDictionary()
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    return gameStateData != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(gameStateData) + 0x208)
        : nullptr;
}

bool GameplayInput_AdultGoodsDictionaryContains(
    Il2CppObject* dictionary,
    int adultGoodsType)
{
    if (dictionary == nullptr ||
        il2cpp_object_get_class == nullptr)
    {
        return false;
    }
    auto* dictionaryClass = il2cpp_object_get_class(dictionary);
    auto* containsKey = FindMethod(
        dictionaryClass,
        "ContainsKey",
        1);
    int type = adultGoodsType;
    void* parameters[] = { &type };
    auto* result = Invoke(
        containsKey,
        dictionary,
        parameters);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

void GameplayInput_RemoveAdultGoodsDictionaryEntry(
    Il2CppObject* dictionary,
    int adultGoodsType)
{
    if (dictionary == nullptr ||
        il2cpp_object_get_class == nullptr)
    {
        return;
    }
    auto* dictionaryClass = il2cpp_object_get_class(dictionary);
    auto* remove = FindMethod(
        dictionaryClass,
        "Remove",
        1);
    int type = adultGoodsType;
    void* parameters[] = { &type };
    Invoke(
        remove,
        dictionary,
        parameters);
}

void GameplayInput_SetOriginalVibrationMode(int mode)
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr)
    {
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1B8) =
            mode;
    }
}

void GameplayInput_CaptureVibratorSaveState()
{
    if (g_gameplayVibratorSaveStateCaptured)
    {
        return;
    }
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData == nullptr)
    {
        return;
    }
    const int types[3] = { 1, 6, 7 };
    g_gameplayOriginalVibrationMode =
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1B8);
    auto* dictionary =
        GameplayInput_GetOriginalAdultGoodsDictionary();
    auto* gameStateFacadeClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameStateFacade");
    auto* isEquipped = FindMethod(
        gameStateFacadeClass,
        "IsEquipAdultGoods",
        1);
    for (int index = 0; index < 3; ++index)
    {
        int type = types[index];
        void* parameters[] = { &type };
        auto* value = isEquipped != nullptr
            ? Invoke(isEquipped, nullptr, parameters)
            : nullptr;
        g_gameplayOriginalVibratorActive[index] =
            value != nullptr &&
            il2cpp_object_unbox != nullptr &&
            *static_cast<bool*>(
                il2cpp_object_unbox(value));
        g_gameplayOriginalVibratorPresent[index] =
            GameplayInput_AdultGoodsDictionaryContains(
                dictionary,
                type);
    }
    g_gameplayVibratorSaveStateCaptured = true;
    LogInfo(LogCategory_Action, L"Action", 
        L"Captured original vibrator save state: mode=%d, types=(%d/%d,%d/%d,%d/%d).",
        g_gameplayOriginalVibrationMode,
        g_gameplayOriginalVibratorPresent[0] ? 1 : 0,
        g_gameplayOriginalVibratorActive[0] ? 1 : 0,
        g_gameplayOriginalVibratorPresent[1] ? 1 : 0,
        g_gameplayOriginalVibratorActive[1] ? 1 : 0,
        g_gameplayOriginalVibratorPresent[2] ? 1 : 0,
        g_gameplayOriginalVibratorActive[2] ? 1 : 0);
}

void GameplayInput_RestoreVibratorSaveState()
{
    if (!g_gameplayVibratorSaveStateCaptured)
    {
        return;
    }
    const int types[3] = { 1, 6, 7 };
    auto* dictionary =
        GameplayInput_GetOriginalAdultGoodsDictionary();
    for (int index = 0; index < 3; ++index)
    {
        if (g_gameplayOriginalVibratorPresent[index])
        {
            GameplayInput_SetAdultGoodsActiveState(
                types[index],
                g_gameplayOriginalVibratorActive[index]);
        }
        else
        {
            GameplayInput_RemoveAdultGoodsDictionaryEntry(
                dictionary,
                types[index]);
        }
    }
    GameplayInput_SetOriginalVibrationMode(
        g_gameplayOriginalVibrationMode);
    g_gameplayVibratorSaveStateCaptured = false;
    LogInfo(LogCategory_Action, L"Action", L"Restored original vibrator save-state fields.");
}

void GameplayInput_UseAdultGoodsRecord(int adultGoodsType)
{
    auto* adultGoodsClass = FindClass(
        "ExposureUnnoticed2.Master.AdultGoods",
        "MAdultGoods");
    auto* getRecord = FindMethod(
        adultGoodsClass,
        "Get",
        1);
    auto* useItemProcess = FindMethod(
        adultGoodsClass,
        "UseItemProcess",
        1);
    int type = adultGoodsType;
    void* getParameters[] = { &type };
    auto* record = getRecord != nullptr
        ? Invoke(
            getRecord,
            nullptr,
            getParameters)
        : nullptr;
    if (record != nullptr &&
        useItemProcess != nullptr)
    {
        void* useParameters[] = { record };
        Invoke(
            useItemProcess,
            nullptr,
            useParameters);
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods type=%d original UseItemProcess dispatched.",
            adultGoodsType);
    }
}

void GameplayInput_TickVibratorModelAttachment()
{
    const unsigned long long applyAt =
        g_gameplayVibratorModelApplyAt.load();
    if (applyAt == 0 ||
        GetTickCount64() < applyAt)
    {
        return;
    }
    unsigned long long expected = applyAt;
    if (!g_gameplayVibratorModelApplyAt.compare_exchange_strong(
            expected,
            0))
    {
        return;
    }
    const int previousMode =
        g_gameplayVibratorPreviousMode.load();
    const int currentMode =
        g_gameplayVibratorNextMode.load();
    const int adultGoodsType =
        g_gameplayVibratorModelType.load();
    g_gameplayVibratorPreviousMode = -1;
    g_gameplayVibratorNextMode = -1;
    g_gameplayVibratorModelType = -1;

    const bool attach =
        previousMode == 0 && currentMode == 1;
    const bool detach =
        previousMode == 3 && currentMode == 0;
    if (adultGoodsType < 0 ||
        (!attach && !detach))
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods vibrator model unchanged (type=%d, mode=%d->%d).",
            adultGoodsType,
            previousMode,
            currentMode);
        return;
    }
    EquipmentPreviewUi_ApplyAdultGoodsAttachment(
        adultGoodsType,
        attach);
    if (attach)
    {
        GameplayInput_UseAdultGoodsRecord(adultGoodsType);
        GameplayInput_ApplyVibratorModeToOriginalState(
            adultGoodsType,
            1,
            false);
    }
    else
    {
        GameplayInput_SetAdultGoodsActiveState(
            adultGoodsType,
            false);
        GameplayInput_SetVibrationMode(0);
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods vibrator model rendered after delay (type=%d, mode=%d->%d, active=%d).",
        adultGoodsType,
        previousMode,
        currentMode,
        attach ? 1 : 0);
}

void GameplayInput_UseBoundHandcuff(int adultGoodsType)
{
    if (!GameplayInput_CanUseHandForAdultGoods())
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods handcuff type=%d skipped because PlayerFacade.CanUseHand returned false.",
            adultGoodsType);
        return;
    }
    auto* adultGoodsClass = FindClass(
        "ExposureUnnoticed2.Master.AdultGoods",
        "MAdultGoods");
    auto* getRecord = FindMethod(adultGoodsClass, "Get", 1);
    auto* useItemProcess = FindMethod(
        adultGoodsClass,
        "UseItemProcess",
        1);
    if (getRecord == nullptr || useItemProcess == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Adult goods handcuff methods could not be resolved.");
        return;
    }
    int type = adultGoodsType;
    void* getParameters[] = { &type };
    auto* record = Invoke(
        getRecord,
        nullptr,
        getParameters);
    if (record == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods handcuff record type=%d is unavailable.",
            adultGoodsType);
        return;
    }
    void* useParameters[] = { record };
    Invoke(
        useItemProcess,
        nullptr,
        useParameters);
    g_extendedHandcuffsPanelNextDiscoveryAt = 0;
    ExtendedMode_RefreshOriginalHandcuffsPanelActive();
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods handcuff type=%d dispatched through MAdultGoods.UseItemProcess.",
        adultGoodsType);
}

void GameplayInput_RegisterAdultGoodsWithOriginalState(
    int adultGoodsType)
{
    auto* gameStateFacadeClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameStateFacade");
    auto* addAdultGoods = FindMethod(
        gameStateFacadeClass,
        "AddAdultGoods",
        1);
    auto* isBringAdultGoods = FindMethodOverload(
        gameStateFacadeClass,
        "IsBringAdultGoods",
        1,
        1);
    int type = adultGoodsType;
    void* parameters[] = { &type };
    auto* isBringValue = isBringAdultGoods != nullptr
        ? Invoke(
            isBringAdultGoods,
            nullptr,
            parameters)
        : nullptr;
    const bool brought =
        isBringValue != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(
            il2cpp_object_unbox(isBringValue));
    if (!brought &&
        addAdultGoods != nullptr)
    {
        Invoke(
            addAdultGoods,
            nullptr,
            parameters);
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods type=%d registered with original GameState (brought=%d).",
        adultGoodsType,
        brought ? 1 : 0);
}

void GameplayInput_ApplyAdultQuickSlot(
    Il2CppObject* inputData)
{
    GameplayInput_ClearOriginalAdultGoodsActions(inputData);
    if (CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        g_gameplayConfiguredEyeMaskRequested = false;
        g_gameplayEyeMaskForceThirdPersonRequested = false;
    }
    const int quickSlot =
        g_gameplayAdultQuickSlotRequested.exchange(-1);
    if (inputData == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        g_gameplayConfiguredEyeMaskRequested = false;
        if (quickSlot < 0)
        {
            return;
        }
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods quick slot %d discarded outside gameplay.",
            quickSlot + 1);
        return;
    }
    auto* flags = reinterpret_cast<unsigned char*>(inputData);
    if (EyeMaskVision_IsEyeMaskActive())
    {
        flags[0x2D] = 0; // JustViewChange
    }
    if (g_gameplayConfiguredEyeMaskRequested.exchange(false))
    {
        const bool forceThirdPerson =
            g_gameplayEyeMaskForceThirdPersonRequested.exchange(false);
        if (forceThirdPerson)
        {
            flags[0x2D] = 0; // JustViewChange
        }
        flags[0x3D] = 1; // JustEyeMask
        GameplayInput_ScheduleEyeMaskModel();
        LogInfo(LogCategory_Action, L"Action", 
            L"Configured EyeMask input dispatched through the original action path (forceThirdPerson=%d).",
            forceThirdPerson ? 1 : 0);
    }
    if (quickSlot < 0)
    {
        return;
    }

    const int itemIndex =
        InventoryUi_FindQuickSlotBoundItem(quickSlot);
    if (itemIndex < 0)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods quick slot %d has no valid backpack or safe-box binding.",
            quickSlot + 1);
        return;
    }
    if (std::strcmp(
            kEquipmentInventoryDefinitions[
                g_inventoryItems[itemIndex].definition].id,
            "217-adult-011-mitemdildo") == 0)
    {
        if (DildoInventory_TryUseBoundItem(itemIndex))
        {
            flags[0x40] = 1; // JustRingDildo
        }
        return;
    }
    const int adultGoodsType =
        InventoryUi_GetAdultGoodsType(
            g_inventoryItems[itemIndex].definition);
    const int group =
        InventoryUi_GetAdultGoodsGroup(adultGoodsType);
    GameplayInput_RegisterAdultGoodsWithOriginalState(
        adultGoodsType);
    switch (group)
    {
    case 1:
    {
        if (GameplayInput_GetCurrentActionType() == 10024)
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Adult goods vibrator input ignored while action 10024 is still active.");
            return;
        }
        if (!GameplayInput_CanUseHandForAdultGoods())
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Adult goods vibrator input rejected by PlayerFacade.CanUseHand.");
            return;
        }
        // MOD inventory is authoritative. Mirror the selected vibrator into
        // the shipped GameState before its one-shot input is processed so the
        // original save cannot emit a false "vibrator not equipped" warning.
        GameplayInput_SetAdultGoodsActiveState(
            adultGoodsType,
            true);
        LogInfo(
            LogCategory_Action,
            L"Action",
            L"MOD vibrator type=%d synchronized to original equipment "
            L"state before action validation.",
            adultGoodsType);
        GameplayInput_ScheduleVibratorModel(adultGoodsType);
        // Reuse the shipped PlayerAction.OnUpdate path. It validates hand
        // availability, starts SwitchVibrator through TransAction, advances
        // PlayerVibrationModeType from the animation event, and clears the
        // action after the one-shot animation ends.
        GameplayAnimationDiagnostic_Log(
            L"vibrator-input-before",
            ExtendedMode_GetAnimationManager(),
            inputData,
            true);
        flags[0x3E] = 1;
        GameplayAnimationDiagnostic_Log(
            L"vibrator-input-after",
            ExtendedMode_GetAnimationManager(),
            inputData,
            true);
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods vibrator queued original JustSwitchVibrator input.");
        break;
    }
    case 2:
        flags[0x3F] = 1;
        break;
    case 3:
        flags[0x3D] = 1;
        GameplayInput_ScheduleEyeMaskModel();
        break;
    case 4:
        GameplayInput_UseBoundHandcuff(adultGoodsType);
        break;
    default:
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods quick slot %d has unsupported type=%d.",
            quickSlot + 1,
            adultGoodsType);
        return;
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods quick slot %d dispatched type=%d group=%d.",
        quickSlot + 1,
        adultGoodsType,
        group);
}

void ExtendedMode_DestroyUnityObject(Il2CppObject*& object)
{
    if (object == nullptr)
    {
        return;
    }
    if (!ExtendedMode_IsUnityObjectAlive(object))
    {
        object = nullptr;
        return;
    }
    SetWorldGameObjectActive(object, false);
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* destroy = objectClass != nullptr
        ? FindMethod(objectClass, "Destroy", 1)
        : nullptr;
    void* parameters[] = { object };
    if (destroy != nullptr)
    {
        Invoke(destroy, nullptr, parameters);
    }
    object = nullptr;
}

void ExtendedMode_DestroyRuntimeWindows(const wchar_t* reason)
{
    // These roots are either scene-owned or explicitly DontDestroyOnLoad.
    // Destroy both before any scene load so a stale native pointer can never
    // suppress the next BuildUi/BuildGameplayHud call.
    GameplayActionPursuit_Reset();
    if (EquipmentPreviewUi_IsOpen())
    {
        EquipmentPreviewUi_Close();
    }
    NpcDialogue_ResetForSceneChange();
    InventoryUi_PrepareForSceneTransition();
    CheckpointSystem_ClearSceneObjects();
    GameplayStatusHud_Reset();
    EquipmentStats_ResetSceneRuntimeBindings();
    GameplayMinimap_Reset();
    CheckpointCompass_Reset();
    ExtendedMode_DestroyUnityObject(g_extendedGameplayHudCanvasObject);
    ExtendedMode_DestroyUnityObject(g_extendedCanvas);
    g_extendedCoordinateLabel = nullptr;
    g_extendedInteractionPromptPanel = nullptr;
    g_extendedInteractionPromptActionLabel = nullptr;
    g_extendedInteractionPromptKeyLabel = nullptr;
    g_extendedInteractionPromptKeyBadge = SceneUiElement{};
    g_extendedInteractionPromptStates[0] =
        ExtendedInteractionPromptState{};
    g_extendedInteractionPromptStates[1] =
        ExtendedInteractionPromptState{};
    g_extendedCoatModeButton = SceneUiElement{};
    g_extendedActionPanel = nullptr;
    g_extendedSelectedActionLabel = nullptr;
    g_extendedPageLabel = nullptr;
    g_extendedPrevPageButton = nullptr;
    g_extendedNextPageButton = nullptr;
    g_extendedActionButtons.clear();
    g_extendedActionLabels.clear();
    g_extendedEnterGameButton = SceneUiElement{};
    g_extendedSceneSelectorPanel = SceneUiElement{};
    for (int index = 0; index < kEscapeFromManakaStageCount; ++index)
    {
        g_extendedSceneButtons[index] = SceneUiElement{};
    }
    g_extendedDayButton = SceneUiElement{};
    g_extendedNightButton = SceneUiElement{};
    g_extendedConfirmEnterButton = SceneUiElement{};
    g_extendedCancelEnterButton = SceneUiElement{};
    g_extendedSceneSelectionHint = nullptr;
    g_gameplayInventoryOpen = false;
    LogInfo(LogCategory_Action, L"Action", L"Escape destroyed custom UI before %s.", reason);
}

constexpr wchar_t kExtendedLoadingWindowClass[] =
    L"EscapeFromManakaLoadingWindow";

LRESULT CALLBACK ExtendedMode_LoadingWindowProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        return 1;
    case WM_TIMER:
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT paint{};
        HDC device = BeginPaint(window, &paint);
        RECT client{};
        GetClientRect(window, &client);
        HBRUSH background = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(device, &client, background);
        DeleteObject(background);

        const unsigned int dots =
            1 + static_cast<unsigned int>((GetTickCount64() / 300) % 3);
        wchar_t loadingText[32] = L"NOW LOADING";
        size_t length = wcslen(loadingText);
        for (unsigned int index = 0; index < dots; ++index)
        {
            loadingText[length++] = L'.';
        }
        loadingText[length] = L'\0';

        HDC screen = GetDC(nullptr);
        const int dpi = screen != nullptr
            ? GetDeviceCaps(screen, LOGPIXELSY)
            : 96;
        if (screen != nullptr)
        {
            ReleaseDC(nullptr, screen);
        }
        const int fontSize = -MulDiv(17, dpi > 0 ? dpi : 96, 96);
        HFONT font = CreateFontW(
            fontSize,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI");
        HGDIOBJ previousFont = font != nullptr
            ? SelectObject(device, font)
            : nullptr;
        SetBkMode(device, TRANSPARENT);
        SetTextColor(device, RGB(224, 224, 224));
        const int margin = MulDiv(24, dpi > 0 ? dpi : 96, 96);
        RECT textRect{
            margin,
            margin,
            client.right - margin,
            client.bottom - margin,
        };
        DrawTextW(
            device,
            loadingText,
            static_cast<int>(length),
            &textRect,
            DT_RIGHT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX);
        if (previousFont != nullptr)
        {
            SelectObject(device, previousFont);
        }
        if (font != nullptr)
        {
            DeleteObject(font);
        }
        EndPaint(window, &paint);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}

bool ExtendedMode_EnsureLoadingWindowClass()
{
    if (g_extendedLoadingWindowClassRegistered)
    {
        return true;
    }
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = ExtendedMode_LoadingWindowProc;
    windowClass.hInstance = g_instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground =
        reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    windowClass.lpszClassName = kExtendedLoadingWindowClass;
    const ATOM atom = RegisterClassExW(&windowClass);
    g_extendedLoadingWindowClassRegistered =
        atom != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    return g_extendedLoadingWindowClassRegistered;
}

void ExtendedMode_DestroyTransferLoadingOverlay()
{
    if (g_extendedTransferLoadingWindow != nullptr &&
        IsWindow(g_extendedTransferLoadingWindow))
    {
        DestroyWindow(g_extendedTransferLoadingWindow);
    }
    g_extendedTransferLoadingWindow = nullptr;
    g_extendedTransferLoadingOverlayActive = false;
}

void ExtendedMode_ShowTransferLoadingOverlay()
{
    const HWND gameWindow = FindGameWindow();
    if (gameWindow == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Transfer loading cover could not find the game window.");
        return;
    }
    RECT client{};
    if (!GetClientRect(gameWindow, &client))
    {
        return;
    }
    POINT origin{ 0, 0 };
    if (!ClientToScreen(gameWindow, &origin))
    {
        return;
    }
    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    if (width <= 0 || height <= 0)
    {
        return;
    }
    if (g_extendedTransferLoadingWindow == nullptr ||
        !IsWindow(g_extendedTransferLoadingWindow))
    {
        if (!ExtendedMode_EnsureLoadingWindowClass())
        {
            LogInfo(LogCategory_Action, L"Action", L"Loading page window class registration failed.");
            return;
        }
        g_extendedTransferLoadingWindow = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            kExtendedLoadingWindowClass,
            L"",
            WS_POPUP,
            origin.x,
            origin.y,
            width,
            height,
            gameWindow,
            nullptr,
            g_instance,
            nullptr);
        if (g_extendedTransferLoadingWindow != nullptr)
        {
            SetTimer(
                g_extendedTransferLoadingWindow,
                1,
                250,
                nullptr);
        }
    }
    if (g_extendedTransferLoadingWindow == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Transfer loading cover window creation failed.");
        return;
    }
    SetWindowPos(
        g_extendedTransferLoadingWindow,
        HWND_TOPMOST,
        origin.x,
        origin.y,
        width,
        height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
    ShowWindow(
        g_extendedTransferLoadingWindow,
        SW_SHOWNOACTIVATE);
    InvalidateRect(g_extendedTransferLoadingWindow, nullptr, TRUE);
    UpdateWindow(g_extendedTransferLoadingWindow);
    g_extendedTransferLoadingOverlayActive = true;
    LogInfo(LogCategory_Action, L"Action", L"Game-style NOW LOADING cover enabled.");
}

void ExtendedMode_HideTransferLoadingOverlay()
{
    if (!g_extendedTransferLoadingOverlayActive)
    {
        return;
    }
    if (g_extendedTransferLoadingWindow != nullptr &&
        IsWindow(g_extendedTransferLoadingWindow))
    {
        ShowWindow(g_extendedTransferLoadingWindow, SW_HIDE);
    }
    g_extendedTransferLoadingOverlayActive = false;
    LogInfo(LogCategory_Action, L"Action", L"Transfer loading cover disabled after target spawn.");
}

void ExtendedMode_SetTransformPosition(
    Il2CppObject* transform,
    float x,
    float y,
    float z)
{
    if (transform == nullptr)
    {
        return;
    }
    float value[] = { x, y, z };
    void* parameters[] = { value };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "set_position", 1),
        transform,
        parameters);
}

void ExtendedMode_SetTransformLocalScale(
    Il2CppObject* transform,
    float x,
    float y,
    float z)
{
    if (transform == nullptr)
    {
        return;
    }
    float value[] = { x, y, z };
    void* parameters[] = { value };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "set_localScale", 1),
        transform,
        parameters);
}

void ExtendedMode_SetObjectName(Il2CppObject* object, const char* name)
{
    if (object == nullptr)
    {
        return;
    }
    auto* nameValue = il2cpp_string_new(name);
    void* parameters[] = { nameValue };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Object"), "set_name", 1),
        object,
        parameters);
}

void ExtendedMode_SetGameObjectLayer(Il2CppObject* gameObject, int layer)
{
    if (gameObject == nullptr)
    {
        return;
    }
    void* parameters[] = { &layer };
    Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "set_layer", 1),
        gameObject,
        parameters);
}

void ExtendedMode_SetHierarchyLayer(
    Il2CppObject* gameObject,
    int layer,
    int depth = 0)
{
    if (gameObject == nullptr || depth > 16)
    {
        return;
    }
    ExtendedMode_SetGameObjectLayer(gameObject, layer);

    auto* transform = GetWorldTransform(gameObject);
    if (transform == nullptr)
    {
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* childCountValue = Invoke(
        FindMethod(transformClass, "get_childCount", 0),
        transform,
        nullptr);
    if (childCountValue == nullptr)
    {
        return;
    }
    const int childCount = *static_cast<int*>(
        il2cpp_object_unbox(childCountValue));
    for (int index = 0; index < childCount; ++index)
    {
        void* parameters[] = { &index };
        auto* child = Invoke(
            FindMethod(transformClass, "GetChild", 1),
            transform,
            parameters);
        auto* childGameObject = child != nullptr
            ? Invoke(
                FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
                child,
                nullptr)
            : nullptr;
        ExtendedMode_SetHierarchyLayer(childGameObject, layer, depth + 1);
    }
}

int ExtendedMode_GetGameObjectLayer(Il2CppObject* gameObject)
{
    if (gameObject == nullptr)
    {
        return 0;
    }
    auto* value = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_layer", 0),
        gameObject,
        nullptr);
    return value != nullptr && il2cpp_object_unbox != nullptr
        ? *static_cast<int*>(il2cpp_object_unbox(value))
        : 0;
}

void ExtendedMode_CaptureHierarchyLayers(
    Il2CppObject* gameObject,
    int depth = 0)
{
    if (gameObject == nullptr || depth > 16)
    {
        return;
    }
    g_extendedLayerSnapshots.push_back({
        gameObject,
        ExtendedMode_GetGameObjectLayer(gameObject)});

    auto* transform = GetWorldTransform(gameObject);
    if (transform == nullptr)
    {
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* childCountValue = Invoke(
        FindMethod(transformClass, "get_childCount", 0),
        transform,
        nullptr);
    if (childCountValue == nullptr)
    {
        return;
    }
    const int childCount = *static_cast<int*>(
        il2cpp_object_unbox(childCountValue));
    for (int index = 0; index < childCount; ++index)
    {
        void* parameters[] = { &index };
        auto* child = Invoke(
            FindMethod(transformClass, "GetChild", 1),
            transform,
            parameters);
        auto* childGameObject = child != nullptr
            ? Invoke(
                FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
                child,
                nullptr)
            : nullptr;
        ExtendedMode_CaptureHierarchyLayers(childGameObject, depth + 1);
    }
}

void ExtendedMode_RestoreHierarchyLayers()
{
    for (const auto& snapshot : g_extendedLayerSnapshots)
    {
        ExtendedMode_SetGameObjectLayer(snapshot.gameObject, snapshot.layer);
    }
    g_extendedLayerSnapshots.clear();
}

void ExtendedMode_ForceCursor()
{
    auto* cursorClass = FindClass("UnityEngine", "Cursor");
    int lockState = 0;
    bool visible = true;
    void* lockParameters[] = { &lockState };
    void* visibleParameters[] = { &visible };
    Invoke(
        FindMethod(cursorClass, "set_lockState", 1),
        nullptr,
        lockParameters);
    Invoke(
        FindMethod(cursorClass, "set_visible", 1),
        nullptr,
        visibleParameters);
    auto* cursorControllerClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "CursorController");
    Invoke(
        FindMethod(cursorControllerClass, "SetVisible", 1),
        nullptr,
        visibleParameters);
}

void CursorSetLockStateHook(
    int lockState,
    const Il2CppMethod* method)
{
    if ((NpcDialogue_IsActive() ||
         ModGuideUi_IsOpen() ||
         g_extendedOriginalTutorialActive.load() ||
         g_extendedPoseMenuVisible.load() ||
         g_gameplayInventoryOpen.load() ||
         g_gameplayWorldMapOpen.load() ||
         ExtendedMode_IsOriginalHandcuffsPanelActive() ||
         DildoInventory_IsPanelActive() ||
         CheckpointSystem_IsMapEditorUiOpen()) &&
        g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        lockState = 0; // CursorLockMode.None
    }
    if (g_originalCursorSetLockState != nullptr)
    {
        using OriginalFn = void (*)(int, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalCursorSetLockState)(lockState, method);
    }
}

void CursorSetVisibleHook(
    bool visible,
    const Il2CppMethod* method)
{
    if ((NpcDialogue_IsActive() ||
         ModGuideUi_IsOpen() ||
         g_extendedOriginalTutorialActive.load() ||
         g_extendedPoseMenuVisible.load() ||
         g_gameplayInventoryOpen.load() ||
         g_gameplayWorldMapOpen.load() ||
         ExtendedMode_IsOriginalHandcuffsPanelActive() ||
         DildoInventory_IsPanelActive() ||
         CheckpointSystem_IsMapEditorUiOpen()) &&
        g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        visible = true;
    }
    if (g_originalCursorSetVisible != nullptr)
    {
        using OriginalFn = void (*)(bool, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalCursorSetVisible)(visible, method);
    }
}

BOOL WINAPI SetCursorPosHook(int x, int y)
{
    if ((NpcDialogue_IsActive() ||
         ModGuideUi_IsOpen() ||
         g_extendedOriginalTutorialActive.load() ||
         g_extendedPoseMenuVisible.load() ||
         g_gameplayInventoryOpen.load() ||
         g_gameplayWorldMapOpen.load() ||
         ExtendedMode_IsOriginalHandcuffsPanelActive() ||
         DildoInventory_IsPanelActive() ||
         CheckpointSystem_IsMapEditorUiOpen()) &&
        g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        return TRUE;
    }
    if (g_originalSetCursorPos != nullptr)
    {
        using OriginalFn = BOOL (WINAPI*)(int, int);
        return reinterpret_cast<OriginalFn>(
            g_originalSetCursorPos)(x, y);
    }
    return FALSE;
}

BOOL WINAPI ClipCursorHook(const RECT* rectangle)
{
    if ((NpcDialogue_IsActive() ||
         ModGuideUi_IsOpen() ||
         g_extendedOriginalTutorialActive.load() ||
         g_extendedPoseMenuVisible.load() ||
         g_gameplayInventoryOpen.load() ||
         g_gameplayWorldMapOpen.load() ||
         ExtendedMode_IsOriginalHandcuffsPanelActive() ||
         DildoInventory_IsPanelActive() ||
         CheckpointSystem_IsMapEditorUiOpen()) &&
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        rectangle != nullptr)
    {
        return TRUE;
    }
    if (g_originalClipCursor != nullptr)
    {
        using OriginalFn = BOOL (WINAPI*)(const RECT*);
        return reinterpret_cast<OriginalFn>(
            g_originalClipCursor)(rectangle);
    }
    return FALSE;
}

void ExtendedMode_ForceGameplayCursor()
{
    auto* cursorClass = FindClass("UnityEngine", "Cursor");
    if (cursorClass == nullptr)
    {
        return;
    }
    int lockState = 1; // CursorLockMode.Locked
    bool visible = false;
    void* lockParameters[] = { &lockState };
    void* visibleParameters[] = { &visible };
    Invoke(
        FindMethod(cursorClass, "set_lockState", 1),
        nullptr,
        lockParameters);
    Invoke(
        FindMethod(cursorClass, "set_visible", 1),
        nullptr,
        visibleParameters);
    auto* cursorControllerClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "CursorController");
    if (cursorControllerClass != nullptr)
    {
        Invoke(
            FindMethod(cursorControllerClass, "SetVisible", 1),
            nullptr,
            visibleParameters);
    }
}

void ExtendedMode_SetGameplayHudRaycastsEnabled(bool enabled)
{
    auto* raycaster = GetSceneComponentByClass(
        g_extendedGameplayHudCanvasObject,
        "UnityEngine.UI",
        "GraphicRaycaster");
    if (raycaster != nullptr)
    {
        ExtendedMode_SetComponentEnabled(raycaster, enabled);
    }
}

void GameplayInput_RestoreDisabledActions()
{
    auto* actionClass = FindClass(
        "UnityEngine.InputSystem",
        "InputAction");
    auto* enableMethod = FindMethod(
        actionClass,
        "Enable",
        0);
    if (enableMethod != nullptr)
    {
        for (auto* action : g_gameplayDisabledInputActions)
        {
            if (action != nullptr)
            {
                Invoke(enableMethod, action, nullptr);
            }
        }
    }
    unsigned int restored =
        static_cast<unsigned int>(
            g_gameplayDisabledInputActions.size());
    g_gameplayDisabledInputActions.clear();
    LogInfo(LogCategory_Action, L"Action", 
        L"Gameplay restored tracked original InputActions (count=%u).",
        restored);
}

void GameplayInput_RestoreMapEditorEyeMaskInputAction()
{
    if (g_mapEditorBlockedEyeMaskInputAction == nullptr)
    {
        return;
    }
    auto* actionClass = FindClass(
        "UnityEngine.InputSystem",
        "InputAction");
    auto* enable = FindMethod(actionClass, "Enable", 0);
    if (enable != nullptr &&
        ExtendedMode_IsUnityObjectAlive(
            g_mapEditorBlockedEyeMaskInputAction))
    {
        Invoke(
            enable,
            g_mapEditorBlockedEyeMaskInputAction,
            nullptr);
    }
    g_mapEditorBlockedEyeMaskInputAction = nullptr;
    LogInfo(LogCategory_Action, L"Action", L"Map editor restored the original EyeMask InputAction.");
}

void ExtendedMode_ClearGameplayInputMode()
{
    GameplayInput_RestoreMapEditorEyeMaskInputAction();
    GameplayInput_RestoreDisabledActions();
    GameplayInput_ClearForbiddenKeyState();
    g_extendedSuppressPlayerUpdate = false;
    g_extendedForwardKeyDown = false;
    g_extendedBackwardKeyDown = false;
    g_extendedLeftKeyDown = false;
    g_extendedRightKeyDown = false;
    g_extendedForwardKeyPulse = false;
    g_extendedBackwardKeyPulse = false;
    g_extendedLeftKeyPulse = false;
    g_extendedRightKeyPulse = false;
    ExtendedMode_ForceGameplayCursor();
    LogInfo(LogCategory_Action, L"Action", L"Gameplay input mode restored after closing the raid inventory.");
}

void ExtendedMode_SetAnimatorBool(const char* name, bool value)
{
    if (g_previewAnimator == nullptr || name == nullptr)
    {
        return;
    }
    auto* key = il2cpp_string_new(name);
    void* parameters[] = { key, &value };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Animator"), "SetBool", 2),
        g_previewAnimator,
        parameters);
}

void ExtendedMode_SetAnimatorFloat(const char* name, float value)
{
    if (g_previewAnimator == nullptr || name == nullptr)
    {
        return;
    }
    auto* key = il2cpp_string_new(name);
    void* parameters[] = { key, &value };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Animator"), "SetFloat", 2),
        g_previewAnimator,
        parameters);
}

bool ExtendedMode_GetAnimatorBool(const char* name)
{
    if (g_previewAnimator == nullptr || name == nullptr)
    {
        return false;
    }
    auto* key = il2cpp_string_new(name);
    void* parameters[] = { key };
    auto* result = Invoke(
        FindMethod(FindClass("UnityEngine", "Animator"), "GetBool", 1),
        g_previewAnimator,
        parameters);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

float ExtendedMode_GetAnimatorFloat(const char* name)
{
    if (g_previewAnimator == nullptr || name == nullptr)
    {
        return 0.0f;
    }
    auto* key = il2cpp_string_new(name);
    void* parameters[] = { key };
    auto* result = Invoke(
        FindMethod(FindClass("UnityEngine", "Animator"), "GetFloat", 1),
        g_previewAnimator,
        parameters);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr
        ? *static_cast<float*>(il2cpp_object_unbox(result))
        : 0.0f;
}

void ExtendedMode_CaptureAnimatorState()
{
    if (g_previewAnimator == nullptr)
    {
        return;
    }
    if (g_extendedAnimatorStateSnapshot.animator == g_previewAnimator)
    {
        // A null GetLayerWeight method is cached by the animator pointer too,
        // so a failed snapshot does not log on every gameplay frame.
        return;
    }

    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* getLayerWeight = FindMethod(animatorClass, "GetLayerWeight", 1);
    if (getLayerWeight == nullptr)
    {
        g_extendedAnimatorStateSnapshot =
            ExtendedAnimatorStateSnapshot{};
        g_extendedAnimatorStateSnapshot.animator = g_previewAnimator;
        LogInfo(LogCategory_Action, L"Action", L"Animator.GetLayerWeight is unavailable; using the gameplay fallback pose restore.");
        return;
    }

    ExtendedAnimatorStateSnapshot snapshot;
    snapshot.animator = g_previewAnimator;
    for (int layer = 0; layer < 12; ++layer)
    {
        void* parameters[] = { &layer };
        auto* value = Invoke(
            getLayerWeight,
            g_previewAnimator,
            parameters);
        snapshot.layerWeights[layer] =
            value != nullptr && il2cpp_object_unbox != nullptr
                ? *static_cast<float*>(il2cpp_object_unbox(value))
                : 0.0f;
    }
    snapshot.moveSpeed = ExtendedMode_GetAnimatorFloat("MoveSpeed");
    snapshot.strafeX = ExtendedMode_GetAnimatorFloat("StrafeX");
    snapshot.strafeY = ExtendedMode_GetAnimatorFloat("StrafeY");
    snapshot.isStrafe = ExtendedMode_GetAnimatorBool("IsStrafe");
    snapshot.isCrouch = ExtendedMode_GetAnimatorBool("IsCrouch");
    snapshot.valid = true;
    g_extendedAnimatorStateSnapshot = snapshot;
    LogInfo(LogCategory_Action, L"Action", 
        L"Captured gameplay Animator state: layer0=%.2f MoveSpeed=%.2f strafe=%d crouch=%d.",
        snapshot.layerWeights[0],
        snapshot.moveSpeed,
        snapshot.isStrafe ? 1 : 0,
        snapshot.isCrouch ? 1 : 0);
}

void ExtendedMode_PrepareOriginalAnimator()
{
    if (g_previewAnimator == nullptr)
    {
        return;
    }
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    ExtendedMode_SetComponentEnabled(g_previewAnimator, true);
    bool applyRootMotion = false;
    int updateMode = 0;
    int cullingMode = 0;
    float speed = 1.0f;
    void* rootMotionParameters[] = { &applyRootMotion };
    void* updateModeParameters[] = { &updateMode };
    void* cullingParameters[] = { &cullingMode };
    void* speedParameters[] = { &speed };
    Invoke(
        FindMethod(animatorClass, "set_applyRootMotion", 1),
        g_previewAnimator,
        rootMotionParameters);
    Invoke(
        FindMethod(animatorClass, "set_updateMode", 1),
        g_previewAnimator,
        updateModeParameters);
    Invoke(
        FindMethod(animatorClass, "set_cullingMode", 1),
        g_previewAnimator,
        cullingParameters);
    Invoke(
        FindMethod(animatorClass, "set_speed", 1),
        g_previewAnimator,
        speedParameters);

    // These are the two parameters visible in the shipped Player controller.
    // Neutralising them prevents a saved walking/crouching state from
    // colouring the first full-body action.
    ExtendedMode_SetAnimatorBool("IsStrafe", false);
    ExtendedMode_SetAnimatorBool("IsCrouch", false);
    ExtendedMode_SetAnimatorFloat("MoveSpeed", 0.0f);
    ExtendedMode_SetAnimatorFloat("StrafeX", 0.0f);
    ExtendedMode_SetAnimatorFloat("StrafeY", 0.0f);
}

void ExtendedMode_ApplyGameplayAnimationLayerBaseline()
{
    if (g_previewAnimator == nullptr)
    {
        return;
    }
    GameplayAnimationDiagnostic_Log(
        L"layer-baseline-before",
        ExtendedMode_GetAnimationManager(),
        nullptr,
        true);
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* setLayerWeight = FindMethod(
        animatorClass,
        "SetLayerWeight",
        2);
    if (setLayerWeight == nullptr)
    {
        return;
    }

    // The warehouse action browser zeroes every layer except the selected
    // preview layer. Gameplay needs the full native action stack, while the
    // naked mode keeps only ClothActionArm (layer 3) hidden. SwitchVibrator
    // lives on OverrideRightArmHand (layer 7), not on ClothActionArm.
    constexpr float kGameplayLayerWeights[12] = {
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };
    for (int layer = 0; layer < 12; ++layer)
    {
        float weight = kGameplayLayerWeights[layer];
        void* parameters[] = { &layer, &weight };
        Invoke(setLayerWeight, g_previewAnimator, parameters);
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Applied gameplay animation layer baseline (OverrideRightArmHand=1, ClothActionArm=0).");
    GameplayAnimationDiagnostic_Log(
        L"layer-baseline-after",
        ExtendedMode_GetAnimationManager(),
        nullptr,
        true);
}

void ExtendedMode_RestoreAnimatorForGameplay()
{
    if (g_previewAnimator == nullptr)
    {
        return;
    }
    auto* animatorClass = FindClass("UnityEngine", "Animator");

    const bool hasSnapshot =
        g_extendedAnimatorStateSnapshot.valid &&
        g_extendedAnimatorStateSnapshot.animator == g_previewAnimator;
    if (hasSnapshot)
    {
        auto* setLayerWeight = FindMethod(
            animatorClass,
            "SetLayerWeight",
            2);
        for (int layer = 0; layer < 12; ++layer)
        {
            float weight =
                g_extendedAnimatorStateSnapshot.layerWeights[layer];
            void* parameters[] = { &layer, &weight };
            Invoke(setLayerWeight, g_previewAnimator, parameters);
        }
        ExtendedMode_SetAnimatorFloat(
            "MoveSpeed",
            g_extendedAnimatorStateSnapshot.moveSpeed);
        ExtendedMode_SetAnimatorFloat(
            "StrafeX",
            g_extendedAnimatorStateSnapshot.strafeX);
        ExtendedMode_SetAnimatorFloat(
            "StrafeY",
            g_extendedAnimatorStateSnapshot.strafeY);
        ExtendedMode_SetAnimatorBool(
            "IsStrafe",
            g_extendedAnimatorStateSnapshot.isStrafe);
        ExtendedMode_SetAnimatorBool(
            "IsCrouch",
            g_extendedAnimatorStateSnapshot.isCrouch);
    }
    else
    {
        ExtendedMode_PrepareOriginalAnimator();

        int locomotionLayer = 0;
        float layerWeight = 1.0f;
        void* weightParameters[] = { &locomotionLayer, &layerWeight };
        Invoke(
            FindMethod(animatorClass, "SetLayerWeight", 2),
            g_previewAnimator,
            weightParameters);

        // The warehouse action browser forced every non-selected layer to 0.
        // Unlike Locomotion, the shipped Player controller expects its Action
        // layer to remain enabled while idle. Restore it before gameplay so
        // chair/vending interaction states can transition normally.
        int actionLayer = 1;
        float actionLayerWeight = 1.0f;
        void* actionWeightParameters[] = {
            &actionLayer,
            &actionLayerWeight,
        };
        Invoke(
            FindMethod(animatorClass, "SetLayerWeight", 2),
            g_previewAnimator,
            actionWeightParameters);

        auto* locomotionState =
            il2cpp_string_new("Locomotion.LocomotionTree");
        float normalizedTime = 0.0f;
        void* playParameters[] = {
            locomotionState,
            &locomotionLayer,
            &normalizedTime,
        };
        Invoke(
            FindMethod(animatorClass, "Play", 3),
            g_previewAnimator,
            playParameters);
    }

    ExtendedMode_ApplyGameplayAnimationLayerBaseline();

    constexpr int kActionIdHash = 1957839355;
    int actionId = -1;
    void* actionParameters[] = {
        const_cast<int*>(&kActionIdHash),
        &actionId,
    };
    Invoke(
        FindMethod(animatorClass, "SetInteger", 2),
        g_previewAnimator,
        actionParameters);
    auto* transitionTrigger = il2cpp_string_new("TransAction");
    void* triggerParameters[] = { transitionTrigger };
    Invoke(
        FindMethod(animatorClass, "ResetTrigger", 1),
        g_previewAnimator,
        triggerParameters);
}

void ExtendedMode_ReinitializeAnimationManager()
{
    auto* playerClassAccessor = g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
    auto* animationManager = playerClassAccessor != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x48)
        : nullptr;
    auto* animationManagerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationManager");
    auto* initialize = FindMethod(
        animationManagerClass,
        "Initialize",
        1);
    if (playerClassAccessor == nullptr ||
        animationManager == nullptr ||
        initialize == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Could not reinitialize PlayerAnimationManager after gameplay transition.");
        return;
    }
    void* parameters[] = { playerClassAccessor };
    Invoke(initialize, animationManager, parameters);
    ExtendedMode_CaptureAnimatorState();
    ExtendedMode_ApplyCoatlessLogic(true);
    ExtendedMode_ApplyGameplayAnimationLayerBaseline();
    LogInfo(LogCategory_Action, L"Action", L"Reinitialized PlayerAnimationManager for restored gameplay.");
}

bool ExtendedMode_IsCoatlessLogicActive()
{
    const int state = g_extendedModeState.load();
    return state == ExtendedMode_Active ||
           state == ExtendedMode_EnteringGameplay ||
           state == ExtendedMode_Gameplay;
}

Il2CppObject* ExtendedMode_GetPlayerStateModel()
{
    auto* playerClassAccessor = g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
    return playerClassAccessor != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x60)
        : nullptr;
}

Il2CppObject* ExtendedMode_GetAnimationManager()
{
    auto* playerClassAccessor = g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
    return playerClassAccessor != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x48)
        : nullptr;
}

Il2CppObject* ExtendedMode_GetCoatClothesRecord()
{
    auto* clothesClass = FindClass(
        "ExposureUnnoticed2.Master.Clothes",
        "MClothes");
    auto* get = FindMethod(clothesClass, "Get", 1);
    if (get == nullptr)
    {
        return nullptr;
    }
    int coatType = 0; // ClothesType.Coat
    void* parameters[] = { &coatType };
    return Invoke(get, nullptr, parameters);
}

void ExtendedMode_ApplyNakedGameplayClothingState(bool force)
{
    if (g_extendedModeState.load() != ExtendedMode_EnteringGameplay &&
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }

    auto* playerState = ExtendedMode_GetPlayerStateModel();
    if (playerState == nullptr)
    {
        return;
    }

    auto* playerCurrentClothes =
        reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerState) + 0x10);
    auto* playerClothesBState =
        reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x20C);
    auto* exposedBodyParts =
        reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x27C);
    auto* coatRecord = ExtendedMode_GetCoatClothesRecord();
    if (coatRecord == nullptr)
    {
        return;
    }
    if (!g_extendedClothingStateSnapshot.playerStateValid)
    {
        g_extendedClothingStateSnapshot.playerCurrentClothes =
                *playerCurrentClothes;
        g_extendedClothingStateSnapshot.playerCurrentClothesBState =
            *playerClothesBState;
        g_extendedClothingStateSnapshot.playerExposedBodyParts =
            *exposedBodyParts;
        g_extendedClothingStateSnapshot.playerSkipDropClothes =
            ExtendedMode_GetPlayerSkipDropClothes(playerState);
        g_extendedClothingStateSnapshot.playerStateValid = true;
    }

    // Keep a valid coat record because PlayerController/PlayerAction expect
    // one for the whole lifetime of gameplay. DropClothes (5) is the shipped
    // naked branch used by CoatController.GetExposedBodyParts().
    *playerCurrentClothes = coatRecord;
    *playerClothesBState = 10; // CloseB
    *exposedBodyParts = 32;
    ExtendedMode_SetPlayerSkipDropClothes(playerState, true);

    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr)
    {
        if (!g_extendedClothingStateSnapshot.valid)
        {
            g_extendedClothingStateSnapshot.currentClothes =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + 0x19C);
            g_extendedClothingStateSnapshot.currentHadObject =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + 0x1A0);
            g_extendedClothingStateSnapshot.currentHoldClothes =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + 0x1A4);
            g_extendedClothingStateSnapshot.clothesAState =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + 0x1C4);
            g_extendedClothingStateSnapshot.clothesBState =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + 0x1C8);
            g_extendedClothingStateSnapshot.valid = true;
        }
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x19C) = 0; // Coat
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A0) = -1;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A4) = -1;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1C4) = 5; // DropClothes
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1C8) = 10; // CloseB
    }

    auto* animationManager = ExtendedMode_GetAnimationManager();
    if (animationManager != nullptr)
    {
        auto* managerClass = FindClass(
            "ExposureUnnoticed2.Object3D.Player.Scripts",
            "PlayerAnimationManager");
        if (force ||
            animationManager != g_extendedCoatlessManager)
        {
            bool previewNaked = false;
            bool holdInHand = false;
            auto* setPreviewNaked = FindMethod(
                managerClass,
                "SetPreviewNakedArmPose",
                1);
            auto* setHoldInHand = FindMethod(
                managerClass,
                "SetHoldInHand",
                1);
            if (setPreviewNaked != nullptr)
            {
                void* parameters[] = { &previewNaked };
                Invoke(setPreviewNaked, animationManager, parameters);
            }
            if (setHoldInHand != nullptr)
            {
                void* parameters[] = { &holdInHand };
                Invoke(setHoldInHand, animationManager, parameters);
            }
        }
        g_extendedCoatlessManager = animationManager;
    }

    if (ExtendedMode_GetAnimatorBool("PreviewNakedArmPose"))
    {
        ExtendedMode_SetAnimatorBool("PreviewNakedArmPose", false);
    }
    if (ExtendedMode_GetAnimatorBool("HoldInHand"))
    {
        ExtendedMode_SetAnimatorBool("HoldInHand", false);
    }

    if (g_extendedAvatarReferencer != nullptr)
    {
        auto* clothesParent = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x238);
        auto* coat = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x240);
        auto* coatController = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x250);

        // Q remains a visual-only coat switch. It never restores coat clothing
        // state or changes the gameplay Animator layers.
        const bool showCoatVisual =
            g_extendedSpecialCoatMode.load();
        if (clothesParent != nullptr &&
            ExtendedMode_GetGameObjectActiveSelf(clothesParent) !=
                showCoatVisual)
        {
            SetWorldGameObjectActive(
                clothesParent,
                showCoatVisual);
        }
        if (coat != nullptr &&
            ExtendedMode_GetGameObjectActiveSelf(coat) !=
                showCoatVisual)
        {
            SetWorldGameObjectActive(coat, showCoatVisual);
        }
        if (coatController != nullptr &&
            ExtendedMode_GetBehaviourEnabled(coatController))
        {
            // The coat mesh may be visible, but its shipped controller must not
            // write coat state over the naked gameplay state.
            ExtendedMode_SetComponentEnabled(
                coatController,
                false);
        }
        g_extendedCoatlessCoatController = coatController;
        g_extendedCoatlessCoatVisualEquipped = showCoatVisual;
    }

    ExtendedMode_HideDroppedCoat(force);
    g_extendedCoatlessApplied = true;
}

void ExtendedMode_ApplySpecialCoatLogic()
{
    if (!g_extendedSpecialCoatMode.load() ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }
    auto* playerState = ExtendedMode_GetPlayerStateModel();
    auto* coatRecord = ExtendedMode_GetCoatClothesRecord();
    if (playerState == nullptr || coatRecord == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Escape special coat mode could not resolve the coat record.");
        return;
    }

    auto* clothesParent = g_extendedAvatarReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x238)
        : nullptr;
    auto* coat = g_extendedAvatarReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x240)
        : nullptr;
    auto* coatController = g_extendedAvatarReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x250)
        : nullptr;

    if (!g_extendedSpecialCoatApplied)
    {
        auto& snapshot = g_extendedSpecialCoatSnapshot;
        snapshot.currentClothes = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerState) + 0x10);
        snapshot.currentClothesBState = *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x20C);
        snapshot.exposedBodyParts = *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x27C);
        auto* gameStateData = ExtendedMode_GetGameStateData();
        if (gameStateData != nullptr)
        {
            snapshot.gameStateClothes = *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x19C);
            snapshot.gameStateHoldClothes = *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1A4);
            snapshot.gameStateClothesAState = *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1C4);
            snapshot.gameStateClothesBState = *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1C8);
        }
        snapshot.valid = true;

        *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerState) + 0x10) = coatRecord;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x20C) = 10; // CloseB
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x27C) = 1; // ExposedBodyParts.None
        ExtendedMode_SetPlayerSkipDropClothes(playerState, true);

        if (gameStateData != nullptr)
        {
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x19C) = 0; // Coat
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1A4) = -1;
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1C4) = 0; // CloseA
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1C8) = 10; // CloseB
        }

        if (clothesParent != nullptr)
        {
            SetWorldGameObjectActive(clothesParent, true);
        }
        if (coat != nullptr)
        {
            SetWorldGameObjectActive(coat, true);
        }
        if (coatController != nullptr)
        {
            ExtendedMode_SetComponentEnabled(coatController, true);
            auto* forceBlend = FindMethod(
                FindClass(
                    "ExposureUnnoticed2.Object3D.Player.Scripts.Clothes",
                    "CoatController"),
                "ForceSetClothesStateBlend",
                1);
            int closedState = 0; // CloseA
            void* parameters[] = { &closedState };
            Invoke(forceBlend, coatController, parameters);
        }

        auto* animationManager = ExtendedMode_GetAnimationManager();
        auto* animationManagerClass = FindClass(
            "ExposureUnnoticed2.Object3D.Player.Scripts",
            "PlayerAnimationManager");
        bool previewNaked = false;
        bool holdInHand = false;
        auto* setPreviewNaked = FindMethod(
            animationManagerClass,
            "SetPreviewNakedArmPose",
            1);
        auto* setHoldInHand = FindMethod(
            animationManagerClass,
            "SetHoldInHand",
            1);
        if (animationManager != nullptr)
        {
            void* previewParameters[] = { &previewNaked };
            void* holdParameters[] = { &holdInHand };
            Invoke(setPreviewNaked, animationManager, previewParameters);
            Invoke(setHoldInHand, animationManager, holdParameters);
        }
        ExtendedMode_SetAnimatorBool("PreviewNakedArmPose", false);
        ExtendedMode_SetAnimatorBool("HoldInHand", false);
        g_extendedSpecialCoatApplied = true;
        LogInfo(LogCategory_Action, L"Action", L"Escape special coat mode enabled with removable/drop logic disabled.");
    }

    if (clothesParent != nullptr &&
        !ExtendedMode_GetGameObjectActiveSelf(clothesParent))
    {
        SetWorldGameObjectActive(clothesParent, true);
    }
    if (coat != nullptr && !ExtendedMode_GetGameObjectActiveSelf(coat))
    {
        SetWorldGameObjectActive(coat, true);
    }
    if (coatController != nullptr &&
        !ExtendedMode_GetBehaviourEnabled(coatController))
    {
        ExtendedMode_SetComponentEnabled(coatController, true);
    }
    ExtendedMode_HideDroppedCoat(false);
}

void ExtendedMode_SetSpecialCoatMode(bool enabled)
{
    if (enabled == g_extendedSpecialCoatMode.load())
    {
        return;
    }
    if (enabled)
    {
        g_extendedSpecialCoatMode = true;
        ExtendedMode_ApplySpecialCoatLogic();
        ExtendedMode_UpdateCoatModeIndicator();
        return;
    }

    g_extendedSpecialCoatMode = false;
    g_extendedSpecialCoatApplied = false;
    auto* playerState = ExtendedMode_GetPlayerStateModel();
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (playerState != nullptr &&
        ExtendedMode_IsUnityObjectAlive(g_extendedPlayerController) &&
        g_extendedSpecialCoatSnapshot.valid)
    {
        *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerState) + 0x10) =
            g_extendedSpecialCoatSnapshot.currentClothes;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x20C) =
            g_extendedSpecialCoatSnapshot.currentClothesBState;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x27C) =
            g_extendedSpecialCoatSnapshot.exposedBodyParts;
    }
    if (gameStateData != nullptr && g_extendedSpecialCoatSnapshot.valid)
    {
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x19C) =
            g_extendedSpecialCoatSnapshot.gameStateClothes;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A4) =
            g_extendedSpecialCoatSnapshot.gameStateHoldClothes;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1C4) =
            g_extendedSpecialCoatSnapshot.gameStateClothesAState;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1C8) =
            g_extendedSpecialCoatSnapshot.gameStateClothesBState;
    }
    g_extendedSpecialCoatSnapshot = ExtendedSpecialCoatSnapshot{};
    g_extendedCoatlessApplied = false;
    ExtendedMode_ApplyCoatlessLogic(true);
    ExtendedMode_UpdateCoatModeIndicator();
    LogInfo(LogCategory_Action, L"Action", L"Escape special coat mode disabled; naked Escape logic restored.");
}

void ExtendedMode_SetPlayerSkipDropClothes(
    Il2CppObject* playerState,
    bool skip)
{
    if (playerState == nullptr)
    {
        return;
    }
    auto* playerStateClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerStateModel");
    auto* setter = FindMethod(playerStateClass, "set_SkipDropClothes", 1);
    if (setter != nullptr)
    {
        void* parameters[] = { &skip };
        Invoke(setter, playerState, parameters);
    }
}

bool ExtendedMode_GetPlayerSkipDropClothes(Il2CppObject* playerState)
{
    if (playerState == nullptr)
    {
        return false;
    }
    auto* playerStateClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerStateModel");
    auto* getter = FindMethod(playerStateClass, "get_SkipDropClothes", 0);
    if (getter == nullptr)
    {
        return false;
    }
    auto* result = Invoke(getter, playerState, nullptr);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

bool ExtendedMode_GetGameObjectActiveSelf(Il2CppObject* gameObject)
{
    if (gameObject == nullptr)
    {
        return false;
    }
    auto* result = Invoke(
        FindMethod(
            FindClass("UnityEngine", "GameObject"),
            "get_activeSelf",
            0),
        gameObject,
        nullptr);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

bool ExtendedMode_GetBehaviourEnabled(Il2CppObject* component)
{
    if (component == nullptr)
    {
        return false;
    }
    auto* result = Invoke(
        FindMethod(
            FindClass("UnityEngine", "Behaviour"),
            "get_enabled",
            0),
        component,
        nullptr);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

void ExtendedMode_SetAnimatorLayerWeightNow(int layer, float weight)
{
    if (g_previewAnimator == nullptr)
    {
        return;
    }
    static const Il2CppMethod* setLayerWeight = nullptr;
    if (setLayerWeight == nullptr)
    {
        setLayerWeight = FindMethod(
            FindClass("UnityEngine", "Animator"),
            "SetLayerWeight",
            2);
    }
    void* parameters[] = { &layer, &weight };
    Invoke(setLayerWeight, g_previewAnimator, parameters);
}

float ExtendedMode_GetAnimatorLayerWeightNow(int layer)
{
    if (g_previewAnimator == nullptr)
    {
        return 0.0f;
    }
    static const Il2CppMethod* getLayerWeight = nullptr;
    if (getLayerWeight == nullptr)
    {
        getLayerWeight = FindMethod(
            FindClass("UnityEngine", "Animator"),
            "GetLayerWeight",
            1);
    }
    void* parameters[] = { &layer };
    auto* result = Invoke(getLayerWeight, g_previewAnimator, parameters);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr
        ? *static_cast<float*>(il2cpp_object_unbox(result))
        : 0.0f;
}

void ExtendedMode_HideDroppedCoat(bool force)
{
    if (!force &&
        ++g_extendedCoatlessDroppedCoatFrame < 120)
    {
        return;
    }
    g_extendedCoatlessDroppedCoatFrame = 0;
    auto* managerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "DroppedItemManager");
    auto* getInstance = FindMethod(managerClass, "get_Instance", 0);
    auto* manager = getInstance != nullptr
        ? Invoke(getInstance, nullptr, nullptr)
        : nullptr;
    auto* hide = FindMethod(managerClass, "Hide", 1);
    if (manager == nullptr || hide == nullptr)
    {
        return;
    }
    int coatDropType = 0;
    void* parameters[] = { &coatDropType };
    Invoke(hide, manager, parameters);
    if (!g_extendedCoatlessDropSuppressedLogged)
    {
        g_extendedCoatlessDropSuppressedLogged = true;
        LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka hid any dropped coat item and disabled its interaction.");
    }
}

void ExtendedMode_ApplyCoatlessLogic(bool force)
{
    if (g_extendedSpecialCoatMode.load() &&
        g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        ExtendedMode_ApplySpecialCoatLogic();
        return;
    }
    if (!ExtendedMode_IsCoatlessLogicActive())
    {
        return;
    }

    auto* playerState = ExtendedMode_GetPlayerStateModel();
    if (playerState != nullptr)
    {
        auto* playerCurrentClothes = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerState) + 0x10);
        auto* playerCurrentClothesBState = reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x20C);
        auto* playerExposedBodyParts = reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x27C);
        if (!g_extendedClothingStateSnapshot.playerStateValid)
        {
            g_extendedClothingStateSnapshot.playerCurrentClothes =
                *playerCurrentClothes;
            g_extendedClothingStateSnapshot.playerCurrentClothesBState =
                *playerCurrentClothesBState;
            g_extendedClothingStateSnapshot.playerExposedBodyParts =
                *playerExposedBodyParts;
            g_extendedClothingStateSnapshot.playerSkipDropClothes =
                ExtendedMode_GetPlayerSkipDropClothes(playerState);
            g_extendedClothingStateSnapshot.playerStateValid = true;
        }
        // ExposedBodyParts.All. This keeps NPC visibility checks on the
        // naked branch even while the equipped coat mesh remains visible.
        if (*playerExposedBodyParts != 32)
        {
            *playerExposedBodyParts = 32;
        }
        auto* skipDropClothes = reinterpret_cast<bool*>(
            reinterpret_cast<char*>(playerState) + 0x239);
        if (force || !g_extendedCoatlessApplied || !*skipDropClothes)
        {
            ExtendedMode_SetPlayerSkipDropClothes(playerState, true);
        }
    }

    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr)
    {
        constexpr int offsets[] = { 0x19C, 0x1A0, 0x1A4, 0x1C4, 0x1C8 };
        if (!g_extendedClothingStateSnapshot.valid)
        {
            g_extendedClothingStateSnapshot.currentClothes =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + offsets[0]);
            g_extendedClothingStateSnapshot.currentHadObject =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + offsets[1]);
            g_extendedClothingStateSnapshot.currentHoldClothes =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + offsets[2]);
            g_extendedClothingStateSnapshot.clothesAState =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + offsets[3]);
            g_extendedClothingStateSnapshot.clothesBState =
                *reinterpret_cast<int*>(
                    reinterpret_cast<char*>(gameStateData) + offsets[4]);
            g_extendedClothingStateSnapshot.valid = true;
        }
    }

    auto* animationManager = ExtendedMode_GetAnimationManager();
    const bool managerChanged =
        animationManager != nullptr &&
        animationManager != g_extendedCoatlessManager;
    if (animationManager != nullptr)
    {
        auto* managerClass = FindClass(
            "ExposureUnnoticed2.Object3D.Player.Scripts",
            "PlayerAnimationManager");
        if (force || managerChanged || !g_extendedCoatlessApplied)
        {
            // PreviewNakedArmPose is a warehouse-preview override. Applying it
            // in gameplay changes the shipped action-arm state machine and can
            // leave interaction actions waiting on the wrong arm layer.
            bool previewNaked = false;
            bool holdInHand = false;
            float targetArmLayerWeight = 0.0f;
            auto* setPreviewNaked = FindMethod(
                managerClass,
                "SetPreviewNakedArmPose",
                1);
            if (setPreviewNaked != nullptr)
            {
                void* parameters[] = { &previewNaked };
                Invoke(setPreviewNaked, animationManager, parameters);
            }
            auto* setHoldInHand = FindMethod(
                managerClass,
                "SetHoldInHand",
                1);
            if (setHoldInHand != nullptr)
            {
                void* parameters[] = { &holdInHand };
                Invoke(setHoldInHand, animationManager, parameters);
            }
            auto* setTargetArmLayerWeight = FindMethod(
                managerClass,
                "set_TargetArmLayerWeight",
                1);
            if (setTargetArmLayerWeight != nullptr)
            {
                void* parameters[] = { &targetArmLayerWeight };
                Invoke(
                    setTargetArmLayerWeight,
                    animationManager,
                    parameters);
            }
        }
        g_extendedCoatlessManager = animationManager;
    }

    const bool gameplay = g_extendedModeState.load() == ExtendedMode_Gameplay;
    const bool previewNakedArmPose = !gameplay;
    if (ExtendedMode_GetAnimatorBool("PreviewNakedArmPose") !=
        previewNakedArmPose)
    {
        ExtendedMode_SetAnimatorBool(
            "PreviewNakedArmPose",
            previewNakedArmPose);
    }

    if (g_extendedAvatarReferencer != nullptr)
    {
        auto* clothesParent = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x238);
        auto* coat = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x240);
        auto* coatController = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x250);
        const bool coatEquipped = InventoryUi_IsCoatEquipped();
        if (coatEquipped &&
            clothesParent != nullptr &&
            !ExtendedMode_GetGameObjectActiveSelf(clothesParent))
        {
            SetWorldGameObjectActive(clothesParent, true);
        }
        if (coat != nullptr &&
            ExtendedMode_GetGameObjectActiveSelf(coat) != coatEquipped)
        {
            SetWorldGameObjectActive(coat, coatEquipped);
        }
        if (coatController != nullptr)
        {
            const bool controllerChanged =
                coatController != g_extendedCoatlessCoatController;
            if (coatEquipped &&
                (force ||
                 controllerChanged ||
                 !g_extendedCoatlessCoatVisualEquipped))
            {
                int closedCoat = 0;
                auto* forceCoatBlend = FindMethod(
                    FindClass(
                        "ExposureUnnoticed2.Object3D.Player.Scripts.Clothes",
                        "CoatController"),
                    "ForceSetClothesStateBlend",
                    1);
                if (forceCoatBlend != nullptr)
                {
                    void* parameters[] = { &closedCoat };
                    Invoke(forceCoatBlend, coatController, parameters);
                }
            }
            if (ExtendedMode_GetBehaviourEnabled(coatController))
            {
                ExtendedMode_SetComponentEnabled(coatController, false);
            }
            g_extendedCoatlessCoatController = coatController;
        }
        g_extendedCoatlessCoatVisualEquipped = coatEquipped;
    }

    ExtendedMode_HideDroppedCoat(force || !g_extendedCoatlessApplied);
    if (!g_extendedCoatlessApplied)
    {
        g_extendedCoatlessApplied = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape from Manaka applied naked exposure and arm overrides while preserving the equipped coat visual.");
    }
}

void ExtendedMode_RestoreClothingStateSnapshot()
{
    if (!g_extendedClothingStateSnapshot.valid &&
        !g_extendedClothingStateSnapshot.playerStateValid)
    {
        return;
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedPlayerController))
    {
        // The previous gameplay scene is gone. Its PlayerState pointers are
        // stale and the next slot load obtains fresh state from the save file.
        g_extendedClothingStateSnapshot = ExtendedClothingStateSnapshot{};
        g_extendedCoatlessManager = nullptr;
        g_extendedCoatlessCoatController = nullptr;
        g_extendedCoatlessDroppedCoatFrame = 0;
        g_extendedCoatlessApplied = false;
        g_extendedCoatlessDropSuppressedLogged = false;
        g_extendedCoatlessCoatVisualEquipped = false;
        return;
    }
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr &&
        g_extendedClothingStateSnapshot.valid)
    {
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x19C) =
            g_extendedClothingStateSnapshot.currentClothes;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A0) =
            g_extendedClothingStateSnapshot.currentHadObject;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A4) =
            g_extendedClothingStateSnapshot.currentHoldClothes;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1C4) =
            g_extendedClothingStateSnapshot.clothesAState;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1C8) =
            g_extendedClothingStateSnapshot.clothesBState;
    }
    auto* playerState = ExtendedMode_GetPlayerStateModel();
    if (playerState != nullptr &&
        g_extendedClothingStateSnapshot.playerStateValid)
    {
        *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerState) + 0x10) =
            g_extendedClothingStateSnapshot.playerCurrentClothes;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x20C) =
            g_extendedClothingStateSnapshot.playerCurrentClothesBState;
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x27C) =
            g_extendedClothingStateSnapshot.playerExposedBodyParts;
        ExtendedMode_SetPlayerSkipDropClothes(
            playerState,
            g_extendedClothingStateSnapshot.playerSkipDropClothes);
    }
    g_extendedClothingStateSnapshot = ExtendedClothingStateSnapshot{};
    g_extendedCoatlessManager = nullptr;
    g_extendedCoatlessCoatController = nullptr;
    g_extendedCoatlessDroppedCoatFrame = 0;
    g_extendedCoatlessApplied = false;
    g_extendedCoatlessDropSuppressedLogged = false;
    g_extendedCoatlessCoatVisualEquipped = false;
}

void ExtendedMode_EnforceActionLayers()
{
    if (g_previewAnimator == nullptr ||
        g_extendedSelectedAction < 0 ||
        g_extendedSelectedAction >= kEscapeFromManakaActionCount)
    {
        return;
    }
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* setLayerWeight = FindMethod(
        animatorClass,
        "SetLayerWeight",
        2);
    const int selectedLayer =
        kEscapeFromManakaActions[g_extendedSelectedAction].layer;
    for (int layer = 0; layer < 12; ++layer)
    {
        float weight = layer == selectedLayer ? 1.0f : 0.0f;
        void* parameters[] = { &layer, &weight };
        Invoke(setLayerWeight, g_previewAnimator, parameters);
    }
}

Il2CppObject* ExtendedMode_FindPlayerAnimator(Il2CppObject* avatarRoot)
{
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (!ExtendedMode_IsUnityObjectAlive(avatarRoot) ||
        animatorClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return nullptr;
    }
    il2cpp_runtime_class_init(animatorClass);
    auto* animatorType = il2cpp_type_get_object(
        il2cpp_class_get_type(animatorClass));
    bool includeInactive = true;
    void* parameters[] = { animatorType, &includeInactive };
    auto* animators = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        avatarRoot,
        parameters);
    if (animators == nullptr)
    {
        return nullptr;
    }

    const auto count = il2cpp_array_length(animators);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(animators) +
        il2cpp_array_object_header_size());
    Il2CppObject* fallback = nullptr;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* animator = elements[index];
        if (animator == nullptr)
        {
            continue;
        }
        auto* controller = Invoke(
            FindMethod(animatorClass, "get_runtimeAnimatorController", 0),
            animator,
            nullptr);
        if (controller != nullptr)
        {
            const auto controllerName = GetObjectName(controller);
            if (controllerName == L"Player" ||
                controllerName.find(L"Player") != std::wstring::npos)
            {
                LogInfo(LogCategory_Action, L"Action", 
                    L"Extended mode selected original Animator[%u] controller=%s.",
                    index,
                    controllerName.c_str());
                return animator;
            }
            if (fallback == nullptr)
            {
                fallback = animator;
            }
        }
    }
    if (fallback != nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Extended mode fell back to the first available avatar Animator.");
    }
    return fallback;
}

Il2CppObject* ExtendedMode_CreatePrimitive(
    const char* name,
    int primitiveType,
    float x,
    float y,
    float z,
    float scaleX,
    float scaleY,
    float scaleZ,
    float red,
    float green,
    float blue)
{
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (gameObjectClass == nullptr)
    {
        return nullptr;
    }
    void* parameters[] = { &primitiveType };
    auto* gameObject = Invoke(
        FindMethod(gameObjectClass, "CreatePrimitive", 1),
        nullptr,
        parameters);
    if (gameObject == nullptr)
    {
        return nullptr;
    }
    ExtendedMode_SetObjectName(gameObject, name);
    ExtendedMode_SetGameObjectLayer(gameObject, kExtendedRoomLayer);
    auto* transform = GetWorldTransform(gameObject);
    ExtendedMode_SetTransformPosition(transform, x, y, z);
    ExtendedMode_SetTransformLocalScale(transform, scaleX, scaleY, scaleZ);

    auto* renderer = GetSceneComponentByClass(
        gameObject,
        "UnityEngine",
        "Renderer");
    if (renderer != nullptr)
    {
        auto* material = Invoke(
            FindMethod(FindClass("UnityEngine", "Renderer"), "get_material", 0),
            renderer,
            nullptr);
        if (material != nullptr)
        {
            float color[] = { red, green, blue, 1.0f };
            void* colorParameters[] = { color };
            Invoke(
                FindMethod(FindClass("UnityEngine", "Material"), "set_color", 1),
                material,
                colorParameters);
        }
    }
    return gameObject;
}

Il2CppObject* ExtendedMode_CreateLight(
    const char* name,
    int lightType,
    float x,
    float y,
    float z,
    float rotX,
    float rotY,
    float rotZ,
    float intensity,
    float range,
    float red,
    float green,
    float blue)
{
    auto* gameObject = CreateWorldGameObject(name);
    if (gameObject == nullptr)
    {
        return nullptr;
    }
    ExtendedMode_SetGameObjectLayer(gameObject, kExtendedRoomLayer);
    auto* transform = GetWorldTransform(gameObject);
    ExtendedMode_SetTransformPosition(transform, x, y, z);
    SetTransformEuler(transform, rotX, rotY, rotZ);
    auto* light = AddSceneComponentToGameObject(
        gameObject,
        "UnityEngine",
        "Light");
    auto* lightClass = FindClass("UnityEngine", "Light");
    float color[] = { red, green, blue, 1.0f };
    int cullingMask = 1 << kExtendedRoomLayer;
    int shadows = 1;
    float shadowStrength = 0.55f;
    void* typeParameters[] = { &lightType };
    void* intensityParameters[] = { &intensity };
    void* rangeParameters[] = { &range };
    void* colorParameters[] = { color };
    void* maskParameters[] = { &cullingMask };
    void* shadowParameters[] = { &shadows };
    void* shadowStrengthParameters[] = { &shadowStrength };
    Invoke(
        FindMethod(lightClass, "set_type", 1),
        light,
        typeParameters);
    Invoke(
        FindMethod(lightClass, "set_intensity", 1),
        light,
        intensityParameters);
    Invoke(
        FindMethod(lightClass, "set_range", 1),
        light,
        rangeParameters);
    Invoke(
        FindMethod(lightClass, "set_color", 1),
        light,
        colorParameters);
    Invoke(
        FindMethod(lightClass, "set_cullingMask", 1),
        light,
        maskParameters);
    Invoke(
        FindMethod(lightClass, "set_shadows", 1),
        light,
        shadowParameters);
    Invoke(
        FindMethod(lightClass, "set_shadowStrength", 1),
        light,
        shadowStrengthParameters);
    return gameObject;
}

void ExtendedMode_BuildBlackRoom()
{
    // Cube primitive type is 3 in UnityEngine.PrimitiveType.
    ExtendedMode_CreatePrimitive(
        "EFM_BlackRoomFloor",
        3,
        g_extendedRoomCenterX,
        g_extendedRoomFloorY - 0.06f,
        g_extendedRoomCenterZ,
        10.0f,
        0.1f,
        10.0f,
        0.018f,
        0.021f,
        0.026f);
    ExtendedMode_CreatePrimitive(
        "EFM_BlackRoomBack",
        3,
        g_extendedRoomCenterX,
        g_extendedRoomFloorY + 2.5f,
        g_extendedRoomCenterZ + 4.5f,
        10.0f,
        5.2f,
        0.12f,
        0.028f,
        0.032f,
        0.040f);
    ExtendedMode_CreatePrimitive(
        "EFM_BlackRoomLeft",
        3,
        g_extendedRoomCenterX - 4.5f,
        g_extendedRoomFloorY + 2.5f,
        g_extendedRoomCenterZ,
        0.12f,
        5.2f,
        9.0f,
        0.020f,
        0.024f,
        0.030f);
    ExtendedMode_CreatePrimitive(
        "EFM_BlackRoomRight",
        3,
        g_extendedRoomCenterX + 4.5f,
        g_extendedRoomFloorY + 2.5f,
        g_extendedRoomCenterZ,
        0.12f,
        5.2f,
        9.0f,
        0.020f,
        0.024f,
        0.030f);
    ExtendedMode_CreatePrimitive(
        "EFM_BlackRoomCeiling",
        3,
        g_extendedRoomCenterX,
        g_extendedRoomFloorY + 5.0f,
        g_extendedRoomCenterZ,
        10.0f,
        0.1f,
        10.0f,
        0.012f,
        0.014f,
        0.018f);

    ExtendedMode_CreateLight(
        "EFM_KeyLight",
        1,
        g_extendedRoomCenterX - 1.5f,
        g_extendedRoomFloorY + 4.0f,
        g_extendedRoomCenterZ - 2.0f,
        42.0f,
        -30.0f,
        0.0f,
        0.25f,
        20.0f,
        1.0f,
        0.88f,
        0.78f);
    ExtendedMode_CreateLight(
        "EFM_FillLight",
        2,
        g_extendedRoomCenterX + 1.7f,
        g_extendedRoomFloorY + 1.8f,
        g_extendedRoomCenterZ - 1.6f,
        0.0f,
        0.0f,
        0.0f,
        0.08f,
        9.0f,
        0.30f,
        0.42f,
        1.0f);
    ExtendedMode_CreateLight(
        "EFM_RimLight",
        2,
        g_extendedRoomCenterX,
        g_extendedRoomFloorY + 2.7f,
        g_extendedRoomCenterZ + 2.2f,
        0.0f,
        0.0f,
        0.0f,
        0.05f,
        7.0f,
        1.0f,
        0.36f,
        0.20f);
}

void ExtendedMode_CapturePlayerFrame()
{
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* playerTransform = GetWorldTransform(g_extendedPlayerGameObject);
    auto* avatarTransform = GetWorldTransform(g_extendedAvatarRoot);
    auto* playerPosition = playerTransform != nullptr
        ? Invoke(
            FindMethod(transformClass, "get_position", 0),
            playerTransform,
            nullptr)
        : nullptr;
    if (playerPosition != nullptr && il2cpp_object_unbox != nullptr)
    {
        const auto* position = static_cast<float*>(
            il2cpp_object_unbox(playerPosition));
        g_extendedRoomCenterX = position[0];
        g_extendedRoomFloorY = position[1];
        g_extendedRoomCenterZ = position[2];
        g_extendedModelCenterX = position[0];
        g_extendedModelCenterY = position[1] + 0.85f;
        g_extendedModelCenterZ = position[2];
    }

    auto* avatarPosition = avatarTransform != nullptr
        ? Invoke(
            FindMethod(transformClass, "get_position", 0),
            avatarTransform,
            nullptr)
        : nullptr;
    if (avatarPosition != nullptr && il2cpp_object_unbox != nullptr)
    {
        const auto* position = static_cast<float*>(
            il2cpp_object_unbox(avatarPosition));
        g_extendedModelCenterX = position[0];
        g_extendedModelCenterY = position[1] + 0.80f;
        g_extendedModelCenterZ = position[2];
    }

    auto* localEuler = avatarTransform != nullptr
        ? Invoke(
            FindMethod(transformClass, "get_localEulerAngles", 0),
            avatarTransform,
            nullptr)
        : nullptr;
    if (localEuler != nullptr && il2cpp_object_unbox != nullptr)
    {
        const auto* euler = static_cast<float*>(
            il2cpp_object_unbox(localEuler));
        g_extendedModelBaseLocalEulerX = euler[0];
        g_extendedModelBaseLocalEulerY = euler[1];
        g_extendedModelBaseLocalEulerZ = euler[2];
        g_extendedOriginalLocalEulerX = euler[0];
        g_extendedOriginalLocalEulerY = euler[1];
        g_extendedOriginalLocalEulerZ = euler[2];
    }

    auto* forwardValue = avatarTransform != nullptr
        ? Invoke(
            FindMethod(transformClass, "get_forward", 0),
            avatarTransform,
            nullptr)
        : nullptr;
    if (forwardValue != nullptr && il2cpp_object_unbox != nullptr)
    {
        const auto* forward = static_cast<float*>(
            il2cpp_object_unbox(forwardValue));
        const float length = std::sqrt(
            forward[0] * forward[0] +
            forward[2] * forward[2]);
        if (length > 0.001f)
        {
            g_extendedModelForwardX = forward[0] / length;
            g_extendedModelForwardZ = forward[2] / length;
        }
    }

    if (g_extendedAvatarReferencer != nullptr)
    {
        auto* bodyRenderer = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x208);
        auto* boundsValue = bodyRenderer != nullptr
            ? Invoke(
                FindMethod(FindClass("UnityEngine", "Renderer"), "get_bounds", 0),
                bodyRenderer,
                nullptr)
            : nullptr;
        if (boundsValue != nullptr && il2cpp_object_unbox != nullptr)
        {
            const auto* bounds = static_cast<float*>(
                il2cpp_object_unbox(boundsValue));
            g_extendedModelCenterX = bounds[0];
            g_extendedModelCenterY = bounds[1];
            g_extendedModelCenterZ = bounds[2];
        }
    }

    LogInfo(LogCategory_Action, L"Action", 
        L"Extended player frame: room=(%.2f, %.2f, %.2f), model=(%.2f, %.2f, %.2f).",
        g_extendedRoomCenterX,
        g_extendedRoomFloorY,
        g_extendedRoomCenterZ,
        g_extendedModelCenterX,
        g_extendedModelCenterY,
        g_extendedModelCenterZ);
}

void ExtendedMode_FaceAvatarTowardCamera()
{
    if (g_extendedCamera == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(g_extendedAvatarRoot))
    {
        return;
    }
    auto* cameraTransform = GetWorldTransform(
        GetComponentGameObject(g_extendedCamera));
    auto* avatarTransform = GetWorldTransform(g_extendedAvatarRoot);
    auto* cameraPosition = cameraTransform != nullptr
        ? Invoke(
            FindMethod(FindClass("UnityEngine", "Transform"), "get_position", 0),
            cameraTransform,
            nullptr)
        : nullptr;
    if (avatarTransform == nullptr ||
        cameraPosition == nullptr ||
        il2cpp_object_unbox == nullptr)
    {
        return;
    }
    const auto* position = static_cast<float*>(
        il2cpp_object_unbox(cameraPosition));
    const float deltaX = position[0] - g_extendedModelCenterX;
    const float deltaZ = position[2] - g_extendedModelCenterZ;
    constexpr float radiansToDegrees =
        57.2957795130823208768f;
    const float facingYaw =
        std::atan2(deltaX, deltaZ) * radiansToDegrees +
        g_extendedModelYaw;
    SetTransformEuler(avatarTransform, 0.0f, facingYaw, 0.0f);
}

void ExtendedMode_ConfigureCamera()
{
    auto* cameraClass = FindClass("UnityEngine", "Camera");
    if (g_extendedCamera == nullptr)
    {
        g_extendedCamera = Invoke(
            FindMethod(cameraClass, "get_main", 0),
            nullptr,
            nullptr);
    }
    if (g_extendedCamera == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Extended mode could not resolve the gameplay Camera.");
        return;
    }

    auto* cameraGameObject = GetComponentGameObject(g_extendedCamera);
    auto* transform = GetWorldTransform(cameraGameObject);
    constexpr float targetScreenX = 0.12f;
    auto* aspectValue = Invoke(
        FindMethod(cameraClass, "get_aspect", 0),
        g_extendedCamera,
        nullptr);
    const float cameraAspect =
        aspectValue != nullptr && il2cpp_object_unbox != nullptr
            ? *static_cast<float*>(il2cpp_object_unbox(aspectValue))
            : 1.7777778f;
    const float cameraOffsetX =
        (0.5f - targetScreenX) * 2.0f * 1.25f * cameraAspect;
    ExtendedMode_SetTransformPosition(
        transform,
        g_extendedModelCenterX + cameraOffsetX,
        g_extendedModelCenterY - 0.16f,
        g_extendedModelCenterZ - 3.60f);
    SetTransformEuler(transform, 0.0f, 0.0f, 0.0f);

    int cullingMask = 1 << kExtendedRoomLayer;
    int clearFlags = 2;
    float nearClip = 0.03f;
    float farClip = 30.0f;
    bool orthographic = true;
    float orthographicSize = 1.25f;
    float fieldOfView = 32.0f;
    float backgroundColor[] = { 0.008f, 0.010f, 0.014f, 1.0f };
    void* maskParameters[] = { &cullingMask };
    void* clearParameters[] = { &clearFlags };
    void* nearParameters[] = { &nearClip };
    void* farParameters[] = { &farClip };
    void* orthographicParameters[] = { &orthographic };
    void* orthographicSizeParameters[] = { &orthographicSize };
    void* fovParameters[] = { &fieldOfView };
    void* colorParameters[] = { backgroundColor };
    Invoke(
        FindMethod(cameraClass, "set_cullingMask", 1),
        g_extendedCamera,
        maskParameters);
    Invoke(
        FindMethod(cameraClass, "set_clearFlags", 1),
        g_extendedCamera,
        clearParameters);
    Invoke(
        FindMethod(cameraClass, "set_nearClipPlane", 1),
        g_extendedCamera,
        nearParameters);
    Invoke(
        FindMethod(cameraClass, "set_farClipPlane", 1),
        g_extendedCamera,
        farParameters);
    Invoke(
        FindMethod(cameraClass, "set_orthographic", 1),
        g_extendedCamera,
        orthographicParameters);
    Invoke(
        FindMethod(cameraClass, "set_orthographicSize", 1),
        g_extendedCamera,
        orthographicSizeParameters);
    Invoke(
        FindMethod(cameraClass, "set_fieldOfView", 1),
        g_extendedCamera,
        fovParameters);
    Invoke(
        FindMethod(cameraClass, "set_backgroundColor", 1),
        g_extendedCamera,
        colorParameters);
    ExtendedMode_SetComponentEnabled(g_extendedCamera, true);
    ExtendedMode_FaceAvatarTowardCamera();
}

void ExtendedMode_FrameModel()
{
    if (g_extendedCamera == nullptr || g_extendedAvatarRoot == nullptr)
    {
        return;
    }
    auto* rendererClass = FindClass("UnityEngine", "SkinnedMeshRenderer");
    auto* rendererBaseClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (rendererClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    bool includeInactive = false;
    void* findParameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_extendedAvatarRoot,
        findParameters);
    if (renderers == nullptr)
    {
        return;
    }
    auto* getEnabled = FindMethod(rendererClass, "get_enabled", 0);
    auto* getBounds = FindMethod(rendererClass, "get_bounds", 0);
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    float minX = 1.0e9f;
    float maxX = -1.0e9f;
    float minY = 1.0e9f;
    float maxY = -1.0e9f;
    float minZ = 1.0e9f;
    float maxZ = -1.0e9f;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        if (renderer == nullptr)
        {
            continue;
        }
        auto* enabledValue = Invoke(getEnabled, renderer, nullptr);
        if (enabledValue != nullptr &&
            il2cpp_object_unbox != nullptr &&
            !*static_cast<bool*>(il2cpp_object_unbox(enabledValue)))
        {
            continue;
        }
        auto* boundsValue = Invoke(getBounds, renderer, nullptr);
        if (boundsValue == nullptr || il2cpp_object_unbox == nullptr)
        {
            continue;
        }
        const auto* bounds = static_cast<float*>(
            il2cpp_object_unbox(boundsValue));
        minX = (bounds[0] - bounds[3] < minX)
            ? bounds[0] - bounds[3]
            : minX;
        maxX = (bounds[0] + bounds[3] > maxX)
            ? bounds[0] + bounds[3]
            : maxX;
        minY = (bounds[1] - bounds[4] < minY)
            ? bounds[1] - bounds[4]
            : minY;
        maxY = (bounds[1] + bounds[4] > maxY)
            ? bounds[1] + bounds[4]
            : maxY;
        minZ = (bounds[2] - bounds[5] < minZ)
            ? bounds[2] - bounds[5]
            : minZ;
        maxZ = (bounds[2] + bounds[5] > maxZ)
            ? bounds[2] + bounds[5]
            : maxZ;
    }
    if (minY > maxY)
    {
        return;
    }
    g_extendedModelCenterX = (minX + maxX) * 0.5f;
    g_extendedModelCenterY = (minY + maxY) * 0.5f;
    g_extendedModelCenterZ = (minZ + maxZ) * 0.5f;
    const float halfHeight = (maxY - minY) * 0.5f;
    const float orthographicSize = halfHeight > 0.55f
        ? halfHeight + 0.12f
        : 0.67f;
    auto* cameraTransform = GetWorldTransform(
        GetComponentGameObject(g_extendedCamera));
    constexpr float targetScreenX = 0.12f;
    auto* aspectValue = Invoke(
        FindMethod(FindClass("UnityEngine", "Camera"), "get_aspect", 0),
        g_extendedCamera,
        nullptr);
    const float cameraAspect =
        aspectValue != nullptr && il2cpp_object_unbox != nullptr
            ? *static_cast<float*>(il2cpp_object_unbox(aspectValue))
            : 1.7777778f;
    const float cameraOffsetX =
        (0.5f - targetScreenX) * 2.0f * orthographicSize * cameraAspect;
    ExtendedMode_SetTransformPosition(
        cameraTransform,
        g_extendedModelCenterX + cameraOffsetX,
        g_extendedModelCenterY,
        g_extendedModelCenterZ - 3.60f);
    SetTransformEuler(cameraTransform, 0.0f, 0.0f, 0.0f);
    void* sizeParameters[] = { const_cast<float*>(&orthographicSize) };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Camera"), "set_orthographicSize", 1),
        g_extendedCamera,
        sizeParameters);
    ExtendedMode_FaceAvatarTowardCamera();
}

void ExtendedMode_DisableGameplayControllers()
{
    if (g_extendedInGameReferencer != nullptr)
    {
        auto* baseCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x38);
        auto* tpCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x40);
        auto* inventoryCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x48);
        auto* faceCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x98);
        ExtendedMode_SetComponentEnabled(baseCamera, false);
        ExtendedMode_SetComponentEnabled(tpCamera, false);
        ExtendedMode_SetComponentEnabled(inventoryCamera, false);
        ExtendedMode_SetComponentEnabled(faceCamera, false);
        auto* volumeController = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x58);
        if (volumeController != nullptr)
        {
            SetWorldGameObjectActive(
                GetComponentGameObject(volumeController),
                false);
            LogInfo(LogCategory_Action, L"Action", L"Extended mode disabled the original scene post-process volume.");
        }
        if (baseCamera != nullptr)
        {
            auto* cameraProperty = *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(baseCamera) + 0x20);
            if (cameraProperty != nullptr)
            {
                g_extendedCamera = cameraProperty;
            }
        }
    }
    if (g_extendedAvatarReferencer != nullptr)
    {
        auto* fpCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x78);
        ExtendedMode_SetComponentEnabled(fpCamera, false);
    }
}

void ExtendedMode_HideGameplayUi()
{
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedInGameUiManager))
    {
        g_extendedInGameUiManager = nullptr;
        g_extendedInGameUiManager = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Object3D.IngameManager",
            "InGameUiManager",
            "Instance");
    }
    if (g_extendedInGameUiManager == nullptr)
    {
        return;
    }
    // Keep the InGameUiManager root alive because the original system menu is
    // one of its children. Hide only HUD elements, leaving the menu functional.
    constexpr int hidableComponentOffsets[] = {
        0x28, 0x38, 0x40, 0x50, 0x58, 0x60, 0x68,
        // 0x78 is TempUnlockPanelView. Keep it active so the original
        // first-stage and accumulated unlock rings remain visible.
        0x70, 0x80, 0x88, 0x90, 0x98, 0xA0,
        0xE8, 0xF0,
        0xF8, 0x110, 0x118, 0x120, 0x138, 0x140
    };
    constexpr int hidableObjectOffsets[] = {
        0xD8, 0x100, 0x128, 0x130, 0x150, 0x158, 0x160
    };
    for (const int offset : hidableComponentOffsets)
    {
        auto* component = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameUiManager) + offset);
        if (component == nullptr)
        {
            continue;
        }
        auto* componentObject = GetComponentGameObject(component);
        if (componentObject != nullptr)
        {
            SetWorldGameObjectActive(componentObject, false);
        }
    }
    for (const int offset : hidableObjectOffsets)
    {
        auto* gameObject = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameUiManager) + offset);
        SetWorldGameObjectActive(gameObject, false);
    }
    static bool loggedTempUnlockPanelRetained = false;
    if (!loggedTempUnlockPanelRetained)
    {
        loggedTempUnlockPanelRetained = true;
        LogInfo(
            LogCategory_Action,
            L"Action",
            L"Escape retained the original TempUnlockPanelView for handcuff "
            L"unlock progress rings.");
    }
    auto* statusInfoObject = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedInGameUiManager) + 0xD0);
    auto* ecstasyGauge = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedInGameUiManager) + 0x30);
    auto* ecstasyHeart = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedInGameUiManager) + 0xE0);
    SetWorldGameObjectActive(statusInfoObject, false);
    if (ecstasyGauge != nullptr)
    {
        SetWorldGameObjectActive(
            GetComponentGameObject(ecstasyGauge),
            false);
    }
    if (ecstasyHeart != nullptr)
    {
        SetWorldGameObjectActive(
            GetComponentGameObject(ecstasyHeart),
            false);
    }
    static bool loggedHide = false;
    if (!loggedHide)
    {
        loggedHide = true;
        LogInfo(
            LogCategory_Action,
            L"Action",
            L"Extended mode hid original status/RP, stamina, ecstasy, mental, "
            L"and moisture gauges while retaining the "
            L"system menu root.");
    }
}

void ExtendedMode_HideGameplayUiCompletely()
{
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedInGameUiManager))
    {
        g_extendedInGameUiManager = nullptr;
        g_extendedInGameUiManager = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Object3D.IngameManager",
            "InGameUiManager",
            "Instance");
    }
    if (g_extendedInGameUiManager == nullptr)
    {
        return;
    }
    SetWorldGameObjectActive(
        GetComponentGameObject(g_extendedInGameUiManager),
        false);
}

void ExtendedMode_SilenceGameplayAudio()
{
    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (audioSourceClass == nullptr ||
        objectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(audioSourceClass);
    auto* sourceType = il2cpp_type_get_object(
        il2cpp_class_get_type(audioSourceClass));
    bool includeInactive = true;
    void* parameters[] = { sourceType, &includeInactive };
    auto* sources = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (sources == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(sources);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(sources) +
        il2cpp_array_object_header_size());
    bool muted = true;
    float volume = 0.0f;
    void* muteParameters[] = { &muted };
    void* volumeParameters[] = { &volume };
    int changed = 0;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* source = elements[index];
        if (source == nullptr)
        {
            continue;
        }
        if (MoaningAudio_IsManagedSource(source))
        {
            continue;
        }
        Invoke(
            FindMethod(audioSourceClass, "Stop", 0),
            source,
            nullptr);
        Invoke(
            FindMethod(audioSourceClass, "set_mute", 1),
            source,
            muteParameters);
        Invoke(
            FindMethod(audioSourceClass, "set_volume", 1),
            source,
            volumeParameters);
        ++changed;
    }
    if (changed > 0)
    {
        LogInfo(LogCategory_Action, L"Action", L"Extended mode muted %d original AudioSource(s).", changed);
    }
}

void ExtendedMode_StopSceneParticles()
{
    auto* particleClass = FindClass("UnityEngine", "ParticleSystem");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (particleClass == nullptr ||
        objectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(particleClass);
    auto* particleType = il2cpp_type_get_object(
        il2cpp_class_get_type(particleClass));
    bool includeInactive = true;
    void* parameters[] = { particleType, &includeInactive };
    auto* systems = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (systems == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(systems);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(systems) +
        il2cpp_array_object_header_size());
    auto* stop = FindMethod(particleClass, "Stop", 0);
    auto* clear = FindMethod(particleClass, "Clear", 0);
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* system = elements[index];
        if (system == nullptr)
        {
            continue;
        }
        Invoke(stop, system, nullptr);
        Invoke(clear, system, nullptr);
    }
    LogInfo(LogCategory_Action, L"Action", L"Extended mode stopped %u original scene particle system(s).", count);
}

void ExtendedMode_SetBlackRoomVisible(bool visible)
{
    const wchar_t* objectNames[] = {
        L"EFM_BlackRoomFloor",
        L"EFM_BlackRoomBack",
        L"EFM_BlackRoomLeft",
        L"EFM_BlackRoomRight",
        L"EFM_BlackRoomCeiling",
        L"EFM_KeyLight",
        L"EFM_FillLight",
        L"EFM_RimLight",
    };
    for (const auto* objectName : objectNames)
    {
        auto* gameObject = FindGameObjectByNameToken(objectName);
        if (gameObject != nullptr)
        {
            SetWorldGameObjectActive(gameObject, visible);
        }
    }
}

void GameplayInput_EnsureVibratorAudioClips(
    Il2CppObject* vibratorController,
    int mode)
{
    if (vibratorController == nullptr || mode == 0)
    {
        return;
    }
    auto* lowAudioSource = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(vibratorController) + 0x38);
    auto* highAudioSource = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(vibratorController) + 0x30);
    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    auto* getClip = FindMethod(audioSourceClass, "get_clip", 0);
    if (getClip == nullptr)
    {
        return;
    }
    auto* lowClip =
        lowAudioSource != nullptr
            ? Invoke(getClip, lowAudioSource, nullptr)
            : nullptr;
    auto* highClip =
        highAudioSource != nullptr
            ? Invoke(getClip, highAudioSource, nullptr)
            : nullptr;
    auto* lowSe = reinterpret_cast<int*>(
        reinterpret_cast<char*>(vibratorController) + 0x24);
    auto* highSe = reinterpret_cast<int*>(
        reinterpret_cast<char*>(vibratorController) + 0x20);
    constexpr int kRotorSeType = 96;
    constexpr int kBaibuLowSeType = 28;
    constexpr int kBaibuHighSeType = 29;
    if (lowClip == nullptr && *lowSe == kRotorSeType)
    {
        *lowSe = kBaibuLowSeType;
        LogInfo(LogCategory_Action, L"Action", L"Adult goods vibrator low sound mapped from empty Rotor data to BaibuLow.");
    }
    if (highClip == nullptr && *highSe == kRotorSeType)
    {
        *highSe = kBaibuHighSeType;
        LogInfo(LogCategory_Action, L"Action", L"Adult goods vibrator high sound mapped from empty Rotor data to BaibuHigh.");
    }
}

void GameplayInput_ApplyVibratorAudioVolume(
    Il2CppObject* vibratorController,
    int mode)
{
    if (vibratorController == nullptr || mode == 0)
    {
        return;
    }
    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    auto* setVolume = FindMethod(audioSourceClass, "set_volume", 1);
    if (setVolume == nullptr)
    {
        return;
    }
    constexpr float kPreferredVibratorVolume = 0.8f;
    const int sourceOffsets[] = { 0x38, 0x30 };
    const int smoothVolumeOffsets[] = { 0x48, 0x40 };
    for (int index = 0; index < 2; ++index)
    {
        auto* audioSource = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(vibratorController) +
            sourceOffsets[index]);
        auto* smoothVolume = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(vibratorController) +
            smoothVolumeOffsets[index]);
        if (smoothVolume != nullptr)
        {
            *reinterpret_cast<float*>(
                reinterpret_cast<char*>(smoothVolume) + 0x10) =
                kPreferredVibratorVolume;
            *reinterpret_cast<float*>(
                reinterpret_cast<char*>(smoothVolume) + 0x1C) =
                kPreferredVibratorVolume;
        }
        if (audioSource != nullptr)
        {
            float volume = kPreferredVibratorVolume;
            void* parameters[] = { &volume };
            Invoke(setVolume, audioSource, parameters);
        }
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods vibrator output volume set to %.2f.",
        kPreferredVibratorVolume);
}

void GameplayInput_ApplyVibratorModeToOriginalState(
    int adultGoodsType,
    int mode,
    bool keepModelVisible)
{
    GameplayAnimationDiagnostic_Log(
        L"vibrator-state-before",
        ExtendedMode_GetAnimationManager(),
        g_extendedLastResolvedInputData,
        true);
    GameplayInput_SetOriginalVibrationMode(mode);
    GameplayInput_SetAdultGoodsActiveState(
        adultGoodsType,
        mode != 0);
    auto* vibratorClass = FindClass(
        "ExposureUnnoticed2.Object3D.AdultGoods",
        "CommonVibratorController");
    Il2CppObject* vibratorController = nullptr;
    Il2CppObject* vibratorRoot =
        g_extendedAvatarReferencer != nullptr
            ? EquipmentPreviewUi_GetArrayElement(
                *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<char*>(
                        g_extendedAvatarReferencer) + 0x258),
                adultGoodsType)
            : nullptr;
    if (vibratorRoot != nullptr &&
        vibratorClass != nullptr &&
        il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(vibratorClass);
        auto* vibratorType = il2cpp_type_get_object(
            il2cpp_class_get_type(vibratorClass));
        bool includeInactive = true;
        void* componentParameters[] = {
            vibratorType,
            &includeInactive,
        };
        vibratorController = Invoke(
            FindMethod(
                FindClass("UnityEngine", "GameObject"),
                "GetComponentInChildren",
                2),
            vibratorRoot,
            componentParameters);
        if (vibratorController != nullptr)
        {
            ExtendedMode_SetComponentEnabled(
                vibratorController,
                true);
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(vibratorController) + 0x58) =
                -1;
        }
        auto* leader = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Object3D.AdultGoods",
            "CommonVibratorController",
            "Leader");
        LogInfo(LogCategory_Action, L"Action", 
            L"Adult goods vibrator controller=%p leader=%p before mode=%d.",
            vibratorController,
            leader,
            mode);
    }
    GameplayInput_EnsureVibratorAudioClips(
        vibratorController,
        mode);
    GameplayInput_SetVibrationMode(mode);
    if (vibratorController != nullptr)
    {
        auto* update = FindMethod(
            vibratorClass,
            "Update",
            0);
        if (update != nullptr)
        {
            Invoke(
                update,
                vibratorController,
                nullptr);
        }
    }
    GameplayInput_ApplyVibratorAudioVolume(
        vibratorController,
        mode);
    if (mode == 0 && keepModelVisible)
    {
        EquipmentPreviewUi_ApplyAdultGoodsAttachment(
            adultGoodsType,
            true);
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Adult goods vibrator original state applied (type=%d, mode=%d, keepModel=%d).",
        adultGoodsType,
        mode,
        keepModelVisible ? 1 : 0);
    GameplayAnimationDiagnostic_Log(
        L"vibrator-state-after",
        ExtendedMode_GetAnimationManager(),
        g_extendedLastResolvedInputData,
        true);
}

Il2CppObject* ExtendedMode_GetGameStateData()
{
    return ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameState",
        "GameStateData");
}

int ExtendedMode_GetCurrentStageType()
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData == nullptr)
    {
        return -1;
    }
    return *reinterpret_cast<int*>(
        reinterpret_cast<char*>(gameStateData) + 0x10);
}

int ExtendedMode_GetUnityFrameCount()
{
    auto* timeClass = FindClass("UnityEngine", "Time");
    auto* getFrameCount = timeClass != nullptr
        ? FindMethod(timeClass, "get_frameCount", 0)
        : nullptr;
    auto* value = getFrameCount != nullptr
        ? Invoke(getFrameCount, nullptr, nullptr)
        : nullptr;
    return value != nullptr && il2cpp_object_unbox != nullptr
        ? *static_cast<int*>(il2cpp_object_unbox(value))
        : -1;
}

int ExtendedMode_GetLoadedStageType()
{
    if (g_extendedInGameManager == nullptr)
    {
        return -1;
    }
    auto* stageController = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedInGameManager) + 0x40);
    if (stageController == nullptr)
    {
        return -1;
    }
    return *reinterpret_cast<int*>(
        reinterpret_cast<char*>(stageController) + 0x20);
}

void ExtendedMode_ApplySelectedTime()
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr)
    {
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x20) =
            g_extendedTargetDaytime.load();
    }

    auto* changeTimeEvent = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Scripts.Base",
        "Events",
        "ChangeTimeEvent");
    auto* eventManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "EventManager");
    if (changeTimeEvent != nullptr && eventManagerClass != nullptr)
    {
        void* broadcastParameters[] = { changeTimeEvent };
        Invoke(
            FindMethod(eventManagerClass, "Broadcast", 1),
            nullptr,
            broadcastParameters);
    }

    if (g_extendedInGameManager != nullptr)
    {
        auto* stageController = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameManager) + 0x40);
        if (stageController != nullptr)
        {
            bool isInitialize = true;
            void* updateParameters[] = { &isInitialize };
            Invoke(
                FindMethod(
                    FindClass(
                        "ExposureUnnoticed2.Scripts.InGame",
                        "StageController"),
                    "UpdateByDaytimeNight",
                    1),
                stageController,
                updateParameters);
        }
    }
    g_extendedTimeApplied = true;
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape from Manaka selected %s time for stage %d.",
        g_extendedTargetDaytime.load() ? L"daytime" : L"night",
        g_extendedTargetStageType.load());
}

void ExtendedMode_BroadcastStageEvent(const char* fieldName)
{
    auto* stageEvent = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Scripts.Base",
        "Events",
        fieldName);
    auto* eventManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "EventManager");
    if (stageEvent == nullptr || eventManagerClass == nullptr)
    {
        return;
    }
    void* parameters[] = { stageEvent };
    Invoke(
        FindMethod(eventManagerClass, "Broadcast", 1),
        nullptr,
        parameters);
}

void ExtendedMode_ApplyLoadedStageSpawn()
{
    if (g_extendedSpawnPositionApplied ||
        g_extendedInGameManager == nullptr ||
        g_extendedPlayerGameObject == nullptr)
    {
        return;
    }
    auto* stageController = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedInGameManager) + 0x40);
    if (stageController == nullptr)
    {
        return;
    }
    auto* stageControllerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "StageController");
    Il2CppObject* spawnPoint = nullptr;
    const char* spawnSource = "none";

    // The shipped GetSpawnPoint(fromStageType) method resolves a portal using
    // StageTransController.LastPortalId. Entering from our custom selector does
    // not cross a portal, so that id is stale and can point at a distant or
    // elevated portal. Prefer the scene's first/default portal for a clean
    // deployment, then fall back to the original transition lookup.
    auto* portalManager = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(stageController) + 0xD0);
    if (portalManager != nullptr)
    {
        auto* portalManagerClass = FindClass(
            "ExposureUnnoticed2.Scripts.InGame",
            "PortalManager");
        auto* getPortals = FindMethod(
            portalManagerClass,
            "get_Portals",
            0);
        auto* portals = getPortals != nullptr
            ? Invoke(getPortals, portalManager, nullptr)
            : nullptr;
        if (portals != nullptr)
        {
            auto* portalControllerClass = FindClass(
                "ExposureUnnoticed2.Object3D.Portal",
                "PortalController");
            auto* getPortalSpawn = FindMethod(
                portalControllerClass,
                "get_SpawnPoint",
                0);
            auto* getPortalId = FindMethod(
                portalControllerClass,
                "get_Id",
                0);
            auto* getPortalStage = FindMethod(
                portalControllerClass,
                "get_StageType",
                0);
            auto* getPortalPosition = FindMethod(
                FindClass("UnityEngine", "Transform"),
                "get_position",
                0);
            auto* portalElements = reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(portals) +
                il2cpp_array_object_header_size());
            const auto portalCount = il2cpp_array_length(portals);
            LogInfo(LogCategory_Action, L"Action", 
                L"Escape from Manaka loaded %u portal controller(s).",
                portalCount);
            for (uint32_t index = 0; index < portalCount; ++index)
            {
                auto* portal = portalElements[index];
                auto* portalSpawn = portal != nullptr &&
                    getPortalSpawn != nullptr
                    ? Invoke(getPortalSpawn, portal, nullptr)
                    : nullptr;
                auto* portalPosition = portalSpawn != nullptr &&
                    getPortalPosition != nullptr
                    ? Invoke(getPortalPosition, portalSpawn, nullptr)
                    : nullptr;
                auto* portalId = portal != nullptr &&
                    getPortalId != nullptr
                    ? Invoke(getPortalId, portal, nullptr)
                    : nullptr;
                auto* portalStage = portal != nullptr &&
                    getPortalStage != nullptr
                    ? Invoke(getPortalStage, portal, nullptr)
                    : nullptr;
                if (portalPosition == nullptr)
                {
                    continue;
                }
                const auto* position = static_cast<float*>(
                    il2cpp_object_unbox(portalPosition));
                LogInfo(LogCategory_Action, L"Action", 
                    L"Escape portal %u: id=%d stage=%d spawn=(%.2f, %.2f, %.2f).",
                    index,
                    portalId != nullptr
                        ? *static_cast<int*>(il2cpp_object_unbox(portalId))
                        : -1,
                    portalStage != nullptr
                        ? *static_cast<int*>(il2cpp_object_unbox(portalStage))
                        : -1,
                    position[0],
                    position[1],
                    position[2]);
            }
        }
        auto* getFirstPortal = FindMethod(
            portalManagerClass,
            "GetFirstPortal",
            0);
        auto* firstPortal = getFirstPortal != nullptr
            ? Invoke(getFirstPortal, portalManager, nullptr)
            : nullptr;
        if (firstPortal != nullptr)
        {
            auto* portalControllerClass = FindClass(
                "ExposureUnnoticed2.Object3D.Portal",
                "PortalController");
            auto* getSpawnPoint = FindMethod(
                portalControllerClass,
                "get_SpawnPoint",
                0);
            spawnPoint = getSpawnPoint != nullptr
                ? Invoke(getSpawnPoint, firstPortal, nullptr)
                : nullptr;
            if (spawnPoint != nullptr)
            {
                spawnSource = "first portal";
            }
        }
    }

    auto* getSpawnPoint = FindMethod(
        stageControllerClass,
        "GetSpawnPoint",
        1);
    if (spawnPoint == nullptr && getSpawnPoint != nullptr)
    {
        int fromStageType = g_extendedSourceStageType;
        void* spawnParameters[] = { &fromStageType };
        spawnPoint = Invoke(getSpawnPoint, stageController, spawnParameters);
        if (spawnPoint != nullptr)
        {
            spawnSource = "transition portal";
        }
    }
    if (spawnPoint == nullptr)
    {
        auto* randomPositionManager = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(stageController) + 0xC8);
        auto* getRandomPosition = randomPositionManager != nullptr
            ? FindMethod(
                FindClass(
                    "ExposureUnnoticed2.Scripts.InGame",
                    "StageRandomPositionManager"),
                "GetRandomPosition",
                1)
            : nullptr;
        auto* playerTransform = GetWorldTransform(g_extendedPlayerGameObject);
        auto* playerPosition = playerTransform != nullptr
            ? Invoke(
                FindMethod(FindClass("UnityEngine", "Transform"), "get_position", 0),
                playerTransform,
                nullptr)
            : nullptr;
        if (getRandomPosition != nullptr && playerPosition != nullptr)
        {
            void* rawPlayerPosition = il2cpp_object_unbox(playerPosition);
            void* randomParameters[] = { rawPlayerPosition };
            spawnPoint = Invoke(
                getRandomPosition,
                randomPositionManager,
                randomParameters);
            if (spawnPoint != nullptr)
            {
                spawnSource = "random position";
            }
        }
    }
    if (spawnPoint == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka could not resolve a valid stage spawn point.");
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* position = Invoke(
        FindMethod(transformClass, "get_position", 0),
        spawnPoint,
        nullptr);
    auto* rotation = Invoke(
        FindMethod(transformClass, "get_rotation", 0),
        spawnPoint,
        nullptr);
    auto* playerTransform = GetWorldTransform(g_extendedPlayerGameObject);
    if (position == nullptr || rotation == nullptr || playerTransform == nullptr)
    {
        return;
    }

    auto* characterController = GetSceneComponentByClass(
        g_extendedPlayerGameObject,
        "UnityEngine",
        "CharacterController");
    if (characterController != nullptr)
    {
        bool enabled = false;
        void* enabledParameters[] = { &enabled };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "CharacterController"),
                "set_enabled",
                1),
            characterController,
            enabledParameters);
    }
    void* rawPosition = il2cpp_object_unbox(position);
    void* rawRotation = il2cpp_object_unbox(rotation);
    float checkpointSpawn[3]{};
    if (CheckpointSystem_TryGetSelectedSpawn(
            checkpointSpawn[0],
            checkpointSpawn[1],
            checkpointSpawn[2]))
    {
        rawPosition = checkpointSpawn;
        spawnSource = "checkpoint spawn";
    }
    // Disable the original portal collider before moving the player onto the
    // resolved spawn. Otherwise a checkpoint spawn overlapping a portal can
    // trigger PortalController.OnTriggerEnter during this assignment.
    InteractionFilter_DisablePortalTriggers(false, true);
    void* positionParameters[] = { rawPosition };
    void* rotationParameters[] = { rawRotation };
    Invoke(
        FindMethod(transformClass, "set_position", 1),
        playerTransform,
        positionParameters);
    Invoke(
        FindMethod(transformClass, "set_rotation", 1),
        playerTransform,
        rotationParameters);
    if (characterController != nullptr)
    {
        bool enabled = true;
        void* enabledParameters[] = { &enabled };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "CharacterController"),
                "set_enabled",
                1),
            characterController,
            enabledParameters);
    }
    g_extendedSpawnPositionApplied = true;
    const auto* value = static_cast<float*>(rawPosition);
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape from Manaka placed the player at stage spawn %S (%.2f, %.2f, %.2f).",
        spawnSource,
        value[0],
        value[1],
        value[2]);
    ExtendedMode_BroadcastStageEvent("CompleteTransitionStageEvent");
}

void ExtendedMode_SnapGameplayCameraToPlayer();

void ExtendedMode_DebugTeleportToStationUnderpass()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        g_extendedPlayerGameObject == nullptr)
    {
        return;
    }

    const int loadedStageType = ExtendedMode_GetLoadedStageType();
    if (loadedStageType != 12)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Underpass debug teleport ignored: loaded stage=%d, expected StationFront=12.",
            loadedStageType);
        return;
    }

    auto* playerTransform = GetWorldTransform(g_extendedPlayerGameObject);
    if (playerTransform == nullptr)
    {
        return;
    }

    auto* characterController = GetSceneComponentByClass(
        g_extendedPlayerGameObject,
        "UnityEngine",
        "CharacterController");
    if (characterController != nullptr)
    {
        bool enabled = false;
        void* enabledParameters[] = { &enabled };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "CharacterController"),
                "set_enabled",
                1),
            characterController,
            enabledParameters);
    }

    // This is the authored spawn point of the disabled StageType.Underpass
    // portal in StationFront, not an arbitrary coordinate.
    ExtendedMode_SetTransformPosition(
        playerTransform,
        -232.004f,
        0.15f,
        -40.634f);

    if (characterController != nullptr)
    {
        bool enabled = true;
        void* enabledParameters[] = { &enabled };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "CharacterController"),
                "set_enabled",
                1),
            characterController,
            enabledParameters);
    }

    ExtendedMode_SnapGameplayCameraToPlayer();
    LogInfo(LogCategory_Action, L"Action", 
        L"Underpass debug teleport placed the player at StationFront underpass portal spawn (-232.00, 0.15, -40.63).");
}

void ExtendedMode_LogTransformPosition(
    const wchar_t* label,
    Il2CppObject* object)
{
    auto* transform = GetWorldTransform(object);
    auto* position = transform != nullptr
        ? Invoke(
            FindMethod(FindClass("UnityEngine", "Transform"), "get_position", 0),
            transform,
            nullptr)
        : nullptr;
    if (position == nullptr || il2cpp_object_unbox == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka could not read the %s position.", label);
        return;
    }
    const auto* value = static_cast<float*>(
        il2cpp_object_unbox(position));
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape from Manaka %s position (%.2f, %.2f, %.2f).",
        label,
        value[0],
        value[1],
        value[2]);
}

void ExtendedMode_SnapGameplayCameraToPlayer()
{
    if (g_extendedCamera == nullptr ||
        g_extendedPlayerGameObject == nullptr)
    {
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* playerTransform = GetWorldTransform(g_extendedPlayerGameObject);
    auto* cameraTransform = GetWorldTransform(
        GetComponentGameObject(g_extendedCamera));
    if (playerTransform == nullptr || cameraTransform == nullptr)
    {
        return;
    }
    auto* playerPositionValue = Invoke(
        FindMethod(transformClass, "get_position", 0),
        playerTransform,
        nullptr);
    auto* playerForwardValue = Invoke(
        FindMethod(transformClass, "get_forward", 0),
        playerTransform,
        nullptr);
    auto* playerRightValue = Invoke(
        FindMethod(transformClass, "get_right", 0),
        playerTransform,
        nullptr);
    if (playerPositionValue == nullptr ||
        playerForwardValue == nullptr ||
        playerRightValue == nullptr ||
        il2cpp_object_unbox == nullptr)
    {
        return;
    }
    const auto* playerPosition = static_cast<float*>(
        il2cpp_object_unbox(playerPositionValue));
    const auto* playerForward = static_cast<float*>(
        il2cpp_object_unbox(playerForwardValue));
    const auto* playerRight = static_cast<float*>(
        il2cpp_object_unbox(playerRightValue));
    ExtendedMode_SetTransformPosition(
        cameraTransform,
        playerPosition[0] -
            playerForward[0] * 1.5f +
            playerRight[0] * 4.0f,
        playerPosition[1] + 1.65f,
        playerPosition[2] -
            playerForward[2] * 1.5f +
            playerRight[2] * 4.0f);
    auto* cameraPositionValue = Invoke(
        FindMethod(transformClass, "get_position", 0),
        cameraTransform,
        nullptr);
    auto* cameraPosition = cameraPositionValue != nullptr
        ? static_cast<float*>(il2cpp_object_unbox(cameraPositionValue))
        : nullptr;
    if (cameraPosition != nullptr)
    {
        const float dx = playerPosition[0] - cameraPosition[0];
        const float dy = playerPosition[1] + 1.2f - cameraPosition[1];
        const float dz = playerPosition[2] - cameraPosition[2];
        const float horizontal = std::sqrt(dx * dx + dz * dz);
        constexpr float radiansToDegrees = 57.29577951308232f;
        const float yaw = std::atan2(dx, dz) * radiansToDegrees;
        const float pitch = std::atan2(-dy, horizontal) * radiansToDegrees;
        SetTransformEuler(cameraTransform, pitch, yaw, 0.0f);
    }
    LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka snapped the gameplay camera behind the player.");
}

void ExtendedMode_UpdateCoatModeIndicator()
{
    if (g_extendedCoatModeButton.button == nullptr)
    {
        return;
    }
    const bool coatMode = g_extendedSpecialCoatMode.load();
    SetSceneGraphicColor(
        g_extendedCoatModeButton.image,
        coatMode ? 0.12f : 0.52f,
        coatMode ? 0.48f : 0.10f,
        coatMode ? 0.22f : 0.12f,
        0.96f);
    SetSceneText(
        g_extendedCoatModeButton.text,
        coatMode ? "Q  大衣模式" : "Q  裸露模式",
        16.0f,
        514,
        0.97f,
        0.98f,
        1.0f,
        1.0f);
}

struct ExtendedPoseMenuOption
{
    const char* label;
    int actionType;
    int exposureKind;
};

constexpr int kExtendedPoseExposureNone = 0;
constexpr int kExtendedPoseExposureFront = 1;
constexpr int kExtendedPoseExposureBack = 2;

static constexpr ExtendedPoseMenuOption kExtendedPoseMenuOptions[] = {
    { "叉腿走路",        50000, kExtendedPoseExposureFront },
    { "阿黑颜双V",       50001, kExtendedPoseExposureNone },
    { "扭动屁股",        50002, kExtendedPoseExposureBack },
    { "外八四足爬姿",    50003, kExtendedPoseExposureBack },
    { "开腿口交",        50004, kExtendedPoseExposureFront },
    { "土下座",          50005, kExtendedPoseExposureNone },
    { "丁丁",            50006, kExtendedPoseExposureFront },
    { "I字劈叉平衡",     50007, kExtendedPoseExposureFront },
    { "露腋蹲姿",        50008, kExtendedPoseExposureFront },
    { "三足自慰",        50009, kExtendedPoseExposureFront },
    { "手遮胸",          50010, kExtendedPoseExposureNone },
    { "开腿小便",        50011, kExtendedPoseExposureFront },
    { "狗撒尿",          50012, kExtendedPoseExposureBack },
    { "玩弄乳头",        50013, kExtendedPoseExposureNone },
    { "狗爬式自慰",      50014, kExtendedPoseExposureBack },
    { "仰卧自慰",        50015, kExtendedPoseExposureFront },
    { "自慰",            50016, kExtendedPoseExposureFront },
    { "仰卧单腿抬起",    50017, kExtendedPoseExposureFront },
    { "玩弄阴蒂",        50018, kExtendedPoseExposureNone },
    { "撸管",            50019, kExtendedPoseExposureNone },
    { "摆动腰部",        50020, kExtendedPoseExposureFront },
    { "高叉姿势",        50021, kExtendedPoseExposureFront },
    { "站立小便",        50022, kExtendedPoseExposureNone },
};

constexpr int kExtendedPoseMenuOptionCount =
    sizeof(kExtendedPoseMenuOptions) / sizeof(kExtendedPoseMenuOptions[0]);
constexpr int kExtendedPoseMenuColumns = 6;
constexpr float kExtendedPoseMenuMinX = 0.050f;
constexpr float kExtendedPoseMenuMinY = 0.070f;
constexpr float kExtendedPoseMenuMaxX = 0.950f;
constexpr float kExtendedPoseMenuMaxY = 0.920f;

void ExtendedMode_GetPoseMenuCellRect(
    int index,
    float& minX,
    float& minY,
    float& maxX,
    float& maxY)
{
    constexpr float gridMinX = 0.025f;
    constexpr float cellWidth = 0.148333f;
    constexpr float cellStepX = 0.160333f;
    constexpr float gridTop = 0.840f;
    constexpr float cellHeight = 0.175f;
    constexpr float cellStepY = 0.193f;
    const int column = index % kExtendedPoseMenuColumns;
    const int row = index / kExtendedPoseMenuColumns;
    minX = gridMinX + column * cellStepX;
    maxX = minX + cellWidth;
    maxY = gridTop - row * cellStepY;
    minY = maxY - cellHeight;
}

void ExtendedMode_GetPoseMenuButtonColor(
    int index,
    bool selected,
    float& red,
    float& green,
    float& blue,
    float& alpha)
{
    if (index < 0 || index >= kExtendedPoseMenuOptionCount)
    {
        red = 0.035f;
        green = 0.045f;
        blue = 0.065f;
        alpha = 0.60f;
        return;
    }

    switch (kExtendedPoseMenuOptions[index].exposureKind)
    {
    case kExtendedPoseExposureNone:
        red = selected ? 0.10f : 0.08f;
        green = selected ? 0.55f : 0.34f;
        blue = selected ? 0.24f : 0.16f;
        break;
    case kExtendedPoseExposureFront:
        red = selected ? 0.75f : 0.52f;
        green = selected ? 0.15f : 0.08f;
        blue = selected ? 0.15f : 0.08f;
        break;
    case kExtendedPoseExposureBack:
        red = selected ? 0.68f : 0.48f;
        green = selected ? 0.50f : 0.34f;
        blue = selected ? 0.08f : 0.04f;
        break;
    default:
        red = 0.065f;
        green = 0.085f;
        blue = 0.125f;
        break;
    }
    alpha = 0.98f;
}

void ExtendedMode_BuildPoseMenu()
{
    if (ExtendedMode_IsUnityObjectAlive(g_extendedPoseMenuPanel) ||
        !ExtendedMode_IsUnityObjectAlive(g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    auto* rootTransform = GetWorldTransform(g_extendedGameplayHudCanvasObject);
    auto panel = CreateScenePanel(
        "EscapeFromManakaPoseMenu",
        rootTransform,
        kExtendedPoseMenuMinX,
        kExtendedPoseMenuMinY,
        kExtendedPoseMenuMaxX,
        kExtendedPoseMenuMaxY,
        0.025f,
        0.035f,
        0.055f,
        0.97f);
    g_extendedPoseMenuPanel = panel.gameObject;
    if (g_extendedPoseMenuPanel == nullptr)
    {
        return;
    }
    BringSceneObjectToFront(g_extendedPoseMenuPanel);
    CreateSceneTextElement(
        "PoseMenuTitle",
        panel.rect,
        "原版动作",
        0.035f,
        0.900f,
        0.965f,
        0.982f,
        24.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    CreateSceneTextElement(
        "PoseMenuHint",
        panel.rect,
        "绿=无暴露 · 红=正面 · 黄=背面/臀部 · 按住 E 保持",
        0.035f,
        0.850f,
        0.965f,
        0.900f,
        15.0f,
        514,
        0.68f,
        0.76f,
        0.86f,
        1.0f);
    for (int index = 0; index < 24; ++index)
    {
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        ExtendedMode_GetPoseMenuCellRect(
            index,
            minX,
            minY,
            maxX,
            maxY);
        char objectName[64]{};
        std::snprintf(
            objectName,
            sizeof(objectName),
            "PoseMenuButton%02d",
            index);
        const bool hasAction = index < kExtendedPoseMenuOptionCount;
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        float alpha = 0.0f;
        ExtendedMode_GetPoseMenuButtonColor(
            index,
            false,
            red,
            green,
            blue,
            alpha);
        g_extendedPoseMenuButtons[index] = CreateScenePanel(
            objectName,
            panel.rect,
            minX,
            minY,
            maxX,
            maxY,
            red,
            green,
            blue,
            alpha);
        if (hasAction)
        {
            CreateSceneTextElement(
                "Label",
                g_extendedPoseMenuButtons[index].rect,
                kExtendedPoseMenuOptions[index].label,
                0.03f,
                0.06f,
                0.97f,
                0.94f,
                17.0f,
                514,
                0.92f,
                0.95f,
                1.0f,
                1.0f);
        }
    }
    SetWorldGameObjectActive(g_extendedPoseMenuPanel, false);
    LogInfo(LogCategory_Action, L"Action", L"Escape pose menu built with 23 original player actions.");
}

HWND ExtendedMode_FindInputWindow()
{
    HWND foreground = GetForegroundWindow();
    DWORD processId = 0;
    if (foreground != nullptr)
    {
        GetWindowThreadProcessId(foreground, &processId);
        if (processId == GetCurrentProcessId())
        {
            return foreground;
        }
    }
    return FindGameWindow();
}

void ExtendedMode_RefreshPoseMenuHover()
{
    int hovered = -1;
    const HWND window = ExtendedMode_FindInputWindow();
    RECT clientRect{};
    POINT point{
        g_extendedManualPointerCurrentX.load(),
        g_extendedManualPointerCurrentY.load()};
    if (window != nullptr &&
        GetForegroundWindow() == window &&
        GetClientRect(window, &clientRect) &&
        ScreenToClient(window, &point))
    {
        const float width = static_cast<float>(
            clientRect.right - clientRect.left);
        const float height = static_cast<float>(
            clientRect.bottom - clientRect.top);
        if (width > 0.0f && height > 0.0f)
        {
            const float screenX = static_cast<float>(point.x) / width;
            const float screenY =
                1.0f - static_cast<float>(point.y) / height;
            const float localX =
                (screenX - kExtendedPoseMenuMinX) /
                (kExtendedPoseMenuMaxX - kExtendedPoseMenuMinX);
            const float localY =
                (screenY - kExtendedPoseMenuMinY) /
                (kExtendedPoseMenuMaxY - kExtendedPoseMenuMinY);
            for (int index = 0;
                 index < kExtendedPoseMenuOptionCount;
                 ++index)
            {
                float minX = 0.0f;
                float minY = 0.0f;
                float maxX = 0.0f;
                float maxY = 0.0f;
                ExtendedMode_GetPoseMenuCellRect(
                    index,
                    minX,
                    minY,
                    maxX,
                    maxY);
                if (localX >= minX && localX <= maxX &&
                    localY >= minY && localY <= maxY)
                {
                    hovered = index;
                    break;
                }
            }
        }
    }
    if (hovered == g_extendedPoseMenuHoverIndex)
    {
        return;
    }
    g_extendedPoseMenuHoverIndex = hovered;
    for (int index = 0;
         index < kExtendedPoseMenuOptionCount;
         ++index)
    {
        const bool selected = index == hovered;
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        float alpha = 0.0f;
        ExtendedMode_GetPoseMenuButtonColor(
            index,
            selected,
            red,
            green,
            blue,
            alpha);
        SetSceneGraphicColor(
            g_extendedPoseMenuButtons[index].image,
            red,
            green,
            blue,
            alpha);
    }
}

bool ExtendedMode_PlayOriginalPose(int actionType)
{
    if (g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        auto* playerClassAccessor =
            g_extendedPlayerController != nullptr
                ? *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<char*>(g_extendedPlayerController) +
                    0x40)
                : nullptr;
        auto* playerAction =
            playerClassAccessor != nullptr
                ? *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<char*>(playerClassAccessor) +
                    0x68)
                : nullptr;
        auto* playerActionClass = FindClass(
            "ExposureUnnoticed2.Object3D.Player.Scripts",
            "PlayerAction");
        auto* changeAction = FindMethod(
            playerActionClass,
            "ChangeAction",
            1);
        if (playerAction != nullptr &&
            changeAction != nullptr)
        {
            int action = actionType;
            void* parameters[] = { &action };
            Invoke(
                changeAction,
                playerAction,
                parameters);
            const int observedActionType =
                GameplayInput_GetCurrentActionType();
            LogInfo(
                LogCategory_Action,
                L"Action",
                L"Escape pose menu PlayerAction.ChangeAction type=%d "
                L"accepted=%d observed=%d vibrationMode=%d.",
                actionType,
                observedActionType == actionType ? 1 : 0,
                observedActionType,
                GameplayInput_GetOriginalVibrationMode());
            // The original method owns the strong-vibration rejection and its
            // localized message. Only start the MOD pursuit session when the
            // original PlayerAction actually accepted the requested pose.
            return observedActionType == actionType;
        }
    }
    auto* animationManager = ExtendedMode_GetAnimationManager();
    auto* animationManagerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationManager");
    auto* transAction = FindMethod(
        animationManagerClass,
        "TransAction",
        1);
    if (animationManager == nullptr || transAction == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape pose menu could not resolve the original TransAction pipeline for type=%d.",
            actionType);
        return false;
    }
    void* parameters[] = { &actionType };
    Invoke(transAction, animationManager, parameters);
    LogInfo(
        LogCategory_Action,
        L"Action",
        L"Escape pose menu could not apply original PlayerAction for type=%d.",
        actionType);
    return false;
}

void ExtendedMode_ClosePoseMenu(bool suppressUntilRelease)
{
    if (g_extendedPoseMenuPanel != nullptr)
    {
        SetWorldGameObjectActive(g_extendedPoseMenuPanel, false);
    }
    g_extendedPoseMenuVisible = false;
    ClipCursor(nullptr);
    g_extendedPoseMenuClickRequested = false;
    g_extendedManualPointerDown = false;
    g_extendedPoseMenuHoverIndex = -1;
    g_extendedPoseMenuSuppressedUntilRelease = suppressUntilRelease;
    ExtendedMode_ForceGameplayCursor();
}

void ExtendedMode_OpenPoseMenu()
{
    ExtendedMode_BuildPoseMenu();
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedPoseMenuPanel))
    {
        return;
    }
    g_extendedPoseMenuHoverIndex = -1;
    ExtendedMode_RefreshPoseMenuHover();
    SetWorldGameObjectActive(g_extendedPoseMenuPanel, true);
    BringSceneObjectToFront(g_extendedPoseMenuPanel);
    g_extendedPoseMenuVisible = true;
    ClipCursor(nullptr);
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Action, L"Action", L"Escape pose menu opened by holding E.");
}

void ExtendedMode_TickPoseMenu(
    bool systemMenuActive,
    bool interactionMenuActive)
{
    bool held = g_extendedPoseMenuHeld.load();
    if (held &&
        (GameplayInput_ReadRawAsyncKeyState('E') & 0x8000) == 0)
    {
        held = false;
        g_extendedPoseMenuHeld = false;
    }
    if (!held)
    {
        g_extendedPoseMenuSuppressedUntilRelease = false;
        if (g_extendedPoseMenuVisible.load())
        {
            ExtendedMode_ClosePoseMenu(false);
            LogInfo(LogCategory_Action, L"Action", L"Escape pose menu closed after E release.");
        }
        return;
    }
    if (g_extendedPoseMenuSuppressedUntilRelease)
    {
        return;
    }
    const bool allowed =
        !systemMenuActive &&
        !interactionMenuActive &&
        !ModGuideUi_IsOpen() &&
        !g_extendedOriginalTutorialActive.load() &&
        !g_gameplayInventoryOpen.load() &&
        !CheckpointSystem_IsTaskUiOpen() &&
        !g_extendedSceneSelectorOpen;
    if (!allowed)
    {
        if (g_extendedPoseMenuVisible.load())
        {
            ExtendedMode_ClosePoseMenu(false);
        }
        return;
    }
    if (!g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_OpenPoseMenu();
        return;
    }
    ExtendedMode_RefreshPoseMenuHover();
    ExtendedMode_ForceCursor();
    if (!g_extendedPoseMenuClickRequested.exchange(false))
    {
        return;
    }
    const int selected = g_extendedPoseMenuHoverIndex;
    if (selected < 0 || selected >= kExtendedPoseMenuOptionCount)
    {
        return;
    }
    const int actionType = kExtendedPoseMenuOptions[selected].actionType;
    ExtendedMode_ClosePoseMenu(true);
    g_extendedPoseMenuRequestedAction = actionType;
    LogInfo(
        LogCategory_Action,
        L"Action",
        L"Escape pose menu queued action=%d for the original "
        L"PlayerAction.OnUpdate validation.",
        actionType);
}

constexpr int kExtendedInteractionPromptOwnerNpc = 0;
constexpr int kExtendedInteractionPromptOwnerCheckpoint = 1;
constexpr float kExtendedInteractionPromptMinX = 0.500f;
constexpr float kExtendedInteractionPromptMinY = 0.438f;
constexpr float kExtendedInteractionPromptMaxY = 0.500f;

float ExtendedMode_GetTextPreferredWidth(Il2CppObject* text)
{
    if (text == nullptr)
    {
        return 0.0f;
    }
    auto* value = Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "Text"),
            "get_preferredWidth",
            0),
        text,
        nullptr);
    return value != nullptr && il2cpp_object_unbox != nullptr
        ? *static_cast<float*>(il2cpp_object_unbox(value))
        : 0.0f;
}

void ExtendedMode_UpdateInteractionPromptSize()
{
    if (g_extendedInteractionPromptPanelRect == nullptr ||
        g_extendedInteractionPromptActionRect == nullptr ||
        g_extendedInteractionPromptActionLabel == nullptr ||
        g_extendedInteractionPromptKeyLabel == nullptr ||
        g_extendedInteractionPromptPrefix.rect == nullptr ||
        g_extendedInteractionPromptKeyBadge.rect == nullptr)
    {
        return;
    }

    auto* actionTextClass = FindClass("UnityEngine.UI", "Text");
    int overflow = 1;
    void* overflowParameters[] = { &overflow };
    Invoke(
        FindMethod(actionTextClass, "set_horizontalOverflow", 1),
        g_extendedInteractionPromptActionLabel,
        overflowParameters);

    Invoke(
        FindMethod(
            FindClass("UnityEngine", "Canvas"),
            "ForceUpdateCanvases",
            0),
        nullptr,
        nullptr);

    constexpr float leftPadding = 10.0f;
    constexpr float gapAfterPrefix = 6.0f;
    constexpr float keyBadgePadding = 16.0f;
    constexpr float gapAfterBadge = 8.0f;
    constexpr float rightPadding = 12.0f;

    const float prefixWidth =
        ExtendedMode_GetTextPreferredWidth(
            g_extendedInteractionPromptPrefix.text);
    const float keyWidth =
        ExtendedMode_GetTextPreferredWidth(
            g_extendedInteractionPromptKeyLabel);
    const float actionWidth =
        ExtendedMode_GetTextPreferredWidth(
            g_extendedInteractionPromptActionLabel);
    const float badgeWidth = keyWidth + keyBadgePadding;
    const float panelWidth =
        leftPadding +
        prefixWidth +
        gapAfterPrefix +
        badgeWidth +
        gapAfterBadge +
        actionWidth +
        rightPadding;
    if (panelWidth <= 0.0f)
    {
        return;
    }

    auto* screenClass = FindClass("UnityEngine", "Screen");
    auto* widthValue = Invoke(
        FindMethod(screenClass, "get_width", 0),
        nullptr,
        nullptr);
    const float screenWidth =
        widthValue != nullptr && il2cpp_object_unbox != nullptr
            ? static_cast<float>(
                *static_cast<int*>(
                    il2cpp_object_unbox(widthValue)))
            : 0.0f;
    if (screenWidth <= 0.0f)
    {
        return;
    }

    const float panelWidthNormalized =
        std::clamp(
            panelWidth / screenWidth,
            0.06f,
            0.90f);
    SetSceneRect(
        g_extendedInteractionPromptPanelRect,
        kExtendedInteractionPromptMinX,
        kExtendedInteractionPromptMinY,
        kExtendedInteractionPromptMinX +
            panelWidthNormalized,
        kExtendedInteractionPromptMaxY);

    const float prefixStart = leftPadding / panelWidth;
    const float prefixEnd =
        (leftPadding + prefixWidth) / panelWidth;
    const float badgeStart =
        prefixEnd + gapAfterPrefix / panelWidth;
    const float badgeEnd =
        badgeStart + badgeWidth / panelWidth;
    const float actionStart =
        badgeEnd + gapAfterBadge / panelWidth;
    const float actionEnd =
        actionStart + actionWidth / panelWidth;

    SetSceneRect(
        g_extendedInteractionPromptPrefix.rect,
        prefixStart,
        0.15f,
        prefixEnd,
        0.85f);
    SetSceneRect(
        g_extendedInteractionPromptKeyBadge.rect,
        badgeStart,
        0.18f,
        badgeEnd,
        0.82f);
    SetSceneRect(
        g_extendedInteractionPromptActionRect,
        actionStart,
        0.15f,
        actionEnd,
        0.85f);
}

void ExtendedMode_RefreshInteractionPrompt()
{
    int owner = -1;
    if (g_extendedInteractionPromptStates[
            kExtendedInteractionPromptOwnerCheckpoint].visible)
    {
        owner = kExtendedInteractionPromptOwnerCheckpoint;
    }
    else if (g_extendedInteractionPromptStates[
                 kExtendedInteractionPromptOwnerNpc].visible)
    {
        owner = kExtendedInteractionPromptOwnerNpc;
    }

    if (g_extendedInteractionPromptPanel != nullptr)
    {
        SetWorldGameObjectActive(
            g_extendedInteractionPromptPanel,
            owner >= 0);
        if (owner >= 0)
        {
            BringSceneObjectToFront(
                g_extendedInteractionPromptPanel);
        }
    }
    if (owner < 0)
    {
        return;
    }

    const auto& state =
        g_extendedInteractionPromptStates[owner];
    char keyText[2] = {
        state.key,
        '\0',
    };
    SetSceneText(
        g_extendedInteractionPromptKeyLabel,
        keyText,
        18.0f,
        4,
        0.98f,
        0.98f,
        0.98f,
        1.0f);
    SetSceneText(
        g_extendedInteractionPromptActionLabel,
        state.action.c_str(),
        20.0f,
        514,
        0.93f,
        0.94f,
        0.95f,
        1.0f);
    ExtendedMode_UpdateInteractionPromptSize();
}

void ExtendedMode_SetInteractionPromptState(
    int owner,
    bool visible,
    char key,
    const char* action)
{
    if (owner < kExtendedInteractionPromptOwnerNpc ||
        owner > kExtendedInteractionPromptOwnerCheckpoint)
    {
        return;
    }
    auto& state = g_extendedInteractionPromptStates[owner];
    const std::string nextAction =
        action != nullptr ? action : "";
    const bool changed =
        state.visible != visible ||
        state.key != key ||
        state.action != nextAction;
    state.visible = visible;
    state.key = key;
    state.action = nextAction;
    if (changed)
    {
        ExtendedMode_RefreshInteractionPrompt();
    }
}

void ExtendedMode_SetNpcInteractionPrompt(
    bool visible,
    const char* action)
{
    ExtendedMode_SetInteractionPromptState(
        kExtendedInteractionPromptOwnerNpc,
        visible,
        'T',
        action);
}

void ExtendedMode_SetCheckpointInteractionPrompt(
    bool visible,
    const char* action)
{
    ExtendedMode_SetInteractionPromptState(
        kExtendedInteractionPromptOwnerCheckpoint,
        visible,
        'F',
        action);
}

void ExtendedMode_SetCheckpointEditorPrompt(
    bool visible,
    const char* action)
{
    ExtendedMode_SetInteractionPromptState(
        kExtendedInteractionPromptOwnerCheckpoint,
        visible,
        'B',
        action);
}

void ExtendedMode_BuildGameplayHud()
{
    // Font assets are scene-owned and are deliberately released before each
    // scene load. Reacquire them before creating any legacy UI.Text labels;
    // otherwise the interaction frames remain visible but their text is blank.
    ExtendedMode_CacheFont();

    if (!ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        // The old HUD and every child handle died with the previous scene.
        // Drop those raw pointers before any later compass validation.
        CheckpointCompass_Reset();
        g_extendedGameplayHudCanvasObject = nullptr;
        g_extendedCoordinateLabel = nullptr;
        g_extendedInteractionPromptPanel = nullptr;
        g_extendedInteractionPromptPanelRect = nullptr;
        g_extendedInteractionPromptActionRect = nullptr;
        g_extendedInteractionPromptActionLabel = nullptr;
        g_extendedInteractionPromptKeyLabel = nullptr;
        g_extendedInteractionPromptPrefix = SceneUiElement{};
        g_extendedInteractionPromptKeyBadge = SceneUiElement{};
        g_extendedCoatModeButton = SceneUiElement{};
        g_extendedPoseMenuPanel = nullptr;
        for (auto& button : g_extendedPoseMenuButtons)
        {
            button = SceneUiElement{};
        }
    }
    else
    {
        SetWorldGameObjectActive(g_extendedGameplayHudCanvasObject, true);
        auto* gameplayHudTransform =
            GetWorldTransform(g_extendedGameplayHudCanvasObject);
        GameplayMinimap_Build(gameplayHudTransform);
        GameplayWorldMap_Build(gameplayHudTransform);
        ExtendedMode_UpdateCoatModeIndicator();
        GameplayStatusHud_Build();
        ExtendedMode_BuildPoseMenu();
        return;
    }

    g_extendedGameplayHudCanvasObject = CreateSceneGameObject(
        "EscapeFromManakaGameplayHud",
        nullptr);
    if (g_extendedGameplayHudCanvasObject == nullptr)
    {
        return;
    }
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (objectClass != nullptr)
    {
        void* dontDestroyParameters[] = { g_extendedGameplayHudCanvasObject };
        Invoke(
            FindMethod(objectClass, "DontDestroyOnLoad", 1),
            nullptr,
            dontDestroyParameters);
    }
    auto* canvasRect = GetSceneComponentByClass(
        g_extendedGameplayHudCanvasObject,
        "UnityEngine",
        "RectTransform");
    auto* canvas = AddSceneComponentToGameObject(
        g_extendedGameplayHudCanvasObject,
        "UnityEngine",
        "Canvas");
    auto* scaler = AddSceneComponentToGameObject(
        g_extendedGameplayHudCanvasObject,
        "UnityEngine.UI",
        "CanvasScaler");
    AddSceneComponentToGameObject(
        g_extendedGameplayHudCanvasObject,
        "UnityEngine.UI",
        "GraphicRaycaster");
    int renderMode = 0;
    int sortingOrder = kExtendedGameplayHudSortingOrder;
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

    if (scaler != nullptr)
    {
        auto* screenClass = FindClass("UnityEngine", "Screen");
        auto* widthObject = Invoke(
            FindMethod(screenClass, "get_width", 0),
            nullptr,
            nullptr);
        const int unityWidth = widthObject != nullptr
            ? *static_cast<int*>(il2cpp_object_unbox(widthObject))
            : 0;
        const HWND gameWindow = FindGameWindow();
        RECT clientRect{};
        const int clientWidth =
            gameWindow != nullptr && GetClientRect(gameWindow, &clientRect)
                ? clientRect.right - clientRect.left
                : 0;
        if (unityWidth > 0 && clientWidth > 0)
        {
            float scaleFactor =
                static_cast<float>(clientWidth) /
                static_cast<float>(unityWidth);
            void* scaleParameters[] = { &scaleFactor };
            Invoke(
                FindMethod(
                    FindClass("UnityEngine.UI", "CanvasScaler"),
                    "set_scaleFactor",
                    1),
                scaler,
                scaleParameters);
        }
    }

    auto* rootTransform = GetWorldTransform(g_extendedGameplayHudCanvasObject);
    auto panel = CreateScenePanel(
        "CoordinatePanel",
        rootTransform,
        0.690f,
        0.018f,
        0.976f,
        0.082f,
        0.018f,
        0.022f,
        0.028f,
        0.76f);
    g_extendedCoordinateLabel = CreateSceneTextElement(
        "PlayerWorldPosition",
        panel.rect,
        "POS  X +000.00   Y +000.00   Z +000.00",
        0.035f,
        0.08f,
        0.965f,
        0.92f,
        16.0f,
        514,
        0.86f,
        0.91f,
        0.96f,
        1.0f).text;
    auto interactionPrompt = CreateScenePanel(
        "GameplayInteractionPrompt",
        rootTransform,
        kExtendedInteractionPromptMinX,
        kExtendedInteractionPromptMinY,
        0.650f,
        kExtendedInteractionPromptMaxY,
        0.045f,
        0.045f,
        0.048f,
        0.72f);
    g_extendedInteractionPromptPanel =
        interactionPrompt.gameObject;
    g_extendedInteractionPromptPanelRect =
        interactionPrompt.rect;
    g_extendedInteractionPromptPrefix = CreateSceneTextElement(
        "GameplayInteractionPromptPrefix",
        interactionPrompt.rect,
        "按",
        0.035f,
        0.15f,
        0.145f,
        0.85f,
        18.0f,
        514,
        0.82f,
        0.84f,
        0.87f,
        1.0f);
    g_extendedInteractionPromptKeyBadge = CreateScenePanel(
        "GameplayInteractionPromptKeyBadge",
        interactionPrompt.rect,
        0.155f,
        0.18f,
        0.305f,
        0.82f,
        0.16f,
        0.17f,
        0.19f,
        0.96f);
    g_extendedInteractionPromptKeyLabel =
        CreateSceneTextElement(
            "GameplayInteractionPromptKey",
            g_extendedInteractionPromptKeyBadge.rect,
            "",
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            18.0f,
            4,
            0.98f,
            0.98f,
            0.98f,
            1.0f).text;
    g_extendedInteractionPromptActionLabel =
        CreateSceneTextElement(
            "GameplayInteractionPromptAction",
            interactionPrompt.rect,
            "",
            0.335f,
            0.15f,
            0.965f,
            0.85f,
            20.0f,
            514,
            0.93f,
            0.94f,
            0.95f,
            1.0f).text;
    g_extendedInteractionPromptActionRect =
        GetSceneComponentByClass(
            g_extendedInteractionPromptActionLabel != nullptr
                ? GetComponentGameObject(
                    g_extendedInteractionPromptActionLabel)
                : nullptr,
            "UnityEngine",
            "RectTransform");
    SetWorldGameObjectActive(
        g_extendedInteractionPromptPanel,
        false);
    GameplayStatusHud_Build();
    g_extendedCoatModeButton = CreateSceneButton(
        "CoatModeIndicator",
        rootTransform,
        "Q  裸露模式",
        0.835f,
        0.730f,
        0.976f,
        0.795f,
        0.52f,
        0.10f,
        0.12f,
        0.96f,
        16.0f,
        514,
        false);
    ExtendedMode_UpdateCoatModeIndicator();
    ExtendedMode_BuildPoseMenu();
    ExtendedMode_RefreshInteractionPrompt();
    GameplayMinimap_Build(rootTransform);
    GameplayWorldMap_Build(rootTransform);
    LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka gameplay coordinate HUD created.");
}

void ExtendedMode_UpdateGameplayCoordinates()
{
    if (g_extendedCoordinateLabel == nullptr)
    {
        return;
    }
    auto* playerObject = g_extendedPlayerGameObject != nullptr
        ? g_extendedPlayerGameObject
        : g_extendedAvatarRoot;
    auto* transform = GetWorldTransform(playerObject);
    if (transform == nullptr)
    {
        return;
    }
    auto* positionValue = Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "get_position", 0),
        transform,
        nullptr);
    if (positionValue == nullptr || il2cpp_object_unbox == nullptr)
    {
        return;
    }
    const auto* position = static_cast<float*>(
        il2cpp_object_unbox(positionValue));
    char text[160]{};
    std::snprintf(
        text,
        sizeof(text),
        "POS  X %+08.2f   Y %+08.2f   Z %+08.2f",
        position[0],
        position[1],
        position[2]);
    SetSceneText(
        g_extendedCoordinateLabel,
        text,
        16.0f,
        514,
        0.86f,
        0.91f,
        0.96f,
        1.0f);
}

void ExtendedMode_RefreshSceneSelector()
{
    if (g_extendedSelectedStageIndex < 0 ||
        g_extendedSelectedStageIndex >= kEscapeFromManakaStageCount)
    {
        g_extendedSelectedStageIndex = 0;
    }
    for (int index = 0; index < kEscapeFromManakaStageCount; ++index)
    {
        const bool active = index == g_extendedSelectedStageIndex;
        SetSceneGraphicColor(
            g_extendedSceneButtons[index].image,
            active ? 0.52f : 0.075f,
            active ? 0.31f : 0.090f,
            active ? 0.12f : 0.110f,
            0.99f);
        SetSceneText(
            g_extendedSceneButtons[index].text,
            kExtendedStageOptions[index].label,
            16.0f,
            514,
            active ? 1.0f : 0.78f,
            active ? 1.0f : 0.84f,
            active ? 1.0f : 0.90f,
            1.0f);
    }
    SetSceneGraphicColor(
        g_extendedDayButton.image,
        g_extendedSelectedDaytime ? 0.16f : 0.070f,
        g_extendedSelectedDaytime ? 0.44f : 0.082f,
        g_extendedSelectedDaytime ? 0.42f : 0.100f,
        0.99f);
    SetSceneGraphicColor(
        g_extendedNightButton.image,
        g_extendedSelectedDaytime ? 0.070f : 0.24f,
        g_extendedSelectedDaytime ? 0.082f : 0.15f,
        g_extendedSelectedDaytime ? 0.100f : 0.42f,
        0.99f);
    if (g_extendedSceneSelectionHint != nullptr)
    {
        char text[192]{};
        std::snprintf(
            text,
            sizeof(text),
            "已选择：%s   ·   %s",
            kExtendedStageOptions[g_extendedSelectedStageIndex].label,
            g_extendedSelectedDaytime ? "白天 DAY" : "夜晚 NIGHT");
        SetSceneText(
            g_extendedSceneSelectionHint,
            text,
            17.0f,
            514,
            0.82f,
            0.87f,
            0.93f,
            1.0f);
    }
}

void ExtendedMode_CloseSceneSelector()
{
    g_extendedSceneSelectorOpen = false;
    if (g_extendedSceneSelectorPanel.gameObject != nullptr)
    {
        SetWorldGameObjectActive(g_extendedSceneSelectorPanel.gameObject, false);
    }
    ExtendedMode_ForceCursor();
}

void ExtendedMode_OpenSceneSelector()
{
    if (g_extendedModeState.load() != ExtendedMode_Active ||
        g_extendedSceneSelectorPanel.gameObject == nullptr)
    {
        return;
    }
    if (EquipmentPreviewUi_IsOpen())
    {
        EquipmentPreviewUi_Close();
    }
    g_extendedActionListOpen = false;
    if (g_extendedActionPanel != nullptr)
    {
        SetWorldGameObjectActive(g_extendedActionPanel, false);
    }
    g_extendedSceneSelectorOpen = true;
    SetWorldGameObjectActive(g_extendedSceneSelectorPanel.gameObject, true);
    BringSceneObjectToFront(g_extendedSceneSelectorPanel.gameObject);
    ExtendedMode_RefreshSceneSelector();
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka scene selector opened.");
}

void ExtendedMode_ResetDynamicBones()
{
    auto* dynamicBoneClass = FindClass("", "DynamicBone");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedAvatarRoot) ||
        dynamicBoneClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(dynamicBoneClass);
    auto* dynamicBoneType = il2cpp_type_get_object(
        il2cpp_class_get_type(dynamicBoneClass));
    bool includeInactive = true;
    void* findParameters[] = { dynamicBoneType, &includeInactive };
    auto* components = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_extendedAvatarRoot,
        findParameters);
    if (components == nullptr)
    {
        return;
    }
    auto* setupParticles = FindMethod(
        dynamicBoneClass,
        "SetupParticles",
        0);
    auto* resetParticles = FindMethod(
        dynamicBoneClass,
        "ResetParticlesPosition",
        0);
    auto* setWeight = FindMethod(
        dynamicBoneClass,
        "SetWeight",
        1);
    const auto count = il2cpp_array_length(components);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(components) +
        il2cpp_array_object_header_size());
    int resetCount = 0;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* component = elements[index];
        if (component == nullptr)
        {
            continue;
        }
        Invoke(setupParticles, component, nullptr);
        Invoke(resetParticles, component, nullptr);
        float weight = 0.0f;
        void* weightParameters[] = { &weight };
        Invoke(setWeight, component, weightParameters);
        ++resetCount;
    }
    LogInfo(LogCategory_Action, L"Action", L"Extended mode reset %d dynamic-bone chain(s).", resetCount);
}

void ExtendedMode_PrepareSkinnedRenderers()
{
    static unsigned int prepareCalls = 0;
    auto* rendererClass = FindClass("UnityEngine", "SkinnedMeshRenderer");
    auto* rendererBaseClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedAvatarRoot) ||
        rendererClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_extendedAvatarRoot,
        parameters);
    if (renderers == nullptr)
    {
        return;
    }
    auto* updateWhenOffscreen = FindMethod(
        rendererClass,
        "set_updateWhenOffscreen",
        1);
    auto* getRendererEnabled = FindMethod(
        rendererBaseClass,
        "get_enabled",
        0);
    auto* setForceRenderingOff = FindMethod(
        rendererBaseClass,
        "set_forceRenderingOff",
        1);
    bool updateOffscreen = true;
    void* updateOffscreenParameters[] = { &updateOffscreen };
    bool forceRenderingOff = false;
    void* forceRenderingOffParameters[] = { &forceRenderingOff };
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    int changed = 0;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        auto* enabledValue = renderer != nullptr &&
            getRendererEnabled != nullptr
            ? Invoke(getRendererEnabled, renderer, nullptr)
            : nullptr;
        if (enabledValue == nullptr ||
            il2cpp_object_unbox == nullptr ||
            !*static_cast<bool*>(il2cpp_object_unbox(enabledValue)))
        {
            continue;
        }
        Invoke(
            updateWhenOffscreen,
            renderer,
            updateOffscreenParameters);
        Invoke(
            setForceRenderingOff,
            renderer,
            forceRenderingOffParameters);
        ++changed;
    }
    if (++prepareCalls <= 2)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Extended mode prepared %d already-enabled renderer(s) without enabling hidden variants.",
            changed);
    }
}

void ExtendedMode_LogAvatarRendererState()
{
    auto* rendererClass = FindClass("UnityEngine", "SkinnedMeshRenderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedAvatarRoot) ||
        rendererClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_extendedAvatarRoot,
        parameters);
    if (renderers == nullptr)
    {
        return;
    }
    auto* getIsVisible = FindMethod(rendererClass, "get_isVisible", 0);
    auto* rendererBaseClass = FindClass("UnityEngine", "Renderer");
    auto* getRendererEnabled = FindMethod(
        rendererBaseClass,
        "get_enabled",
        0);
    auto* getForceRenderingOff = FindMethod(
        rendererBaseClass,
        "get_forceRenderingOff",
        0);
    auto* getSharedMesh = FindMethod(
        rendererClass,
        "get_sharedMesh",
        0);
    auto* getBounds = FindMethod(rendererClass, "get_bounds", 0);
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* getTransform = FindMethod(
        FindClass("UnityEngine", "Component"),
        "get_transform",
        0);
    auto* getPosition = FindMethod(transformClass, "get_position", 0);
    auto* getLossyScale = FindMethod(transformClass, "get_lossyScale", 0);
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    int visibleCount = 0;
    int enabledCount = 0;
    int forceOffCount = 0;
    int meshCount = 0;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* visibleValue = Invoke(
            getIsVisible,
            elements[index],
            nullptr);
        if (visibleValue != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(visibleValue)))
        {
            ++visibleCount;
        }
        auto* enabledValue = Invoke(
            getRendererEnabled,
            elements[index],
            nullptr);
        if (enabledValue != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(enabledValue)))
        {
            ++enabledCount;
        }
        auto* forceOffValue = Invoke(
            getForceRenderingOff,
            elements[index],
            nullptr);
        if (forceOffValue != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(forceOffValue)))
        {
            ++forceOffCount;
        }
        auto* meshValue = Invoke(
            getSharedMesh,
            elements[index],
            nullptr);
        if (meshValue != nullptr)
        {
            ++meshCount;
        }
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape from Manaka avatar rendering: name=%s renderers=%u visible=%d enabled=%d forceOff=%d meshes=%d.",
        GetObjectName(g_extendedAvatarRoot).c_str(),
        count,
        visibleCount,
        enabledCount,
        forceOffCount,
        meshCount);
    const uint32_t sampleCount = count < 5 ? count : 5;
    for (uint32_t index = 0; index < sampleCount; ++index)
    {
        auto* rendererTransform = Invoke(
            getTransform,
            elements[index],
            nullptr);
        auto* positionValue = rendererTransform != nullptr
            ? Invoke(getPosition, rendererTransform, nullptr)
            : nullptr;
        auto* scaleValue = rendererTransform != nullptr
            ? Invoke(getLossyScale, rendererTransform, nullptr)
            : nullptr;
        if (positionValue == nullptr || scaleValue == nullptr)
        {
            continue;
        }
        const auto* position = static_cast<float*>(
            il2cpp_object_unbox(positionValue));
        const auto* scale = static_cast<float*>(
            il2cpp_object_unbox(scaleValue));
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape avatar renderer %u: position=(%.2f, %.2f, %.2f) scale=(%.2f, %.2f, %.2f).",
            index,
            position[0],
            position[1],
            position[2],
            scale[0],
            scale[1],
            scale[2]);
    }
    if (count > 0)
    {
        auto* bounds = Invoke(getBounds, elements[0], nullptr);
        if (bounds != nullptr)
        {
            const auto* values = static_cast<float*>(
                il2cpp_object_unbox(bounds));
            LogInfo(LogCategory_Action, L"Action", 
                L"Escape from Manaka primary avatar bounds center=(%.2f, %.2f, %.2f) extents=(%.2f, %.2f, %.2f).",
                values[0],
                values[1],
                values[2],
                values[3],
                values[4],
                values[5]);
        }
    }
}

void ExtendedMode_RefreshActionPage()
{
    MoaningAudio_RefreshDebugPanel();
}

void ExtendedMode_UpdateSelectedActionLabel()
{
    if (g_extendedSelectedActionLabel == nullptr)
    {
        return;
    }
    char text[256]{};
    std::snprintf(
        text,
        sizeof(text),
        "当前动作  %s",
        kEscapeFromManakaActions[g_extendedSelectedAction].label);
    SetSceneText(
        g_extendedSelectedActionLabel,
        text,
        17.0f,
        257,
        0.92f,
        0.94f,
        0.96f,
        1.0f);
}

void ExtendedMode_BuildUi()
{
    ExtendedMode_CacheFont();

    if (!ExtendedMode_IsUnityObjectAlive(g_extendedCanvas))
    {
        g_extendedCanvas = nullptr;
        g_extendedActionPanel = nullptr;
        g_extendedSelectedActionLabel = nullptr;
        g_extendedPageLabel = nullptr;
        g_extendedPrevPageButton = nullptr;
        g_extendedNextPageButton = nullptr;
        g_extendedActionButtons.clear();
        g_extendedActionLabels.clear();
        g_extendedEnterGameButton = SceneUiElement{};
        g_extendedSceneSelectorPanel = SceneUiElement{};
        for (int index = 0; index < kEscapeFromManakaStageCount; ++index)
        {
            g_extendedSceneButtons[index] = SceneUiElement{};
        }
        g_extendedDayButton = SceneUiElement{};
        g_extendedNightButton = SceneUiElement{};
        g_extendedConfirmEnterButton = SceneUiElement{};
        g_extendedCancelEnterButton = SceneUiElement{};
        g_extendedSceneSelectionHint = nullptr;
        InventoryUi_Reset();
    }
    else
    {
        return;
    }
    if (g_tmpFontAsset == nullptr)
    {
        ExtendedMode_CacheFont();
        LogInfo(LogCategory_Action, L"Action", 
            g_tmpFontAsset != nullptr
                ? L"Escape from Manaka recovered the title font before gameplay UI creation."
                : L"Escape from Manaka is building gameplay UI without a cached TMP font.");
    }
    g_extendedCanvas = CreateSceneGameObject("EscapeFromManakaGameplayCanvas", nullptr);
    if (g_extendedCanvas == nullptr)
    {
        return;
    }
    auto* canvasRect = GetSceneComponentByClass(
        g_extendedCanvas,
        "UnityEngine",
        "RectTransform");
    auto* canvas = AddSceneComponentToGameObject(
        g_extendedCanvas,
        "UnityEngine",
        "Canvas");
    auto* scaler = AddSceneComponentToGameObject(
        g_extendedCanvas,
        "UnityEngine.UI",
        "CanvasScaler");
    AddSceneComponentToGameObject(
        g_extendedCanvas,
        "UnityEngine.UI",
        "GraphicRaycaster");
    int renderMode = 0;
    int sortingOrder = kExtendedCanvasSortingOrder;
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

    auto* screenClass = FindClass("UnityEngine", "Screen");
    auto* widthObject = Invoke(
        FindMethod(screenClass, "get_width", 0),
        nullptr,
        nullptr);
    const int unityWidth = widthObject != nullptr
        ? *static_cast<int*>(il2cpp_object_unbox(widthObject))
        : 0;
    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    const int clientWidth =
        gameWindow != nullptr && GetClientRect(gameWindow, &clientRect)
            ? clientRect.right - clientRect.left
            : 0;
    if (scaler != nullptr && unityWidth > 0 && clientWidth > 0)
    {
        float scaleFactor =
            static_cast<float>(clientWidth) /
            static_cast<float>(unityWidth);
        void* scaleParameters[] = { &scaleFactor };
        Invoke(
            FindMethod(FindClass("UnityEngine.UI", "CanvasScaler"), "set_scaleFactor", 1),
            scaler,
            scaleParameters);
    }

    auto* rootTransform = GetWorldTransform(g_extendedCanvas);
    auto header = CreateScenePanel(
        "EFM_Header",
        rootTransform,
        0.024f,
        0.902f,
        0.976f,
        0.974f,
        0.025f,
        0.030f,
        0.038f,
        0.96f);
    CreateSceneTextElement(
        "EFM_Title",
        header.rect,
        "ESCAPE FROM MANAKA",
        0.018f,
        0.12f,
        0.238f,
        0.88f,
        21.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    g_extendedEnterGameButton = CreateSceneButton(
        "EFM_EnterGame",
        header.rect,
        "进入游戏",
        0.785f,
        0.12f,
        0.975f,
        0.88f,
        0.16f,
        0.36f,
        0.20f,
        1.0f,
        17.0f,
        514,
        true);

    InventoryUi_Build(rootTransform, header.rect);

    g_extendedSceneSelectorPanel = CreateScenePanel(
        "EFM_SceneSelector",
        rootTransform,
        0.145f,
        0.085f,
        0.855f,
        0.888f,
        0.018f,
        0.022f,
        0.030f,
        0.985f);
    BringSceneObjectToFront(g_extendedSceneSelectorPanel.gameObject);
    CreateSceneTextElement(
        "EFM_SceneSelectorTitle",
        g_extendedSceneSelectorPanel.rect,
        "进入游戏",
        0.045f,
        0.910f,
        0.955f,
        0.975f,
        24.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    CreateSceneTextElement(
        "EFM_SceneSelectorSubtitle",
        g_extendedSceneSelectorPanel.rect,
        "选择场景与时间。确认后将隐藏所有界面，只保留 WASD、鼠标视角与 Esc。",
        0.045f,
        0.850f,
        0.955f,
        0.905f,
        14.0f,
        257,
        0.68f,
        0.74f,
        0.82f,
        1.0f);

    for (int index = 0; index < kEscapeFromManakaStageCount; ++index)
    {
        const int column = index % 2;
        const int row = index / 2;
        const float minX = 0.055f + column * 0.455f;
        const float minY = 0.680f - row * 0.120f;
        char objectName[64]{};
        std::snprintf(
            objectName,
            sizeof(objectName),
            "EFM_SceneOption%d",
            index);
        g_extendedSceneButtons[index] = CreateSceneButton(
            objectName,
            g_extendedSceneSelectorPanel.rect,
            kExtendedStageOptions[index].label,
            minX,
            minY,
            minX + 0.405f,
            minY + 0.092f,
            0.075f,
            0.090f,
            0.110f,
            0.99f,
            16.0f,
            514,
            true);
    }

    g_extendedDayButton = CreateSceneButton(
        "EFM_TimeDay",
        g_extendedSceneSelectorPanel.rect,
        "白天  DAY",
        0.055f,
        0.180f,
        0.260f,
        0.270f,
        0.070f,
        0.082f,
        0.100f,
        0.99f,
        16.0f,
        514,
        true);
    g_extendedNightButton = CreateSceneButton(
        "EFM_TimeNight",
        g_extendedSceneSelectorPanel.rect,
        "夜晚  NIGHT",
        0.275f,
        0.180f,
        0.480f,
        0.270f,
        0.070f,
        0.082f,
        0.100f,
        0.99f,
        16.0f,
        514,
        true);
    g_extendedSceneSelectionHint = CreateSceneTextElement(
        "EFM_SceneSelectionHint",
        g_extendedSceneSelectorPanel.rect,
        "",
        0.505f,
        0.185f,
        0.945f,
        0.265f,
        17.0f,
        514,
        0.82f,
        0.87f,
        0.93f,
        1.0f).text;
    g_extendedCancelEnterButton = CreateSceneButton(
        "EFM_CancelEnter",
        g_extendedSceneSelectorPanel.rect,
        "返回",
        0.545f,
        0.055f,
        0.715f,
        0.140f,
        0.10f,
        0.12f,
        0.15f,
        0.99f,
        17.0f,
        514,
        true);
    g_extendedConfirmEnterButton = CreateSceneButton(
        "EFM_ConfirmEnter",
        g_extendedSceneSelectorPanel.rect,
        "进入游戏",
        0.735f,
        0.055f,
        0.945f,
        0.140f,
        0.16f,
        0.36f,
        0.20f,
        1.0f,
        18.0f,
        514,
        true);
    SetWorldGameObjectActive(g_extendedSceneSelectorPanel.gameObject, false);
    ExtendedMode_RefreshSceneSelector();

    auto actionPanel = CreateScenePanel(
        "EFM_ActionPanel",
        rootTransform,
        0.505f,
        0.105f,
        0.976f,
        0.865f,
        0.020f,
        0.025f,
        0.032f,
        0.985f);
    g_extendedActionPanel = actionPanel.gameObject;
    CreateSceneTextElement(
        "EFM_ActionTitle",
        actionPanel.rect,
        "MOANING3 AUDIO TEST  ·  ~ CLOSE\n"
        "Click an individual WAV file to play it.",
        0.045f,
        0.925f,
        0.955f,
        0.985f,
        19.0f,
        257,
        0.94f,
        0.96f,
        0.98f,
        1.0f);
    auto previous = CreateSceneButton(
        "EFM_PreviousAudioPage",
        actionPanel.rect,
        "PREV",
        0.045f,
        0.845f,
        0.235f,
        0.915f,
        0.10f,
        0.13f,
        0.18f,
        1.0f,
        16.0f,
        514,
        true);
    auto next = CreateSceneButton(
        "EFM_NextAudioPage",
        actionPanel.rect,
        "NEXT",
        0.765f,
        0.845f,
        0.955f,
        0.915f,
        0.10f,
        0.13f,
        0.18f,
        1.0f,
        16.0f,
        514,
        true);
    g_extendedPrevPageButton = previous.button;
    g_extendedNextPageButton = next.button;
    g_extendedPageLabel = CreateSceneTextElement(
        "EFM_PageLabel",
        actionPanel.rect,
        "",
        0.25f,
        0.845f,
        0.75f,
        0.915f,
        15.0f,
        514,
        0.78f,
        0.84f,
        0.90f,
        1.0f).text;

    for (int index = 0;
         index < kMoaningAudioFilesPerPage;
         ++index)
    {
        const int row = index;
        const float minX = 0.045f;
        const float minY = 0.775f - row * 0.078f;
        const float maxX = 0.495f;
        const float maxY = minY + 0.065f;
        char objectName[64]{};
        std::snprintf(
            objectName,
            sizeof(objectName),
            "EFM_AudioButton%d",
            index);
        auto button = CreateSceneButton(
            objectName,
            actionPanel.rect,
            "FILE",
            minX,
            minY,
            maxX,
            maxY,
            0.075f,
            0.095f,
            0.125f,
            1.0f,
            12.0f,
            514,
            true);
        g_extendedActionButtons.push_back(button.button);
        g_extendedActionLabels.push_back(button.text);
    }

    SetWorldGameObjectActive(g_extendedActionPanel, false);
    ExtendedMode_RefreshActionPage();
    ExtendedMode_UpdateSelectedActionLabel();
    LogInfo(LogCategory_Action, L"Action", L"Extended mode created its independent gameplay UI.");
}

bool ExtendedMode_IsGameplayActive()
{
    return g_extendedGameplayActive.load();
}

bool ExtendedMode_ShouldBlockUiActions()
{
    return g_extendedSceneSelectorOpen ||
           g_extendedGameplayActive.load();
}

bool ExtendedMode_HandleButtonClick(Il2CppObject* instance)
{
    if (instance == nullptr)
    {
        return false;
    }
    if (instance == g_extendedEnterGameButton.button)
    {
        ExtendedMode_OpenSceneSelector();
        return true;
    }
    if (!g_extendedSceneSelectorOpen)
    {
        return false;
    }
    for (int index = 0; index < kEscapeFromManakaStageCount; ++index)
    {
        if (instance == g_extendedSceneButtons[index].button)
        {
            g_extendedSelectedStageIndex = index;
            ExtendedMode_RefreshSceneSelector();
            return true;
        }
    }
    if (instance == g_extendedDayButton.button)
    {
        g_extendedSelectedDaytime = true;
        ExtendedMode_RefreshSceneSelector();
        return true;
    }
    if (instance == g_extendedNightButton.button)
    {
        g_extendedSelectedDaytime = false;
        ExtendedMode_RefreshSceneSelector();
        return true;
    }
    if (instance == g_extendedCancelEnterButton.button)
    {
        ExtendedMode_CloseSceneSelector();
        return true;
    }
    if (instance == g_extendedConfirmEnterButton.button)
    {
        g_extendedTargetStageType =
            kExtendedStageOptions[g_extendedSelectedStageIndex].stageType;
        g_extendedTargetDaytime = g_extendedSelectedDaytime;
        g_extendedEnterGameRequested = true;
        return true;
    }
    // While the modal is open, no underlying inventory or route button may run.
    return true;
}

void ExtendedMode_RestoreGameplayAudio()
{
    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (audioSourceClass == nullptr ||
        objectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(audioSourceClass);
    auto* sourceType = il2cpp_type_get_object(
        il2cpp_class_get_type(audioSourceClass));
    bool includeInactive = true;
    void* parameters[] = { sourceType, &includeInactive };
    auto* sources = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (sources == nullptr)
    {
        return;
    }
    bool muted = false;
    float volume = 1.0f;
    void* muteParameters[] = { &muted };
    void* volumeParameters[] = { &volume };
    const auto count = il2cpp_array_length(sources);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(sources) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* source = elements[index];
        if (source == nullptr)
        {
            continue;
        }
        if (MoaningAudio_IsManagedSource(source))
        {
            continue;
        }
        Invoke(
            FindMethod(audioSourceClass, "set_mute", 1),
            source,
            muteParameters);
        Invoke(
            FindMethod(audioSourceClass, "set_volume", 1),
            source,
            volumeParameters);
    }
}

void ExtendedMode_RestoreGameplayPresentation()
{
    if (!g_extendedSceneLoadStarted)
    {
        ExtendedMode_RestoreHierarchyLayers();
    }
    const bool reusedPreviousPlayer =
        g_extendedSceneLoadStarted &&
        g_extendedPlayerBeforeSceneLoad != nullptr &&
        g_extendedPlayerBeforeSceneLoad == g_extendedPlayerGameObject &&
        ExtendedMode_IsUnityObjectAlive(g_extendedPlayerBeforeSceneLoad);
    if ((!g_extendedSceneLoadStarted || reusedPreviousPlayer) &&
        g_extendedAvatarRoot != nullptr)
    {
        auto* avatarTransform = GetWorldTransform(g_extendedAvatarRoot);
        SetTransformLocalEuler(
            avatarTransform,
            g_extendedOriginalLocalEulerX,
            g_extendedOriginalLocalEulerY,
            g_extendedOriginalLocalEulerZ);
        LogInfo(LogCategory_Action, L"Action", 
            reusedPreviousPlayer
                ? L"Escape from Manaka restored avatar rotation on the reused player."
                : L"Escape from Manaka restored avatar rotation for gameplay.");
    }

    if (g_extendedInGameReferencer != nullptr)
    {
        auto* baseCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x38);
        auto* tpCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x40);
        auto* inventoryCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x48);
        auto* faceCamera = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x98);
        auto* tpCameraClass = FindClass(
            "ExposureUnnoticed2.Object3D.Camera",
            "TPCameraController");
        if (tpCamera != nullptr &&
            baseCamera != nullptr &&
            g_extendedPlayerController != nullptr)
        {
            auto* initializeTp = FindMethod(
                tpCameraClass,
                "Initialize",
                2);
            void* initializeParameters[] = {
                g_extendedPlayerController,
                baseCamera,
            };
            if (initializeTp != nullptr)
            {
                Invoke(initializeTp, tpCamera, initializeParameters);
            }
            LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka rebound the gameplay camera to the new player.");
        }
        ExtendedMode_SetComponentEnabled(baseCamera, true);
        ExtendedMode_SetComponentEnabled(tpCamera, true);
        ExtendedMode_SetComponentEnabled(inventoryCamera, false);
        ExtendedMode_SetComponentEnabled(faceCamera, true);
        if (tpCamera != nullptr)
        {
            auto* resetToPlayer = FindMethod(
                FindClass(
                    "ExposureUnnoticed2.Object3D.Camera",
                    "TPCameraController"),
                "ResetToPlayerDirect",
                0);
            if (resetToPlayer != nullptr)
            {
                Invoke(resetToPlayer, tpCamera, nullptr);
                auto* updateCamera = FindMethod(
                    tpCameraClass,
                    "OnLateUpdate",
                    0);
                if (updateCamera != nullptr)
                {
                    Invoke(updateCamera, tpCamera, nullptr);
                    Invoke(updateCamera, tpCamera, nullptr);
                }
                LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka reset the gameplay camera to the player.");
            }
        }
        auto* volumeController = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x58);
        if (volumeController != nullptr)
        {
            SetWorldGameObjectActive(
                GetComponentGameObject(volumeController),
                true);
        }
    }

    if (g_extendedCamera == nullptr)
    {
        auto* cameraClass = FindClass("UnityEngine", "Camera");
        if (cameraClass != nullptr)
        {
            g_extendedCamera = Invoke(
                FindMethod(cameraClass, "get_main", 0),
                nullptr,
                nullptr);
        }
    }
    if (g_extendedCamera != nullptr)
    {
        auto* cameraClass = FindClass("UnityEngine", "Camera");
        int cullingMask = -1;
        int clearFlags = 1;
        float nearClip = 0.05f;
        float farClip = 1000.0f;
        bool orthographic = false;
        float fieldOfView = 60.0f;
        void* maskParameters[] = { &cullingMask };
        void* clearParameters[] = { &clearFlags };
        void* nearParameters[] = { &nearClip };
        void* farParameters[] = { &farClip };
        void* orthographicParameters[] = { &orthographic };
        void* fovParameters[] = { &fieldOfView };
        Invoke(
            FindMethod(cameraClass, "set_cullingMask", 1),
            g_extendedCamera,
            maskParameters);
        Invoke(
            FindMethod(cameraClass, "set_clearFlags", 1),
            g_extendedCamera,
            clearParameters);
        Invoke(
            FindMethod(cameraClass, "set_nearClipPlane", 1),
            g_extendedCamera,
            nearParameters);
        Invoke(
            FindMethod(cameraClass, "set_farClipPlane", 1),
            g_extendedCamera,
            farParameters);
        Invoke(
            FindMethod(cameraClass, "set_orthographic", 1),
            g_extendedCamera,
            orthographicParameters);
        Invoke(
            FindMethod(cameraClass, "set_fieldOfView", 1),
            g_extendedCamera,
            fovParameters);
        ExtendedMode_SetComponentEnabled(g_extendedCamera, true);
    }
    ExtendedMode_SetBlackRoomVisible(false);
    EquipmentPreviewUi_Close();
}

bool ExtendedMode_StartSelectedStageTransition()
{
    const int targetStage = g_extendedTargetStageType.load();
    const int currentStage = ExtendedMode_GetCurrentStageType();
    g_extendedSceneLoadStarted = false;
    g_extendedTargetAlreadyActive = currentStage == targetStage;
    // Inventory presentation rotates the avatar in local space. That yaw must
    // never survive SceneManager.LoadScene when PlayerController is reused.
    if (g_extendedAvatarRoot != nullptr)
    {
        SetTransformLocalEuler(
            GetWorldTransform(g_extendedAvatarRoot),
            g_extendedOriginalLocalEulerX,
            g_extendedOriginalLocalEulerY,
            g_extendedOriginalLocalEulerZ);
        LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka reset presentation yaw before stage transition.");
    }
    if (currentStage == targetStage)
    {
        g_extendedTransitionInvoked = true;
        g_extendedSpawnPositionApplied = true;
        LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka selected the already active stage %d.", targetStage);
        return true;
    }
    const ExtendedStageOption* target =
        ExtendedMode_FindStageOption(targetStage);
    if (target == nullptr)
    {
        return false;
    }
    CheckpointSystem_ClearSceneObjects();
    // The player can be owned by DontDestroyOnLoad. Restore its presentation
    // layers before unloading the room, otherwise Layer 31 follows the player
    // into the next stage and its renderers remain outside the gameplay camera
    // rendering path.
    ExtendedMode_RestoreHierarchyLayers();
    InventoryUi_PrepareForSceneTransition();
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr)
    {
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x10) = targetStage;
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x20) =
            g_extendedTargetDaytime.load();
    }
    if (g_extendedInGameManager != nullptr)
    {
        // Preserve the same transition metadata the shipped StageTransController
        // establishes before its fade callback loads the next scene.
        auto* temporaryState = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameManager) + 0x30);
        if (temporaryState != nullptr)
        {
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(temporaryState) + 0x18) =
                currentStage;
        }
        auto* transitionController = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameManager) + 0x60);
        if (transitionController != nullptr)
        {
            int portalId = 0;
            void* portalParameters[] = { &portalId };
            Invoke(
                FindMethod(
                    FindClass(
                        "ExposureUnnoticed2.Scripts.InGame",
                        "StageTransController"),
                    "set_LastPortalId",
                    1),
                transitionController,
                portalParameters);
        }
        if (transitionController != nullptr && currentStage >= 0)
        {
            auto* transitionClass = FindClass(
                "ExposureUnnoticed2.Scripts.InGame",
                "StageTransController");
            auto* transStage = FindMethod(
                transitionClass,
                "TransStage",
                5);
            if (transStage != nullptr)
            {
                int fromStage = currentStage;
                int toStage = targetStage;
                float waitAfterLoad = 0.0f;
                void* transitionParameters[] = {
                    &fromStage,
                    &toStage,
                    nullptr,
                    &waitAfterLoad,
                    nullptr,
                };
                Invoke(
                    transStage,
                    transitionController,
                    transitionParameters);
                g_extendedTransitionInvoked = true;
                g_extendedSceneLoadStarted = true;
                LogInfo(LogCategory_Action, L"Action", 
                    L"Escape used the original StageTransController loading page for stage %d -> %d.",
                    fromStage,
                    toStage);
                return true;
            }
        }
    }
    auto* sceneManager = FindClass(
        "UnityEngine.SceneManagement",
        "SceneManager");
    auto* loadScene = FindMethod(sceneManager, "LoadScene", 2);
    if (loadScene == nullptr)
    {
        return false;
    }
    auto* sceneName = il2cpp_string_new(target->sceneName);
    int loadMode = 0;
    g_extendedPlayerBeforeSceneLoad = g_extendedPlayerGameObject;
    void* parameters[] = {
        sceneName,
        &loadMode,
    };
    Invoke(loadScene, nullptr, parameters);
    g_extendedTransitionInvoked = true;
    g_extendedSceneLoadStarted = true;
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape from Manaka loaded scene %S for stage %d -> %d.",
        target->sceneName,
        currentStage,
        targetStage);
    return true;
}

bool ExtendedMode_QueueSceneTransition(
    int targetStage,
    const wchar_t* reason,
    bool preservePendingDeployment,
    bool fromSelection)
{
    if (ExtendedMode_FindStageOption(targetStage) == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape scene transition has no scene for stage %d.",
            targetStage);
        return false;
    }
    if (g_extendedSceneTransitionQueued.load() ||
        g_extendedSceneCleanupFramesRemaining > 0)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape ignored overlapping scene transition request to stage %d.",
            targetStage);
        return false;
    }

    g_extendedQueuedTransitionReason =
        reason != nullptr ? reason : L"scene change";
    g_extendedQueuedSceneTargetStage = targetStage;
    g_extendedQueuedTransitionPreserveDeployment =
        preservePendingDeployment;
    g_extendedQueuedTransitionFromSelection = fromSelection;
    g_extendedSceneTransitionQueued = true;
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape queued scene transition to stage %d (%s) for the safe LateUpdate pump.",
        targetStage,
        g_extendedQueuedTransitionReason.c_str());
    return true;
}

bool ExtendedMode_BeginDirectSceneTransition(
    int targetStage,
    const wchar_t* reason,
    bool preservePendingDeployment = false)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape direct transition ignored in state %d.",
            g_extendedModeState.load());
        return false;
    }
    return ExtendedMode_QueueSceneTransition(
        targetStage,
        reason,
        preservePendingDeployment,
        false);
}

bool ExtendedMode_PrepareQueuedSceneTransition()
{
    if (!g_extendedSceneTransitionQueued.exchange(false))
    {
        return false;
    }

    const int targetStage =
        g_extendedQueuedSceneTargetStage.exchange(-1);
    const bool preservePendingDeployment =
        g_extendedQueuedTransitionPreserveDeployment.exchange(false);
    const bool fromSelection =
        g_extendedQueuedTransitionFromSelection.exchange(false);
    const std::wstring reason = g_extendedQueuedTransitionReason;
    g_extendedQueuedTransitionReason.clear();

    const int currentState = g_extendedModeState.load();
    const bool stateAllowsTransition =
        fromSelection
            ? currentState == ExtendedMode_Active
            : currentState == ExtendedMode_Gameplay;
    if (!stateAllowsTransition ||
        ExtendedMode_FindStageOption(targetStage) == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape discarded scene transition to stage %d because state=%d and selection=%d.",
            targetStage,
            currentState,
            fromSelection ? 1 : 0);
        g_extendedDirectWarehouseTransition = false;
        ExtendedMode_HideTransferLoadingOverlay();
        return false;
    }

    const int currentStage = ExtendedMode_GetCurrentStageType();
    bool daytime = g_extendedTargetDaytime.load();
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr)
    {
        daytime = *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x20);
    }

    InventoryUi_SaveState();
    g_extendedSourceStageType = currentStage;
    g_extendedTargetStageType = targetStage;
    g_extendedTargetDaytime = daytime;
    g_extendedDirectWarehouseTransition =
        !fromSelection && targetStage == 0;
    g_extendedReturnToWarehouseAfterExit = false;
    g_extendedWarehouseReentryDelay = 0;
    if (!preservePendingDeployment)
    {
        g_extendedPendingDeploymentStageType = -1;
        g_extendedPendingDeploymentDelayFrames = 0;
    }
    g_extendedReloadAfterMenuOpenFrames = 0;
    g_extendedOpenSystemMenuRequested = false;
    g_extendedExitRequested = false;
    g_extendedGameplayActive = false;
    g_extendedTakeoverComplete = false;
    g_extendedSuppressPlayerUpdate = true;
    g_extendedSpawnPositionApplied = false;
    g_extendedTimeApplied = false;
    g_extendedSettledStageFrames = 0;
    g_extendedTransitionInvoked = false;
    g_extendedSceneLoadStarted = false;
    g_extendedTargetAlreadyActive = false;
    g_extendedStateFrames = 0;
    g_extendedSceneBindingStableFrames = 0;
    g_extendedSceneBindingEventManagerCandidate = nullptr;

    g_extendedForwardKeyDown = false;
    g_extendedBackwardKeyDown = false;
    g_extendedLeftKeyDown = false;
    g_extendedRightKeyDown = false;
    g_extendedForwardKeyPulse = false;
    g_extendedBackwardKeyPulse = false;
    g_extendedLeftKeyPulse = false;
    g_extendedRightKeyPulse = false;
    g_extendedPoseMenuHeld = false;
    g_extendedPoseMenuVisible = false;
    g_extendedPoseMenuClickRequested = false;
    g_extendedPoseMenuRequestedAction = -1;
    g_extendedManualPointerDown = false;
    g_extendedManualPointerConsumed = false;
    g_extendedManualPointerInventoryEligible = false;

    if (fromSelection)
    {
        if (!g_extendedSpecialCoatMode.load())
        {
            ExtendedMode_SetSpecialCoatMode(true);
            g_extendedSpecialCoatApplied = false;
        }
        g_extendedSceneSelectorOpen = false;
        g_extendedPoseMenuSuppressedUntilRelease = false;
        if (g_extendedSceneSelectorPanel.gameObject != nullptr)
        {
            SetWorldGameObjectActive(
                g_extendedSceneSelectorPanel.gameObject,
                false);
        }
        if (g_extendedCanvas != nullptr)
        {
            SetWorldGameObjectActive(g_extendedCanvas, false);
        }
        ExtendedMode_BroadcastStageEvent("StartTransitionStageEvent");
        SetStatus(
            L"ESCAPE FROM MANAKA: entering selected deployment");
    }

    ExtendedMode_CloseOriginalSystemMenuForSceneChange();
    ClearSceneEventSelection();
    GameplayInput_RestoreVibratorSaveState();
    if (targetStage == 0 &&
        g_extendedSpecialCoatMode.load())
    {
        ExtendedMode_SetSpecialCoatMode(false);
    }

    ExtendedMode_ForceCursor();
    ModGuideUi_Reset();
    ExtendedMode_DestroyRuntimeWindows(reason.c_str());
    GameplayInventory_Reset();
    ExtendedMode_ClearCrossSceneUiCaches();
    ExtendedMode_RestoreHierarchyLayers();
    if (preservePendingDeployment)
    {
        ExtendedMode_ShowTransferLoadingOverlay();
    }

    g_extendedModeState = ExtendedMode_EnteringGameplay;
    g_extendedSceneCleanupFramesRemaining = 2;
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape prepared scene transition: stage %d -> %d (%s), generation=%llu; waiting for Unity cleanup.",
        currentStage,
        targetStage,
        reason.c_str(),
        static_cast<unsigned long long>(
            g_extendedSceneGeneration.load()));
    return true;
}

bool ExtendedMode_CompletePreparedSceneTransition()
{
    if (g_extendedSceneCleanupFramesRemaining <= 0)
    {
        return false;
    }
    if (g_extendedSceneCleanupFramesRemaining > 1)
    {
        --g_extendedSceneCleanupFramesRemaining;
        return false;
    }
    g_extendedSceneCleanupFramesRemaining = 0;

    const int targetStage = g_extendedTargetStageType.load();
    const int sourceStage = g_extendedSourceStageType;
    if (!ExtendedMode_StartSelectedStageTransition())
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape scene transition could not start for stage %d -> %d.",
            sourceStage,
            targetStage);
        g_extendedDirectWarehouseTransition = false;
        ExtendedMode_HideTransferLoadingOverlay();
        ExtendedMode_ExitToTitle();
        return false;
    }

    if (g_extendedSceneLoadStarted)
    {
        const unsigned long long generation =
            g_extendedSceneGeneration.fetch_add(1) + 1;
        g_extendedAwaitingFreshSceneManager = true;
        g_extendedActiveEventMethodManager = nullptr;
        g_extendedEventManagerUpdateFrames = 0;
        g_extendedSceneBindingEventManagerCandidate = nullptr;
        g_extendedSceneBindingStableFrames = 0;
        g_extendedInGameManager = nullptr;
        g_extendedInGameReferencer = nullptr;
        g_extendedInGameUiManager = nullptr;
        g_extendedCamera = nullptr;
        g_extendedPlayerController = nullptr;
        g_extendedPlayerGameObject = nullptr;
        g_extendedAvatarReferencer = nullptr;
        g_extendedAvatarRoot = nullptr;
        g_previewModel = nullptr;
        g_previewAnimator = nullptr;
        g_extendedLastResolvedInputData = nullptr;
        g_extendedLayerSnapshots.clear();
        g_extendedAnimatorStateSnapshot =
            ExtendedAnimatorStateSnapshot{};
        LogInfo(LogCategory_Action, L"Action", 
            L"Scene transition load started at generation %llu; released all old bindings.",
            generation);
    }
    return true;
}

void ExtendedMode_ProcessSafeSceneTransitionPump()
{
    const int frame = ExtendedMode_GetUnityFrameCount();
    if (frame >= 0 && frame == g_extendedLastSafeTransitionPumpFrame)
    {
        return;
    }
    if (frame >= 0)
    {
        g_extendedLastSafeTransitionPumpFrame = frame;
    }

    const int state = g_extendedModeState.load();
    if (g_extendedSceneTransitionQueued.load() &&
        (state == ExtendedMode_Gameplay ||
         state == ExtendedMode_Active))
    {
        ExtendedMode_PrepareQueuedSceneTransition();
        return;
    }
    if (state == ExtendedMode_EnteringGameplay &&
        g_extendedSceneCleanupFramesRemaining > 0)
    {
        ExtendedMode_CompletePreparedSceneTransition();
    }
}

bool ExtendedMode_BeginTransferToStage(int targetStage)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        targetStage < 0)
    {
        return false;
    }
    bool daytime = g_extendedTargetDaytime.load();
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData != nullptr)
    {
        daytime = *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x20);
    }
    g_extendedPendingDeploymentStageType = targetStage;
    g_extendedPendingDeploymentDaytime = daytime;
    g_extendedPendingDeploymentDelayFrames = 0;
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape transfer requested warehouse re-entry before deployment to stage %d.",
        targetStage);
    return ExtendedMode_BeginDirectSceneTransition(
        0,
        L"checkpoint transfer staging",
        true);
}

void ExtendedMode_BeginSelectedGameplay()
{
    if (g_extendedModeState.load() != ExtendedMode_Active)
    {
        return;
    }
    if (g_extendedPendingDeploymentStageType < 0)
    {
        EquipmentStats_BeginRaid(L"selected deployment");
    }
    g_extendedSuppressPlayerUpdate = true;
    ExtendedMode_ForceCursor();
    if (!ExtendedMode_QueueSceneTransition(
            g_extendedTargetStageType.load(),
            L"selected deployment",
            false,
            true))
    {
        g_extendedSuppressPlayerUpdate = false;
    }
}

void ExtendedMode_FinalizeGameplay()
{
    g_extendedModeState = ExtendedMode_Gameplay;
    GameplayInput_CaptureVibratorSaveState();
    InteractionFilter_ApplySceneWhitelist();
    ExtendedMode_RestoreGameplayPresentation();
    ExtendedMode_SnapGameplayCameraToPlayer();
    ExtendedMode_RestoreGameplayAudio();
    g_extendedGameplayActive = true;
    g_extendedSuppressPlayerUpdate = false;
    g_extendedSettledStageFrames = 0;
    ExtendedMode_RestoreAnimatorForGameplay();
    ExtendedMode_ReinitializeAnimationManager();
    g_extendedGameplayStableFrames = 0;
    SetWorldGameObjectActive(g_extendedPlayerGameObject, true);
    if (g_extendedAvatarRoot != g_extendedPlayerGameObject)
    {
        SetWorldGameObjectActive(g_extendedAvatarRoot, true);
    }
    ExtendedMode_PrepareSkinnedRenderers();
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* getActiveSelf = FindMethod(gameObjectClass, "get_activeSelf", 0);
    auto* getActiveInHierarchy = FindMethod(
        gameObjectClass,
        "get_activeInHierarchy",
        0);
    auto* getLayer = FindMethod(gameObjectClass, "get_layer", 0);
    auto* getCameraMask = FindMethod(
        FindClass("UnityEngine", "Camera"),
        "get_cullingMask",
        0);
    auto* playerActive = Invoke(
        getActiveSelf,
        g_extendedPlayerGameObject,
        nullptr);
    auto* avatarActive = Invoke(
        getActiveSelf,
        g_extendedAvatarRoot,
        nullptr);
    auto* playerActiveInHierarchy = Invoke(
        getActiveInHierarchy,
        g_extendedPlayerGameObject,
        nullptr);
    auto* avatarActiveInHierarchy = Invoke(
        getActiveInHierarchy,
        g_extendedAvatarRoot,
        nullptr);
    auto* playerLayer = Invoke(
        getLayer,
        g_extendedPlayerGameObject,
        nullptr);
    auto* avatarLayer = Invoke(
        getLayer,
        g_extendedAvatarRoot,
        nullptr);
    auto* cameraMask = Invoke(
        getCameraMask,
        g_extendedCamera,
        nullptr);
    LogInfo(LogCategory_Action, L"Action", 
        L"Escape from Manaka gameplay visibility: playerActive=%d/%d avatarActive=%d/%d playerLayer=%d avatarLayer=%d cameraMask=%d.",
        playerActive != nullptr && *static_cast<bool*>(il2cpp_object_unbox(playerActive)) ? 1 : 0,
        playerActiveInHierarchy != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(playerActiveInHierarchy)) ? 1 : 0,
        avatarActive != nullptr && *static_cast<bool*>(il2cpp_object_unbox(avatarActive)) ? 1 : 0,
        avatarActiveInHierarchy != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(avatarActiveInHierarchy)) ? 1 : 0,
        playerLayer != nullptr ? *static_cast<int*>(il2cpp_object_unbox(playerLayer)) : -999,
        avatarLayer != nullptr ? *static_cast<int*>(il2cpp_object_unbox(avatarLayer)) : -999,
        cameraMask != nullptr ? *static_cast<int*>(il2cpp_object_unbox(cameraMask)) : -999);
    ExtendedMode_LogTransformPosition(L"player root", g_extendedPlayerGameObject);
    ExtendedMode_LogTransformPosition(L"avatar root", g_extendedAvatarRoot);
    if (g_extendedCamera != nullptr)
    {
        ExtendedMode_LogTransformPosition(
            L"gameplay camera",
            GetComponentGameObject(g_extendedCamera));
    }
    // Re-apply the loadout only to the new scene's avatar. UI references were
    // released before scene loading, so no TMP objects are touched here.
    InventoryUi_RefreshCharacterModel();
    ExtendedMode_ApplyCoatlessLogic(true);
    ExtendedMode_HideGameplayUi();
    ExtendedMode_BuildGameplayHud();
    ExtendedMode_UpdateGameplayCoordinates();
    CheckpointSystem_OnGameplayReady(
        ExtendedMode_GetCurrentStageType());
    ExtendedMode_ForceGameplayCursor();
    ExtendedMode_HideTransferLoadingOverlay();
    SetStatus(
        L"ESCAPE FROM MANAKA: in game  |  WASD 移动  |  鼠标视角  |  Esc 退出");
    LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka gameplay mode is active.");
}

void ExtendedMode_SelectAction(int action)
{
    action = action < 0
        ? 0
        : (action >= kEscapeFromManakaActionCount
            ? kEscapeFromManakaActionCount - 1
            : action);
    g_extendedSelectedAction = action;
    EscapeFromManaka_SelectAction(action);
    if (EscapeFromManaka_ApplyOriginalAction(action))
    {
        ExtendedMode_ResetDynamicBones();
        ExtendedMode_ApplyCoatlessLogic(true);
        wchar_t stage[48]{};
        swprintf_s(stage, L"after-action-%d", action);
        LogEscapeFromManakaLegState(stage);
        g_extendedReframeCountdown = 4;
        ExtendedMode_UpdateSelectedActionLabel();
    }
}

void ExtendedMode_ToggleActionList()
{
    if (g_extendedActionPanel == nullptr ||
        g_inventoryActiveRoute != kInventoryRouteEquipment)
    {
        return;
    }
    const bool open = !g_extendedActionListOpen.exchange(true);
    if (!open)
    {
        g_extendedActionListOpen = false;
        MoaningAudio_Stop();
    }
    SetWorldGameObjectActive(g_extendedActionPanel, open);
    if (open)
    {
        if (EquipmentPreviewUi_IsOpen())
        {
            EquipmentPreviewUi_Close();
        }
        ExtendedMode_RefreshActionPage();
        LogInfo(LogCategory_Action, L"Action", L"Moaning3 audio debug panel opened in the warehouse interface.");
    }
    else
    {
        LogInfo(LogCategory_Action, L"Action", L"Moaning3 audio debug panel closed.");
    }
}

bool ExtendedMode_PointInRect(
    float x,
    float y,
    float minX,
    float minY,
    float maxX,
    float maxY)
{
    return x >= minX && x <= maxX && y >= minY && y <= maxY;
}

bool ExtendedMode_PointInChildRect(
    float x,
    float y,
    float panelMinX,
    float panelMinY,
    float panelMaxX,
    float panelMaxY,
    float childMinX,
    float childMinY,
    float childMaxX,
    float childMaxY)
{
    if (!ExtendedMode_PointInRect(
            x,
            y,
            panelMinX,
            panelMinY,
            panelMaxX,
            panelMaxY))
    {
        return false;
    }
    const float localX =
        (x - panelMinX) / (panelMaxX - panelMinX);
    const float localY =
        (y - panelMinY) / (panelMaxY - panelMinY);
    return ExtendedMode_PointInRect(
        localX,
        localY,
        childMinX,
        childMinY,
        childMaxX,
        childMaxY);
}

void ExtendedMode_HandleManualPointerPress()
{
    static unsigned int handledSerial = 0;
    const unsigned int serial = g_extendedManualPointerPressSerial.load();
    if (serial == handledSerial)
    {
        return;
    }
    handledSerial = serial;

    if (ItemContextMenu_ShouldBlockPointer())
    {
        g_extendedManualPointerConsumed = true;
        g_extendedManualPointerInventoryEligible = false;
        return;
    }
    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    if (gameWindow == nullptr ||
        !GetClientRect(gameWindow, &clientRect))
    {
        return;
    }
    POINT cursor{
        g_extendedManualPointerPressX.load(),
        g_extendedManualPointerPressY.load(),
    };
    if (!ScreenToClient(gameWindow, &cursor))
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
    const float x = static_cast<float>(cursor.x) / width;
    const float y = 1.0f - static_cast<float>(cursor.y) / height;

    if (EquipmentPreviewUi_IsOpen())
    {
        g_extendedManualPointerConsumed = true;
        EquipmentPreviewUi_HandleManualPointer(x, y);
        return;
    }

    if (g_extendedSceneSelectorOpen)
    {
        g_extendedManualPointerConsumed = true;
        constexpr float panelMinX = 0.145f;
        constexpr float panelMinY = 0.085f;
        constexpr float panelMaxX = 0.855f;
        constexpr float panelMaxY = 0.888f;
        for (int index = 0; index < kEscapeFromManakaStageCount; ++index)
        {
            const int column = index % 2;
            const int row = index / 2;
            const float minX = 0.055f + column * 0.455f;
            const float minY = 0.680f - row * 0.120f;
            if (ExtendedMode_PointInChildRect(
                    x,
                    y,
                    panelMinX,
                    panelMinY,
                    panelMaxX,
                    panelMaxY,
                    minX,
                    minY,
                    minX + 0.405f,
                    minY + 0.092f))
            {
                g_extendedSelectedStageIndex = index;
                ExtendedMode_RefreshSceneSelector();
                return;
            }
        }
        if (ExtendedMode_PointInChildRect(
                x, y, panelMinX, panelMinY, panelMaxX, panelMaxY,
                0.055f, 0.180f, 0.260f, 0.270f))
        {
            g_extendedSelectedDaytime = true;
            ExtendedMode_RefreshSceneSelector();
            return;
        }
        if (ExtendedMode_PointInChildRect(
                x, y, panelMinX, panelMinY, panelMaxX, panelMaxY,
                0.275f, 0.180f, 0.480f, 0.270f))
        {
            g_extendedSelectedDaytime = false;
            ExtendedMode_RefreshSceneSelector();
            return;
        }
        if (ExtendedMode_PointInChildRect(
                x, y, panelMinX, panelMinY, panelMaxX, panelMaxY,
                0.545f, 0.055f, 0.715f, 0.140f))
        {
            ExtendedMode_CloseSceneSelector();
            return;
        }
        if (ExtendedMode_PointInChildRect(
                x, y, panelMinX, panelMinY, panelMaxX, panelMaxY,
                0.735f, 0.055f, 0.945f, 0.140f))
        {
            g_extendedTargetStageType =
                kExtendedStageOptions[g_extendedSelectedStageIndex].stageType;
            g_extendedTargetDaytime = g_extendedSelectedDaytime;
            g_extendedEnterGameRequested = true;
            return;
        }
        return;
    }

    if (g_extendedActionListOpen)
    {
        g_extendedManualPointerConsumed = true;
        constexpr float panelMinX = 0.505f;
        constexpr float panelMinY = 0.105f;
        constexpr float panelMaxX = 0.976f;
        constexpr float panelMaxY = 0.865f;
        if (ExtendedMode_PointInChildRect(
                x, y, panelMinX, panelMinY, panelMaxX, panelMaxY,
                0.045f, 0.845f, 0.235f, 0.915f))
        {
            MoaningAudio_ChangePage(-1);
            ExtendedMode_RefreshActionPage();
            return;
        }
        if (ExtendedMode_PointInChildRect(
                x, y, panelMinX, panelMinY, panelMaxX, panelMaxY,
                0.765f, 0.845f, 0.955f, 0.915f))
        {
            MoaningAudio_ChangePage(1);
            ExtendedMode_RefreshActionPage();
            return;
        }
        for (int index = 0;
             index < kMoaningAudioFilesPerPage;
             ++index)
        {
            const int row = index;
            const float minX = 0.045f;
            const float minY = 0.775f - row * 0.078f;
            if (ExtendedMode_PointInChildRect(
                    x,
                    y,
                    panelMinX,
                    panelMinY,
                    panelMaxX,
                    panelMaxY,
                    minX,
                    minY,
                    0.495f,
                    minY + 0.065f))
            {
                MoaningAudio_PlayDebugSlot(index);
                return;
            }
        }
        return;
    }

    constexpr float headerMinX = 0.024f;
    constexpr float headerMinY = 0.902f;
    constexpr float headerMaxX = 0.976f;
    constexpr float headerMaxY = 0.974f;
    if (ExtendedMode_PointInChildRect(
            x, y, headerMinX, headerMinY, headerMaxX, headerMaxY,
            0.785f, 0.12f, 0.975f, 0.88f))
    {
        g_extendedManualPointerConsumed = true;
        ExtendedMode_OpenSceneSelector();
        return;
    }
    const bool headerHandled =
        InventoryUi_HandleManualPointerPress(x, y);
    g_extendedManualPointerInventoryEligible =
        !headerHandled &&
        InventoryUi_ShouldHandleManualPointerAt(x, y);
    g_extendedManualPointerConsumed = headerHandled;
}

void ExtendedMode_UpdateActionRequests()
{
    if (g_extendedToggleActionListRequested.exchange(false))
    {
        ExtendedMode_ToggleActionList();
    }
    const int pending = g_extendedPendingAction.exchange(-1);
    if (pending >= 0)
    {
        ExtendedMode_SelectAction(pending);
    }
    MoaningAudio_TickDebugPanel();
}

void ExtendedMode_UpdateModelDrag()
{
    if (g_extendedModeState.load() != ExtendedMode_Active ||
        g_extendedAvatarRoot == nullptr ||
        !InventoryUi_CanDragModel())
    {
        g_extendedModelDragging = false;
        return;
    }
    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    if (gameWindow == nullptr ||
        !GetClientRect(gameWindow, &clientRect))
    {
        g_extendedModelDragging = false;
        return;
    }
    const int width = clientRect.right - clientRect.left;
    const int height = clientRect.bottom - clientRect.top;
    if (width <= 0 || height <= 0)
    {
        g_extendedModelDragging = false;
        return;
    }
    POINT cursor{};
    if (!GetCursorPos(&cursor) ||
        !ScreenToClient(gameWindow, &cursor))
    {
        g_extendedModelDragging = false;
        return;
    }
    const float normalizedX =
        static_cast<float>(cursor.x - clientRect.left) /
        static_cast<float>(width);
    const float normalizedY =
        static_cast<float>(cursor.y - clientRect.top) /
        static_cast<float>(height);
    const bool insideModelArea =
        normalizedX >= 0.02f && normalizedX <= 0.345f &&
        normalizedY >= 0.10f && normalizedY <= 0.90f;
    const bool leftButtonDown =
        g_extendedModeState.load() == ExtendedMode_Active
            ? (g_extendedManualPointerDown.load() &&
               !g_extendedManualPointerConsumed.load())
            : ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
    const bool leftButtonPressed =
        leftButtonDown && !g_extendedModelLeftWasDown;
    g_extendedModelLeftWasDown = leftButtonDown;

    if (!leftButtonDown)
    {
        if (g_extendedModelDragging)
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Extended model drag ended at %.1f degrees.",
                g_extendedModelYaw);
        }
        g_extendedModelDragging = false;
        return;
    }
    if (!g_extendedModelDragging)
    {
        if (!leftButtonPressed || !insideModelArea)
        {
            return;
        }
        g_extendedModelDragging = true;
        g_extendedModelDragPoint = cursor;
        return;
    }
    const int deltaX = cursor.x - g_extendedModelDragPoint.x;
    if (deltaX != 0)
    {
        // A leftward drag rotates the preview left; vertical rotation is locked.
        g_extendedModelYaw -=
            static_cast<float>(deltaX) * 360.0f /
            static_cast<float>(width);
        if (g_extendedModelYaw > 180.0f)
        {
            g_extendedModelYaw -= 360.0f;
        }
        else if (g_extendedModelYaw < -180.0f)
        {
            g_extendedModelYaw += 360.0f;
        }
        ExtendedMode_FaceAvatarTowardCamera();
    }
    g_extendedModelDragPoint = cursor;
}

void ExtendedMode_UpdateFutanariVisibility()
{
    if (g_extendedAvatarReferencer == nullptr)
    {
        return;
    }
    auto* object = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x198);
    auto* root = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x1A0);
    if (object != nullptr)
    {
        SetWorldGameObjectActive(object, false);
    }
    if (root != nullptr)
    {
        auto* rootObject = Invoke(
            FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
            root,
            nullptr);
        SetWorldGameObjectActive(rootObject, false);
    }
}

Il2CppObject* ExtendedMode_TryCreateFullScTmpFont()
{
    const std::wstring fontPath =
        GetEscapeFromManakaSubdirectory(L"Fonts") +
        L"\\NotoSerifSC-Regular.ttf";
    if (GetFileAttributesW(fontPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        LogInfo(LogCategory_Action, L"Action", L"Full SC font file is unavailable: %s", fontPath.c_str());
        return nullptr;
    }

    auto* tmpFontClass = FindClass("TMPro", "TMP_FontAsset");
    auto* assetReferencer = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "AssetReferencer",
        "Instance");
    auto* originalScFont = assetReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
              reinterpret_cast<char*>(assetReferencer) + 0x360)
        : nullptr;
    if (tmpFontClass == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(originalScFont) ||
        !SceneInstallFullScFontHook() ||
        !SceneLoadFullScFontFace(90))
    {
        LogInfo(LogCategory_Action, L"Action", L"Full SC TMP asset or face preparation failed.");
        return nullptr;
    }

    auto* objectClass = FindClass("UnityEngine", "Object");
    void* instantiateParameters[] = { originalScFont };
    auto* tmpFontAsset = objectClass != nullptr
        ? Invoke(
              FindMethod(objectClass, "Instantiate", 1),
              nullptr,
              instantiateParameters)
        : nullptr;
    if (!ExtendedMode_IsUnityObjectAlive(tmpFontAsset))
    {
        LogInfo(LogCategory_Action, L"Action", L"Could not clone the original SC TMP font asset.");
        return nullptr;
    }
    bool setAtlasSizeToZero = false;
    void* clearParameters[] = { &setAtlasSizeToZero };
    Invoke(
        FindMethod(tmpFontClass, "ClearFontAssetData", 1),
        tmpFontAsset,
        clearParameters);

    auto* sourceFont = FindLoadedAssetByName(
        "UnityEngine",
        "Font",
        L"NotoSerifSC-Regular");
    auto* unityFontClass = FindClass("UnityEngine", "Font");
    int shiCharacter = 0x5E02;
    void* hasCharacterParameters[] = { &shiCharacter };
    auto* hasCharacter = unityFontClass != nullptr
        ? Invoke(
              FindMethodOverload(
                  unityFontClass,
                  "HasCharacter",
                  1,
                  0),
              sourceFont,
              hasCharacterParameters)
        : nullptr;
    LogInfo(LogCategory_Action, L"Action", 
        L"Full SC TMP source font: %p U+5E02 available=%d.",
        sourceFont,
        hasCharacter != nullptr &&
            *static_cast<bool*>(
                il2cpp_object_unbox(hasCharacter))
            ? 1
            : 0);
    if (!ExtendedMode_IsUnityObjectAlive(sourceFont))
    {
        return nullptr;
    }
    void* sourceParameters[] = { sourceFont };
    Invoke(
        FindMethod(tmpFontClass, "set_sourceFontFile", 1),
        tmpFontAsset,
        sourceParameters);

    bool enableMultiAtlas = true;
    void* multiAtlasParameters[] = { &enableMultiAtlas };
    Invoke(
        FindMethod(
            tmpFontClass,
            "set_isMultiAtlasTexturesEnabled",
            1),
        tmpFontAsset,
        multiAtlasParameters);
    int dynamicPopulationMode = 1;
    void* populationParameters[] = { &dynamicPopulationMode };
    Invoke(
        FindMethod(tmpFontClass, "set_atlasPopulationMode", 1),
        tmpFontAsset,
        populationParameters);
    Invoke(
        FindMethod(tmpFontClass, "ReadFontAssetDefinition", 0),
        tmpFontAsset,
        nullptr);

    g_tmpFontAsset = tmpFontAsset;
    g_tmpFontUsesFullScHook = true;
    constexpr int shiCodepoint = 0x5E02;
    const bool before = SceneTmpFontHasCodepoint(
        tmpFontAsset,
        shiCodepoint);
    uint32_t unicode = shiCodepoint;
    Il2CppObject* addedCharacter = nullptr;
    void* addCharacterParameters[] = { &unicode, &addedCharacter };
    auto* addCharacterResult = Invoke(
        FindMethod(
            tmpFontClass,
            "TryAddCharacterInternal",
            2),
        tmpFontAsset,
        addCharacterParameters);
    const bool addResult = addCharacterResult != nullptr &&
        *static_cast<bool*>(
            il2cpp_object_unbox(addCharacterResult));
    const bool after = SceneTmpFontHasCodepoint(
        tmpFontAsset,
        shiCodepoint);
    LogInfo(LogCategory_Action, L"Action", 
        L"Full SC TMP font: asset=%p add=%d glyph=%p; U+5E02 before=%d after=%d.",
        tmpFontAsset,
        addResult ? 1 : 0,
        addedCharacter,
        before ? 1 : 0,
        after ? 1 : 0);
    if (!after)
    {
        g_tmpFontUsesFullScHook = false;
        g_tmpFontAsset = nullptr;
    }
    return after ? tmpFontAsset : nullptr;
}

bool ExtendedMode_AddTmpFontFallback(
    Il2CppObject* fontAsset,
    Il2CppObject* fallback,
    const wchar_t* label)
{
    if (!ExtendedMode_IsUnityObjectAlive(fontAsset) ||
        !ExtendedMode_IsUnityObjectAlive(fallback) ||
        fontAsset == fallback)
    {
        return false;
    }
    auto* tmpFontClass = FindClass("TMPro", "TMP_FontAsset");
    auto* fallbacks = Invoke(
        FindMethod(tmpFontClass, "get_fallbackFontAssetTable", 0),
        fontAsset,
        nullptr);
    if (fallbacks == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"TMP fallback list is unavailable for %s.", label);
        return false;
    }
    auto* fallbackClass = il2cpp_object_get_class(fallbacks);
    auto* contains = FindMethod(fallbackClass, "Contains", 1);
    if (contains != nullptr)
    {
        void* containsParameters[] = { fallback };
        auto* containsValue = Invoke(
            contains,
            fallbacks,
            containsParameters);
        if (containsValue != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(containsValue)))
        {
            return true;
        }
    }
    auto* add = FindMethod(fallbackClass, "Add", 1);
    if (add == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"TMP fallback Add method is unavailable for %s.", label);
        return false;
    }
    void* addParameters[] = { fallback };
    Invoke(add, fallbacks, addParameters);
    LogInfo(LogCategory_Action, L"Action", 
        L"TMP fallback added: primary=%s fallback=%s.",
        GetObjectName(fontAsset).c_str(),
        GetObjectName(fallback).c_str());
    return true;
}

void ExtendedMode_AddAllLoadedTmpFallbacks(Il2CppObject* primary)
{
    auto* tmpFontClass = FindClass("TMPro", "TMP_FontAsset");
    auto* resourcesClass = FindClass("UnityEngine", "Resources");
    if (tmpFontClass == nullptr ||
        resourcesClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(tmpFontClass);
    auto* fontType = il2cpp_type_get_object(
        il2cpp_class_get_type(tmpFontClass));
    void* parameters[] = { fontType };
    auto* assets = Invoke(
        FindMethod(resourcesClass, "FindObjectsOfTypeAll", 1),
        nullptr,
        parameters);
    if (assets == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(assets);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(assets) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        ExtendedMode_AddTmpFontFallback(
            primary,
            elements[index],
            L"loaded TMP fonts");
    }
}

void ExtendedMode_CacheFont()
{
    if (!ExtendedMode_IsUnityObjectAlive(g_tmpFontAsset))
    {
        g_tmpFontAsset = nullptr;
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_fullScUnityFont))
    {
        g_fullScUnityFont = nullptr;
    }
    if (g_tmpFontAsset != nullptr &&
        g_fullScUnityFont != nullptr)
    {
        return;
    }

    g_fullScUnityFont = FindLoadedAssetByName(
        "UnityEngine",
        "Font",
        L"NotoSerifSC-Regular");
    if (ExtendedMode_IsUnityObjectAlive(g_fullScUnityFont))
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Mod UI bound to full NotoSerifSC-Regular Font: %p.",
            g_fullScUnityFont);
    }

    auto* tmpFontClass = FindClass("TMPro", "TMP_FontAsset");
    auto* assetReferencer = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "AssetReferencer",
        "Instance");
    Il2CppObject* defaultFont = assetReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
              reinterpret_cast<char*>(assetReferencer) + 0x370)
        : nullptr;
    Il2CppObject* scFont = assetReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
              reinterpret_cast<char*>(assetReferencer) + 0x360)
        : nullptr;
    Il2CppObject* tcFont = assetReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
              reinterpret_cast<char*>(assetReferencer) + 0x368)
        : nullptr;
    Il2CppObject* krFont = assetReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
              reinterpret_cast<char*>(assetReferencer) + 0x358)
        : nullptr;
    if (tmpFontClass != nullptr &&
        ExtendedMode_IsUnityObjectAlive(defaultFont))
    {
        g_tmpFontAsset = defaultFont;
        ExtendedMode_AddTmpFontFallback(defaultFont, scFont, L"fontSc");
        ExtendedMode_AddTmpFontFallback(defaultFont, tcFont, L"fontTc");
        ExtendedMode_AddTmpFontFallback(defaultFont, krFont, L"fontKr");
        ExtendedMode_AddAllLoadedTmpFallbacks(defaultFont);
        auto* characters = Invoke(
            FindMethod(tmpFontClass, "GetCharacters", 1),
            nullptr,
            reinterpret_cast<void**>(&defaultFont));
        LogInfo(LogCategory_Action, L"Action", 
            L"TMP font chain prepared: primary=%s characters=%zu.",
            GetObjectName(defaultFont).c_str(),
            characters != nullptr
                ? ReadIl2CppString(
                      static_cast<Il2CppString*>(characters)).size()
                : 0);
        return;
    }

    auto* fontSource = g_nativeButton;
    if (!ExtendedMode_IsUnityObjectAlive(fontSource))
    {
        fontSource = nullptr;
    }
    if (fontSource == nullptr)
    {
        auto* templateButton = FindAnyButtonView();
        auto* componentClass = FindClass("UnityEngine", "Component");
        auto* getGameObject = componentClass != nullptr
            ? FindMethod(componentClass, "get_gameObject", 0)
            : nullptr;
        if (templateButton != nullptr && getGameObject != nullptr)
        {
            fontSource = Invoke(
                getGameObject,
                templateButton,
                nullptr);
        }
    }
    auto* titleText = FindFirstTmpTextComponent(fontSource);
    if (titleText != nullptr)
    {
        g_tmpFontAsset = Invoke(
            FindMethod(FindClass("TMPro", "TMP_Text"), "get_font", 0),
            titleText,
            nullptr);
    }
}

void ExtendedMode_ClearCrossSceneUiCaches()
{
    // Do not carry UnityEngine.Object references across SceneManager.LoadScene.
    // A raw non-null IL2CPP pointer can still refer to an object whose native
    // Unity object was destroyed, which is especially fatal for TMP_FontAsset.
    g_tmpFontAsset = nullptr;
    g_fullScUnityFont = nullptr;
    g_tmpFontUsesFullScHook = false;
    g_fullScFontHookActive.store(false);
    g_uiButtons.clear();
    g_roomRowLabels.clear();
    g_titleMenuButtons.clear();
    g_efmActionButtons.clear();
    g_efmActionLabels.clear();
    g_efmActionPageLabel = nullptr;
    g_efmPrevActionPageButton = nullptr;
    g_efmNextActionPageButton = nullptr;
}

void ExtendedMode_ResetForTitle()
{
    EquipmentStats_DiscardRaidEarnings(L"title reset");
    GameplayInput_RestoreVibratorSaveState();
    ItemContextMenu_Reset();
    ExhibitionUi_Reset();
    if (!g_extendedWarehouseReturnInProgress.load())
    {
        ExtendedMode_DestroyTransferLoadingOverlay();
    }
    else
    {
        LogInfo(LogCategory_Action, L"Action", L"Warehouse loading cover retained across the Title scene.");
    }
    ExtendedMode_HideGameplayUiCompletely();
    ExtendedMode_DestroyRuntimeWindows(L"the mode reset");
    CheckpointSystem_Reset();
    ExtendedMode_RestoreClothingStateSnapshot();
    g_extendedSpecialCoatMode = false;
    g_extendedToggleSpecialCoatRequested = false;
    g_extendedSpecialCoatApplied = false;
    g_extendedSpecialCoatSnapshot = ExtendedSpecialCoatSnapshot{};
    ExtendedMode_ClearCrossSceneUiCaches();
    GameplayMinimap_Reset();
    ModGuideUi_Reset();
    ClearScenePngTextureCache();
    if (ExtendedMode_IsUnityObjectAlive(g_nativeButton))
    {
        SetWorldGameObjectActive(g_nativeButton, false);
    }
    g_nativeButtonInjected = false;
    g_nativeButtonTitleInstance = nullptr;
    g_nativeButton = nullptr;
    g_nativeButtonComponent = nullptr;
    g_nativeButtonRect = nullptr;
    g_buttonScreenRect = {};
    g_buttonRectReady = false;
    g_extendedModeState = ExtendedMode_Inactive;
    g_extendedStateFrames = 0;
    g_extendedSaveSelectionInvoked = false;
    g_extendedDefaultSavePending = false;
    g_extendedDefaultSaveWriteAttempts = 0;
    g_extendedTakeoverComplete = false;
    g_extendedGameplayUpdateObserved = false;
    g_extendedSceneTransitionQueued = false;
    g_extendedQueuedSceneTargetStage = -1;
    g_extendedQueuedTransitionPreserveDeployment = false;
    g_extendedQueuedTransitionFromSelection = false;
    g_extendedQueuedTransitionReason.clear();
    g_extendedAwaitingFreshSceneManager = false;
    g_extendedSceneCleanupFramesRemaining = 0;
    g_extendedSceneBindingStableFrames = 0;
    g_extendedEventManagerUpdateFrames = 0;
    g_extendedActiveEventMethodManager = nullptr;
    g_extendedSceneBindingEventManagerCandidate = nullptr;
    g_extendedAwaitingReturnConfirmation = false;
    g_extendedReloadAfterMenuOpenFrames = 0;
    g_extendedSuppressPlayerUpdate = false;
    g_extendedToggleActionListRequested = false;
    g_extendedToggleEquipmentPreviewRequested = false;
    g_extendedToggleEquipmentCatalogBatchRequested = false;
    g_extendedActionListOpen = false;
    g_extendedSceneSelectorOpen = false;
    g_extendedExitRequested = false;
    g_extendedExitPreviousState = ExtendedMode_Inactive;
    g_extendedOpenSystemMenuRequested = false;
    g_extendedOriginalSystemMenuOpen = false;
    g_extendedOriginalSystemMenuVisible = false;
    g_extendedOriginalTutorialActive = false;
    g_extendedOriginalInteractionMenuActive = false;
    g_extendedOriginalInteractionMenuClickRequested = false;
    g_extendedOriginalHandcuffsPanelActive = false;
    g_extendedOriginalHandcuffsPanelClickRequested = false;
    g_extendedHandcuffsPanelInstance = nullptr;
    g_extendedHandcuffsPanelNextDiscoveryAt = 0;
    g_gameplayEyeMaskModelApplyAt = 0;
    g_gameplayVibratorModelApplyAt = 0;
    g_gameplayVibratorModelType = -1;
    g_gameplayVibratorPreviousMode = -1;
    g_gameplayVibratorNextMode = -1;
    g_gameplayVibratorLogicalMode = 0;
    g_gameplayAdultQuickSlotRequested = -1;
    g_gameplayConfiguredEyeMaskRequested = false;
    g_gameplayEyeMaskForceThirdPersonRequested = false;
    EyeMaskVision_ResetCameraTracking();
    for (auto& physicalDown :
         g_gameplayAdultQuickSlotPhysicalDown)
    {
        physicalDown = false;
    }
    g_extendedOpenSceneSelectorRequested = false;
    g_extendedEnterGameRequested = false;
    g_extendedGameplayActive = false;
    g_extendedTransitionInvoked = false;
    g_extendedDirectWarehouseTransition = false;
    g_extendedSceneLoadStarted = false;
    g_extendedTargetAlreadyActive = false;
    g_extendedTimeApplied = false;
    g_extendedSettledStageFrames = 0;
    g_extendedGameplayStableFrames = -1;
    g_extendedBindingCandidate = nullptr;
    g_extendedBindingManagerCandidate = nullptr;
    g_extendedPlayerBeforeSceneLoad = nullptr;
    g_extendedBindingStableFrames = 0;
    g_extendedOutfitRefreshStage = 0;
    g_extendedGameplayHudFrame = 0;
    g_extendedPendingAction = -1;
    g_extendedActionPage = 0;
    g_extendedSelectedAction = 0;
    g_extendedSelectedStageIndex = 0;
    g_extendedSelectedDaytime = true;
    g_extendedTargetStageType = 0;
    g_extendedTargetDaytime = true;
    g_extendedModelDragging = false;
    g_extendedModelLeftWasDown = false;
    g_extendedModelYaw = 0.0f;
    g_extendedRoomCenterX = 0.0f;
    g_extendedRoomFloorY = 0.0f;
    g_extendedRoomCenterZ = 0.0f;
    g_extendedModelCenterX = 0.0f;
    g_extendedModelCenterY = 0.0f;
    g_extendedModelCenterZ = 0.0f;
    g_extendedModelForwardX = 0.0f;
    g_extendedModelForwardZ = 1.0f;
    g_extendedModelBaseLocalEulerX = 0.0f;
    g_extendedModelBaseLocalEulerY = 0.0f;
    g_extendedModelBaseLocalEulerZ = 0.0f;
    g_extendedOriginalLocalEulerX = 0.0f;
    g_extendedOriginalLocalEulerY = 0.0f;
    g_extendedOriginalLocalEulerZ = 0.0f;
    g_extendedReframeCountdown = 0;
    g_extendedSavePanel = nullptr;
    g_extendedInGameManager = nullptr;
    g_extendedInGameReferencer = nullptr;
    g_extendedPlayerController = nullptr;
    g_extendedPlayerGameObject = nullptr;
    g_extendedAvatarReferencer = nullptr;
    g_extendedAvatarRoot = nullptr;
    g_extendedCamera = nullptr;
    g_extendedInGameUiManager = nullptr;
    g_extendedCanvas = nullptr;
    g_extendedActionPanel = nullptr;
    g_extendedSelectedActionLabel = nullptr;
    g_extendedPageLabel = nullptr;
    g_extendedPrevPageButton = nullptr;
    g_extendedNextPageButton = nullptr;
    g_extendedActionButtons.clear();
    g_extendedActionLabels.clear();
    g_extendedLayerSnapshots.clear();
    g_extendedAnimatorStateSnapshot = ExtendedAnimatorStateSnapshot{};
    g_extendedEnterGameButton = SceneUiElement{};
    g_extendedSceneSelectorPanel = SceneUiElement{};
    for (int index = 0; index < kEscapeFromManakaStageCount; ++index)
    {
        g_extendedSceneButtons[index] = SceneUiElement{};
    }
    g_extendedDayButton = SceneUiElement{};
    g_extendedNightButton = SceneUiElement{};
    g_extendedConfirmEnterButton = SceneUiElement{};
    g_extendedCancelEnterButton = SceneUiElement{};
    g_extendedSceneSelectionHint = nullptr;
    g_extendedGameplayHudCanvasObject = nullptr;
    g_extendedCoordinateLabel = nullptr;
    g_extendedInteractionPromptPanel = nullptr;
    g_extendedInteractionPromptPanelRect = nullptr;
    g_extendedInteractionPromptActionRect = nullptr;
    g_extendedInteractionPromptActionLabel = nullptr;
    g_extendedInteractionPromptKeyLabel = nullptr;
    g_extendedInteractionPromptPrefix = SceneUiElement{};
    g_extendedInteractionPromptKeyBadge = SceneUiElement{};
    g_extendedInteractionPromptStates[0] =
        ExtendedInteractionPromptState{};
    g_extendedInteractionPromptStates[1] =
        ExtendedInteractionPromptState{};
    g_extendedCoatModeButton = SceneUiElement{};
    g_extendedPoseMenuPanel = nullptr;
    for (auto& button : g_extendedPoseMenuButtons)
    {
        button = SceneUiElement{};
    }
    g_previewModel = nullptr;
    g_previewAnimator = nullptr;
    g_extendedLastResolvedInputData = nullptr;
    g_gameplayDisabledInputActions.clear();
    EquipmentPreviewUi_Reset();
    InventoryUi_Reset();
    GameplayInventory_Reset();
    g_gameplayInventoryOpen = false;
}

void ExtendedMode_LogClassSurfaceOnce(
    const char* namespaceName,
    const char* className)
{
    auto* klass = FindClass(namespaceName, className);
    if (klass == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Gameplay surface unavailable: %S.%S.",
            namespaceName,
            className);
        return;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(klass);
    }
    LogInfo(LogCategory_Action, L"Action", L"Gameplay surface %S.%S fields:", namespaceName, className);
    void* fieldIterator = nullptr;
    while (const auto field =
        il2cpp_class_get_fields(klass, &fieldIterator))
    {
        const auto* fieldName = il2cpp_field_get_name(field);
        const auto offset =
            il2cpp_field_get_offset != nullptr
                ? il2cpp_field_get_offset(field)
                : -1;
        LogInfo(LogCategory_Action, L"Action", L"  field %S offset=0x%X.", fieldName, offset);
    }
    void* methodIterator = nullptr;
    while (const auto method =
        il2cpp_class_get_methods(klass, &methodIterator))
    {
        const auto* methodName = il2cpp_method_get_name(method);
        const int parameterCount =
            il2cpp_method_get_param_count(method);
        LogInfo(LogCategory_Action, L"Action", 
            L"  method %S parameterCount=%d.",
            methodName,
            parameterCount);
    }
}

void ExtendedMode_Fail(const wchar_t* reason)
{
    LogInfo(LogCategory_Action, L"Action", L"Extended mode aborted: %s", reason);
    SetStatus(std::wstring(L"ESCAPE FROM MANAKA: ") + reason);
    g_extendedWarehouseReturnInProgress = false;
    g_extendedWarehouseCoverHideDelayFrames = 0;
    ExtendedMode_SetOriginalTutorialSuppressed(
        false,
        L"MOD startup failure");
    ExtendedMode_ResetForTitle();
}

void ExtendedMode_Begin()
{
    if (ExtendedMode_IsRunning())
    {
        return;
    }
    ExtendedMode_ResetForTitle();
    ExtendedMode_SetOriginalTutorialSuppressed(
        true,
        L"MOD session entry");
    ExtendedMode_CacheFont();
    if (g_titleInstance == nullptr)
    {
        ExtendedMode_Fail(L"title scene is unavailable");
        return;
    }
    auto* onClickContinue = FindMethod(g_titleClass, "OnClickContinue", 0);
    if (onClickContinue == nullptr)
    {
        ExtendedMode_Fail(L"TitleSceneView.OnClickContinue is unavailable");
        return;
    }
    g_extendedWarehouseReturnInProgress = true;
    g_extendedWarehouseCoverHideDelayFrames = 0;
    ExtendedMode_ShowTransferLoadingOverlay();
    g_extendedModeState = ExtendedMode_OpeningSavePanel;
    g_extendedStateFrames = 0;
    g_lobbyUiOpen = false;
    Invoke(onClickContinue, g_titleInstance, nullptr);
    SetStatus(L"ESCAPE FROM MANAKA: opening manual save slot 0");
    LogInfo(LogCategory_Action, L"Action", L"Extended mode opened the original continue/save selection flow.");
}

bool ExtendedMode_SetSaveManagerCurrentSlot(int slot)
{
    auto* saveManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "SaveManager");
    auto* currentSlotField =
        saveManagerClass != nullptr &&
        il2cpp_class_get_field_from_name != nullptr
            ? il2cpp_class_get_field_from_name(
                saveManagerClass,
                "CurrentSaveSlot")
            : nullptr;
    if (currentSlotField == nullptr ||
        il2cpp_field_get_offset == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"SaveManager.CurrentSaveSlot field unavailable: class=%p field=%p.",
            saveManagerClass,
            currentSlotField);
        return false;
    }
    auto* staticFields = *reinterpret_cast<char**>(
        reinterpret_cast<char*>(saveManagerClass) + 0xB8);
    const int fieldOffset = il2cpp_field_get_offset(currentSlotField);
    if (staticFields == nullptr || fieldOffset < 0)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"SaveManager static field storage unavailable: static=%p offset=%d.",
            staticFields,
            fieldOffset);
        return false;
    }
    int requestedSlot = slot;
    if (il2cpp_field_static_set_value != nullptr)
    {
        il2cpp_field_static_set_value(currentSlotField, &requestedSlot);
    }
    *reinterpret_cast<int*>(staticFields + fieldOffset) = requestedSlot;
    const int observedSlot =
        *reinterpret_cast<int*>(staticFields + fieldOffset);
    LogInfo(LogCategory_Action, L"Action", 
        L"SaveManager.CurrentSaveSlot requested=%d observed=%d field=%p static=%p offset=0x%X.",
        slot,
        observedSlot,
        currentSlotField,
        staticFields,
        fieldOffset);
    return observedSlot == slot;
}

void ExtendedMode_HandleSavePanel()
{
    ++g_extendedStateFrames;
    if (g_extendedSavePanel == nullptr)
    {
        g_extendedSavePanel = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.ObjectUI.SaveDataSelectPanel",
            "SaveDataSelectPanelView",
            "instance");
    }
    if (g_extendedSavePanel == nullptr)
    {
        if (g_extendedStateFrames.load() > 240)
        {
            ExtendedMode_Fail(L"save selection panel did not open");
        }
        return;
    }
    if (g_extendedSaveSelectionInvoked.exchange(true))
    {
        return;
    }

    auto* saveManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "SaveManager");
    auto* isExist = FindMethod(saveManagerClass, "IsExistSaveData", 2);
    int saveType = 3; // SaveRapper.SaveDataType.ManualSlot
    int slot = 0;
    void* existParameters[] = { &saveType, &slot };
    auto* existsValue = Invoke(isExist, nullptr, existParameters);
    const bool exists = existsValue != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(existsValue));
    if (!exists)
    {
        LogInfo(LogCategory_Action, L"Action", L"Manual save slot 0 is empty; initializing it through the original NewGame path.");
        auto* saveManagerClass = FindClass(
            "ExposureUnnoticed2.Scripts.InGame",
            "SaveManager");
        auto* load = FindMethod(saveManagerClass, "Load", 2);
        auto* savePanelClass = FindClass(
            "ExposureUnnoticed2.ObjectUI.SaveDataSelectPanel",
            "SaveDataSelectPanelView");
        auto* transScene = FindMethod(
            savePanelClass,
            "TransScene",
            0);
        if (load == nullptr || transScene == nullptr)
        {
            ExtendedMode_Fail(
                L"original NewGame initialization methods are unavailable");
            return;
        }
        if (!ExtendedMode_SetSaveManagerCurrentSlot(0))
        {
            ExtendedMode_Fail(
                L"SaveManager.CurrentSaveSlot could not be set to slot 0");
            return;
        }

        g_extendedDefaultSavePending = true;
        g_extendedDefaultSaveWriteAttempts = 0;
        int newGameType = 5; // SaveRapper.SaveDataType.NewGame
        int newGameSlot = 0;
        void* loadParameters[] = { &newGameType, &newGameSlot };
        Invoke(load, nullptr, loadParameters);

        auto* gameStateData = ExtendedMode_GetGameStateData();
        auto* playerCustomData = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Scripts.InGame",
            "GameState",
            "PlayerCustomData");
        auto* tutorialData = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Scripts.InGame",
            "GameState",
            "TutorialData");
        auto* optionData = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Scripts.InGame",
            "GameState",
            "OptionData");
        if (gameStateData == nullptr ||
            playerCustomData == nullptr ||
            tutorialData == nullptr ||
            optionData == nullptr)
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Original NewGame load left invalid state: game=%p player=%p tutorial=%p options=%p.",
                gameStateData,
                playerCustomData,
                tutorialData,
                optionData);
            g_extendedDefaultSavePending = false;
            ExtendedMode_Fail(L"original NewGame state initialization failed");
            return;
        }

        g_extendedModeState = ExtendedMode_LoadingGameplay;
        g_extendedStateFrames = 0;
        Invoke(transScene, g_extendedSavePanel, nullptr);
        SetStatus(L"ESCAPE FROM MANAKA: creating the original default save");
        LogInfo(LogCategory_Action, L"Action", 
            L"Extended mode loaded original NewGame defaults and started TransScene; waiting for the player before creating slot 0.");
        return;
    }

    auto* savePanelClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.SaveDataSelectPanel",
        "SaveDataSelectPanelView");
    auto* onClickItem = FindMethod(savePanelClass, "OnClickItem", 2);
    if (onClickItem == nullptr)
    {
        ExtendedMode_Fail(L"SaveDataSelectPanelView.OnClickItem is unavailable");
        return;
    }
    void* clickParameters[] = { &saveType, &slot };
    g_extendedModeState = ExtendedMode_LoadingGameplay;
    g_extendedStateFrames = 0;
    Invoke(onClickItem, g_extendedSavePanel, clickParameters);
    SetStatus(L"ESCAPE FROM MANAKA: loading original manual save slot 0");
    LogInfo(LogCategory_Action, L"Action", L"Extended mode invoked original SaveDataSelectPanelView.OnClickItem(3, 0).");
}

bool ExtendedMode_IsGameplayManagerHealthy()
{
    auto* eventManager = g_extendedActiveEventMethodManager;
    auto* referencer = g_extendedInGameReferencer;
    auto* inGameManager = g_extendedInGameManager;
    auto* player = g_extendedPlayerController;
    if (eventManager == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(eventManager) ||
        referencer == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(referencer) ||
        inGameManager == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(inGameManager) ||
        player == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(player))
    {
        return false;
    }

    // Scene objects can be rebuilt in a different order between stages.
    // Core liveness plus a short update window is sufficient here; deep
    // cross-reference identity churn made otherwise healthy stages stall.
    return g_extendedEventManagerUpdateFrames >= 2 &&
           !ExtendedMode_IsOriginalSystemMenuActive();
}

void ExtendedMode_LogGameplayBindingDiagnostic(const wchar_t* source)
{
    auto* eventManager = g_extendedActiveEventMethodManager;
    auto* referencer = g_extendedInGameReferencer;
    auto* inGameManager = g_extendedInGameManager;
    auto* player = g_extendedPlayerController;
    auto* eventReferencer = eventManager != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(eventManager) + 0x38)
        : nullptr;
    auto* referencerManager = referencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(referencer) + 0x20)
        : nullptr;
    auto* referencerPlayer = referencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(referencer) + 0x30)
        : nullptr;
    auto* managerReferencer = inGameManager != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(inGameManager) + 0x28)
        : nullptr;
    auto* managerEventMethod = inGameManager != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(inGameManager) + 0x78)
        : nullptr;
    LogInfo(
        LogCategory_Action,
        L"Action",
        L"Scene transition diagnostic (%s): stage=%d target=%d "
        L"bound=%d healthy=%d menu=%d updateFrames=%d stable=%d "
        L"settled=%d spawn=%d time=%d event=%p ref=%p inGame=%p "
        L"player=%p ui=%p eventRef=%p refManager=%p refPlayer=%p "
        L"managerRef=%p managerEvent=%p.",
        source != nullptr ? source : L"unknown",
        ExtendedMode_GetLoadedStageType(),
        g_extendedTargetStageType.load(),
        g_extendedPlayerGameObject != nullptr &&
            g_extendedAvatarRoot != nullptr &&
            g_previewAnimator != nullptr,
        ExtendedMode_IsGameplayManagerHealthy() ? 1 : 0,
        ExtendedMode_IsOriginalSystemMenuActive() ? 1 : 0,
        g_extendedEventManagerUpdateFrames,
        g_extendedSceneBindingStableFrames,
        g_extendedSettledStageFrames,
        g_extendedSpawnPositionApplied ? 1 : 0,
        g_extendedTimeApplied ? 1 : 0,
        eventManager,
        referencer,
        inGameManager,
        player,
        g_extendedInGameUiManager,
        eventReferencer,
        referencerManager,
        referencerPlayer,
        managerReferencer,
        managerEventMethod);
}

bool ExtendedMode_TryBindGameplay(Il2CppObject* playerInstance)
{
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedPlayerController))
    {
        g_extendedPlayerController = nullptr;
    }
    if (playerInstance != nullptr)
    {
        g_extendedPlayerController = playerInstance;
    }
    if (g_extendedPlayerController == nullptr)
    {
        g_extendedPlayerController = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Object3D.Player.Scripts",
            "PlayerController",
            "Instance");
    }
    if (g_extendedPlayerController == nullptr)
    {
        return false;
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedInGameManager))
    {
        g_extendedInGameManager = nullptr;
        g_extendedInGameManager = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Scripts.InGame",
            "InGameManager",
            "Instance");
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedInGameReferencer))
    {
        g_extendedInGameReferencer = nullptr;
    }
    if (g_extendedInGameManager != nullptr &&
        g_extendedInGameReferencer == nullptr)
    {
        g_extendedInGameReferencer = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedInGameManager) + 0x28);
    }
    if (g_extendedAvatarReferencer == nullptr)
    {
        g_extendedAvatarReferencer = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x30);
    }
    auto* playerClassAccessor = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_extendedPlayerController) + 0x40);
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedPlayerGameObject))
    {
        g_extendedPlayerGameObject = nullptr;
    }
    if (g_extendedPlayerGameObject == nullptr)
    {
        g_extendedPlayerGameObject = playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) + 0x28)
            : GetComponentGameObject(g_extendedPlayerController);
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedAvatarRoot))
    {
        g_extendedAvatarRoot = nullptr;
    }
    if (g_extendedAvatarRoot == nullptr && playerClassAccessor != nullptr)
    {
        auto* avatarTransform = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x20);
        g_extendedAvatarRoot = GetComponentGameObject(avatarTransform);
    }
    if (g_extendedAvatarRoot == nullptr &&
        g_extendedAvatarReferencer != nullptr)
    {
        g_extendedAvatarRoot = GetComponentGameObject(g_extendedAvatarReferencer);
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_previewAnimator))
    {
        g_previewAnimator = nullptr;
    }
    if (g_previewAnimator == nullptr && playerClassAccessor != nullptr)
    {
        g_previewAnimator = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x10);
        if (g_previewAnimator != nullptr)
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Extended mode bound PlayerClassAccessor.Animator=%p.",
                g_previewAnimator);
        }
    }
    if (g_previewAnimator == nullptr && g_extendedAvatarRoot != nullptr)
    {
        g_previewAnimator = ExtendedMode_FindPlayerAnimator(
            g_extendedAvatarRoot);
    }
    ExtendedMode_CaptureAnimatorState();
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedInGameUiManager))
    {
        g_extendedInGameUiManager = nullptr;
        g_extendedInGameUiManager = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Object3D.IngameManager",
            "InGameUiManager",
            "Instance");
    }

    return g_extendedPlayerGameObject != nullptr &&
           g_extendedAvatarRoot != nullptr &&
           g_previewAnimator != nullptr &&
           g_extendedInGameManager != nullptr &&
           g_extendedInGameReferencer != nullptr &&
           g_extendedInGameUiManager != nullptr;
}

void ExtendedMode_TakeOverGameplay()
{
    if (g_extendedDefaultSavePending.load())
    {
        LogInfo(LogCategory_Action, L"Action", L"Gameplay takeover deferred until the default manual save is ready.");
        return;
    }
    g_extendedModeState = ExtendedMode_PreparingGameplay;
    g_extendedStateFrames = 0;
    g_extendedLegDiagnosticFrame = 0;
    ExtendedMode_DisableGameplayControllers();
    ExtendedMode_HideGameplayUi();
    ExtendedMode_SilenceGameplayAudio();
    ExtendedMode_StopSceneParticles();
    ExtendedMode_ForceCursor();

    g_extendedLayerSnapshots.clear();
    ExtendedMode_CaptureHierarchyLayers(g_extendedPlayerGameObject);
    if (g_extendedAvatarRoot != g_extendedPlayerGameObject)
    {
        ExtendedMode_CaptureHierarchyLayers(g_extendedAvatarRoot);
    }
    ExtendedMode_SetHierarchyLayer(
        g_extendedPlayerGameObject,
        kExtendedRoomLayer);
    ExtendedMode_SetHierarchyLayer(
        g_extendedAvatarRoot,
        kExtendedRoomLayer);

    g_previewModel = g_extendedAvatarRoot;
    EscapeFromManaka_ApplyClosetHairCustomization();
    if (g_previewAnimator == nullptr)
    {
        g_previewAnimator = ExtendedMode_FindPlayerAnimator(
            g_extendedAvatarRoot);
    }
    ExtendedMode_CapturePlayerFrame();
    LogEscapeFromManakaLegState(L"after-bind");
    // Clothing visibility is owned by the equipment UI below. Directly
    // rewriting PlayerState/GameState clothing fields at this point triggers a
    // delayed body-scale rebuild which can reparent the lower-leg chain with
    // zero local offsets, gluing the feet to the hips.
    LogEscapeFromManakaLegState(L"after-naked-state");
    auto* avatarTransform = GetWorldTransform(g_extendedAvatarRoot);
    // Presentation rotation is independent from the saved gameplay facing.
    // The avatar's forward axis is opposite the camera-facing preview pose.
    g_extendedModelBaseLocalEulerY = 0.0f;
    g_extendedModelYaw = 0.0f;
    SetTransformLocalEuler(
        avatarTransform,
        g_extendedModelBaseLocalEulerX,
        180.0f,
        g_extendedModelBaseLocalEulerZ);
    ExtendedMode_PrepareOriginalAnimator();
    LogEscapeFromManakaLegState(L"after-animator-parameters");
    ExtendedMode_PrepareSkinnedRenderers();
    ExtendedMode_ResetDynamicBones();
    LogEscapeFromManakaLegState(L"after-dynamic-bones");
    // The equipment page owns clothing visibility now. InventoryUi_Build()
    // applies the current loadout after all avatar references are available.
    ExtendedMode_UpdateFutanariVisibility();

    ExtendedMode_ConfigureCamera();
    ExtendedMode_BuildBlackRoom();
    ExtendedMode_BuildUi();

    // LocomotionTree is the neutral full-body state. Speed=0 and
    // IsStrafe=false keep it out of the shipped walk/strafe sub-trees.
    g_extendedSelectedAction = 0;
    EscapeFromManaka_SelectAction(0);
    EscapeFromManaka_ApplyOriginalAction(0);
    LogEscapeFromManakaLegState(L"after-locomotion-play");
    ExtendedMode_UpdateSelectedActionLabel();

    g_extendedSuppressPlayerUpdate = true;
    g_extendedTakeoverComplete = true;
    g_extendedReframeCountdown = 5;
    g_extendedModeState = ExtendedMode_Active;
    g_extendedStateFrames = 0;
    ExtendedMode_ApplyCoatlessLogic(true);
    SetStatus(L"ESCAPE FROM MANAKA: loaded slot 0 and entered the black room");
    LogInfo(LogCategory_Action, L"Action", L"Extended mode takeover completed with the original gameplay avatar.");
    if (g_extendedWarehouseReturnInProgress.load() &&
        g_extendedPendingDeploymentStageType < 0)
    {
        g_extendedWarehouseCoverHideDelayFrames = 3;
        LogInfo(LogCategory_Action, L"Action", L"Warehouse UI is ready; retaining its loading cover for three rendered frames.");
    }
    if (g_extendedPendingDeploymentStageType >= 0)
    {
        g_extendedPendingDeploymentDelayFrames = 30;
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape fresh warehouse is stable; deployment to stage %d will start after 30 frames.",
            g_extendedPendingDeploymentStageType);
    }
}

void ExtendedMode_ExitToTitle()
{
    GameplayInput_RestoreVibratorSaveState();
    const int previousState = g_extendedModeState.load();
    g_extendedReloadAfterMenuOpenFrames = 0;
    const bool returnToWarehouse =
        g_extendedReturnToWarehouseAfterExit;
    if (previousState == ExtendedMode_Inactive)
    {
        return;
    }
    if (g_extendedSpecialCoatMode.load())
    {
        ExtendedMode_SetSpecialCoatMode(false);
    }
    const bool retry = previousState == ExtendedMode_Exiting;
    if (!retry)
    {
        g_extendedExitPreviousState = previousState;
    }
    g_extendedModeState = ExtendedMode_Exiting;
    g_extendedExitRequested = false;
    g_extendedSuppressPlayerUpdate = true;

    auto* systemMenuClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.SystemMenu",
        "SystemMenuView");
    auto* systemMenuInstance = systemMenuClass != nullptr
        ? Invoke(
            FindMethod(systemMenuClass, "get_Instance", 0),
            nullptr,
            nullptr)
        : nullptr;
    auto* saveAndGoToTitle = systemMenuClass != nullptr
        ? FindMethod(systemMenuClass, "OnClickSaveAndGoToTitle", 0)
        : nullptr;
    if (!retry)
    {
        g_extendedGameplayUpdateObserved = false;
        g_extendedBindingCandidate = nullptr;
        g_extendedBindingManagerCandidate = nullptr;
        g_extendedBindingStableFrames = 0;
        g_extendedOutfitRefreshStage = 0;
        // TitleUpdateHook installs the new scene instance before the Exiting
        // state can complete its hand-off. Keeping the destroyed pointer here
        // makes the first unrelated gameplay hook look like a completed load.
        g_titleInstance = nullptr;
        InventoryUi_SaveState();
        ExtendedMode_DestroyRuntimeWindows(
            returnToWarehouse
                ? L"the warehouse title reload"
                : L"the title transition");
        ExtendedMode_RestoreHierarchyLayers();
        g_extendedInGameManager = nullptr;
        g_extendedInGameReferencer = nullptr;
        g_extendedInGameUiManager = nullptr;
        g_extendedCamera = nullptr;
        g_extendedPlayerController = nullptr;
        g_extendedPlayerGameObject = nullptr;
        g_extendedAvatarReferencer = nullptr;
        g_extendedAvatarRoot = nullptr;
        g_previewModel = nullptr;
        g_previewAnimator = nullptr;
        g_extendedLastResolvedInputData = nullptr;
        g_extendedLayerSnapshots.clear();
        g_extendedAnimatorStateSnapshot =
            ExtendedAnimatorStateSnapshot{};
        ExtendedMode_ForceCursor();
        LogInfo(LogCategory_Action, L"Action", L"Extended mode released all gameplay bindings before the scene reload.");
    }
    else
    {
        LogInfo(LogCategory_Action, L"Action", L"Extended mode is retrying the interrupted Title transition.");
    }

    SetStatus(
        returnToWarehouse
            ? L"ESCAPE FROM MANAKA: ending run and returning to warehouse"
            : L"ESCAPE FROM MANAKA: exiting to Title");
    LogInfo(LogCategory_Action, L"Action", 
        retry
            ? L"Extended mode retried the clean scene transition."
            : (returnToWarehouse
                ? L"Extended mode requested a run-ending warehouse transition."
                : L"Extended mode requested a clean Title scene through Esc."));
    if (systemMenuInstance != nullptr &&
        saveAndGoToTitle != nullptr &&
        g_originalSystemMenuSaveAndGoToTitle != nullptr)
    {
        using OriginalFn =
            void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalSystemMenuSaveAndGoToTitle)(
                systemMenuInstance,
                saveAndGoToTitle);
        g_extendedAwaitingReturnConfirmation = true;
        ExtendedMode_SetCustomUiBehindSystemMenu(true);
        LogInfo(LogCategory_Action, L"Action", L"Extended mode delegated player and scene cleanup to the original title-menu transition.");
    }
    else
    {
        if (ExtendedMode_IsUnityObjectAlive(systemMenuInstance))
        {
            SetWorldGameObjectActive(
                GetComponentGameObject(systemMenuInstance),
                false);
            g_extendedOriginalSystemMenuOpen = false;
            g_extendedOriginalSystemMenuVisible = false;
            ExtendedMode_SetCustomUiBehindSystemMenu(false);
            LogInfo(
                LogCategory_Action,
                L"Action",
                L"Escape hid the original system menu before the direct Title transition.");
        }
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
        LogInfo(LogCategory_Action, L"Action", L"Extended mode used the direct Title-scene fallback.");
    }
}

void ExtendedMode_RequestReturnToWarehouse(
    bool systemMenuAlreadyOpen = false)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }
    EquipmentStats_DiscardRaidEarnings(
        systemMenuAlreadyOpen
            ? L"system-menu warehouse return"
            : L"non-extraction warehouse return");
    g_extendedReturnToWarehouseAfterExit = true;
    g_extendedWarehouseReturnInProgress = true;
    g_extendedWarehouseCoverHideDelayFrames = 0;
    g_extendedPendingDeploymentStageType = -1;
    g_extendedPendingDeploymentDelayFrames = 0;
    g_extendedSuppressPlayerUpdate = true;
    GameplayInventory_Reset();
    LogInfo(LogCategory_Action, L"Action", 
        systemMenuAlreadyOpen
            ? L"Escape return-to-title button requested the stable warehouse return."
            : L"Escape run ended; requesting the stable warehouse return.");
    ExtendedMode_ShowTransferLoadingOverlay();
    g_extendedOpenSystemMenuRequested = !systemMenuAlreadyOpen;
    g_extendedReloadAfterMenuOpenFrames = 30;
}

bool ExtendedMode_RequestFailureReturnToWarehouse()
{
    EquipmentStats_DiscardRaidEarnings(L"game failure");
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Failure warehouse return ignored in state %d.",
            g_extendedModeState.load());
        return false;
    }
    g_extendedReturnToWarehouseAfterExit = false;
    g_extendedWarehouseReturnInProgress = true;
    g_extendedWarehouseCoverHideDelayFrames = 0;
    g_extendedPendingDeploymentStageType = -1;
    g_extendedPendingDeploymentDelayFrames = 0;
    g_extendedSuppressPlayerUpdate = true;
    g_extendedOpenSystemMenuRequested = false;
    g_extendedExitRequested = false;
    GameplayInventory_Reset();
    ExtendedMode_ShowTransferLoadingOverlay();
    if (!ExtendedMode_QueueSceneTransition(
            0,
            L"failure return to warehouse",
            true,
            false))
    {
        g_extendedWarehouseReturnInProgress = false;
        ExtendedMode_HideTransferLoadingOverlay();
        LogInfo(LogCategory_Action, L"Action", L"Failure warehouse return could not queue the Stage 0 transition.");
        return false;
    }
    LogInfo(LogCategory_Action, L"Action", L"Failure warehouse return queued a direct safe transition to Stage 0.");
    return true;
}

bool ExtendedMode_IsOriginalSystemMenuActive()
{
    auto* systemMenuClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.SystemMenu",
        "SystemMenuView");
    if (systemMenuClass == nullptr)
    {
        return false;
    }
    auto* instance = Invoke(
        FindMethod(systemMenuClass, "get_Instance", 0),
        nullptr,
        nullptr);
    if (instance != nullptr &&
        !ExtendedMode_IsUnityObjectAlive(instance))
    {
        return false;
    }
    auto* basePanelClass = FindClass("Common.Scripts.UI", "BasePanelView");
    auto* isActive = FindMethod(basePanelClass, "IsActivePanel", 0);
    auto* isActiveValue = instance != nullptr
        ? Invoke(isActive, instance, nullptr)
        : nullptr;
    return isActiveValue != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(isActiveValue));
}

void ExtendedMode_RefreshOriginalTutorialActive()
{
    const int state = g_extendedModeState.load();
    if (state != ExtendedMode_Active &&
        state != ExtendedMode_Gameplay)
    {
        g_extendedOriginalTutorialActive = false;
        return;
    }

    bool active = false;
    auto* managerClass = FindClass(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "InGameUiManager");
    auto* tutorialClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.TutorialPanel",
        "TutorialPanelView");
    auto* manager = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "InGameUiManager",
        "Instance");
    auto* getCurrent = managerClass != nullptr
        ? FindMethod(managerClass, "GetCurrentBasePanelView", 0)
        : nullptr;
    if (manager != nullptr &&
        tutorialClass != nullptr &&
        getCurrent != nullptr &&
        il2cpp_object_get_class != nullptr)
    {
        auto* current = Invoke(getCurrent, manager, nullptr);
        if (current != nullptr &&
            ExtendedMode_IsUnityObjectAlive(current))
        {
            active =
                il2cpp_object_get_class(current) == tutorialClass;
        }
    }

    const bool previous =
        g_extendedOriginalTutorialActive.exchange(active);
    if (active != previous)
    {
        if (active)
        {
            ExtendedMode_SetCustomUiBehindSystemMenu(true);
        }
        else if (!ExtendedMode_IsOriginalSystemMenuActive())
        {
            ExtendedMode_SetCustomUiBehindSystemMenu(false);
        }
        LogInfo(LogCategory_Action, L"Action", 
            active
                ? L"Original tutorial panel acquired MOD cursor ownership."
                : L"Original tutorial panel closed; MOD modal state cleared.");
    }
}

bool ExtendedMode_IsOriginalTutorialActive()
{
    return g_extendedOriginalTutorialActive.load();
}

void ExtendedMode_CloseOriginalSystemMenuForSceneChange()
{
    // Do not call SystemMenuView.MenuClose here. The instance exists even when
    // the panel is closed, and invoking its close path in that state crashes
    // inside UnityPlayer. Scene loading and the full-screen cover own the
    // visual transition; only the cached active flags need clearing.
    g_extendedOriginalSystemMenuOpen = false;
    g_extendedOriginalSystemMenuVisible = false;
    ExtendedMode_SetCustomUiBehindSystemMenu(false);
}

bool ExtendedMode_IsOriginalInteractionMenuActive()
{
    auto* interactionMenuClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.InteractMenuPanel",
        "InteractMenuPanelView");
    auto* getInstance = FindMethod(interactionMenuClass, "get_Instance", 0);
    auto* instance = getInstance != nullptr
        ? Invoke(getInstance, nullptr, nullptr)
        : nullptr;
    if (instance == nullptr)
    {
        return false;
    }
    auto* basePanelClass = FindClass("Common.Scripts.UI", "BasePanelView");
    auto* isActive = FindMethod(basePanelClass, "IsActivePanel", 0);
    auto* isActiveValue = isActive != nullptr
        ? Invoke(isActive, instance, nullptr)
        : nullptr;
    return isActiveValue != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(isActiveValue));
}

void ExtendedMode_RefreshOriginalHandcuffsPanelActive()
{
    const unsigned long long now = GetTickCount64();
    if (!ExtendedMode_IsUnityObjectAlive(
            g_extendedHandcuffsPanelInstance))
    {
        g_extendedHandcuffsPanelInstance = nullptr;
    }
    if (g_extendedHandcuffsPanelInstance == nullptr &&
        now >= g_extendedHandcuffsPanelNextDiscoveryAt)
    {
        g_extendedHandcuffsPanelNextDiscoveryAt = now + 5000;
        auto* panelClass = FindClass(
            "ExposureUnnoticed2.ObjectUI.ChooseHandcuffsPanel",
            "ChooseHandcuffsPanelView");
        auto* objectClass = FindClass("UnityEngine", "Object");
        auto* findAll = FindMethod(
            objectClass,
            "FindObjectsOfType",
            2);
        auto* basePanelClass = FindClass(
            "Common.Scripts.UI",
            "BasePanelView");
        auto* isActive = FindMethod(
            basePanelClass,
            "IsActivePanel",
            0);
        if (panelClass != nullptr &&
            findAll != nullptr &&
            isActive != nullptr &&
            il2cpp_runtime_class_init != nullptr)
        {
            il2cpp_runtime_class_init(panelClass);
            auto* managedType = il2cpp_type_get_object(
                il2cpp_class_get_type(panelClass));
            bool includeInactive = true;
            void* parameters[] = { managedType, &includeInactive };
            auto* panels = Invoke(
                findAll,
                nullptr,
                parameters);
            if (panels != nullptr)
            {
                const uint32_t count =
                    il2cpp_array_length(panels);
                auto* elements =
                    reinterpret_cast<Il2CppObject**>(
                        reinterpret_cast<char*>(panels) +
                        il2cpp_array_object_header_size());
                g_extendedHandcuffsPanelInstance = nullptr;
                for (uint32_t index = 0;
                     index < count;
                     ++index)
                {
                    auto* candidate = elements[index];
                    if (candidate == nullptr)
                    {
                        continue;
                    }
                    if (g_extendedHandcuffsPanelInstance == nullptr)
                    {
                        g_extendedHandcuffsPanelInstance = candidate;
                    }
                    auto* activeValue = Invoke(
                        isActive,
                        candidate,
                        nullptr);
                    if (activeValue != nullptr &&
                        il2cpp_object_unbox != nullptr &&
                        *static_cast<bool*>(
                            il2cpp_object_unbox(activeValue)))
                    {
                        g_extendedHandcuffsPanelInstance = candidate;
                        break;
                    }
                }
            }
        }
    }

    static const Il2CppMethod* isActivePanelMethod = nullptr;
    if (isActivePanelMethod == nullptr)
    {
        isActivePanelMethod = FindMethod(
            FindClass(
                "Common.Scripts.UI",
                "BasePanelView"),
            "IsActivePanel",
            0);
    }
    bool active = false;
    if (g_extendedHandcuffsPanelInstance != nullptr &&
        isActivePanelMethod != nullptr)
    {
        auto* isActiveValue = Invoke(
            isActivePanelMethod,
            g_extendedHandcuffsPanelInstance,
            nullptr);
        active =
            isActiveValue != nullptr &&
            il2cpp_object_unbox != nullptr &&
            *static_cast<bool*>(
                il2cpp_object_unbox(isActiveValue));
    }
    const bool wasActive =
        g_extendedOriginalHandcuffsPanelActive.exchange(active);
    if (active != wasActive)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape original handcuffs panel %s.",
            active ? L"opened" : L"closed");
    }
}

bool ExtendedMode_IsOriginalHandcuffsPanelActive()
{
    return g_extendedOriginalHandcuffsPanelActive.load();
}

void ExtendedMode_ActivateHoveredOriginalHandcuffsItem()
{
    if (g_extendedHandcuffsPanelInstance == nullptr ||
        !g_extendedOriginalHandcuffsPanelActive.load())
    {
        return;
    }
    auto* panel = g_extendedHandcuffsPanelInstance;
    auto* buttonGroup = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(panel) + 0x58);
    if (buttonGroup == nullptr)
    {
        auto* panelClass = FindClass(
            "ExposureUnnoticed2.ObjectUI.ChooseHandcuffsPanel",
            "ChooseHandcuffsPanelView");
        auto* cancel = FindMethod(panelClass, "OnClickCancel", 0);
        if (cancel != nullptr)
        {
            Invoke(cancel, panel, nullptr);
            LogInfo(
                LogCategory_Action,
                L"Action",
                L"Original handcuffs panel dispatched OnClickCancel "
                L"because no hovered choice group was available.");
        }
        return;
    }
    auto* buttons = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(buttonGroup) + 0x38);
    if (buttons == nullptr ||
        il2cpp_array_length == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        auto* panelClass = FindClass(
            "ExposureUnnoticed2.ObjectUI.ChooseHandcuffsPanel",
            "ChooseHandcuffsPanelView");
        auto* cancel = FindMethod(panelClass, "OnClickCancel", 0);
        if (cancel != nullptr)
        {
            Invoke(cancel, panel, nullptr);
            LogInfo(
                LogCategory_Action,
                L"Action",
                L"Original handcuffs panel dispatched OnClickCancel "
                L"because the choice button array was unavailable.");
        }
        return;
    }

    const uint32_t count = il2cpp_array_length(buttons);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(buttons) +
        il2cpp_array_object_header_size());
    int selectedIndex = -1;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* buttonView = elements[index];
        if (buttonView != nullptr &&
            *reinterpret_cast<unsigned char*>(
                reinterpret_cast<char*>(buttonView) + 0x71) != 0)
        {
            selectedIndex = static_cast<int>(index);
            break;
        }
    }
    if (selectedIndex < 0)
    {
        auto* panelClass = FindClass(
            "ExposureUnnoticed2.ObjectUI.ChooseHandcuffsPanel",
            "ChooseHandcuffsPanelView");
        auto* cancel = FindMethod(panelClass, "OnClickCancel", 0);
        if (cancel != nullptr)
        {
            Invoke(cancel, panel, nullptr);
            LogInfo(
                LogCategory_Action,
                L"Action",
                L"Original handcuffs panel dispatched OnClickCancel "
                L"for a click outside the PutBack/PutMap choices.");
        }
        return;
    }

    auto* panelClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.ChooseHandcuffsPanel",
        "ChooseHandcuffsPanelView");
    const char* methodNames[] = {
        "OnClickPutBack",
        "OnClickPutMap",
        "OnClickCancel",
    };
    if (selectedIndex >= 0 &&
        selectedIndex <
            static_cast<int>(
                sizeof(methodNames) / sizeof(methodNames[0])))
    {
        auto* method = FindMethod(
            panelClass,
            methodNames[selectedIndex],
            0);
        if (method != nullptr)
        {
            Invoke(method, panel, nullptr);
            LogInfo(LogCategory_Action, L"Action", 
                L"Original handcuffs panel dispatched %S for hovered index=%d.",
                methodNames[selectedIndex],
                selectedIndex);
        }
    }
}

void ExtendedMode_ActivateHoveredOriginalInteractionItem()
{
    auto* interactionMenuClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.InteractMenuPanel",
        "InteractMenuPanelView");
    auto* getInstance = FindMethod(interactionMenuClass, "get_Instance", 0);
    auto* panel = getInstance != nullptr
        ? Invoke(getInstance, nullptr, nullptr)
        : nullptr;
    if (panel == nullptr)
    {
        return;
    }

    // InteractMenuPanelView.buttonGroupManager is a serialized reference in
    // this game build. Its buttons expose isEntered on the concrete ButtonView.
    auto* buttonGroup = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(panel) + 0x68);
    if (buttonGroup == nullptr)
    {
        return;
    }
    auto* buttons = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(buttonGroup) + 0x38);
    if (buttons == nullptr ||
        il2cpp_array_length == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        return;
    }

    const uint32_t count = il2cpp_array_length(buttons);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(buttons) +
        il2cpp_array_object_header_size());
    auto* onClick = FindMethod(interactionMenuClass, "OnClick", 1);
    if (onClick == nullptr)
    {
        return;
    }
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* buttonView = elements[index];
        if (buttonView == nullptr ||
            *reinterpret_cast<unsigned char*>(
                reinterpret_cast<char*>(buttonView) + 0x71) == 0)
        {
            continue;
        }
        int selectedIndex = static_cast<int>(index);
        void* parameters[] = { &selectedIndex };
        Invoke(onClick, panel, parameters);
        LogInfo(LogCategory_Action, L"Action", 
            L"Original interaction menu dispatched OnClick index=%u.",
            index);
        return;
    }
}

void ExtendedMode_SetCustomUiBehindSystemMenu(bool behind)
{
    auto* canvasClass = FindClass("UnityEngine", "Canvas");
    auto* setSortingOrder = FindMethod(
        canvasClass,
        "set_sortingOrder",
        1);
    const Il2CppObject* canvasObjects[] = {
        g_extendedCanvas,
        g_extendedGameplayHudCanvasObject,
    };
    const int normalOrders[] = {
        kExtendedCanvasSortingOrder,
        kExtendedGameplayHudSortingOrder,
    };
    for (int index = 0; index < 2; ++index)
    {
        auto* canvasObject = const_cast<Il2CppObject*>(canvasObjects[index]);
        if (canvasObject == nullptr)
        {
            continue;
        }
        SetWorldGameObjectActive(canvasObject, !behind);
        auto* canvas = GetComponentByName(canvasObject, "Canvas");
        if (canvas != nullptr && setSortingOrder != nullptr)
        {
            int sortingOrder = behind
                ? kExtendedCanvasOrderBehindSystemMenu
                : normalOrders[index];
            void* parameters[] = { &sortingOrder };
            Invoke(setSortingOrder, canvas, parameters);
        }
        auto* raycaster = GetSceneComponentByClass(
            canvasObject,
            "UnityEngine.UI",
            "GraphicRaycaster");
        if (raycaster != nullptr)
        {
            ExtendedMode_SetComponentEnabled(raycaster, !behind);
        }
    }
}

void ExtendedMode_RecoverFromInterruptedExit()
{
    if (g_extendedModeState.load() != ExtendedMode_Exiting)
    {
        return;
    }
    int previousState = g_extendedExitPreviousState.exchange(
        ExtendedMode_Inactive);
    if (previousState != ExtendedMode_Active &&
        previousState != ExtendedMode_PreparingGameplay &&
        previousState != ExtendedMode_EnteringGameplay &&
        previousState != ExtendedMode_Gameplay)
    {
        previousState = ExtendedMode_Gameplay;
    }
    g_extendedModeState = previousState;
    g_extendedSuppressPlayerUpdate = false;
    g_extendedGameplayActive =
        previousState == ExtendedMode_Gameplay;
    g_extendedOriginalSystemMenuOpen = false;
    g_extendedOriginalSystemMenuVisible = false;
    ExtendedMode_SetCustomUiBehindSystemMenu(false);
    if (previousState == ExtendedMode_Gameplay)
    {
        ExtendedMode_ForceGameplayCursor();
    }
    else
    {
        ExtendedMode_ForceCursor();
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Extended mode cancelled the interrupted Title transition and restored state %d.",
        previousState);
}

void ExtendedMode_OpenOriginalSystemMenu()
{
    auto* systemMenuClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.SystemMenu",
        "SystemMenuView");
    auto* open = FindMethod(systemMenuClass, "Open", 0);
    if (open == nullptr)
    {
        LogInfo(LogCategory_Action, L"Action", L"Extended mode could not resolve SystemMenuView.Open.");
        return;
    }
    ExtendedMode_SetCustomUiBehindSystemMenu(true);
    g_extendedOriginalSystemMenuOpen = true;
    Invoke(open, nullptr, nullptr);
    g_extendedOriginalSystemMenuVisible = true;
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Action, L"Action", L"Extended mode opened the original system menu for Esc.");
}

void ExtendedMode_ResetLegAnimationState()
{
    auto* playerClassAccessor = g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
    if (playerClassAccessor == nullptr)
    {
        return;
    }

    // The equipment page drives the same avatar through a presentation pose.
    // Reinitialise the toe/heel and knee solvers when returning to gameplay so
    // their smoothed offsets cannot survive with values computed while the
    // presentation rotated or scaled the character.
    auto* kneeClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerKneeAdjuster");
    auto* knee = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerClassAccessor) + 0x198);
    auto* kneeInitialize = FindMethod(kneeClass, "Initialize", 1);
    if (knee != nullptr && kneeInitialize != nullptr)
    {
        void* parameters[] = { playerClassAccessor };
        Invoke(kneeInitialize, knee, parameters);
    }

    auto* toeClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerToeHeelController");
    auto* toe = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerClassAccessor) + 0x108);
    auto* toeInitialize = FindMethod(toeClass, "Initialize", 1);
    if (toe != nullptr && toeInitialize != nullptr)
    {
        void* parameters[] = { playerClassAccessor };
        Invoke(toeInitialize, toe, parameters);
    }
    LogInfo(LogCategory_Action, L"Action", L"Reset the original knee and toe/heel animation solvers.");
}

void ExtendedMode_TickMainThread(Il2CppObject* playerInstance)
{
    const int unityFrame = ExtendedMode_GetUnityFrameCount();
    if (unityFrame >= 0 &&
        unityFrame == g_extendedLastMainThreadTickFrame)
    {
        return;
    }
    if (unityFrame >= 0)
    {
        g_extendedLastMainThreadTickFrame = unityFrame;
    }

    DevMapCapture_Tick();
    int state = g_extendedModeState.load();
    if (state == ExtendedMode_Active ||
        state == ExtendedMode_Gameplay)
    {
        ExtendedMode_RefreshOriginalTutorialActive();
    }
    else
    {
        g_extendedOriginalTutorialActive = false;
    }
    if (state != ExtendedMode_Gameplay)
    {
        DildoInventory_ResetPending();
        g_extendedToggleSpecialCoatRequested = false;
        g_extendedOriginalSystemMenuVisible = false;
        g_extendedForwardKeyDown = false;
        g_extendedBackwardKeyDown = false;
        g_extendedLeftKeyDown = false;
        g_extendedRightKeyDown = false;
        g_extendedForwardKeyPulse = false;
        g_extendedBackwardKeyPulse = false;
        g_extendedLeftKeyPulse = false;
        g_extendedRightKeyPulse = false;
    }
    if (state != ExtendedMode_Active)
    {
        g_extendedManualPointerDown = false;
        g_extendedManualPointerConsumed = false;
        g_extendedManualPointerInventoryEligible = false;
    }
    if (state == ExtendedMode_Inactive)
    {
        ModGuideUi_Reset();
        g_extendedOriginalSystemMenuVisible = false;
        ExtendedMode_ForceCursor();
        return;
    }
    if (state == ExtendedMode_Exiting)
    {
        if (g_extendedAwaitingReturnConfirmation.load() &&
            TitleHooks_TryAutoConfirmReturnToTitle())
        {
            g_extendedAwaitingReturnConfirmation = false;
            LogInfo(LogCategory_Action, L"Action", L"Escape auto-confirmed the original return-to-title prompt.");
        }
        // TitleSceneView.Update is the first reliable object in the new scene.
        if (g_titleInstance != nullptr)
        {
            const bool returnToWarehouse =
                g_extendedReturnToWarehouseAfterExit;
            if (!returnToWarehouse)
            {
                ExtendedMode_SetOriginalTutorialSuppressed(
                    false,
                    L"stable Title after MOD exit");
            }
            ExtendedMode_ResetForTitle();
            g_extendedReturnToWarehouseAfterExit = false;
            if (returnToWarehouse)
            {
                g_extendedWarehouseReentryDelay = 30;
                LogInfo(LogCategory_Action, L"Action", L"Escape scheduled the title-page Escape button flow for warehouse return.");
                if (g_extendedPendingDeploymentStageType >= 0)
                {
                    LogInfo(LogCategory_Action, L"Action", 
                        L"Escape retained pending deployment to stage %d across the title reload.",
                        g_extendedPendingDeploymentStageType);
                }
            }
        }
        else if (g_extendedExitRequested.exchange(false))
        {
            ExtendedMode_ExitToTitle();
        }
        return;
    }
    ModGuideUi_Tick();
    if ((state == ExtendedMode_Active ||
         state == ExtendedMode_Gameplay) &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        ItemContextMenu_Tick();
    }
    if (state == ExtendedMode_Gameplay &&
        g_extendedReloadAfterMenuOpenFrames > 0)
    {
        if (g_extendedOpenSystemMenuRequested.exchange(false))
        {
            ExtendedMode_OpenOriginalSystemMenu();
        }
        --g_extendedReloadAfterMenuOpenFrames;
        if (g_extendedReloadAfterMenuOpenFrames == 0)
        {
            g_extendedExitRequested = true;
            LogInfo(LogCategory_Action, L"Action", L"Escape system menu is ready; requesting its clean Title transition.");
        }
    }
    if (g_extendedExitRequested.exchange(false) &&
        state >= ExtendedMode_LoadingGameplay)
    {
        ExtendedMode_ExitToTitle();
        return;
    }
    if (state == ExtendedMode_OpeningSavePanel)
    {
        ExtendedMode_HandleSavePanel();
        return;
    }
    if (state == ExtendedMode_LoadingGameplay ||
        state == ExtendedMode_PreparingGameplay)
    {
        if (!g_extendedTakeoverComplete)
        {
            ++g_extendedStateFrames;
            const int frames = g_extendedStateFrames.load();
            if (ExtendedMode_IsUnityObjectAlive(g_titleInstance))
            {
                if (frames % 120 == 0)
                {
                    LogInfo(LogCategory_Action, L"Action", L"Extended load is waiting for the old Title scene to unload.");
                }
                return;
            }
            g_titleInstance = nullptr;
            // Do not bind while the save panel is still tearing down the Title
            // scene. InGameManager.OnUpdate first fires after the gameplay
            // scene has installed its manager, and the additional stability
            // window prevents an old DontDestroyOnLoad player from being
            // adopted immediately before the new player replaces it.
            if (!g_extendedGameplayUpdateObserved.load())
            {
                if (frames % 120 == 0)
                {
                    LogInfo(LogCategory_Action, L"Action", L"Extended load is waiting for the gameplay manager update pump.");
                }
                return;
            }
            const bool ready =
                ExtendedMode_TryBindGameplay(playerInstance) &&
                ExtendedMode_IsGameplayManagerHealthy();
            if (ready)
            {
                if (g_extendedPlayerController != g_extendedBindingCandidate ||
                    g_extendedInGameManager !=
                        g_extendedBindingManagerCandidate)
                {
                    g_extendedBindingCandidate = g_extendedPlayerController;
                    g_extendedBindingManagerCandidate =
                        g_extendedInGameManager;
                    g_extendedBindingStableFrames = 1;
                    LogInfo(LogCategory_Action, L"Action", 
                        L"Extended gameplay binding changed; waiting for three stable frames (player=%p manager=%p).",
                        g_extendedBindingCandidate,
                        g_extendedBindingManagerCandidate);
                }
                else
                {
                    ++g_extendedBindingStableFrames;
                }
                if (frames % 120 == 0)
                {
                    LogInfo(LogCategory_Action, L"Action", 
                        L"Extended bind probe: player=%p ref=%p avatar=%p root=%p animator=%p inGame=%p ui=%p ready=%d stable=%d.",
                        g_extendedPlayerController,
                        g_extendedInGameReferencer,
                        g_extendedAvatarReferencer,
                        g_extendedAvatarRoot,
                        g_previewAnimator,
                        g_extendedInGameManager,
                        g_extendedInGameUiManager,
                        ready ? 1 : 0,
                        g_extendedBindingStableFrames);
                }
                if (g_extendedBindingStableFrames >= 60)
                {
                    if (g_extendedDefaultSavePending.load())
                    {
                        auto* saveManagerClass = FindClass(
                            "ExposureUnnoticed2.Scripts.InGame",
                            "SaveManager");
                        auto* isExist = FindMethod(
                            saveManagerClass,
                            "IsExistSaveData",
                            2);
                        int manualType = 3; // SaveRapper.SaveDataType.ManualSlot
                        int manualSlot = 0;
                        void* existParameters[] = {
                            &manualType,
                            &manualSlot,
                        };
                        if (isExist == nullptr)
                        {
                            g_extendedDefaultSavePending = false;
                            ExtendedMode_Fail(
                                L"SaveManager.IsExistSaveData became unavailable");
                            return;
                        }
                        auto* existsValue = isExist != nullptr
                            ? Invoke(isExist, nullptr, existParameters)
                            : nullptr;
                        const bool exists =
                            existsValue != nullptr &&
                            il2cpp_object_unbox != nullptr &&
                            *static_cast<bool*>(
                                il2cpp_object_unbox(existsValue));
                        if (exists)
                        {
                            g_extendedDefaultSavePending = false;
                            LogInfo(LogCategory_Action, L"Action", 
                                L"Original NewGame path created manual save slot 0.");
                        }
                        else if (g_extendedBindingStableFrames >= 3 &&
                                 g_extendedDefaultSaveWriteAttempts < 2)
                        {
                            ++g_extendedDefaultSaveWriteAttempts;
                            if (g_originalSaveManagerSave == nullptr ||
                                g_saveManagerSaveMethod == nullptr)
                            {
                                g_extendedDefaultSavePending = false;
                                ExtendedMode_Fail(
                                    L"cached SaveManager.Save target is unavailable");
                                return;
                            }
                            LogInfo(LogCategory_Action, L"Action", 
                                L"Calling cached original SaveManager.Save(type=%d, slot=%d, backup=0, method=%p, target=%p).",
                                manualType,
                                manualSlot,
                                g_saveManagerSaveMethod,
                                g_originalSaveManagerSave);
                            using SaveFn = void (*)(
                                int,
                                int,
                                bool,
                                const Il2CppMethod*);
                            reinterpret_cast<SaveFn>(
                                g_originalSaveManagerSave)(
                                    manualType,
                                    manualSlot,
                                    false,
                                    g_saveManagerSaveMethod);
                            existsValue = Invoke(
                                isExist,
                                nullptr,
                                existParameters);
                            if (existsValue != nullptr &&
                                il2cpp_object_unbox != nullptr &&
                                *static_cast<bool*>(
                                    il2cpp_object_unbox(existsValue)))
                            {
                                g_extendedDefaultSavePending = false;
                                LogInfo(LogCategory_Action, L"Action", 
                                    L"Original SaveManager saved initialized manual slot 0.");
                            }
                        }
                        if (g_extendedDefaultSavePending.load())
                        {
                            if (frames > 900)
                            {
                                ExtendedMode_Fail(
                                    L"default save initialization timed out");
                            }
                            return;
                        }
                    }
                    LogInfo(LogCategory_Action, L"Action", 
                        L"Extended gameplay bindings remained stable for 60 frames; taking over the new scene.");
                    ExtendedMode_TakeOverGameplay();
                }
            }
            else
            {
                g_extendedBindingCandidate = nullptr;
                g_extendedBindingManagerCandidate = nullptr;
                g_extendedBindingStableFrames = 0;
            }
        }
    }
    if (state == ExtendedMode_EnteringGameplay)
    {
        ++g_extendedStateFrames;
        if (g_extendedSceneCleanupFramesRemaining > 0)
        {
            return;
        }
        const bool gameplayBound =
            ExtendedMode_TryBindGameplay(playerInstance);
        if (!g_extendedTransitionInvoked)
        {
            ExtendedMode_StartSelectedStageTransition();
        }
        const bool targetStageReady =
            g_extendedTargetAlreadyActive ||
            ExtendedMode_GetLoadedStageType() == g_extendedTargetStageType.load();
        if (targetStageReady)
        {
            ++g_extendedSettledStageFrames;
            const bool managerHealthy =
                gameplayBound &&
                ExtendedMode_IsGameplayManagerHealthy();
            if (managerHealthy &&
                g_extendedActiveEventMethodManager !=
                    g_extendedSceneBindingEventManagerCandidate)
            {
                g_extendedSceneBindingEventManagerCandidate =
                    g_extendedActiveEventMethodManager;
                g_extendedSceneBindingStableFrames = 1;
                LogInfo(LogCategory_Action, L"Action", 
                    L"Scene binding candidate changed; waiting for a stable EventMethodManager (generation=%llu manager=%p).",
                    static_cast<unsigned long long>(
                        g_extendedSceneGeneration.load()),
                    g_extendedActiveEventMethodManager);
            }
            else if (managerHealthy)
            {
                ++g_extendedSceneBindingStableFrames;
            }
            else
            {
                g_extendedSceneBindingEventManagerCandidate = nullptr;
                g_extendedSceneBindingStableFrames = 0;
            }
            if (g_extendedStateFrames % 120 == 0)
            {
                ExtendedMode_LogGameplayBindingDiagnostic(
                    L"waiting for stable gameplay bindings");
            }
            if (g_extendedDirectWarehouseTransition)
            {
                if (g_extendedSettledStageFrames == 1)
                {
                    CheckpointSystem_Reset();
                }
                if (managerHealthy &&
                    g_extendedSettledStageFrames >= 30 &&
                    g_extendedSceneBindingStableFrames >= 30)
                {
                    LogInfo(LogCategory_Action, L"Action", 
                        L"Direct warehouse transition reached a stable stage 0 manager; rebuilding the warehouse runtime.");
                    g_extendedDirectWarehouseTransition = false;
                    g_extendedTakeoverComplete = false;
                    ExtendedMode_TakeOverGameplay();
                }
                return;
            }
            const int checkpointStageType =
                ExtendedMode_GetLoadedStageType() >= 0
                    ? ExtendedMode_GetLoadedStageType()
                    : g_extendedTargetStageType.load();
            CheckpointSystem_OnGameplayReady(checkpointStageType);
            if (!g_extendedSpawnPositionApplied &&
                g_extendedSettledStageFrames >= 3 &&
                (g_extendedSettledStageFrames == 3 ||
                 g_extendedSettledStageFrames % 30 == 0))
            {
                ExtendedMode_ApplyLoadedStageSpawn();
            }
            if (!g_extendedTimeApplied &&
                g_extendedSpawnPositionApplied &&
                g_extendedSettledStageFrames >= 10)
            {
                ExtendedMode_ApplySelectedTime();
            }
            const bool relaxedTransitionReady =
                !managerHealthy &&
                gameplayBound &&
                g_extendedSpawnPositionApplied &&
                g_extendedTimeApplied &&
                g_extendedSettledStageFrames >= 180 &&
                g_extendedInGameUiManager != nullptr &&
                !ExtendedMode_IsOriginalSystemMenuActive();
            if (relaxedTransitionReady)
            {
                LogInfo(
                    LogCategory_Action,
                    L"Action",
                    L"Escape completed the loaded stage with relaxed manager "
                    L"readiness after %d settled frames: stage=%d "
                    L"updateFrames=%d event=%p.",
                    g_extendedSettledStageFrames,
                    checkpointStageType,
                    g_extendedEventManagerUpdateFrames,
                    g_extendedActiveEventMethodManager);
                ExtendedMode_FinalizeGameplay();
                return;
            }
            if (managerHealthy &&
                g_extendedSceneBindingStableFrames >= 30 &&
                g_extendedTimeApplied &&
                g_extendedSettledStageFrames >= 30)
            {
                ExtendedMode_FinalizeGameplay();
            }
        }
        else
        {
            g_extendedSettledStageFrames = 0;
        }
        if (g_extendedStateFrames.load() > 1800)
        {
            LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka stage transition timed out.");
            ExtendedMode_ExitToTitle();
        }
        return;
    }
    if (state == ExtendedMode_Gameplay)
    {
        // Drive the dialogue from the same gameplay update pump as the proven
        // E pose menu and Tab inventory. PlayerController/InGameManager-specific
        // hooks can stop being scheduled while a custom modal owns the player.
        NpcDialogue_Tick();
        if (NpcDialogue_IsActive())
        {
            return;
        }
        static bool loggedInputSurfaces = false;
        if (!loggedInputSurfaces)
        {
            loggedInputSurfaces = true;
            ExtendedMode_LogClassSurfaceOnce(
                "ExposureUnnoticed2.Scripts.Base",
                "InputManager");
            ExtendedMode_LogClassSurfaceOnce(
                "ExposureUnnoticed2.ObjectUI.InventoryPanel",
                "InventoryPanelView");
            ExtendedMode_LogClassSurfaceOnce(
                "ExposureUnnoticed2.ObjectUI.InventoryPanel",
                "InventoryPanel");
            ExtendedMode_LogClassSurfaceOnce(
                "ExposureUnnoticed2.ObjectUI.InGameMenu",
                "InGameMenuView");
        }
        if (g_extendedOriginalSystemMenuVisible.load() ||
            CheckpointSystem_IsCoatToggleLocked())
        {
            g_extendedToggleSpecialCoatRequested = false;
        }
        if (g_extendedToggleSpecialCoatRequested.exchange(false))
        {
            ExtendedMode_SetSpecialCoatMode(
                !g_extendedSpecialCoatMode.load());
        }
        // Gameplay must never inherit the suppression used by the preview
        // room. Keep this invariant here as well as in FinalizeGameplay so a
        // late transition hook cannot leave movement permanently disabled.
        if (g_extendedSuppressPlayerUpdate.exchange(false))
        {
            LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka cleared stale player-update suppression.");
        }
        static unsigned int gameplayHousekeepingCounter = 0;
        static bool gameplaySystemMenuWasActive = false;
        static bool gameplayInteractionMenuWasActive = false;
        static bool gameplayHandcuffsPanelWasActive = false;
        const bool gameplaySystemMenuActive =
            ExtendedMode_IsOriginalSystemMenuActive();
        const bool gameplayInteractionMenuActive =
            ExtendedMode_IsOriginalInteractionMenuActive();
        ExtendedMode_RefreshOriginalHandcuffsPanelActive();
        const bool gameplayHandcuffsPanelActive =
            ExtendedMode_IsOriginalHandcuffsPanelActive();
        g_extendedOriginalSystemMenuVisible = gameplaySystemMenuActive;
        g_extendedOriginalInteractionMenuActive =
            gameplayInteractionMenuActive;
        if (!gameplaySystemMenuActive &&
            g_extendedEscapeDismissRequested.exchange(false))
        {
            g_extendedOpenSystemMenuRequested = false;
            g_extendedToggleGameplayInventoryRequested = false;
            g_gameplayWorldMapToggleRequested = false;
            if (ModGuideUi_IsOpen())
            {
                ModGuideUi_RequestClose();
                LogInfo(LogCategory_Action, L"Action", L"Escape dismissed the independent MOD guide.");
            }
            else if (CheckpointSystem_IsMapEditorUiOpen())
            {
                CheckpointSystem_CloseMapEditorPanel(true);
                LogInfo(LogCategory_Action, L"Action", L"Escape dismissed the map-point editor panel.");
            }
            else if (g_gameplayInventoryOpen.load())
            {
                if (CheckpointSystem_IsRewardInventoryOpen())
                {
                    CheckpointSystem_RequestRewardInventoryClose();
                }
                else
                {
                    GameplayInventory_Close();
                }
                LogInfo(LogCategory_Action, L"Action", L"Escape dismissed the gameplay inventory.");
            }
            else if (g_gameplayWorldMapOpen.load())
            {
                GameplayWorldMap_SetOpen(false);
                LogInfo(LogCategory_Action, L"Action", L"Escape dismissed the world map.");
            }
            else
            {
                LogInfo(LogCategory_Action, L"Action", L"Escape dismiss request was stale; no modal UI remained.");
            }
        }
        ExtendedMode_TickPoseMenu(
            gameplaySystemMenuActive,
            gameplayInteractionMenuActive);
        if (gameplayInteractionMenuActive)
        {
            if (g_extendedOriginalInteractionMenuClickRequested.exchange(false))
            {
                ExtendedMode_ActivateHoveredOriginalInteractionItem();
            }
        }
        else
        {
            g_extendedOriginalInteractionMenuClickRequested = false;
        }
        if (gameplayInteractionMenuActive != gameplayInteractionMenuWasActive)
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Escape original interaction menu %s; cursor handed back to %s.",
                gameplayInteractionMenuActive ? L"opened" : L"closed",
                gameplayInteractionMenuActive ? L"the original menu" : L"gameplay");
            ExtendedMode_SetGameplayHudRaycastsEnabled(
                !gameplayInteractionMenuActive &&
                !gameplayHandcuffsPanelActive);
            if (gameplayInteractionMenuActive)
            {
                ExtendedMode_ForceCursor();
            }
        }
        if (gameplayHandcuffsPanelActive !=
            gameplayHandcuffsPanelWasActive)
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"Escape handcuffs panel %s; cursor handed back to %s.",
                gameplayHandcuffsPanelActive ? L"opened" : L"closed",
                gameplayHandcuffsPanelActive
                    ? L"the original panel"
                    : L"gameplay");
            ExtendedMode_SetGameplayHudRaycastsEnabled(
                !gameplayInteractionMenuActive &&
                !gameplayHandcuffsPanelActive);
            if (gameplayHandcuffsPanelActive)
            {
                ExtendedMode_ForceCursor();
            }
            else if (!gameplaySystemMenuActive &&
                     !gameplayInteractionMenuActive)
            {
                ExtendedMode_ForceGameplayCursor();
            }
        }
        gameplayHandcuffsPanelWasActive =
            gameplayHandcuffsPanelActive;
        if (gameplayHandcuffsPanelActive)
        {
            if (g_extendedOriginalHandcuffsPanelClickRequested.exchange(
                    false))
            {
                ExtendedMode_ActivateHoveredOriginalHandcuffsItem();
            }
        }
        else if (DildoInventory_IsPanelActive())
        {
            if (g_extendedOriginalHandcuffsPanelClickRequested.exchange(
                    false))
            {
                DildoInventory_ActivateHoveredChoice();
            }
        }
        else
        {
            g_extendedOriginalHandcuffsPanelClickRequested = false;
        }
        if (gameplayInteractionMenuActive)
        {
            g_extendedOpenSystemMenuRequested = false;
        }
        gameplayInteractionMenuWasActive = gameplayInteractionMenuActive;
        if (gameplaySystemMenuActive ||
            gameplayHandcuffsPanelActive ||
            DildoInventory_IsPanelActive())
        {
            g_extendedToggleGameplayInventoryRequested = false;
        }
        else
        {
            GameplayInventory_TickMainThread();
        }
        if (gameplaySystemMenuActive != gameplaySystemMenuWasActive)
        {
            ExtendedMode_SetCustomUiBehindSystemMenu(
                gameplaySystemMenuActive);
        }
        if (gameplaySystemMenuActive ||
            gameplayHandcuffsPanelActive ||
            DildoInventory_IsPanelActive())
        {
            g_extendedForwardKeyDown = false;
            g_extendedBackwardKeyDown = false;
            g_extendedLeftKeyDown = false;
            g_extendedRightKeyDown = false;
            g_extendedForwardKeyPulse = false;
            g_extendedBackwardKeyPulse = false;
            g_extendedLeftKeyPulse = false;
            g_extendedRightKeyPulse = false;
        }
        if (gameplaySystemMenuWasActive && !gameplaySystemMenuActive)
        {
            g_extendedOriginalSystemMenuOpen = false;
            ExtendedMode_ForceGameplayCursor();
            LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka closed the original system menu; gameplay resumed.");
        }
        else if (!gameplaySystemMenuWasActive && gameplaySystemMenuActive)
        {
            ExtendedMode_ForceCursor();
            LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka opened the original system menu; gameplay paused.");
        }
        gameplaySystemMenuWasActive = gameplaySystemMenuActive;
        if (g_extendedOpenSystemMenuRequested.exchange(false))
        {
            if (NpcDialogue_ShouldBlockSystemMenuOpen())
            {
                LogInfo(LogCategory_Action, L"Action", L"NPC dialogue blocked a leaked system-menu request after Escape.");
            }
            else
            {
                ExtendedMode_OpenOriginalSystemMenu();
            }
        }
        g_extendedToggleActionListRequested = false;
        g_extendedToggleEquipmentPreviewRequested = false;
        g_extendedToggleEquipmentCatalogBatchRequested = false;
        if (g_extendedExitRequested.exchange(false))
        {
            ExtendedMode_ExitToTitle();
            return;
        }
        ExtendedMode_TryBindGameplay(playerInstance);
        GameplayInput_TickEyeMaskModelAttachment();
        GameplayInput_TickVibratorModelAttachment();
        DildoInventory_Tick();
        GameplayStatusHud_Update();
        GameplayActionPursuit_Tick();
        GameplayMinimap_Update();
        static bool underpassDebugKeyWasDown = false;
        const bool underpassDebugKeyDown =
            (GetAsyncKeyState(VK_F9) & 0x8000) != 0;
        if (underpassDebugKeyDown && !underpassDebugKeyWasDown)
        {
            ExtendedMode_DebugTeleportToStationUnderpass();
        }
        underpassDebugKeyWasDown = underpassDebugKeyDown;
        if (!gameplaySystemMenuActive &&
            !gameplayHandcuffsPanelActive &&
            !DildoInventory_IsPanelActive())
        {
            CheckpointSystem_Tick();
        }
        CheckpointCompass_Update();
        ++gameplayHousekeepingCounter;
        if (gameplayHousekeepingCounter % 15 == 0)
        {
            ExtendedMode_PrepareSkinnedRenderers();
        }
        if (gameplayHousekeepingCounter == 60)
        {
            ExtendedMode_LogTransformPosition(
                L"player root after settling",
                g_extendedPlayerGameObject);
            ExtendedMode_LogTransformPosition(
                L"avatar root after settling",
                g_extendedAvatarRoot);
            if (g_extendedCamera != nullptr)
            {
                ExtendedMode_LogTransformPosition(
                    L"gameplay camera after settling",
                    GetComponentGameObject(g_extendedCamera));
            }
            ExtendedMode_LogAvatarRendererState();
            int remainingBones = 512;
            LogSkeletonTransforms(
                GetWorldTransform(g_extendedAvatarRoot),
                0,
                remainingBones);
        }
        if (gameplayHousekeepingCounter % 3 == 0)
        {
            ExtendedMode_UpdateGameplayCoordinates();
        }
        if (gameplayHousekeepingCounter % 15 == 0)
        {
            ExtendedMode_HideGameplayUi();
        }
        if (gameplayHousekeepingCounter <= 240 &&
            gameplayHousekeepingCounter % 15 == 0)
        {
            InteractionFilter_ApplySceneWhitelist();
        }
        if ((g_extendedOriginalTutorialActive.load() ||
             ModGuideUi_IsOpen()) &&
            !gameplaySystemMenuActive)
        {
            ExtendedMode_ForceCursor();
        }
        else if (g_extendedPoseMenuVisible.load() &&
            !gameplaySystemMenuActive)
        {
            ExtendedMode_ForceCursor();
        }
        else if (gameplayInteractionMenuActive &&
            !gameplaySystemMenuActive)
        {
            if (gameplayHousekeepingCounter % 10 == 0)
            {
                ExtendedMode_ForceCursor();
            }
        }
        else if ((gameplayHandcuffsPanelActive ||
                  DildoInventory_IsPanelActive()) &&
                 !gameplaySystemMenuActive)
        {
            if (gameplayHousekeepingCounter % 10 == 0)
            {
                ExtendedMode_ForceCursor();
            }
        }
        else if ((g_gameplayInventoryOpen.load() ||
                  CheckpointSystem_IsTaskUiOpen() ||
                  CheckpointSystem_IsMapEditorUiOpen()) &&
            !gameplaySystemMenuActive)
        {
            ExtendedMode_ForceCursor();
        }
        else if (NpcDialogue_IsActive() &&
                 !gameplaySystemMenuActive)
        {
            ExtendedMode_ForceCursor();
        }
        else if (!gameplaySystemMenuActive &&
                 gameplayHousekeepingCounter % 30 == 0)
        {
            ExtendedMode_ForceGameplayCursor();
        }
        return;
    }
    state = g_extendedModeState.load();
    if (state == ExtendedMode_Active)
    {
        if (g_extendedWarehouseReturnInProgress.load() &&
            g_extendedPendingDeploymentStageType < 0 &&
            g_extendedWarehouseCoverHideDelayFrames > 0 &&
            --g_extendedWarehouseCoverHideDelayFrames == 0)
        {
            ExtendedMode_HideTransferLoadingOverlay();
            g_extendedWarehouseReturnInProgress = false;
            LogInfo(LogCategory_Action, L"Action", L"Warehouse return completed; loading cover removed after the rebuilt UI was rendered.");
        }
        static unsigned int actionTickCounter = 0;
        static unsigned int housekeepingCounter = 0;
        static bool originalSystemMenuWasActive = false;
        const bool originalSystemMenuActive =
            ExtendedMode_IsOriginalSystemMenuActive();
        if (originalSystemMenuActive != originalSystemMenuWasActive)
        {
            ExtendedMode_SetCustomUiBehindSystemMenu(
                originalSystemMenuActive);
        }
        if (originalSystemMenuWasActive && !originalSystemMenuActive)
        {
            g_extendedOriginalSystemMenuOpen = false;
            // The original menu owns the close path. Restore our cursor as soon
            // as its active panel disappears, including when Resume was clicked.
            ExtendedMode_ForceCursor();
            LogInfo(LogCategory_Action, L"Action", L"Extended mode detected the original system menu closing.");
        }
        if (g_extendedOpenSystemMenuRequested.exchange(false))
        {
            if (g_extendedSceneSelectorOpen)
            {
                ExtendedMode_CloseSceneSelector();
            }
            else if (originalSystemMenuActive)
            {
                // SystemMenuView.LateUpdate handles the same Esc input and will
                // close the active panel. Calling Open here creates extra layers.
                LogInfo(LogCategory_Action, L"Action", L"Extended mode left the active system menu to its Esc close path.");
            }
            else if (originalSystemMenuWasActive)
            {
                // The polling thread can queue Esc just before LateUpdate closes
                // the menu. By the time this request is processed the panel is
                // already inactive; reopening it here is the stale-second-press bug.
                LogInfo(LogCategory_Action, L"Action", L"Extended mode ignored a stale system-menu reopen request.");
            }
            else
            {
                ExtendedMode_OpenOriginalSystemMenu();
            }
        }
        originalSystemMenuWasActive = originalSystemMenuActive;
        InventoryUi_TickStashWheel();
        ExtendedMode_HandleManualPointerPress();
        ExhibitionUi_Tick();
        if (g_extendedPendingDeploymentStageType >= 0 &&
            g_extendedPendingDeploymentDelayFrames > 0 &&
            --g_extendedPendingDeploymentDelayFrames == 0)
        {
            g_extendedTargetStageType =
                g_extendedPendingDeploymentStageType;
            g_extendedTargetDaytime =
                g_extendedPendingDeploymentDaytime;
            g_extendedPendingDeploymentStageType = -1;
            g_extendedEnterGameRequested = true;
            LogInfo(LogCategory_Action, L"Action", L"Escape started deployment from the freshly reloaded warehouse.");
        }
        if (g_extendedOpenSceneSelectorRequested.exchange(false))
        {
            ExtendedMode_OpenSceneSelector();
        }
        if (g_extendedEnterGameRequested.exchange(false))
        {
            ExtendedMode_BeginSelectedGameplay();
            return;
        }
        if (g_extendedSceneSelectorOpen)
        {
            if (++housekeepingCounter % 10 == 0)
            {
                ExtendedMode_ForceCursor();
            }
            return;
        }
        if (g_extendedToggleEquipmentPreviewRequested.exchange(false))
        {
            EquipmentPreviewUi_Toggle();
        }
        ExtendedMode_UpdateActionRequests();
        InventoryUi_UpdateDynamicLayout();
        EquipmentPreviewUi_TickDildoAutoCapture();
        EquipmentPreviewUi_TickMainThread();
        InventoryUi_UpdateDrag();
        if (++actionTickCounter % 15 == 0)
        {
            ExtendedMode_EnforceActionLayers();
        }
        ExtendedMode_UpdateModelDrag();
        ++housekeepingCounter;
        ++g_extendedLegDiagnosticFrame;
        if (g_extendedLegDiagnosticFrame == 1 ||
            g_extendedLegDiagnosticFrame == 30 ||
            g_extendedLegDiagnosticFrame == 120)
        {
            wchar_t stage[48]{};
            swprintf_s(
                stage,
                L"active-frame-%u",
                g_extendedLegDiagnosticFrame);
            LogEscapeFromManakaLegState(stage);
        }
        if (g_extendedReframeCountdown > 0 &&
            --g_extendedReframeCountdown == 0)
        {
            ExtendedMode_FrameModel();
        }
        else if (housekeepingCounter % 30 == 0)
        {
            ExtendedMode_FrameModel();
        }
        if (g_extendedOutfitRefreshStage < 3 &&
            (g_extendedLegDiagnosticFrame == 30 ||
             g_extendedLegDiagnosticFrame == 90 ||
             g_extendedLegDiagnosticFrame == 180))
        {
            ++g_extendedOutfitRefreshStage;
            InventoryUi_RefreshCharacterModel();
            ExtendedMode_ApplyCoatlessLogic(true);
            LogInfo(LogCategory_Action, L"Action", 
                L"Reapplied gameplay outfit after scene stabilization (%d/3).",
                g_extendedOutfitRefreshStage);
        }
        if (housekeepingCounter % 30 == 0)
        {
            ExtendedMode_UpdateFutanariVisibility();
            ExtendedMode_ApplyCoatlessLogic(false);
        }
        if (housekeepingCounter % 10 == 0)
        {
            ExtendedMode_ForceCursor();
        }
        if (housekeepingCounter % 180 == 0)
        {
            ExtendedMode_SilenceGameplayAudio();
        }
    }
}

void PlayerActionUpdateHook(
    Il2CppObject* instance,
    Il2CppObject* inputData,
    const Il2CppMethod* method)
{
    int requestedAction = -1;
    bool blockedByStrongVibration = false;
    if (inputData != nullptr &&
        g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        requestedAction =
            g_extendedPoseMenuRequestedAction.exchange(-1);
        if (requestedAction >= 0)
        {
            auto* playerState = ExtendedMode_GetPlayerStateModel();
            blockedByStrongVibration =
                playerState != nullptr &&
                (*reinterpret_cast<bool*>(
                    reinterpret_cast<char*>(playerState) + 0x120) ||
                 GameplayInput_GetOriginalVibrationMode() == 2);
            LogInfo(
                LogCategory_Action,
                L"Action",
                L"E-menu action request=%d strongVibration=%d route=%s.",
                requestedAction,
                blockedByStrongVibration ? 1 : 0,
                blockedByStrongVibration
                    ? L"mod-notification"
                    : L"direct-change-action");
            if (blockedByStrongVibration)
            {
                GameplayStatusHud_ShowNotification(
                    "振动器强档时无法使用动作",
                    1800);
                requestedAction = -1;
            }
        }
    }
    const bool justSwitchVibrator =
        inputData != nullptr &&
        *reinterpret_cast<unsigned char*>(
            reinterpret_cast<char*>(inputData) + 0x3E) != 0;
    if (justSwitchVibrator)
    {
        LogDebug(
            LogCategory_Action,
            L"VibratorInput",
            L"PlayerAction.OnUpdate received JustSwitchVibrator action=%d guard=%d.",
            GameplayInput_GetCurrentActionType(),
            instance != nullptr &&
                *reinterpret_cast<bool*>(
                    reinterpret_cast<char*>(instance) + 0x38)
                ? 1
                : 0);
    }
    if (g_originalPlayerActionUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerActionUpdate)(
            instance,
            inputData,
            method);
    }
    if (requestedAction >= 0)
    {
        auto* changeAction = FindMethod(
            FindClass(
                "ExposureUnnoticed2.Object3D.Player.Scripts",
                "PlayerAction"),
            "ChangeAction",
            1);
        int action = requestedAction;
        void* parameters[] = { &action };
        Invoke(changeAction, instance, parameters);
        const int observedAction =
            GameplayInput_GetCurrentActionType();
        const bool accepted = observedAction == requestedAction;
        LogInfo(
            LogCategory_Action,
            L"Action",
            L"Direct E-menu action request=%d accepted=%d observed=%d.",
            requestedAction,
            accepted ? 1 : 0,
            observedAction);
        if (accepted)
        {
            GameplayActionPursuit_RequestFromEMenu(requestedAction);
        }
    }
}

void PlayerActionChangeActionHook(
    Il2CppObject* instance,
    int nextAction,
    const Il2CppMethod* method)
{
    const int previousAction = GameplayInput_GetCurrentActionType();
    LogInfo(
        LogCategory_Action,
        L"ActionChange",
        L"PlayerAction.ChangeAction requested old=%d new=%d guard=%d.",
        previousAction,
        nextAction,
        instance != nullptr &&
            *reinterpret_cast<bool*>(
                reinterpret_cast<char*>(instance) + 0x38)
            ? 1
            : 0);
    if (g_originalPlayerActionChangeAction != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            int,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerActionChangeAction)(
            instance,
            nextAction,
            method);
    }
    LogDebug(
        LogCategory_Action,
        L"ActionChangeApplied",
        L"PlayerAction.ChangeAction applied requested=%d observed=%d.",
        nextAction,
        GameplayInput_GetCurrentActionType());
}

void PlayerActionOnFrameEndHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    const bool guardBefore =
        instance != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(instance) + 0x38);
    if (g_originalPlayerActionOnFrameEnd != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerActionOnFrameEnd)(
            instance,
            method);
    }
    const bool guardAfter =
        instance != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(instance) + 0x38);
    if (guardBefore && !guardAfter)
    {
        LogDebug(
            LogCategory_Action,
            L"FrameGuardReleased",
            L"PlayerAction one-change-per-frame guard released.");
    }
}

void PlayerAnimationTransActionHook(
    Il2CppObject* instance,
    int actionType,
    const Il2CppMethod* method)
{
    const int actionIdBefore =
        GameplayAnimationDiagnostic_GetAnimatorActionId(instance);
    LogDebug(
        LogCategory_Animation,
        L"TransAction",
        L"PlayerAnimationManager.TransAction requested action=%d actionId=%d.",
        actionType,
        actionIdBefore);
    if (g_originalPlayerAnimationTransAction != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            int,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerAnimationTransAction)(
            instance,
            actionType,
            method);
    }
    LogTrace(
        LogCategory_Animation,
        L"TransActionApplied",
        L"PlayerAnimationManager.TransAction applied action=%d actionId=%d.",
        actionType,
        GameplayAnimationDiagnostic_GetAnimatorActionId(instance));
}

void PlayerAnimationSetMoveSpeedHook(
    Il2CppObject* instance,
    float speed,
    const Il2CppMethod* method)
{
    const float oldTarget =
        instance != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(instance) + 0x10)
            : 0.0f;
    const float oldBase =
        instance != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(instance) + 0x24)
            : 0.0f;
    if (g_originalPlayerAnimationSetMoveSpeed != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            float,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationSetMoveSpeed)(
                instance,
                speed,
                method);
    }
    const float newTarget =
        instance != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(instance) + 0x10)
            : 0.0f;
    const float newBase =
        instance != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(instance) + 0x24)
            : 0.0f;
    if (std::fabs(oldTarget - newTarget) > 0.0001f ||
        std::fabs(oldBase - newBase) > 0.0001f)
    {
        LogInfo(
            LogCategory_Action,
            L"MoveSpeedSet",
            L"PlayerAnimationManager.SetMoveSpeed requested=%.4f target %.4f->%.4f base %.4f->%.4f.",
            speed,
            oldTarget,
            newTarget,
            oldBase,
            newBase);
    }
}

void PlayerAnimationSetGamanBaibuHook(
    Il2CppObject* instance,
    bool enabled,
    const Il2CppMethod* method)
{
    const bool previous =
        instance != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(instance) + 0x2C);
    if (g_originalPlayerAnimationSetGamanBaibu != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            bool,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationSetGamanBaibu)(
                instance,
                enabled,
                method);
    }
    const bool observed =
        instance != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(instance) + 0x2C);
    if (previous != observed || previous != enabled)
    {
        LogInfo(
            LogCategory_Action,
            L"GamanBaibu",
            L"PlayerAnimationManager.SetGamanBaibu requested=%d previous=%d observed=%d.",
            enabled ? 1 : 0,
            previous ? 1 : 0,
            observed ? 1 : 0);
    }
}

void PlayerAnimationSetEcstasyHook(
    Il2CppObject* instance,
    bool enabled,
    const Il2CppMethod* method)
{
    LogInfo(
        LogCategory_Action,
        L"Ecstasy",
        L"PlayerAnimationManager.SetEcstasy requested=%d action=%d.",
        enabled ? 1 : 0,
        GameplayInput_GetCurrentActionType());
    if (g_originalPlayerAnimationSetEcstasy != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            bool,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationSetEcstasy)(
                instance,
                enabled,
                method);
    }
}

void PlayerAdultGoodsControllerUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    const bool previousOldGaman =
        instance != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(instance) + 0x20);
    if (g_originalPlayerAdultGoodsControllerUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAdultGoodsControllerUpdate)(
                instance,
                method);
    }
    const bool observedOldGaman =
        instance != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(instance) + 0x20);
    if (previousOldGaman != observedOldGaman)
    {
        LogDebug(
            LogCategory_Action,
            L"AdultGoodsGaman",
            L"PlayerAdultGoodsController gaman state changed %d->%d.",
            previousOldGaman ? 1 : 0,
            observedOldGaman ? 1 : 0);
    }
}

void PlayerAnimationCommonStartAnimHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    LogDebug(
        LogCategory_Animation,
        L"CommonStartAnim",
        L"PlayerAnimationReceiver.CommonStartAnim action=%d.",
        GameplayInput_GetCurrentActionType());
    if (g_originalPlayerAnimationCommonStartAnim != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationCommonStartAnim)(
                instance,
                method);
    }
}

void PlayerAnimationCommonOnEndAnimHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    const int actionBefore = GameplayInput_GetCurrentActionType();
    if (g_originalPlayerAnimationCommonOnEndAnim != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationCommonOnEndAnim)(
                instance,
                method);
    }
    LogInfo(
        LogCategory_Animation,
        L"CommonEndAnim",
        L"PlayerAnimationReceiver.CommonOnEndAnim action %d->%d.",
        actionBefore,
        GameplayInput_GetCurrentActionType());
}

void PlayerAnimationChangeVibratorHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    LogInfo(
        LogCategory_Action,
        L"SwitchVibratorEvent",
        L"PlayerAnimationReceiver.ChangeVibrator action=%d.",
        GameplayInput_GetCurrentActionType());
    if (g_originalPlayerAnimationChangeVibrator != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationChangeVibrator)(
                instance,
                method);
    }
}

void PlayerAnimationEndEcstasyHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    LogInfo(
        LogCategory_Action,
        L"EcstasyEnd",
        L"PlayerAnimationReceiver.OnAnimEndEcstasy action=%d.",
        GameplayInput_GetCurrentActionType());
    if (g_originalPlayerAnimationEndEcstasy != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationEndEcstasy)(
                instance,
                method);
    }
}

void PlayerControllerUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    static bool loggedEntry = false;
    if (!loggedEntry)
    {
        loggedEntry = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"PlayerController.OnUpdate hook entered (state=%d).",
            g_extendedModeState.load());
    }
    EquipmentStats_PrepareHandcuffUnlock(instance);
    if (ExtendedMode_IsRunning() &&
        (ExtendedMode_ShouldSuppressPlayerUpdate() ||
         ExtendedMode_ShouldPreserveModalPoseAnimation()))
    {
        EquipmentStats_ObserveHandcuffUnlock(instance);
        return;
    }
    if (g_originalPlayerUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerUpdate)(instance, method);
    }
    EquipmentStats_ObserveHandcuffUnlock(instance);
    if (g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_ForceCursor();
    }
}

void PlayerAnimationManagerUpdateHook(
    Il2CppObject* instance,
    Il2CppObject* inputData,
    const Il2CppMethod* method)
{
    if (ExtendedMode_ShouldSuppressPlayerUpdate() ||
        ExtendedMode_ShouldPreserveModalPoseAnimation())
    {
        return;
    }
    static unsigned int diagnosticFrame = 0;
    static int diagnosticBurstFrames = 0;
    static int lastActionType = -999999;
    static int lastAnimatorActionId = -999999;
    static int lastLogicalMode = -999999;
    static bool lastGamanBaibu = false;
    ++diagnosticFrame;
    const bool gamanBaibu =
        instance != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(instance) + 0x2C);
    const bool justSwitchVibrator =
        inputData != nullptr &&
        *reinterpret_cast<unsigned char*>(
            reinterpret_cast<char*>(inputData) + 0x3E) != 0;
    const int actionType = GameplayInput_GetCurrentActionType();
    const int animatorActionId =
        GameplayAnimationDiagnostic_GetAnimatorActionId(instance);
    const int logicalMode = g_gameplayVibratorLogicalMode.load();
    const bool diagnosticEdge =
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        (justSwitchVibrator ||
         actionType == 10024 ||
         animatorActionId == 10024 ||
         gamanBaibu != lastGamanBaibu ||
         actionType != lastActionType ||
         animatorActionId != lastAnimatorActionId ||
         logicalMode != lastLogicalMode);
    if (diagnosticEdge)
    {
        diagnosticBurstFrames = 2400;
    }
    const bool logFrame =
        diagnosticEdge ||
        (diagnosticBurstFrames > 0 &&
         diagnosticFrame % 30 == 0);
    if (logFrame)
    {
        GameplayAnimationDiagnostic_Log(
            L"manager-before-original",
            instance,
            inputData,
            true);
    }
    if (g_originalPlayerAnimationManagerUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerAnimationManagerUpdate)(
            instance,
            inputData,
            method);
    }
    if (logFrame)
    {
        GameplayAnimationDiagnostic_Log(
            L"manager-after-original",
            instance,
            inputData,
            true);
    }
    if (diagnosticBurstFrames > 0)
    {
        --diagnosticBurstFrames;
    }
    lastActionType = actionType;
    lastAnimatorActionId = animatorActionId;
    lastLogicalMode = logicalMode;
    lastGamanBaibu = gamanBaibu;
    ExtendedMode_ApplyCoatlessLogic(false);
}

void PlayerClothesActionUpdateHook(
    Il2CppObject* instance,
    Il2CppObject* inputData,
    const Il2CppMethod* method)
{
    const bool specialCoatMode =
        g_extendedSpecialCoatMode.load() &&
        g_extendedModeState.load() == ExtendedMode_Gameplay;
    const bool coatlessGameplay =
        ExtendedMode_IsRunning() &&
        !specialCoatMode &&
        ExtendedMode_IsCoatlessLogicActive();
    if (ExtendedMode_IsRunning() &&
        !specialCoatMode &&
        !coatlessGameplay)
    {
        return;
    }
    if (g_originalPlayerClothesActionUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerClothesActionUpdate)(
            instance,
            inputData,
            method);
    }
    if (coatlessGameplay)
    {
        // Let the shipped clothes-action state machine drive interaction and
        // action-arm transitions, then restore the naked gameplay invariants.
        ExtendedMode_ApplyCoatlessLogic(false);
    }
}

int PlayerClothesGetExposedBodyPartsHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    const bool specialCoatMode =
        g_extendedSpecialCoatMode.load() &&
        g_extendedModeState.load() == ExtendedMode_Gameplay;
    if (ExtendedMode_IsCoatlessLogicActive() && !specialCoatMode)
    {
        return 32; // ExposedBodyParts.All
    }
    if (g_originalPlayerClothesGetExposedBodyParts != nullptr)
    {
        using OriginalFn =
            int (*)(Il2CppObject*, const Il2CppMethod*);
        return reinterpret_cast<OriginalFn>(
            g_originalPlayerClothesGetExposedBodyParts)(
                instance,
                method);
    }
    return 0;
}

void PlayerHoldInHandHoldHook(
    Il2CppObject* instance,
    int clothesType,
    bool inInit,
    const Il2CppMethod* method)
{
    if ((ExtendedMode_IsCoatlessLogicActive() ||
         g_extendedSpecialCoatMode.load()) &&
        clothesType == 0)
    {
        if (!g_extendedCoatlessDropSuppressedLogged)
        {
            g_extendedCoatlessDropSuppressedLogged = true;
            LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka blocked holding or dropping the coat.");
        }
        return;
    }
    if (g_originalPlayerHoldInHandHold != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            int,
            bool,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerHoldInHandHold)(
            instance,
            clothesType,
            inInit,
            method);
    }
}

void PlayerAnimationDropClothesHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_IsCoatlessLogicActive())
    {
        return;
    }
    if (g_originalPlayerAnimationDropClothes != nullptr)
    {
        using OriginalFn =
            void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalPlayerAnimationDropClothes)(
            instance,
            method);
    }
}

void PlayerAnimationEndPickingCoatHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_IsCoatlessLogicActive())
    {
        return;
    }
    if (g_originalPlayerAnimationEndPickingCoat != nullptr)
    {
        using OriginalFn =
            void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerAnimationEndPickingCoat)(
                instance,
                method);
    }
}

void InputManagerUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    static bool loggedEntry = false;
    if (!loggedEntry)
    {
        loggedEntry = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"InputManager.Update hook entered (state=%d).",
            g_extendedModeState.load());
    }
    if (g_extendedModeState.load() == ExtendedMode_Active)
    {
        return;
    }
    if (g_originalInputManagerUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalInputManagerUpdate)(
            instance,
            method);
    }
}

void InputManagerUpdateProcessHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    static bool loggedEntry = false;
    if (!loggedEntry)
    {
        loggedEntry = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"InputManager.UpdateProcess hook entered (state=%d).",
            g_extendedModeState.load());
    }
    if (g_extendedModeState.load() == ExtendedMode_Active)
    {
        return;
    }
    if (g_originalInputManagerUpdateProcess != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalInputManagerUpdateProcess)(
            instance,
            method);
    }
}

Il2CppObject* InputResolverResolveHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    ++g_extendedInputResolverCallCount;
    Il2CppObject* inputData = nullptr;
    if (g_originalInputResolverResolve != nullptr)
    {
        using OriginalFn =
            Il2CppObject* (*)(Il2CppObject*, const Il2CppMethod*);
        inputData = reinterpret_cast<OriginalFn>(
            g_originalInputResolverResolve)(instance, method);
    }
    if (inputData != nullptr &&
        g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        GameplayInput_SuppressReservedNativeActions(inputData);
        GameplayInput_ApplyAdultQuickSlot(inputData);
        CheckpointSystem_ApplyRequiredTaskPose(inputData);

        // The Win32 key hook does not remove device state already sampled by
        // Unity's input backend. Clear only the movement vector so the native
        // player update can settle into its normal idle animation while a MOD
        // panel is open, without stopping player/NPC world updates.
        if (GameplayInput_IsCustomModalUiOpen() &&
            !g_extendedOriginalSystemMenuVisible.load())
        {
            float* movement = reinterpret_cast<float*>(
                reinterpret_cast<char*>(inputData) + 0x10);
            movement[0] = 0.0f;
            movement[1] = 0.0f;
            movement[2] = 0.0f;
        }
    }
    static bool loggedOriginalInput = false;
    if (inputData != nullptr &&
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        !loggedOriginalInput)
    {
        loggedOriginalInput = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"Escape from Manaka is using the original InputResolver and PlayerLocomotion path.");
    }
    return inputData;
}

bool GameplayInput_GetKeyHook(
    int keyCode,
    const Il2CppMethod* method)
{
    if (keyCode == 'N' &&
        CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        return false;
    }
    if (keyCode == VK_ESCAPE &&
        NpcDialogue_ShouldBlockSystemMenuOpen())
    {
        return false;
    }
    if (keyCode == VK_ESCAPE &&
        (g_extendedEscapeDismissConsumed.load() ||
         GetTickCount64() <
             g_extendedEscapeDismissBlockedUntil.load()))
    {
        return false;
    }
    if (g_gameplayWorldMapOpen.load() &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if ((keyCode == 'M' || keyCode == 'm') &&
        GameplayWorldMap_ShouldBlockNativeInput())
    {
        return false;
    }
    if (CheckpointSystem_IsMapEditorTextInputFocused())
    {
        if (g_originalUnityInputGetKey == nullptr)
        {
            return false;
        }
        using OriginalFn = bool (*)(int, const Il2CppMethod*);
        return reinterpret_cast<OriginalFn>(
            g_originalUnityInputGetKey)(keyCode, method);
    }
    if (CheckpointSystem_IsMapEditorUiOpen() &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if ((keyCode == 'T' || keyCode == 't') &&
        NpcDialogue_IsTalkPromptVisible())
    {
        NpcDialogue_RequestTalk();
        return false;
    }
    if (GameplayInput_IsCustomModalUiOpen() &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (g_originalUnityInputGetKey == nullptr)
    {
        return false;
    }
    using OriginalFn = bool (*)(int, const Il2CppMethod*);
    const bool down = reinterpret_cast<OriginalFn>(
        g_originalUnityInputGetKey)(keyCode, method);
    if (keyCode == 'Y' &&
        down &&
        CheckpointSystem_HasAcceptedTask() &&
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        bool expected = false;
        if (g_checkpointAbandonActionDown.compare_exchange_strong(
                expected,
                true))
        {
            g_checkpointAbandonTaskRequested = true;
            LogInfo(LogCategory_Action, L"Action", L"Checkpoint task abandon queued from Unity Input.GetKey(Y).");
        }
        return down;
    }
    return down;
}

bool GameplayInput_GetKeyDownHook(
    int keyCode,
    const Il2CppMethod* method)
{
    if (keyCode == VK_ESCAPE &&
        NpcDialogue_ShouldBlockSystemMenuOpen())
    {
        return false;
    }
    if (GameplayInput_ProcessEscapeDismissKey(
            keyCode,
            true,
            false,
            L"Unity Input.GetKeyDown"))
    {
        return false;
    }
    if (GameplayWorldMap_ProcessPhysicalKey(
            keyCode,
            true,
            L"Unity Input.GetKeyDown"))
    {
        return false;
    }
    if (CheckpointSystem_ProcessMapEditorPhysicalKey(
            keyCode,
            true,
            L"Unity Input.GetKeyDown"))
    {
        return false;
    }
    if (keyCode == 'N' &&
        CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        return false;
    }
    if (CheckpointSystem_IsMapEditorTextInputFocused())
    {
        if (g_originalUnityInputGetKeyDown == nullptr)
        {
            return false;
        }
        using OriginalFn = bool (*)(int, const Il2CppMethod*);
        return reinterpret_cast<OriginalFn>(
            g_originalUnityInputGetKeyDown)(keyCode, method);
    }
    if (CheckpointSystem_IsMapEditorUiOpen() &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (keyCode == 'T' || keyCode == 't')
    {
        NpcDialogue_ReserveTalkKey();
        if (NpcDialogue_HandleTalkKeyDown())
        {
            return false;
        }
    }
    if (keyCode == 'Y' &&
        CheckpointSystem_HasAcceptedTask() &&
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        bool expected = false;
        if (g_checkpointAbandonActionDown.compare_exchange_strong(
                expected,
                true))
        {
            g_checkpointAbandonTaskRequested = true;
            LogInfo(LogCategory_Action, L"Action", L"Checkpoint task abandon queued from Unity Input.GetKeyDown(Y).");
        }
        // Keep the original key state; this key also drives an optional
        // Escape checkpoint action.
    }
    if (GameplayInput_IsCustomModalUiOpen() &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (g_originalUnityInputGetKeyDown == nullptr)
    {
        return false;
    }
    using OriginalFn = bool (*)(int, const Il2CppMethod*);
    return reinterpret_cast<OriginalFn>(
        g_originalUnityInputGetKeyDown)(keyCode, method);
}

bool GameplayInput_GetKeyUpHook(
    int keyCode,
    const Il2CppMethod* method)
{
    if (keyCode == VK_ESCAPE &&
        NpcDialogue_ShouldBlockSystemMenuOpen())
    {
        return false;
    }
    if (GameplayInput_ProcessEscapeDismissKey(
            keyCode,
            false,
            true,
            L"Unity Input.GetKeyUp"))
    {
        return false;
    }
    GameplayWorldMap_ProcessPhysicalKey(
        keyCode,
        false,
        L"Unity Input.GetKeyUp");
    if ((keyCode == 'M' || keyCode == 'm') &&
        GameplayWorldMap_ShouldBlockNativeInput())
    {
        return false;
    }
    CheckpointSystem_ProcessMapEditorPhysicalKey(
        keyCode,
        false,
        L"Unity Input.GetKeyUp");
    if (keyCode == 'N' &&
        CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        return false;
    }
    if (CheckpointSystem_IsMapEditorTextInputFocused())
    {
        if (g_originalUnityInputGetKeyUp == nullptr)
        {
            return false;
        }
        using OriginalFn = bool (*)(int, const Il2CppMethod*);
        return reinterpret_cast<OriginalFn>(
            g_originalUnityInputGetKeyUp)(keyCode, method);
    }
    if (keyCode == 'F')
    {
        g_checkpointInteractActionDown = false;
        g_checkpointInteractionKeyConsumed = false;
    }
    else if (keyCode == 'Y')
    {
        g_checkpointAbandonActionDown = false;
    }
    if (GameplayInput_IsCustomModalUiOpen() &&
        !g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (g_originalUnityInputGetKeyUp == nullptr)
    {
        return false;
    }
    using OriginalFn = bool (*)(int, const Il2CppMethod*);
    const bool up = reinterpret_cast<OriginalFn>(
        g_originalUnityInputGetKeyUp)(keyCode, method);
    return up;
}

bool GameplayInput_InputManagerIsDownHook(
    int actionType,
    const Il2CppMethod* method)
{
    if (GameplayInput_ShouldBlockOriginalAction(actionType))
    {
        return false;
    }
    if (g_originalInputManagerIsDown == nullptr)
    {
        return false;
    }
    using OriginalFn =
        bool (*)(int, const Il2CppMethod*);
    return reinterpret_cast<OriginalFn>(
        g_originalInputManagerIsDown)(
            actionType,
            method);
}

bool GameplayInput_InputManagerIsDownFrameHook(
    int actionType,
    const Il2CppMethod* method)
{
    if (GameplayInput_ShouldBlockOriginalAction(actionType))
    {
        return false;
    }
    if (g_originalInputManagerIsDownFrame == nullptr)
    {
        return false;
    }
    using OriginalFn =
        bool (*)(int, const Il2CppMethod*);
    const bool down = reinterpret_cast<OriginalFn>(
        g_originalInputManagerIsDownFrame)(
            actionType,
            method);
    if (actionType == 1016 &&
        down &&
        !CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        GameplayInput_RequestConfiguredEyeMaskAction();
    }
    else if (actionType == 4 &&
             down &&
             EyeMaskVision_IsEyeMaskActive())
    {
        GameplayInput_RequestEyeMaskForceThirdPerson();
    }
    return down;
}

bool GameplayInput_InputManagerIsUpFrameHook(
    int actionType,
    const Il2CppMethod* method)
{
    if (GameplayInput_ShouldBlockOriginalAction(actionType))
    {
        return false;
    }
    if (g_originalInputManagerIsUpFrame == nullptr)
    {
        return false;
    }
    using OriginalFn =
        bool (*)(int, const Il2CppMethod*);
    const bool up = reinterpret_cast<OriginalFn>(
        g_originalInputManagerIsUpFrame)(
            actionType,
            method);
    return up;
}

Il2CppObject* GameplayInput_InputManagerGetInputActionHook(
    int actionType,
    const Il2CppMethod* method)
{
    Il2CppObject* action = nullptr;
    if (g_originalInputManagerGetInputAction != nullptr)
    {
        using OriginalFn =
            Il2CppObject* (*)(int, const Il2CppMethod*);
        action = reinterpret_cast<OriginalFn>(
            g_originalInputManagerGetInputAction)(
                actionType,
                method);
    }
    if (action == nullptr)
    {
        return nullptr;
    }
    auto* actionClass = FindClass(
        "UnityEngine.InputSystem",
        "InputAction");
    if (GameplayWorldMap_ShouldBlockNativeInput())
    {
        auto* disable = FindMethod(actionClass, "Disable", 0);
        if (disable != nullptr &&
            g_gameplayDisabledInputActions.insert(action).second)
        {
            Invoke(disable, action, nullptr);
            static bool loggedWorldMapActionBlock = false;
            if (!loggedWorldMapActionBlock)
            {
                loggedWorldMapActionBlock = true;
                LogInfo(LogCategory_Action, L"Action", 
                    L"World map blocked direct InputAction type=%d.",
                    actionType);
            }
        }
        return action;
    }

    if (actionType == 1016 &&
        CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        auto* disable = FindMethod(actionClass, "Disable", 0);
        if (disable != nullptr &&
            g_mapEditorBlockedEyeMaskInputAction != action)
        {
            GameplayInput_RestoreMapEditorEyeMaskInputAction();
            Invoke(disable, action, nullptr);
            g_mapEditorBlockedEyeMaskInputAction = action;
            LogInfo(LogCategory_Action, L"Action", 
                L"Map editor disabled the original EyeMask InputAction.");
        }
        return action;
    }

    if (actionType == 1016 &&
        !CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey())
    {
        static const Il2CppMethod* wasPressedThisFrame = nullptr;
        if (wasPressedThisFrame == nullptr)
        {
            wasPressedThisFrame = FindMethod(
                actionClass,
                "WasPressedThisFrame",
                0);
        }
        auto* pressed = Invoke(
            wasPressedThisFrame,
            action,
            nullptr);
        if (pressed != nullptr &&
            il2cpp_object_unbox != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(pressed)))
        {
            GameplayInput_RequestConfiguredEyeMaskAction();
        }
    }
    else if (actionType == 4 &&
             EyeMaskVision_IsEyeMaskActive())
    {
        static const Il2CppMethod* wasPressedThisFrame = nullptr;
        if (wasPressedThisFrame == nullptr)
        {
            wasPressedThisFrame = FindMethod(
                actionClass,
                "WasPressedThisFrame",
                0);
        }
        auto* pressed = Invoke(
            wasPressedThisFrame,
            action,
            nullptr);
        if (pressed != nullptr &&
            il2cpp_object_unbox != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(pressed)))
        {
            GameplayInput_RequestEyeMaskForceThirdPerson();
        }
    }

    static bool loggedActionTypes[512]{};
    static bool loggedActionNames[512]{};
    if (actionType >= 0 && actionType < 512 &&
        !loggedActionTypes[actionType])
    {
        loggedActionTypes[actionType] = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"Gameplay InputAction resolved type=%d.",
            actionType);
    }

    auto* getName = FindMethod(actionClass, "get_name", 0);
    if (actionType >= 0 && actionType < 512 &&
        !loggedActionNames[actionType] &&
        getName != nullptr)
    {
        auto* nameValue = Invoke(getName, action, nullptr);
        if (nameValue != nullptr)
        {
            loggedActionNames[actionType] = true;
            LogInfo(LogCategory_Action, L"Action", 
                L"Gameplay InputAction type=%d name=%s.",
                actionType,
                ReadIl2CppString(
                    static_cast<Il2CppString*>(nameValue)).c_str());
        }
    }

    // Custom modal input is gated by InputManager and the window/keyboard
    // hooks. Mutating the shipped InputAction enable state here can strand an
    // action after an action is no longer queried, so leave it untouched.
    return action;
}

void GameplayOriginalInventoryOpenHook(
    Il2CppObject* instance,
    int category,
    int selectedIndex,
    const Il2CppMethod* method)
{
    if (g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        static bool loggedBlockedOpen = false;
        if (!loggedBlockedOpen)
        {
            loggedBlockedOpen = true;
            LogInfo(LogCategory_Action, L"Action", 
                L"Escape gameplay blocked original InventoryPanelView.Open(%d, %d).",
                category,
                selectedIndex);
        }
        return;
    }
    if (g_originalInventoryPanelOpen != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            int,
            int,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalInventoryPanelOpen)(
                instance,
                category,
                selectedIndex,
                method);
    }
}

void InGameManagerUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    static bool loggedEntry = false;
    if (!loggedEntry)
    {
        loggedEntry = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"InGameManager.OnUpdate hook entered (state=%d).",
            g_extendedModeState.load());
    }
    auto* currentInGameManager = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Scripts.InGame",
        "InGameManager",
        "Instance");
    if (currentInGameManager != nullptr &&
        instance != nullptr &&
        currentInGameManager != instance &&
        g_extendedModeState.load() >= ExtendedMode_LoadingGameplay &&
        g_extendedModeState.load() <= ExtendedMode_Gameplay)
    {
        static Il2CppObject* loggedStaleInGameManager = nullptr;
        if (loggedStaleInGameManager != instance)
        {
            loggedStaleInGameManager = instance;
            LogInfo(LogCategory_Action, L"Action", 
                L"Rejected stale InGameManager update (instance=%p current=%p generation=%llu).",
                instance,
                currentInGameManager,
                static_cast<unsigned long long>(
                    g_extendedSceneGeneration.load()));
        }
        return;
    }
    if (currentInGameManager == nullptr &&
        g_extendedAwaitingFreshSceneManager.load() &&
        ExtendedMode_IsRunning())
    {
        return;
    }
    if (currentInGameManager != nullptr &&
        instance == currentInGameManager &&
        g_extendedModeState.load() >= ExtendedMode_LoadingGameplay &&
        g_extendedModeState.load() <= ExtendedMode_Gameplay)
    {
        g_extendedInGameManager = instance;
        g_extendedGameplayUpdateObserved = true;
    }
    if (ExtendedMode_IsRunning() &&
        ExtendedMode_ShouldSuppressPlayerUpdate() &&
        g_extendedModeState.load() != ExtendedMode_EnteringGameplay)
    {
        return;
    }
    if (g_originalInGameManagerUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalInGameManagerUpdate)(
            instance,
            method);
    }
    EyeMaskVision_ApplyAfterOriginalUpdate();
    if (g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_ForceCursor();
    }
}

void EventMethodManagerUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    static bool loggedEntry = false;
    if (!loggedEntry)
    {
        loggedEntry = true;
        LogInfo(LogCategory_Action, L"Action", 
            L"EventMethodManager.Update hook entered (state=%d).",
            g_extendedModeState.load());
    }
    const bool extendedRunning = ExtendedMode_IsRunning();
    if (extendedRunning && instance != nullptr)
    {
        auto* currentInGameManager = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Scripts.InGame",
            "InGameManager",
            "Instance");
        auto* currentEventManager = currentInGameManager != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(currentInGameManager) + 0x78)
            : nullptr;
        const bool identityConfirmed =
            currentInGameManager != nullptr &&
            currentEventManager == instance;
        if (!identityConfirmed &&
            (g_extendedAwaitingFreshSceneManager.load() ||
             currentEventManager != nullptr))
        {
            static Il2CppObject* loggedStaleEventManager = nullptr;
            if (loggedStaleEventManager != instance)
            {
                loggedStaleEventManager = instance;
                LogInfo(LogCategory_Action, L"Action", 
                    L"Rejected stale EventMethodManager update (instance=%p current=%p generation=%llu).",
                    instance,
                    currentEventManager,
                    static_cast<unsigned long long>(
                        g_extendedSceneGeneration.load()));
            }
            return;
        }
        if (identityConfirmed)
        {
            if (g_extendedActiveEventMethodManager != instance)
            {
                g_extendedActiveEventMethodManager = instance;
                g_extendedEventManagerUpdateFrames = 0;
                g_extendedSceneBindingEventManagerCandidate = nullptr;
                g_extendedSceneBindingStableFrames = 0;
                LogInfo(LogCategory_Action, L"Action", 
                    L"Accepted fresh EventMethodManager=%p at generation=%llu.",
                    instance,
                    static_cast<unsigned long long>(
                        g_extendedSceneGeneration.load()));
            }
            g_extendedAwaitingFreshSceneManager = false;
        }
        auto* referencer = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(instance) + 0x38);
        if (referencer != nullptr)
        {
            g_extendedInGameReferencer = referencer;
            auto* inGameManager = *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0x20);
            auto* player = *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0x30);
            if (inGameManager != nullptr)
            {
                g_extendedInGameManager = inGameManager;
            }
            if (player != nullptr)
            {
                g_extendedPlayerController = player;
            }
        }
        ExtendedMode_TickMainThread(g_extendedPlayerController);
        if (ExtendedMode_ShouldSuppressPlayerUpdate() &&
            g_extendedModeState.load() != ExtendedMode_EnteringGameplay)
        {
            return;
        }
    }
    const bool gameplayActive =
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        !ExtendedMode_ShouldSuppressPlayerUpdate() &&
        g_extendedPlayerController != nullptr;
    if (g_originalEventMethodManagerUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalEventMethodManagerUpdate)(
            instance,
            method);
        ++g_extendedEventManagerUpdateFrames;
    }
    ExtendedMode_ApplyCoatlessLogic(false);
    if (g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_ForceCursor();
    }
    static unsigned int gameplayVisibilityCounter = 0;
    if (gameplayActive &&
        ++gameplayVisibilityCounter % 15 == 0)
    {
        ExtendedMode_PrepareSkinnedRenderers();
    }
}

void BaseCameraLateUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    ExtendedMode_ProcessSafeSceneTransitionPump();
    if (ExtendedMode_ShouldSuppressGameplayCamera())
    {
        return;
    }
    if (g_originalBaseCameraLateUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalBaseCameraLateUpdate)(
            instance,
            method);
    }
    if (g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        GameplayStatusHud_HideResidualGauges();
    }
    if (g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_ForceCursor();
    }
}

void TpCameraLateUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_ShouldSuppressGameplayCamera())
    {
        return;
    }
    if (g_originalTpCameraLateUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalTpCameraLateUpdate)(
            instance,
            method);
    }
    if (g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        GameplayStatusHud_HideResidualGauges();
    }
    if (g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_ForceCursor();
    }
}

void InventoryCameraLateUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_ShouldSuppressGameplayCamera())
    {
        return;
    }
    if (g_originalInventoryCameraLateUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalInventoryCameraLateUpdate)(
            instance,
            method);
    }
    if (g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        GameplayStatusHud_HideResidualGauges();
    }
    if (g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_ForceCursor();
    }
}

void FpCameraLateUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_ShouldSuppressGameplayCamera())
    {
        return;
    }
    if (g_originalFpCameraLateUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalFpCameraLateUpdate)(
            instance,
            method);
    }
    if (g_extendedModeState.load() == ExtendedMode_Gameplay)
    {
        GameplayStatusHud_HideResidualGauges();
    }
    if (g_extendedPoseMenuVisible.load())
    {
        ExtendedMode_ForceCursor();
    }
}

void SaveManagerSaveHook(
    int type,
    int slot,
    bool isBackup,
    const Il2CppMethod* method)
{
    const bool allowDefaultSave =
        g_extendedDefaultSavePending.load() &&
        type == 3 && // SaveRapper.SaveDataType.ManualSlot
        slot == 0;
    if (ExtendedMode_ShouldBlockSave() && !allowDefaultSave)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Extended mode blocked original SaveManager.Save(type=%d, slot=%d, backup=%d).",
            type,
            slot,
            isBackup ? 1 : 0);
        return;
    }
    if (g_originalSaveManagerSave != nullptr)
    {
        using OriginalFn = void (*)(int, int, bool, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalSaveManagerSave)(
            type,
            slot,
            isBackup,
            method);
    }
}

void SystemMenuOpenHook(const Il2CppMethod* method)
{
    if (ModGuideUi_IsOpen() ||
        g_extendedOriginalTutorialActive.load())
    {
        g_extendedOpenSystemMenuRequested = false;
        g_extendedOriginalSystemMenuOpen = false;
        LogInfo(LogCategory_Action, L"Action", L"A modal guide blocked the original system menu from opening.");
        return;
    }
    if (NpcDialogue_ShouldBlockSystemMenuOpen())
    {
        g_extendedOpenSystemMenuRequested = false;
        g_extendedOriginalSystemMenuOpen = false;
        LogInfo(LogCategory_Action, L"Action", L"NPC dialogue blocked the original system menu from opening after Escape.");
        return;
    }
    if (GameplayInput_ShouldBlockSystemMenuAfterDismiss())
    {
        g_extendedOpenSystemMenuRequested = false;
        g_extendedOriginalSystemMenuOpen = false;
        g_extendedOriginalSystemMenuVisible = false;
        ExtendedMode_SetCustomUiBehindSystemMenu(false);
        LogInfo(LogCategory_Action, L"Action", 
            L"Modal Escape dismissal blocked the original SystemMenuView.Open call.");
        return;
    }
    g_extendedOriginalSystemMenuOpen = true;
    ExtendedMode_SetCustomUiBehindSystemMenu(true);
    if (ExtendedMode_IsGameplayActive())
    {
        LogInfo(LogCategory_Action, L"Action", L"Escape from Manaka forwarded Esc to the original system menu.");
        ExtendedMode_ForceCursor();
    }
    if (g_originalSystemMenuOpen != nullptr)
    {
        using OriginalFn = void (*)(const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalSystemMenuOpen)(method);
    }
}

void TutorialPanelOpenHook(
    int kind,
    const Il2CppMethod* method)
{
    if (kind != 0 &&
        g_modOriginalTutorialSuppressed.load())
    {
        static int lastSuppressedKind = -1;
        if (lastSuppressedKind != kind)
        {
            lastSuppressedKind = kind;
            LogInfo(LogCategory_Action, L"Action", 
                L"MOD session suppressed original tutorial kind=%d before its state-update path.",
                kind);
        }
        return;
    }
    if (kind == 0 &&
        ExtendedMode_IsRunning() &&
        (g_extendedModeState.load() == ExtendedMode_Active ||
         g_extendedModeState.load() == ExtendedMode_Gameplay))
    {
        ModGuideUi_RequestOpenFromTutorial();
        LogInfo(LogCategory_Action, L"Action", L"MOD mode redirected the original tutorial entry to the independent MOD guide.");
        return;
    }
    if (g_originalTutorialPanelOpen != nullptr)
    {
        using OriginalFn = void (*)(int, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalTutorialPanelOpen)(kind, method);
    }
}

void SystemMenuResumeHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    ExtendedMode_RecoverFromInterruptedExit();
    g_extendedOriginalSystemMenuOpen = false;
    g_extendedOriginalSystemMenuVisible = false;
    ExtendedMode_SetCustomUiBehindSystemMenu(false);
    if (g_originalSystemMenuResume != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalSystemMenuResume)(
            instance,
            method);
    }
}

void SystemMenuSaveAndGoToTitleHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (g_originalSystemMenuSaveAndGoToTitle != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalSystemMenuSaveAndGoToTitle)(
            instance,
            method);
    }
}

void MissionFailedGoToTitleHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_IsRunning())
    {
        auto* panelObject = GetComponentGameObject(instance);
        SetWorldGameObjectActive(panelObject, false);
        CheckpointSystem_RequestFailureReturn(
            L"MissionFailedPanelView.OnClickGoToTitle");
        return;
    }
    if (g_originalMissionFailedGoToTitle != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalMissionFailedGoToTitle)(
            instance,
            method);
    }
}

void MissionFailedLoadHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_IsRunning())
    {
        auto* panelObject = GetComponentGameObject(instance);
        SetWorldGameObjectActive(panelObject, false);
        CheckpointSystem_RequestFailureReturn(
            L"MissionFailedPanelView.OnClickLoad");
        return;
    }
    if (g_originalMissionFailedLoad != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalMissionFailedLoad)(
            instance,
            method);
    }
}

void MissionFailedOpenHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (ExtendedMode_IsRunning())
    {
        CheckpointSystem_OnGameFailureUiOpened();
    }
    if (ExtendedMode_IsRunning() &&
        CheckpointSystem_ShouldSuppressFailurePanelOpen())
    {
        LogInfo(LogCategory_Action, L"Action", L"Suppressed reopening MissionFailedPanel because the warehouse return is already queued.");
        return;
    }
    if (g_originalMissionFailedOpen != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalMissionFailedOpen)(
            instance,
            method);
    }
}
