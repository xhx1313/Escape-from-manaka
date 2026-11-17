// JSON-driven checkpoint system for the Escape from Manaka mode.

enum class CheckpointTriggerMode
{
    None,
    ManualInteract,
    ProximityEnter,
    ProximityExit,
};

struct CheckpointActionDefinition
{
    std::string handler;
    nlohmann::json params;
};

struct CheckpointTaskDefinition
{
    std::string id;
    std::string label;
    std::string description;
    std::string kind;
    std::string gait;
    std::string startMode;
    int difficulty = 1;
    int distance = 0;
    int targetCount = 1;
    float protectionSeconds = 3.0f;
    std::string interruptPolicy;
    float requiredSeconds = 0.0f;
    int actionType = 0;
    float triggerRadius = 1.25f;
    bool lightOnly = false;
    bool requiresNaked = true;
};

struct CheckpointRewardItem
{
    int itemDefinition = -1;
    int itemQuality = 0;
};

struct CheckpointRewardRoll
{
    bool active = false;
    int supplyQuality = 0;
    std::vector<CheckpointRewardItem> rewards;
    int taskIndex = -1;
    bool searchCompleted = false;
    bool taskCompleted = false;
    bool taskFailed = false;
    bool rewardGranted = false;
    float taskProgress = 0.0f;
    int taskVisitedCount = 0;
};

struct CheckpointPendingLoot
{
    std::string checkpointId;
    int itemDefinition = -1;
    int quality = 0;
};

struct CheckpointDefinition
{
    std::string id;
    std::string kind;
    bool deleted = false;
    int stageType = -1;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float diameter = 1.6f;
    float triggerRadius = 1.1f;
    float heightTolerance = 1.5f;
    float heightOffset = 0.02f;
    float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool visible = true;
    bool once = true;
    CheckpointTriggerMode triggerMode = CheckpointTriggerMode::ManualInteract;
    nlohmann::json sourceJson = nlohmann::json::object();
    nlohmann::json metadata = nlohmann::json::object();
    std::vector<CheckpointActionDefinition> actions;
};

struct CheckpointRuntime
{
    size_t definitionIndex = 0;
    Il2CppObject* visualRoot = nullptr;
    bool playerInside = false;
    bool consumed = false;
    CheckpointRewardRoll roll;
};

enum class CheckpointMapPointKind
{
    Supply,
    Task,
    Extract,
    Transfer,
};

struct CheckpointMapPoint
{
    int runtimeIndex = -1;
    CheckpointMapPointKind kind =
        CheckpointMapPointKind::Supply;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const char* label = "地图点";
};

using CheckpointActionHandler = std::function<bool(
    const CheckpointDefinition&,
    const nlohmann::json&)>;

std::vector<CheckpointDefinition> g_checkpointDefinitions;
std::vector<CheckpointRuntime> g_checkpointRuntime;
std::unordered_map<std::string, CheckpointActionHandler>
    g_checkpointActionHandlers;
int g_checkpointLoadedStageType = -1;
nlohmann::json g_checkpointLoadedConfigRoot = nlohmann::json::object();
std::string g_checkpointSelectedSpawnId;
size_t g_checkpointSelectedSpawnIndex = static_cast<size_t>(-1);
std::vector<CheckpointTaskDefinition> g_checkpointTaskPool;
std::vector<CheckpointPendingLoot> g_checkpointPendingLoot;
std::string g_checkpointPendingTransferSpawnId;
double g_checkpointPointQualityWeights[5] = {
    20.0, 20.0, 20.0, 20.0, 20.0
};
double g_checkpointItemQualityWeights[5][5] = {
    { 53.8, 24.4, 10.6, 6.5, 4.6 },
    { 20.4, 44.9, 20.4, 8.9, 5.4 },
    { 8.6, 19.7, 43.4, 19.7, 8.6 },
    { 5.4, 8.9, 20.4, 44.9, 20.4 },
    { 4.6, 6.5, 10.6, 24.4, 53.8 },
};
Il2CppObject* g_checkpointRingTexture = nullptr;
Il2CppObject* g_checkpointRingSprite = nullptr;
int g_checkpointVisualCheckFrame = 0;
bool g_checkpointVisualCheckDone = false;

struct CheckpointActiveTaskRuntime
{
    bool active = false;
    int sourceRuntimeIndex = -1;
    int taskIndex = -1;
    int startRuntimeIndex = -1;
    std::vector<int> targetRuntimeIndices;
    std::vector<bool> targetVisited;
    float lastPlayerX = 0.0f;
    float lastPlayerY = 0.0f;
    float lastPlayerZ = 0.0f;
    float baselineDistance = 0.0f;
    float accumulatedDistance = 0.0f;
    float protectionRemaining = 0.0f;
    float conditionSeconds = 0.0f;
    float escalatorStartY = 0.0f;
    float escalatorMaxVerticalDistance = 0.0f;
    int selectedActionIndex = -1;
    int selectedActionType = 0;
    unsigned long long lastTick = 0;
    unsigned long long resetNoticeUntil = 0;
    bool conditionInvalid = false;
    bool targetsReady = false;
    bool escalatorWasPushed = false;
    bool previousEcstasyMotion = false;
    std::vector<bool> targetArmed;
    std::vector<bool> targetOrgasmDone;
};

CheckpointActiveTaskRuntime g_checkpointActiveTask;
std::atomic<int> g_checkpointRequestedTaskCrouch{-1};

bool CheckpointSystem_StartActiveTask(int runtimeIndex);
void CheckpointSystem_StopActiveTask(bool hideTaskPoints, bool restoreNaked);
void CheckpointSystem_SetTaskRandomPointsVisible(bool visible);
void CheckpointSystem_UpdateTaskTracker();
void CheckpointSystem_ResetMapEditor();
void CheckpointSystem_UpdateMapEditor(
    const float player[3],
    bool& ownsPrompt);

enum class CheckpointOverlayMode
{
    None,
    TaskOffer,
    TaskReplaceConfirm,
    RewardExitConfirm,
};

static CheckpointOverlayMode g_checkpointOverlayMode =
    CheckpointOverlayMode::None;
static int g_checkpointOverlayRuntimeIndex = -1;
static SceneUiElement g_checkpointOverlayRoot{};
static SceneUiElement g_checkpointOverlayPanel{};
static Il2CppObject* g_checkpointOverlayTitle = nullptr;
static Il2CppObject* g_checkpointOverlayBody = nullptr;
static Il2CppObject* g_checkpointOverlayWarning = nullptr;
static SceneUiElement g_checkpointOverlayPrimaryButton{};
static SceneUiElement g_checkpointOverlaySecondaryButton{};
static unsigned int g_checkpointOverlayObservedPressSerial = 0;
static unsigned int g_checkpointOverlayObservedReleaseSerial = 0;
static unsigned int g_checkpointOverlayPressSerial = 0;
static SceneUiElement g_checkpointTrackerRoot{};
static Il2CppObject* g_checkpointTrackerTitle = nullptr;
static Il2CppObject* g_checkpointTrackerProgress = nullptr;
static SceneUiElement g_checkpointTrackerBarBackground{};
static SceneUiElement g_checkpointTrackerBarFill{};
static int g_checkpointTrackedRuntimeIndex = -1;
static unsigned long long g_checkpointTrackerCompletedAt = 0;
static unsigned long long g_checkpointTrackerFailedAt = 0;
static bool g_checkpointRewardOpenPending = false;
static bool g_checkpointFailureHandled = false;
static Il2CppObject* g_checkpointFailureFallbackPanel = nullptr;
static Il2CppObject* g_checkpointSaveFallbackPanel = nullptr;
static unsigned long long g_checkpointFailureFallbackNextScanAt = 0;
static bool g_checkpointTrackerAbandoning = false;
static unsigned long long g_checkpointTrackerAbandonedAt = 0;
static int g_checkpointAbandonedTaskIndex = -1;
static int g_checkpointPendingReplacementRuntimeIndex = -1;
static bool g_checkpointFailureTaskMarked = false;

std::mt19937& CheckpointSystem_RandomEngine()
{
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

std::atomic_bool g_checkpointInteractionAvailable{false};

bool CheckpointSystem_HasNearbyInteraction()
{
    return g_checkpointInteractionAvailable.load() &&
           !CheckpointSystem_IsMapEditorModeActive();
}

bool CheckpointSystem_IsCoatToggleLocked()
{
    return g_checkpointActiveTask.active &&
           g_checkpointActiveTask.protectionRemaining > 0.0f;
}

void CheckpointSystem_ApplyRequiredTaskPose(
    Il2CppObject* inputData)
{
    if (inputData == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }
    const int requested = g_checkpointRequestedTaskCrouch.load();
    if (requested < 0)
    {
        return;
    }
    GameplayStatusHudMovementState movement{};
    if (!GameplayStatusHud_ReadMovementState(movement))
    {
        return;
    }
    const bool requestedCrouch = requested == 1;
    if (movement.isCrouch == requestedCrouch)
    {
        if (g_checkpointRequestedTaskCrouch.exchange(-1) != -1)
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                requestedCrouch
                    ? L"Checkpoint task posture reached: crawling."
                    : L"Checkpoint task posture reached: standing/walking.");
        }
        return;
    }
    *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(inputData) + 0x27) = true;
}

bool CheckpointSystem_IsActionable(
    const CheckpointRuntime& runtime,
    int runtimeIndex)
{
    if (runtime.definitionIndex >=
        g_checkpointDefinitions.size())
    {
        return false;
    }
    const auto& definition =
        g_checkpointDefinitions[runtime.definitionIndex];
    if (definition.deleted)
    {
        return false;
    }
    if (definition.kind == "supply")
    {
        return
            runtime.roll.active &&
            runtime.roll.taskIndex >= 0 &&
            !runtime.roll.searchCompleted &&
            !runtime.roll.taskCompleted &&
            !runtime.roll.taskFailed;
    }
    if (definition.kind == "task_random")
    {
        if (!g_checkpointActiveTask.active ||
            g_checkpointActiveTask.protectionRemaining > 0.0f ||
            !g_checkpointActiveTask.targetsReady)
        {
            return false;
        }
        if (g_checkpointActiveTask.taskIndex >= 0 &&
            g_checkpointActiveTask.taskIndex <
                static_cast<int>(g_checkpointTaskPool.size()))
        {
            const auto& task =
                g_checkpointTaskPool[
                    static_cast<size_t>(
                        g_checkpointActiveTask.taskIndex)];
            if (task.kind == "orgasm_location" ||
                task.kind == "orgasm_checkpoints")
            {
                return false;
            }
        }
        for (size_t index = 0;
             index <
                 g_checkpointActiveTask
                     .targetRuntimeIndices.size();
             ++index)
        {
            if (g_checkpointActiveTask
                    .targetRuntimeIndices[index] !=
                runtimeIndex)
            {
                continue;
            }
            return index >=
                       g_checkpointActiveTask
                           .targetVisited.size() ||
                   !g_checkpointActiveTask
                        .targetVisited[index];
        }
        return false;
    }
    return
        (definition.kind == "extract" ||
         definition.kind == "transfer") &&
        !runtime.consumed;
}

void CheckpointSystem_Reset()
{
    ExtendedMode_SetCheckpointInteractionPrompt(false, "");
    CheckpointSystem_ResetMapEditor();
    g_checkpointDefinitions.clear();
    g_checkpointRuntime.clear();
    g_checkpointTaskPool.clear();
    g_checkpointPendingLoot.clear();
    g_checkpointActiveTask = CheckpointActiveTaskRuntime{};
    g_checkpointRequestedTaskCrouch = -1;
    g_checkpointVisualCheckFrame = 0;
    g_checkpointVisualCheckDone = false;
    g_checkpointOverlayMode = CheckpointOverlayMode::None;
    g_checkpointOverlayRuntimeIndex = -1;
    g_checkpointOverlayRoot = SceneUiElement{};
    g_checkpointOverlayPanel = SceneUiElement{};
    g_checkpointOverlayTitle = nullptr;
    g_checkpointOverlayBody = nullptr;
    g_checkpointOverlayWarning = nullptr;
    g_checkpointOverlayPrimaryButton = SceneUiElement{};
    g_checkpointOverlaySecondaryButton = SceneUiElement{};
    g_checkpointTrackerRoot = SceneUiElement{};
    g_checkpointTrackerTitle = nullptr;
    g_checkpointTrackerProgress = nullptr;
    g_checkpointTrackerBarBackground = SceneUiElement{};
    g_checkpointTrackerBarFill = SceneUiElement{};
    g_checkpointTrackedRuntimeIndex = -1;
    g_checkpointTrackerCompletedAt = 0;
    g_checkpointTrackerFailedAt = 0;
    g_checkpointRewardOpenPending = false;
    g_checkpointFailureHandled = false;
    g_checkpointFailureFallbackPanel = nullptr;
    g_checkpointSaveFallbackPanel = nullptr;
    g_checkpointFailureFallbackNextScanAt = 0;
    g_checkpointTrackerAbandoning = false;
    g_checkpointTrackerAbandonedAt = 0;
    g_checkpointAbandonedTaskIndex = -1;
    g_checkpointPendingReplacementRuntimeIndex = -1;
    g_checkpointFailureUiActive = false;
    g_checkpointFailureTaskMarked = false;
    g_checkpointLoadedStageType = -1;
    g_checkpointLoadedConfigRoot = nlohmann::json::object();
    g_checkpointSelectedSpawnIndex = static_cast<size_t>(-1);
    g_checkpointInteractionAvailable = false;
    g_checkpointInteractActionDown = false;
    g_checkpointInteractionKeyConsumed = false;
    g_checkpointMapEditorToggleRequested = false;
    g_checkpointMapEditorEditRequested = false;
    g_checkpointMapEditorActionRequested = -1;
}

std::string CheckpointSystem_ReadUtf8File(
    const std::wstring& path)
{
    HANDLE file = CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint config not found: %s", path.c_str());
        return {};
    }
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) ||
        size.QuadPart <= 0 ||
        size.QuadPart > 2 * 1024 * 1024)
    {
        CloseHandle(file);
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint config has an invalid size: %s", path.c_str());
        return {};
    }
    std::string content(static_cast<size_t>(size.QuadPart), '\0');
    DWORD read = 0;
    const bool ok =
        ReadFile(
            file,
            content.data(),
            static_cast<DWORD>(content.size()),
            &read,
            nullptr) &&
        read == content.size();
    CloseHandle(file);
    if (!ok)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint config read failed: %s", path.c_str());
        return {};
    }
    if (content.size() >= 3 &&
        static_cast<uint8_t>(content[0]) == 0xEF &&
        static_cast<uint8_t>(content[1]) == 0xBB &&
        static_cast<uint8_t>(content[2]) == 0xBF)
    {
        content.erase(0, 3);
    }
    return content;
}

const char* CheckpointSystem_StageKey(int stageType)
{
    switch (stageType)
    {
    case 6: return "konbini";
    case 7: return "fashion_shop";
    case 10: return "residence";
    case 11: return "shopping_mall";
    case 12: return "station_front";
    case 14: return "park";
    case 15: return "mansion";
    default: return nullptr;
    }
}

const wchar_t* CheckpointSystem_StageConfigFileName(int stageType)
{
    switch (stageType)
    {
    case 6: return L"06_konbini.json";
    case 7: return L"07_fashion_shop.json";
    case 10: return L"10_residence.json";
    case 11: return L"11_shopping_mall.json";
    case 12: return L"12_station_front.json";
    case 14: return L"14_park.json";
    case 15: return L"15_mansion.json";
    default: return nullptr;
    }
}

std::wstring CheckpointSystem_GetShippedConfigPath(int stageType)
{
    const wchar_t* fileName =
        CheckpointSystem_StageConfigFileName(stageType);
    if (fileName == nullptr)
    {
        return {};
    }
    return GetEscapeFromManakaSubdirectory(L"Checkpoints") +
        L"\\" +
        fileName;
}

