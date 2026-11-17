// Native.cpp module: npc_dialogue.inc
// Included inside the shared implementation namespace by Native.cpp.

bool GameplayStatusHud_IsVibratorStrong();
void NpcDialogue_Close();
void NpcDialogue_UpdateEnhancedBars();

struct NpcDialogueQteZone
{
    float startDegrees = 0.0f;
    float endDegrees = 0.0f;
    bool active = false;
};

std::atomic<bool> g_npcDialoguePromptVisible{false};
std::atomic<bool> g_npcDialogueTalkRequested{false};
std::atomic<bool> g_npcDialogueCloseRequested{false};
std::atomic<bool> g_npcDialogueActive{false};
std::atomic<unsigned long long> g_npcDialogueTalkReservedUntil{0};
std::atomic<unsigned long long> g_npcDialogueSystemMenuBlockedUntil{0};
Il2CppObject* g_npcDialogueTarget = nullptr;
Il2CppObject* g_npcDialogueTargetGameObject = nullptr;
Il2CppObject* g_npcDialogueRoot = nullptr;
Il2CppObject* g_npcDialogueNameText = nullptr;
Il2CppObject* g_npcDialogueBodyText = nullptr;
Il2CppObject* g_npcDialogueChoice0Panel = nullptr;
Il2CppObject* g_npcDialogueChoice1Panel = nullptr;
Il2CppObject* g_npcDialogueChoice0Text = nullptr;
Il2CppObject* g_npcDialogueChoice1Text = nullptr;
Il2CppObject* g_npcDialogueHintText = nullptr;
int g_npcDialogueStage = 0;
bool g_npcDialogueMouseWasDown = false;
unsigned int g_npcDialogueHandledPressSerial = 0;
bool g_npcDialogueCameraSaved = false;
bool g_npcDialogueCameraDirectionReady = false;
bool g_npcDialogueCameraInitialized = false;
bool g_npcDialogueCameraTransitionComplete = false;
bool g_npcDialogueStabilitySampleReady = false;
int g_npcDialogueStillFrames = 0;
bool g_npcDialogueActorLockActive = false;
bool g_npcDialoguePlayerRotationSaved = false;
bool g_npcDialogueNpcRotationSaved = false;
bool g_npcDialoguePlayerMoveSpeedSaved = false;
bool g_npcDialogueNpcMoveSpeedSaved = false;
bool g_npcDialogueEnhancedMode = false;
bool g_npcDialogueCloseAfterTalk = false;
bool g_npcDialogueClimaxActive = false;
bool g_npcDialogueClimaxKeepAudioAfterClose = false;
bool g_npcDialogueAudioActive = false;
float g_npcDialoguePatience = 60.0f;
float g_npcDialogueExcitementSnapshot = 0.0f;
float g_npcDialogueExcitementDisplayed = 0.0f;
float g_npcDialogueExcitementAnimationStart = 0.0f;
unsigned long long g_npcDialogueExcitementAnimationStartedAt = 0;
unsigned long long g_npcDialogueClimaxAnimationStartedAt = 0;
Il2CppObject* g_npcDialogueClimaxAnimator = nullptr;
int g_npcDialogueReplyIndex = 0;
int g_npcDialogueTalkAnimationId = 0;
unsigned long long g_npcDialogueLastPatienceTick = 0;
unsigned long long g_npcDialogueNextChoicesAt = 0;
unsigned long long g_npcDialogueTalkAnimationEndsAt = 0;
SceneUiElement g_npcDialoguePatienceLabel{};
SceneUiElement g_npcDialogueExcitementLabel{};
SceneUiElement g_npcDialoguePatienceTrack{};
SceneUiElement g_npcDialoguePatienceFill{};
SceneUiElement g_npcDialogueExcitementTrack{};
SceneUiElement g_npcDialogueExcitementFill{};
SceneUiElement g_npcDialogueQteBackdrop{};
SceneUiElement g_npcDialogueQteFeedbackOverlay{};
SceneUiElement g_npcDialogueQteWheel{};
SceneUiElement g_npcDialogueQtePointer{};
SceneUiElement g_npcDialogueQtePointerShape{};
SceneUiElement g_npcDialogueQteButton{};
SceneUiElement g_npcDialogueQteButtonText{};
std::vector<SceneUiElement> g_npcDialogueQteSegments;
std::vector<SceneUiElement> g_npcDialogueQtePointerStrips;
Il2CppObject* g_npcDialogueQteWheelTexture = nullptr;
Il2CppObject* g_npcDialogueQtePointerTexture = nullptr;
NpcDialogueQteZone g_npcDialogueQteZones[3]{};
float g_npcDialogueQteAngle = 0.0f;
unsigned long long g_npcDialogueQteCycleStartedAt = 0;
unsigned long long g_npcDialogueQtePreviousPhase = 0;
unsigned long long g_npcDialogueQteRefillAt = 0;
unsigned long long g_npcDialogueQteCooldownUntil = 0;
bool g_npcDialogueQteInitialized = false;
unsigned long long g_npcDialogueQteZoneFadeStartedAt[3]{};
unsigned long long g_npcDialogueQteZoneFadeEndsAt[3]{};
unsigned long long g_npcDialogueQteMissFeedbackEndsAt = 0;
float g_npcDialogueSavedPlayerRotation[4]{};
float g_npcDialogueSavedNpcRotation[4]{};
float g_npcDialogueSavedPlayerBaseMoveSpeed = 0.0f;
float g_npcDialogueSavedNpcMoveSpeed = 0.0f;
float g_npcDialogueLastPlayerPosition[3]{};
float g_npcDialogueLastNpcPosition[3]{};
float g_npcDialogueTargetCameraPosition[3]{};
float g_npcDialogueTargetCameraYaw = 0.0f;
unsigned long long g_npcDialogueLastCameraTick = 0;
float g_npcDialogueSavedCameraPosition[3]{};
float g_npcDialogueSavedCameraRotation[4]{};
float g_npcDialogueCameraBaseOffsetX = 0.0f;
float g_npcDialogueCameraBaseOffsetZ = 0.0f;

constexpr float kNpcDialogueChoiceMinX = 0.615f;
constexpr float kNpcDialogueChoiceMaxX = 0.865f;
constexpr float kNpcDialogueChoice0MinY = 0.335f;
constexpr float kNpcDialogueChoice0MaxY = 0.405f;
constexpr float kNpcDialogueChoice1MinY = 0.415f;
constexpr float kNpcDialogueChoice1MaxY = 0.485f;
constexpr float kNpcDialogueCameraPitchDegrees = 10.0f;
constexpr float kNpcDialoguePatienceMax = 60.0f;
constexpr float kNpcDialogueTalkRangeMeters = 2.0f;
constexpr float kNpcDialogueExcitementAnimationSeconds = 0.42f;
constexpr float kNpcDialogueBarMinY = 0.165f;
constexpr float kNpcDialogueBarMaxY = 0.780f;
constexpr float kNpcDialogueQteButtonMinX = 0.758f;
constexpr float kNpcDialogueQteButtonMaxX = 0.878f;
constexpr float kNpcDialogueQteButtonMinY = 0.515f;
constexpr float kNpcDialogueQteButtonMaxY = 0.565f;

bool NpcDialogue_IsActive()
{
    return g_npcDialogueActive.load();
}

bool NpcDialogue_ShouldBlockSystemMenuOpen()
{
    return g_npcDialogueActive.load() ||
           GetTickCount64() <
               g_npcDialogueSystemMenuBlockedUntil.load();
}

bool NpcDialogue_IsTalkPromptVisible()
{
    return g_npcDialoguePromptVisible.load();
}

void NpcDialogue_RequestTalk()
{
    g_npcDialogueTalkRequested = true;
}

void NpcDialogue_RequestClose()
{
    g_npcDialogueSystemMenuBlockedUntil.store(
        GetTickCount64() + 750);
    g_npcDialogueCloseRequested = true;
    // Esc belongs to the dialogue while it is open. Never let the same key
    // press continue into the original system menu after the panel closes.
    g_extendedOpenSystemMenuRequested = false;
}

void NpcDialogue_ReserveTalkKey()
{
    g_npcDialogueTalkReservedUntil.store(
        GetTickCount64() + 250);
}

bool NpcDialogue_IsTalkKeyReserved()
{
    return GetTickCount64() <
        g_npcDialogueTalkReservedUntil.load();
}

bool NpcDialogue_HandleTalkKeyDown()
{
    if (NpcDialogue_IsActive())
    {
        NpcDialogue_RequestClose();
        return true;
    }
    if (NpcDialogue_IsTalkPromptVisible())
    {
        NpcDialogue_RequestTalk();
        return true;
    }
    return false;
}

Il2CppObject* NpcDialogue_GetNpcControllerManager()
{
    auto* managerClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcManager");
    return managerClass != nullptr
        ? Invoke(
            FindMethod(managerClass, "get_Instance", 0),
            nullptr,
            nullptr)
        : nullptr;
}

bool NpcDialogue_IsAlive(Il2CppObject* npc)
{
    auto* controllerClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcController");
    auto* method = FindMethod(controllerClass, "get_IsAlive", 0);
    if (npc == nullptr || method == nullptr)
    {
        return false;
    }
    auto* value = Invoke(method, npc, nullptr);
    return value != nullptr &&
           il2cpp_object_unbox != nullptr &&
           *static_cast<bool*>(il2cpp_object_unbox(value));
}

Il2CppObject* NpcDialogue_GetNpcStateModel(Il2CppObject* npc)
{
    auto* nca = npc != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(npc) + 0x30)
        : nullptr;
    return nca != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(nca) + 0x40)
        : nullptr;
}

Il2CppObject* NpcDialogue_GetNpcGameObject(Il2CppObject* npc)
{
    return npc != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(npc) + 0x18)
        : nullptr;
}

bool NpcDialogue_GetTransformPosition(
    Il2CppObject* transform,
    float* output)
{
    if (transform == nullptr || output == nullptr)
    {
        return false;
    }
    auto* value = Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "get_position", 0),
        transform,
        nullptr);
    if (value == nullptr)
    {
        return false;
    }
    std::memcpy(output, il2cpp_object_unbox(value), sizeof(float) * 3);
    return true;
}

bool NpcDialogue_IsTargetWithinTalkRange(Il2CppObject* npc)
{
    if (npc == nullptr ||
        g_extendedPlayerGameObject == nullptr)
    {
        return false;
    }
    auto* playerTransform = GetWorldTransform(
        g_extendedPlayerGameObject);
    auto* npcTransform = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(npc) + 0x10);
    float playerPosition[3]{};
    float npcPosition[3]{};
    if (!NpcDialogue_GetTransformPosition(
            playerTransform,
            playerPosition) ||
        !NpcDialogue_GetTransformPosition(
            npcTransform,
            npcPosition))
    {
        return false;
    }
    const float dx = npcPosition[0] - playerPosition[0];
    const float dy = npcPosition[1] - playerPosition[1];
    const float dz = npcPosition[2] - playerPosition[2];
    return dx * dx + dy * dy + dz * dz <=
        kNpcDialogueTalkRangeMeters *
            kNpcDialogueTalkRangeMeters;
}

bool NpcDialogue_GetModelBounds(
    Il2CppObject* gameObject,
    float* center,
    float& height)
{
    if (gameObject == nullptr ||
        center == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        return false;
    }
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (rendererClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return false;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    bool includeInactive = false;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        gameObject,
        parameters);
    if (renderers == nullptr)
    {
        return false;
    }

    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    auto* getEnabled = FindMethod(rendererClass, "get_enabled", 0);
    auto* getBounds = FindMethod(rendererClass, "get_bounds", 0);
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
        if (boundsValue == nullptr)
        {
            continue;
        }
        const auto* bounds = static_cast<float*>(
            il2cpp_object_unbox(boundsValue));
        minX = std::min(minX, bounds[0] - bounds[3]);
        maxX = std::max(maxX, bounds[0] + bounds[3]);
        minY = std::min(minY, bounds[1] - bounds[4]);
        maxY = std::max(maxY, bounds[1] + bounds[4]);
        minZ = std::min(minZ, bounds[2] - bounds[5]);
        maxZ = std::max(maxZ, bounds[2] + bounds[5]);
    }
    if (minY > maxY)
    {
        return false;
    }
    center[0] = (minX + maxX) * 0.5f;
    center[1] = (minY + maxY) * 0.5f;
    center[2] = (minZ + maxZ) * 0.5f;
    height = maxY - minY;
    return height > 0.1f;
}

