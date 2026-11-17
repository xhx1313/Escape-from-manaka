// Native.cpp module: gameplay_action_pursuit.inc
// Temporarily redirects NPC route data toward a NavMesh path queue while an
// E-menu animation is active and that NPC can actually see the player.

constexpr int kPursuitMaxNpcs = 6;
constexpr float kPursuitApproachDistance = 1.6f;
constexpr float kPursuitMaxQueueDistance = 8.0f;
constexpr float kPursuitPlayerCollisionRadiusFallback = 0.35f;
constexpr float kPursuitTargetSampleRadius = 1.0f;
constexpr float kPursuitHoldEnterDistance = 2.10f;
constexpr float kPursuitHoldExitDistance = 2.75f;
constexpr float kPursuitMinimumInwardStep = 0.15f;
constexpr float kPursuitApproachConeDot = 0.65f;
constexpr float kPursuitArrivalDistance = 0.65f;
constexpr float kPursuitPlayerMoveRefreshDistance = 1.00f;
constexpr float kPursuitTargetUpdateDistance = 0.35f;
constexpr int kPursuitNavMeshAreaMask = -1;
constexpr unsigned long long kPursuitAnimationTimeoutMs = 750;
constexpr unsigned long long kPursuitScanIntervalMs = 50;
constexpr unsigned long long kPursuitTargetRefreshMs = 200;
constexpr unsigned long long kPursuitProgressTimeoutMs = 3500;
constexpr unsigned long long kPursuitProgressCheckMs = 250;
constexpr float kPursuitMinimumProgressMeters = 0.15f;

constexpr int kNpcStateIdle = 0;
constexpr int kNpcStateFoundStranger = 1;
constexpr int kNpcFixedTypeNone = 0;
constexpr int kPursuitActionLayer = 1;

enum GameplayActionPursuitPhase
{
    GameplayActionPursuit_Disabled = 0,
    GameplayActionPursuit_WaitingForAnimation,
    GameplayActionPursuit_Active,
    GameplayActionPursuit_Ending
};

struct NativeVector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct NativeQuaternion
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

struct NativeNavMeshHit
{
    NativeVector3 position{};
    NativeVector3 normal{};
    float distance = 0.0f;
    int mask = 0;
    int hit = 0;
};

struct GameplayActionPursuitAnimatorState
{
    bool valid = false;
    int fullPathHash = 0;
};

struct GameplayActionPursuitNpcRecord
{
    unsigned int sessionId = 0;
    uint32_t npcHandle = 0;
    uint32_t routeDataHandle = 0;
    uint32_t originalRouteListHandle = 0;
    uint32_t temporaryRouteListHandle = 0;
    uint32_t temporaryRoutePointHandle = 0;
    uint32_t temporaryGameObjectHandle = 0;

    int savedCurrentIndex = 0;
    int originalListCount = 0;
    int queueIndex = -1;
    float collisionRadius = 0.35f;
    NativeVector3 currentTarget{};
    NativeVector3 lastPlayerPosition{};
    bool hasPlayerPosition = false;
    bool holding = false;
    bool hasSavedAgentStopState = false;
    bool savedAgentIsStopped = false;
    bool hasSavedAgentPositionState = false;
    bool savedAgentUpdatePosition = true;
    bool hasSavedAgentRotationState = false;
    bool savedAgentUpdateRotation = true;
    bool hasHoldPose = false;
    NativeVector3 holdPosition{};
    NativeQuaternion holdRotation{};
    bool hasSavedMotionSpeed = false;
    float savedMotionSpeed = 0.0f;
    bool hasSavedWaitState = false;
    bool savedWaitState = false;

    float bestDistanceToTarget = 0.0f;
    unsigned long long lastProgressTick = 0;
    unsigned long long lastTargetRefreshTick = 0;
    unsigned long long lastProgressCheckTick = 0;
};

struct GameplayActionPursuitContext
{
    bool attempted = false;
    bool valid = false;

    Il2CppClass* npcManagerClass = nullptr;
    Il2CppClass* npcControllerClass = nullptr;
    Il2CppClass* npcComponentAccessorClass = nullptr;
    Il2CppClass* routeDataClass = nullptr;
    Il2CppClass* routePointClass = nullptr;
    Il2CppClass* gameObjectClass = nullptr;
    Il2CppClass* transformClass = nullptr;
    Il2CppClass* navMeshClass = nullptr;
    Il2CppClass* navMeshAgentClass = nullptr;
    Il2CppClass* navMeshPathClass = nullptr;
    Il2CppClass* animatorClass = nullptr;
    Il2CppClass* playerClassAccessorClass = nullptr;
    Il2CppClass* characterControllerClass = nullptr;
    Il2CppClass* npcAnimationManagerClass = nullptr;
    Il2CppClass* routePointListClass = nullptr;

    const Il2CppMethod* npcManagerGetInstance = nullptr;
    const Il2CppMethod* npcGetIsPlayerFound = nullptr;
    const Il2CppMethod* npcGetTransform = nullptr;
    const Il2CppMethod* npcChangeState = nullptr;
    const Il2CppMethod* npcComponentAccessorGetAnimationManager = nullptr;
    const Il2CppMethod* npcAnimationGetCurrentMoveSpeed = nullptr;
    const Il2CppMethod* npcAnimationSetMotionSpeed = nullptr;
    const Il2CppMethod* npcAnimationSetWait = nullptr;

    const Il2CppMethod* routePointSetType = nullptr;
    const Il2CppMethod* routePointSetInteractType = nullptr;
    const Il2CppMethod* routePointSetLinkPublisher = nullptr;
    const Il2CppMethod* routePointGetFloor = nullptr;
    const Il2CppMethod* routePointSetFloor = nullptr;
    const Il2CppMethod* routePointSetEscalatorTarget = nullptr;
    const Il2CppMethod* routePointSetMaxPathRightScale = nullptr;
    const Il2CppMethod* routePointSetParam = nullptr;
    const Il2CppMethod* routePointSetPathRight = nullptr;

    const Il2CppMethod* transformGetPosition = nullptr;
    const Il2CppMethod* transformSetPosition = nullptr;
    const Il2CppMethod* transformGetForward = nullptr;
    const Il2CppMethod* transformGetRotation = nullptr;
    const Il2CppMethod* transformSetRotation = nullptr;
    const Il2CppMethod* gameObjectGetTransform = nullptr;
    const Il2CppMethod* gameObjectConstructor = nullptr;
    const Il2CppMethod* playerClassAccessorGetTransform = nullptr;

    const Il2CppMethod* listGetCount = nullptr;
    const Il2CppMethod* listGetItem = nullptr;
    const Il2CppMethod* listAdd = nullptr;
    const Il2CppMethod* listConstructor = nullptr;

    const Il2CppMethod* navMeshSamplePosition = nullptr;
    const Il2CppMethod* navMeshCalculatePath = nullptr;
    const Il2CppMethod* navPathConstructor = nullptr;
    const Il2CppMethod* navPathGetStatus = nullptr;

    const Il2CppMethod* animatorGetCurrentState = nullptr;
    const Il2CppMethod* animatorStringToHash = nullptr;
    const Il2CppMethod* navAgentGetRadius = nullptr;
    const Il2CppMethod* navAgentGetIsStopped = nullptr;
    const Il2CppMethod* navAgentSetIsStopped = nullptr;
    const Il2CppMethod* navAgentGetUpdatePosition = nullptr;
    const Il2CppMethod* navAgentSetUpdatePosition = nullptr;
    const Il2CppMethod* navAgentGetUpdateRotation = nullptr;
    const Il2CppMethod* navAgentSetUpdateRotation = nullptr;
    const Il2CppMethod* navAgentResetPath = nullptr;
    const Il2CppMethod* playerClassAccessorGetCharacterController = nullptr;
    const Il2CppMethod* characterControllerGetRadius = nullptr;
};

GameplayActionPursuitPhase g_gameplayActionPursuitPhase =
    GameplayActionPursuit_Disabled;
int g_gameplayActionPursuitRequestedType = -1;
int g_gameplayActionPursuitConfirmedType = -1;
int g_gameplayActionPursuitExpectedLayer = kPursuitActionLayer;
int g_gameplayActionPursuitExpectedStateHash = 0;
int g_gameplayActionPursuitMatchingFrames = 0;
int g_gameplayActionPursuitMissingFrames = 0;
unsigned int g_gameplayActionPursuitSessionId = 0;
unsigned long long g_gameplayActionPursuitRequestTick = 0;
unsigned long long g_gameplayActionPursuitLastScanTick = 0;
bool g_gameplayActionPursuitQueueDirty = false;
bool g_gameplayActionPursuitPostUpdateHookReady = false;
std::vector<GameplayActionPursuitNpcRecord> g_gameplayActionPursuitRecords;
std::unordered_set<Il2CppObject*> g_gameplayActionPursuitLoggedSkips;
std::unordered_set<Il2CppObject*> g_gameplayActionPursuitSuppressedNpcs;
GameplayActionPursuitContext g_gameplayActionPursuitContext;