std::wstring CheckpointSystem_GetUserConfigPath(int stageType)
{
    const std::wstring shipped =
        CheckpointSystem_GetShippedConfigPath(stageType);
    if (shipped.empty())
    {
        return {};
    }
    const size_t separator = shipped.find_last_of(L"\\/");
    const std::wstring fileName =
        separator == std::wstring::npos
            ? shipped
            : shipped.substr(separator + 1);
    wchar_t profile[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(
        L"USERPROFILE",
        profile,
        MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
    {
        return GetEscapeFromManakaSubdirectory(L"Checkpoints") +
            L"\\UserOverrides\\" +
            fileName;
    }
    return std::wstring(profile) +
        L"\\AppData\\LocalLow\\SheableSoft\\"
        L"SecretFlasherManaka\\SaveData\\EscapeFromManaka\\"
        L"MapPoints\\" + fileName;
}

std::wstring CheckpointSystem_GetConfigPath(int stageType)
{
    const std::wstring userPath =
        CheckpointSystem_GetUserConfigPath(stageType);
    if (!userPath.empty() &&
        GetFileAttributesW(userPath.c_str()) !=
            INVALID_FILE_ATTRIBUTES)
    {
        return userPath;
    }
    return CheckpointSystem_GetShippedConfigPath(stageType);
}

int CheckpointSystem_HexDigit(char value)
{
    if (value >= '0' && value <= '9')
    {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f')
    {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F')
    {
        return value - 'A' + 10;
    }
    return -1;
}

bool CheckpointSystem_ParseColor(
    const std::string& value,
    float output[4])
{
    if (value.size() != 7 && value.size() != 9)
    {
        return false;
    }
    if (value[0] != '#')
    {
        return false;
    }
    uint32_t channels[4] = { 0, 0, 0, 255 };
    const size_t count = value.size() == 9 ? 4 : 3;
    for (size_t index = 0; index < count; ++index)
    {
        const int high = CheckpointSystem_HexDigit(
            value[1 + index * 2]);
        const int low = CheckpointSystem_HexDigit(
            value[2 + index * 2]);
        if (high < 0 || low < 0)
        {
            return false;
        }
        channels[index] =
            static_cast<uint32_t>((high << 4) | low);
    }
    for (size_t index = 0; index < 4; ++index)
    {
        output[index] = channels[index] / 255.0f;
    }
    return true;
}

CheckpointTriggerMode CheckpointSystem_ParseTriggerMode(
    const std::string& value)
{
    if (value == "proximity_enter")
    {
        return CheckpointTriggerMode::ProximityEnter;
    }
    if (value == "proximity_exit")
    {
        return CheckpointTriggerMode::ProximityExit;
    }
    if (value == "manual_interact" || value == "manual")
    {
        return CheckpointTriggerMode::ManualInteract;
    }
    return CheckpointTriggerMode::None;
}

int CheckpointSystem_ParseQuality(const std::string& value)
{
    if (value == "white")
    {
        return 0;
    }
    if (value == "green")
    {
        return 1;
    }
    if (value == "purple")
    {
        return 2;
    }
    if (value == "gold")
    {
        return 3;
    }
    if (value == "red")
    {
        return 4;
    }
    return -1;
}

int CheckpointSystem_GetSupplyCount(
    const nlohmann::json& metadata)
{
    return std::clamp(
        metadata.value("supplyCount", 1),
        1,
        3);
}

int CheckpointSystem_ResolveSupplyPointQuality(
    const nlohmann::json& metadata,
    std::mt19937& generator)
{
    if (metadata.contains("supplyQuality") &&
        metadata["supplyQuality"].is_number_integer())
    {
        const int configured =
            metadata["supplyQuality"].get<int>();
        if (configured >= 1 && configured <= 5)
        {
            return configured - 1;
        }
    }
    else if (metadata.contains("quality") &&
             metadata["quality"].is_string())
    {
        const int legacy = CheckpointSystem_ParseQuality(
            metadata["quality"].get<std::string>());
        if (legacy >= 0)
        {
            return legacy;
        }
    }
    std::discrete_distribution<int> distribution(
        g_checkpointPointQualityWeights,
        g_checkpointPointQualityWeights + 5);
    return distribution(generator);
}

const char* CheckpointSystem_QualityName(int quality)
{
    static const char* names[] = {
        "white",
        "green",
        "purple",
        "gold",
        "red"
    };
    return quality >= 0 && quality < 5
        ? names[quality]
        : "unknown";
}

const char* CheckpointSystem_QualityDisplayName(int quality)
{
    static const char* names[] = {
        "白",
        "绿",
        "紫",
        "金",
        "红"
    };
    return quality >= 0 && quality < 5
        ? names[quality]
        : "未知";
}

const char* CheckpointSystem_StageDisplayName(int stageType)
{
    switch (stageType)
    {
    case 0: return "自宅";
    case 6: return "便利店";
    case 7: return "服装店";
    case 10: return "住宅区";
    case 11: return "购物中心";
    case 12: return "繁华街";
    case 14: return "综合公园";
    case 15: return "公寓前";
    default: return "未知地点";
    }
}

nlohmann::json CheckpointSystem_DefaultTaskPool(
    const char* stageKey)
{
    static const int distances[5] = {
        5,
        8,
        12,
        16,
        20,
    };
    static const char* labels[5] = {
        "巡逻I",
        "巡逻II",
        "巡逻III",
        "巡逻IV",
        "巡逻V",
    };
    nlohmann::json tasks = nlohmann::json::array();
    for (int index = 0; index < 5; ++index)
    {
        char id[160]{};
        std::snprintf(
            id,
            sizeof(id),
            "%s.patrol.%d",
            stageKey != nullptr ? stageKey : "unknown_stage",
            index + 1);
        char description[192]{};
        std::snprintf(
            description,
            sizeof(description),
            "在当前地图累计移动%d米；不受姿态限制。",
            distances[index]);
        tasks.push_back({
            { "id", id },
            { "label", labels[index] },
            { "description", description },
            { "kind", "distance" },
            { "gait", "any" },
            { "startMode", "current" },
            { "difficulty", index + 1 },
            { "distance", distances[index] },
            { "targetCount", 0 },
            { "protectionSeconds", 3 },
            { "interrupt", "reset" },
            { "requiresNaked", false },
            { "lightOnly", false },
        });
    }
    return tasks;
}

const float* CheckpointSystem_QualityColor(int quality)
{
    static const float colors[5][4] = {
        { 0.88f, 0.90f, 0.94f, 0.92f },
        { 0.24f, 0.82f, 0.42f, 0.95f },
        { 0.63f, 0.32f, 0.95f, 0.95f },
        { 0.96f, 0.73f, 0.18f, 0.96f },
        { 0.90f, 0.20f, 0.24f, 0.98f }
    };
    return colors[
        quality >= 0 && quality < 5
            ? quality
            : 0];
}

void CheckpointSystem_RegisterActionHandlers()
{
    if (!g_checkpointActionHandlers.empty())
    {
        return;
    }
    g_checkpointActionHandlers["debug.checkpoint_reached"] =
        [](const CheckpointDefinition& definition, const nlohmann::json&)
        {
            const std::wstring id = Utf8ToWide(definition.id);
            const std::wstring kind = Utf8ToWide(definition.kind);
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint reached: id=%s kind=%s stage=%d.",
                id.c_str(),
                kind.c_str(),
                definition.stageType);
            return true;
        };
    g_checkpointActionHandlers["system.select_spawn"] =
        [](const CheckpointDefinition& definition, const nlohmann::json&)
        {
            g_checkpointSelectedSpawnId = definition.id;
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint spawn selected: %s.",
                Utf8ToWide(definition.id).c_str());
            return true;
        };
}

bool CheckpointSystem_LoadConfig(int stageType)
{
    const char* stageKey = CheckpointSystem_StageKey(stageType);
    if (stageKey == nullptr)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint system does not support stage %d.",
            stageType);
        return false;
    }
    const std::wstring path = CheckpointSystem_GetConfigPath(stageType);
    if (path.empty())
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint system has no test config for stage %d.",
            stageType);
        return false;
    }
    const std::string content = CheckpointSystem_ReadUtf8File(path);
    nlohmann::json root;
    if (content.empty() &&
        GetFileAttributesW(path.c_str()) ==
            INVALID_FILE_ATTRIBUTES)
    {
        root = {
            { "schemaVersion", 1 },
            { "stage", {
                { "type", stageType },
                { "key", stageKey },
                { "displayName", CheckpointSystem_StageDisplayName(stageType) },
            } },
            { "defaults", nlohmann::json::object() },
            { "taskPool", nlohmann::json::array() },
            { "supplyRules", nlohmann::json::object() },
            { "checkpoints", nlohmann::json::array() },
        };
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint editor created an empty in-memory config for stage %d.",
            stageType);
    }
    else if (content.empty())
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint config is empty or unreadable: %s",
            path.c_str());
        return false;
    }
    else
    {
        try
        {
            root = nlohmann::json::parse(content);
        }
        catch (const std::exception& exception)
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint JSON parse failed: %S",
                exception.what());
            return false;
        }
    }
    if (!root.is_object() ||
        root.value("schemaVersion", 0) != 1 ||
        !root.contains("stage") ||
        !root.contains("checkpoints") ||
        !root["checkpoints"].is_array())
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint JSON has an unsupported root schema.");
        return false;
    }
    g_checkpointLoadedConfigRoot = root;
    const auto& stage = root["stage"];
    if (!stage.is_object() ||
        stage.value("type", -1) != stageType)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint JSON stage mismatch: expected %d.",
            stageType);
        return false;
    }
    if (!root.contains("taskPool") ||
        !root["taskPool"].is_array() ||
        root["taskPool"].empty())
    {
        root["taskPool"] =
            CheckpointSystem_DefaultTaskPool(stageKey);
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint stage %d uses the built-in patrol task pool.",
            stageType);
    }
    g_checkpointLoadedConfigRoot = root;

    float defaultColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float defaultDiameter = 1.6f;
    float defaultTriggerRadius = 1.1f;
    float defaultHeightTolerance = 1.5f;
    float defaultHeightOffset = 0.02f;
    std::string defaultTriggerMode = "manual_interact";
    bool defaultOnce = true;
    if (root.contains("defaults") &&
        root["defaults"].is_object())
    {
        const auto& defaults = root["defaults"];
        if (defaults.contains("visual") &&
            defaults["visual"].is_object())
        {
            const auto& visual = defaults["visual"];
            defaultDiameter = visual.value(
                "diameter",
                defaultDiameter);
            defaultHeightOffset = visual.value(
                "heightOffset",
                defaultHeightOffset);
            if (visual.contains("color") &&
                visual["color"].is_string())
            {
                CheckpointSystem_ParseColor(
                    visual["color"].get<std::string>(),
                    defaultColor);
            }
        }
        if (defaults.contains("trigger") &&
            defaults["trigger"].is_object())
        {
            const auto& trigger = defaults["trigger"];
            defaultTriggerMode = trigger.value(
                "mode",
                defaultTriggerMode);
            defaultTriggerRadius = trigger.value(
                "radius",
                defaultTriggerRadius);
            defaultHeightTolerance = trigger.value(
                "heightTolerance",
                defaultHeightTolerance);
            defaultOnce = trigger.value(
                "once",
                defaultOnce);
        }
    }

    if (root.contains("taskPool") &&
        root["taskPool"].is_array())
    {
        for (const auto& task : root["taskPool"])
        {
            if (!task.is_object() ||
                !task.contains("id") ||
                !task["id"].is_string())
            {
                continue;
            }
            CheckpointTaskDefinition definition;
            definition.id = task["id"].get<std::string>();
            definition.label = task.value(
                "label",
                definition.id);
            definition.description = task.value(
                "description",
                std::string());
            definition.kind = task.value(
                "kind",
                "unimplemented");
            definition.gait = task.value(
                "gait",
                "any");
            definition.startMode = task.value(
                "startMode",
                "current");
            definition.difficulty = std::clamp(
                task.value("difficulty", definition.difficulty),
                1,
                5);
            definition.distance = std::max(
                0,
                task.value("distance", definition.distance));
            definition.targetCount = std::max(
                0,
                task.value("targetCount", definition.targetCount));
            definition.protectionSeconds = std::max(
                0.0f,
                task.value(
                    "protectionSeconds",
                    definition.protectionSeconds));
            definition.interruptPolicy = task.value(
                "interrupt",
                "reset");
            definition.requiredSeconds = std::max(
                0.0f,
                task.value(
                    "requiredSeconds",
                    definition.requiredSeconds));
            definition.actionType = task.value(
                "actionType",
                definition.actionType);
            definition.triggerRadius = std::max(
                0.1f,
                task.value(
                    "triggerRadius",
                    definition.triggerRadius));
            definition.lightOnly = task.value(
                "lightOnly",
                definition.lightOnly);
            definition.requiresNaked = task.value(
                "requiresNaked",
                definition.requiresNaked);
            g_checkpointTaskPool.push_back(std::move(definition));
        }
    }
    if (root.contains("supplyRules") &&
        root["supplyRules"].is_object())
    {
        const auto& rules = root["supplyRules"];
        if (rules.contains("pointQualityWeights") &&
            rules["pointQualityWeights"].is_array() &&
            rules["pointQualityWeights"].size() == 5)
        {
            for (size_t index = 0; index < 5; ++index)
            {
                g_checkpointPointQualityWeights[index] =
                    rules["pointQualityWeights"][index].get<double>();
            }
        }
        if (rules.contains("itemQualityWeights") &&
            rules["itemQualityWeights"].is_array() &&
            rules["itemQualityWeights"].size() == 5)
        {
            for (size_t row = 0; row < 5; ++row)
            {
                if (!rules["itemQualityWeights"][row].is_array() ||
                    rules["itemQualityWeights"][row].size() != 5)
                {
                    continue;
                }
                for (size_t column = 0; column < 5; ++column)
                {
                    g_checkpointItemQualityWeights[row][column] =
                        rules["itemQualityWeights"][row][column]
                            .get<double>();
                }
            }
        }
    }

    for (const auto& checkpoint : root["checkpoints"])
    {
        if (!checkpoint.is_object() ||
            !checkpoint.contains("id") ||
            !checkpoint["id"].is_string() ||
            !checkpoint.contains("position") ||
            !checkpoint["position"].is_object())
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Skipping malformed checkpoint record.");
            continue;
        }
        CheckpointDefinition definition;
        definition.id = checkpoint["id"].get<std::string>();
        definition.sourceJson = checkpoint;
        definition.kind = checkpoint.value("kind", "visual");
        definition.stageType = stageType;
        const auto& position = checkpoint["position"];
        definition.x = position.value("x", 0.0f);
        definition.y = position.value("y", 0.0f);
        definition.z = position.value("z", 0.0f);
        definition.diameter = defaultDiameter;
        definition.triggerRadius = defaultTriggerRadius;
        definition.heightTolerance = defaultHeightTolerance;
        definition.heightOffset = defaultHeightOffset;
        std::memcpy(definition.color, defaultColor, sizeof(defaultColor));
        definition.triggerMode =
            CheckpointSystem_ParseTriggerMode(defaultTriggerMode);
        definition.once = defaultOnce;
        definition.visible = definition.kind != "spawn";

        if (checkpoint.contains("visual") &&
            checkpoint["visual"].is_object())
        {
            const auto& visual = checkpoint["visual"];
            definition.visible = visual.value(
                "visible",
                definition.visible);
            definition.diameter = visual.value(
                "diameter",
                definition.diameter);
            definition.heightOffset = visual.value(
                "heightOffset",
                definition.heightOffset);
            if (visual.contains("color") &&
                visual["color"].is_string())
            {
                CheckpointSystem_ParseColor(
                    visual["color"].get<std::string>(),
                    definition.color);
            }
        }
        if (definition.kind == "transfer")
        {
            definition.color[0] = 0.04f;
            definition.color[1] = 0.18f;
            definition.color[2] = 0.62f;
            definition.color[3] = 1.0f;
        }
        if (checkpoint.contains("trigger") &&
            checkpoint["trigger"].is_object())
        {
            const auto& trigger = checkpoint["trigger"];
            definition.triggerMode = CheckpointSystem_ParseTriggerMode(
                trigger.value("mode", defaultTriggerMode));
            definition.triggerRadius = trigger.value(
                "radius",
                definition.triggerRadius);
            definition.heightTolerance = trigger.value(
                "heightTolerance",
                definition.heightTolerance);
            definition.once = trigger.value(
                "once",
                definition.once);
        }
        if (checkpoint.contains("metadata") &&
            checkpoint["metadata"].is_object())
        {
            definition.metadata = checkpoint["metadata"];
        }
        if (checkpoint.contains("actions") &&
            checkpoint["actions"].is_array())
        {
            for (const auto& action : checkpoint["actions"])
            {
                if (!action.is_object() ||
                    !action.contains("handler") ||
                    !action["handler"].is_string())
                {
                    continue;
                }
                CheckpointActionDefinition parsedAction;
                parsedAction.handler =
                    action["handler"].get<std::string>();
                parsedAction.params = action.value(
                    "params",
                    nlohmann::json::object());
                definition.actions.push_back(parsedAction);
            }
        }

        const bool duplicate = std::any_of(
            g_checkpointDefinitions.begin(),
            g_checkpointDefinitions.end(),
            [&](const CheckpointDefinition& existing)
            {
                return existing.id == definition.id;
            });
        if (duplicate)
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Duplicate checkpoint id ignored: %s.",
                Utf8ToWide(definition.id).c_str());
            continue;
        }
        g_checkpointDefinitions.push_back(std::move(definition));
    }

    g_checkpointLoadedStageType = stageType;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint config loaded: stage=%d records=%u.",
        stageType,
        static_cast<unsigned int>(g_checkpointDefinitions.size()));
    return true;
}

Il2CppObject* CheckpointSystem_GetRingSprite()
{
    if (ExtendedMode_IsUnityObjectAlive(g_checkpointRingSprite))
    {
        return g_checkpointRingSprite;
    }
    constexpr int size = 128;
    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* color32Class = FindClass("UnityEngine", "Color32");
    if (textureClass == nullptr ||
        color32Class == nullptr ||
        il2cpp_object_new == nullptr ||
        il2cpp_array_new == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        return nullptr;
    }
    il2cpp_runtime_class_init(textureClass);
    il2cpp_runtime_class_init(color32Class);
    auto* texture = il2cpp_object_new(textureClass);
    int textureFormat = 4; // TextureFormat.RGBA32
    bool mipChain = false;
    void* constructorParameters[] = {
        const_cast<int*>(&size),
        const_cast<int*>(&size),
        &textureFormat,
        &mipChain
    };
    Invoke(
        FindMethod(textureClass, ".ctor", 4),
        texture,
        constructorParameters);

    auto* pixels = il2cpp_array_new(
        color32Class,
        size * size);
    if (texture == nullptr || pixels == nullptr)
    {
        return nullptr;
    }
    auto* bytes = reinterpret_cast<uint8_t*>(pixels) +
        il2cpp_array_object_header_size();
    constexpr float center = (size - 1) * 0.5f;
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            const float dx = (x - center) / center;
            const float dy = (y - center) / center;
            const float distance = std::sqrt(dx * dx + dy * dy);
            const float ring =
                std::exp(-std::pow((distance - 0.82f) / 0.055f, 2.0f));
            const float innerGlow =
                distance < 0.88f
                    ? 0.12f * (1.0f - distance)
                    : 0.0f;
            const float alpha = std::clamp(
                ring + innerGlow,
                0.0f,
                1.0f);
            const size_t offset =
                static_cast<size_t>((y * size + x) * 4);
            bytes[offset + 0] = 255;
            bytes[offset + 1] = 255;
            bytes[offset + 2] = 255;
            bytes[offset + 3] =
                static_cast<uint8_t>(alpha * 255.0f);
        }
    }
    int mipLevel = 0;
    void* pixelParameters[] = {
        pixels,
        &mipLevel
    };
    Invoke(
        FindMethod(textureClass, "SetAllPixels32", 2),
        texture,
        pixelParameters);
    Invoke(
        FindMethod(textureClass, "Apply", 0),
        texture,
        nullptr);

    float rect[] = {
        0.0f,
        0.0f,
        static_cast<float>(size),
        static_cast<float>(size)
    };
    float pivot[] = { 0.5f, 0.5f };
    float pixelsPerUnit = static_cast<float>(size);
    void* spriteParameters[] = {
        texture,
        rect,
        pivot,
        &pixelsPerUnit
    };
    auto* sprite = Invoke(
        FindMethod(FindClass("UnityEngine", "Sprite"), "Create", 4),
        nullptr,
        spriteParameters);
    g_checkpointRingTexture = texture;
    g_checkpointRingSprite = sprite;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint ring sprite created: texture=%p sprite=%p.",
        texture,
        sprite);
    return sprite;
}

Il2CppObject* CheckpointSystem_CreateRingMaterial(
    const float color[4])
{
    auto* shaderClass = FindClass("UnityEngine", "Shader");
    auto* materialClass = FindClass("UnityEngine", "Material");
    Il2CppObject* shader = nullptr;
    const char* shaderNames[] = {
        "Unlit/Color",
        "Unlit/Transparent",
        "Particles/Standard Unlit",
        "Sprites/Default"
    };
    for (const auto* shaderName : shaderNames)
    {
        auto* nameValue = il2cpp_string_new(shaderName);
        void* shaderParameters[] = { nameValue };
        shader = Invoke(
            FindMethod(shaderClass, "Find", 1),
            nullptr,
            shaderParameters);
        if (shader != nullptr)
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint ring shader selected: %S.",
                shaderName);
            break;
        }
    }
    if (shader == nullptr ||
        materialClass == nullptr ||
        il2cpp_object_new == nullptr)
    {
        return nullptr;
    }
    auto* material = il2cpp_object_new(materialClass);
    void* constructorParameters[] = { shader };
    Invoke(
        FindMethod(materialClass, ".ctor", 1),
        material,
        constructorParameters);
    const char* colorProperties[] = {
        "_Color",
        "_BaseColor"
    };
    for (const auto* property : colorProperties)
    {
        auto* propertyName = il2cpp_string_new(property);
        void* colorParameters[] = {
            propertyName,
            const_cast<float*>(color)
        };
        Invoke(
            FindMethod(materialClass, "SetColor", 2),
            material,
            colorParameters);
    }
    int renderQueue = 3000;
    void* renderQueueParameters[] = { &renderQueue };
    Invoke(
        FindMethod(materialClass, "set_renderQueue", 1),
        material,
        renderQueueParameters);
    return material;
}