void NpcDialogue_BuildUi()
{
    if (g_npcDialogueRoot != nullptr ||
        g_extendedGameplayHudCanvasObject == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    auto* rootTransform = GetWorldTransform(
        g_extendedGameplayHudCanvasObject);
    if (rootTransform == nullptr)
    {
        return;
    }

    auto root = CreateScenePanel(
        "EFM_NpcDialogue",
        rootTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    g_npcDialogueRoot = root.gameObject;

    auto box = CreateScenePanel(
        "EFM_NpcDialogueBox",
        root.rect,
        0.115f,
        0.065f,
        0.885f,
        0.300f,
        0.018f,
        0.027f,
        0.045f,
        0.96f);
    CreateScenePanel(
        "EFM_NpcDialogueAccent",
        root.rect,
        0.115f,
        0.286f,
        0.885f,
        0.300f,
        0.12f,
        0.36f,
        0.42f,
        0.96f);
    auto namePlate = CreateScenePanel(
        "EFM_NpcDialogueNamePlate",
        root.rect,
        0.135f,
        0.270f,
        0.300f,
        0.335f,
        0.04f,
        0.11f,
        0.14f,
        0.98f);
    g_npcDialogueNameText = CreateSceneTextElement(
        "EFM_NpcDialogueName",
        namePlate.rect,
        "NPC",
        0.04f,
        0.10f,
        0.96f,
        0.90f,
        18.0f,
        514,
        0.96f,
        0.98f,
        1.0f,
        1.0f).text;
    g_npcDialogueBodyText = CreateSceneTextElement(
        "EFM_NpcDialogueBody",
        box.rect,
        "找我有事吗",
        0.045f,
        0.38f,
        0.955f,
        0.82f,
        24.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f).text;
    g_npcDialogueHintText = CreateSceneTextElement(
        "EFM_NpcDialogueHint",
        box.rect,
        "选择回答",
        0.045f,
        0.10f,
        0.955f,
        0.24f,
        14.0f,
        514,
        0.62f,
        0.72f,
        0.82f,
        1.0f).text;

    auto choice0 = CreateScenePanel(
        "EFM_NpcDialogueChoice0",
        root.rect,
        kNpcDialogueChoiceMinX,
        kNpcDialogueChoice0MinY,
        kNpcDialogueChoiceMaxX,
        kNpcDialogueChoice0MaxY,
        0.055f,
        0.085f,
        0.12f,
        0.98f);
    g_npcDialogueChoice0Panel = choice0.gameObject;
    g_npcDialogueChoice0Text = CreateSceneTextElement(
        "EFM_NpcDialogueChoiceText0",
        choice0.rect,
        "有事",
        0.04f,
        0.10f,
        0.96f,
        0.90f,
        21.0f,
        514,
        0.95f,
        0.97f,
        1.0f,
        1.0f).text;
    auto choice1 = CreateScenePanel(
        "EFM_NpcDialogueChoice1",
        root.rect,
        kNpcDialogueChoiceMinX,
        kNpcDialogueChoice1MinY,
        kNpcDialogueChoiceMaxX,
        kNpcDialogueChoice1MaxY,
        0.055f,
        0.085f,
        0.12f,
        0.98f);
    g_npcDialogueChoice1Panel = choice1.gameObject;
    g_npcDialogueChoice1Text = CreateSceneTextElement(
        "EFM_NpcDialogueChoiceText1",
        choice1.rect,
        "没事",
        0.04f,
        0.10f,
        0.96f,
        0.90f,
        21.0f,
        514,
        0.95f,
        0.97f,
        1.0f,
        1.0f).text;

    g_npcDialoguePatienceLabel = CreateSceneTextElement(
        "EFM_NpcDialoguePatienceLabel",
        root.rect,
        "耐心",
        0.025f,
        0.785f,
        0.075f,
        0.835f,
        15.0f,
        514,
        0.98f,
        0.76f,
        0.18f,
        1.0f);
    g_npcDialogueExcitementLabel = CreateSceneTextElement(
        "EFM_NpcDialogueExcitementLabel",
        root.rect,
        "兴奋",
        0.675f,
        0.785f,
        0.735f,
        0.835f,
        15.0f,
        514,
        1.0f,
        0.48f,
        0.72f,
        1.0f);
    g_npcDialoguePatienceTrack = CreateScenePanel(
        "EFM_NpcDialoguePatienceTrack",
        root.rect,
        0.034f,
        kNpcDialogueBarMinY,
        0.062f,
        kNpcDialogueBarMaxY,
        0.035f,
        0.040f,
        0.050f,
        0.82f);
    g_npcDialoguePatienceFill = CreateScenePanel(
        "EFM_NpcDialoguePatienceFill",
        root.rect,
        0.036f,
        kNpcDialogueBarMinY,
        0.060f,
        kNpcDialogueBarMaxY,
        0.98f,
        0.72f,
        0.12f,
        0.96f);
    g_npcDialogueExcitementTrack = CreateScenePanel(
        "EFM_NpcDialogueExcitementTrack",
        root.rect,
        0.690f,
        kNpcDialogueBarMinY,
        0.718f,
        kNpcDialogueBarMaxY,
        0.035f,
        0.040f,
        0.050f,
        0.82f);
    g_npcDialogueExcitementFill = CreateScenePanel(
        "EFM_NpcDialogueExcitementFill",
        root.rect,
        0.692f,
        kNpcDialogueBarMinY,
        0.716f,
        kNpcDialogueBarMaxY,
        1.0f,
        0.30f,
        0.62f,
        0.96f);
    g_npcDialogueQteBackdrop = CreateScenePanel(
        "EFM_NpcDialogueQteBackdrop",
        root.rect,
        0.738f,
        0.580f,
        0.898f,
        0.720f,
        0.025f,
        0.040f,
        0.060f,
        0.34f);
    g_npcDialogueQteWheel = CreateSceneRawImageElement(
        "EFM_NpcDialogueQteWheel",
        root.rect,
        L"",
        0.738f,
        0.580f,
        0.898f,
        0.720f,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    g_npcDialogueQtePointer = CreateSceneRawImageElement(
        "EFM_NpcDialogueQtePointer",
        root.rect,
        L"",
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        1.0f,
        0.48f,
        0.72f,
        1.0f);
    g_npcDialogueQteButton = CreateScenePanel(
        "EFM_NpcDialogueQteButton",
        root.rect,
        kNpcDialogueQteButtonMinX,
        kNpcDialogueQteButtonMinY,
        kNpcDialogueQteButtonMaxX,
        kNpcDialogueQteButtonMaxY,
        0.20f,
        0.35f,
        0.55f,
        0.96f);
    g_npcDialogueQteButtonText = CreateSceneTextElement(
        "EFM_NpcDialogueQteButtonText",
        g_npcDialogueQteButton.rect,
        "QTE",
        0.04f,
        0.10f,
        0.96f,
        0.90f,
        15.0f,
        514,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    g_npcDialogueQteFeedbackOverlay = CreateScenePanel(
        "EFM_NpcDialogueQteFeedback",
        root.rect,
        0.738f,
        0.515f,
        0.898f,
        0.720f,
        1.0f,
        0.08f,
        0.08f,
        0.0f);

    SetWorldGameObjectActive(
        g_npcDialoguePatienceLabel.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueExcitementLabel.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialoguePatienceTrack.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialoguePatienceFill.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueExcitementTrack.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueExcitementFill.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueQteWheel.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueQteBackdrop.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueQtePointer.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueQteButton.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueQteFeedbackOverlay.gameObject,
        false);
    BringSceneObjectToFront(g_npcDialoguePatienceTrack.gameObject);
    BringSceneObjectToFront(g_npcDialoguePatienceFill.gameObject);
    BringSceneObjectToFront(g_npcDialogueExcitementTrack.gameObject);
    BringSceneObjectToFront(g_npcDialogueExcitementFill.gameObject);
    BringSceneObjectToFront(g_npcDialoguePatienceLabel.gameObject);
    BringSceneObjectToFront(g_npcDialogueExcitementLabel.gameObject);
    BringSceneObjectToFront(g_npcDialogueQteBackdrop.gameObject);
    BringSceneObjectToFront(g_npcDialogueQteWheel.gameObject);
    BringSceneObjectToFront(g_npcDialogueQtePointer.gameObject);
    BringSceneObjectToFront(g_npcDialogueQteButton.gameObject);
    BringSceneObjectToFront(
        g_npcDialogueQteFeedbackOverlay.gameObject);

    SetWorldGameObjectActive(g_npcDialogueRoot, false);
    BringSceneObjectToFront(g_npcDialogueRoot);
    LogInfo(LogCategory_Dialogue, L"Dialogue", L"Custom NPC dialogue UI built.");
}

Il2CppObject* NpcDialogue_FindCenterNpc()
{
    if (g_extendedCamera == nullptr ||
        g_extendedPlayerGameObject == nullptr)
    {
        return nullptr;
    }
    auto* manager = NpcDialogue_GetNpcControllerManager();
    auto* list = manager != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(manager) + 0x38)
        : nullptr;
    if (list == nullptr)
    {
        return nullptr;
    }
    auto* items = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(list) + 0x10);
    const int count = *reinterpret_cast<int*>(
        reinterpret_cast<char*>(list) + 0x18);
    if (items == nullptr || count <= 0)
    {
        return nullptr;
    }

    auto* cameraClass = FindClass("UnityEngine", "Camera");
    auto* screenClass = FindClass("UnityEngine", "Screen");
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* worldToScreenPoint = FindMethod(
        cameraClass,
        "WorldToScreenPoint",
        1);
    auto* getScreenWidth = FindMethod(screenClass, "get_width", 0);
    auto* getScreenHeight = FindMethod(screenClass, "get_height", 0);
    auto* getPosition = FindMethod(
        transformClass,
        "get_position",
        0);
    if (worldToScreenPoint == nullptr ||
        getScreenWidth == nullptr ||
        getScreenHeight == nullptr ||
        getPosition == nullptr)
    {
        return nullptr;
    }
    auto* widthValue = Invoke(getScreenWidth, nullptr, nullptr);
    auto* heightValue = Invoke(getScreenHeight, nullptr, nullptr);
    if (widthValue == nullptr || heightValue == nullptr)
    {
        return nullptr;
    }
    const float screenWidth = static_cast<float>(
        *static_cast<int*>(
        il2cpp_object_unbox(widthValue)));
    const float screenHeight = static_cast<float>(
        *static_cast<int*>(
        il2cpp_object_unbox(heightValue)));
    if (screenWidth <= 1.0f || screenHeight <= 1.0f)
    {
        return nullptr;
    }
    const float centerTolerance =
        std::max(64.0f, screenHeight * 0.16f);
    auto* playerTransform = GetWorldTransform(
        g_extendedPlayerGameObject);
    float playerPosition[3]{};
    if (!NpcDialogue_GetTransformPosition(
            playerTransform,
            playerPosition))
    {
        return nullptr;
    }
    auto** npcs = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(items) +
        il2cpp_array_object_header_size());

    Il2CppObject* best = nullptr;
    float bestDistance = centerTolerance;
    float nearestDistance = 1.0e9f;
    float nearestWorldDistance = 1.0e9f;
    for (int index = 0; index < count; ++index)
    {
        auto* npc = npcs[index];
        auto* gameObject = NpcDialogue_GetNpcGameObject(npc);
        auto* stateModel = NpcDialogue_GetNpcStateModel(npc);
        auto* transform = npc != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(npc) + 0x10)
            : nullptr;
        if (gameObject == nullptr ||
            stateModel == nullptr ||
            transform == nullptr ||
            !NpcDialogue_IsAlive(npc) ||
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(stateModel) + 0x10) != 0 ||
            *reinterpret_cast<int*>(
                reinterpret_cast<char*>(stateModel) + 0x58) != 0 ||
            *reinterpret_cast<bool*>(
                reinterpret_cast<char*>(stateModel) + 0x1F) ||
            *reinterpret_cast<bool*>(
                reinterpret_cast<char*>(stateModel) + 0x21))
        {
            continue;
        }
        auto* positionValue = Invoke(
            getPosition,
            transform,
            nullptr);
        if (positionValue == nullptr)
        {
            continue;
        }
        const auto* position = static_cast<float*>(
            il2cpp_object_unbox(positionValue));
        const float worldDx = position[0] - playerPosition[0];
        const float worldDy = position[1] - playerPosition[1];
        const float worldDz = position[2] - playerPosition[2];
        const float worldDistanceSquared =
            worldDx * worldDx +
            worldDy * worldDy +
            worldDz * worldDz;
        nearestWorldDistance = std::min(
            nearestWorldDistance,
            std::sqrt(worldDistanceSquared));
        if (worldDistanceSquared >
            kNpcDialogueTalkRangeMeters *
                kNpcDialogueTalkRangeMeters)
        {
            continue;
        }
        const float scale = *reinterpret_cast<float*>(
            reinterpret_cast<char*>(stateModel) + 0x5C);
        float npcCenter[3] = {
            position[0],
            position[1] + 1.10f * std::max(0.7f, scale),
            position[2],
        };
        void* projectParameters[] = { npcCenter };
        auto* projectedValue = Invoke(
            worldToScreenPoint,
            g_extendedCamera,
            projectParameters);
        if (projectedValue == nullptr)
        {
            continue;
        }
        const auto* projected = static_cast<float*>(
            il2cpp_object_unbox(projectedValue));
        if (projected[2] <= 0.01f)
        {
            continue;
        }
        const float dx = projected[0] - screenWidth * 0.5f;
        const float dy = projected[1] - screenHeight * 0.5f;
        const float distance = std::sqrt(dx * dx + dy * dy);
        nearestDistance = std::min(nearestDistance, distance);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = npc;
        }
    }
    static unsigned long long lastScanLog = 0;
    const unsigned long long now = GetTickCount64();
    if (best == nullptr && now - lastScanLog >= 1500)
    {
        lastScanLog = now;
        LogTrace(
            LogCategory_Dialogue,
            L"AimScan",
            L"NPC dialogue aim scan: count=%d nearest=%0.1f threshold=%0.1f range=%.1fm nearestWorld=%.2fm.",
            count,
            nearestDistance,
            centerTolerance,
            kNpcDialogueTalkRangeMeters,
            nearestWorldDistance);
    }
    return best;
}

bool NpcDialogue_IsHorizontalPathBlocked(
    const float* pivot,
    const float* candidate,
    int layerMask)
{
    float direction[3] = {
        candidate[0] - pivot[0],
        0.0f,
        candidate[2] - pivot[2],
    };
    const float length = std::sqrt(
        direction[0] * direction[0] +
        direction[2] * direction[2]);
    if (length <= 0.2f)
    {
        return false;
    }
    direction[0] /= length;
    direction[2] /= length;
    float origin[3] = {
        pivot[0] + direction[0] * 0.25f,
        pivot[1],
        pivot[2] + direction[2] * 0.25f,
    };
    float maxDistance = std::max(0.1f, length - 0.35f);
    int queryTriggerInteraction = 1; // QueryTriggerInteraction.Ignore
    void* parameters[] = {
        origin,
        direction,
        &maxDistance,
        &layerMask,
        &queryTriggerInteraction,
    };
    auto* result = Invoke(
        FindMethod(FindClass("UnityEngine", "Physics"), "Raycast", 5),
        nullptr,
        parameters);
    return result != nullptr &&
           il2cpp_object_unbox != nullptr &&
           *static_cast<bool*>(il2cpp_object_unbox(result));
}