bool GameplayActionPursuit_ReadBoxedInt(Il2CppObject* value, int& output)
{
    if (value == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    output = *static_cast<int*>(il2cpp_object_unbox(value));
    return true;
}

bool GameplayActionPursuit_ReadBoxedBool(Il2CppObject* value, bool& output)
{
    if (value == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    output = *static_cast<bool*>(il2cpp_object_unbox(value));
    return true;
}

bool GameplayActionPursuit_ReadBoxedFloat(Il2CppObject* value, float& output)
{
    if (value == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    output = *static_cast<float*>(il2cpp_object_unbox(value));
    return true;
}

float GameplayActionPursuit_HorizontalDistance(
    const NativeVector3& left,
    const NativeVector3& right)
{
    const float dx = left.x - right.x;
    const float dz = left.z - right.z;
    return std::sqrt(dx * dx + dz * dz);
}

bool GameplayActionPursuit_ReadVector(Il2CppObject* value, NativeVector3& output)
{
    if (value == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    const auto* data = static_cast<const float*>(il2cpp_object_unbox(value));
    output = NativeVector3{ data[0], data[1], data[2] };
    return true;
}

bool GameplayActionPursuit_ReadQuaternion(
    Il2CppObject* value,
    NativeQuaternion& output)
{
    if (value == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    const auto* data = static_cast<const float*>(il2cpp_object_unbox(value));
    output = NativeQuaternion{
        data[0],
        data[1],
        data[2],
        data[3]
    };
    return true;
}

Il2CppObject* GameplayActionPursuit_GetLiveObject(uint32_t handle)
{
    if (handle == 0 || il2cpp_gchandle_get_target == nullptr)
    {
        return nullptr;
    }
    return il2cpp_gchandle_get_target(handle);
}

uint32_t GameplayActionPursuit_CreateHandle(Il2CppObject* object)
{
    if (object == nullptr || il2cpp_gchandle_new == nullptr)
    {
        return 0;
    }
    return il2cpp_gchandle_new(object, false);
}

void GameplayActionPursuit_FreeHandle(uint32_t& handle)
{
    if (handle != 0 && il2cpp_gchandle_free != nullptr)
    {
        il2cpp_gchandle_free(handle);
    }
    handle = 0;
}

bool GameplayActionPursuit_SetNpcState(
    Il2CppObject* npc,
    int state)
{
    if (npc == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(npc) ||
        g_gameplayActionPursuitContext.npcChangeState == nullptr)
    {
        return false;
    }
    void* stateParameters[] = { &state };
    Invoke(
        g_gameplayActionPursuitContext.npcChangeState,
        npc,
        stateParameters);
    return true;
}

bool GameplayActionPursuit_ReadIntField(
    Il2CppObject* object,
    int offset,
    int& output)
{
    if (object == nullptr)
    {
        return false;
    }
    output = *reinterpret_cast<int*>(
        reinterpret_cast<char*>(object) + offset);
    return true;
}

bool GameplayActionPursuit_ReadBoolField(
    Il2CppObject* object,
    int offset,
    bool& output)
{
    if (object == nullptr)
    {
        return false;
    }
    output = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(object) + offset);
    return true;
}

Il2CppObject* GameplayActionPursuit_ReadObjectField(
    Il2CppObject* object,
    int offset)
{
    return object != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(object) + offset)
        : nullptr;
}

bool GameplayActionPursuit_GetTransformPosition(
    Il2CppObject* transform,
    NativeVector3& output)
{
    if (transform == nullptr ||
        g_gameplayActionPursuitContext.transformGetPosition == nullptr)
    {
        return false;
    }
    auto* value = Invoke(
        g_gameplayActionPursuitContext.transformGetPosition,
        transform,
        nullptr);
    return GameplayActionPursuit_ReadVector(value, output);
}

bool GameplayActionPursuit_GetPlayerPosition(NativeVector3& output)
{
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    if (playerClassAccessor == nullptr ||
        g_gameplayActionPursuitContext.playerClassAccessorGetTransform == nullptr)
    {
        return false;
    }
    auto* transform = Invoke(
        g_gameplayActionPursuitContext.playerClassAccessorGetTransform,
        playerClassAccessor,
        nullptr);
    return GameplayActionPursuit_GetTransformPosition(transform, output);
}

int GameplayActionPursuit_ReadPlayerRuntimeState()
{
    auto* playerState = GameplayStatusHud_GetPlayerState();
    return playerState != nullptr
        ? *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerState) + 0x5C)
        : -1;
}

bool GameplayActionPursuit_GetNpcPosition(
    Il2CppObject* npc,
    NativeVector3& output)
{
    if (npc == nullptr ||
        g_gameplayActionPursuitContext.npcGetTransform == nullptr)
    {
        return false;
    }
    auto* transform = Invoke(
        g_gameplayActionPursuitContext.npcGetTransform,
        npc,
        nullptr);
    return GameplayActionPursuit_GetTransformPosition(transform, output);
}

bool GameplayActionPursuit_BuildHoldAnchor(
    Il2CppObject* npc,
    const NativeVector3& npcPosition,
    const NativeVector3& previousTarget,
    NativeVector3& output)
{
    auto& context = g_gameplayActionPursuitContext;
    NativeVector3 forward{};
    if (npc != nullptr && context.npcGetTransform != nullptr)
    {
        auto* transform = Invoke(
            context.npcGetTransform,
            npc,
            nullptr);
        if (transform != nullptr && context.transformGetForward != nullptr)
        {
            auto* value = Invoke(
                context.transformGetForward,
                transform,
                nullptr);
            GameplayActionPursuit_ReadVector(value, forward);
        }
    }
    float dx = forward.x;
    float dz = forward.z;
    float length = std::sqrt(dx * dx + dz * dz);
    if (length < 0.05f)
    {
        dx = previousTarget.x - npcPosition.x;
        dz = previousTarget.z - npcPosition.z;
        length = std::sqrt(dx * dx + dz * dz);
    }
    NativeVector3 playerPosition{};
    if (length < 0.05f &&
        GameplayActionPursuit_GetPlayerPosition(playerPosition))
    {
        dx = playerPosition.x - npcPosition.x;
        dz = playerPosition.z - npcPosition.z;
        length = std::sqrt(dx * dx + dz * dz);
    }
    if (length < 0.05f)
    {
        return false;
    }
    dx /= length;
    dz /= length;
    output = NativeVector3{
        npcPosition.x + dx * 0.25f,
        npcPosition.y,
        npcPosition.z + dz * 0.25f
    };
    return true;
}

bool GameplayActionPursuit_ReadNpcState(
    Il2CppObject* npc,
    int& currentState,
    int& fixedType,
    bool& isSit)
{
    auto* stateModel = GameplayActionPursuit_ReadObjectField(npc, 0x48);
    return
        GameplayActionPursuit_ReadIntField(
            stateModel,
            0x10,
            currentState) &&
        GameplayActionPursuit_ReadIntField(
            stateModel,
            0x58,
            fixedType) &&
        GameplayActionPursuit_ReadBoolField(
            stateModel,
            0x1F,
            isSit);
}

bool GameplayActionPursuit_ReadSeen(
    Il2CppObject* npc,
    bool& isSeen)
{
    if (npc == nullptr ||
        g_gameplayActionPursuitContext.npcGetIsPlayerFound == nullptr)
    {
        return false;
    }
    auto* value = Invoke(
        g_gameplayActionPursuitContext.npcGetIsPlayerFound,
        npc,
        nullptr);
    return GameplayActionPursuit_ReadBoxedBool(value, isSeen);
}

void GameplayActionPursuit_LogSeenSkip(
    Il2CppObject* npc,
    const wchar_t* reason,
    int state = -1,
    int fixedType = -1,
    bool isSit = false)
{
    if (npc == nullptr ||
        !g_gameplayActionPursuitLoggedSkips.insert(npc).second)
    {
        return;
    }
    LogInfo(LogCategory_Action, L"Action", 
        L"Seen NPC was not pursued: npc=%p state=%d fixed=%d sit=%d reason=%s.",
        npc,
        state,
        fixedType,
        isSit ? 1 : 0,
        reason != nullptr ? reason : L"unknown");
}

int GameplayActionPursuit_GetListCount(
    Il2CppObject* list,
    const Il2CppMethod* countMethod)
{
    if (list == nullptr || countMethod == nullptr)
    {
        return -1;
    }
    auto* value = Invoke(countMethod, list, nullptr);
    int count = -1;
    return GameplayActionPursuit_ReadBoxedInt(value, count) ? count : -1;
}

Il2CppObject* GameplayActionPursuit_GetListItem(
    Il2CppObject* list,
    const Il2CppMethod* itemMethod,
    int index)
{
    if (list == nullptr || itemMethod == nullptr)
    {
        return nullptr;
    }
    void* parameters[] = { &index };
    return Invoke(itemMethod, list, parameters);
}

bool GameplayActionPursuit_RefreshRouteListMethods(Il2CppClass* listClass)
{
    if (listClass == nullptr || il2cpp_object_get_class == nullptr)
    {
        return false;
    }
    auto& context = g_gameplayActionPursuitContext;
    if (context.routePointListClass == listClass &&
        context.listGetCount != nullptr &&
        context.listGetItem != nullptr &&
        context.listAdd != nullptr &&
        context.listConstructor != nullptr)
    {
        return true;
    }
    context.routePointListClass = listClass;
    context.listGetCount = FindMethod(listClass, "get_Count", 0);
    context.listGetItem = FindMethod(listClass, "get_Item", 1);
    context.listAdd = FindMethod(listClass, "Add", 1);
    context.listConstructor = FindMethod(listClass, ".ctor", 0);
    return
        context.listGetCount != nullptr &&
        context.listGetItem != nullptr &&
        context.listAdd != nullptr &&
        context.listConstructor != nullptr;
}

bool GameplayActionPursuit_EnsureContext()
{
    auto& context = g_gameplayActionPursuitContext;
    if (context.attempted)
    {
        return context.valid;
    }
    context.attempted = true;

    context.npcManagerClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcManager");
    context.npcControllerClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcController");
    context.npcComponentAccessorClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcComponentAccessor");
    context.routeDataClass = FindClass(
        "ExposureUnnoticed2.Object3D.RoutePoint",
        "RouteData");
    context.routePointClass = FindClass(
        "ExposureUnnoticed2.Object3D.RoutePoint",
        "RoutePoint");
    context.gameObjectClass = FindClass("UnityEngine", "GameObject");
    context.transformClass = FindClass("UnityEngine", "Transform");
    context.navMeshClass = FindClass("UnityEngine.AI", "NavMesh");
    context.navMeshAgentClass = FindClass("UnityEngine.AI", "NavMeshAgent");
    context.navMeshPathClass = FindClass("UnityEngine.AI", "NavMeshPath");
    context.animatorClass = FindClass("UnityEngine", "Animator");
    context.playerClassAccessorClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerClassAccessor");
    context.characterControllerClass = FindClass(
        "UnityEngine",
        "CharacterController");
    context.npcAnimationManagerClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAnimationManager");

    context.npcManagerGetInstance = FindMethod(
        context.npcManagerClass,
        "get_Instance",
        0);
    context.npcGetIsPlayerFound = FindMethod(
        context.npcControllerClass,
        "get_IsPlayerFound",
        0);
    context.npcGetTransform = FindMethod(
        context.npcControllerClass,
        "get_Transform",
        0);
    context.npcChangeState = FindMethod(
        context.npcControllerClass,
        "ChangeState",
        1);
    context.npcComponentAccessorGetAnimationManager = FindMethod(
        context.npcComponentAccessorClass,
        "get_NpcAnimationManager",
        0);
    context.npcAnimationGetCurrentMoveSpeed = FindMethod(
        context.npcAnimationManagerClass,
        "GetCurrentMoveSpeed",
        0);
    context.npcAnimationSetMotionSpeed = FindMethod(
        context.npcAnimationManagerClass,
        "SetMotionSpeed",
        1);
    context.npcAnimationSetWait = FindMethod(
        context.npcAnimationManagerClass,
        "SetWait",
        1);

    context.routePointSetType = FindMethod(
        context.routePointClass,
        "set_Type",
        1);
    context.routePointSetInteractType = FindMethod(
        context.routePointClass,
        "set_RouteInteractType",
        1);
    context.routePointSetLinkPublisher = FindMethod(
        context.routePointClass,
        "set_LinkTokenPublisher",
        1);
    context.routePointGetFloor = FindMethod(
        context.routePointClass,
        "get_Floor",
        0);
    context.routePointSetFloor = FindMethod(
        context.routePointClass,
        "set_Floor",
        1);
    context.routePointSetEscalatorTarget = FindMethod(
        context.routePointClass,
        "set_EscalatorTarget",
        1);
    context.routePointSetMaxPathRightScale = FindMethod(
        context.routePointClass,
        "set_MaxPathRightScale",
        1);
    context.routePointSetParam = FindMethod(
        context.routePointClass,
        "set_Param",
        1);
    context.routePointSetPathRight = FindMethod(
        context.routePointClass,
        "set_PathRight",
        1);

    context.transformGetPosition = FindMethod(
        context.transformClass,
        "get_position",
        0);
    context.transformSetPosition = FindMethod(
        context.transformClass,
        "set_position",
        1);
    context.transformGetForward = FindMethod(
        context.transformClass,
        "get_forward",
        0);
    context.transformGetRotation = FindMethod(
        context.transformClass,
        "get_rotation",
        0);
    context.transformSetRotation = FindMethod(
        context.transformClass,
        "set_rotation",
        1);
    context.gameObjectGetTransform = FindMethod(
        context.gameObjectClass,
        "get_transform",
        0);
    context.gameObjectConstructor = FindMethod(
        context.gameObjectClass,
        ".ctor",
        1);
    context.playerClassAccessorGetTransform = FindMethod(
        context.playerClassAccessorClass,
        "get_Transform",
        0);

    context.navMeshSamplePosition = FindMethod(
        context.navMeshClass,
        "SamplePosition",
        4);
    context.navMeshCalculatePath = FindMethod(
        context.navMeshClass,
        "CalculatePath",
        4);
    context.navPathConstructor = FindMethod(
        context.navMeshPathClass,
        ".ctor",
        0);
    context.navPathGetStatus = FindMethod(
        context.navMeshPathClass,
        "get_status",
        0);

    context.animatorGetCurrentState = FindMethod(
        context.animatorClass,
        "GetCurrentAnimatorStateInfo",
        1);
    context.animatorStringToHash = FindMethod(
        context.animatorClass,
        "StringToHash",
        1);
    context.navAgentGetRadius = FindMethod(
        context.navMeshAgentClass,
        "get_radius",
        0);
    context.navAgentGetIsStopped = FindMethod(
        context.navMeshAgentClass,
        "get_isStopped",
        0);
    context.navAgentSetIsStopped = FindMethod(
        context.navMeshAgentClass,
        "set_isStopped",
        1);
    context.navAgentGetUpdatePosition = FindMethod(
        context.navMeshAgentClass,
        "get_updatePosition",
        0);
    context.navAgentSetUpdatePosition = FindMethod(
        context.navMeshAgentClass,
        "set_updatePosition",
        1);
    context.navAgentGetUpdateRotation = FindMethod(
        context.navMeshAgentClass,
        "get_updateRotation",
        0);
    context.navAgentSetUpdateRotation = FindMethod(
        context.navMeshAgentClass,
        "set_updateRotation",
        1);
    context.navAgentResetPath = FindMethod(
        context.navMeshAgentClass,
        "ResetPath",
        0);
    context.playerClassAccessorGetCharacterController = FindMethod(
        context.playerClassAccessorClass,
        "get_CharacterController",
        0);
    context.characterControllerGetRadius = FindMethod(
        context.characterControllerClass,
        "get_radius",
        0);

    context.valid =
        context.npcManagerClass != nullptr &&
        context.npcControllerClass != nullptr &&
        context.npcComponentAccessorClass != nullptr &&
        context.routeDataClass != nullptr &&
        context.routePointClass != nullptr &&
        context.gameObjectClass != nullptr &&
        context.transformClass != nullptr &&
        context.navMeshClass != nullptr &&
        context.navMeshAgentClass != nullptr &&
        context.navMeshPathClass != nullptr &&
        context.animatorClass != nullptr &&
        context.playerClassAccessorClass != nullptr &&
        context.characterControllerClass != nullptr &&
        context.npcAnimationManagerClass != nullptr &&
        context.npcManagerGetInstance != nullptr &&
        context.npcGetIsPlayerFound != nullptr &&
        context.npcGetTransform != nullptr &&
        context.npcChangeState != nullptr &&
        context.npcComponentAccessorGetAnimationManager != nullptr &&
        context.npcAnimationGetCurrentMoveSpeed != nullptr &&
        context.npcAnimationSetMotionSpeed != nullptr &&
        context.npcAnimationSetWait != nullptr &&
        context.routePointSetType != nullptr &&
        context.routePointSetInteractType != nullptr &&
        context.routePointSetLinkPublisher != nullptr &&
        context.routePointGetFloor != nullptr &&
        context.routePointSetFloor != nullptr &&
        context.routePointSetEscalatorTarget != nullptr &&
        context.routePointSetMaxPathRightScale != nullptr &&
        context.routePointSetParam != nullptr &&
        context.routePointSetPathRight != nullptr &&
        context.transformGetPosition != nullptr &&
        context.transformSetPosition != nullptr &&
        context.gameObjectGetTransform != nullptr &&
        context.gameObjectConstructor != nullptr &&
        context.playerClassAccessorGetTransform != nullptr &&
        context.navMeshSamplePosition != nullptr &&
        context.navMeshCalculatePath != nullptr &&
        context.navPathConstructor != nullptr &&
        context.navPathGetStatus != nullptr &&
        context.animatorGetCurrentState != nullptr &&
        context.animatorStringToHash != nullptr;

    if (!context.valid)
    {
        struct PursuitDependency
        {
            const void* value;
            const wchar_t* name;
        };
        const PursuitDependency dependencies[] = {
            { context.npcManagerClass, L"NpcManager class" },
            { context.npcControllerClass, L"NpcController class" },
            {
                context.npcComponentAccessorClass,
                L"NpcComponentAccessor class"
            },
            { context.routeDataClass, L"RouteData class" },
            { context.routePointClass, L"RoutePoint class" },
            { context.gameObjectClass, L"GameObject class" },
            { context.transformClass, L"Transform class" },
            { context.navMeshClass, L"NavMesh class" },
            { context.navMeshAgentClass, L"NavMeshAgent class" },
            { context.navMeshPathClass, L"NavMeshPath class" },
            { context.animatorClass, L"Animator class" },
            {
                context.playerClassAccessorClass,
                L"PlayerClassAccessor class"
            },
            {
                context.npcManagerGetInstance,
                L"NpcManager.get_Instance"
            },
            {
                context.npcGetIsPlayerFound,
                L"NpcController.get_IsPlayerFound"
            },
            {
                context.npcGetTransform,
                L"NpcController.get_Transform"
            },
            {
                context.npcChangeState,
                L"NpcController.ChangeState"
            },
            {
                context.npcAnimationManagerClass,
                L"NpcAnimationManager class"
            },
            {
                context.npcComponentAccessorGetAnimationManager,
                L"NpcComponentAccessor.get_NpcAnimationManager"
            },
            {
                context.npcAnimationGetCurrentMoveSpeed,
                L"NpcAnimationManager.GetCurrentMoveSpeed"
            },
            {
                context.npcAnimationSetMotionSpeed,
                L"NpcAnimationManager.SetMotionSpeed"
            },
            {
                context.npcAnimationSetWait,
                L"NpcAnimationManager.SetWait"
            },
            { context.routePointSetType, L"RoutePoint.set_Type" },
            {
                context.routePointSetInteractType,
                L"RoutePoint.set_RouteInteractType"
            },
            {
                context.routePointSetLinkPublisher,
                L"RoutePoint.set_LinkTokenPublisher"
            },
            { context.routePointGetFloor, L"RoutePoint.get_Floor" },
            { context.routePointSetFloor, L"RoutePoint.set_Floor" },
            {
                context.routePointSetEscalatorTarget,
                L"RoutePoint.set_EscalatorTarget"
            },
            {
                context.routePointSetMaxPathRightScale,
                L"RoutePoint.set_MaxPathRightScale"
            },
            { context.routePointSetParam, L"RoutePoint.set_Param" },
            {
                context.routePointSetPathRight,
                L"RoutePoint.set_PathRight"
            },
            {
                context.transformGetPosition,
                L"Transform.get_position"
            },
            {
                context.transformSetPosition,
                L"Transform.set_position"
            },
            {
                context.gameObjectGetTransform,
                L"GameObject.get_transform"
            },
            {
                context.gameObjectConstructor,
                L"GameObject..ctor(String)"
            },
            {
                context.playerClassAccessorGetTransform,
                L"PlayerClassAccessor.get_Transform"
            },
            {
                context.navMeshSamplePosition,
                L"NavMesh.SamplePosition"
            },
            {
                context.navMeshCalculatePath,
                L"NavMesh.CalculatePath"
            },
            {
                context.navPathConstructor,
                L"NavMeshPath..ctor"
            },
            {
                context.navPathGetStatus,
                L"NavMeshPath.get_status"
            },
            {
                context.animatorGetCurrentState,
                L"Animator.GetCurrentAnimatorStateInfo"
            },
            {
                context.animatorStringToHash,
                L"Animator.StringToHash"
            }
        };
        for (const auto& dependency : dependencies)
        {
            if (dependency.value == nullptr)
            {
                LogInfo(LogCategory_Action, L"Action", 
                    L"NPC pursuit missing dependency: %s.",
                    dependency.name);
            }
        }
        LogInfo(LogCategory_Action, L"Action", L"NPC pursuit disabled because required IL2CPP dependencies are unavailable.");
    }
    return context.valid;
}

bool GameplayActionPursuit_ResolveActionVisual(
    int actionType,
    int& layer,
    const char*& statePath)
{
    if (actionType < 50000 || actionType > 50022)
    {
        return false;
    }
    for (int index = 0; index < kEscapeFromManakaActionCount; ++index)
    {
        if (EscapeFromManaka_GetActionTypeId(index) != actionType)
        {
            continue;
        }
        const auto& definition = kEscapeFromManakaActions[index];
        if (definition.layer != kPursuitActionLayer ||
            definition.statePath == nullptr)
        {
            return false;
        }
        layer = definition.layer;
        statePath = definition.statePath;
        return true;
    }
    return false;
}

int GameplayActionPursuit_HashAnimatorState(const char* statePath)
{
    if (statePath == nullptr ||
        g_gameplayActionPursuitContext.animatorStringToHash == nullptr ||
        il2cpp_string_new == nullptr)
    {
        return 0;
    }
    auto* value = il2cpp_string_new(statePath);
    void* parameters[] = { value };
    auto* hashValue = Invoke(
        g_gameplayActionPursuitContext.animatorStringToHash,
        nullptr,
        parameters);
    int hash = 0;
    return GameplayActionPursuit_ReadBoxedInt(hashValue, hash) ? hash : 0;
}

GameplayActionPursuitAnimatorState
GameplayActionPursuit_ReadAnimatorState(int layer)
{
    GameplayActionPursuitAnimatorState output{};
    auto& context = g_gameplayActionPursuitContext;
    if (g_previewAnimator == nullptr ||
        context.animatorGetCurrentState == nullptr)
    {
        return output;
    }

    void* stateParameters[] = { &layer };
    auto* stateObject = Invoke(
        context.animatorGetCurrentState,
        g_previewAnimator,
        stateParameters);
    if (stateObject == nullptr || il2cpp_object_unbox == nullptr)
    {
        return output;
    }
    const auto* stateData =
        static_cast<const unsigned char*>(il2cpp_object_unbox(stateObject));
    output.fullPathHash =
        *reinterpret_cast<const int*>(stateData + 0x08);
    output.valid = true;
    return output;
}

bool GameplayActionPursuit_SampleTarget(
    const NativeVector3& desired,
    NativeVector3& sampled)
{
    auto& context = g_gameplayActionPursuitContext;
    if (context.navMeshSamplePosition == nullptr)
    {
        return false;
    }
    NativeVector3 query = desired;
    NativeNavMeshHit hit{};
    float maxDistance = kPursuitTargetSampleRadius;
    int areaMask = kPursuitNavMeshAreaMask;
    void* parameters[] = { &query, &hit, &maxDistance, &areaMask };
    bool found = false;
    auto* value = Invoke(
        context.navMeshSamplePosition,
        nullptr,
        parameters);
    if (!GameplayActionPursuit_ReadBoxedBool(value, found) || !found)
    {
        return false;
    }
    sampled = hit.position;
    return true;
}

bool GameplayActionPursuit_ValidatePath(
    const NativeVector3& source,
    const NativeVector3& target)
{
    auto& context = g_gameplayActionPursuitContext;
    if (il2cpp_object_new == nullptr ||
        context.navMeshCalculatePath == nullptr ||
        context.navPathConstructor == nullptr ||
        context.navPathGetStatus == nullptr)
    {
        return false;
    }
    auto* path = il2cpp_object_new(context.navMeshPathClass);
    if (path == nullptr)
    {
        return false;
    }
    uint32_t pathHandle = GameplayActionPursuit_CreateHandle(path);
    if (pathHandle == 0)
    {
        return false;
    }
    Invoke(context.navPathConstructor, path, nullptr);

    NativeVector3 sourcePosition = source;
    NativeVector3 destination = target;
    int areaMask = kPursuitNavMeshAreaMask;
    void* parameters[] = {
        &sourcePosition,
        &destination,
        &areaMask,
        path
    };
    bool calculated = false;
    auto* calculateValue = Invoke(
        context.navMeshCalculatePath,
        nullptr,
        parameters);
    const bool calculateRead = GameplayActionPursuit_ReadBoxedBool(
        calculateValue,
        calculated);
    auto* statusValue = Invoke(
        context.navPathGetStatus,
        path,
        nullptr);
    int status = -1;
    const bool statusRead = GameplayActionPursuit_ReadBoxedInt(
        statusValue,
        status);
    GameplayActionPursuit_FreeHandle(pathHandle);
    return calculateRead &&
        calculated &&
        statusRead &&
        status == 0;
}

float GameplayActionPursuit_GetPlayerCollisionRadius()
{
    auto& context = g_gameplayActionPursuitContext;
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    if (playerClassAccessor == nullptr ||
        context.playerClassAccessorGetCharacterController == nullptr ||
        context.characterControllerGetRadius == nullptr)
    {
        return kPursuitPlayerCollisionRadiusFallback;
    }
    auto* controller = Invoke(
        context.playerClassAccessorGetCharacterController,
        playerClassAccessor,
        nullptr);
    auto* radiusValue = controller != nullptr
        ? Invoke(context.characterControllerGetRadius, controller, nullptr)
        : nullptr;
    float radius = kPursuitPlayerCollisionRadiusFallback;
    return GameplayActionPursuit_ReadBoxedFloat(radiusValue, radius)
        ? radius
        : kPursuitPlayerCollisionRadiusFallback;
}

float GameplayActionPursuit_GetNpcCollisionRadius(
    Il2CppObject* navAgent)
{
    auto& context = g_gameplayActionPursuitContext;
    if (navAgent == nullptr || context.navAgentGetRadius == nullptr)
    {
        return 0.35f;
    }
    auto* radiusValue = Invoke(
        context.navAgentGetRadius,
        navAgent,
        nullptr);
    float radius = 0.35f;
    return GameplayActionPursuit_ReadBoxedFloat(radiusValue, radius)
        ? radius
        : 0.35f;
}

Il2CppObject* GameplayActionPursuit_GetNavAgent(Il2CppObject* npc)
{
    return GameplayActionPursuit_ReadObjectField(
        GameplayActionPursuit_ReadObjectField(npc, 0x30),
        0x28);
}

Il2CppObject* GameplayActionPursuit_GetNpcAnimationManager(
    Il2CppObject* npc)
{
    auto& context = g_gameplayActionPursuitContext;
    auto* accessor = GameplayActionPursuit_ReadObjectField(npc, 0x30);
    if (accessor == nullptr ||
        context.npcComponentAccessorGetAnimationManager == nullptr)
    {
        return nullptr;
    }
    return Invoke(
        context.npcComponentAccessorGetAnimationManager,
        accessor,
        nullptr);
}

void GameplayActionPursuit_SetNpcMotionSpeed(
    Il2CppObject* npc,
    float speed)
{
    auto& context = g_gameplayActionPursuitContext;
    auto* animationManager =
        GameplayActionPursuit_GetNpcAnimationManager(npc);
    if (animationManager == nullptr ||
        context.npcAnimationSetMotionSpeed == nullptr)
    {
        return;
    }
    void* parameters[] = { &speed };
    Invoke(
        context.npcAnimationSetMotionSpeed,
        animationManager,
        parameters);
}

void GameplayActionPursuit_SetNpcWait(
    Il2CppObject* npc,
    bool wait)
{
    auto& context = g_gameplayActionPursuitContext;
    auto* animationManager =
        GameplayActionPursuit_GetNpcAnimationManager(npc);
    if (animationManager == nullptr ||
        context.npcAnimationSetWait == nullptr)
    {
        return;
    }
    bool value = wait;
    void* parameters[] = { &value };
    Invoke(
        context.npcAnimationSetWait,
        animationManager,
        parameters);
}

bool GameplayActionPursuit_InvokeAgentBoolGetter(
    const Il2CppMethod* method,
    Il2CppObject* navAgent,
    bool& output)
{
    if (method == nullptr || navAgent == nullptr)
    {
        return false;
    }
    auto* value = Invoke(method, navAgent, nullptr);
    return GameplayActionPursuit_ReadBoxedBool(value, output);
}

bool GameplayActionPursuit_CaptureHoldPose(
    Il2CppObject* npc,
    GameplayActionPursuitNpcRecord& record);

bool GameplayActionPursuit_EnterHold(
    Il2CppObject* npc,
    GameplayActionPursuitNpcRecord& record)
{
    auto& context = g_gameplayActionPursuitContext;
    auto* navAgent = GameplayActionPursuit_GetNavAgent(npc);
    if (!g_gameplayActionPursuitPostUpdateHookReady ||
        navAgent == nullptr ||
        context.navAgentSetIsStopped == nullptr ||
        context.navAgentSetUpdateRotation == nullptr)
    {
        return false;
    }
    if (!GameplayActionPursuit_CaptureHoldPose(npc, record))
    {
        return false;
    }
    auto* animationManager =
        GameplayActionPursuit_GetNpcAnimationManager(npc);
    if (!record.hasSavedMotionSpeed &&
        animationManager != nullptr &&
        context.npcAnimationGetCurrentMoveSpeed != nullptr)
    {
        auto* speedValue = Invoke(
            context.npcAnimationGetCurrentMoveSpeed,
            animationManager,
            nullptr);
        float speed = 0.0f;
        if (GameplayActionPursuit_ReadBoxedFloat(speedValue, speed))
        {
            record.savedMotionSpeed = speed;
            record.hasSavedMotionSpeed = true;
        }
    }
    if (!record.hasSavedWaitState)
    {
        record.savedWaitState = false;
        record.hasSavedWaitState = true;
    }
    GameplayActionPursuit_SetNpcMotionSpeed(npc, 0.0f);
    GameplayActionPursuit_SetNpcWait(npc, true);
    if (!record.hasSavedAgentStopState)
    {
        bool isStopped = false;
        GameplayActionPursuit_InvokeAgentBoolGetter(
            context.navAgentGetIsStopped,
            navAgent,
            isStopped);
        record.savedAgentIsStopped = isStopped;
        record.hasSavedAgentStopState = true;
    }
    if (!record.hasSavedAgentPositionState)
    {
        bool updatePosition = true;
        GameplayActionPursuit_InvokeAgentBoolGetter(
            context.navAgentGetUpdatePosition,
            navAgent,
            updatePosition);
        record.savedAgentUpdatePosition = updatePosition;
        record.hasSavedAgentPositionState = true;
    }
    if (!record.hasSavedAgentRotationState)
    {
        bool updateRotation = true;
        GameplayActionPursuit_InvokeAgentBoolGetter(
            context.navAgentGetUpdateRotation,
            navAgent,
            updateRotation);
        record.savedAgentUpdateRotation = updateRotation;
        record.hasSavedAgentRotationState = true;
    }

    bool stopped = true;
    void* stopParameters[] = { &stopped };
    Invoke(context.navAgentSetIsStopped, navAgent, stopParameters);
    if (context.navAgentSetUpdatePosition != nullptr)
    {
        bool updatePosition = false;
        void* positionParameters[] = { &updatePosition };
        Invoke(
            context.navAgentSetUpdatePosition,
            navAgent,
            positionParameters);
    }
    bool updateRotation = false;
    void* rotationParameters[] = { &updateRotation };
    Invoke(
        context.navAgentSetUpdateRotation,
        navAgent,
        rotationParameters);
    if (!record.holding &&
        context.navAgentResetPath != nullptr)
    {
        Invoke(context.navAgentResetPath, navAgent, nullptr);
    }
    return true;
}

void GameplayActionPursuit_LeaveHold(
    Il2CppObject* npc,
    GameplayActionPursuitNpcRecord& record)
{
    auto& context = g_gameplayActionPursuitContext;
    auto* navAgent = GameplayActionPursuit_GetNavAgent(npc);
    if (navAgent == nullptr)
    {
        return;
    }
    if (record.hasSavedAgentPositionState &&
        context.navAgentSetUpdatePosition != nullptr)
    {
        bool updatePosition = record.savedAgentUpdatePosition;
        void* positionParameters[] = { &updatePosition };
        Invoke(
            context.navAgentSetUpdatePosition,
            navAgent,
            positionParameters);
    }
    if (record.hasSavedAgentRotationState &&
        context.navAgentSetUpdateRotation != nullptr)
    {
        bool updateRotation = record.savedAgentUpdateRotation;
        void* rotationParameters[] = { &updateRotation };
        Invoke(
            context.navAgentSetUpdateRotation,
            navAgent,
            rotationParameters);
    }
    if (record.hasSavedAgentStopState &&
        context.navAgentSetIsStopped != nullptr)
    {
        bool isStopped = record.savedAgentIsStopped;
        void* stopParameters[] = { &isStopped };
        Invoke(context.navAgentSetIsStopped, navAgent, stopParameters);
    }
    if (record.hasSavedMotionSpeed)
    {
        GameplayActionPursuit_SetNpcMotionSpeed(
            npc,
            record.savedMotionSpeed);
    }
    if (record.hasSavedWaitState)
    {
        GameplayActionPursuit_SetNpcWait(
            npc,
            record.savedWaitState);
    }
    record.hasSavedAgentStopState = false;
    record.hasSavedAgentPositionState = false;
    record.hasSavedAgentRotationState = false;
    record.hasHoldPose = false;
    record.hasSavedMotionSpeed = false;
    record.hasSavedWaitState = false;
}

bool GameplayActionPursuit_TargetIsFree(
    const NativeVector3& target,
    float candidateRadius,
    size_t ignoredRecordIndex)
{
    for (size_t index = 0;
         index < g_gameplayActionPursuitRecords.size();
         ++index)
    {
        if (index == ignoredRecordIndex)
        {
            continue;
        }
        const auto& record = g_gameplayActionPursuitRecords[index];
        const float dx = target.x - record.currentTarget.x;
        const float dz = target.z - record.currentTarget.z;
        const float distance = std::sqrt(dx * dx + dz * dz);
        if (distance <
            candidateRadius + record.collisionRadius + 0.30f)
        {
            return false;
        }
    }
    return true;
}

bool GameplayActionPursuit_TargetMovesTowardPlayer(
    const NativeVector3& playerPosition,
    const NativeVector3& npcPosition,
    const NativeVector3& candidate)
{
    const float playerDx = playerPosition.x - npcPosition.x;
    const float playerDz = playerPosition.z - npcPosition.z;
    const float playerLength =
        std::sqrt(playerDx * playerDx + playerDz * playerDz);
    const float candidateDx = candidate.x - npcPosition.x;
    const float candidateDz = candidate.z - npcPosition.z;
    const float candidateLength =
        std::sqrt(candidateDx * candidateDx + candidateDz * candidateDz);
    if (playerLength <= 0.05f || candidateLength <= 0.05f)
    {
        return false;
    }
    const float alignment =
        (playerDx * candidateDx + playerDz * candidateDz) /
        (playerLength * candidateLength);
    if (alignment < kPursuitApproachConeDot)
    {
        return false;
    }
    return GameplayActionPursuit_HorizontalDistance(
               candidate,
               playerPosition) <=
        GameplayActionPursuit_HorizontalDistance(
            npcPosition,
            playerPosition) -
            kPursuitMinimumInwardStep;
}

bool GameplayActionPursuit_ChooseQueueTarget(
    Il2CppObject* navAgent,
    const NativeVector3& npcPosition,
    size_t ignoredRecordIndex,
    NativeVector3& target)
{
    NativeVector3 playerPosition{};
    if (!GameplayActionPursuit_GetPlayerPosition(playerPosition) ||
        navAgent == nullptr)
    {
        return false;
    }
    const float npcDistance =
        GameplayActionPursuit_HorizontalDistance(
            npcPosition,
            playerPosition);
    if (npcDistance <= kPursuitHoldEnterDistance)
    {
        return false;
    }

    const float playerRadius =
        GameplayActionPursuit_GetPlayerCollisionRadius();
    const float candidateRadius =
        GameplayActionPursuit_GetNpcCollisionRadius(navAgent);
    const float minimumDistance = std::max(
        kPursuitApproachDistance,
        playerRadius + candidateRadius + 0.30f);
    const float maximumDistance = std::min(
        kPursuitMaxQueueDistance,
        npcDistance - kPursuitMinimumInwardStep);
    if (maximumDistance < minimumDistance)
    {
        return false;
    }

    const float baseAngle = std::atan2(
        npcPosition.z - playerPosition.z,
        npcPosition.x - playerPosition.x);
    auto tryCandidate = [&](float angle, float radius)
    {
        NativeVector3 desired{
            playerPosition.x + std::cos(angle) * radius,
            playerPosition.y,
            playerPosition.z + std::sin(angle) * radius
        };
        NativeVector3 sampled{};
        if (!GameplayActionPursuit_SampleTarget(desired, sampled))
        {
            return false;
        }
        const float sampledDistance =
            GameplayActionPursuit_HorizontalDistance(
                sampled,
                playerPosition);
        if (sampledDistance < minimumDistance - 0.05f ||
            std::fabs(sampled.y - playerPosition.y) > 0.8f ||
            !GameplayActionPursuit_TargetMovesTowardPlayer(
                playerPosition,
                npcPosition,
                sampled) ||
            !GameplayActionPursuit_TargetIsFree(
                sampled,
                candidateRadius,
                ignoredRecordIndex) ||
            !GameplayActionPursuit_ValidatePath(
                npcPosition,
                sampled))
        {
            return false;
        }
        target = sampled;
        return true;
    };

    // First keep the same approach bearing and queue farther out only for
    // NPCs that are still outside that position. This prevents a nearby NPC
    // from being pushed outward merely because the first ring is occupied.
    for (float radius = minimumDistance;
         radius <= maximumDistance;
         radius += 0.35f)
    {
        if (tryCandidate(baseAngle, radius))
        {
            return true;
        }
    }

    // If the direct corridor is blocked, try nearby bearings while keeping
    // the same monotonic requirement: the candidate must be closer to the
    // player than the NPC's current position.
    constexpr float kAngleStepDegrees = 10.0f;
    constexpr int kAngleSteps = 4;
    for (int step = 1; step <= kAngleSteps; ++step)
    {
        const float offset =
            step * kAngleStepDegrees *
            3.14159265358979323846f / 180.0f;
        for (int sign : { -1, 1 })
        {
            const float angle = baseAngle + offset * sign;
            for (float radius = minimumDistance;
                 radius <= maximumDistance;
                 radius += 0.35f)
            {
                if (tryCandidate(angle, radius))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

Il2CppObject* GameplayActionPursuit_FindSafeRouteTemplate(
    Il2CppObject* routeList,
    int count)
{
    auto& context = g_gameplayActionPursuitContext;
    for (int index = 0; index < count; ++index)
    {
        auto* point = GameplayActionPursuit_GetListItem(
            routeList,
            context.listGetItem,
            index);
        if (point == nullptr || !ExtendedMode_IsUnityObjectAlive(point))
        {
            continue;
        }
        return point;
    }
    return nullptr;
}

bool GameplayActionPursuit_SetTransformPosition(
    Il2CppObject* transform,
    const NativeVector3& position)
{
    auto& context = g_gameplayActionPursuitContext;
    if (transform == nullptr || context.transformSetPosition == nullptr)
    {
        return false;
    }
    NativeVector3 value = position;
    void* parameters[] = { &value };
    Invoke(context.transformSetPosition, transform, parameters);
    return true;
}

bool GameplayActionPursuit_SetTransformRotation(
    Il2CppObject* transform,
    const NativeQuaternion& rotation)
{
    auto& context = g_gameplayActionPursuitContext;
    if (transform == nullptr || context.transformSetRotation == nullptr)
    {
        return false;
    }
    NativeQuaternion value = rotation;
    void* parameters[] = { &value };
    Invoke(context.transformSetRotation, transform, parameters);
    return true;
}

bool GameplayActionPursuit_CaptureHoldPose(
    Il2CppObject* npc,
    GameplayActionPursuitNpcRecord& record)
{
    if (record.hasHoldPose)
    {
        return true;
    }
    auto& context = g_gameplayActionPursuitContext;
    if (npc == nullptr ||
        context.npcGetTransform == nullptr ||
        context.transformGetPosition == nullptr ||
        context.transformGetRotation == nullptr)
    {
        return false;
    }
    auto* transform = Invoke(
        context.npcGetTransform,
        npc,
        nullptr);
    if (transform == nullptr ||
        !GameplayActionPursuit_GetTransformPosition(
            transform,
            record.holdPosition))
    {
        return false;
    }
    auto* rotationValue = Invoke(
        context.transformGetRotation,
        transform,
        nullptr);
    if (!GameplayActionPursuit_ReadQuaternion(
            rotationValue,
            record.holdRotation))
    {
        return false;
    }
    record.hasHoldPose = true;
    return true;
}

void GameplayActionPursuit_EnforceHoldsAfterNpcUpdate()
{
    if (g_gameplayActionPursuitPhase != GameplayActionPursuit_Active)
    {
        return;
    }
    auto& context = g_gameplayActionPursuitContext;
    if (context.npcGetTransform == nullptr ||
        context.transformSetPosition == nullptr ||
        context.transformSetRotation == nullptr)
    {
        return;
    }
    for (auto& record : g_gameplayActionPursuitRecords)
    {
        if (!record.holding || !record.hasHoldPose)
        {
            continue;
        }
        auto* npc = GameplayActionPursuit_GetLiveObject(record.npcHandle);
        if (npc == nullptr || !ExtendedMode_IsUnityObjectAlive(npc))
        {
            continue;
        }
        auto* transform = Invoke(
            context.npcGetTransform,
            npc,
            nullptr);
        if (transform == nullptr)
        {
            continue;
        }
        GameplayActionPursuit_SetTransformPosition(
            transform,
            record.holdPosition);
        GameplayActionPursuit_SetTransformRotation(
            transform,
            record.holdRotation);
        GameplayActionPursuit_SetNpcMotionSpeed(npc, 0.0f);
        GameplayActionPursuit_SetNpcWait(npc, true);
    }
}

bool GameplayActionPursuit_ConfigureTemporaryPoint(
    Il2CppObject* point,
    Il2CppObject* safeTemplate)
{
    auto& context = g_gameplayActionPursuitContext;
    if (point == nullptr || safeTemplate == nullptr)
    {
        return false;
    }

    int floor = 0;
    auto* floorValue = Invoke(
        context.routePointGetFloor,
        safeTemplate,
        nullptr);
    GameplayActionPursuit_ReadBoxedInt(floorValue, floor);

    int type = 0;
    int interactType = 0;
    float maxPathRightScale = 0.0f;
    int parameter = 0;
    NativeVector3 pathRight{};
    Il2CppObject* nullObject = nullptr;
    void* typeParameters[] = { &type };
    void* interactTypeParameters[] = { &interactType };
    void* nullParameters[] = { nullObject };
    void* floorParameters[] = { &floor };
    void* maxParameters[] = { &maxPathRightScale };
    void* parameterParameters[] = { &parameter };
    void* pathParameters[] = { &pathRight };
    Invoke(context.routePointSetType, point, typeParameters);
    Invoke(
        context.routePointSetInteractType,
        point,
        interactTypeParameters);
    Invoke(context.routePointSetLinkPublisher, point, nullParameters);
    Invoke(context.routePointSetFloor, point, floorParameters);
    Invoke(context.routePointSetEscalatorTarget, point, nullParameters);
    Invoke(
        context.routePointSetMaxPathRightScale,
        point,
        maxParameters);
    Invoke(context.routePointSetParam, point, parameterParameters);
    Invoke(context.routePointSetPathRight, point, pathParameters);
    return true;
}

bool GameplayActionPursuit_IsRecordForNpc(
    const GameplayActionPursuitNpcRecord& record,
    Il2CppObject* npc)
{
    return GameplayActionPursuit_GetLiveObject(record.npcHandle) == npc;
}

bool GameplayActionPursuit_HasRecordForNpc(Il2CppObject* npc)
{
    return std::any_of(
        g_gameplayActionPursuitRecords.begin(),
        g_gameplayActionPursuitRecords.end(),
        [npc](const auto& record)
        {
            return GameplayActionPursuit_IsRecordForNpc(record, npc);
        });
}

bool GameplayActionPursuit_RouteDataIsOwned(Il2CppObject* routeData)
{
    return std::any_of(
        g_gameplayActionPursuitRecords.begin(),
        g_gameplayActionPursuitRecords.end(),
        [routeData](const auto& record)
        {
            return GameplayActionPursuit_GetLiveObject(
                record.routeDataHandle) == routeData;
        });
}

void GameplayActionPursuit_DestroyTemporaryObject(
    GameplayActionPursuitNpcRecord& record)
{
    auto* gameObject = GameplayActionPursuit_GetLiveObject(
        record.temporaryGameObjectHandle);
    if (gameObject != nullptr &&
        ExtendedMode_IsUnityObjectAlive(gameObject))
    {
        auto* objectClass = FindClass("UnityEngine", "Object");
        auto* destroy = FindMethod(objectClass, "Destroy", 1);
        if (destroy != nullptr)
        {
            void* parameters[] = { gameObject };
            Invoke(destroy, nullptr, parameters);
        }
    }
}

void GameplayActionPursuit_ReleaseRecord(
    size_t recordIndex,
    const wchar_t* reason,
    bool restoreIdle = true)
{
    if (recordIndex >= g_gameplayActionPursuitRecords.size())
    {
        return;
    }
    auto record = g_gameplayActionPursuitRecords[recordIndex];
    auto* npc = GameplayActionPursuit_GetLiveObject(record.npcHandle);
    auto* routeData = GameplayActionPursuit_GetLiveObject(
        record.routeDataHandle);
    auto* originalRouteList = GameplayActionPursuit_GetLiveObject(
        record.originalRouteListHandle);
    auto* temporaryRouteList = GameplayActionPursuit_GetLiveObject(
        record.temporaryRouteListHandle);
    if (npc != nullptr)
    {
        g_gameplayActionPursuitSuppressedNpcs.insert(npc);
    }
    if (record.holding && npc != nullptr)
    {
        GameplayActionPursuit_LeaveHold(npc, record);
        record.holding = false;
    }

    if (routeData != nullptr &&
        originalRouteList != nullptr &&
        temporaryRouteList != nullptr &&
        GameplayActionPursuit_ReadObjectField(routeData, 0x30) ==
            temporaryRouteList)
    {
        SetManagedObjectField(
            g_gameplayActionPursuitContext.routeDataClass,
            routeData,
            "routeList",
            originalRouteList);
        int restoredIndex = record.savedCurrentIndex;
        const int originalCount = GameplayActionPursuit_GetListCount(
            originalRouteList,
            g_gameplayActionPursuitContext.listGetCount);
        if (originalCount > 0)
        {
            restoredIndex = std::clamp(
                restoredIndex,
                0,
                originalCount - 1);
        }
        else
        {
            restoredIndex = 0;
        }
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(routeData) + 0x3C) =
            restoredIndex;
    }
    if (restoreIdle &&
        npc != nullptr &&
        ExtendedMode_IsUnityObjectAlive(npc) &&
        g_gameplayActionPursuitContext.npcChangeState != nullptr)
    {
        GameplayActionPursuit_SetNpcState(npc, kNpcStateIdle);
    }

    LogInfo(LogCategory_Action, L"Action", 
        L"NPC pursuit lease released: npc=%p reason=%s.",
        npc,
        reason != nullptr ? reason : L"unknown");
    GameplayActionPursuit_DestroyTemporaryObject(record);
    GameplayActionPursuit_FreeHandle(record.temporaryGameObjectHandle);
    GameplayActionPursuit_FreeHandle(record.temporaryRoutePointHandle);
    GameplayActionPursuit_FreeHandle(record.temporaryRouteListHandle);
    GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
    GameplayActionPursuit_FreeHandle(record.routeDataHandle);
    GameplayActionPursuit_FreeHandle(record.npcHandle);
    g_gameplayActionPursuitRecords.erase(
        g_gameplayActionPursuitRecords.begin() +
        static_cast<std::ptrdiff_t>(recordIndex));
    g_gameplayActionPursuitQueueDirty = true;
}

void GameplayActionPursuit_ReleaseAll(
    const wchar_t* reason,
    bool restoreIdle = true)
{
    while (!g_gameplayActionPursuitRecords.empty())
    {
        GameplayActionPursuit_ReleaseRecord(
            g_gameplayActionPursuitRecords.size() - 1,
            reason,
            restoreIdle);
    }
}

bool GameplayActionPursuit_BeginLease(
    Il2CppObject* npc,
    Il2CppObject* routeData,
    Il2CppObject* originalRouteList,
    int originalListCount,
    Il2CppObject* safeTemplate,
    int queueIndex,
    float collisionRadius,
    const NativeVector3& target,
    bool beginHolding)
{
    auto& context = g_gameplayActionPursuitContext;
    if (npc == nullptr ||
        routeData == nullptr ||
        originalRouteList == nullptr ||
        safeTemplate == nullptr ||
        il2cpp_object_new == nullptr)
    {
        return false;
    }
    if (GameplayActionPursuit_ReadObjectField(routeData, 0x30) !=
        originalRouteList)
    {
        return false;
    }

    GameplayActionPursuitNpcRecord record{};
    record.sessionId = g_gameplayActionPursuitSessionId;
    record.savedCurrentIndex =
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(routeData) + 0x3C);
    record.originalListCount = originalListCount;
    record.queueIndex = queueIndex;
    record.collisionRadius = collisionRadius;
    record.currentTarget = target;

    record.npcHandle = GameplayActionPursuit_CreateHandle(npc);
    record.routeDataHandle =
        GameplayActionPursuit_CreateHandle(routeData);
    record.originalRouteListHandle =
        GameplayActionPursuit_CreateHandle(originalRouteList);
    if (record.npcHandle == 0 ||
        record.routeDataHandle == 0 ||
        record.originalRouteListHandle == 0)
    {
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }

    auto* gameObject = il2cpp_object_new(context.gameObjectClass);
    if (gameObject == nullptr)
    {
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }
    record.temporaryGameObjectHandle =
        GameplayActionPursuit_CreateHandle(gameObject);
    if (record.temporaryGameObjectHandle == 0)
    {
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }
    auto* objectName =
        il2cpp_string_new("EscapeFromManakaPursuitTarget");
    void* constructorParameters[] = { objectName };
    Invoke(
        context.gameObjectConstructor,
        gameObject,
        constructorParameters);

    auto* routePoint = AddSceneComponentToGameObject(
        gameObject,
        "ExposureUnnoticed2.Object3D.RoutePoint",
        "RoutePoint");
    if (routePoint == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(routePoint))
    {
        GameplayActionPursuit_DestroyTemporaryObject(record);
        GameplayActionPursuit_FreeHandle(record.temporaryGameObjectHandle);
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }
    record.temporaryRoutePointHandle =
        GameplayActionPursuit_CreateHandle(routePoint);
    if (record.temporaryRoutePointHandle == 0 ||
        !GameplayActionPursuit_ConfigureTemporaryPoint(
            routePoint,
            safeTemplate))
    {
        GameplayActionPursuit_DestroyTemporaryObject(record);
        GameplayActionPursuit_FreeHandle(
            record.temporaryRoutePointHandle);
        GameplayActionPursuit_FreeHandle(record.temporaryGameObjectHandle);
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }

    auto* transform = Invoke(
        context.gameObjectGetTransform,
        gameObject,
        nullptr);
    if (transform == nullptr ||
        !GameplayActionPursuit_SetTransformPosition(transform, target))
    {
        GameplayActionPursuit_DestroyTemporaryObject(record);
        GameplayActionPursuit_FreeHandle(
            record.temporaryRoutePointHandle);
        GameplayActionPursuit_FreeHandle(record.temporaryGameObjectHandle);
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }

    auto* temporaryList = il2cpp_object_new(
        context.routePointListClass);
    if (temporaryList == nullptr)
    {
        GameplayActionPursuit_DestroyTemporaryObject(record);
        GameplayActionPursuit_FreeHandle(
            record.temporaryRoutePointHandle);
        GameplayActionPursuit_FreeHandle(record.temporaryGameObjectHandle);
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }
    record.temporaryRouteListHandle =
        GameplayActionPursuit_CreateHandle(temporaryList);
    if (record.temporaryRouteListHandle == 0)
    {
        GameplayActionPursuit_DestroyTemporaryObject(record);
        GameplayActionPursuit_FreeHandle(
            record.temporaryRoutePointHandle);
        GameplayActionPursuit_FreeHandle(record.temporaryGameObjectHandle);
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }
    Invoke(context.listConstructor, temporaryList, nullptr);

    void* addParameters[] = { routePoint };
    for (int copy = 0; copy < 3; ++copy)
    {
        Invoke(context.listAdd, temporaryList, addParameters);
    }
    const int temporaryCount = GameplayActionPursuit_GetListCount(
        temporaryList,
        context.listGetCount);
    if (temporaryCount != 3 ||
        !SetManagedObjectField(
            context.routeDataClass,
            routeData,
            "routeList",
            temporaryList))
    {
        GameplayActionPursuit_DestroyTemporaryObject(record);
        GameplayActionPursuit_FreeHandle(
            record.temporaryRouteListHandle);
        GameplayActionPursuit_FreeHandle(
            record.temporaryRoutePointHandle);
        GameplayActionPursuit_FreeHandle(record.temporaryGameObjectHandle);
        GameplayActionPursuit_FreeHandle(record.originalRouteListHandle);
        GameplayActionPursuit_FreeHandle(record.routeDataHandle);
        GameplayActionPursuit_FreeHandle(record.npcHandle);
        return false;
    }
    *reinterpret_cast<int*>(
        reinterpret_cast<char*>(routeData) + 0x3C) = 0;
    GameplayActionPursuit_SetNpcState(npc, kNpcStateIdle);
    if (beginHolding)
    {
        record.holding =
            GameplayActionPursuit_EnterHold(npc, record);
    }

    NativeVector3 currentNpcPosition{};
    if (GameplayActionPursuit_GetNpcPosition(npc, currentNpcPosition))
    {
        const float dx = currentNpcPosition.x - target.x;
        const float dy = currentNpcPosition.y - target.y;
        const float dz = currentNpcPosition.z - target.z;
        record.bestDistanceToTarget =
            std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    record.lastProgressTick = GetTickCount64();
    record.lastTargetRefreshTick = GetTickCount64();
    record.lastProgressCheckTick = GetTickCount64();
    g_gameplayActionPursuitRecords.push_back(record);
    LogInfo(LogCategory_Action, L"Action", 
        L"NPC pursuit lease started: npc=%p route=%p queue=%d mode=%s target=(%.2f, %.2f, %.2f).",
        npc,
        routeData,
        queueIndex,
        record.holding ? L"hold" : L"approach",
        target.x,
        target.y,
        target.z);
    return true;
}

bool GameplayActionPursuit_TryCaptureNpc(
    Il2CppObject* npc,
    bool requireSeen)
{
    if (!GameplayActionPursuit_EnsureContext() ||
        npc == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(npc) ||
        GameplayActionPursuit_HasRecordForNpc(npc) ||
        g_gameplayActionPursuitSuppressedNpcs.find(npc) !=
            g_gameplayActionPursuitSuppressedNpcs.end() ||
        static_cast<int>(g_gameplayActionPursuitRecords.size()) >=
            kPursuitMaxNpcs)
    {
        return false;
    }

    bool isSeen = false;
    int currentState = -1;
    int fixedType = -1;
    bool isSit = false;
    const bool seenRead = GameplayActionPursuit_ReadSeen(npc, isSeen);
    if ((requireSeen && (!seenRead || !isSeen)) ||
        !GameplayActionPursuit_ReadNpcState(
            npc,
            currentState,
            fixedType,
            isSit) ||
        (currentState != kNpcStateIdle &&
         currentState != kNpcStateFoundStranger) ||
        fixedType != kNpcFixedTypeNone ||
        isSit)
    {
        if (isSeen || !requireSeen)
        {
            GameplayActionPursuit_LogSeenSkip(
                npc,
                L"not in an eligible NPC movement state",
                currentState,
                fixedType,
                isSit);
        }
        return false;
    }

    auto* nca = GameplayActionPursuit_ReadObjectField(npc, 0x30);
    auto* stateModel = GameplayActionPursuit_ReadObjectField(npc, 0x48);
    auto* navAgent = GameplayActionPursuit_ReadObjectField(nca, 0x28);
    auto* routeData = GameplayActionPursuit_ReadObjectField(
        stateModel,
        0x40);
    if (navAgent == nullptr || routeData == nullptr)
    {
        GameplayActionPursuit_LogSeenSkip(
            npc,
            L"NavMeshAgent or RouteData missing",
            currentState,
            fixedType,
            isSit);
        return false;
    }
    auto* routeDataClass = il2cpp_object_get_class(routeData);
    if (routeDataClass != g_gameplayActionPursuitContext.routeDataClass ||
        GameplayActionPursuit_RouteDataIsOwned(routeData))
    {
        GameplayActionPursuit_LogSeenSkip(
            npc,
            routeDataClass != g_gameplayActionPursuitContext.routeDataClass
                ? L"RouteData runtime class is not the base RouteData"
                : L"RouteData is already owned by another lease",
            currentState,
            fixedType,
            isSit);
        return false;
    }

    auto* originalRouteList =
        GameplayActionPursuit_ReadObjectField(routeData, 0x30);
    if (originalRouteList == nullptr ||
        !GameplayActionPursuit_RefreshRouteListMethods(
            il2cpp_object_get_class(originalRouteList)))
    {
        GameplayActionPursuit_LogSeenSkip(
            npc,
            L"routeList/List<RoutePoint> methods unavailable",
            currentState,
            fixedType,
            isSit);
        return false;
    }
    const int originalCount = GameplayActionPursuit_GetListCount(
        originalRouteList,
        g_gameplayActionPursuitContext.listGetCount);
    if (originalCount <= 0)
    {
        GameplayActionPursuit_LogSeenSkip(
            npc,
            L"routeList is empty",
            currentState,
            fixedType,
            isSit);
        return false;
    }
    auto* safeTemplate = GameplayActionPursuit_FindSafeRouteTemplate(
        originalRouteList,
        originalCount);
    if (safeTemplate == nullptr)
    {
        GameplayActionPursuit_LogSeenSkip(
            npc,
            L"no alive RoutePoint template",
            currentState,
            fixedType,
            isSit);
        return false;
    }

    NativeVector3 playerPosition{};
    NativeVector3 npcPosition{};
    if (!GameplayActionPursuit_GetPlayerPosition(playerPosition) ||
        !GameplayActionPursuit_GetNpcPosition(npc, npcPosition))
    {
        GameplayActionPursuit_LogSeenSkip(
            npc,
            L"player/NPC position unavailable",
            currentState,
            fixedType,
            isSit);
        return false;
    }
    const bool beginHolding =
        GameplayActionPursuit_HorizontalDistance(
            npcPosition,
            playerPosition) <=
        kPursuitHoldEnterDistance;
    NativeVector3 target{};
    const bool targetReady = beginHolding
        ? GameplayActionPursuit_BuildHoldAnchor(
            npc,
            npcPosition,
            npcPosition,
            target)
        : GameplayActionPursuit_ChooseQueueTarget(
            navAgent,
            npcPosition,
            g_gameplayActionPursuitRecords.size(),
            target);
    if (targetReady &&
        GameplayActionPursuit_BeginLease(
            npc,
            routeData,
            originalRouteList,
            originalCount,
            safeTemplate,
            static_cast<int>(g_gameplayActionPursuitRecords.size()),
            GameplayActionPursuit_GetNpcCollisionRadius(navAgent),
            target,
            beginHolding))
    {
        return true;
    }
    GameplayActionPursuit_LogSeenSkip(
        npc,
        L"no inward NavMesh target or temporary lease creation failed");
    return false;
}

void GameplayActionPursuit_ScanSeenNpcs()
{
    if (!GameplayActionPursuit_EnsureContext() ||
        g_gameplayActionPursuitRecords.size() >= kPursuitMaxNpcs)
    {
        return;
    }
    auto* manager = Invoke(
        g_gameplayActionPursuitContext.npcManagerGetInstance,
        nullptr,
        nullptr);
    auto* npcList = GameplayActionPursuit_ReadObjectField(
        manager,
        0x38);
    if (npcList == nullptr)
    {
        return;
    }
    auto* listClass = il2cpp_object_get_class(npcList);
    auto* countMethod = FindMethod(listClass, "get_Count", 0);
    auto* itemMethod = FindMethod(listClass, "get_Item", 1);
    const int count = GameplayActionPursuit_GetListCount(
        npcList,
        countMethod);
    for (int index = 0;
         index < count &&
         g_gameplayActionPursuitRecords.size() < kPursuitMaxNpcs;
         ++index)
    {
        auto* npc = GameplayActionPursuit_GetListItem(
            npcList,
            itemMethod,
            index);
        if (npc == nullptr ||
            GameplayActionPursuit_HasRecordForNpc(npc))
        {
            continue;
        }
        GameplayActionPursuit_TryCaptureNpc(npc, true);
    }
}

void GameplayActionPursuit_RefreshRecords()
{
    const unsigned long long now = GetTickCount64();
    for (size_t index = 0; index < g_gameplayActionPursuitRecords.size();)
    {
        auto& record = g_gameplayActionPursuitRecords[index];
        auto* npc = GameplayActionPursuit_GetLiveObject(record.npcHandle);
        auto* routeData = GameplayActionPursuit_GetLiveObject(
            record.routeDataHandle);
        auto* routePoint = GameplayActionPursuit_GetLiveObject(
            record.temporaryRoutePointHandle);
        auto* temporaryList = GameplayActionPursuit_GetLiveObject(
            record.temporaryRouteListHandle);
        if (npc == nullptr ||
            !ExtendedMode_IsUnityObjectAlive(npc) ||
            routeData == nullptr ||
            routePoint == nullptr ||
            !ExtendedMode_IsUnityObjectAlive(routePoint) ||
            temporaryList == nullptr ||
            GameplayActionPursuit_ReadObjectField(routeData, 0x30) !=
                temporaryList)
        {
            GameplayActionPursuit_ReleaseRecord(index, L"object became stale");
            continue;
        }

        int currentState = -1;
        int fixedType = -1;
        bool isSit = false;
        if (!GameplayActionPursuit_ReadNpcState(
                npc,
                currentState,
                fixedType,
                isSit) ||
            fixedType != kNpcFixedTypeNone ||
            isSit)
        {
            GameplayActionPursuit_ReleaseRecord(
                index,
                L"npc state changed",
                false);
            continue;
        }
        if (currentState != kNpcStateIdle)
        {
            GameplayActionPursuit_ReleaseRecord(
                index,
                L"npc entered original state",
                false);
            continue;
        }

        if (now - record.lastTargetRefreshTick >=
            kPursuitTargetRefreshMs)
        {
            NativeVector3 playerPosition{};
            NativeVector3 npcPosition{};
            if (GameplayActionPursuit_GetPlayerPosition(playerPosition) &&
                GameplayActionPursuit_GetNpcPosition(npc, npcPosition))
            {
                const float playerDistance =
                    GameplayActionPursuit_HorizontalDistance(
                        npcPosition,
                        playerPosition);
                const bool playerMoved =
                    !record.hasPlayerPosition ||
                    GameplayActionPursuit_HorizontalDistance(
                        playerPosition,
                        record.lastPlayerPosition) >=
                        kPursuitPlayerMoveRefreshDistance;
                bool chooseNow = false;
                const bool shouldHold = record.holding
                    ? playerDistance <= kPursuitHoldExitDistance
                    : playerDistance <= kPursuitHoldEnterDistance;
                if (shouldHold && !record.holding)
                {
                    NativeVector3 holdAnchor{};
                    if (GameplayActionPursuit_BuildHoldAnchor(
                            npc,
                            npcPosition,
                            record.currentTarget,
                            holdAnchor) &&
                        GameplayActionPursuit_EnterHold(npc, record))
                    {
                        record.holding = true;
                        record.currentTarget = holdAnchor;
                        auto* transform = Invoke(
                            g_gameplayActionPursuitContext
                                .gameObjectGetTransform,
                            GameplayActionPursuit_GetLiveObject(
                                record.temporaryGameObjectHandle),
                            nullptr);
                        GameplayActionPursuit_SetTransformPosition(
                            transform,
                            holdAnchor);
                        record.bestDistanceToTarget = 0.0f;
                        record.lastProgressTick = now;
                        LogInfo(LogCategory_Action, L"Action", 
                            L"NPC pursuit entered hold: npc=%p playerDistance=%.2f anchor=(%.2f, %.2f, %.2f).",
                            npc,
                            playerDistance,
                            holdAnchor.x,
                            holdAnchor.y,
                            holdAnchor.z);
                    }
                }
                if (shouldHold && record.holding)
                {
                    GameplayActionPursuit_EnterHold(npc, record);
                }
                else if (!shouldHold && record.holding)
                {
                    GameplayActionPursuit_LeaveHold(npc, record);
                    record.holding = false;
                    chooseNow = true;
                    LogInfo(LogCategory_Action, L"Action", 
                        L"NPC pursuit left hold to approach: npc=%p playerDistance=%.2f.",
                        npc,
                        playerDistance);
                }

                if (!record.holding &&
                    (chooseNow ||
                     playerMoved ||
                     g_gameplayActionPursuitQueueDirty))
                {
                    NativeVector3 newTarget{};
                    auto* navAgent =
                        GameplayActionPursuit_GetNavAgent(npc);
                    const bool foundTarget =
                        GameplayActionPursuit_ChooseQueueTarget(
                            navAgent,
                            npcPosition,
                            index,
                            newTarget);
                    if (foundTarget)
                    {
                        const float targetDelta =
                            GameplayActionPursuit_HorizontalDistance(
                                newTarget,
                                record.currentTarget);
                        if (targetDelta >=
                            kPursuitTargetUpdateDistance)
                        {
                            auto* transform = Invoke(
                                g_gameplayActionPursuitContext
                                    .gameObjectGetTransform,
                                GameplayActionPursuit_GetLiveObject(
                                    record.temporaryGameObjectHandle),
                                nullptr);
                            if (GameplayActionPursuit_SetTransformPosition(
                                    transform,
                                    newTarget))
                            {
                                record.currentTarget = newTarget;
                                record.bestDistanceToTarget =
                                    GameplayActionPursuit_HorizontalDistance(
                                        npcPosition,
                                        newTarget);
                                record.lastProgressTick = now;
                                LogInfo(LogCategory_Action, L"Action", 
                                    L"NPC pursuit queue target updated: npc=%p queue=%d target=(%.2f, %.2f, %.2f).",
                                    npc,
                                    record.queueIndex,
                                    newTarget.x,
                                    newTarget.y,
                                    newTarget.z);
                            }
                        }
                    }
                }
                record.lastPlayerPosition = playerPosition;
                record.hasPlayerPosition = true;
            }
            record.lastTargetRefreshTick = now;
        }

        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(routeData) + 0x3C) = 0;
        if (record.holding)
        {
            record.lastProgressTick = now;
            record.lastProgressCheckTick = now;
            ++index;
            continue;
        }

        if (now - record.lastProgressCheckTick >=
            kPursuitProgressCheckMs)
        {
            NativeVector3 npcPosition{};
            if (GameplayActionPursuit_GetNpcPosition(npc, npcPosition))
            {
                const float dx = npcPosition.x - record.currentTarget.x;
                const float dy = npcPosition.y - record.currentTarget.y;
                const float dz = npcPosition.z - record.currentTarget.z;
                const float distance =
                    std::sqrt(dx * dx + dy * dy + dz * dz);
                if (distance <= kPursuitArrivalDistance)
                {
                    record.bestDistanceToTarget = distance;
                    record.lastProgressTick = now;
                }
                else if (distance + kPursuitMinimumProgressMeters <
                    record.bestDistanceToTarget)
                {
                    record.bestDistanceToTarget = distance;
                    record.lastProgressTick = now;
                }
            }
            record.lastProgressCheckTick = now;
        }

        if (now - record.lastProgressTick >=
            kPursuitProgressTimeoutMs)
        {
            GameplayActionPursuit_ReleaseRecord(index, L"no movement progress");
            continue;
        }
        ++index;
    }
    g_gameplayActionPursuitQueueDirty = false;
}

void GameplayActionPursuit_EndSession(
    const wchar_t* reason,
    bool restoreIdle = true)
{
    if (g_gameplayActionPursuitPhase != GameplayActionPursuit_Disabled ||
        !g_gameplayActionPursuitRecords.empty())
    {
        GameplayActionPursuit_ReleaseAll(reason, restoreIdle);
        LogInfo(LogCategory_Action, L"Action", 
            L"NPC pursuit session ended: reason=%s.",
            reason != nullptr ? reason : L"unknown");
    }
    g_gameplayActionPursuitPhase = GameplayActionPursuit_Disabled;
    g_gameplayActionPursuitRequestedType = -1;
    g_gameplayActionPursuitConfirmedType = -1;
    g_gameplayActionPursuitExpectedStateHash = 0;
    g_gameplayActionPursuitMatchingFrames = 0;
    g_gameplayActionPursuitMissingFrames = 0;
    g_gameplayActionPursuitQueueDirty = false;
    g_gameplayActionPursuitSuppressedNpcs.clear();
    g_gameplayActionPursuitLoggedSkips.clear();
}

void GameplayActionPursuit_Reset()
{
    GameplayActionPursuit_EndSession(L"scene or mode reset");
    g_gameplayActionPursuitRequestTick = 0;
    g_gameplayActionPursuitLastScanTick = 0;
}

void GameplayActionPursuit_ProbeDependencies()
{
    const bool ready = GameplayActionPursuit_EnsureContext();
    if (ready)
    {
        LogInfo(LogCategory_Action, L"Action", L"NPC pursuit IL2CPP dependency probe succeeded.");
        LogInfo(LogCategory_Action, L"Action", 
            L"NPC pursuit hold API probe: stopGet=%p stopSet=%p rotationGet=%p rotationSet=%p resetPath=%p.",
            g_gameplayActionPursuitContext.navAgentGetIsStopped,
            g_gameplayActionPursuitContext.navAgentSetIsStopped,
            g_gameplayActionPursuitContext.navAgentGetUpdateRotation,
            g_gameplayActionPursuitContext.navAgentSetUpdateRotation,
            g_gameplayActionPursuitContext.navAgentResetPath);
    }
    // Force a fresh runtime-class check when gameplay actually starts.
    g_gameplayActionPursuitContext = GameplayActionPursuitContext{};
}

void GameplayActionPursuit_RequestFromEMenu(int actionType)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        !GameplayActionPursuit_EnsureContext())
    {
        return;
    }
    if (actionType < 50000 || actionType > 50022)
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"NPC pursuit request rejected for unexpected action type %d.",
            actionType);
        return;
    }

    GameplayActionPursuit_EndSession(L"new E action requested");
    g_gameplayActionPursuitRequestedType = actionType;
    g_gameplayActionPursuitExpectedLayer = kPursuitActionLayer;
    g_gameplayActionPursuitExpectedStateHash = 0;
    g_gameplayActionPursuitMatchingFrames = 0;
    g_gameplayActionPursuitMissingFrames = 0;
    g_gameplayActionPursuitRequestTick = GetTickCount64();
    g_gameplayActionPursuitPhase =
        GameplayActionPursuit_WaitingForAnimation;
    LogInfo(LogCategory_Action, L"Action", 
        L"NPC pursuit waiting for existing action listener: action=%d.",
        actionType);
}