Il2CppObject* CheckpointSystem_CreateVisual(
    const CheckpointDefinition& definition)
{
    if (!definition.visible)
    {
        return nullptr;
    }
    const std::string objectName =
        "EFM_Checkpoint_" + definition.id;
    auto* visualRoot = CreateWorldGameObject(objectName.c_str());
    if (visualRoot == nullptr)
    {
        return nullptr;
    }
    SetWorldGameObjectActive(visualRoot, true);
    auto* transform = GetWorldTransform(visualRoot);
    ExtendedMode_SetTransformPosition(
        transform,
        definition.x,
        definition.y + definition.heightOffset,
        definition.z);
    ExtendedMode_SetTransformLocalScale(transform, 1.0f, 1.0f, 1.0f);

    auto* material = CheckpointSystem_CreateRingMaterial(
        definition.color);
    if (material == nullptr)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint visual could not create the ring material: id=%s.",
            Utf8ToWide(definition.id).c_str());
        return nullptr;
    }

    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    int primitiveType = 2; // PrimitiveType.Cylinder
    void* primitiveParameters[] = { &primitiveType };
    auto* disk = Invoke(
        FindMethod(gameObjectClass, "CreatePrimitive", 1),
        nullptr,
        primitiveParameters);
    if (!ExtendedMode_IsUnityObjectAlive(disk))
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint disk primitive creation failed: id=%s.",
            Utf8ToWide(definition.id).c_str());
        return nullptr;
    }
    auto* diskTransform = GetWorldTransform(disk);
    bool keepWorldPosition = false;
    void* parentParameters[] = {
        transform,
        &keepWorldPosition
    };
    Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "SetParent",
            2),
        diskTransform,
        parentParameters);
    SetTransformLocalPosition(diskTransform, 0.0f, 0.0f, 0.0f);
    ExtendedMode_SetTransformLocalScale(
        diskTransform,
        definition.diameter,
        0.01f,
        definition.diameter);
    auto* collider = GetSceneComponentByClass(
        disk,
        "UnityEngine",
        "Collider");
    if (collider != nullptr)
    {
        bool enabled = false;
        void* enabledParameters[] = { &enabled };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "Collider"),
                "set_enabled",
                1),
            collider,
            enabledParameters);
    }
    auto* meshRenderer = GetSceneComponentByClass(
        disk,
        "UnityEngine",
        "MeshRenderer");
    if (meshRenderer != nullptr)
    {
        void* materialParameters[] = { material };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "Renderer"),
                "set_material",
                1),
            meshRenderer,
            materialParameters);
    }
    ExtendedMode_SetHierarchyLayer(disk, kExtendedRoomLayer);
    ExtendedMode_SetHierarchyLayer(visualRoot, kExtendedRoomLayer);
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint visual created: id=%s position=(%.2f, %.2f, %.2f) diameter=%.2f root=%p.",
        Utf8ToWide(definition.id).c_str(),
        definition.x,
        definition.y + definition.heightOffset,
        definition.z,
        definition.diameter,
        visualRoot);
    return visualRoot;
}

bool CheckpointSystem_DispatchActions(
    const CheckpointDefinition& definition)
{
    CheckpointSystem_RegisterActionHandlers();
    for (const auto& action : definition.actions)
    {
        const auto iterator =
            g_checkpointActionHandlers.find(action.handler);
        if (iterator == g_checkpointActionHandlers.end())
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Unknown checkpoint action handler: %S.",
                action.handler.c_str());
            continue;
        }
        if (!iterator->second(definition, action.params))
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint action failed: checkpoint=%s handler=%S.",
                Utf8ToWide(definition.id).c_str(),
                action.handler.c_str());
            return false;
        }
    }
    return true;
}

void CheckpointSystem_OnGameplayReady(int stageType)
{
    if (g_checkpointLoadedStageType == stageType)
    {
        return;
    }
    CheckpointSystem_Reset();
    if (CheckpointSystem_StageKey(stageType) == nullptr)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint system is unsupported for stage %d.",
            stageType);
        return;
    }
    if (!CheckpointSystem_LoadConfig(stageType))
    {
        return;
    }
    if (g_checkpointDefinitions.empty())
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint stage %d has no configured points yet.",
            stageType);
        return;
    }

    CheckpointSystem_RegisterActionHandlers();

    std::vector<size_t> spawnIndices;
    for (size_t index = 0;
         index < g_checkpointDefinitions.size();
         ++index)
    {
        if (g_checkpointDefinitions[index].kind == "spawn")
        {
            spawnIndices.push_back(index);
        }
    }
    if (!spawnIndices.empty())
    {
        g_checkpointSelectedSpawnIndex = static_cast<size_t>(-1);
        if (!g_checkpointPendingTransferSpawnId.empty())
        {
            for (const size_t index : spawnIndices)
            {
                if (g_checkpointDefinitions[index].id ==
                    g_checkpointPendingTransferSpawnId)
                {
                    g_checkpointSelectedSpawnIndex = index;
                    break;
                }
            }
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint transfer target spawn request: id=%S matched=%d.",
                g_checkpointPendingTransferSpawnId.c_str(),
                g_checkpointSelectedSpawnIndex !=
                    static_cast<size_t>(-1)
                    ? 1
                    : 0);
            g_checkpointPendingTransferSpawnId.clear();
        }
        if (g_checkpointSelectedSpawnIndex ==
            static_cast<size_t>(-1))
        {
            auto& generator = CheckpointSystem_RandomEngine();
            std::uniform_int_distribution<size_t> distribution(
                0,
                spawnIndices.size() - 1);
            g_checkpointSelectedSpawnIndex =
                spawnIndices[distribution(generator)];
        }
        g_checkpointSelectedSpawnId =
            g_checkpointDefinitions[
                g_checkpointSelectedSpawnIndex].id;
    }

    g_checkpointRuntime.reserve(g_checkpointDefinitions.size());
    constexpr int kInventoryDefinitionCount =
        static_cast<int>(kEquipmentInventoryDefinitionCount);
    std::uniform_real_distribution<float> unitDistribution(
        0.0f,
        1.0f);
    auto& generator = CheckpointSystem_RandomEngine();
    int configuredSupplyCount = 0;
    int materializedSupplyCount = 0;
    int skippedSupplyCount = 0;
    int visibleCheckpointCount = 0;
    for (size_t index = 0;
         index < g_checkpointDefinitions.size();
         ++index)
    {
        auto& definition = g_checkpointDefinitions[index];
        CheckpointRewardRoll roll;
        if (definition.kind == "supply")
        {
            ++configuredSupplyCount;
            const float spawnChance = definition.metadata.value(
                "spawnChance",
                1.0f);
            if (unitDistribution(generator) > spawnChance)
            {
                ++skippedSupplyCount;
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                    L"Checkpoint supply skipped by spawn chance: id=%s chance=%.3f.",
                    Utf8ToWide(definition.id).c_str(),
                    spawnChance);
                continue;
            }

            const int pointQuality =
                CheckpointSystem_ResolveSupplyPointQuality(
                    definition.metadata,
                    generator);
            roll.active = true;
            roll.supplyQuality = pointQuality;
            std::memcpy(
                definition.color,
                CheckpointSystem_QualityColor(pointQuality),
                sizeof(definition.color));

            std::vector<int> matchingTasks;
            matchingTasks.reserve(g_checkpointTaskPool.size());
            for (size_t taskIndex = 0;
                 taskIndex < g_checkpointTaskPool.size();
                 ++taskIndex)
            {
                if (g_checkpointTaskPool[taskIndex].difficulty ==
                    pointQuality + 1)
                {
                    matchingTasks.push_back(
                        static_cast<int>(taskIndex));
                }
            }
            if (!matchingTasks.empty())
            {
                std::uniform_int_distribution<size_t> taskDistribution(
                    0,
                    matchingTasks.size() - 1);
                roll.taskIndex =
                    matchingTasks[taskDistribution(generator)];
            }
            else
            {
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                    L"Checkpoint supply has no task for quality=%S id=%s.",
                    CheckpointSystem_QualityName(pointQuality),
                    Utf8ToWide(definition.id).c_str());
            }

            const int supplyCount =
                CheckpointSystem_GetSupplyCount(
                    definition.metadata);
            roll.rewards.reserve(
                static_cast<size_t>(supplyCount));
            std::uniform_int_distribution<int> itemDistribution(
                0,
                kInventoryDefinitionCount - 1);
            std::discrete_distribution<int> itemQualityDistribution(
                g_checkpointItemQualityWeights[pointQuality],
                g_checkpointItemQualityWeights[pointQuality] + 5);
            for (int rewardIndex = 0;
                 rewardIndex < supplyCount;
                 ++rewardIndex)
            {
                roll.rewards.push_back({
                    itemDistribution(generator),
                    itemQualityDistribution(generator),
                });
            }
        }

        CheckpointRuntime runtime;
        runtime.definitionIndex = index;
        runtime.roll = roll;
        runtime.visualRoot = CheckpointSystem_CreateVisual(definition);
        g_checkpointRuntime.push_back(runtime);
        if (definition.kind == "supply" &&
            !roll.rewards.empty())
        {
            ++materializedSupplyCount;
            const int taskDifficulty =
                roll.taskIndex >= 0
                    ? g_checkpointTaskPool[
                          static_cast<size_t>(
                              roll.taskIndex)].difficulty
                    : 0;
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint supply rolled: id=%s pointQuality=%S taskDifficulty=%d rewards=%u task=%S.",
                Utf8ToWide(definition.id).c_str(),
                CheckpointSystem_QualityName(
                    roll.supplyQuality),
                taskDifficulty,
                static_cast<unsigned int>(
                    roll.rewards.size()),
                roll.taskIndex >= 0
                    ? g_checkpointTaskPool[
                          static_cast<size_t>(
                              roll.taskIndex)].id.c_str()
                    : "none");
        }
        if (runtime.visualRoot != nullptr)
        {
            ++visibleCheckpointCount;
        }
        if (index == g_checkpointSelectedSpawnIndex)
        {
            CheckpointSystem_DispatchActions(definition);
        }
    }
    CheckpointSystem_SetTaskRandomPointsVisible(false);
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint runtime created: stage=%d visual=%u total=%u selectedSpawn=%S.",
        stageType,
        static_cast<unsigned int>(std::count_if(
            g_checkpointRuntime.begin(),
            g_checkpointRuntime.end(),
            [](const CheckpointRuntime& runtime)
            {
                return runtime.visualRoot != nullptr;
            })),
        static_cast<unsigned int>(g_checkpointRuntime.size()),
        g_checkpointSelectedSpawnId.c_str());
    if (g_checkpointSelectedSpawnIndex <
        g_checkpointDefinitions.size())
    {
        const auto& selectedSpawn =
            g_checkpointDefinitions[g_checkpointSelectedSpawnIndex];
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint random summary: stage=%d supplies=%d materialized=%d skipped=%d visible=%d spawn=%S position=(%.2f, %.2f, %.2f).",
            stageType,
            configuredSupplyCount,
            materializedSupplyCount,
            skippedSupplyCount,
            visibleCheckpointCount,
            g_checkpointSelectedSpawnId.c_str(),
            selectedSpawn.x,
            selectedSpawn.y,
            selectedSpawn.z);
    }
    else
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint random summary: stage=%d has no spawn point.",
            stageType);
    }
}

bool CheckpointSystem_TryGetSelectedSpawn(
    float& x,
    float& y,
    float& z)
{
    if (g_checkpointSelectedSpawnIndex >=
        g_checkpointDefinitions.size())
    {
        return false;
    }
    const auto& definition =
        g_checkpointDefinitions[g_checkpointSelectedSpawnIndex];
    if (definition.kind != "spawn")
    {
        return false;
    }
    x = definition.x;
    y = definition.y;
    z = definition.z;
    return true;
}

void CheckpointSystem_ClearSceneObjects()
{
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* destroy = objectClass != nullptr
        ? FindMethod(objectClass, "Destroy", 1)
        : nullptr;
    for (auto& runtime : g_checkpointRuntime)
    {
        if (destroy != nullptr &&
            ExtendedMode_IsUnityObjectAlive(runtime.visualRoot))
        {
            void* parameters[] = { runtime.visualRoot };
            Invoke(destroy, nullptr, parameters);
        }
    }
    CheckpointSystem_Reset();
}

bool CheckpointSystem_IsTaskUiOpen()
{
    return g_checkpointOverlayMode != CheckpointOverlayMode::None;
}

bool CheckpointSystem_HasAcceptedTask()
{
    for (const auto& runtime : g_checkpointRuntime)
    {
        if (runtime.roll.taskIndex >= 0 &&
            runtime.roll.searchCompleted &&
            !runtime.roll.taskCompleted &&
            !runtime.roll.taskFailed)
        {
            return true;
        }
    }
    return false;
}

bool CheckpointSystem_IsTaskRandomRuntime(int runtimeIndex)
{
    if (runtimeIndex < 0 ||
        runtimeIndex >= static_cast<int>(g_checkpointRuntime.size()))
    {
        return false;
    }
    const auto& runtime =
        g_checkpointRuntime[static_cast<size_t>(runtimeIndex)];
    return runtime.definitionIndex <
               g_checkpointDefinitions.size() &&
           g_checkpointDefinitions[
               runtime.definitionIndex].kind == "task_random";
}

bool CheckpointSystem_IsRuntimeMapVisible(int runtimeIndex)
{
    if (runtimeIndex < 0 ||
        runtimeIndex >=
            static_cast<int>(g_checkpointRuntime.size()))
    {
        return false;
    }
    const auto& runtime =
        g_checkpointRuntime[
            static_cast<size_t>(runtimeIndex)];
    if (runtime.definitionIndex >=
            g_checkpointDefinitions.size())
    {
        return false;
    }
    const auto& definition =
        g_checkpointDefinitions[
            runtime.definitionIndex];
    if (definition.deleted ||
        !definition.visible ||
        runtime.visualRoot == nullptr)
    {
        return false;
    }

    if (definition.kind == "supply")
    {
        return runtime.roll.active &&
            !runtime.roll.rewards.empty() &&
            !(runtime.roll.taskIndex >= 0 &&
              runtime.roll.searchCompleted);
    }
    if (definition.kind == "task_random")
    {
        if (!g_checkpointActiveTask.active ||
            !g_checkpointActiveTask.targetsReady)
        {
            return false;
        }
        for (size_t index = 0;
             index <
                 g_checkpointActiveTask
                     .targetRuntimeIndices.size();
             ++index)
        {
            if (g_checkpointActiveTask
                    .targetRuntimeIndices[index] !=
                runtimeIndex)
            {
                continue;
            }
            return index >=
                       g_checkpointActiveTask
                           .targetVisited.size() ||
                !g_checkpointActiveTask
                     .targetVisited[index];
        }
        return false;
    }
    if (definition.kind == "extract" ||
        definition.kind == "transfer")
    {
        return !runtime.consumed;
    }
    return false;
}

bool CheckpointSystem_GetMapPoint(
    int runtimeIndex,
    CheckpointMapPoint& point)
{
    if (!CheckpointSystem_IsRuntimeMapVisible(
            runtimeIndex))
    {
        return false;
    }
    const auto& runtime =
        g_checkpointRuntime[
            static_cast<size_t>(runtimeIndex)];
    const auto& definition =
        g_checkpointDefinitions[
            runtime.definitionIndex];
    point = CheckpointMapPoint{};
    point.runtimeIndex = runtimeIndex;
    point.x = definition.x;
    point.y = definition.y;
    point.z = definition.z;
    std::memcpy(
        point.color,
        definition.color,
        sizeof(point.color));
    if (definition.kind == "supply")
    {
        point.kind = CheckpointMapPointKind::Supply;
        point.label = "物资点";
    }
    else if (definition.kind == "task_random")
    {
        point.kind = CheckpointMapPointKind::Task;
        point.color[0] = 1.0f;
        point.color[1] = 0.86f;
        point.color[2] = 0.08f;
        point.color[3] = 1.0f;
        point.label = "任务点";
    }
    else if (definition.kind == "extract")
    {
        point.kind = CheckpointMapPointKind::Extract;
        point.label = "撤离点";
    }
    else
    {
        point.kind = CheckpointMapPointKind::Transfer;
        point.color[0] = 0.04f;
        point.color[1] = 0.18f;
        point.color[2] = 0.62f;
        point.color[3] = 1.0f;
        point.label = "转移点";
    }
    return true;
}

void CheckpointSystem_CollectMapPoints(
    std::vector<CheckpointMapPoint>& points)
{
    points.clear();
    points.reserve(g_checkpointRuntime.size());
    for (size_t index = 0;
         index < g_checkpointRuntime.size();
         ++index)
    {
        CheckpointMapPoint point;
        if (CheckpointSystem_GetMapPoint(
                static_cast<int>(index),
                point))
        {
            points.push_back(point);
        }
    }
}

void CheckpointSystem_SetTaskRandomPointsVisible(bool visible)
{
    for (auto& runtime : g_checkpointRuntime)
    {
        if (runtime.definitionIndex >=
                g_checkpointDefinitions.size() ||
            g_checkpointDefinitions[
                runtime.definitionIndex].kind != "task_random")
        {
            continue;
        }
        if (runtime.visualRoot != nullptr)
        {
            SetWorldGameObjectActive(runtime.visualRoot, visible);
        }
    }
}

void CheckpointSystem_ShowOnlyTaskPoint(int runtimeIndex)
{
    CheckpointSystem_SetTaskRandomPointsVisible(false);
    if (!CheckpointSystem_IsTaskRandomRuntime(runtimeIndex))
    {
        return;
    }
    auto& runtime =
        g_checkpointRuntime[static_cast<size_t>(runtimeIndex)];
    if (runtime.visualRoot != nullptr)
    {
        SetWorldGameObjectActive(runtime.visualRoot, true);
    }
}

bool CheckpointSystem_WarpPlayerToTaskPoint(int runtimeIndex)
{
    if (!CheckpointSystem_IsTaskRandomRuntime(runtimeIndex) ||
        g_extendedPlayerGameObject == nullptr)
    {
        return false;
    }
    const auto& runtime =
        g_checkpointRuntime[static_cast<size_t>(runtimeIndex)];
    const auto& target =
        g_checkpointDefinitions[runtime.definitionIndex];
    auto* transform = GetWorldTransform(g_extendedPlayerGameObject);
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* rotation = transform != nullptr
        ? Invoke(
              FindMethod(transformClass, "get_rotation", 0),
              transform,
              nullptr)
        : nullptr;
    if (rotation == nullptr)
    {
        return false;
    }
    float position[3] = {
        target.x,
        target.y,
        target.z,
    };
    float* rotationValue =
        static_cast<float*>(il2cpp_object_unbox(rotation));
    void* parameters[] = {
        position,
        rotationValue,
    };
    auto* playerControllerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerController");
    auto* warp = FindMethod(playerControllerClass, "Warp", 2);
    if (warp == nullptr || g_extendedPlayerController == nullptr)
    {
        return false;
    }
    Invoke(warp, g_extendedPlayerController, parameters);
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint task warped player to random point: id=%s position=(%.2f, %.2f, %.2f).",
        Utf8ToWide(target.id).c_str(),
        target.x,
        target.y,
        target.z);
    return true;
}

int CheckpointSystem_ReadCurrentActionType()
{
    auto* playerState = GameplayStatusHud_GetPlayerState();
    if (playerState == nullptr)
    {
        return 0;
    }
    auto* currentAction = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerState) + 0x48);
    return currentAction != nullptr
        ? *reinterpret_cast<int*>(
            reinterpret_cast<char*>(currentAction) + 0x18)
        : 0;
}