void NpcDialogue_PlaceCamera(
    Il2CppObject* npc,
    bool initialize)
{
    if (g_extendedCamera == nullptr ||
        g_extendedPlayerGameObject == nullptr ||
        npc == nullptr)
    {
        return;
    }
    float playerCenter[3]{};
    float npcCenter[3]{};
    float playerHeight = 1.65f;
    float npcHeight = 1.65f;
    const bool playerBoundsReady = NpcDialogue_GetModelBounds(
        g_extendedAvatarRoot != nullptr
            ? g_extendedAvatarRoot
            : g_extendedPlayerGameObject,
        playerCenter,
        playerHeight);
    const bool npcBoundsReady = NpcDialogue_GetModelBounds(
        g_npcDialogueTargetGameObject,
        npcCenter,
        npcHeight);
    if (!playerBoundsReady || !npcBoundsReady)
    {
        if (initialize)
        {
            LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue camera used transform fallback for model bounds.");
        }
        auto* playerTransform = GetWorldTransform(g_extendedPlayerGameObject);
        auto* npcTransform = npc != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(npc) + 0x10)
            : nullptr;
        if (!NpcDialogue_GetTransformPosition(
                playerTransform,
                playerCenter) ||
            !NpcDialogue_GetTransformPosition(
                npcTransform,
                npcCenter))
        {
            return;
        }
        playerCenter[1] += 0.9f;
        npcCenter[1] += 0.9f;
        playerHeight = 1.65f;
        npcHeight = 1.65f;
    }

    auto* cameraTransform = GetWorldTransform(
        GetComponentGameObject(g_extendedCamera));
    if (cameraTransform == nullptr)
    {
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    if (initialize)
    {
        auto* positionValue = Invoke(
            FindMethod(transformClass, "get_position", 0),
            cameraTransform,
            nullptr);
        auto* rotationValue = Invoke(
            FindMethod(transformClass, "get_rotation", 0),
            cameraTransform,
            nullptr);
        if (positionValue == nullptr || rotationValue == nullptr)
        {
            return;
        }
        std::memcpy(
            g_npcDialogueSavedCameraPosition,
            il2cpp_object_unbox(positionValue),
            sizeof(float) * 3);
        std::memcpy(
            g_npcDialogueSavedCameraRotation,
            il2cpp_object_unbox(rotationValue),
            sizeof(float) * 4);
        g_npcDialogueCameraSaved = true;
    }

    const float floorY =
        (playerCenter[1] - playerHeight * 0.5f +
         npcCenter[1] - npcHeight * 0.5f) * 0.5f;
    float pivot[3] = {
        (playerCenter[0] + npcCenter[0]) * 0.5f,
        floorY + (playerHeight + npcHeight) * 0.5f,
        (playerCenter[2] + npcCenter[2]) * 0.5f,
    };

    if (initialize || !g_npcDialogueCameraDirectionReady)
    {
        auto* fpCamera = g_extendedAvatarReferencer != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedAvatarReferencer) + 0x78)
            : nullptr;
        const bool firstPerson =
            fpCamera != nullptr &&
            ExtendedMode_GetBehaviourEnabled(fpCamera);
        float baseX =
            g_npcDialogueSavedCameraPosition[0] - pivot[0];
        float baseZ =
            g_npcDialogueSavedCameraPosition[2] - pivot[2];
        float radius = std::sqrt(baseX * baseX + baseZ * baseZ);
        auto* tpCamera = g_extendedInGameReferencer != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedInGameReferencer) + 0x40)
            : nullptr;
        const float configuredRadius = tpCamera != nullptr
            ? *reinterpret_cast<float*>(
                reinterpret_cast<char*>(tpCamera) + 0x28)
            : 3.2f;
        if (firstPerson || radius < 0.75f)
        {
            radius = configuredRadius > 0.5f ? configuredRadius : 3.2f;
            auto* forwardValue = Invoke(
                FindMethod(transformClass, "get_forward", 0),
                GetWorldTransform(g_extendedPlayerGameObject),
                nullptr);
            if (forwardValue != nullptr)
            {
                const auto* forward = static_cast<float*>(
                    il2cpp_object_unbox(forwardValue));
                baseX = -forward[0] * radius;
                baseZ = -forward[2] * radius;
            }
            else
            {
                baseX = 0.0f;
                baseZ = -radius;
            }
        }
        g_npcDialogueCameraBaseOffsetX = baseX;
        g_npcDialogueCameraBaseOffsetZ = baseZ;
        g_npcDialogueCameraDirectionReady = true;
    }

    const float averageModelHeight =
        (playerHeight + npcHeight) * 0.5f;
    const float cameraHeight =
        floorY + averageModelHeight * 0.75f;
    constexpr float orbitAngleDegrees = 135.0f;
    constexpr float degreesToRadians =
        0.01745329251994329577f;
    const float orbitCos =
        std::cos(orbitAngleDegrees * degreesToRadians);
    const float orbitSin =
        std::sin(orbitAngleDegrees * degreesToRadians);
    float ccw[3] = {
        pivot[0] +
            g_npcDialogueCameraBaseOffsetX * orbitCos -
            g_npcDialogueCameraBaseOffsetZ * orbitSin,
        cameraHeight,
        pivot[2] +
            g_npcDialogueCameraBaseOffsetX * orbitSin +
            g_npcDialogueCameraBaseOffsetZ * orbitCos,
    };
    float cw[3] = {
        pivot[0] +
            g_npcDialogueCameraBaseOffsetX * orbitCos +
            g_npcDialogueCameraBaseOffsetZ * orbitSin,
        cameraHeight,
        pivot[2] -
            g_npcDialogueCameraBaseOffsetX * orbitSin +
            g_npcDialogueCameraBaseOffsetZ * orbitCos,
    };
    int playerLayer = ExtendedMode_GetGameObjectLayer(
        g_extendedPlayerGameObject);
    int npcLayer = ExtendedMode_GetGameObjectLayer(
        g_npcDialogueTargetGameObject);
    int layerMask = ~((1 << playerLayer) | (1 << npcLayer));
    const bool useClockwise = NpcDialogue_IsHorizontalPathBlocked(
        pivot,
        ccw,
        layerMask);
    float* selected = useClockwise ? cw : ccw;
    const float yaw =
        std::atan2(
            pivot[0] - selected[0],
            pivot[2] - selected[2]) *
        57.29577951308232f;
    g_npcDialogueTargetCameraPosition[0] = selected[0];
    g_npcDialogueTargetCameraPosition[1] = selected[1];
    g_npcDialogueTargetCameraPosition[2] = selected[2];
    g_npcDialogueTargetCameraYaw = yaw;
    g_npcDialogueCameraInitialized = true;
    if (initialize)
    {
        g_npcDialogueLastCameraTick = 0;
        g_npcDialogueCameraTransitionComplete = false;
        LogInfo(LogCategory_Dialogue, L"Dialogue", 
            L"NPC dialogue camera target set %s at %.2f height.",
            useClockwise ? L"clockwise" : L"counter-clockwise",
            selected[1]);
    }
}

void NpcDialogue_UpdateCameraMotion()
{
    if (!g_npcDialogueCameraInitialized ||
        g_extendedCamera == nullptr)
    {
        return;
    }
    auto* cameraTransform = GetWorldTransform(
        GetComponentGameObject(g_extendedCamera));
    auto* transformClass = FindClass("UnityEngine", "Transform");
    if (cameraTransform == nullptr || transformClass == nullptr)
    {
        return;
    }
    const unsigned long long now = GetTickCount64();
    if (g_npcDialogueLastCameraTick == 0)
    {
        g_npcDialogueLastCameraTick = now;
        return;
    }
    const float deltaSeconds = std::clamp(
        static_cast<float>(now - g_npcDialogueLastCameraTick) / 1000.0f,
        0.0f,
        0.1f);
    g_npcDialogueLastCameraTick = now;

    auto* positionValue = Invoke(
        FindMethod(transformClass, "get_position", 0),
        cameraTransform,
        nullptr);
    auto* eulerValue = Invoke(
        FindMethod(transformClass, "get_eulerAngles", 0),
        cameraTransform,
        nullptr);
    if (positionValue == nullptr || eulerValue == nullptr)
    {
        return;
    }
    auto* position = static_cast<float*>(
        il2cpp_object_unbox(positionValue));
    auto* euler = static_cast<float*>(
        il2cpp_object_unbox(eulerValue));
    float positionDelta[3] = {
        g_npcDialogueTargetCameraPosition[0] - position[0],
        g_npcDialogueTargetCameraPosition[1] - position[1],
        g_npcDialogueTargetCameraPosition[2] - position[2],
    };
    const float distance = std::sqrt(
        positionDelta[0] * positionDelta[0] +
        positionDelta[1] * positionDelta[1] +
        positionDelta[2] * positionDelta[2]);
    // Twenty percent faster than the previous 2.5 m/s dialogue orbit.
    constexpr float cameraMoveSpeed = 3.0f;
    const float positionStep = cameraMoveSpeed * deltaSeconds;
    if (distance <= positionStep || distance <= 0.0001f)
    {
        position[0] = g_npcDialogueTargetCameraPosition[0];
        position[1] = g_npcDialogueTargetCameraPosition[1];
        position[2] = g_npcDialogueTargetCameraPosition[2];
    }
    else
    {
        position[0] +=
            positionDelta[0] / distance * positionStep;
        position[1] +=
            positionDelta[1] / distance * positionStep;
        position[2] +=
            positionDelta[2] / distance * positionStep;
    }

    float yawDelta = std::remainder(
        g_npcDialogueTargetCameraYaw - euler[1],
        360.0f);
    float pitchDelta = std::remainder(
        kNpcDialogueCameraPitchDegrees - euler[0],
        360.0f);
    float rollDelta = std::remainder(-euler[2], 360.0f);
    constexpr float cameraRotateSpeed = 110.0f;
    const float rotationStep = cameraRotateSpeed * deltaSeconds;
    euler[0] += std::clamp(pitchDelta, -rotationStep, rotationStep);
    euler[1] += std::clamp(yawDelta, -rotationStep, rotationStep);
    euler[2] += std::clamp(rollDelta, -rotationStep, rotationStep);
    if (std::abs(pitchDelta) <= rotationStep)
    {
        euler[0] = kNpcDialogueCameraPitchDegrees;
    }
    if (std::abs(rollDelta) <= rotationStep)
    {
        euler[2] = 0.0f;
    }

    void* positionParameters[] = { position };
    void* eulerParameters[] = { euler };
    Invoke(
        FindMethod(transformClass, "set_position", 1),
        cameraTransform,
        positionParameters);
    Invoke(
        FindMethod(transformClass, "set_eulerAngles", 1),
        cameraTransform,
        eulerParameters);
    g_npcDialogueCameraTransitionComplete =
        distance <= positionStep + 0.001f &&
        std::abs(yawDelta) <= rotationStep + 0.01f &&
        std::abs(pitchDelta) <= rotationStep + 0.01f &&
        std::abs(rollDelta) <= rotationStep + 0.01f;
}

void NpcDialogue_UpdateContent()
{
    auto* stateModel = NpcDialogue_GetNpcStateModel(
        g_npcDialogueTarget);
    const bool female =
        stateModel != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(stateModel) + 0x22);
    if (g_npcDialogueNameText != nullptr)
    {
        SetSceneText(
            g_npcDialogueNameText,
            female ? "NPC女" : "NPC男",
            18.0f,
            514,
            0.96f,
            0.98f,
            1.0f,
            1.0f);
    }
    if (g_npcDialogueBodyText != nullptr)
    {
        SetSceneText(
            g_npcDialogueBodyText,
            g_npcDialogueStage == 0
                ? "找我有事吗"
                : (g_npcDialogueStage == 1
                    ? "别烦我"
                    : "何意味"),
            24.0f,
            514,
            0.96f,
            0.97f,
            0.99f,
            1.0f);
    }
    SetWorldGameObjectActive(
        g_npcDialogueChoice0Panel,
        g_npcDialogueStage == 0 &&
        !g_npcDialogueClimaxActive);
    SetWorldGameObjectActive(
        g_npcDialogueChoice1Panel,
        g_npcDialogueStage == 0 &&
        !g_npcDialogueClimaxActive);
    if (g_npcDialogueHintText != nullptr)
    {
        SetSceneText(
            g_npcDialogueHintText,
            g_npcDialogueClimaxActive
                ? "高潮反应中，按 T 或 Esc 结束对话"
                : (g_npcDialogueStage == 0
                ? "左键选择回答"
                : "按 T 或 Esc 结束对话"),
            14.0f,
            514,
            0.62f,
            0.72f,
            0.82f,
            1.0f);
    }
}

Il2CppObject* NpcDialogue_GetNpcLocomotion(Il2CppObject* npc)
{
    auto* nca = npc != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(npc) + 0x30)
        : nullptr;
    return nca != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(nca) + 0x20)
        : nullptr;
}

Il2CppObject* NpcDialogue_GetNpcAnimationManager(Il2CppObject* npc)
{
    auto* nca = npc != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(npc) + 0x30)
        : nullptr;
    return nca != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(nca) + 0xA0)
        : nullptr;
}

Il2CppObject* NpcDialogue_GetPlayerClassAccessor()
{
    return g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
}

Il2CppObject* NpcDialogue_GetPlayerAnimationManager()
{
    auto* pca = NpcDialogue_GetPlayerClassAccessor();
    return pca != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(pca) + 0x48)
        : nullptr;
}

bool NpcDialogue_SaveTransformRotation(
    Il2CppObject* transform,
    float* output)
{
    if (transform == nullptr || output == nullptr)
    {
        return false;
    }
    auto* value = Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "get_rotation",
            0),
        transform,
        nullptr);
    if (value == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    std::memcpy(
        output,
        il2cpp_object_unbox(value),
        sizeof(float) * 4);
    return true;
}