void GameplayActionPursuit_StartSession()
{
    ++g_gameplayActionPursuitSessionId;
    if (g_gameplayActionPursuitSessionId == 0)
    {
        g_gameplayActionPursuitSessionId = 1;
    }
    g_gameplayActionPursuitConfirmedType =
        g_gameplayActionPursuitRequestedType;
    g_gameplayActionPursuitMatchingFrames = 0;
    g_gameplayActionPursuitMissingFrames = 0;
    g_gameplayActionPursuitLastScanTick = 0;
    g_gameplayActionPursuitSuppressedNpcs.clear();
    g_gameplayActionPursuitLoggedSkips.clear();
    g_gameplayActionPursuitPhase = GameplayActionPursuit_Active;
    LogInfo(LogCategory_Action, L"Action", 
        L"NPC pursuit active: action=%d session=%u.",
        g_gameplayActionPursuitConfirmedType,
        g_gameplayActionPursuitSessionId);
    GameplayActionPursuit_ScanSeenNpcs();
}

void GameplayActionPursuit_Tick()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        NpcDialogue_IsActive())
    {
        GameplayActionPursuit_EndSession(
            g_extendedModeState.load() != ExtendedMode_Gameplay
                ? L"left gameplay"
                : L"npc dialogue active");
        return;
    }
    if (!GameplayActionPursuit_EnsureContext())
    {
        GameplayActionPursuit_EndSession(L"pursuit dependencies unavailable");
        return;
    }

    const auto action = GameplayAction_ReadCurrent();
    if (g_gameplayActionPursuitPhase ==
        GameplayActionPursuit_WaitingForAnimation)
    {
        const bool matches =
            action.active &&
            action.actionType ==
                g_gameplayActionPursuitRequestedType;
        if (matches)
        {
            GameplayActionPursuit_StartSession();
        }
        else if (GetTickCount64() -
                g_gameplayActionPursuitRequestTick >=
            kPursuitAnimationTimeoutMs)
        {
            LogInfo(LogCategory_Action, L"Action", 
                L"NPC pursuit action listener timeout: action=%d current=%d.",
                g_gameplayActionPursuitRequestedType,
                action.actionType);
            GameplayActionPursuit_EndSession(
                L"existing action listener confirmation timeout");
        }
        return;
    }

    if (g_gameplayActionPursuitPhase != GameplayActionPursuit_Active)
    {
        return;
    }

    const bool actionChanged =
        !action.active ||
        action.actionType !=
            g_gameplayActionPursuitConfirmedType;
    if (actionChanged)
    {
        ++g_gameplayActionPursuitMissingFrames;
        if (g_gameplayActionPursuitMissingFrames >= 3)
        {
            GameplayActionPursuit_EndSession(L"E animation ended");
        }
        return;
    }

    g_gameplayActionPursuitMissingFrames = 0;
    if (GameplayActionPursuit_ReadPlayerRuntimeState() == 1)
    {
        GameplayActionPursuit_EndSession(
            L"original player found state",
            false);
        return;
    }
    const unsigned long long now = GetTickCount64();
    if (g_gameplayActionPursuitLastScanTick == 0 ||
        now - g_gameplayActionPursuitLastScanTick >=
            kPursuitScanIntervalMs)
    {
        GameplayActionPursuit_ScanSeenNpcs();
        g_gameplayActionPursuitLastScanTick = now;
    }
    GameplayActionPursuit_RefreshRecords();
}

void PlayerControllerOnFoundStrangerEventHook(
    Il2CppObject* player,
    Il2CppObject* npc,
    const Il2CppMethod* method)
{
    if (g_gameplayActionPursuitPhase ==
        GameplayActionPursuit_Active)
    {
        GameplayActionPursuit_EndSession(
            L"yield to original found event",
            false);
    }
    if (g_originalPlayerOnFoundStrangerEvent != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerOnFoundStrangerEvent)(
            player,
            npc,
            method);
    }
}

void NpcManagerUpdateHook(
    Il2CppObject* manager,
    const Il2CppMethod* method)
{
    if (g_originalNpcManagerUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalNpcManagerUpdate)(
            manager,
            method);
    }
    GameplayActionPursuit_EnforceHoldsAfterNpcUpdate();
}