bool CheckpointSystem_ReadEcstasyMotion()
{
    auto* playerState = GameplayStatusHud_GetPlayerState();
    return playerState != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(playerState) + 0x50);
}

bool CheckpointSystem_IsNearMissionNpc(bool secondary)
{
    auto* playerState = GameplayStatusHud_GetPlayerState();
    return playerState != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(playerState) +
            (secondary ? 0x89 : 0x88));
}

bool CheckpointSystem_IsMissionLightActive()
{
    auto* playerState = GameplayStatusHud_GetPlayerState();
    auto* gameStateData = ExtendedMode_GetGameStateData();
    const bool underStreetLight =
        playerState != nullptr &&
        *reinterpret_cast<float*>(
            reinterpret_cast<char*>(playerState) + 0x9C) > 0.0f;
    const bool daytime =
        gameStateData != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x20);
    return underStreetLight || daytime;
}

Il2CppObject* CheckpointSystem_GetElevatorController()
{
    return ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.ScenePlops.Elevator",
        "ElevatorController",
        "Instance");
}

bool CheckpointSystem_IsPlayerInsideElevator()
{
    auto* elevator = CheckpointSystem_GetElevatorController();
    auto* elevatorClass = FindClass(
        "ExposureUnnoticed2.Object3D.ScenePlops.Elevator",
        "ElevatorController");
    auto* isInside = elevatorClass != nullptr
        ? FindMethod(elevatorClass, "IsInsidePlayer", 0)
        : nullptr;
    auto* value = elevator != nullptr && isInside != nullptr
        ? Invoke(isInside, elevator, nullptr)
        : nullptr;
    return value != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(value));
}

bool CheckpointSystem_IsNpcInsideElevator()
{
    auto* elevator = CheckpointSystem_GetElevatorController();
    auto* npcManager = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcManager",
        "Instance");
    if (elevator == nullptr || npcManager == nullptr)
    {
        return false;
    }
    auto* npcList = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(npcManager) + 0x38);
    if (npcList == nullptr)
    {
        return false;
    }
    auto* items = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(npcList) + 0x10);
    const int count = *reinterpret_cast<int*>(
        reinterpret_cast<char*>(npcList) + 0x18);
    if (items == nullptr ||
        count <= 0 ||
        il2cpp_array_object_header_size == nullptr ||
        il2cpp_array_length == nullptr)
    {
        return false;
    }
    const uint32_t safeCount = std::min(
        static_cast<uint32_t>(count),
        il2cpp_array_length(items));
    auto** elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(items) +
        il2cpp_array_object_header_size());
    auto* elevatorClass = FindClass(
        "ExposureUnnoticed2.Object3D.ScenePlops.Elevator",
        "ElevatorController");
    auto* isInArea = elevatorClass != nullptr
        ? FindMethod(elevatorClass, "IsInArea", 1)
        : nullptr;
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* getPosition = transformClass != nullptr
        ? FindMethod(transformClass, "get_position", 0)
        : nullptr;
    if (isInArea == nullptr || getPosition == nullptr)
    {
        return false;
    }
    for (uint32_t index = 0; index < safeCount; ++index)
    {
        auto* npc = elements[index];
        if (npc == nullptr ||
            !ExtendedMode_IsUnityObjectAlive(npc))
        {
            continue;
        }
        auto* npcObject = GetComponentGameObject(npc);
        auto* transform = GetWorldTransform(npcObject);
        auto* positionValue = transform != nullptr
            ? Invoke(getPosition, transform, nullptr)
            : nullptr;
        if (positionValue == nullptr)
        {
            continue;
        }
        auto* position = static_cast<float*>(
            il2cpp_object_unbox(positionValue));
        void* parameters[] = { position };
        auto* inArea = Invoke(
            isInArea,
            elevator,
            parameters);
        if (inArea != nullptr &&
            *static_cast<bool*>(
                il2cpp_object_unbox(inArea)))
        {
            return true;
        }
    }
    return false;
}

bool CheckpointSystem_IsPlayerOnEscalator()
{
    auto* manager = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.ScenePlops.Escalator",
        "EscalatorManager",
        "Instance");
    auto* managerClass = FindClass(
        "ExposureUnnoticed2.Object3D.ScenePlops.Escalator",
        "EscalatorManager");
    auto* isPushed = managerClass != nullptr
        ? FindMethod(managerClass, "IsPushed", 1)
        : nullptr;
    if (manager == nullptr ||
        isPushed == nullptr ||
        g_extendedPlayerGameObject == nullptr)
    {
        return false;
    }
    void* parameters[] = { g_extendedPlayerGameObject };
    auto* value = Invoke(isPushed, manager, parameters);
    return value != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(value));
}

const char* CheckpointSystem_SelectedActionLabel()
{
    const int index =
        g_checkpointActiveTask.selectedActionIndex;
    return index >= 0 &&
           index < kExtendedPoseMenuOptionCount
        ? kExtendedPoseMenuOptions[index].label
        : "指定动作";
}

bool CheckpointSystem_TaskUsesWorldTargets(
    const CheckpointTaskDefinition& task)
{
    return task.kind == "reach" ||
        task.kind == "checkpoints" ||
        task.kind == "orgasm_location" ||
        task.kind == "orgasm_checkpoints" ||
        (task.kind == "pose_hold" &&
         task.targetCount > 0);
}

bool CheckpointSystem_PlayerWithinTaskTarget(
    const CheckpointDefinition& definition,
    float playerX,
    float playerY,
    float playerZ,
    float radiusScale = 1.0f)
{
    const float horizontalDx = playerX - definition.x;
    const float horizontalDz = playerZ - definition.z;
    const float radius =
        std::max(0.1f, definition.triggerRadius) *
        radiusScale;
    return horizontalDx * horizontalDx +
               horizontalDz * horizontalDz <=
           radius * radius &&
        std::fabs(playerY - definition.y) <=
            std::max(
                0.5f,
                definition.heightTolerance);
}

float CheckpointSystem_DistanceSquaredToDefinition(
    const CheckpointDefinition& definition,
    float playerX,
    float playerY,
    float playerZ)
{
    const float dx = playerX - definition.x;
    const float dy = playerY - definition.y;
    const float dz = playerZ - definition.z;
    return dx * dx + dy * dy + dz * dz;
}

bool CheckpointSystem_SampleTaskTargets(
    CheckpointActiveTaskRuntime& task,
    float playerX,
    float playerY,
    float playerZ)
{
    if (task.taskIndex < 0 ||
        task.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return false;
    }
    const auto& definition =
        g_checkpointTaskPool[static_cast<size_t>(task.taskIndex)];
    if (!CheckpointSystem_TaskUsesWorldTargets(definition))
    {
        task.targetsReady = true;
        task.baselineDistance = 0.0f;
        task.accumulatedDistance = 0.0f;
        return true;
    }
    if (definition.targetCount <= 0)
    {
        return false;
    }

    std::vector<int> candidates;
    candidates.reserve(g_checkpointRuntime.size());
    for (size_t index = 0;
         index < g_checkpointRuntime.size();
         ++index)
    {
        if (!CheckpointSystem_IsTaskRandomRuntime(
                static_cast<int>(index)))
        {
            continue;
        }
        if (static_cast<int>(index) == task.startRuntimeIndex)
        {
            continue;
        }
        const auto& candidate =
            g_checkpointDefinitions[
                g_checkpointRuntime[index].definitionIndex];
        if (CheckpointSystem_DistanceSquaredToDefinition(
                candidate,
                playerX,
                playerY,
                playerZ) <
            1.2f * 1.2f)
        {
            continue;
        }
        candidates.push_back(static_cast<int>(index));
    }
    if (static_cast<int>(candidates.size()) <
        definition.targetCount)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint task target sampling failed: need=%d available=%u task=%S.",
            definition.targetCount,
            static_cast<unsigned int>(candidates.size()),
            definition.id.c_str());
        return false;
    }

    auto& generator = CheckpointSystem_RandomEngine();
    std::shuffle(
        candidates.begin(),
        candidates.end(),
        generator);
    task.targetRuntimeIndices.assign(
        candidates.begin(),
        candidates.begin() + definition.targetCount);
    task.targetVisited.assign(
        static_cast<size_t>(definition.targetCount),
        false);
    task.targetArmed.assign(
        static_cast<size_t>(definition.targetCount),
        false);
    task.targetOrgasmDone.assign(
        static_cast<size_t>(definition.targetCount),
        false);

    CheckpointSystem_SetTaskRandomPointsVisible(false);
    for (const int targetRuntimeIndex :
         task.targetRuntimeIndices)
    {
        auto& targetRuntime =
            g_checkpointRuntime[
                static_cast<size_t>(targetRuntimeIndex)];
        if (targetRuntime.visualRoot != nullptr)
        {
            SetWorldGameObjectActive(
                targetRuntime.visualRoot,
                true);
        }
        const auto& targetDefinition =
            g_checkpointDefinitions[
                targetRuntime.definitionIndex];
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint task target sampled: task=%S point=%s position=(%.2f, %.2f, %.2f).",
            definition.id.c_str(),
            Utf8ToWide(targetDefinition.id).c_str(),
            targetDefinition.x,
            targetDefinition.y,
            targetDefinition.z);
    }

    task.targetsReady = true;
    task.baselineDistance =
        definition.kind == "reach" &&
        !task.targetRuntimeIndices.empty()
            ? std::sqrt(
                  CheckpointSystem_DistanceSquaredToDefinition(
                      g_checkpointDefinitions[
                          g_checkpointRuntime[
                              static_cast<size_t>(
                                  task.targetRuntimeIndices[0])]
                              .definitionIndex],
                      playerX,
                      playerY,
                      playerZ))
            : 0.0f;
    return true;
}

void CheckpointSystem_StopActiveTask(
    bool hideTaskPoints,
    bool restoreNaked)
{
    if (hideTaskPoints)
    {
        CheckpointSystem_SetTaskRandomPointsVisible(false);
    }
    if (restoreNaked)
    {
        ExtendedMode_SetSpecialCoatMode(false);
    }
    g_checkpointRequestedTaskCrouch = -1;
    g_checkpointActiveTask = CheckpointActiveTaskRuntime{};
}

bool CheckpointSystem_StartActiveTask(int runtimeIndex)
{
    if (runtimeIndex < 0 ||
        runtimeIndex >= static_cast<int>(g_checkpointRuntime.size()))
    {
        return false;
    }
    auto& source =
        g_checkpointRuntime[static_cast<size_t>(runtimeIndex)];
    if (source.definitionIndex >=
            g_checkpointDefinitions.size() ||
        source.roll.taskIndex < 0 ||
        source.roll.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return false;
    }
    const auto& sourceDefinition =
        g_checkpointDefinitions[source.definitionIndex];
    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(source.roll.taskIndex)];
    if (sourceDefinition.kind != "supply")
    {
        return false;
    }

    if (g_checkpointActiveTask.active)
    {
        CheckpointSystem_StopActiveTask(true, true);
    }

    std::vector<int> taskPoints;
    taskPoints.reserve(g_checkpointRuntime.size());
    for (size_t index = 0;
         index < g_checkpointRuntime.size();
         ++index)
    {
        if (CheckpointSystem_IsTaskRandomRuntime(
                static_cast<int>(index)))
        {
            taskPoints.push_back(static_cast<int>(index));
        }
    }
    const int requiredTaskPoints =
        CheckpointSystem_TaskUsesWorldTargets(task)
            ? task.targetCount
            : 0;
    const int requiredRandomStarts =
        task.startMode == "random" ? 1 : 0;
    if (static_cast<int>(taskPoints.size()) <
        requiredTaskPoints + requiredRandomStarts)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint task start rejected: insufficient task_random points task=%S required=%d available=%u.",
            task.id.c_str(),
            requiredTaskPoints + requiredRandomStarts,
            static_cast<unsigned int>(taskPoints.size()));
        return false;
    }

    auto& generator = CheckpointSystem_RandomEngine();
    std::shuffle(
        taskPoints.begin(),
        taskPoints.end(),
        generator);

    int randomStartIndex = -1;
    if (task.startMode == "random")
    {
        randomStartIndex = taskPoints.front();
    }
    CheckpointSystem_SetTaskRandomPointsVisible(false);
    if (randomStartIndex >= 0)
    {
        CheckpointSystem_ShowOnlyTaskPoint(randomStartIndex);
        if (!CheckpointSystem_WarpPlayerToTaskPoint(
                randomStartIndex))
        {
            CheckpointSystem_SetTaskRandomPointsVisible(false);
            return false;
        }
    }
    else
    {
        CheckpointSystem_SetTaskRandomPointsVisible(false);
    }

    source.roll.searchCompleted = true;
    source.roll.taskCompleted = false;
    source.roll.taskFailed = false;
    source.roll.rewardGranted = false;
    source.roll.taskProgress = 0.0f;
    source.roll.taskVisitedCount = 0;
    if (source.visualRoot != nullptr)
    {
        SetWorldGameObjectActive(source.visualRoot, false);
    }

    g_checkpointActiveTask =
        CheckpointActiveTaskRuntime{};
    g_checkpointActiveTask.active = true;
    g_checkpointActiveTask.sourceRuntimeIndex = runtimeIndex;
    g_checkpointActiveTask.taskIndex = source.roll.taskIndex;
    g_checkpointActiveTask.startRuntimeIndex = randomStartIndex;
    g_checkpointActiveTask.protectionRemaining =
        std::max(0.0f, task.protectionSeconds);
    g_checkpointActiveTask.lastTick = GetTickCount64();
    g_checkpointActiveTask.previousEcstasyMotion =
        CheckpointSystem_ReadEcstasyMotion();
    if (task.kind == "pose_hold")
    {
        int selectedPose = -1;
        if (task.actionType >= 50000 &&
            task.actionType < 60000)
        {
            for (int index = 0;
                 index < kExtendedPoseMenuOptionCount;
                 ++index)
            {
                if (kExtendedPoseMenuOptions[index].actionType ==
                    task.actionType)
                {
                    selectedPose = index;
                    break;
                }
            }
        }
        if (selectedPose < 0 && kExtendedPoseMenuOptionCount > 0)
        {
            std::uniform_int_distribution<int> poseDistribution(
                0,
                kExtendedPoseMenuOptionCount - 1);
            selectedPose =
                poseDistribution(CheckpointSystem_RandomEngine());
        }
        g_checkpointActiveTask.selectedActionIndex =
            selectedPose;
        g_checkpointActiveTask.selectedActionType =
            selectedPose >= 0
                ? kExtendedPoseMenuOptions[selectedPose].actionType
                : task.actionType;
    }
    g_checkpointRequestedTaskCrouch =
        task.gait == "crawl"
            ? 1
            : (task.gait == "walk" ? 0 : -1);
    g_checkpointTrackedRuntimeIndex = runtimeIndex;
    g_checkpointTrackerCompletedAt = 0;
    g_checkpointTrackerFailedAt = 0;
    g_checkpointTrackerAbandoning = false;
    g_checkpointTrackerAbandonedAt = 0;
    g_checkpointAbandonedTaskIndex = -1;
    g_checkpointPendingReplacementRuntimeIndex = -1;

    if (g_checkpointActiveTask.protectionRemaining > 0.0f)
    {
        ExtendedMode_SetSpecialCoatMode(true);
    }

    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint task started: source=%s task=%S quality=%d start=%S protection=%.1f.",
        Utf8ToWide(sourceDefinition.id).c_str(),
        task.id.c_str(),
        task.difficulty,
        task.startMode.c_str(),
        task.protectionSeconds);
    CheckpointSystem_UpdateTaskTracker();
    return true;
}

bool CheckpointSystem_IsRewardInventoryOpen()
{
    return g_gameplayInventoryOpen.load() &&
           g_gameplayInventoryRewardMode;
}

void CheckpointSystem_ProcessPendingReward()
{
    if (!g_checkpointRewardOpenPending ||
        g_checkpointTrackedRuntimeIndex >= 0 ||
        CheckpointSystem_ReadEcstasyMotion())
    {
        return;
    }
    g_checkpointRewardOpenPending = false;
    if (!g_extendedSpecialCoatMode.load())
    {
        ExtendedMode_SetSpecialCoatMode(true);
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint reward switched the player to coat mode before opening.");
    }
    GameplayInventory_OpenRewardMode();
}