void NpcDialogue_RestoreTransformRotation(
    Il2CppObject* transform,
    const float* rotation)
{
    if (transform == nullptr || rotation == nullptr)
    {
        return;
    }
    void* parameters[] = { const_cast<float*>(rotation) };
    Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "set_rotation",
            1),
        transform,
        parameters);
}

bool NpcDialogue_SetHorizontalFacing(
    Il2CppObject* transform,
    float directionX,
    float directionZ)
{
    const float length = std::sqrt(
        directionX * directionX +
        directionZ * directionZ);
    if (transform == nullptr || length <= 0.0001f)
    {
        return false;
    }
    float euler[3] = {
        0.0f,
        std::atan2(directionX, directionZ) *
            57.29577951308232f,
        0.0f,
    };
    auto* setEulerAngles = FindMethod(
        FindClass("UnityEngine", "Transform"),
        "set_eulerAngles",
        1);
    if (setEulerAngles == nullptr)
    {
        return false;
    }
    void* parameters[] = { euler };
    Invoke(
        setEulerAngles,
        transform,
        parameters);
    return true;
}

void NpcDialogue_FaceActorsTowardEachOther()
{
    auto* playerTransform = GetWorldTransform(
        g_extendedPlayerGameObject);
    auto* npcTransform = g_npcDialogueTarget != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_npcDialogueTarget) + 0x10)
        : nullptr;
    float playerPosition[3]{};
    float npcPosition[3]{};
    if (!NpcDialogue_GetTransformPosition(
            playerTransform,
            playerPosition) ||
        !NpcDialogue_GetTransformPosition(
            npcTransform,
            npcPosition))
    {
        return;
    }
    const float directionX =
        npcPosition[0] - playerPosition[0];
    const float directionZ =
        npcPosition[2] - playerPosition[2];
    NpcDialogue_SetHorizontalFacing(
        playerTransform,
        directionX,
        directionZ);
    NpcDialogue_SetHorizontalFacing(
        npcTransform,
        -directionX,
        -directionZ);
}

void NpcDialogue_LockActorAnimations()
{
    auto* playerTransform = GetWorldTransform(
        g_extendedPlayerGameObject);
    auto* npcTransform = g_npcDialogueTarget != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_npcDialogueTarget) + 0x10)
        : nullptr;
    g_npcDialoguePlayerRotationSaved =
        NpcDialogue_SaveTransformRotation(
            playerTransform,
            g_npcDialogueSavedPlayerRotation);
    g_npcDialogueNpcRotationSaved =
        NpcDialogue_SaveTransformRotation(
            npcTransform,
            g_npcDialogueSavedNpcRotation);

    auto* playerAnimation = NpcDialogue_GetPlayerAnimationManager();
    if (playerAnimation != nullptr)
    {
        // TargetMoveSpeed is zero whenever the player stopped before opening
        // the dialogue. Restoring that transient value overwrites the original
        // locomotion base speed and leaves walking animation frozen at frame 0.
        g_npcDialogueSavedPlayerBaseMoveSpeed =
            *reinterpret_cast<float*>(
                reinterpret_cast<char*>(playerAnimation) + 0x24);
        g_npcDialoguePlayerMoveSpeedSaved = true;
    }

    auto* npcAnimation =
        NpcDialogue_GetNpcAnimationManager(g_npcDialogueTarget);
    auto* npcAnimationClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAnimationManager");
    auto* getNpcSpeed = npcAnimation != nullptr
        ? FindMethod(
            npcAnimationClass,
            "GetCurrentMoveSpeed",
            0)
        : nullptr;
    if (npcAnimation != nullptr && getNpcSpeed != nullptr)
    {
        auto* value = Invoke(
            getNpcSpeed,
            npcAnimation,
            nullptr);
        if (value != nullptr && il2cpp_object_unbox != nullptr)
        {
            g_npcDialogueSavedNpcMoveSpeed =
                *static_cast<float*>(
                    il2cpp_object_unbox(value));
            g_npcDialogueNpcMoveSpeedSaved = true;
        }
    }

    // Return the player action layer to the shipped idle/locomotion path.
    ExtendedMode_PlayOriginalPose(-1);
    auto* playerAnimationClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationManager");
    auto* setPlayerSpeed = FindMethod(
        playerAnimationClass,
        "SetMoveSpeed",
        1);
    if (playerAnimation != nullptr &&
        setPlayerSpeed != nullptr)
    {
        float zero = 0.0f;
        void* parameters[] = { &zero };
        Invoke(
            setPlayerSpeed,
            playerAnimation,
            parameters);
    }

    if (npcAnimation != nullptr &&
        npcAnimationClass != nullptr)
    {
        Invoke(
            FindMethod(
                npcAnimationClass,
                "ForceExitActionState",
                0),
            npcAnimation,
            nullptr);
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetNoAction",
                0),
            npcAnimation,
            nullptr);
        float zero = 0.0f;
        void* speedParameters[] = { &zero };
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetSpeed",
                1),
            npcAnimation,
            speedParameters);
        bool enabled = true;
        void* waitParameters[] = { &enabled };
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetWait",
                1),
            npcAnimation,
            waitParameters);
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetOverrideIdle",
                1),
            npcAnimation,
            waitParameters);
    }

    g_npcDialogueActorLockActive = true;
    NpcDialogue_FaceActorsTowardEachOther();
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"NPC dialogue locked player and NPC animations: playerRotation=%d npcRotation=%d playerSpeed=%d npcSpeed=%d.",
        g_npcDialoguePlayerRotationSaved ? 1 : 0,
        g_npcDialogueNpcRotationSaved ? 1 : 0,
        g_npcDialoguePlayerMoveSpeedSaved ? 1 : 0,
        g_npcDialogueNpcMoveSpeedSaved ? 1 : 0);
}

void NpcDialogue_HoldActorAnimationLock()
{
    if (!g_npcDialogueActorLockActive ||
        !NpcDialogue_IsAlive(g_npcDialogueTarget))
    {
        return;
    }
    NpcDialogue_FaceActorsTowardEachOther();
    auto* npcAnimation =
        NpcDialogue_GetNpcAnimationManager(g_npcDialogueTarget);
    auto* npcAnimationClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAnimationManager");
    if (npcAnimation == nullptr ||
        npcAnimationClass == nullptr)
    {
        return;
    }
    float zero = 0.0f;
    void* speedParameters[] = { &zero };
    Invoke(
        FindMethod(
            npcAnimationClass,
            "SetSpeed",
            1),
        npcAnimation,
        speedParameters);
    if (g_npcDialogueTalkAnimationId != 0)
    {
        bool disabled = false;
        void* talkParameters[] = { &disabled };
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetWait",
                1),
            npcAnimation,
            talkParameters);
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetOverrideIdle",
                1),
            npcAnimation,
            talkParameters);
        return;
    }
    bool enabled = true;
    void* waitParameters[] = { &enabled };
    Invoke(
        FindMethod(
            npcAnimationClass,
            "SetWait",
            1),
        npcAnimation,
        waitParameters);
    Invoke(
        FindMethod(
            npcAnimationClass,
            "SetOverrideIdle",
            1),
        npcAnimation,
        waitParameters);
}

void NpcDialogue_UnlockActorAnimations()
{
    if (!g_npcDialogueActorLockActive)
    {
        return;
    }

    auto* npcAnimation =
        NpcDialogue_GetNpcAnimationManager(g_npcDialogueTarget);
    auto* npcAnimationClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAnimationManager");
    if (npcAnimation != nullptr &&
        npcAnimationClass != nullptr)
    {
        bool disabled = false;
        void* waitParameters[] = { &disabled };
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetWait",
                1),
            npcAnimation,
            waitParameters);
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetOverrideIdle",
                1),
            npcAnimation,
            waitParameters);
        float speed = g_npcDialogueNpcMoveSpeedSaved
            ? g_npcDialogueSavedNpcMoveSpeed
            : 0.0f;
        void* speedParameters[] = { &speed };
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetSpeed",
                1),
            npcAnimation,
            speedParameters);
    }

    auto* playerAnimation = NpcDialogue_GetPlayerAnimationManager();
    auto* playerAnimationClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationManager");
    if (playerAnimation != nullptr &&
        playerAnimationClass != nullptr)
    {
        float speed = g_npcDialoguePlayerMoveSpeedSaved
            ? g_npcDialogueSavedPlayerBaseMoveSpeed
            : 1.0f;
        void* speedParameters[] = { &speed };
        Invoke(
            FindMethod(
                playerAnimationClass,
                "SetMoveSpeed",
                1),
            playerAnimation,
            speedParameters);
    }

    if (g_npcDialogueNpcRotationSaved)
    {
        auto* npcTransform = g_npcDialogueTarget != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_npcDialogueTarget) + 0x10)
            : nullptr;
        NpcDialogue_RestoreTransformRotation(
            npcTransform,
            g_npcDialogueSavedNpcRotation);
    }
    if (g_npcDialoguePlayerRotationSaved)
    {
        NpcDialogue_RestoreTransformRotation(
            GetWorldTransform(g_extendedPlayerGameObject),
            g_npcDialogueSavedPlayerRotation);
    }

    g_npcDialogueActorLockActive = false;
    g_npcDialoguePlayerRotationSaved = false;
    g_npcDialogueNpcRotationSaved = false;
    g_npcDialoguePlayerMoveSpeedSaved = false;
    g_npcDialogueNpcMoveSpeedSaved = false;
    LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue released player and NPC animation locks.");
}

Il2CppObject* NpcDialogue_CreateRgbaTexture(
    int width,
    int height,
    const std::vector<unsigned char>& pixels)
{
    if (width <= 0 || height <= 0 ||
        pixels.size() !=
            static_cast<size_t>(width) *
                static_cast<size_t>(height) * 4 ||
        il2cpp_object_new == nullptr ||
        il2cpp_array_new == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        return nullptr;
    }
    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* byteClass = FindClass("System", "Byte");
    if (textureClass == nullptr || byteClass == nullptr)
    {
        return nullptr;
    }
    il2cpp_runtime_class_init(textureClass);
    auto* texture = il2cpp_object_new(textureClass);
    int textureFormat = 4; // TextureFormat.RGBA32
    bool mipChain = false;
    void* constructorParameters[] = {
        &width,
        &height,
        &textureFormat,
        &mipChain,
    };
    Invoke(
        FindMethod(textureClass, ".ctor", 4),
        texture,
        constructorParameters);
    auto* byteArray = il2cpp_array_new(
        byteClass,
        pixels.size());
    if (texture == nullptr || byteArray == nullptr)
    {
        return nullptr;
    }
    std::memcpy(
        reinterpret_cast<char*>(byteArray) +
            il2cpp_array_object_header_size(),
        pixels.data(),
        pixels.size());
    void* pixelParameters[] = { byteArray };
    Invoke(
        FindMethod(textureClass, "LoadRawTextureData", 1),
        texture,
        pixelParameters);
    Invoke(
        FindMethod(textureClass, "Apply", 0),
        texture,
        nullptr);
    return texture;
}

void NpcDialogue_SetQtePointerLayout()
{
    auto* rect = g_npcDialogueQtePointerShape.rect;
    auto* rectClass = FindClass("UnityEngine", "RectTransform");
    if (rect == nullptr || rectClass == nullptr)
    {
        return;
    }
    float anchor[] = { 0.818f, 0.575f };
    float size[] = { 34.0f, 150.0f };
    float pivot[] = { 0.5f, 0.0f };
    float position[] = { 0.0f, 0.0f };
    void* anchorParameters[] = { anchor };
    void* sizeParameters[] = { size };
    void* pivotParameters[] = { pivot };
    void* positionParameters[] = { position };
    Invoke(
        FindMethod(rectClass, "set_anchorMin", 1),
        rect,
        anchorParameters);
    Invoke(
        FindMethod(rectClass, "set_anchorMax", 1),
        rect,
        anchorParameters);
    Invoke(
        FindMethod(rectClass, "set_sizeDelta", 1),
        rect,
        sizeParameters);
    Invoke(
        FindMethod(rectClass, "set_pivot", 1),
        rect,
        pivotParameters);
    Invoke(
        FindMethod(rectClass, "set_anchoredPosition", 1),
        rect,
        positionParameters);
}

void NpcDialogue_SetRectLocalEulerZ(
    Il2CppObject* rect,
    float angle)
{
    float euler[] = { 0.0f, 0.0f, angle };
    void* parameters[] = { euler };
    Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "set_localEulerAngles",
            1),
        rect,
        parameters);
}

void NpcDialogue_SetQteVisualOffset(float x, float y)
{
    auto* rectClass = FindClass(
        "UnityEngine",
        "RectTransform");
    auto* setPosition = FindMethod(
        rectClass,
        "set_anchoredPosition",
        1);
    if (setPosition == nullptr)
    {
        return;
    }
    float position[] = { x, y };
    void* parameters[] = { position };
    void** parameterPointer = parameters;
    const auto apply =
        [setPosition, parameterPointer](Il2CppObject* rect)
        {
            if (rect != nullptr)
            {
                Invoke(
                    setPosition,
                    rect,
                    parameterPointer);
            }
        };
    apply(g_npcDialogueQteBackdrop.rect);
    apply(g_npcDialogueQteButton.rect);
    apply(g_npcDialogueQtePointerShape.rect);
    for (auto& segment : g_npcDialogueQteSegments)
    {
        apply(segment.rect);
    }
    apply(g_npcDialogueQteFeedbackOverlay.rect);
}

void NpcDialogue_StartQteMissFeedback()
{
    g_npcDialogueQteMissFeedbackEndsAt =
        GetTickCount64() + 400;
    SetWorldGameObjectActive(
        g_npcDialogueQteFeedbackOverlay.gameObject,
        true);
    BringSceneObjectToFront(
        g_npcDialogueQteFeedbackOverlay.gameObject);
}