void CheckpointSystem_SetOverlayButtonLabel(
    SceneUiElement& button,
    const char* text)
{
    if (button.text == nullptr)
    {
        return;
    }
    SetSceneText(
        button.text,
        text,
        19.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
}

void CheckpointSystem_EnsureTaskOverlay()
{
    if (g_checkpointOverlayRoot.gameObject != nullptr ||
        !ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    auto* canvasTransform = GetWorldTransform(
        g_extendedGameplayHudCanvasObject);
    g_checkpointOverlayRoot = CreateScenePanel(
        "CheckpointTaskOverlay",
        canvasTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.62f);
    g_checkpointOverlayPanel = CreateScenePanel(
        "CheckpointTaskPanel",
        g_checkpointOverlayRoot.rect,
        0.245f,
        0.205f,
        0.755f,
        0.795f,
        0.035f,
        0.046f,
        0.061f,
        0.99f);
    g_checkpointOverlayTitle = CreateSceneTextElement(
        "CheckpointTaskTitle",
        g_checkpointOverlayPanel.rect,
        "资源点任务",
        0.06f,
        0.82f,
        0.94f,
        0.95f,
        24.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f).text;
    g_checkpointOverlayBody = CreateSceneTextElement(
        "CheckpointTaskBody",
        g_checkpointOverlayPanel.rect,
        "",
        0.08f,
        0.46f,
        0.92f,
        0.79f,
        17.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f).text;
    g_checkpointOverlayWarning = CreateSceneTextElement(
        "CheckpointTaskWarning",
        g_checkpointOverlayPanel.rect,
        "",
        0.08f,
        0.34f,
        0.92f,
        0.44f,
        16.0f,
        514,
        0.95f,
        0.20f,
        0.22f,
        1.0f).text;
    SetWorldGameObjectActive(
        GetComponentGameObject(g_checkpointOverlayWarning),
        false);
    g_checkpointOverlayPrimaryButton = CreateSceneButton(
        "CheckpointTaskPrimary",
        g_checkpointOverlayPanel.rect,
        "接取",
        0.12f,
        0.15f,
        0.45f,
        0.30f,
        0.16f,
        0.38f,
        0.22f,
        0.99f,
        19.0f,
        514,
        true);
    g_checkpointOverlaySecondaryButton = CreateSceneButton(
        "CheckpointTaskSecondary",
        g_checkpointOverlayPanel.rect,
        "放弃",
        0.55f,
        0.15f,
        0.88f,
        0.30f,
        0.16f,
        0.18f,
        0.22f,
        0.99f,
        19.0f,
        514,
        true);
    SetWorldGameObjectActive(
        g_checkpointOverlayRoot.gameObject,
        false);
}

void CheckpointSystem_CloseTaskOverlay()
{
    g_checkpointOverlayMode = CheckpointOverlayMode::None;
    g_checkpointOverlayRuntimeIndex = -1;
    if (g_checkpointOverlayRoot.gameObject != nullptr)
    {
        SetWorldGameObjectActive(
            g_checkpointOverlayRoot.gameObject,
            false);
    }
    GameplayInventory_CancelPointerInteraction();
}

void CheckpointSystem_ShowTaskOffer(int runtimeIndex)
{
    if (runtimeIndex < 0 ||
        runtimeIndex >= static_cast<int>(g_checkpointRuntime.size()))
    {
        return;
    }
    const auto& runtime =
        g_checkpointRuntime[static_cast<size_t>(runtimeIndex)];
    if (runtime.definitionIndex < 0 ||
        runtime.definitionIndex >=
            static_cast<int>(g_checkpointDefinitions.size()) ||
        runtime.roll.taskIndex < 0 ||
        runtime.roll.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return;
    }
    CheckpointSystem_EnsureTaskOverlay();
    if (g_checkpointOverlayRoot.gameObject == nullptr)
    {
        return;
    }
    const auto& task = g_checkpointTaskPool[
        static_cast<size_t>(runtime.roll.taskIndex)];
    char body[512]{};
    const char* completionHint =
        task.kind == "distance"
            ? "该任务本阶段暂不启用完成判定；可按 Y 放弃并重新抽取。"
            : "接取后将按状态自动完成；进度中断规则见任务描述。";
    std::snprintf(
        body,
        sizeof(body),
        "资源点：%s品质，%d 件物资\n"
        "任务品质：%s（%d级）\n\n%s\n%s\n\n保护时间：%.1f 秒\n"
        "%s\n按 Y 放弃",
        CheckpointSystem_QualityDisplayName(
            runtime.roll.supplyQuality),
        static_cast<int>(runtime.roll.rewards.size()),
        CheckpointSystem_QualityDisplayName(task.difficulty - 1),
        task.difficulty,
        task.label.c_str(),
        task.description.empty()
            ? ""
            : task.description.c_str(),
        task.protectionSeconds,
        completionHint);
    SetSceneText(
        g_checkpointOverlayTitle,
        "资源点任务",
        24.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    SetSceneText(
        g_checkpointOverlayBody,
        body,
        17.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    if (g_checkpointOverlayWarning != nullptr)
    {
        SetWorldGameObjectActive(
            GetComponentGameObject(g_checkpointOverlayWarning),
            false);
    }
    CheckpointSystem_SetOverlayButtonLabel(
        g_checkpointOverlayPrimaryButton,
        "接取");
    CheckpointSystem_SetOverlayButtonLabel(
        g_checkpointOverlaySecondaryButton,
        "放弃");
    g_checkpointOverlayMode =
        CheckpointOverlayMode::TaskOffer;
    g_checkpointOverlayRuntimeIndex = runtimeIndex;
    g_checkpointOverlayObservedPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_checkpointOverlayObservedReleaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    g_extendedManualPointerPositionValid = false;
    SetWorldGameObjectActive(
        g_checkpointOverlayRoot.gameObject,
        true);
    BringSceneObjectToFront(
        g_checkpointOverlayRoot.gameObject);
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint task offer UI opened.");
}

void CheckpointSystem_ShowTaskReplaceConfirm(int runtimeIndex)
{
    if (runtimeIndex < 0 ||
        runtimeIndex >= static_cast<int>(g_checkpointRuntime.size()))
    {
        return;
    }
    const auto& runtime =
        g_checkpointRuntime[static_cast<size_t>(runtimeIndex)];
    if (runtime.roll.taskIndex < 0 ||
        runtime.roll.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return;
    }
    CheckpointSystem_EnsureTaskOverlay();
    if (g_checkpointOverlayRoot.gameObject == nullptr)
    {
        return;
    }
    const auto& newTask = g_checkpointTaskPool[
        static_cast<size_t>(runtime.roll.taskIndex)];
    const char* currentLabel = "";
    if (g_checkpointTrackedRuntimeIndex >= 0 &&
        g_checkpointTrackedRuntimeIndex <
            static_cast<int>(g_checkpointRuntime.size()))
    {
        const auto& current = g_checkpointRuntime[
            static_cast<size_t>(
                g_checkpointTrackedRuntimeIndex)];
        if (current.roll.taskIndex >= 0 &&
            current.roll.taskIndex <
                static_cast<int>(g_checkpointTaskPool.size()))
        {
            currentLabel = g_checkpointTaskPool[
                static_cast<size_t>(
                    current.roll.taskIndex)].label.c_str();
        }
    }
    char body[640]{};
    std::snprintf(
        body,
        sizeof(body),
        "新任务\n\n%s\n\n当前任务\n\n%s",
        newTask.label.c_str(),
        currentLabel);
    SetSceneText(
        g_checkpointOverlayTitle,
        "发现新的任务",
        24.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    SetSceneText(
        g_checkpointOverlayBody,
        body,
        16.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    if (g_checkpointOverlayWarning != nullptr)
    {
        SetSceneText(
            g_checkpointOverlayWarning,
            "是否要放弃当前任务？",
            16.0f,
            514,
            0.95f,
            0.20f,
            0.22f,
            1.0f);
        SetWorldGameObjectActive(
            GetComponentGameObject(g_checkpointOverlayWarning),
            true);
    }
    CheckpointSystem_SetOverlayButtonLabel(
        g_checkpointOverlayPrimaryButton,
        "是");
    CheckpointSystem_SetOverlayButtonLabel(
        g_checkpointOverlaySecondaryButton,
        "否");
    g_checkpointOverlayMode =
        CheckpointOverlayMode::TaskReplaceConfirm;
    g_checkpointOverlayRuntimeIndex = runtimeIndex;
    g_checkpointOverlayObservedPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_checkpointOverlayObservedReleaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    g_extendedManualPointerPositionValid = false;
    SetWorldGameObjectActive(
        g_checkpointOverlayRoot.gameObject,
        true);
    BringSceneObjectToFront(
        g_checkpointOverlayRoot.gameObject);
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint replacement-task confirmation opened.");
}

void CheckpointSystem_ShowRewardExitConfirm()
{
    CheckpointSystem_EnsureTaskOverlay();
    if (g_checkpointOverlayRoot.gameObject == nullptr)
    {
        return;
    }
    SetSceneText(
        g_checkpointOverlayTitle,
        "确认退出",
        24.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    SetSceneText(
        g_checkpointOverlayBody,
        "物资区还有物资，退出会永久丢失。\n\n确认要退出吗？",
        18.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    if (g_checkpointOverlayWarning != nullptr)
    {
        SetWorldGameObjectActive(
            GetComponentGameObject(g_checkpointOverlayWarning),
            false);
    }
    CheckpointSystem_SetOverlayButtonLabel(
        g_checkpointOverlayPrimaryButton,
        "是");
    CheckpointSystem_SetOverlayButtonLabel(
        g_checkpointOverlaySecondaryButton,
        "否");
    g_checkpointOverlayMode =
        CheckpointOverlayMode::RewardExitConfirm;
    g_checkpointOverlayRuntimeIndex = -1;
    g_checkpointOverlayObservedPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_checkpointOverlayObservedReleaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    g_extendedManualPointerPositionValid = false;
    SetWorldGameObjectActive(
        g_checkpointOverlayRoot.gameObject,
        true);
    BringSceneObjectToFront(
        g_checkpointOverlayRoot.gameObject);
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint reward exit confirmation opened.");
}

void CheckpointSystem_RequestRewardInventoryClose()
{
    if (!CheckpointSystem_IsRewardInventoryOpen())
    {
        return;
    }
    if (g_checkpointOverlayMode ==
        CheckpointOverlayMode::RewardExitConfirm)
    {
        return;
    }
    if (GameplayInventory_HasTemporaryStorageItems(
            kGameplayInventorySupply))
    {
        CheckpointSystem_ShowRewardExitConfirm();
        return;
    }
    GameplayInventory_CloseRewardMode();
}

void CheckpointSystem_RerollTaskAndRewardForRuntime(
    CheckpointRuntime& runtime)
{
    if (!runtime.roll.active ||
        runtime.roll.supplyQuality < 0 ||
        runtime.roll.supplyQuality >= 5)
    {
        return;
    }
    std::vector<int> matchingTasks;
    matchingTasks.reserve(g_checkpointTaskPool.size());
    for (size_t taskIndex = 0;
         taskIndex < g_checkpointTaskPool.size();
         ++taskIndex)
    {
        if (g_checkpointTaskPool[taskIndex].difficulty ==
            runtime.roll.supplyQuality + 1)
        {
            matchingTasks.push_back(
                static_cast<int>(taskIndex));
        }
    }
    auto& generator = CheckpointSystem_RandomEngine();
    if (!matchingTasks.empty())
    {
        std::uniform_int_distribution<size_t> taskDistribution(
            0,
            matchingTasks.size() - 1);
        runtime.roll.taskIndex =
            matchingTasks[taskDistribution(generator)];
    }

    std::uniform_int_distribution<int> itemDistribution(
        0,
        static_cast<int>(
            kEquipmentInventoryDefinitionCount) - 1);
    std::discrete_distribution<int> itemQualityDistribution(
        g_checkpointItemQualityWeights[
            runtime.roll.supplyQuality],
        g_checkpointItemQualityWeights[
            runtime.roll.supplyQuality] + 5);
    const auto& definition =
        g_checkpointDefinitions[runtime.definitionIndex];
    const int supplyCount =
        CheckpointSystem_GetSupplyCount(
            definition.metadata);
    runtime.roll.rewards.clear();
    runtime.roll.rewards.reserve(
        static_cast<size_t>(supplyCount));
    for (int index = 0; index < supplyCount; ++index)
    {
        runtime.roll.rewards.push_back({
            itemDistribution(generator),
            itemQualityDistribution(generator),
        });
    }
}

void CheckpointSystem_BeginAbandonTrackedTask(
    int replacementRuntimeIndex)
{
    if (g_checkpointTrackedRuntimeIndex < 0 ||
        g_checkpointTrackedRuntimeIndex >=
            static_cast<int>(g_checkpointRuntime.size()))
    {
        return;
    }
    auto& runtime = g_checkpointRuntime[
        static_cast<size_t>(
            g_checkpointTrackedRuntimeIndex)];
    g_checkpointAbandonedTaskIndex =
        runtime.roll.taskIndex;
    CheckpointSystem_StopActiveTask(true, true);
    runtime.roll.searchCompleted = false;
    runtime.roll.taskCompleted = false;
    runtime.roll.taskFailed = false;
    runtime.roll.rewardGranted = false;
    runtime.roll.taskProgress = 0.0f;
    runtime.roll.taskVisitedCount = 0;
    CheckpointSystem_RerollTaskAndRewardForRuntime(runtime);
    if (runtime.visualRoot != nullptr)
    {
        SetWorldGameObjectActive(runtime.visualRoot, true);
    }
    g_checkpointTrackerAbandoning = true;
    g_checkpointTrackerAbandonedAt = GetTickCount64();
    g_checkpointPendingReplacementRuntimeIndex =
        replacementRuntimeIndex;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        replacementRuntimeIndex >= 0
            ? L"Checkpoint current task abandoned for a replacement task."
            : L"Checkpoint accepted task abandoned by Y.");
}

void CheckpointSystem_HandleOverlayButton(bool primary)
{
    if (g_checkpointOverlayMode ==
        CheckpointOverlayMode::TaskOffer)
    {
        bool started = !primary;
        if (primary &&
            g_checkpointOverlayRuntimeIndex >= 0 &&
            g_checkpointOverlayRuntimeIndex <
                static_cast<int>(g_checkpointRuntime.size()))
        {
            started = CheckpointSystem_StartActiveTask(
                g_checkpointOverlayRuntimeIndex);
            if (started)
            {
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint task accepted.");
            }
        }
        if (started)
        {
            CheckpointSystem_CloseTaskOverlay();
        }
        return;
    }
    if (g_checkpointOverlayMode ==
        CheckpointOverlayMode::TaskReplaceConfirm)
    {
        const int replacementIndex =
            g_checkpointOverlayRuntimeIndex;
        if (primary)
        {
            CheckpointSystem_BeginAbandonTrackedTask(
                replacementIndex);
        }
        CheckpointSystem_CloseTaskOverlay();
        return;
    }
    if (g_checkpointOverlayMode ==
        CheckpointOverlayMode::RewardExitConfirm)
    {
        CheckpointSystem_CloseTaskOverlay();
        if (primary)
        {
            GameplayInventory_CloseRewardMode();
        }
    }
}

void CheckpointSystem_ProcessOverlayPointer()
{
    if (g_checkpointOverlayMode == CheckpointOverlayMode::None ||
        g_checkpointOverlayRoot.gameObject == nullptr)
    {
        return;
    }
    const unsigned int releaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    if (releaseSerial == g_checkpointOverlayObservedReleaseSerial)
    {
        return;
    }
    g_checkpointOverlayObservedReleaseSerial = releaseSerial;
    const HWND window = FindGameWindow();
    RECT clientRect{};
    POINT point{
        g_extendedManualPointerCurrentX.load(),
        g_extendedManualPointerCurrentY.load()};
    if (window == nullptr ||
        !GetClientRect(window, &clientRect) ||
        !ScreenToClient(window, &point))
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
    const float x = static_cast<float>(point.x) / width;
    const float y = 1.0f - static_cast<float>(point.y) / height;
    constexpr float primaryMinX = 0.245f + 0.12f * 0.51f;
    constexpr float primaryMaxX = 0.245f + 0.45f * 0.51f;
    constexpr float secondaryMinX = 0.245f + 0.55f * 0.51f;
    constexpr float secondaryMaxX = 0.245f + 0.88f * 0.51f;
    constexpr float buttonMinY = 0.205f + 0.15f * 0.59f;
    constexpr float buttonMaxY = 0.205f + 0.30f * 0.59f;
    if (y >= buttonMinY && y <= buttonMaxY)
    {
        if (x >= primaryMinX && x <= primaryMaxX)
        {
            CheckpointSystem_HandleOverlayButton(true);
        }
        else if (x >= secondaryMinX && x <= secondaryMaxX)
        {
            CheckpointSystem_HandleOverlayButton(false);
        }
    }
}

void CheckpointSystem_EnsureTaskTracker()
{
    if (g_checkpointTrackerRoot.gameObject != nullptr ||
        !ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    g_checkpointTrackerRoot = CreateScenePanel(
        "CheckpointTaskTracker",
        GetWorldTransform(g_extendedGameplayHudCanvasObject),
        0.675f,
        0.805f,
        0.976f,
        0.875f,
        0.025f,
        0.035f,
        0.050f,
        0.94f);
    g_checkpointTrackerTitle = CreateSceneTextElement(
        "CheckpointTaskTrackerTitle",
        g_checkpointTrackerRoot.rect,
        "",
        0.04f,
        0.53f,
        0.96f,
        0.94f,
        14.0f,
        514,
        0.92f,
        0.94f,
        0.98f,
        1.0f).text;
    g_checkpointTrackerProgress = CreateSceneTextElement(
        "CheckpointTaskTrackerProgress",
        g_checkpointTrackerRoot.rect,
        "",
        0.04f,
        0.27f,
        0.96f,
        0.56f,
        10.0f,
        514,
        0.68f,
        0.74f,
        0.82f,
        1.0f).text;
    g_checkpointTrackerBarBackground = CreateScenePanel(
        "CheckpointTaskTrackerBarBackground",
        g_checkpointTrackerRoot.rect,
        0.04f,
        0.08f,
        0.96f,
        0.22f,
        0.055f,
        0.065f,
        0.085f,
        0.98f);
    g_checkpointTrackerBarFill = CreateScenePanel(
        "CheckpointTaskTrackerBarFill",
        g_checkpointTrackerRoot.rect,
        0.04f,
        0.08f,
        0.06f,
        0.22f,
        0.16f,
        0.38f,
        0.22f,
        1.0f);
    SetWorldGameObjectActive(
        g_checkpointTrackerRoot.gameObject,
        false);
}

void CheckpointSystem_UpdateTaskTracker()
{
    if (g_checkpointTrackedRuntimeIndex < 0)
    {
        for (size_t index = 0;
             index < g_checkpointRuntime.size();
             ++index)
        {
            const auto& runtime = g_checkpointRuntime[index];
            if (runtime.roll.searchCompleted &&
                (!runtime.roll.rewardGranted ||
                 runtime.roll.taskFailed))
            {
                g_checkpointTrackedRuntimeIndex =
                    static_cast<int>(index);
                break;
            }
        }
    }
    if (g_checkpointTrackedRuntimeIndex < 0 ||
        g_checkpointTrackedRuntimeIndex >=
            static_cast<int>(g_checkpointRuntime.size()))
    {
        return;
    }
    auto& runtime = g_checkpointRuntime[
        static_cast<size_t>(g_checkpointTrackedRuntimeIndex)];
    if ((!runtime.roll.searchCompleted &&
         !g_checkpointTrackerAbandoning) ||
        runtime.roll.taskIndex < 0 ||
        runtime.roll.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return;
    }
    CheckpointSystem_EnsureTaskTracker();
    if (g_checkpointTrackerRoot.gameObject == nullptr)
    {
        return;
    }
    const int displayTaskIndex =
        g_checkpointTrackerAbandoning &&
        g_checkpointAbandonedTaskIndex >= 0
            ? g_checkpointAbandonedTaskIndex
            : runtime.roll.taskIndex;
    if (displayTaskIndex < 0 ||
        displayTaskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return;
    }
    const auto& task = g_checkpointTaskPool[
        static_cast<size_t>(displayTaskIndex)];
    const bool terminal =
        runtime.roll.taskCompleted ||
        runtime.roll.taskFailed ||
        g_checkpointTrackerAbandoning;
    unsigned long long completionElapsed = 0;
    if (runtime.roll.taskCompleted)
    {
        if (g_checkpointTrackerCompletedAt == 0)
        {
            g_checkpointTrackerCompletedAt = GetTickCount64();
        }
        completionElapsed =
            GetTickCount64() - g_checkpointTrackerCompletedAt;
    }
    else if (runtime.roll.taskFailed)
    {
        if (g_checkpointTrackerFailedAt == 0)
        {
            g_checkpointTrackerFailedAt = GetTickCount64();
        }
        completionElapsed =
            GetTickCount64() - g_checkpointTrackerFailedAt;
    }
    else if (g_checkpointTrackerAbandoning)
    {
        if (g_checkpointTrackerAbandonedAt == 0)
        {
            g_checkpointTrackerAbandonedAt = GetTickCount64();
        }
        completionElapsed =
            GetTickCount64() - g_checkpointTrackerAbandonedAt;
    }
    const float progress =
        terminal
            ? 1.0f
            : std::clamp(runtime.roll.taskProgress, 0.0f, 1.0f);
    const float fade =
        terminal
            ? std::clamp(
                static_cast<float>(completionElapsed) / 1200.0f,
                0.0f,
                1.0f)
            : 0.0f;
    const float alpha = 1.0f - fade;
    const float shift = fade * 0.12f;
    char title[320]{};
    std::snprintf(
        title,
        sizeof(title),
        g_checkpointTrackerAbandoning
            ? "任务已放弃  %s"
            : (runtime.roll.taskFailed
                ? "任务失败  %s"
                : (runtime.roll.taskCompleted
                    ? "任务完成  %s"
                    : "任务  %s")),
        task.label.c_str());
    char progressText[128]{};
    if (g_checkpointTrackerAbandoning)
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "已放弃  ·  100%%");
    }
    else if (runtime.roll.taskFailed)
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "任务失败  ·  未获得奖励");
    }
    else if (runtime.roll.taskCompleted)
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "完成  ·  100%%");
    }
    else if (g_checkpointActiveTask.active &&
             g_checkpointActiveTask.protectionRemaining > 0.0f)
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "保护中  ·  %.1f 秒  ·  Q 锁定",
            g_checkpointActiveTask.protectionRemaining);
    }
    else if (g_checkpointActiveTask.resetNoticeUntil >
             GetTickCount64())
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "状态中断  ·  进度已清零  ·  Y 放弃");
    }
    else if (task.kind == "near_npc_hold")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "路人附近 %.1f/%.0f 秒  ·  Y 放弃",
            g_checkpointActiveTask.conditionSeconds,
            task.requiredSeconds);
    }
    else if (task.kind == "near_npc_orgasm")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "在路人附近完成一次高潮  ·  Y 放弃");
    }
    else if (task.kind == "pose_hold")
    {
        const bool locationConfirmed =
            g_checkpointActiveTask.targetRuntimeIndices.empty() ||
            (!g_checkpointActiveTask.targetVisited.empty() &&
             g_checkpointActiveTask.targetVisited[0]);
        if (!locationConfirmed)
        {
            std::snprintf(
                progressText,
                sizeof(progressText),
                "先到黄点按 F 确认位置  ·  Y 放弃");
        }
        else
        {
            std::snprintf(
                progressText,
                sizeof(progressText),
                "%s  ·  %.1f/%.0f 秒%s  ·  Y 放弃",
                CheckpointSystem_SelectedActionLabel(),
                g_checkpointActiveTask.conditionSeconds,
                task.requiredSeconds,
                task.lightOnly ? "  ·  光源下" : "");
        }
    }
    else if (task.kind == "orgasm_location")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "进入黄点范围后触发高潮  ·  Y 放弃");
    }
    else if (task.kind == "orgasm_checkpoints")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "高潮点 %d/%d  ·  进入黄点范围后触发高潮  ·  Y 放弃",
            runtime.roll.taskVisitedCount,
            std::max(1, task.targetCount));
    }
    else if (task.kind == "vending_once")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "以裸露状态使用一次售货机  ·  Y 放弃");
    }
    else if (task.kind == "bench_hold")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "长椅 %.1f/%.0f 秒  ·  离椅重计  ·  Y 放弃",
            g_checkpointActiveTask.conditionSeconds,
            task.requiredSeconds);
    }
    else if (task.kind == "escalator_once")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "完整乘坐自动扶梯  ·  %d%%  ·  Y 放弃",
            static_cast<int>(
                std::clamp(
                    runtime.roll.taskProgress,
                    0.0f,
                    1.0f) *
                100.0f));
    }
    else if (task.kind == "elevator_hold" ||
             task.kind == "elevator_npc_hold")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            task.kind == "elevator_npc_hold"
                ? "电梯与路人 %.1f/%.0f 秒  ·  Y 放弃"
                : "电梯内 %.1f/%.0f 秒  ·  Y 放弃",
            g_checkpointActiveTask.conditionSeconds,
            task.requiredSeconds);
    }
    else if (task.kind == "checkpoints")
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "检查点 %d/%d  ·  %d%%  ·  Y 放弃",
            runtime.roll.taskVisitedCount,
            std::max(1, task.targetCount),
            static_cast<int>(
                std::clamp(
                    runtime.roll.taskProgress,
                    0.0f,
                    1.0f) *
                100.0f));
    }
    else if (task.kind == "distance")
    {
        const float required =
            std::max(
                1.0f,
                static_cast<float>(task.distance));
        const float traveled =
            std::clamp(
                runtime.roll.taskProgress,
                0.0f,
                1.0f) *
            required;
        std::snprintf(
            progressText,
            sizeof(progressText),
            "%s %.1f/%.0f 米  ·  %d%%  ·  Y 放弃",
            task.gait == "crawl"
                ? "爬行"
                : "行走",
            traveled,
            required,
            static_cast<int>(
                std::clamp(
                    runtime.roll.taskProgress,
                    0.0f,
                    1.0f) *
                100.0f));
    }
    else
    {
        std::snprintf(
            progressText,
            sizeof(progressText),
            "进行中  ·  %d%%  ·  Y 放弃",
            static_cast<int>(
                std::clamp(
                    runtime.roll.taskProgress,
                    0.0f,
                    1.0f) *
                100.0f));
    }
    SetSceneRect(
        g_checkpointTrackerRoot.rect,
        0.675f + shift,
        0.805f,
        0.976f + shift,
        0.875f);
    SetSceneGraphicColor(
        g_checkpointTrackerRoot.image,
        g_checkpointTrackerAbandoning
            ? 0.42f
            : (runtime.roll.taskFailed
                ? 0.34f
                : (runtime.roll.taskCompleted ? 0.10f : 0.025f)),
        g_checkpointTrackerAbandoning
            ? 0.055f
            : (runtime.roll.taskFailed
                ? 0.045f
                : (runtime.roll.taskCompleted ? 0.38f : 0.035f)),
        g_checkpointTrackerAbandoning
            ? 0.065f
            : (runtime.roll.taskFailed
                ? 0.065f
                : (runtime.roll.taskCompleted ? 0.18f : 0.050f)),
        0.94f * alpha);
    SetSceneText(
        g_checkpointTrackerTitle,
        title,
        14.0f,
        514,
        g_checkpointTrackerAbandoning
            ? 1.0f
            : (runtime.roll.taskFailed
                ? 1.0f
                : (runtime.roll.taskCompleted ? 0.80f : 0.92f)),
        g_checkpointTrackerAbandoning
            ? 0.34f
            : (runtime.roll.taskFailed
                ? 0.34f
                : (runtime.roll.taskCompleted ? 1.0f : 0.94f)),
        g_checkpointTrackerAbandoning
            ? 0.34f
            : (runtime.roll.taskFailed
                ? 0.34f
                : (runtime.roll.taskCompleted ? 0.82f : 0.98f)),
        alpha);
    SetSceneText(
        g_checkpointTrackerProgress,
        progressText,
        10.0f,
        514,
        g_checkpointTrackerAbandoning
            ? 1.0f
            : (runtime.roll.taskFailed
                ? 0.92f
                : (runtime.roll.taskCompleted ? 0.72f : 0.68f)),
        g_checkpointTrackerAbandoning
            ? 0.34f
            : (runtime.roll.taskFailed
                ? 0.52f
                : (runtime.roll.taskCompleted ? 1.0f : 0.74f)),
        g_checkpointTrackerAbandoning
            ? 0.34f
            : (runtime.roll.taskFailed
                ? 0.55f
                : (runtime.roll.taskCompleted ? 0.74f : 0.82f)),
        alpha);
    SetSceneGraphicColor(
        g_checkpointTrackerBarBackground.image,
        0.055f,
        0.065f,
        0.085f,
        0.98f * alpha);
    const float fillMinX = 0.04f;
    const float fillMaxX =
        fillMinX + 0.92f * std::max(0.02f, progress);
    SetSceneRect(
        g_checkpointTrackerBarFill.rect,
        fillMinX,
        0.08f,
        fillMaxX,
        0.22f);
    SetSceneGraphicColor(
        g_checkpointTrackerBarFill.image,
        g_checkpointTrackerAbandoning
            ? 0.92f
            : (runtime.roll.taskFailed
                ? 0.82f
                : (runtime.roll.taskCompleted ? 0.28f : 0.16f)),
        g_checkpointTrackerAbandoning
            ? 0.16f
            : (runtime.roll.taskFailed
                ? 0.16f
                : (runtime.roll.taskCompleted ? 0.86f : 0.38f)),
        g_checkpointTrackerAbandoning
            ? 0.16f
            : (runtime.roll.taskFailed
                ? 0.18f
                : (runtime.roll.taskCompleted ? 0.36f : 0.22f)),
        alpha);
    SetWorldGameObjectActive(
        g_checkpointTrackerRoot.gameObject,
        true);
    if (terminal && fade >= 1.0f)
    {
        SetWorldGameObjectActive(
            g_checkpointTrackerRoot.gameObject,
            false);
        g_checkpointTrackedRuntimeIndex = -1;
        g_checkpointTrackerCompletedAt = 0;
        g_checkpointTrackerFailedAt = 0;
        const int replacement =
            g_checkpointPendingReplacementRuntimeIndex;
        g_checkpointPendingReplacementRuntimeIndex = -1;
        g_checkpointTrackerAbandoning = false;
        g_checkpointTrackerAbandonedAt = 0;
        g_checkpointAbandonedTaskIndex = -1;
        if (replacement >= 0 &&
            replacement <
                static_cast<int>(g_checkpointRuntime.size()))
        {
            if (CheckpointSystem_StartActiveTask(replacement))
            {
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint replacement task accepted.");
            }
        }
        else if (g_checkpointRewardOpenPending)
        {
            CheckpointSystem_ProcessPendingReward();
        }
    }
}

bool CheckpointSystem_TaskGameplayPaused()
{
    return g_checkpointOverlayMode !=
               CheckpointOverlayMode::None ||
           g_extendedOriginalSystemMenuVisible.load() ||
           g_extendedOriginalInteractionMenuActive.load() ||
           g_extendedOriginalHandcuffsPanelActive.load() ||
           g_gameplayInventoryOpen.load() ||
           NpcDialogue_IsActive();
}

bool CheckpointSystem_TaskConditionValid(
    const CheckpointTaskDefinition& task,
    const GameplayStatusHudMovementState& movement)
{
    if (task.requiresNaked &&
        g_extendedSpecialCoatMode.load())
    {
        return false;
    }
    if (task.gait == "walk")
    {
        return
            !movement.isCrouch &&
            !(movement.moveSpeed > 0.02f &&
              movement.isDash);
    }
    if (task.gait == "crawl")
    {
        return movement.isCrouch;
    }
    return true;
}

void CheckpointSystem_ResetActiveTaskProgress(
    const char* reason)
{
    if (!g_checkpointActiveTask.active)
    {
        return;
    }
    auto& source = g_checkpointRuntime[
        static_cast<size_t>(
            g_checkpointActiveTask.sourceRuntimeIndex)];
    source.roll.taskProgress = 0.0f;
    source.roll.taskVisitedCount = 0;
    g_checkpointActiveTask.accumulatedDistance = 0.0f;
    g_checkpointActiveTask.conditionSeconds = 0.0f;
    g_checkpointActiveTask.escalatorStartY = 0.0f;
    g_checkpointActiveTask.escalatorMaxVerticalDistance = 0.0f;
    std::fill(
        g_checkpointActiveTask.targetOrgasmDone.begin(),
        g_checkpointActiveTask.targetOrgasmDone.end(),
        false);
    std::fill(
        g_checkpointActiveTask.targetVisited.begin(),
        g_checkpointActiveTask.targetVisited.end(),
        false);
    for (const int runtimeIndex :
         g_checkpointActiveTask.targetRuntimeIndices)
    {
        if (runtimeIndex < 0 ||
            runtimeIndex >=
                static_cast<int>(g_checkpointRuntime.size()))
        {
            continue;
        }
        auto& target =
            g_checkpointRuntime[
                static_cast<size_t>(runtimeIndex)];
        if (target.visualRoot != nullptr)
        {
            SetWorldGameObjectActive(
                target.visualRoot,
                true);
        }
    }

    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(
                g_checkpointActiveTask.taskIndex)];
    if (task.kind == "reach" &&
        !g_checkpointActiveTask.targetRuntimeIndices.empty())
    {
        const auto& targetDefinition =
            g_checkpointDefinitions[
                g_checkpointRuntime[
                    static_cast<size_t>(
                        g_checkpointActiveTask
                            .targetRuntimeIndices[0])]
                    .definitionIndex];
        g_checkpointActiveTask.baselineDistance =
            std::sqrt(
                CheckpointSystem_DistanceSquaredToDefinition(
                    targetDefinition,
                    g_checkpointActiveTask.lastPlayerX,
                    g_checkpointActiveTask.lastPlayerY,
                    g_checkpointActiveTask.lastPlayerZ));
    }
    else if (task.kind == "distance")
    {
        g_checkpointActiveTask.baselineDistance = 0.0f;
        g_checkpointActiveTask.targetsReady = true;
    }
    g_checkpointActiveTask.resetNoticeUntil =
        GetTickCount64() + 1500;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint task progress reset: task=%S reason=%S.",
        task.id.c_str(),
        reason);
}

bool CheckpointSystem_FailActiveTask(const char* reason)
{
    if (!g_checkpointActiveTask.active ||
        g_checkpointActiveTask.sourceRuntimeIndex < 0 ||
        g_checkpointActiveTask.sourceRuntimeIndex >=
            static_cast<int>(g_checkpointRuntime.size()))
    {
        return false;
    }
    auto& source = g_checkpointRuntime[
        static_cast<size_t>(
            g_checkpointActiveTask.sourceRuntimeIndex)];
    if (source.roll.taskIndex < 0 ||
        source.roll.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return false;
    }
    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(source.roll.taskIndex)];
    source.roll.taskFailed = true;
    source.roll.taskCompleted = false;
    source.roll.rewardGranted = false;
    source.roll.taskProgress = 1.0f;
    g_checkpointTrackedRuntimeIndex =
        g_checkpointActiveTask.sourceRuntimeIndex;
    g_checkpointTrackerCompletedAt = 0;
    g_checkpointTrackerFailedAt = GetTickCount64();
    const int failedSource =
        g_checkpointActiveTask.sourceRuntimeIndex;
    CheckpointSystem_StopActiveTask(true, false);
    g_checkpointTrackedRuntimeIndex = failedSource;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint task failed: task=%S reason=%S.",
        task.id.c_str(),
        reason);
    CheckpointSystem_RequestFailureReturn(
        L"checkpoint task failure");
    return true;
}

bool CheckpointSystem_CompleteActiveTask(const char* reason)
{
    if (!g_checkpointActiveTask.active ||
        g_checkpointActiveTask.sourceRuntimeIndex < 0 ||
        g_checkpointActiveTask.sourceRuntimeIndex >=
            static_cast<int>(g_checkpointRuntime.size()))
    {
        return false;
    }
    auto& source = g_checkpointRuntime[
        static_cast<size_t>(
            g_checkpointActiveTask.sourceRuntimeIndex)];
    if (source.roll.taskIndex < 0 ||
        source.roll.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()) ||
        source.roll.rewards.empty())
    {
        return CheckpointSystem_FailActiveTask(
            "invalid pre-rolled reward");
    }
    static unsigned long long nextRewardAttempt = 0;
    const unsigned long long now = GetTickCount64();
    if (now < nextRewardAttempt)
    {
        return true;
    }

    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(source.roll.taskIndex)];
    const auto& sourceDefinition =
        g_checkpointDefinitions[source.definitionIndex];
    std::vector<GameplayInventoryLootRequest> lootRequests;
    lootRequests.reserve(source.roll.rewards.size());
    for (const auto& reward : source.roll.rewards)
    {
        if (reward.itemDefinition < 0 ||
            reward.itemDefinition >=
                kEquipmentInventoryDefinitionCount)
        {
            return CheckpointSystem_FailActiveTask(
                "invalid pre-rolled reward item");
        }
        lootRequests.push_back({
            reward.itemDefinition,
            reward.itemQuality,
        });
    }
    const bool granted =
        GameplayInventory_AddLootBatch(
            lootRequests,
            sourceDefinition.id);
    if (!granted)
    {
        nextRewardAttempt = now + 500;
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint task completion waiting for reward inventory space: task=%S.",
            task.id.c_str());
        return true;
    }

    const int sourceRuntimeIndex =
        g_checkpointActiveTask.sourceRuntimeIndex;
    source.roll.taskCompleted = true;
    source.roll.taskFailed = false;
    source.roll.rewardGranted = true;
    source.roll.taskProgress = 1.0f;
    g_checkpointTrackedRuntimeIndex = sourceRuntimeIndex;
    g_checkpointTrackerCompletedAt = now;
    g_checkpointTrackerFailedAt = 0;
    g_checkpointRewardOpenPending = true;
    for (const auto& reward : source.roll.rewards)
    {
        g_checkpointPendingLoot.push_back({
            sourceDefinition.id,
            reward.itemDefinition,
            reward.itemQuality,
        });
    }
    CheckpointSystem_StopActiveTask(true, true);
    g_checkpointTrackedRuntimeIndex = sourceRuntimeIndex;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint task completed: id=%s task=%S rewards=%u reason=%S.",
        Utf8ToWide(sourceDefinition.id).c_str(),
        task.id.c_str(),
        static_cast<unsigned int>(
            source.roll.rewards.size()),
        reason);
    return true;
}

void CheckpointSystem_HandleActiveTaskConditionFailure(
    const char* reason)
{
    if (!g_checkpointActiveTask.active)
    {
        return;
    }
    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(
                g_checkpointActiveTask.taskIndex)];
    if (task.interruptPolicy == "keep")
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint task condition paused without losing progress: task=%S reason=%S.",
            task.id.c_str(),
            reason);
    }
    else if (task.interruptPolicy == "fail")
    {
        CheckpointSystem_FailActiveTask(reason);
    }
    else
    {
        CheckpointSystem_ResetActiveTaskProgress(reason);
    }
}