void NpcDialogue_UpdateQteMissFeedback(
    unsigned long long now)
{
    if (g_npcDialogueQteMissFeedbackEndsAt == 0)
    {
        return;
    }
    if (now >= g_npcDialogueQteMissFeedbackEndsAt)
    {
        g_npcDialogueQteMissFeedbackEndsAt = 0;
        SetWorldGameObjectActive(
            g_npcDialogueQteFeedbackOverlay.gameObject,
            false);
        NpcDialogue_SetQteVisualOffset(0.0f, 0.0f);
        return;
    }
    constexpr unsigned long long duration = 400;
    const float progress =
        static_cast<float>(
            duration -
            (g_npcDialogueQteMissFeedbackEndsAt - now)) /
        static_cast<float>(duration);
    const float pulse = std::abs(
        std::sin(
            progress * 12.566370614359172f));
    SetSceneGraphicColor(
        g_npcDialogueQteFeedbackOverlay.image,
        1.0f,
        0.10f,
        0.10f,
        0.18f + pulse * 0.32f);
    const float amplitude =
        8.0f * (1.0f - progress);
    const float offsetX =
        std::sin(
            progress * 37.69911184307752f) *
        amplitude;
    const float offsetY =
        std::cos(
            progress * 31.41592653589793f) *
        amplitude * 0.6f;
    NpcDialogue_SetQteVisualOffset(
        offsetX,
        offsetY);
}

void NpcDialogue_EnsureQteVisuals()
{
    if (!g_npcDialogueQteSegments.empty())
    {
        return;
    }
    auto* rootTransform = GetWorldTransform(
        g_npcDialogueRoot);
    if (rootTransform == nullptr)
    {
        return;
    }
    constexpr int segmentCount = 90;
    constexpr float centerX = 0.818f;
    constexpr float centerY = 0.575f;
    constexpr float radiusX = 0.058f;
    const float radiusY =
        radiusX * (2752.0f / 1152.0f);
    g_npcDialogueQteSegments.reserve(segmentCount);
    for (int index = 0; index < segmentCount; ++index)
    {
        const float centerAngle =
            (static_cast<float>(index) + 0.5f) * 2.0f;
        const float radians =
            centerAngle * 0.01745329251994329577f;
        const float x =
            centerX + std::cos(radians) * radiusX;
        const float y =
            centerY + std::sin(radians) * radiusY;
        char name[64]{};
        std::snprintf(
            name,
            sizeof(name),
            "EFM_QteArc%02d",
            index);
        auto segment = CreateScenePanel(
            name,
            rootTransform,
            x - 0.0022f,
            y - 0.0130f,
            x + 0.0022f,
            y + 0.0130f,
            0.14f,
            0.18f,
            0.28f,
            0.88f);
        NpcDialogue_SetRectLocalEulerZ(
            segment.rect,
            centerAngle - 90.0f);
        SetWorldGameObjectActive(
            segment.gameObject,
            g_npcDialogueEnhancedMode);
        g_npcDialogueQteSegments.push_back(segment);
    }
    g_npcDialogueQtePointerShape =
        CreateScenePanel(
            "EFM_NpcDialogueQtePointer",
            GetWorldTransform(g_npcDialogueRoot),
            0.0f,
            0.0f,
            0.001f,
            0.001f,
            1.0f,
            0.48f,
            0.72f,
            0.0f);
    NpcDialogue_SetQtePointerLayout();
    constexpr int stripCount = 48;
    g_npcDialogueQtePointerStrips.reserve(stripCount);
    for (int index = 0; index < stripCount; ++index)
    {
        const float y0 =
            static_cast<float>(index) /
            static_cast<float>(stripCount);
        const float y1 =
            static_cast<float>(index + 1) /
            static_cast<float>(stripCount);
        const float halfWidth =
            0.5f * (1.0f - y0);
        auto strip = CreateScenePanel(
            "EFM_NpcDialogueQtePointerStrip",
            g_npcDialogueQtePointerShape.rect,
            0.5f - halfWidth,
            y0,
            0.5f + halfWidth,
            y1,
            1.0f,
            0.48f,
            0.72f,
            1.0f);
        g_npcDialogueQtePointerStrips.push_back(strip);
    }
    SetWorldGameObjectActive(
        g_npcDialogueQtePointerShape.gameObject,
        g_npcDialogueEnhancedMode);
    for (auto& segment : g_npcDialogueQteSegments)
    {
        BringSceneObjectToFront(segment.gameObject);
    }
    BringSceneObjectToFront(
        g_npcDialogueQtePointerShape.gameObject);
}

void NpcDialogue_UpdateQteZoneVisuals()
{
    if (g_npcDialogueQteSegments.empty())
    {
        return;
    }
    for (size_t index = 0;
         index < g_npcDialogueQteSegments.size();
         ++index)
    {
        const unsigned long long now = GetTickCount64();
        const float angle =
            (static_cast<float>(index) + 0.5f) * 2.0f;
        float red = 0.14f;
        float green = 0.18f;
        float blue = 0.28f;
        float alpha = 0.88f;
        for (int zoneIndex = 0;
             zoneIndex < 3;
             ++zoneIndex)
        {
            const auto& zone =
                g_npcDialogueQteZones[zoneIndex];
            if (angle < zone.startDegrees ||
                angle > zone.endDegrees)
            {
                continue;
            }
            if (!zone.active &&
                (g_npcDialogueQteZoneFadeEndsAt[zoneIndex] == 0 ||
                 now >= g_npcDialogueQteZoneFadeEndsAt[zoneIndex]))
            {
                continue;
            }
            if (zoneIndex == 0)
            {
                red = 1.0f; green = 0.70f; blue = 0.12f;
            }
            else if (zoneIndex == 1)
            {
                red = 1.0f; green = 0.28f; blue = 0.52f;
            }
            else
            {
                red = 0.24f; green = 0.78f; blue = 1.0f;
            }
            if (zone.active)
            {
                alpha = 0.96f;
            }
            else
            {
                const unsigned long long started =
                    g_npcDialogueQteZoneFadeStartedAt[zoneIndex];
                const unsigned long long ends =
                    g_npcDialogueQteZoneFadeEndsAt[zoneIndex];
                const float progress = ends > started
                    ? static_cast<float>(now - started) /
                        static_cast<float>(ends - started)
                    : 1.0f;
                alpha = 0.96f *
                    (1.0f - std::clamp(progress, 0.0f, 1.0f));
            }
            break;
        }
        SetSceneGraphicColor(
            g_npcDialogueQteSegments[index].image,
            red,
            green,
            blue,
            alpha);
    }
}

void NpcDialogue_UpdateQteWheelTexture()
{
    constexpr int width = 512;
    constexpr int height = 256;
    std::vector<unsigned char> pixels(
        static_cast<size_t>(width) * height * 4,
        0);
    const float centerX = width * 0.5f;
    const float centerY = 1.0f;
    constexpr float outerRadius = 246.0f;
    constexpr float innerRadius = 176.0f;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const float dx = static_cast<float>(x) - centerX;
            const float dy = static_cast<float>(y) - centerY;
            const float radius =
                std::sqrt(dx * dx + dy * dy);
            if (radius < innerRadius ||
                radius > outerRadius)
            {
                continue;
            }
            float angle =
                std::atan2(dy, dx) *
                57.29577951308232f;
            if (angle < 0.0f || angle > 180.0f)
            {
                continue;
            }
            unsigned char red = 42;
            unsigned char green = 55;
            unsigned char blue = 82;
            unsigned char alpha = 190;
            for (int index = 0; index < 3; ++index)
            {
                const auto& zone = g_npcDialogueQteZones[index];
                if (!zone.active ||
                    angle < zone.startDegrees ||
                    angle > zone.endDegrees)
                {
                    continue;
                }
                if (index == 0)
                {
                    red = 255; green = 184; blue = 46;
                }
                else if (index == 1)
                {
                    red = 255; green = 82; blue = 132;
                }
                else
                {
                    red = 74; green = 210; blue = 255;
                }
                alpha = 225;
                break;
            }
            const size_t offset =
                (static_cast<size_t>(y) * width + x) * 4;
            pixels[offset + 0] = red;
            pixels[offset + 1] = green;
            pixels[offset + 2] = blue;
            pixels[offset + 3] = alpha;
        }
    }
    if (g_npcDialogueQteWheelTexture != nullptr)
    {
        ExtendedMode_DestroyUnityObject(
            g_npcDialogueQteWheelTexture);
        g_npcDialogueQteWheelTexture = nullptr;
    }
    g_npcDialogueQteWheelTexture =
        NpcDialogue_CreateRgbaTexture(
            width,
            height,
            pixels);
    SetSceneRawImageTexture(
        g_npcDialogueQteWheel.rawImage,
        g_npcDialogueQteWheelTexture);
}

void NpcDialogue_CreateQtePointerTexture()
{
    if (g_npcDialogueQtePointerTexture != nullptr)
    {
        return;
    }
    constexpr int width = 128;
    constexpr int height = 128;
    std::vector<unsigned char> pixels(
        static_cast<size_t>(width) * height * 4,
        0);
    for (int y = 0; y < height; ++y)
    {
        const float halfWidth =
            57.0f *
            (1.0f -
             static_cast<float>(y) /
                 static_cast<float>(height - 1));
        const int minX = static_cast<int>(
            width * 0.5f - halfWidth);
        const int maxX = static_cast<int>(
            width * 0.5f + halfWidth);
        for (int x = std::max(0, minX);
             x <= std::min(width - 1, maxX);
             ++x)
        {
            const size_t offset =
                (static_cast<size_t>(y) * width + x) * 4;
            pixels[offset + 0] = 255;
            pixels[offset + 1] = 255;
            pixels[offset + 2] = 255;
            pixels[offset + 3] = 255;
        }
    }
    g_npcDialogueQtePointerTexture =
        NpcDialogue_CreateRgbaTexture(
            width,
            height,
            pixels);
    SetSceneRawImageTexture(
        g_npcDialogueQtePointer.rawImage,
        g_npcDialogueQtePointerTexture);
    NpcDialogue_SetQtePointerLayout();
}

void NpcDialogue_GenerateQteZones()
{
    static std::mt19937 generator(
        static_cast<unsigned int>(
            std::random_device{}() ^ GetTickCount64()));
    std::uniform_int_distribution<int> widthDistribution(20, 40);
    std::uniform_int_distribution<int> gapChoice(0, 3);
    int widths[3]{};
    int totalWidth = 0;
    for (int index = 0; index < 3; ++index)
    {
        widths[index] =
            widthDistribution(generator);
        totalWidth += widths[index];
    }
    int gaps[4]{};
    int remaining =
        std::max(0, 180 - totalWidth);
    for (int index = 0; index < remaining; ++index)
    {
        ++gaps[gapChoice(generator)];
    }
    float cursor = static_cast<float>(gaps[0]);
    for (int index = 0; index < 3; ++index)
    {
        g_npcDialogueQteZones[index].startDegrees = cursor;
        g_npcDialogueQteZones[index].endDegrees =
            cursor + static_cast<float>(widths[index]);
        g_npcDialogueQteZones[index].active = true;
        g_npcDialogueQteZoneFadeStartedAt[index] = 0;
        g_npcDialogueQteZoneFadeEndsAt[index] = 0;
        cursor =
            g_npcDialogueQteZones[index].endDegrees +
            static_cast<float>(gaps[index + 1]);
    }
    g_npcDialogueQteRefillAt = 0;
    NpcDialogue_UpdateQteZoneVisuals();
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"QTE generated zones %.1f-%.1f, %.1f-%.1f, %.1f-%.1f.",
        g_npcDialogueQteZones[0].startDegrees,
        g_npcDialogueQteZones[0].endDegrees,
        g_npcDialogueQteZones[1].startDegrees,
        g_npcDialogueQteZones[1].endDegrees,
        g_npcDialogueQteZones[2].startDegrees,
        g_npcDialogueQteZones[2].endDegrees);
}

int NpcDialogue_ActiveQteZoneCount()
{
    int count = 0;
    for (const auto& zone : g_npcDialogueQteZones)
    {
        count += zone.active ? 1 : 0;
    }
    return count;
}

bool NpcDialogue_IsQteButtonEnabled(unsigned long long now)
{
    return
        g_npcDialogueEnhancedMode &&
        g_npcDialogueQteInitialized &&
        !g_npcDialogueClimaxActive &&
        g_npcDialogueStage != 0 &&
        NpcDialogue_ActiveQteZoneCount() > 0 &&
        now >= g_npcDialogueQteCooldownUntil &&
        (g_npcDialogueQteRefillAt == 0 ||
         now >= g_npcDialogueQteRefillAt);
}

void NpcDialogue_UpdateQteButton()
{
    const bool enabled =
        NpcDialogue_IsQteButtonEnabled(GetTickCount64());
    SetSceneGraphicColor(
        g_npcDialogueQteButton.image,
        enabled ? 0.12f : 0.18f,
        enabled ? 0.48f : 0.21f,
        enabled ? 0.72f : 0.25f,
        0.96f);
    SetSceneText(
        g_npcDialogueQteButtonText.text,
        enabled
            ? "QTE"
            : (g_npcDialogueClimaxActive
                ? "已高潮"
                : "QTE 锁定"),
        15.0f,
        514,
        1.0f,
        1.0f,
        1.0f,
        enabled ? 1.0f : 0.62f);
}

void NpcDialogue_TriggerQteSettlementAnimation()
{
    auto* animationManager =
        NpcDialogue_GetPlayerAnimationManager();
    auto* animator = animationManager != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(animationManager) + 0x30)
        : nullptr;
    if (animator == nullptr)
    {
        animator = g_previewAnimator;
    }
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* play = FindMethod(animatorClass, "Play", 3);
    if (animator == nullptr || play == nullptr)
    {
        g_npcDialogueQteCooldownUntil =
            GetTickCount64() + 500;
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"QTE reaction skipped because the player Animator is unavailable.");
        return;
    }

    // This is the shipped one-second reaction state used when Baibu strength
    // changes. Play() always restarts it from the beginning, so rapid QTE
    // settlements cannot collapse into one IsGamanBaibu boolean transition.
    auto* stateName = il2cpp_string_new(
        "Locomotion.GamanBaibuBikkuri");
    int layer = 0;
    float normalizedTime = 0.0f;
    void* playParameters[] = {
        stateName,
        &layer,
        &normalizedTime,
    };
    Invoke(play, animator, playParameters);
    g_npcDialogueQteCooldownUntil =
        GetTickCount64() + 500;
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"QTE settlement restarted Locomotion.GamanBaibuBikkuri animator=%p.",
        animator);
}

void NpcDialogue_PlayExcitementAudio(int category)
{
    if (MoaningAudio_PlayCategory(category, false))
    {
        g_npcDialogueAudioActive = true;
        LogInfo(LogCategory_Dialogue, L"Dialogue", 
            L"NPC dialogue played %S audio for excitement %.1f.",
            kMoaningAudioCategoryNames[category],
            g_npcDialogueExcitementSnapshot);
    }
}

void NpcDialogue_TriggerClimaxAnimation()
{
    if (g_npcDialogueClimaxActive)
    {
        return;
    }
    g_npcDialogueClimaxActive = true;

    auto* animationManager =
        NpcDialogue_GetPlayerAnimationManager();
    auto* animationManagerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationManager");
    auto* animator = animationManager != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(animationManager) + 0x30)
        : nullptr;
    if (animator == nullptr)
    {
        animator = g_previewAnimator;
    }
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* play = FindMethod(animatorClass, "Play", 3);
    if (animationManager != nullptr &&
        animationManagerClass != nullptr)
    {
        bool ecstasy = true;
        void* ecstasyParameters[] = { &ecstasy };
        Invoke(
            FindMethod(
                animationManagerClass,
                "SetEcstasy",
                1),
            animationManager,
            ecstasyParameters);
    }
    if (animator != nullptr && play != nullptr)
    {
        auto* stateName =
            il2cpp_string_new("Action.IkuStand");
        int layer = 1;
        float normalizedTime = 0.0f;
        void* playParameters[] = {
            stateName,
            &layer,
            &normalizedTime,
        };
        Invoke(play, animator, playParameters);
        LogInfo(LogCategory_Dialogue, L"Dialogue", 
            L"QTE excitement reached 100; started Action.IkuStand animator=%p.",
            animator);
        g_npcDialogueClimaxAnimator = animator;
        g_npcDialogueClimaxAnimationStartedAt =
            GetTickCount64();
    }
    else
    {
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"QTE climax animation skipped because the player Animator is unavailable.");
    }
    NpcDialogue_PlayExcitementAudio(5);
    NpcDialogue_UpdateQteButton();
    NpcDialogue_UpdateContent();
}

bool NpcDialogue_ClimaxAnimationFinished(unsigned long long now)
{
    if (!g_npcDialogueClimaxActive ||
        g_npcDialogueClimaxAnimationStartedAt == 0)
    {
        return false;
    }
    const unsigned long long elapsed =
        now - g_npcDialogueClimaxAnimationStartedAt;
    if (elapsed >= 1250)
    {
        return true;
    }

    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* getState = FindMethod(
        animatorClass,
        "GetCurrentAnimatorStateInfo",
        1);
    auto* hashCode = FindMethod(
        animatorClass,
        "StringToHash",
        1);
    if (g_npcDialogueClimaxAnimator == nullptr ||
        getState == nullptr ||
        hashCode == nullptr ||
        il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    static int expectedHash = 0;
    if (expectedHash == 0)
    {
        auto* stateName =
            il2cpp_string_new("Action.IkuStand");
        void* hashParameters[] = { stateName };
        auto* hashValue = Invoke(
            hashCode,
            nullptr,
            hashParameters);
        if (hashValue == nullptr)
        {
            return false;
        }
        expectedHash =
            *static_cast<int*>(
                il2cpp_object_unbox(hashValue));
    }
    int layer = 1;
    void* stateParameters[] = { &layer };
    auto* stateValue = Invoke(
        getState,
        g_npcDialogueClimaxAnimator,
        stateParameters);
    if (stateValue == nullptr)
    {
        return false;
    }
    const auto* state = static_cast<const unsigned char*>(
        il2cpp_object_unbox(stateValue));
    const int currentHash =
        *reinterpret_cast<const int*>(state + 0x08);
    const float normalizedTime =
        *reinterpret_cast<const float*>(state + 0x0C);
    return currentHash == expectedHash &&
           normalizedTime >= 0.99f;
}

void NpcDialogue_SettleQte(int gain, const wchar_t* reason)
{
    if (gain <= 0)
    {
        return;
    }
    const float previous = g_npcDialogueExcitementSnapshot;
    g_npcDialogueExcitementSnapshot = std::clamp(
        previous + static_cast<float>(gain),
        0.0f,
        100.0f);
    const bool reachedClimax =
        previous < 100.0f &&
        g_npcDialogueExcitementSnapshot >= 100.0f;
    g_npcDialogueExcitementAnimationStart =
        g_npcDialogueExcitementDisplayed;
    g_npcDialogueExcitementAnimationStartedAt =
        GetTickCount64();
    NpcDialogue_UpdateEnhancedBars();
    if (reachedClimax)
    {
        NpcDialogue_TriggerClimaxAnimation();
    }
    else
    {
        const int audioCategory =
            g_npcDialogueExcitementSnapshot > 75.0f
                ? 4
                : (g_npcDialogueExcitementSnapshot > 50.0f
                    ? 3
                    : (g_npcDialogueExcitementSnapshot > 25.0f
                        ? 2
                        : (g_npcDialogueExcitementSnapshot > 0.0f
                            ? 1
                            : -1)));
        if (audioCategory >= 0)
        {
            NpcDialogue_PlayExcitementAudio(audioCategory);
        }
        NpcDialogue_TriggerQteSettlementAnimation();
    }
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"QTE settlement %s: gain=%d excitement=%.1f.",
        reason != nullptr ? reason : L"unknown",
        gain,
        g_npcDialogueExcitementSnapshot);
}

void NpcDialogue_HandleQteClick()
{
    const unsigned long long now = GetTickCount64();
    if (!NpcDialogue_IsQteButtonEnabled(now))
    {
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"QTE click ignored because the button is locked.");
        return;
    }
    for (int zoneIndex = 0;
         zoneIndex < 3;
         ++zoneIndex)
    {
        auto& zone = g_npcDialogueQteZones[zoneIndex];
        if (!zone.active ||
            g_npcDialogueQteAngle < zone.startDegrees ||
            g_npcDialogueQteAngle > zone.endDegrees)
        {
            continue;
        }
        zone.active = false;
        g_npcDialogueQteZoneFadeStartedAt[zoneIndex] = now;
        g_npcDialogueQteZoneFadeEndsAt[zoneIndex] =
            now + 350;
        NpcDialogue_UpdateQteZoneVisuals();
        const int remaining = NpcDialogue_ActiveQteZoneCount();
        if (remaining == 0)
        {
            g_npcDialogueQteRefillAt = 0;
            LogInfo(LogCategory_Dialogue, L"Dialogue", L"QTE is empty and waits for the pointer at zero to refill.");
        }
        NpcDialogue_UpdateQteButton();
        LogInfo(LogCategory_Dialogue, L"Dialogue", 
            L"QTE hit at %.1f degrees; remaining zones=%d.",
            g_npcDialogueQteAngle,
            remaining);
        return;
    }
    NpcDialogue_StartQteMissFeedback();
    NpcDialogue_SettleQte(10, L"miss");
    NpcDialogue_UpdateQteButton();
}

void NpcDialogue_UpdateQte()
{
    if (!g_npcDialogueEnhancedMode ||
        !g_npcDialogueQteInitialized ||
        g_npcDialogueClimaxActive)
    {
        return;
    }
    const unsigned long long now = GetTickCount64();
    const unsigned long long elapsed =
        now - g_npcDialogueQteCycleStartedAt;
    const unsigned long long phase = elapsed % 6000;
    g_npcDialogueQteAngle = phase <= 3000
        ? static_cast<float>(phase) / 3000.0f * 180.0f
        : (6000.0f - static_cast<float>(phase)) /
            3000.0f * 180.0f;
    if (g_npcDialogueQteInitialized &&
        phase < g_npcDialogueQtePreviousPhase)
    {
        const int remaining =
            NpcDialogue_ActiveQteZoneCount();
        if (remaining > 0)
        {
            NpcDialogue_SettleQte(
                remaining * 10,
                L"pointer returned to zero");
        }
        else
        {
            NpcDialogue_GenerateQteZones();
            LogInfo(LogCategory_Dialogue, L"Dialogue", L"QTE refilled at zero with an empty field.");
        }
    }
    g_npcDialogueQtePreviousPhase = phase;

    NpcDialogue_SetRectLocalEulerZ(
        g_npcDialogueQtePointerShape.rect,
        g_npcDialogueQteAngle - 90.0f);
    NpcDialogue_UpdateQteZoneVisuals();
    NpcDialogue_UpdateQteMissFeedback(now);
    NpcDialogue_UpdateQteButton();
}

void NpcDialogue_InitializeQte()
{
    g_npcDialogueQteInitialized = true;
    g_npcDialogueQteCycleStartedAt = GetTickCount64();
    g_npcDialogueQtePreviousPhase = 0;
    g_npcDialogueQteRefillAt = 0;
    g_npcDialogueQteCooldownUntil = 0;
    g_npcDialogueQteAngle = 0.0f;
    NpcDialogue_EnsureQteVisuals();
    NpcDialogue_GenerateQteZones();
    LogInfo(LogCategory_Dialogue, L"Dialogue", L"QTE arc and judgment zones are ready.");
    NpcDialogue_UpdateQte();
    LogInfo(LogCategory_Dialogue, L"Dialogue", L"QTE initialized with pointer starting at zero degrees.");
}

void NpcDialogue_SetEnhancedBarsVisible(bool visible)
{
    SetWorldGameObjectActive(
        g_npcDialoguePatienceLabel.gameObject,
        visible);
    SetWorldGameObjectActive(
        g_npcDialogueExcitementLabel.gameObject,
        visible);
    SetWorldGameObjectActive(
        g_npcDialoguePatienceTrack.gameObject,
        visible);
    SetWorldGameObjectActive(
        g_npcDialogueExcitementTrack.gameObject,
        visible);
    SetWorldGameObjectActive(
        g_npcDialoguePatienceFill.gameObject,
        visible && g_npcDialoguePatience > 0.0f);
    SetWorldGameObjectActive(
        g_npcDialogueExcitementFill.gameObject,
        visible && g_npcDialogueExcitementSnapshot > 0.0f);
    SetWorldGameObjectActive(
        g_npcDialogueQteBackdrop.gameObject,
        visible);
    SetWorldGameObjectActive(
        g_npcDialogueQteWheel.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueQtePointer.gameObject,
        false);
    SetWorldGameObjectActive(
        g_npcDialogueQteButton.gameObject,
        visible);
    SetWorldGameObjectActive(
        g_npcDialogueQteFeedbackOverlay.gameObject,
        false);
    for (auto& segment : g_npcDialogueQteSegments)
    {
        SetWorldGameObjectActive(
            segment.gameObject,
            visible);
    }
    SetWorldGameObjectActive(
        g_npcDialogueQtePointerShape.gameObject,
        visible);
}

void NpcDialogue_UpdateEnhancedBars()
{
    if (!g_npcDialogueEnhancedMode)
    {
        NpcDialogue_SetEnhancedBarsVisible(false);
        return;
    }
    const float patienceRatio = std::clamp(
        g_npcDialoguePatience /
            kNpcDialoguePatienceMax,
        0.0f,
        1.0f);
    const unsigned long long now = GetTickCount64();
    if (g_npcDialogueExcitementAnimationStartedAt == 0)
    {
        g_npcDialogueExcitementDisplayed =
            g_npcDialogueExcitementSnapshot;
    }
    else
    {
        const float progress = std::clamp(
            static_cast<float>(
                now -
                g_npcDialogueExcitementAnimationStartedAt) /
                (kNpcDialogueExcitementAnimationSeconds * 1000.0f),
            0.0f,
            1.0f);
        const float inverse = 1.0f - progress;
        const float eased =
            1.0f - inverse * inverse * inverse;
        g_npcDialogueExcitementDisplayed =
            g_npcDialogueExcitementAnimationStart +
            (g_npcDialogueExcitementSnapshot -
             g_npcDialogueExcitementAnimationStart) *
                eased;
        if (progress >= 1.0f)
        {
            g_npcDialogueExcitementDisplayed =
                g_npcDialogueExcitementSnapshot;
            g_npcDialogueExcitementAnimationStartedAt = 0;
        }
    }
    const float excitementRatio = std::clamp(
        g_npcDialogueExcitementDisplayed / 100.0f,
        0.0f,
        1.0f);
    SetSceneRect(
        g_npcDialoguePatienceFill.rect,
        0.036f,
        kNpcDialogueBarMinY,
        0.060f,
        kNpcDialogueBarMinY +
            (kNpcDialogueBarMaxY -
             kNpcDialogueBarMinY) * patienceRatio);
    SetSceneRect(
        g_npcDialogueExcitementFill.rect,
        0.692f,
        kNpcDialogueBarMinY,
        0.716f,
        kNpcDialogueBarMinY +
            (kNpcDialogueBarMaxY -
             kNpcDialogueBarMinY) * excitementRatio);
    SetWorldGameObjectActive(
        g_npcDialoguePatienceFill.gameObject,
        patienceRatio > 0.0f);
    SetWorldGameObjectActive(
        g_npcDialogueExcitementFill.gameObject,
        excitementRatio > 0.0f);
    BringSceneObjectToFront(g_npcDialoguePatienceTrack.gameObject);
    BringSceneObjectToFront(g_npcDialoguePatienceFill.gameObject);
    BringSceneObjectToFront(g_npcDialogueExcitementTrack.gameObject);
    BringSceneObjectToFront(g_npcDialogueExcitementFill.gameObject);
    BringSceneObjectToFront(g_npcDialogueQteBackdrop.gameObject);
    BringSceneObjectToFront(g_npcDialogueQteButton.gameObject);
    for (auto& segment : g_npcDialogueQteSegments)
    {
        BringSceneObjectToFront(segment.gameObject);
    }
    BringSceneObjectToFront(
        g_npcDialogueQtePointerShape.gameObject);
    BringSceneObjectToFront(g_npcDialoguePatienceLabel.gameObject);
    BringSceneObjectToFront(g_npcDialogueExcitementLabel.gameObject);
}