bool CheckpointSystem_UpdateActiveTaskTargets(
    float elapsedSeconds,
    float playerX,
    float playerY,
    float playerZ,
    bool ecstasyStarted)
{
    if (!g_checkpointActiveTask.active ||
        g_checkpointActiveTask.taskIndex < 0 ||
        g_checkpointActiveTask.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return false;
    }
    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(
                g_checkpointActiveTask.taskIndex)];
    auto& source = g_checkpointRuntime[
        static_cast<size_t>(
            g_checkpointActiveTask.sourceRuntimeIndex)];
    const int currentAction =
        CheckpointSystem_ReadCurrentActionType();
    const auto advanceHold =
        [&](bool condition, float requiredSeconds)
        {
            if (condition)
            {
                g_checkpointActiveTask.conditionSeconds +=
                    elapsedSeconds;
            }
            else
            {
                g_checkpointActiveTask.conditionSeconds = 0.0f;
            }
            const float required =
                std::max(0.05f, requiredSeconds);
            source.roll.taskProgress = std::clamp(
                g_checkpointActiveTask.conditionSeconds /
                    required,
                0.0f,
                1.0f);
            return condition &&
                g_checkpointActiveTask.conditionSeconds >=
                    required;
        };

    if (task.kind == "distance")
    {
        const float traveled =
            g_checkpointActiveTask.accumulatedDistance;
        const float required =
            std::max(1.0f, static_cast<float>(task.distance));
        source.roll.taskProgress =
            std::clamp(traveled / required, 0.0f, 1.0f);
        if (traveled >= required)
        {
            return CheckpointSystem_CompleteActiveTask(
                task.gait == "crawl"
                    ? "crawling distance reached"
                    : "walking distance reached");
        }
        return true;
    }
    if (task.kind == "near_npc_hold")
    {
        // MissionExposeByNearNpc's state is PlayerStateModel.IsNearNpcForMission.
        // This task explicitly keeps accumulated progress when the player
        // leaves the NPC or temporarily loses the exposed condition.
        if (CheckpointSystem_IsNearMissionNpc(false))
        {
            g_checkpointActiveTask.conditionSeconds +=
                elapsedSeconds;
        }
        const float required = std::max(
            0.05f,
            task.requiredSeconds);
        source.roll.taskProgress = std::clamp(
            g_checkpointActiveTask.conditionSeconds /
                required,
            0.0f,
            1.0f);
        if (g_checkpointActiveTask.conditionSeconds >= required)
        {
            return CheckpointSystem_CompleteActiveTask(
                "nearby NPC duration reached");
        }
        return true;
    }
    if (task.kind == "near_npc_orgasm")
    {
        if (ecstasyStarted &&
            CheckpointSystem_IsNearMissionNpc(true))
        {
            return CheckpointSystem_CompleteActiveTask(
                "ecstasy near mission NPC");
        }
        return true;
    }
    if (task.kind == "pose_hold")
    {
        bool targetCondition = true;
        if (!g_checkpointActiveTask.targetRuntimeIndices.empty())
        {
            const int runtimeIndex =
                g_checkpointActiveTask.targetRuntimeIndices[0];
            targetCondition = false;
            if (runtimeIndex >= 0 &&
                runtimeIndex <
                    static_cast<int>(g_checkpointRuntime.size()) &&
                !g_checkpointActiveTask.targetVisited.empty() &&
                g_checkpointActiveTask.targetVisited[0])
            {
                const auto& definition =
                    g_checkpointDefinitions[
                        g_checkpointRuntime[
                            static_cast<size_t>(runtimeIndex)]
                            .definitionIndex];
                targetCondition =
                    CheckpointSystem_PlayerWithinTaskTarget(
                        definition,
                        playerX,
                        playerY,
                        playerZ,
                        task.triggerRadius);
            }
        }
        const bool actionCondition =
            currentAction ==
            g_checkpointActiveTask.selectedActionType;
        const bool lightCondition =
            !task.lightOnly ||
            CheckpointSystem_IsMissionLightActive();
        if (advanceHold(
                targetCondition &&
                    actionCondition &&
                    lightCondition,
                task.requiredSeconds))
        {
            return CheckpointSystem_CompleteActiveTask(
                task.lightOnly
                    ? "pose held under mission light"
                    : "pose held at task location");
        }
        return true;
    }
    if (task.kind == "vending_once")
    {
        if (currentAction == 10011)
        {
            return CheckpointSystem_CompleteActiveTask(
                "vending machine action");
        }
        return true;
    }
    if (task.kind == "bench_hold")
    {
        if (advanceHold(
                currentAction == 10026 ||
                    currentAction == 10027,
                task.requiredSeconds))
        {
            return CheckpointSystem_CompleteActiveTask(
                "bench duration reached");
        }
        return true;
    }
    if (task.kind == "escalator_once")
    {
        const bool pushed =
            CheckpointSystem_IsPlayerOnEscalator();
        if (pushed &&
            !g_checkpointActiveTask.escalatorWasPushed)
        {
            g_checkpointActiveTask.conditionSeconds = 0.0f;
            g_checkpointActiveTask.escalatorStartY = playerY;
            g_checkpointActiveTask.escalatorMaxVerticalDistance =
                0.0f;
        }
        if (pushed)
        {
            g_checkpointActiveTask.conditionSeconds +=
                elapsedSeconds;
            g_checkpointActiveTask.escalatorMaxVerticalDistance =
                std::max(
                    g_checkpointActiveTask
                        .escalatorMaxVerticalDistance,
                    std::fabs(
                        playerY -
                        g_checkpointActiveTask.escalatorStartY));
            source.roll.taskProgress = std::clamp(
                std::max(
                    g_checkpointActiveTask.conditionSeconds / 4.0f,
                    g_checkpointActiveTask
                            .escalatorMaxVerticalDistance /
                        1.5f) *
                    0.95f,
                0.0f,
                0.95f);
        }
        else if (g_checkpointActiveTask.escalatorWasPushed)
        {
            const bool completed =
                g_checkpointActiveTask.conditionSeconds >= 2.0f &&
                g_checkpointActiveTask
                        .escalatorMaxVerticalDistance >=
                    1.25f;
            if (completed)
            {
                g_checkpointActiveTask.escalatorWasPushed =
                    false;
                return CheckpointSystem_CompleteActiveTask(
                    "escalator trip completed");
            }
            g_checkpointActiveTask.conditionSeconds = 0.0f;
            g_checkpointActiveTask
                .escalatorMaxVerticalDistance = 0.0f;
            source.roll.taskProgress = 0.0f;
        }
        g_checkpointActiveTask.escalatorWasPushed = pushed;
        return true;
    }
    if (task.kind == "elevator_hold" ||
        task.kind == "elevator_npc_hold")
    {
        const bool condition =
            CheckpointSystem_IsPlayerInsideElevator() &&
            (task.kind == "elevator_hold" ||
             CheckpointSystem_IsNpcInsideElevator());
        if (advanceHold(
                condition,
                task.requiredSeconds))
        {
            return CheckpointSystem_CompleteActiveTask(
                task.kind == "elevator_npc_hold"
                    ? "elevator duration with NPC reached"
                    : "elevator duration reached");
        }
        return true;
    }
    if (task.kind == "orgasm_location" ||
        task.kind == "orgasm_checkpoints")
    {
        if (ecstasyStarted)
        {
            for (size_t index = 0;
                 index <
                     g_checkpointActiveTask
                         .targetRuntimeIndices.size();
                 ++index)
            {
                if (index <
                        g_checkpointActiveTask
                            .targetOrgasmDone.size() &&
                    g_checkpointActiveTask
                        .targetOrgasmDone[index])
                {
                    continue;
                }
                const int runtimeIndex =
                    g_checkpointActiveTask
                        .targetRuntimeIndices[index];
                if (runtimeIndex < 0 ||
                    runtimeIndex >=
                        static_cast<int>(
                            g_checkpointRuntime.size()))
                {
                    continue;
                }
                auto& targetRuntime =
                    g_checkpointRuntime[
                        static_cast<size_t>(runtimeIndex)];
                const auto& targetDefinition =
                    g_checkpointDefinitions[
                        targetRuntime.definitionIndex];
                if (!CheckpointSystem_PlayerWithinTaskTarget(
                        targetDefinition,
                        playerX,
                        playerY,
                        playerZ,
                        task.triggerRadius))
                {
                    continue;
                }
                if (index <
                    g_checkpointActiveTask
                        .targetOrgasmDone.size())
                {
                    g_checkpointActiveTask
                        .targetOrgasmDone[index] = true;
                }
                if (index <
                    g_checkpointActiveTask
                        .targetVisited.size())
                {
                    g_checkpointActiveTask
                        .targetVisited[index] = true;
                }
                ++source.roll.taskVisitedCount;
                source.roll.taskProgress = std::clamp(
                    static_cast<float>(
                        source.roll.taskVisitedCount) /
                        static_cast<float>(
                            std::max(1, task.targetCount)),
                    0.0f,
                    1.0f);
                if (targetRuntime.visualRoot != nullptr)
                {
                    SetWorldGameObjectActive(
                        targetRuntime.visualRoot,
                        false);
                }
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                    L"Checkpoint orgasm target completed: task=%S point=%s count=%d/%d.",
                    task.id.c_str(),
                    Utf8ToWide(targetDefinition.id).c_str(),
                    source.roll.taskVisitedCount,
                    task.targetCount);
                if (source.roll.taskVisitedCount >=
                    task.targetCount)
                {
                    return
                        CheckpointSystem_CompleteActiveTask(
                            "orgasm completed at all task locations");
                }
                break;
            }
        }
        return true;
    }
    if (task.kind != "reach" &&
        task.kind != "checkpoints")
    {
        return true;
    }
    if (task.kind == "reach")
    {
        if (g_checkpointActiveTask.targetRuntimeIndices.empty())
        {
            return false;
        }
        const int runtimeIndex =
            g_checkpointActiveTask.targetRuntimeIndices[0];
        if (runtimeIndex < 0 ||
            runtimeIndex >=
                static_cast<int>(g_checkpointRuntime.size()))
        {
            return false;
        }
        const auto& targetDefinition =
            g_checkpointDefinitions[
                g_checkpointRuntime[
                    static_cast<size_t>(runtimeIndex)]
                    .definitionIndex];
        const float dx =
            g_checkpointActiveTask.lastPlayerX -
            targetDefinition.x;
        const float dy =
            g_checkpointActiveTask.lastPlayerY -
            targetDefinition.y;
        const float dz =
            g_checkpointActiveTask.lastPlayerZ -
            targetDefinition.z;
        const float distance =
            std::sqrt(dx * dx + dy * dy + dz * dz);
        if (g_checkpointActiveTask.baselineDistance >
            0.01f)
        {
            g_checkpointRuntime[
                static_cast<size_t>(
                    g_checkpointActiveTask
                        .sourceRuntimeIndex)]
                .roll.taskProgress =
                std::clamp(
                    1.0f -
                        distance /
                            g_checkpointActiveTask
                                .baselineDistance,
                    0.0f,
                    1.0f);
        }
        return true;
    }

    source.roll.taskProgress =
        task.targetCount > 0
            ? std::clamp(
                  static_cast<float>(
                      source.roll.taskVisitedCount) /
                      static_cast<float>(task.targetCount),
                  0.0f,
                  1.0f)
            : 0.0f;
    return true;
}

bool CheckpointSystem_InteractWithActiveTaskPoint(
    int runtimeIndex)
{
    if (!g_checkpointActiveTask.active ||
        g_checkpointActiveTask.protectionRemaining > 0.0f ||
        !g_checkpointActiveTask.targetsReady ||
        runtimeIndex < 0 ||
        runtimeIndex >= static_cast<int>(g_checkpointRuntime.size()))
    {
        return false;
    }
    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(
                g_checkpointActiveTask.taskIndex)];
    for (size_t index = 0;
         index <
             g_checkpointActiveTask
                 .targetRuntimeIndices.size();
         ++index)
    {
        if (g_checkpointActiveTask
                .targetRuntimeIndices[index] !=
            runtimeIndex)
        {
            continue;
        }
        if (index <
                g_checkpointActiveTask
                    .targetVisited.size() &&
            g_checkpointActiveTask
                .targetVisited[index])
        {
            return true;
        }
        if (index <
            g_checkpointActiveTask
                .targetVisited.size())
        {
            g_checkpointActiveTask
                .targetVisited[index] = true;
        }
        const bool locationTask =
            task.kind == "pose_hold";
        if (locationTask)
        {
            if (index <
                g_checkpointActiveTask
                    .targetArmed.size())
            {
                g_checkpointActiveTask
                    .targetArmed[index] = true;
            }
            const auto& targetDefinition =
                g_checkpointDefinitions[
                    g_checkpointRuntime[
                        static_cast<size_t>(runtimeIndex)]
                        .definitionIndex];
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint task location confirmed with F: task=%S point=%s.",
                task.id.c_str(),
                Utf8ToWide(targetDefinition.id).c_str());
            CheckpointSystem_UpdateTaskTracker();
            return true;
        }
        auto& source = g_checkpointRuntime[
            static_cast<size_t>(
                g_checkpointActiveTask.sourceRuntimeIndex)];
        ++source.roll.taskVisitedCount;
        source.roll.taskProgress =
            task.targetCount > 0
                ? std::clamp(
                      static_cast<float>(
                          source.roll.taskVisitedCount) /
                          static_cast<float>(
                              task.targetCount),
                      0.0f,
                      1.0f)
                : 1.0f;
        auto& targetRuntime =
            g_checkpointRuntime[
                static_cast<size_t>(runtimeIndex)];
        const auto& targetDefinition =
            g_checkpointDefinitions[
                targetRuntime.definitionIndex];
        if (targetRuntime.visualRoot != nullptr)
        {
            SetWorldGameObjectActive(
                targetRuntime.visualRoot,
                false);
        }
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint task point inspected: task=%S point=%s count=%d/%d.",
            task.id.c_str(),
            Utf8ToWide(targetDefinition.id).c_str(),
            source.roll.taskVisitedCount,
            task.targetCount);
        if (task.kind == "reach" ||
            source.roll.taskVisitedCount >=
                task.targetCount)
        {
            return CheckpointSystem_CompleteActiveTask(
                task.kind == "reach"
                    ? "destination inspected"
                    : "all checkpoints inspected");
        }
        CheckpointSystem_UpdateTaskTracker();
        return true;
    }
    return false;
}

void CheckpointSystem_UpdateActiveTask(
    float playerX,
    float playerY,
    float playerZ)
{
    if (!g_checkpointActiveTask.active)
    {
        return;
    }
    const unsigned long long now = GetTickCount64();
    if (CheckpointSystem_TaskGameplayPaused())
    {
        g_checkpointActiveTask.lastTick = now;
        g_checkpointActiveTask.lastPlayerX = playerX;
        g_checkpointActiveTask.lastPlayerY = playerY;
        g_checkpointActiveTask.lastPlayerZ = playerZ;
        return;
    }
    const unsigned long long elapsed =
        g_checkpointActiveTask.lastTick == 0
            ? 0
            : now - g_checkpointActiveTask.lastTick;
    g_checkpointActiveTask.lastTick = now;
    const float previousPlayerX =
        g_checkpointActiveTask.lastPlayerX;
    const float previousPlayerZ =
        g_checkpointActiveTask.lastPlayerZ;
    g_checkpointActiveTask.lastPlayerX = playerX;
    g_checkpointActiveTask.lastPlayerY = playerY;
    g_checkpointActiveTask.lastPlayerZ = playerZ;

    bool protectionEndedThisFrame = false;
    if (g_checkpointActiveTask.protectionRemaining > 0.0f)
    {
        g_checkpointActiveTask.protectionRemaining -=
            static_cast<float>(elapsed) / 1000.0f;
        if (g_checkpointActiveTask.protectionRemaining > 0.0f)
        {
            g_checkpointActiveTask.previousEcstasyMotion =
                CheckpointSystem_ReadEcstasyMotion();
            return;
        }
        g_checkpointActiveTask.protectionRemaining = 0.0f;
        protectionEndedThisFrame = true;
        ExtendedMode_SetSpecialCoatMode(false);
        if (!CheckpointSystem_SampleTaskTargets(
                g_checkpointActiveTask,
                playerX,
                playerY,
                playerZ))
        {
            CheckpointSystem_FailActiveTask(
                "could not sample task targets");
            return;
        }
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint task protection ended; player switched to naked mode and task targets are armed.");
    }

    if (!g_checkpointActiveTask.targetsReady)
    {
        if (!CheckpointSystem_SampleTaskTargets(
                g_checkpointActiveTask,
                playerX,
                playerY,
                playerZ))
        {
            CheckpointSystem_FailActiveTask(
                "could not sample task targets");
            return;
        }
    }

    GameplayStatusHudMovementState movement{};
    if (!GameplayStatusHud_ReadMovementState(movement))
    {
        return;
    }
    if (g_checkpointActiveTask.taskIndex < 0 ||
        g_checkpointActiveTask.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return;
    }
    const auto& task =
        g_checkpointTaskPool[
            static_cast<size_t>(
                g_checkpointActiveTask.taskIndex)];
    if (!CheckpointSystem_TaskConditionValid(task, movement))
    {
        if (!g_checkpointActiveTask.conditionInvalid)
        {
            g_checkpointActiveTask.conditionInvalid = true;
            CheckpointSystem_HandleActiveTaskConditionFailure(
                task.gait == "walk"
                    ? "walking or naked condition lost"
                    : (task.gait == "crawl"
                        ? "crawling or naked condition lost"
                        : "naked condition lost"));
        }
        g_checkpointActiveTask.previousEcstasyMotion =
            CheckpointSystem_ReadEcstasyMotion();
        return;
    }
    g_checkpointActiveTask.conditionInvalid = false;
    if (task.kind == "distance" &&
        !protectionEndedThisFrame)
    {
        const float dx = playerX - previousPlayerX;
        const float dz = playerZ - previousPlayerZ;
        const float frameDistance =
            std::sqrt(dx * dx + dz * dz);
        if (frameDistance > 0.0001f &&
            frameDistance < 5.0f &&
            movement.moveSpeed > 0.02f)
        {
            g_checkpointActiveTask.accumulatedDistance +=
                frameDistance;
        }
    }
    const bool ecstasyMotion =
        CheckpointSystem_ReadEcstasyMotion();
    const bool ecstasyStarted =
        ecstasyMotion &&
        !g_checkpointActiveTask.previousEcstasyMotion;
    g_checkpointActiveTask.previousEcstasyMotion =
        ecstasyMotion;
    CheckpointSystem_UpdateActiveTaskTargets(
        static_cast<float>(elapsed) / 1000.0f,
        playerX,
        playerY,
        playerZ,
        ecstasyStarted);
}

void CheckpointSystem_RefreshFailureFallbackPanels()
{
    if (!ExtendedMode_IsUnityObjectAlive(
            g_checkpointFailureFallbackPanel))
    {
        g_checkpointFailureFallbackPanel = nullptr;
    }
    if (!ExtendedMode_IsUnityObjectAlive(
            g_checkpointSaveFallbackPanel))
    {
        g_checkpointSaveFallbackPanel = nullptr;
    }
    if (g_checkpointFailureFallbackPanel != nullptr &&
        g_checkpointSaveFallbackPanel != nullptr)
    {
        return;
    }
    const unsigned long long now = GetTickCount64();
    if (now < g_checkpointFailureFallbackNextScanAt)
    {
        return;
    }
    g_checkpointFailureFallbackNextScanAt = now + 5000;

    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (gameObjectClass == nullptr ||
        objectClass == nullptr ||
        il2cpp_type_get_object == nullptr)
    {
        return;
    }
    auto* managedType = il2cpp_type_get_object(
        il2cpp_class_get_type(gameObjectClass));
    bool includeInactive = true;
    void* parameters[] = { managedType, &includeInactive };
    auto* objects = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (objects == nullptr)
    {
        return;
    }
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(objects) +
        il2cpp_array_object_header_size());
    const uint32_t count = il2cpp_array_length(objects);
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* gameObject = elements[index];
        if (gameObject == nullptr)
        {
            continue;
        }
        const std::wstring name = GetObjectName(gameObject);
        if (g_checkpointFailureFallbackPanel == nullptr &&
            name.find(L"MissionFailedPanel") !=
                std::wstring::npos)
        {
            g_checkpointFailureFallbackPanel = gameObject;
        }
        if (g_checkpointSaveFallbackPanel == nullptr &&
            name.find(L"SaveDataSelectPanel") !=
                std::wstring::npos)
        {
            g_checkpointSaveFallbackPanel = gameObject;
        }
        if (g_checkpointFailureFallbackPanel != nullptr &&
            g_checkpointSaveFallbackPanel != nullptr)
        {
            break;
        }
    }
}

bool CheckpointSystem_IsActiveGameObject(
    Il2CppObject* gameObject)
{
    if (!ExtendedMode_IsUnityObjectAlive(gameObject))
    {
        return false;
    }
    static const Il2CppMethod* activeInHierarchy = nullptr;
    if (activeInHierarchy == nullptr)
    {
        activeInHierarchy = FindMethod(
            FindClass("UnityEngine", "GameObject"),
            "get_activeInHierarchy",
            0);
    }
    auto* value = activeInHierarchy != nullptr
        ? Invoke(activeInHierarchy, gameObject, nullptr)
        : nullptr;
    return value != nullptr &&
           *static_cast<bool*>(
               il2cpp_object_unbox(value));
}

bool CheckpointSystem_IsFailureUiActive()
{
    auto* assetReferencer = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "AssetReferencer",
        "Instance");
    if (assetReferencer != nullptr)
    {
        bool authoritativeReferenceFound = false;
        auto* missionFailedPanel =
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(assetReferencer) +
                0x118);
        authoritativeReferenceFound =
            missionFailedPanel != nullptr;
        if (CheckpointSystem_IsActiveGameObject(
                missionFailedPanel))
        {
            return true;
        }
        auto* saveDataSelectPanel =
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(assetReferencer) +
                0x120);
        if (saveDataSelectPanel != nullptr)
        {
            authoritativeReferenceFound = true;
            auto* panelObject =
                GetComponentGameObject(saveDataSelectPanel);
            if (CheckpointSystem_IsActiveGameObject(panelObject))
            {
                return true;
            }
        }
        if (authoritativeReferenceFound)
        {
            return false;
        }
    }

    CheckpointSystem_RefreshFailureFallbackPanels();
    return CheckpointSystem_IsActiveGameObject(
               g_checkpointFailureFallbackPanel) ||
           CheckpointSystem_IsActiveGameObject(
               g_checkpointSaveFallbackPanel);
}

void CheckpointSystem_MarkActiveTaskFailedForGameFailure()
{
    if (g_checkpointFailureTaskMarked ||
        !g_checkpointActiveTask.active ||
        g_checkpointActiveTask.sourceRuntimeIndex < 0 ||
        g_checkpointActiveTask.sourceRuntimeIndex >=
            static_cast<int>(g_checkpointRuntime.size()))
    {
        return;
    }
    const int sourceRuntimeIndex =
        g_checkpointActiveTask.sourceRuntimeIndex;
    auto& source = g_checkpointRuntime[
        static_cast<size_t>(sourceRuntimeIndex)];
    if (source.roll.taskIndex < 0 ||
        source.roll.taskIndex >=
            static_cast<int>(g_checkpointTaskPool.size()))
    {
        return;
    }
    const auto& task = g_checkpointTaskPool[
        static_cast<size_t>(source.roll.taskIndex)];
    source.roll.taskFailed = true;
    source.roll.taskCompleted = false;
    source.roll.rewardGranted = false;
    source.roll.taskProgress = 1.0f;
    g_checkpointTrackedRuntimeIndex = sourceRuntimeIndex;
    g_checkpointTrackerCompletedAt = 0;
    g_checkpointTrackerFailedAt = GetTickCount64();
    g_checkpointFailureTaskMarked = true;
    g_checkpointRequestedTaskCrouch = -1;
    g_checkpointActiveTask.active = false;
    g_checkpointActiveTask.protectionRemaining = 0.0f;
    g_checkpointActiveTask.targetRuntimeIndices.clear();
    g_checkpointActiveTask.targetVisited.clear();
    g_checkpointActiveTask.targetsReady = false;
    g_checkpointInteractionAvailable = false;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint task failed immediately because the original game entered its failure UI: task=%S.",
        task.id.c_str());
}

void CheckpointSystem_OnGameFailureUiOpened()
{
    g_checkpointFailureUiActive = true;
    g_checkpointInteractionAvailable = false;
    CheckpointSystem_MarkActiveTaskFailedForGameFailure();
}

bool CheckpointSystem_ShouldSuppressFailurePanelOpen()
{
    return g_checkpointFailureHandled;
}

void CheckpointSystem_HideFailureUi()
{
    auto* assetReferencer = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "AssetReferencer",
        "Instance");
    if (assetReferencer == nullptr)
    {
        return;
    }

    auto* missionFailedPanel =
        *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(assetReferencer) +
            0x118);
    if (CheckpointSystem_IsActiveGameObject(
            missionFailedPanel))
    {
        SetWorldGameObjectActive(missionFailedPanel, false);
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Escape from Manaka hid the original MissionFailedPanel without destroying it.");
    }

    auto* saveDataSelectPanel =
        *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(assetReferencer) +
            0x120);
    if (saveDataSelectPanel != nullptr)
    {
        auto* panelObject =
            GetComponentGameObject(saveDataSelectPanel);
        if (CheckpointSystem_IsActiveGameObject(panelObject))
        {
            SetWorldGameObjectActive(panelObject, false);
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Escape from Manaka hid the original SaveDataSelectPanel without destroying it.");
        }
    }
}

void CheckpointSystem_RequestFailureReturn(
    const wchar_t* source)
{
    if (g_checkpointFailureHandled)
    {
        return;
    }
    if (!ExtendedMode_IsRunning())
    {
        return;
    }
    g_checkpointFailureHandled = true;
    CheckpointSystem_HideFailureUi();
    g_checkpointFailureUiActive = false;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Escape failure UI requested the warehouse return path: source=%s.",
        source != nullptr ? source : L"unknown");
    ExtendedMode_RequestReturnToWarehouse();
}

void CheckpointSystem_CheckFailureExit()
{
    if (!ExtendedMode_IsRunning())
    {
        g_checkpointFailureUiActive = false;
        g_checkpointFailureTaskMarked = false;
        return;
    }
    static unsigned int failureCheckFrame = 0;
    if (++failureCheckFrame % 10 != 0)
    {
        return;
    }
    const bool failureUiActive =
        CheckpointSystem_IsFailureUiActive();
    g_checkpointFailureUiActive = failureUiActive;
    if (failureUiActive)
    {
        CheckpointSystem_OnGameFailureUiOpened();
    }
    else
    {
        g_checkpointFailureTaskMarked = false;
    }
}

void CheckpointSystem_Tick()
{
    CheckpointSystem_ProcessOverlayPointer();
    CheckpointSystem_ProcessPendingReward();
    CheckpointSystem_CheckFailureExit();
    if (g_checkpointFailureUiActive ||
        g_checkpointFailureHandled ||
        g_extendedSceneTransitionQueued.load() ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        g_checkpointMapEditorToggleRequested = false;
        g_checkpointMapEditorEditRequested = false;
        g_checkpointMapEditorActionRequested = -1;
        g_checkpointInteractionAvailable = false;
        ExtendedMode_SetCheckpointInteractionPrompt(false, "");
        return;
    }
    if (g_extendedPlayerGameObject == nullptr)
    {
        g_checkpointMapEditorToggleRequested = false;
        g_checkpointMapEditorEditRequested = false;
        g_checkpointMapEditorActionRequested = -1;
        g_checkpointInteractionAvailable = false;
        ExtendedMode_SetCheckpointInteractionPrompt(false, "");
        return;
    }
    auto* transform = GetWorldTransform(g_extendedPlayerGameObject);
    auto* positionValue = transform != nullptr
        ? Invoke(
              FindMethod(
                  FindClass("UnityEngine", "Transform"),
                  "get_position",
                  0),
              transform,
              nullptr)
        : nullptr;
    if (positionValue == nullptr)
    {
        return;
    }
    const auto* player = static_cast<float*>(
        il2cpp_object_unbox(positionValue));
    bool mapEditorOwnsPrompt = false;
    CheckpointSystem_UpdateMapEditor(player, mapEditorOwnsPrompt);
    if (g_checkpointRuntime.empty())
    {
        g_checkpointInteractionAvailable = false;
        if (!mapEditorOwnsPrompt)
        {
            ExtendedMode_SetCheckpointInteractionPrompt(false, "");
        }
        return;
    }
    CheckpointSystem_UpdateActiveTask(
        player[0],
        player[1],
        player[2]);
    ++g_checkpointVisualCheckFrame;
    if (!g_checkpointVisualCheckDone &&
        g_checkpointVisualCheckFrame >= 60)
    {
        g_checkpointVisualCheckDone = true;
        auto* rendererClass = FindClass("UnityEngine", "Renderer");
        auto* getVisible = FindMethod(
            rendererClass,
            "get_isVisible",
            0);
        auto* getEnabled = FindMethod(
            rendererClass,
            "get_enabled",
            0);
        auto* getRendererMaterial = FindMethod(
            rendererClass,
            "get_sharedMaterial",
            0);
        auto* objectClass = FindClass("UnityEngine", "Object");
        auto* getObjectName = objectClass != nullptr
            ? FindMethod(objectClass, "get_name", 0)
            : nullptr;
        for (const auto& runtime : g_checkpointRuntime)
        {
            auto* renderer = FindFirstComponentInChildrenByClass(
                runtime.visualRoot,
                "UnityEngine",
                "MeshRenderer");
            const auto& definition =
                g_checkpointDefinitions[runtime.definitionIndex];
            if (definition.deleted)
            {
                continue;
            }
            if (renderer == nullptr)
            {
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                    L"Checkpoint renderer check: id=%s no child MeshRenderer was found.",
                    Utf8ToWide(definition.id).c_str());
                continue;
            }
            auto* visibleValue = Invoke(
                getVisible,
                renderer,
                nullptr);
            auto* enabledValue = Invoke(
                getEnabled,
                renderer,
                nullptr);
            auto* rendererObject = GetComponentGameObject(renderer);
            auto* rendererTransform = GetWorldTransform(rendererObject);
            auto* positionValue = rendererTransform != nullptr
                ? Invoke(
                      FindMethod(
                          FindClass("UnityEngine", "Transform"),
                          "get_position",
                          0),
                      rendererTransform,
                      nullptr)
                : nullptr;
            const float* rendererPosition =
                positionValue != nullptr
                    ? static_cast<const float*>(
                          il2cpp_object_unbox(positionValue))
                    : nullptr;
            auto* material = Invoke(
                getRendererMaterial,
                renderer,
                nullptr);
            auto* materialNameValue =
                material != nullptr && getObjectName != nullptr
                    ? Invoke(getObjectName, material, nullptr)
                    : nullptr;
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint renderer check: id=%s visible=%d enabled=%d layer=%d position=(%.2f, %.2f, %.2f) material=%s.",
                Utf8ToWide(definition.id).c_str(),
                visibleValue != nullptr &&
                    *static_cast<bool*>(
                        il2cpp_object_unbox(visibleValue))
                    ? 1
                    : 0,
                enabledValue != nullptr &&
                    *static_cast<bool*>(
                        il2cpp_object_unbox(enabledValue))
                    ? 1
                    : 0,
                ExtendedMode_GetGameObjectLayer(rendererObject),
                rendererPosition != nullptr ? rendererPosition[0] : -99999.0f,
                rendererPosition != nullptr ? rendererPosition[1] : -99999.0f,
                rendererPosition != nullptr ? rendererPosition[2] : -99999.0f,
                materialNameValue != nullptr
                    ? ReadIl2CppString(
                          static_cast<Il2CppString*>(materialNameValue)).c_str()
                    : L"<unknown>");
        }
    }
    CheckpointRuntime* activeTaskRuntime = nullptr;
    CheckpointRuntime* nearbySupplyRuntime = nullptr;
    CheckpointRuntime* nearbyTransitionRuntime = nullptr;
    CheckpointRuntime* nearestInteractableRuntime = nullptr;
    float nearestInteractableDistanceSquared = 1.0e30f;
    for (auto& runtime : g_checkpointRuntime)
    {
        if (runtime.definitionIndex >= g_checkpointDefinitions.size())
        {
            continue;
        }
        const auto& definition =
            g_checkpointDefinitions[runtime.definitionIndex];
        if (definition.deleted)
        {
            if (runtime.visualRoot != nullptr)
            {
                SetWorldGameObjectActive(runtime.visualRoot, false);
            }
            runtime.playerInside = false;
            continue;
        }
        if (runtime.roll.taskIndex >= 0 &&
            runtime.roll.searchCompleted)
        {
            if (runtime.visualRoot != nullptr)
            {
                SetWorldGameObjectActive(runtime.visualRoot, false);
            }
            runtime.playerInside = false;
            continue;
        }
        const float dx = player[0] - definition.x;
        const float dy = player[1] - definition.y;
        const float dz = player[2] - definition.z;
        const bool inside =
            dx * dx + dz * dz <=
                definition.triggerRadius * definition.triggerRadius &&
            std::fabs(dy) <= definition.heightTolerance;
        if (!runtime.consumed &&
            inside &&
            !runtime.playerInside &&
            definition.triggerMode ==
                CheckpointTriggerMode::ProximityEnter)
        {
            CheckpointSystem_DispatchActions(definition);
            if (definition.once)
            {
                runtime.consumed = true;
            }
        }
        else if (!runtime.consumed &&
                 !inside &&
                 runtime.playerInside &&
                 definition.triggerMode ==
                     CheckpointTriggerMode::ProximityExit)
        {
            CheckpointSystem_DispatchActions(definition);
            if (definition.once)
            {
                runtime.consumed = true;
            }
        }
        runtime.playerInside = inside;
        if (inside &&
            CheckpointSystem_IsActionable(
                runtime,
                static_cast<int>(
                    &runtime -
                    g_checkpointRuntime.data())))
        {
            const float distanceSquared =
                dx * dx + dy * dy + dz * dz;
            if (distanceSquared <
                nearestInteractableDistanceSquared)
            {
                nearestInteractableDistanceSquared =
                    distanceSquared;
                nearestInteractableRuntime = &runtime;
            }
        }
        if (inside &&
            definition.kind == "supply")
        {
            if (nearbySupplyRuntime == nullptr)
            {
                nearbySupplyRuntime = &runtime;
            }
            if (runtime.roll.taskIndex >= 0 &&
                !runtime.roll.taskCompleted)
            {
                activeTaskRuntime = &runtime;
            }
        }
        else if (inside &&
                 (definition.kind == "extract" ||
                  definition.kind == "transfer") &&
                 nearbyTransitionRuntime == nullptr)
        {
            nearbyTransitionRuntime = &runtime;
        }
    }
    g_checkpointInteractionAvailable =
        nearestInteractableRuntime != nullptr;
    static bool loggedInteractionAvailable = false;
    const bool interactionAvailable =
        g_checkpointInteractionAvailable.load();
    if (interactionAvailable != loggedInteractionAvailable)
    {
        loggedInteractionAvailable = interactionAvailable;
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint interaction availability changed: nearby=%d.",
            interactionAvailable ? 1 : 0);
    }

    if (g_checkpointAbandonTaskRequested.exchange(false) &&
        CheckpointSystem_HasAcceptedTask())
    {
        CheckpointSystem_BeginAbandonTrackedTask(-1);
    }

    if (g_checkpointSearchRequested.exchange(false))
    {
        CheckpointRuntime* selected = nullptr;
        int selectedIndex = -1;
        float selectedDistanceSquared = 1.0e30f;
        for (auto& runtime : g_checkpointRuntime)
        {
            if (!runtime.playerInside ||
                runtime.definitionIndex >=
                    g_checkpointDefinitions.size() ||
                !CheckpointSystem_IsActionable(
                    runtime,
                    static_cast<int>(
                        &runtime -
                        g_checkpointRuntime.data())))
            {
                continue;
            }
            const auto& definition =
                g_checkpointDefinitions[
                    runtime.definitionIndex];
            const float dx = player[0] - definition.x;
            const float dy = player[1] - definition.y;
            const float dz = player[2] - definition.z;
            const float distanceSquared =
                dx * dx + dy * dy + dz * dz;
            if (distanceSquared <
                selectedDistanceSquared)
            {
                selectedDistanceSquared =
                    distanceSquared;
                selected = &runtime;
                selectedIndex =
                    static_cast<int>(
                        &runtime -
                        g_checkpointRuntime.data());
            }
        }
        if (selected == nullptr)
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint interaction pressed with no active action point.");
        }
        else if (g_checkpointDefinitions[
                     selected->definitionIndex].kind ==
                 "task_random")
        {
            CheckpointSystem_InteractWithActiveTaskPoint(
                selectedIndex);
        }
        else if (!selected->roll.active ||
                 selected->roll.rewards.empty() ||
                 selected->roll.taskIndex < 0)
        {
            const auto& definition =
                g_checkpointDefinitions[selected->definitionIndex];
            if (definition.kind == "extract")
            {
                if (selected->visualRoot != nullptr)
                {
                    SetWorldGameObjectActive(
                        selected->visualRoot,
                        false);
                }
                selected->consumed = true;
                ExtendedMode_RequestReturnToWarehouse();
                return;
            }
            if (definition.kind == "transfer")
            {
                const int targetStage = definition.metadata.value(
                    "targetStageType",
                    -1);
                g_checkpointPendingTransferSpawnId =
                    definition.metadata.value(
                        "targetCheckpointId",
                        std::string());
                if (selected->visualRoot != nullptr)
                {
                    SetWorldGameObjectActive(
                        selected->visualRoot,
                        false);
                }
                selected->consumed = true;
                if (!ExtendedMode_BeginTransferToStage(targetStage))
                {
                    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                        L"Checkpoint transfer failed for target stage %d.",
                        targetStage);
                }
                return;
            }
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint interaction pressed on an unsupported checkpoint.");
        }
        else if (selected->roll.taskCompleted)
        {
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint task is already complete.");
        }
        else
        {
            if (CheckpointSystem_HasAcceptedTask() &&
                selectedIndex != g_checkpointTrackedRuntimeIndex)
            {
                CheckpointSystem_ShowTaskReplaceConfirm(
                    selectedIndex);
            }
            else
            {
                CheckpointSystem_ShowTaskOffer(selectedIndex);
            }
        }
    }

    const bool promptAllowed =
        !mapEditorOwnsPrompt &&
        !CheckpointSystem_IsMapEditorModeActive() &&
        !g_extendedOriginalSystemMenuVisible.load() &&
        !g_extendedOriginalInteractionMenuActive.load() &&
        !g_extendedOriginalHandcuffsPanelActive.load() &&
        !g_gameplayInventoryOpen.load() &&
        !CheckpointSystem_IsTaskUiOpen() &&
        nearestInteractableRuntime != nullptr;
    char promptAction[160]{};
    if (promptAllowed)
    {
        const auto& definition =
            g_checkpointDefinitions[
                nearestInteractableRuntime->definitionIndex];
        if (definition.kind == "task_random")
        {
            std::snprintf(
                promptAction,
                sizeof(promptAction),
                "检查");
        }
        else if (definition.kind == "supply")
        {
            std::snprintf(
                promptAction,
                sizeof(promptAction),
                "搜索（%s，%d件）",
                CheckpointSystem_QualityDisplayName(
                    nearestInteractableRuntime->roll.supplyQuality),
                static_cast<int>(
                    nearestInteractableRuntime
                        ->roll.rewards.size()));
        }
        else if (definition.kind == "extract")
        {
            std::snprintf(
                promptAction,
                sizeof(promptAction),
                "撤离");
        }
        else
        {
            const int targetStage = definition.metadata.value(
                "targetStageType",
                -1);
            std::snprintf(
                promptAction,
                sizeof(promptAction),
                "转移（%s）",
                CheckpointSystem_StageDisplayName(targetStage));
        }
    }
    if (!CheckpointSystem_IsMapEditorModeActive())
    {
        ExtendedMode_SetCheckpointInteractionPrompt(
            promptAllowed,
            promptAction);
    }
    CheckpointSystem_UpdateTaskTracker();
}