int NpcDialogue_NextChoiceDelayMilliseconds()
{
    static std::mt19937 generator(
        static_cast<unsigned int>(
            std::random_device{}() ^ GetTickCount64()));
    std::uniform_int_distribution<int> distribution(3000, 10000);
    return distribution(generator);
}

void NpcDialogue_FinishNpcTalkAnimation()
{
    if (g_npcDialogueTalkAnimationId == 0)
    {
        return;
    }
    auto* npcAnimation =
        NpcDialogue_GetNpcAnimationManager(g_npcDialogueTarget);
    auto* npcAnimationClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAnimationManager");
    if (npcAnimation != nullptr &&
        npcAnimationClass != nullptr)
    {
        Invoke(
            FindMethod(
                npcAnimationClass,
                "ForceExitActionState",
                0),
            npcAnimation,
            nullptr);
        Invoke(
            FindMethod(
                npcAnimationClass,
                "SetNoAction",
                0),
            npcAnimation,
            nullptr);
    }
    g_npcDialogueTalkAnimationId = 0;
    g_npcDialogueTalkAnimationEndsAt = 0;
    LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue finished the two-second NPC talk animation.");
}

void NpcDialogue_PlayNpcTalkAnimation()
{
    auto* npcAnimation =
        NpcDialogue_GetNpcAnimationManager(g_npcDialogueTarget);
    auto* npcAnimationClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAnimationManager");
    if (npcAnimation == nullptr ||
        npcAnimationClass == nullptr)
    {
        return;
    }
    g_npcDialogueTalkAnimationId =
        g_npcDialogueTalkAnimationId == 1 ? 2 : 1;
    int talkId = g_npcDialogueTalkAnimationId;
    void* parameters[] = { &talkId };
    Invoke(
        FindMethod(
            npcAnimationClass,
            "SetNpcTalk",
            1),
        npcAnimation,
        parameters);
    bool disabled = false;
    void* boolParameters[] = { &disabled };
    Invoke(
        FindMethod(
            npcAnimationClass,
            "SetWait",
            1),
        npcAnimation,
        boolParameters);
    Invoke(
        FindMethod(
            npcAnimationClass,
            "SetOverrideIdle",
            1),
        npcAnimation,
        boolParameters);
    g_npcDialogueTalkAnimationEndsAt =
        GetTickCount64() + 2000;
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"NPC dialogue started NPC talk animation %d for 2000 ms.",
        talkId);
}

void NpcDialogue_ResetEnhancedDialogue()
{
    ExtendedMode_SetNpcInteractionPrompt(false, "");
    const bool keepClimaxAudio =
        g_npcDialogueClimaxKeepAudioAfterClose;
    NpcDialogue_SetEnhancedBarsVisible(false);
    NpcDialogue_SetQteVisualOffset(0.0f, 0.0f);
    if (g_npcDialogueQteWheelTexture != nullptr)
    {
        ExtendedMode_DestroyUnityObject(
            g_npcDialogueQteWheelTexture);
        g_npcDialogueQteWheelTexture = nullptr;
    }
    if (g_npcDialogueQtePointerTexture != nullptr)
    {
        ExtendedMode_DestroyUnityObject(
            g_npcDialogueQtePointerTexture);
        g_npcDialogueQtePointerTexture = nullptr;
    }
    g_npcDialogueEnhancedMode = false;
    if (g_npcDialogueClimaxActive)
    {
        auto* animationManager =
            NpcDialogue_GetPlayerAnimationManager();
        auto* animationManagerClass = FindClass(
            "ExposureUnnoticed2.Object3D.Player.Scripts",
            "PlayerAnimationManager");
        if (animationManager != nullptr &&
            animationManagerClass != nullptr)
        {
            bool ecstasy = false;
            void* parameters[] = { &ecstasy };
            Invoke(
                FindMethod(
                    animationManagerClass,
                    "SetEcstasy",
                    1),
                animationManager,
                parameters);
        }
        ExtendedMode_PlayOriginalPose(-1);
    }
    g_npcDialogueClimaxActive = false;
    g_npcDialogueClimaxKeepAudioAfterClose = false;
    g_npcDialogueClimaxAnimator = nullptr;
    g_npcDialogueClimaxAnimationStartedAt = 0;
    if (g_npcDialogueAudioActive && !keepClimaxAudio)
    {
        MoaningAudio_Stop();
    }
    g_npcDialogueAudioActive = false;
    g_npcDialogueQteInitialized = false;
    g_npcDialogueCloseAfterTalk = false;
    g_npcDialoguePatience = kNpcDialoguePatienceMax;
    g_npcDialogueExcitementSnapshot = 0.0f;
    g_npcDialogueExcitementDisplayed = 0.0f;
    g_npcDialogueExcitementAnimationStart = 0.0f;
    g_npcDialogueExcitementAnimationStartedAt = 0;
    g_npcDialogueReplyIndex = 0;
    g_npcDialogueTalkAnimationId = 0;
    g_npcDialogueLastPatienceTick = 0;
    g_npcDialogueNextChoicesAt = 0;
    g_npcDialogueTalkAnimationEndsAt = 0;
    g_npcDialogueQteAngle = 0.0f;
    g_npcDialogueQteCycleStartedAt = 0;
    g_npcDialogueQtePreviousPhase = 0;
    g_npcDialogueQteRefillAt = 0;
    g_npcDialogueQteCooldownUntil = 0;
    for (auto& zone : g_npcDialogueQteZones)
    {
        zone = NpcDialogueQteZone{};
    }
    for (int index = 0; index < 3; ++index)
    {
        g_npcDialogueQteZoneFadeStartedAt[index] = 0;
        g_npcDialogueQteZoneFadeEndsAt[index] = 0;
    }
    g_npcDialogueQteMissFeedbackEndsAt = 0;
}

void NpcDialogue_InitializeEnhancedDialogue()
{
    NpcDialogue_ResetEnhancedDialogue();
    if (!GameplayStatusHud_IsVibratorStrong())
    {
        return;
    }
    g_npcDialogueEnhancedMode = true;
    g_npcDialoguePatience = kNpcDialoguePatienceMax;
    g_npcDialogueExcitementSnapshot = 0.0f;
    g_npcDialogueLastPatienceTick = GetTickCount64();
    NpcDialogue_SetEnhancedBarsVisible(true);
    NpcDialogue_UpdateEnhancedBars();
    NpcDialogue_InitializeQte();
    auto* activeSelfMethod = FindMethod(
        FindClass("UnityEngine", "GameObject"),
        "get_activeSelf",
        0);
    const auto readActive =
        [activeSelfMethod](Il2CppObject* gameObject)
        {
            auto* value =
                gameObject != nullptr &&
                activeSelfMethod != nullptr
                    ? Invoke(
                        activeSelfMethod,
                        gameObject,
                        nullptr)
                    : nullptr;
            return value != nullptr &&
                   il2cpp_object_unbox != nullptr &&
                   *static_cast<bool*>(
                       il2cpp_object_unbox(value));
        };
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"QTE UI state: backdrop=%p/%d wheel=%p/%d pointer=%p/%d button=%p/%d.",
        g_npcDialogueQteBackdrop.gameObject,
        readActive(g_npcDialogueQteBackdrop.gameObject) ? 1 : 0,
        g_npcDialogueQteWheel.gameObject,
        readActive(g_npcDialogueQteWheel.gameObject) ? 1 : 0,
        g_npcDialogueQtePointer.gameObject,
        readActive(g_npcDialogueQtePointer.gameObject) ? 1 : 0,
        g_npcDialogueQteButton.gameObject,
        readActive(g_npcDialogueQteButton.gameObject) ? 1 : 0);
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"Enhanced NPC dialogue enabled: patience=%.1f excitement=%.2f.",
        g_npcDialoguePatience,
        g_npcDialogueExcitementSnapshot);
}

void NpcDialogue_HandleEnhancedChoice(bool correct)
{
    if (g_npcDialogueClimaxActive)
    {
        return;
    }
    const int replyIndex = g_npcDialogueReplyIndex;
    const float change = correct
        ? 5.0f - static_cast<float>(replyIndex)
        : -5.0f - static_cast<float>(replyIndex);
    g_npcDialoguePatience = std::clamp(
        g_npcDialoguePatience + change,
        0.0f,
        kNpcDialoguePatienceMax);
    ++g_npcDialogueReplyIndex;
    NpcDialogue_PlayNpcTalkAnimation();
    const unsigned long long now = GetTickCount64();
    g_npcDialogueNextChoicesAt =
        now +
        static_cast<unsigned long long>(
            NpcDialogue_NextChoiceDelayMilliseconds());
    g_npcDialogueCloseAfterTalk =
        g_npcDialoguePatience <= 0.0f;
    g_npcDialogueStage = correct ? 1 : 2;
    NpcDialogue_UpdateContent();
    NpcDialogue_UpdateEnhancedBars();
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"Enhanced NPC dialogue reply=%d correct=%d change=%+.1f patience=%.1f nextChoicesInMs=%llu.",
        g_npcDialogueReplyIndex,
        correct ? 1 : 0,
        change,
        g_npcDialoguePatience,
        g_npcDialogueNextChoicesAt - now);
}

bool NpcDialogue_UpdateEnhancedDialogue()
{
    if (!g_npcDialogueEnhancedMode)
    {
        NpcDialogue_SetEnhancedBarsVisible(false);
        return true;
    }
    const unsigned long long now = GetTickCount64();
    if (g_npcDialogueClimaxActive)
    {
        if (NpcDialogue_ClimaxAnimationFinished(now))
        {
            LogInfo(LogCategory_Dialogue, L"Dialogue", 
                L"Climax animation finished; closing NPC dialogue like Escape.");
            g_npcDialogueClimaxKeepAudioAfterClose = true;
            NpcDialogue_Close();
            return false;
        }
        if (g_npcDialogueTalkAnimationId != 0 &&
            now >= g_npcDialogueTalkAnimationEndsAt)
        {
            NpcDialogue_FinishNpcTalkAnimation();
        }
        NpcDialogue_UpdateEnhancedBars();
        return true;
    }
    if (g_npcDialogueLastPatienceTick == 0)
    {
        g_npcDialogueLastPatienceTick = now;
    }
    else
    {
        const float elapsedSeconds =
            static_cast<float>(
                now - g_npcDialogueLastPatienceTick) /
            1000.0f;
        g_npcDialogueLastPatienceTick = now;
        g_npcDialoguePatience = std::max(
            0.0f,
            g_npcDialoguePatience - elapsedSeconds);
    }

    if (g_npcDialogueTalkAnimationId != 0 &&
        now >= g_npcDialogueTalkAnimationEndsAt)
    {
        NpcDialogue_FinishNpcTalkAnimation();
    }
    NpcDialogue_UpdateQte();
    NpcDialogue_UpdateEnhancedBars();

    if (g_npcDialoguePatience <= 0.0f)
    {
        if (g_npcDialogueTalkAnimationId != 0)
        {
            g_npcDialogueCloseAfterTalk = true;
        }
        else
        {
            LogInfo(LogCategory_Dialogue, L"Dialogue", L"Enhanced NPC dialogue patience reached zero.");
            NpcDialogue_Close();
            return false;
        }
    }
    if (g_npcDialogueCloseAfterTalk &&
        g_npcDialogueTalkAnimationId == 0)
    {
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"Enhanced NPC dialogue closed after the final talk animation.");
        NpcDialogue_Close();
        return false;
    }
    if (g_npcDialogueStage != 0 &&
        g_npcDialogueNextChoicesAt != 0 &&
        now >= g_npcDialogueNextChoicesAt)
    {
        g_npcDialogueNextChoicesAt = 0;
        g_npcDialogueStage = 0;
        NpcDialogue_UpdateContent();
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"Enhanced NPC dialogue presented the next choice round.");
    }
    return true;
}

void NpcDialogue_FreezeNpc()
{
    auto* locomotion = NpcDialogue_GetNpcLocomotion(
        g_npcDialogueTarget);
    auto* locomotionClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcLocomotion");
    auto* setMoveZero = FindMethod(
        locomotionClass,
        "SetMoveZero",
        0);
    if (locomotion != nullptr && setMoveZero != nullptr)
    {
        Invoke(setMoveZero, locomotion, nullptr);
    }
}

void NpcDialogue_Open()
{
    if (g_npcDialogueTarget == nullptr ||
        g_npcDialogueRoot == nullptr ||
        !NpcDialogue_IsAlive(g_npcDialogueTarget) ||
        !NpcDialogue_IsTargetWithinTalkRange(
            g_npcDialogueTarget))
    {
        g_npcDialoguePromptVisible = false;
        g_npcDialogueTarget = nullptr;
        g_npcDialogueTargetGameObject = nullptr;
        ExtendedMode_SetNpcInteractionPrompt(false, "");
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue open rejected: target is outside the 2m talk range.");
        return;
    }
    g_npcDialogueActive = true;
    g_npcDialogueStage = 0;
    g_npcDialoguePromptVisible = false;
    g_npcDialogueTalkRequested = false;
    g_npcDialogueCloseRequested = false;
    g_npcDialogueMouseWasDown = false;
    g_extendedOpenSystemMenuRequested = false;
    g_npcDialogueHandledPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_npcDialogueCameraInitialized = false;
    g_npcDialogueCameraDirectionReady = false;
    g_npcDialogueCameraTransitionComplete = false;
    g_npcDialogueStabilitySampleReady = false;
    g_npcDialogueStillFrames = 0;
    g_npcDialogueLastCameraTick = 0;
    std::memset(g_npcDialogueLastPlayerPosition, 0, sizeof(float) * 3);
    std::memset(g_npcDialogueLastNpcPosition, 0, sizeof(float) * 3);
    ExtendedMode_SetNpcInteractionPrompt(false, "");
    SetWorldGameObjectActive(g_npcDialogueRoot, true);
    BringSceneObjectToFront(g_npcDialogueRoot);
    NpcDialogue_InitializeEnhancedDialogue();
    NpcDialogue_UpdateContent();
    NpcDialogue_LockActorAnimations();
    NpcDialogue_FreezeNpc();
    ClipCursor(nullptr);
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Dialogue, L"Dialogue", L"Custom NPC dialogue opened with both actors facing each other.");
}

void NpcDialogue_Close()
{
    if (!g_npcDialogueActive.exchange(false))
    {
        return;
    }
    NpcDialogue_ResetEnhancedDialogue();
    // ResetEnhancedDialogue drives the original ChangeAction(-1) path, which
    // clears the player animation move speed. Restore the saved locomotion
    // values after that reset so gameplay does not resume with MoveSpeed=0.
    NpcDialogue_UnlockActorAnimations();
    GameplayAnimationDiagnostic_Log(
        L"dialogue-close-final",
        NpcDialogue_GetPlayerAnimationManager(),
        g_extendedLastResolvedInputData,
        false);
    if (g_npcDialogueCameraSaved &&
        g_extendedCamera != nullptr)
    {
        auto* cameraTransform = GetWorldTransform(
            GetComponentGameObject(g_extendedCamera));
        auto* transformClass = FindClass("UnityEngine", "Transform");
        if (cameraTransform != nullptr &&
            transformClass != nullptr)
        {
            void* positionParameters[] = {
                g_npcDialogueSavedCameraPosition
            };
            void* rotationParameters[] = {
                g_npcDialogueSavedCameraRotation
            };
            Invoke(
                FindMethod(transformClass, "set_position", 1),
                cameraTransform,
                positionParameters);
            Invoke(
                FindMethod(transformClass, "set_rotation", 1),
                cameraTransform,
                rotationParameters);
        }
    }
    g_npcDialogueCameraSaved = false;
    g_npcDialogueCameraDirectionReady = false;
    g_npcDialogueCameraInitialized = false;
    g_npcDialogueCameraTransitionComplete = false;
    g_npcDialogueStabilitySampleReady = false;
    g_npcDialogueStillFrames = 0;
    g_npcDialogueTarget = nullptr;
    g_npcDialogueTargetGameObject = nullptr;
    SetWorldGameObjectActive(g_npcDialogueRoot, false);
    ExtendedMode_ForceGameplayCursor();
    LogInfo(LogCategory_Dialogue, L"Dialogue", L"Custom NPC dialogue closed.");
}

void NpcDialogue_HandleClick()
{
    const unsigned int serial =
        g_extendedManualPointerPressSerial.load();
    if (serial == g_npcDialogueHandledPressSerial)
    {
        return;
    }
    g_npcDialogueHandledPressSerial = serial;

    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    POINT cursor{
        g_extendedManualPointerPressX.load(),
        g_extendedManualPointerPressY.load()};
    if (gameWindow == nullptr ||
        !GetClientRect(gameWindow, &clientRect))
    {
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue click ignored: game client rectangle unavailable.");
        return;
    }
    const float width =
        static_cast<float>(clientRect.right - clientRect.left);
    const float height =
        static_cast<float>(clientRect.bottom - clientRect.top);
    if (width <= 0.0f || height <= 0.0f)
    {
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue click ignored: empty game client rectangle.");
        return;
    }
    if (!ScreenToClient(gameWindow, &cursor))
    {
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue click ignored: ScreenToClient failed.");
        return;
    }
    const float normalizedX =
        static_cast<float>(cursor.x) / width;
    const float normalizedY =
        1.0f - static_cast<float>(cursor.y) / height;
    const bool choice0Hit =
        normalizedX >= kNpcDialogueChoiceMinX &&
        normalizedX <= kNpcDialogueChoiceMaxX &&
        normalizedY >= kNpcDialogueChoice0MinY &&
        normalizedY <= kNpcDialogueChoice0MaxY;
    const bool choice1Hit =
        normalizedX >= kNpcDialogueChoiceMinX &&
        normalizedX <= kNpcDialogueChoiceMaxX &&
        normalizedY >= kNpcDialogueChoice1MinY &&
        normalizedY <= kNpcDialogueChoice1MaxY;
    const bool qteButtonHit =
        normalizedX >= kNpcDialogueQteButtonMinX &&
        normalizedX <= kNpcDialogueQteButtonMaxX &&
        normalizedY >= kNpcDialogueQteButtonMinY &&
        normalizedY <= kNpcDialogueQteButtonMaxY;
    LogInfo(LogCategory_Dialogue, L"Dialogue", 
        L"NPC dialogue click client=(%ld,%ld) normalized=(%.3f,%.3f) choice0=%d choice1=%d qte=%d.",
        cursor.x,
        cursor.y,
        normalizedX,
        normalizedY,
        choice0Hit ? 1 : 0,
        choice1Hit ? 1 : 0,
        qteButtonHit ? 1 : 0);
    if (qteButtonHit)
    {
        NpcDialogue_HandleQteClick();
        return;
    }
    if (g_npcDialogueStage != 0)
    {
        LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue click ignored after a choice was selected.");
        return;
    }
    if (choice0Hit)
    {
        if (g_npcDialogueEnhancedMode)
        {
            NpcDialogue_HandleEnhancedChoice(true);
        }
        else
        {
            g_npcDialogueStage = 1;
            NpcDialogue_UpdateContent();
            LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue selected choice 0: 有事 -> 别烦我.");
        }
    }
    else if (choice1Hit)
    {
        if (g_npcDialogueEnhancedMode)
        {
            NpcDialogue_HandleEnhancedChoice(false);
        }
        else
        {
            g_npcDialogueStage = 2;
            NpcDialogue_UpdateContent();
            LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue selected choice 1: 没事 -> 何意味.");
        }
    }
}

void NpcDialogue_Tick()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_npcDialogueActive.load())
        {
            NpcDialogue_Close();
        }
        else
        {
            NpcDialogue_UnlockActorAnimations();
        }
        g_npcDialoguePromptVisible = false;
        return;
    }
    NpcDialogue_BuildUi();
    if (g_npcDialogueRoot == nullptr)
    {
        return;
    }

    if (g_npcDialogueActive.load())
    {
        g_npcDialoguePromptVisible = false;
        ExtendedMode_SetNpcInteractionPrompt(false, "");
        if (g_npcDialogueCloseRequested.exchange(false) ||
            !NpcDialogue_IsAlive(g_npcDialogueTarget))
        {
            NpcDialogue_Close();
            return;
        }
        if (!NpcDialogue_UpdateEnhancedDialogue())
        {
            return;
        }
        NpcDialogue_FreezeNpc();
        NpcDialogue_HoldActorAnimationLock();
        if (!g_npcDialogueCameraInitialized)
        {
            auto* playerTransform = GetWorldTransform(
                g_extendedPlayerGameObject);
            auto* npcTransform = g_npcDialogueTarget != nullptr
                ? *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<char*>(g_npcDialogueTarget) + 0x10)
                : nullptr;
            float playerPosition[3]{};
            float npcPosition[3]{};
            if (NpcDialogue_GetTransformPosition(
                    playerTransform,
                    playerPosition) &&
                NpcDialogue_GetTransformPosition(
                    npcTransform,
                    npcPosition))
            {
                const bool hadPrevious =
                    g_npcDialogueStabilitySampleReady;
                const float playerMove =
                    hadPrevious
                        ? std::sqrt(
                            (playerPosition[0] - g_npcDialogueLastPlayerPosition[0]) *
                                (playerPosition[0] - g_npcDialogueLastPlayerPosition[0]) +
                            (playerPosition[1] - g_npcDialogueLastPlayerPosition[1]) *
                                (playerPosition[1] - g_npcDialogueLastPlayerPosition[1]) +
                            (playerPosition[2] - g_npcDialogueLastPlayerPosition[2]) *
                                (playerPosition[2] - g_npcDialogueLastPlayerPosition[2]))
                        : 0.0f;
                const float npcMove =
                    hadPrevious
                        ? std::sqrt(
                            (npcPosition[0] - g_npcDialogueLastNpcPosition[0]) *
                                (npcPosition[0] - g_npcDialogueLastNpcPosition[0]) +
                            (npcPosition[1] - g_npcDialogueLastNpcPosition[1]) *
                                (npcPosition[1] - g_npcDialogueLastNpcPosition[1]) +
                            (npcPosition[2] - g_npcDialogueLastNpcPosition[2]) *
                                (npcPosition[2] - g_npcDialogueLastNpcPosition[2]))
                        : 0.0f;
                std::memcpy(
                    g_npcDialogueLastPlayerPosition,
                    playerPosition,
                    sizeof(playerPosition));
                std::memcpy(
                    g_npcDialogueLastNpcPosition,
                    npcPosition,
                    sizeof(npcPosition));
                g_npcDialogueStabilitySampleReady = true;
                if (hadPrevious &&
                    playerMove < 0.006f &&
                    npcMove < 0.006f)
                {
                    ++g_npcDialogueStillFrames;
                }
                else
                {
                    g_npcDialogueStillFrames = 0;
                }
                if (g_npcDialogueStillFrames >= 6)
                {
                    NpcDialogue_PlaceCamera(
                        g_npcDialogueTarget,
                        true);
                }
            }
        }
        else
        {
            NpcDialogue_PlaceCamera(
                g_npcDialogueTarget,
                false);
            NpcDialogue_UpdateCameraMotion();
        }
        ClipCursor(nullptr);
        ExtendedMode_ForceCursor();
        NpcDialogue_HandleClick();
        return;
    }

    if (g_npcDialogueTalkRequested.exchange(false))
    {
        if (g_npcDialogueTarget == nullptr ||
            !NpcDialogue_IsTargetWithinTalkRange(
                g_npcDialogueTarget))
        {
            g_npcDialoguePromptVisible = false;
            g_npcDialogueTarget = nullptr;
            g_npcDialogueTargetGameObject = nullptr;
            ExtendedMode_SetNpcInteractionPrompt(false, "");
            LogInfo(LogCategory_Dialogue, L"Dialogue", L"NPC dialogue talk request rejected: target moved outside 2m.");
            return;
        }
        NpcDialogue_Open();
        return;
    }

    const bool promptAllowed =
        !ModGuideUi_IsOpen() &&
        !g_extendedOriginalTutorialActive.load() &&
        !g_extendedPoseMenuVisible.load() &&
        !g_gameplayInventoryOpen.load() &&
        !CheckpointSystem_IsTaskUiOpen() &&
        !g_extendedOriginalSystemMenuVisible.load() &&
        !g_extendedOriginalInteractionMenuActive.load() &&
        !g_extendedOriginalHandcuffsPanelActive.load();
    auto* target = promptAllowed
        ? NpcDialogue_FindCenterNpc()
        : nullptr;
    if (target != nullptr)
    {
        const bool targetChanged =
            g_npcDialogueTarget != target;
        g_npcDialogueTarget = target;
        g_npcDialogueTargetGameObject =
            NpcDialogue_GetNpcGameObject(target);
        if (targetChanged)
        {
            LogInfo(LogCategory_Dialogue, L"Dialogue", 
                L"NPC dialogue centre target acquired: %p.",
                target);
        }
        const bool promptWasVisible =
            g_npcDialoguePromptVisible.exchange(true);
        if (targetChanged || !promptWasVisible)
        {
            ExtendedMode_SetNpcInteractionPrompt(
                true,
                "搭讪");
        }
    }
    else
    {
        g_npcDialogueTarget = nullptr;
        g_npcDialogueTargetGameObject = nullptr;
        g_npcDialoguePromptVisible = false;
        ExtendedMode_SetNpcInteractionPrompt(false, "");
    }
}

void NpcDialogue_ResetForSceneChange()
{
    if (g_npcDialogueActive.load())
    {
        NpcDialogue_Close();
    }
    NpcDialogue_UnlockActorAnimations();
    NpcDialogue_ResetEnhancedDialogue();
    g_npcDialoguePromptVisible = false;
    g_npcDialogueTalkRequested = false;
    g_npcDialogueCloseRequested = false;
    g_npcDialogueTarget = nullptr;
    g_npcDialogueTargetGameObject = nullptr;
    g_npcDialogueRoot = nullptr;
    g_npcDialogueNameText = nullptr;
    g_npcDialogueBodyText = nullptr;
    g_npcDialogueChoice0Panel = nullptr;
    g_npcDialogueChoice1Panel = nullptr;
    g_npcDialogueChoice0Text = nullptr;
    g_npcDialogueChoice1Text = nullptr;
    g_npcDialogueHintText = nullptr;
    g_npcDialoguePatienceLabel = SceneUiElement{};
    g_npcDialogueExcitementLabel = SceneUiElement{};
    g_npcDialoguePatienceTrack = SceneUiElement{};
    g_npcDialoguePatienceFill = SceneUiElement{};
    g_npcDialogueExcitementTrack = SceneUiElement{};
    g_npcDialogueExcitementFill = SceneUiElement{};
    g_npcDialogueQteBackdrop = SceneUiElement{};
    g_npcDialogueQteFeedbackOverlay = SceneUiElement{};
    g_npcDialogueQteWheel = SceneUiElement{};
    g_npcDialogueQtePointer = SceneUiElement{};
    g_npcDialogueQtePointerShape = SceneUiElement{};
    g_npcDialogueQteButton = SceneUiElement{};
    g_npcDialogueQteButtonText = SceneUiElement{};
    g_npcDialogueQteSegments.clear();
    g_npcDialogueQtePointerStrips.clear();
    g_npcDialogueStage = 0;
    g_npcDialogueMouseWasDown = false;
    g_npcDialogueHandledPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_npcDialogueCameraSaved = false;
    g_npcDialogueCameraDirectionReady = false;
}
