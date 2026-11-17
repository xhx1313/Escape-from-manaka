// In-game map point annotation and persistence for the Escape mode.

constexpr int kCheckpointMapEditorCycleType = 0;
constexpr int kCheckpointMapEditorChanceDown = 1;
constexpr int kCheckpointMapEditorChanceUp = 2;
constexpr int kCheckpointMapEditorPasteNote = 3;
constexpr int kCheckpointMapEditorClearNote = 4;
constexpr int kCheckpointMapEditorDelete = 5;
constexpr int kCheckpointMapEditorSave = 6;
constexpr int kCheckpointMapEditorClose = 7;
constexpr int kCheckpointMapEditorCycleNearby = 8;
constexpr int kCheckpointMapEditorCreateCandidate = -2;

void CheckpointSystem_ResetMapEditorPhysicalKeys();
bool CheckpointSystem_SaveMapEditorFiles();
void CheckpointSystem_UpdateMapEditorPanelText();

struct CheckpointMapEditorVisual
{
    size_t definitionIndex = 0;
    Il2CppObject* root = nullptr;
};

static std::atomic<bool> g_checkpointMapEditorModeActive{false};
static std::atomic<bool> g_checkpointMapEditorUiOpen{false};
static std::atomic<bool> g_checkpointMapEditorDirty{false};
static std::atomic<bool> g_checkpointMapEditorToggleKeyDown{false};
static std::atomic<bool> g_checkpointMapEditorEditKeyDown{false};
static std::atomic<bool> g_checkpointMapEditorCycleKeyDown{false};
static std::atomic<bool> g_checkpointMapEditorActionKeyDown[9]{};
static int g_checkpointMapEditorDraftIndex = -1;
static bool g_checkpointMapEditorDraftNew = false;
static std::string g_checkpointMapEditorDraftKind;
static int g_checkpointMapEditorDraftChance = 100;
static int g_checkpointMapEditorDraftSupplyCount = 1;
static int g_checkpointMapEditorDraftSupplyQuality = 0;
static std::string g_checkpointMapEditorDraftNote;
static float g_checkpointMapEditorDraftPosition[3]{};
static int g_checkpointMapEditorNearbyIndex = -1;
static std::vector<int> g_checkpointMapEditorCandidates;
static size_t g_checkpointMapEditorCandidateCursor = 0;
static float g_checkpointMapEditorLastPlayer[3]{};
static std::string g_checkpointMapEditorNotice;
static unsigned long long g_checkpointMapEditorNoticeUntil = 0;
static std::vector<bool> g_checkpointMapEditorSavedRuntimeState;
static std::vector<CheckpointMapEditorVisual>
    g_checkpointMapEditorVisuals;
static SceneUiElement g_checkpointMapEditorHudRoot{};
static Il2CppObject* g_checkpointMapEditorStatus = nullptr;
static SceneUiElement g_checkpointMapEditorPanel{};
static SceneUiElement g_checkpointMapEditorBackdrop{};
static Il2CppObject* g_checkpointMapEditorTitle = nullptr;
static Il2CppObject* g_checkpointMapEditorBody = nullptr;
static SceneUiElement g_checkpointMapEditorTypeButton{};
static SceneUiElement g_checkpointMapEditorChanceDownButton{};
static SceneUiElement g_checkpointMapEditorChanceUpButton{};
static SceneUiElement g_checkpointMapEditorChanceInputRoot{};
static Il2CppObject* g_checkpointMapEditorChanceInput = nullptr;
static SceneUiElement g_checkpointMapEditorChanceInputText{};
static SceneUiElement g_checkpointMapEditorSupplyCountLabel{};
static SceneUiElement g_checkpointMapEditorSupplyCountButton{};
static SceneUiElement g_checkpointMapEditorSupplyQualityLabel{};
static SceneUiElement g_checkpointMapEditorSupplyQualityButton{};
static SceneUiElement g_checkpointMapEditorSupplyCountOptions[3]{};
static SceneUiElement g_checkpointMapEditorSupplyQualityOptions[6]{};
static SceneUiElement g_checkpointMapEditorNoteInputRoot{};
static Il2CppObject* g_checkpointMapEditorNoteInput = nullptr;
static SceneUiElement g_checkpointMapEditorNoteInputText{};
static SceneUiElement g_checkpointMapEditorPasteButton{};
static SceneUiElement g_checkpointMapEditorClearButton{};
static SceneUiElement g_checkpointMapEditorDeleteButton{};
static SceneUiElement g_checkpointMapEditorSaveButton{};
static SceneUiElement g_checkpointMapEditorCancelButton{};
static SceneUiElement g_checkpointMapEditorTypeOptions[5]{};
static std::atomic<bool> g_checkpointMapEditorTypeDropdownOpen{false};
static std::atomic<bool> g_checkpointMapEditorSupplyCountDropdownOpen{false};
static std::atomic<bool> g_checkpointMapEditorSupplyQualityDropdownOpen{false};
static std::atomic<bool> g_checkpointMapEditorTextInputFocused{false};
static unsigned int g_checkpointMapEditorObservedReleaseSerial = 0;
static unsigned int g_checkpointMapEditorObservedPressSerial = 0;

constexpr float kCheckpointMapEditorPanelMinX = 0.290f;
constexpr float kCheckpointMapEditorPanelMinY = 0.245f;
constexpr float kCheckpointMapEditorPanelMaxX = 0.745f;
constexpr float kCheckpointMapEditorPanelMaxY = 0.775f;

struct CheckpointMapEditorHitRect
{
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
};

bool CheckpointSystem_MapEditorPointIn(
    float x,
    float y,
    const CheckpointMapEditorHitRect& rect)
{
    return x >= rect.minX && x <= rect.maxX &&
           y >= rect.minY && y <= rect.maxY;
}

CheckpointMapEditorHitRect CheckpointSystem_MapEditorChildRect(
    float minX,
    float minY,
    float maxX,
    float maxY)
{
    const float width =
        kCheckpointMapEditorPanelMaxX -
        kCheckpointMapEditorPanelMinX;
    const float height =
        kCheckpointMapEditorPanelMaxY -
        kCheckpointMapEditorPanelMinY;
    CheckpointMapEditorHitRect rect;
    rect.minX = kCheckpointMapEditorPanelMinX + minX * width;
    rect.minY = kCheckpointMapEditorPanelMinY + minY * height;
    rect.maxX = kCheckpointMapEditorPanelMinX + maxX * width;
    rect.maxY = kCheckpointMapEditorPanelMinY + maxY * height;
    return rect;
}

const char* CheckpointSystem_MapEditorKindDisplay(
    const std::string& kind)
{
    if (kind == "supply") return "物资点";
    if (kind == "extract") return "撤离点";
    if (kind == "transfer") return "转移点";
    if (kind == "spawn") return "出生点";
    if (kind == "task_random") return "任务随机点";
    return kind.c_str();
}

std::string CheckpointSystem_MapEditorStageKeyString()
{
    if (g_checkpointLoadedConfigRoot.contains("stage") &&
        g_checkpointLoadedConfigRoot["stage"].is_object())
    {
        const auto& stage = g_checkpointLoadedConfigRoot["stage"];
        if (stage.contains("key") && stage["key"].is_string())
        {
            return stage["key"].get<std::string>();
        }
    }
    const char* stageKey =
        CheckpointSystem_StageKey(g_checkpointLoadedStageType);
    return stageKey != nullptr ? stageKey : "unknown_stage";
}

std::string CheckpointSystem_MapEditorStageName()
{
    if (g_checkpointLoadedConfigRoot.contains("stage") &&
        g_checkpointLoadedConfigRoot["stage"].is_object())
    {
        const auto& stage = g_checkpointLoadedConfigRoot["stage"];
        if (stage.contains("displayName") &&
            stage["displayName"].is_string())
        {
            return stage["displayName"].get<std::string>();
        }
    }
    return CheckpointSystem_StageDisplayName(
        g_checkpointLoadedStageType);
}

void CheckpointSystem_SetMapEditorNotice(const char* text)
{
    g_checkpointMapEditorNotice =
        text != nullptr ? text : "";
    g_checkpointMapEditorNoticeUntil =
        GetTickCount64() + 3500;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Map point editor: %S", g_checkpointMapEditorNotice.c_str());
}

bool CheckpointSystem_GetGameObjectActive(Il2CppObject* gameObject)
{
    if (!ExtendedMode_IsUnityObjectAlive(gameObject))
    {
        return false;
    }
    auto* value = Invoke(
        FindMethod(
            FindClass("UnityEngine", "GameObject"),
            "get_activeSelf",
            0),
        gameObject,
        nullptr);
    return value != nullptr &&
           il2cpp_object_unbox != nullptr &&
           *static_cast<bool*>(il2cpp_object_unbox(value));
}

void CheckpointSystem_DestroyMapEditorVisual(Il2CppObject* root)
{
    if (!ExtendedMode_IsUnityObjectAlive(root))
    {
        return;
    }
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* destroy = objectClass != nullptr
        ? FindMethod(objectClass, "Destroy", 1)
        : nullptr;
    if (destroy != nullptr)
    {
        void* parameters[] = { root };
        Invoke(destroy, nullptr, parameters);
    }
}

float* CheckpointSystem_MapEditorVisualColor(
    const CheckpointDefinition& definition,
    float color[4])
{
    if (definition.kind == "spawn")
    {
        CheckpointSystem_ParseColor("#006B3CFF", color);
        return color;
    }
    if (definition.kind == "task_random")
    {
        CheckpointSystem_ParseColor("#FFE600FF", color);
        return color;
    }
    if (definition.kind == "supply")
    {
        const std::string quality =
            definition.metadata.value("quality", "random");
        const int parsed = CheckpointSystem_ParseQuality(quality);
        if (parsed >= 0)
        {
            std::memcpy(
                color,
                CheckpointSystem_QualityColor(parsed),
                sizeof(float) * 4);
            return color;
        }
    }
    std::memcpy(color, definition.color, sizeof(float) * 4);
    return color;
}

void CheckpointSystem_RebuildMapEditorVisuals()
{
    for (auto& visual : g_checkpointMapEditorVisuals)
    {
        CheckpointSystem_DestroyMapEditorVisual(visual.root);
    }
    g_checkpointMapEditorVisuals.clear();
    for (size_t index = 0;
         index < g_checkpointDefinitions.size();
         ++index)
    {
        const auto& definition = g_checkpointDefinitions[index];
        if (definition.deleted)
        {
            continue;
        }
        CheckpointDefinition visualDefinition = definition;
        visualDefinition.visible = true;
        CheckpointSystem_MapEditorVisualColor(
            definition,
            visualDefinition.color);
        auto* root =
            CheckpointSystem_CreateVisual(visualDefinition);
        if (root != nullptr)
        {
            g_checkpointMapEditorVisuals.push_back(
                CheckpointMapEditorVisual{ index, root });
        }
    }
}

struct CheckpointMapEditorInput
{
    SceneUiElement root{};
    Il2CppObject* input = nullptr;
    SceneUiElement text{};
};

CheckpointMapEditorInput CheckpointSystem_CreateMapEditorInput(
    const char* name,
    Il2CppObject* parent,
    const char* initialText,
    float minX,
    float minY,
    float maxX,
    float maxY,
    int lineType,
    int contentType,
    int characterLimit,
    float red,
    float green,
    float blue,
    float alpha)
{
    CheckpointMapEditorInput result;
    result.root.gameObject =
        CreateSceneGameObject(name, parent);
    if (result.root.gameObject == nullptr)
    {
        return result;
    }
    result.root.rect = GetSceneComponentByClass(
        result.root.gameObject,
        "UnityEngine",
        "RectTransform");
    result.root.image = AddSceneComponentToGameObject(
        result.root.gameObject,
        "UnityEngine.UI",
        "Image");
    SetSceneRect(
        result.root.rect,
        minX,
        minY,
        maxX,
        maxY);
    SetSceneGraphicColor(
        result.root.image,
        red,
        green,
        blue,
        alpha);
    bool raycastTarget = true;
    void* raycastParameters[] = { &raycastTarget };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "Graphic"),
            "set_raycastTarget",
            1),
        result.root.image,
        raycastParameters);

    result.text = CreateSceneTextElement(
        "Text",
        result.root.rect,
        initialText,
        0.035f,
        0.10f,
        0.965f,
        0.90f,
        15.0f,
        514,
        0.92f,
        0.94f,
        0.98f,
        1.0f);
    if (result.text.text != nullptr)
    {
        bool textRaycast = false;
        void* textRaycastParameters[] = { &textRaycast };
        Invoke(
            FindMethod(
                FindClass("UnityEngine.UI", "Graphic"),
                "set_raycastTarget",
                1),
            result.text.text,
            textRaycastParameters);
    }

    result.input = AddSceneComponentToGameObject(
        result.root.gameObject,
        "UnityEngine.UI",
        "InputField");
    if (result.input == nullptr || result.text.text == nullptr)
    {
        return result;
    }
    void* targetParameters[] = { result.root.image };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "Selectable"),
            "set_targetGraphic",
            1),
        result.input,
        targetParameters);
    void* textComponentParameters[] = { result.text.text };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "set_textComponent",
            1),
        result.input,
        textComponentParameters);
    void* lineTypeParameters[] = { &lineType };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "set_lineType",
            1),
        result.input,
        lineTypeParameters);
    void* contentTypeParameters[] = { &contentType };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "set_contentType",
            1),
        result.input,
        contentTypeParameters);
    void* characterLimitParameters[] = { &characterLimit };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "set_characterLimit",
            1),
        result.input,
        characterLimitParameters);
    auto* textValue = il2cpp_string_new(initialText);
    void* textParameters[] = { textValue };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "set_text",
            1),
        result.input,
        textParameters);
    return result;
}

std::string CheckpointSystem_ReadMapEditorInput(
    Il2CppObject* input)
{
    if (input == nullptr)
    {
        return {};
    }
    auto* value = Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "get_text",
            0),
        input,
        nullptr);
    return value != nullptr
        ? WideToUtf8(ReadIl2CppString(
              static_cast<Il2CppString*>(value)))
        : std::string();
}

void CheckpointSystem_SetMapEditorInputText(
    Il2CppObject* input,
    const std::string& text)
{
    if (input == nullptr)
    {
        return;
    }
    auto* value = il2cpp_string_new(text.c_str());
    void* parameters[] = { value };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "set_text",
            1),
        input,
        parameters);
}

bool CheckpointSystem_InputIsFocused(Il2CppObject* input)
{
    if (input == nullptr)
    {
        return false;
    }
    auto* value = Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "InputField"),
            "get_isFocused",
            0),
        input,
        nullptr);
    return value != nullptr &&
           il2cpp_object_unbox != nullptr &&
           *static_cast<bool*>(il2cpp_object_unbox(value));
}

bool CheckpointSystem_IsMapEditorTextInputFocused()
{
    return g_checkpointMapEditorUiOpen.load() &&
           g_checkpointMapEditorTextInputFocused.load();
}

void CheckpointSystem_UpdateMapEditorTextInputFocus()
{
    g_checkpointMapEditorTextInputFocused =
        g_checkpointMapEditorUiOpen.load() &&
        (CheckpointSystem_InputIsFocused(
             g_checkpointMapEditorChanceInput) ||
         CheckpointSystem_InputIsFocused(
             g_checkpointMapEditorNoteInput));
}

void CheckpointSystem_EnsureMapEditorHud()
{
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorHudRoot.gameObject) ||
        !ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    auto* canvasTransform =
        GetWorldTransform(g_extendedGameplayHudCanvasObject);
    g_checkpointMapEditorHudRoot = CreateScenePanel(
        "CheckpointMapEditorHud",
        canvasTransform,
        0.018f,
        0.835f,
        0.410f,
        0.890f,
        0.025f,
        0.035f,
        0.050f,
        0.88f);
    g_checkpointMapEditorStatus = CreateSceneTextElement(
        "CheckpointMapEditorStatus",
        g_checkpointMapEditorHudRoot.rect,
        "",
        0.035f,
        0.10f,
        0.965f,
        0.90f,
        15.0f,
        514,
        0.94f,
        0.96f,
        0.98f,
        1.0f).text;
    g_checkpointMapEditorBackdrop = CreateScenePanel(
        "CheckpointMapEditorBackdrop",
        canvasTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.32f);
    if (g_checkpointMapEditorBackdrop.image != nullptr)
    {
        bool raycast = true;
        void* raycastParameters[] = { &raycast };
        Invoke(
            FindMethod(
                FindClass("UnityEngine.UI", "Graphic"),
                "set_raycastTarget",
                1),
            g_checkpointMapEditorBackdrop.image,
            raycastParameters);
    }
    g_checkpointMapEditorPanel = CreateScenePanel(
        "CheckpointMapEditorPanel",
        canvasTransform,
        kCheckpointMapEditorPanelMinX,
        kCheckpointMapEditorPanelMinY,
        kCheckpointMapEditorPanelMaxX,
        kCheckpointMapEditorPanelMaxY,
        0.028f,
        0.039f,
        0.055f,
        0.985f);
    g_checkpointMapEditorTitle = CreateSceneTextElement(
        "CheckpointMapEditorTitle",
        g_checkpointMapEditorPanel.rect,
        "地图点编辑",
        0.055f,
        0.865f,
        0.945f,
        0.965f,
        23.0f,
        514,
        0.97f,
        0.98f,
        1.0f,
        1.0f).text;
    g_checkpointMapEditorBody = CreateSceneTextElement(
        "CheckpointMapEditorBody",
        g_checkpointMapEditorPanel.rect,
        "",
        0.060f,
        0.765f,
        0.940f,
        0.850f,
        15.0f,
        514,
        0.84f,
        0.88f,
        0.94f,
        1.0f).text;
    g_checkpointMapEditorTypeButton = CreateSceneButton(
        "CheckpointMapEditorType",
        g_checkpointMapEditorPanel.rect,
        "类型：物资点",
        0.070f,
        0.655f,
        0.930f,
        0.735f,
        0.095f,
        0.145f,
        0.205f,
        0.99f,
        17.0f,
        514,
        true);
    g_checkpointMapEditorChanceDownButton = CreateSceneButton(
        "CheckpointMapEditorChanceDown",
        g_checkpointMapEditorPanel.rect,
        "概率 -10%",
        0.070f,
        0.545f,
        0.310f,
        0.625f,
        0.120f,
        0.075f,
        0.075f,
        0.99f,
        16.0f,
        514,
        true);
    g_checkpointMapEditorChanceUpButton = CreateSceneButton(
        "CheckpointMapEditorChanceUp",
        g_checkpointMapEditorPanel.rect,
        "概率 +10%",
        0.690f,
        0.545f,
        0.930f,
        0.625f,
        0.070f,
        0.145f,
        0.095f,
        0.99f,
        16.0f,
        514,
        true);
    auto chanceInput = CheckpointSystem_CreateMapEditorInput(
        "CheckpointMapEditorChanceInput",
        g_checkpointMapEditorPanel.rect,
        "100",
        0.330f,
        0.545f,
        0.670f,
        0.625f,
        0,
        2,
        3,
        0.075f,
        0.085f,
        0.105f,
        0.99f);
    g_checkpointMapEditorChanceInputRoot = chanceInput.root;
    g_checkpointMapEditorChanceInput = chanceInput.input;
    g_checkpointMapEditorChanceInputText = chanceInput.text;
    g_checkpointMapEditorSupplyCountLabel = CreateSceneTextElement(
        "CheckpointMapEditorSupplyCountLabel",
        g_checkpointMapEditorPanel.rect,
        "物资数量（1-3）",
        0.070f,
        0.515f,
        0.480f,
        0.545f,
        14.0f,
        513,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    g_checkpointMapEditorSupplyCountButton = CreateSceneButton(
        "CheckpointMapEditorSupplyCount",
        g_checkpointMapEditorPanel.rect,
        "物资数量：1",
        0.070f,
        0.425f,
        0.480f,
        0.505f,
        0.075f,
        0.085f,
        0.105f,
        0.99f,
        15.0f,
        514,
        true);
    g_checkpointMapEditorSupplyQualityLabel = CreateSceneTextElement(
        "CheckpointMapEditorSupplyQualityLabel",
        g_checkpointMapEditorPanel.rect,
        "物资品质（0-5）",
        0.520f,
        0.515f,
        0.930f,
        0.545f,
        14.0f,
        513,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    g_checkpointMapEditorSupplyQualityButton = CreateSceneButton(
        "CheckpointMapEditorSupplyQuality",
        g_checkpointMapEditorPanel.rect,
        "物资品质：0 随机",
        0.520f,
        0.425f,
        0.930f,
        0.505f,
        0.075f,
        0.085f,
        0.105f,
        0.99f,
        15.0f,
        514,
        true);
    for (int index = 0; index < 3; ++index)
    {
        char optionLabel[24]{};
        std::snprintf(
            optionLabel,
            sizeof(optionLabel),
            "%d 件",
            index + 1);
        const float maxY = 0.410f - index * 0.060f;
        g_checkpointMapEditorSupplyCountOptions[index] =
            CreateSceneButton(
                "CheckpointMapEditorSupplyCountOption",
                g_checkpointMapEditorPanel.rect,
                optionLabel,
                0.070f,
                maxY - 0.050f,
                0.480f,
                maxY,
                0.070f,
                0.110f,
                0.175f,
                1.0f,
                15.0f,
                514,
                true);
        SetWorldGameObjectActive(
            g_checkpointMapEditorSupplyCountOptions[index].gameObject,
            false);
    }
    static const char* qualityLabels[6] = {
        "0 随机",
        "1 白色",
        "2 绿色",
        "3 紫色",
        "4 金色",
        "5 红色",
    };
    for (int index = 0; index < 6; ++index)
    {
        const float maxY = 0.410f - index * 0.060f;
        g_checkpointMapEditorSupplyQualityOptions[index] =
            CreateSceneButton(
                "CheckpointMapEditorSupplyQualityOption",
                g_checkpointMapEditorPanel.rect,
                qualityLabels[index],
                0.520f,
                maxY - 0.050f,
                0.930f,
                maxY,
                0.070f,
                0.110f,
                0.175f,
                1.0f,
                15.0f,
                514,
                true);
        SetWorldGameObjectActive(
            g_checkpointMapEditorSupplyQualityOptions[index].gameObject,
            false);
    }
    auto noteInput = CheckpointSystem_CreateMapEditorInput(
        "CheckpointMapEditorNoteInput",
        g_checkpointMapEditorPanel.rect,
        "",
        0.070f,
        0.285f,
        0.930f,
        0.385f,
        1,
        0,
        180,
        0.075f,
        0.085f,
        0.105f,
        0.99f);
    g_checkpointMapEditorNoteInputRoot = noteInput.root;
    g_checkpointMapEditorNoteInput = noteInput.input;
    g_checkpointMapEditorNoteInputText = noteInput.text;
    g_checkpointMapEditorPasteButton = CreateSceneButton(
        "CheckpointMapEditorPaste",
        g_checkpointMapEditorPanel.rect,
        "粘贴剪贴板",
        0.070f,
        0.205f,
        0.340f,
        0.270f,
        0.120f,
        0.145f,
        0.205f,
        0.99f,
        15.0f,
        514,
        true);
    g_checkpointMapEditorClearButton = CreateSceneButton(
        "CheckpointMapEditorClear",
        g_checkpointMapEditorPanel.rect,
        "清空备注",
        0.375f,
        0.205f,
        0.625f,
        0.270f,
        0.120f,
        0.125f,
        0.145f,
        0.99f,
        15.0f,
        514,
        true);
    g_checkpointMapEditorDeleteButton = CreateSceneButton(
        "CheckpointMapEditorDelete",
        g_checkpointMapEditorPanel.rect,
        "删除",
        0.070f,
        0.055f,
        0.285f,
        0.150f,
        0.210f,
        0.065f,
        0.070f,
        0.99f,
        16.0f,
        514,
        true);
    g_checkpointMapEditorSaveButton = CreateSceneButton(
        "CheckpointMapEditorSave",
        g_checkpointMapEditorPanel.rect,
        "保存并关闭",
        0.335f,
        0.055f,
        0.665f,
        0.150f,
        0.065f,
        0.205f,
        0.125f,
        0.99f,
        16.0f,
        514,
        true);
    g_checkpointMapEditorCancelButton = CreateSceneButton(
        "CheckpointMapEditorCancel",
        g_checkpointMapEditorPanel.rect,
        "取消",
        0.715f,
        0.055f,
        0.930f,
        0.150f,
        0.150f,
        0.120f,
        0.120f,
        0.99f,
        16.0f,
        514,
        true);
    static const char* typeLabels[5] = {
        "物资点",
        "撤离点",
        "转移点",
        "出生点",
        "任务随机点",
    };
    for (int index = 0; index < 5; ++index)
    {
        const float maxY = 0.635f - index * 0.058f;
        const float minY = maxY - 0.048f;
        g_checkpointMapEditorTypeOptions[index] =
            CreateSceneButton(
                "CheckpointMapEditorTypeOption",
                g_checkpointMapEditorPanel.rect,
                typeLabels[index],
                0.070f,
                minY,
                0.930f,
                maxY,
                0.070f,
                0.110f,
                0.175f,
                1.0f,
                15.0f,
                514,
                true);
        SetWorldGameObjectActive(
            g_checkpointMapEditorTypeOptions[index].gameObject,
            false);
    }
    SetWorldGameObjectActive(
        g_checkpointMapEditorHudRoot.gameObject,
        false);
    SetWorldGameObjectActive(
        g_checkpointMapEditorBackdrop.gameObject,
        false);
    SetWorldGameObjectActive(
        g_checkpointMapEditorPanel.gameObject,
        false);
}

void CheckpointSystem_CloseMapEditorPanel(bool cancelDraft)
{
    if (cancelDraft)
    {
        g_checkpointMapEditorDraftIndex = -1;
        g_checkpointMapEditorDraftNew = false;
    }
    g_checkpointMapEditorUiOpen = false;
    g_checkpointMapEditorTextInputFocused = false;
    g_checkpointMapEditorTypeDropdownOpen = false;
    g_checkpointMapEditorSupplyCountDropdownOpen = false;
    g_checkpointMapEditorSupplyQualityDropdownOpen = false;
    for (auto& option : g_checkpointMapEditorTypeOptions)
    {
        if (ExtendedMode_IsUnityObjectAlive(option.gameObject))
        {
            SetWorldGameObjectActive(option.gameObject, false);
        }
    }
    for (auto& option : g_checkpointMapEditorSupplyCountOptions)
    {
        if (ExtendedMode_IsUnityObjectAlive(option.gameObject))
        {
            SetWorldGameObjectActive(option.gameObject, false);
        }
    }
    for (auto& option : g_checkpointMapEditorSupplyQualityOptions)
    {
        if (ExtendedMode_IsUnityObjectAlive(option.gameObject))
        {
            SetWorldGameObjectActive(option.gameObject, false);
        }
    }
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorBackdrop.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorBackdrop.gameObject,
            false);
    }
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorPanel.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorPanel.gameObject,
            false);
    }
    ExtendedMode_SetCheckpointEditorPrompt(false, "");
    ExtendedMode_SetGameplayHudRaycastsEnabled(true);
    ClipCursor(nullptr);
    ExtendedMode_ForceGameplayCursor();
    GameplayInventory_CancelPointerInteraction();
}

std::string CheckpointSystem_ReadClipboardText()
{
    if (!OpenClipboard(FindGameWindow()))
    {
        return {};
    }
    std::wstring text;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data != nullptr)
    {
        const auto* characters =
            static_cast<const wchar_t*>(GlobalLock(data));
        if (characters != nullptr)
        {
            text.assign(characters);
            GlobalUnlock(data);
        }
    }
    CloseClipboard();
    std::string utf8 = WideToUtf8(text);
    std::replace(utf8.begin(), utf8.end(), '\r', ' ');
    std::replace(utf8.begin(), utf8.end(), '\n', ' ');
    if (utf8.size() > 180)
    {
        size_t boundary = 180;
        while (boundary > 0 &&
               (static_cast<unsigned char>(utf8[boundary]) & 0xC0) == 0x80)
        {
            --boundary;
        }
        utf8.resize(boundary);
    }
    return utf8;
}

void CheckpointSystem_UpdateMapEditorPanelText()
{
    if (!ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorPanel.gameObject) ||
        g_checkpointMapEditorTitle == nullptr ||
        g_checkpointMapEditorBody == nullptr)
    {
        return;
    }
    if (!g_checkpointMapEditorUiOpen)
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorPanel.gameObject,
            false);
        return;
    }

    const bool existing =
        !g_checkpointMapEditorDraftNew &&
        g_checkpointMapEditorDraftIndex >= 0 &&
        g_checkpointMapEditorDraftIndex <
            static_cast<int>(g_checkpointDefinitions.size());
    const CheckpointDefinition* definition = existing
        ? &g_checkpointDefinitions[
              static_cast<size_t>(
                  g_checkpointMapEditorDraftIndex)]
        : nullptr;
    char title[192]{};
    std::snprintf(
        title,
        sizeof(title),
        "%s %s",
        g_checkpointMapEditorDraftNew
            ? "新建地图点"
            : "编辑地图点",
        definition != nullptr
            ? definition->id.c_str()
            : "");
    SetSceneText(
        g_checkpointMapEditorTitle,
        title,
        23.0f,
        514,
        0.97f,
        0.98f,
        1.0f,
        1.0f);

    char notice[256]{};
    if (!g_checkpointMapEditorNotice.empty() &&
        GetTickCount64() < g_checkpointMapEditorNoticeUntil)
    {
        std::snprintf(
            notice,
            sizeof(notice),
            "\n提示：%s",
            g_checkpointMapEditorNotice.c_str());
    }
    char body[768]{};
    std::snprintf(
        body,
        sizeof(body),
        "位置：X %.2f   Y %.2f   Z %.2f\n"
        "类型：点击后从下拉框选择\n"
        "概率：点击文本框输入 0-100，或点击两侧 ±10\n"
        "物资数量/品质：点击按钮后从下拉框选择\n"
        "备注：点击文本框直接输入"
        "%s",
        g_checkpointMapEditorDraftPosition[0],
        g_checkpointMapEditorDraftPosition[1],
        g_checkpointMapEditorDraftPosition[2],
        notice);
    SetSceneText(
        g_checkpointMapEditorBody,
        body,
        16.0f,
        514,
        0.84f,
        0.88f,
        0.94f,
        1.0f);
    char label[192]{};
    std::snprintf(
        label,
        sizeof(label),
        "类型：%s",
        CheckpointSystem_MapEditorKindDisplay(
            g_checkpointMapEditorDraftKind));
    SetSceneText(
        g_checkpointMapEditorTypeButton.text,
        label,
        17.0f,
        514,
        0.94f,
        0.96f,
        0.98f,
        1.0f);
    std::snprintf(
        label,
        sizeof(label),
        "-10");
    SetSceneText(
        g_checkpointMapEditorChanceDownButton.text,
        label,
        16.0f,
        514,
        0.98f,
        0.90f,
        0.90f,
        1.0f);
    std::snprintf(
        label,
        sizeof(label),
        "+10");
    SetSceneText(
        g_checkpointMapEditorChanceUpButton.text,
        label,
        16.0f,
        514,
        0.88f,
        0.98f,
        0.90f,
        1.0f);
    std::snprintf(
        label,
        sizeof(label),
        "物资数量：%d",
        g_checkpointMapEditorDraftSupplyCount);
    SetSceneText(
        g_checkpointMapEditorSupplyCountButton.text,
        label,
        15.0f,
        514,
        0.94f,
        0.96f,
        0.98f,
        1.0f);
    const int configuredQuality = std::clamp(
        g_checkpointMapEditorDraftSupplyQuality,
        0,
        5);
    std::snprintf(
        label,
        sizeof(label),
        configuredQuality > 0
            ? "物资品质：%d %s"
            : "物资品质：0 随机",
        configuredQuality,
        configuredQuality > 0
            ? CheckpointSystem_QualityDisplayName(
                  configuredQuality - 1)
            : "");
    SetSceneText(
        g_checkpointMapEditorSupplyQualityButton.text,
        label,
        15.0f,
        514,
        0.94f,
        0.96f,
        0.98f,
        1.0f);
    const bool supplyKind =
        g_checkpointMapEditorDraftKind == "supply";
    const bool spawnKind =
        g_checkpointMapEditorDraftKind == "spawn";
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorChanceInputRoot.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorChanceInputRoot.gameObject,
            !spawnKind);
        SetWorldGameObjectActive(
            g_checkpointMapEditorChanceDownButton.gameObject,
            !spawnKind);
        SetWorldGameObjectActive(
            g_checkpointMapEditorChanceUpButton.gameObject,
            !spawnKind);
    }
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorSupplyCountButton.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorSupplyCountLabel.gameObject,
            supplyKind);
        SetWorldGameObjectActive(
            g_checkpointMapEditorSupplyCountButton.gameObject,
            supplyKind);
        SetWorldGameObjectActive(
            g_checkpointMapEditorSupplyQualityLabel.gameObject,
            supplyKind);
        SetWorldGameObjectActive(
            g_checkpointMapEditorSupplyQualityButton.gameObject,
            supplyKind);
        if (!supplyKind)
        {
            g_checkpointMapEditorSupplyCountDropdownOpen = false;
            g_checkpointMapEditorSupplyQualityDropdownOpen = false;
            for (auto& option :
                 g_checkpointMapEditorSupplyCountOptions)
            {
                SetWorldGameObjectActive(option.gameObject, false);
            }
            for (auto& option :
                 g_checkpointMapEditorSupplyQualityOptions)
            {
                SetWorldGameObjectActive(option.gameObject, false);
            }
        }
    }
    SetWorldGameObjectActive(
        g_checkpointMapEditorBackdrop.gameObject,
        true);
    SetWorldGameObjectActive(
        g_checkpointMapEditorPanel.gameObject,
        true);
    BringSceneObjectToFront(
        g_checkpointMapEditorBackdrop.gameObject);
    BringSceneObjectToFront(
        g_checkpointMapEditorPanel.gameObject);
}

void CheckpointSystem_RefreshMapEditorInputValues(
    bool chance,
    bool supply,
    bool note)
{
    if (chance)
    {
        CheckpointSystem_SetMapEditorInputText(
            g_checkpointMapEditorChanceInput,
            std::to_string(
                g_checkpointMapEditorDraftChance));
    }
    if (supply)
    {
        // Supply count and quality are selected from buttons, so their
        // displayed values are refreshed with the panel text.
    }
    if (note)
    {
        CheckpointSystem_SetMapEditorInputText(
            g_checkpointMapEditorNoteInput,
            g_checkpointMapEditorDraftNote);
    }
}

void CheckpointSystem_SetMapEditorTypeDropdownOpen(bool open)
{
    if (open)
    {
        g_checkpointMapEditorSupplyCountDropdownOpen = false;
        g_checkpointMapEditorSupplyQualityDropdownOpen = false;
        for (auto& option : g_checkpointMapEditorSupplyCountOptions)
        {
            SetWorldGameObjectActive(option.gameObject, false);
        }
        for (auto& option : g_checkpointMapEditorSupplyQualityOptions)
        {
            SetWorldGameObjectActive(option.gameObject, false);
        }
    }
    g_checkpointMapEditorTypeDropdownOpen = open;
    for (auto& option : g_checkpointMapEditorTypeOptions)
    {
        if (ExtendedMode_IsUnityObjectAlive(option.gameObject))
        {
            SetWorldGameObjectActive(option.gameObject, open);
        }
    }
    if (open)
    {
        for (auto& option : g_checkpointMapEditorTypeOptions)
        {
            if (ExtendedMode_IsUnityObjectAlive(option.gameObject))
            {
                BringSceneObjectToFront(option.gameObject);
            }
        }
    }
}

void CheckpointSystem_SetSupplyCountDropdownOpen(bool open)
{
    if (open)
    {
        CheckpointSystem_SetMapEditorTypeDropdownOpen(false);
        g_checkpointMapEditorSupplyQualityDropdownOpen = false;
        for (auto& option : g_checkpointMapEditorSupplyQualityOptions)
        {
            SetWorldGameObjectActive(option.gameObject, false);
        }
    }
    g_checkpointMapEditorSupplyCountDropdownOpen = open;
    for (auto& option : g_checkpointMapEditorSupplyCountOptions)
    {
        SetWorldGameObjectActive(option.gameObject, open);
        if (open)
        {
            BringSceneObjectToFront(option.gameObject);
        }
    }
}

void CheckpointSystem_SetSupplyQualityDropdownOpen(bool open)
{
    if (open)
    {
        CheckpointSystem_SetMapEditorTypeDropdownOpen(false);
        g_checkpointMapEditorSupplyCountDropdownOpen = false;
        for (auto& option : g_checkpointMapEditorSupplyCountOptions)
        {
            SetWorldGameObjectActive(option.gameObject, false);
        }
    }
    g_checkpointMapEditorSupplyQualityDropdownOpen = open;
    for (auto& option : g_checkpointMapEditorSupplyQualityOptions)
    {
        SetWorldGameObjectActive(option.gameObject, open);
        if (open)
        {
            BringSceneObjectToFront(option.gameObject);
        }
    }
}

void CheckpointSystem_SelectSupplyCount(int count)
{
    g_checkpointMapEditorDraftSupplyCount =
        std::clamp(count, 1, 3);
    LogInfo(
        LogCategory_Checkpoint,
        L"MapEditorSupplyCount",
        L"Map editor supply count selected: %d.",
        g_checkpointMapEditorDraftSupplyCount);
    CheckpointSystem_SetSupplyCountDropdownOpen(false);
    CheckpointSystem_UpdateMapEditorPanelText();
}

void CheckpointSystem_SelectSupplyQuality(int quality)
{
    g_checkpointMapEditorDraftSupplyQuality =
        std::clamp(quality, 0, 5);
    LogInfo(
        LogCategory_Checkpoint,
        L"MapEditorSupplyQuality",
        L"Map editor supply quality selected: %d.",
        g_checkpointMapEditorDraftSupplyQuality);
    CheckpointSystem_SetSupplyQualityDropdownOpen(false);
    CheckpointSystem_UpdateMapEditorPanelText();
}

void CheckpointSystem_SelectMapEditorType(int option)
{
    static const char* kinds[5] = {
        "supply",
        "extract",
        "transfer",
        "spawn",
        "task_random",
    };
    if (option < 0 || option >= 5)
    {
        return;
    }
    g_checkpointMapEditorDraftKind = kinds[option];
    LogInfo(
        LogCategory_Checkpoint,
        L"MapEditorType",
        L"Map editor point type selected: option=%d kind=%S.",
        option,
        g_checkpointMapEditorDraftKind.c_str());
    CheckpointSystem_SetMapEditorTypeDropdownOpen(false);
    CheckpointSystem_UpdateMapEditorPanelText();
}

void CheckpointSystem_OpenMapEditorPanel(int definitionIndex)
{
    if (!g_checkpointMapEditorModeActive ||
        definitionIndex < 0 ||
        definitionIndex >=
            static_cast<int>(g_checkpointDefinitions.size()))
    {
        return;
    }
    const auto& definition =
        g_checkpointDefinitions[static_cast<size_t>(definitionIndex)];
    g_checkpointMapEditorDraftIndex = definitionIndex;
    g_checkpointMapEditorDraftNew = false;
    g_checkpointMapEditorDraftKind = definition.kind;
    g_checkpointMapEditorDraftChance = std::clamp(
        static_cast<int>(std::round(
            definition.metadata.value(
                "spawnChance",
                1.0) * 100.0)),
        0,
        100);
    g_checkpointMapEditorDraftSupplyCount = std::clamp(
        definition.metadata.value("supplyCount", 1),
        1,
        3);
    g_checkpointMapEditorDraftSupplyQuality = 0;
    if (definition.metadata.contains("supplyQuality") &&
        definition.metadata["supplyQuality"].is_number_integer())
    {
        g_checkpointMapEditorDraftSupplyQuality =
            std::clamp(
                definition.metadata["supplyQuality"].get<int>(),
                0,
                5);
    }
    else if (definition.metadata.contains("quality") &&
             definition.metadata["quality"].is_string())
    {
        const int legacyQuality =
            CheckpointSystem_ParseQuality(
                definition.metadata["quality"].get<std::string>());
        if (legacyQuality >= 0)
        {
            g_checkpointMapEditorDraftSupplyQuality =
                legacyQuality + 1;
        }
    }
    g_checkpointMapEditorDraftNote =
        definition.metadata.value(
            "sourceNote",
            std::string());
    g_checkpointMapEditorDraftPosition[0] = definition.x;
    g_checkpointMapEditorDraftPosition[1] = definition.y;
    g_checkpointMapEditorDraftPosition[2] = definition.z;
    g_checkpointMapEditorUiOpen = true;
    g_checkpointMapEditorNotice.clear();
    g_checkpointMapEditorTypeDropdownOpen = false;
    g_checkpointMapEditorSupplyCountDropdownOpen = false;
    g_checkpointMapEditorSupplyQualityDropdownOpen = false;
    CheckpointSystem_EnsureMapEditorHud();
    CheckpointSystem_RefreshMapEditorInputValues(
        true,
        true,
        true);
    CheckpointSystem_UpdateMapEditorPanelText();
    ExtendedMode_SetCheckpointEditorPrompt(false, "");
    g_checkpointMapEditorObservedPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_checkpointMapEditorObservedReleaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    g_extendedManualPointerPositionValid = false;
    ExtendedMode_SetGameplayHudRaycastsEnabled(true);
    ClipCursor(nullptr);
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Map point editor opened: id=%s kind=%S.",
        Utf8ToWide(definition.id).c_str(),
        definition.kind.c_str());
}

void CheckpointSystem_CreateMapEditorPanel(
    const float player[3])
{
    g_checkpointMapEditorDraftIndex = -1;
    g_checkpointMapEditorDraftNew = true;
    g_checkpointMapEditorDraftKind = "supply";
    g_checkpointMapEditorDraftChance = 100;
    g_checkpointMapEditorDraftSupplyCount = 1;
    g_checkpointMapEditorDraftSupplyQuality = 0;
    g_checkpointMapEditorDraftNote.clear();
    std::memcpy(
        g_checkpointMapEditorDraftPosition,
        player,
        sizeof(g_checkpointMapEditorDraftPosition));
    g_checkpointMapEditorUiOpen = true;
    g_checkpointMapEditorNotice.clear();
    g_checkpointMapEditorTypeDropdownOpen = false;
    g_checkpointMapEditorSupplyCountDropdownOpen = false;
    g_checkpointMapEditorSupplyQualityDropdownOpen = false;
    CheckpointSystem_EnsureMapEditorHud();
    CheckpointSystem_RefreshMapEditorInputValues(
        true,
        true,
        true);
    CheckpointSystem_UpdateMapEditorPanelText();
    ExtendedMode_SetCheckpointEditorPrompt(false, "");
    g_checkpointMapEditorObservedPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_checkpointMapEditorObservedReleaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    g_extendedManualPointerPositionValid = false;
    ExtendedMode_SetGameplayHudRaycastsEnabled(true);
    ClipCursor(nullptr);
    ExtendedMode_ForceCursor();
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Map point editor opened for a new point.");
}

std::string CheckpointSystem_GenerateMapEditorId(
    const std::string& kind)
{
    const std::string prefix =
        CheckpointSystem_MapEditorStageKeyString() +
        "." + kind + ".map_editor.";
    for (int suffix = 1; suffix <= 9999; ++suffix)
    {
        char candidate[192]{};
        std::snprintf(
            candidate,
            sizeof(candidate),
            "%s%02d",
            prefix.c_str(),
            suffix);
        const bool used = std::any_of(
            g_checkpointDefinitions.begin(),
            g_checkpointDefinitions.end(),
            [&](const CheckpointDefinition& definition)
            {
                return !definition.deleted &&
                       definition.id == candidate;
            });
        if (!used)
        {
            return candidate;
        }
    }
    return prefix + "overflow";
}

bool CheckpointSystem_DeletingLastSpawn(
    int definitionIndex,
    const std::string& nextKind)
{
    if (definitionIndex < 0 ||
        definitionIndex >=
            static_cast<int>(g_checkpointDefinitions.size()))
    {
        return false;
    }
    const auto& current =
        g_checkpointDefinitions[static_cast<size_t>(definitionIndex)];
    if (current.kind != "spawn" || nextKind == "spawn")
    {
        return false;
    }
    const int remainingSpawns = static_cast<int>(std::count_if(
        g_checkpointDefinitions.begin(),
        g_checkpointDefinitions.end(),
        [&](const CheckpointDefinition& definition)
        {
            return !definition.deleted &&
                   definition.kind == "spawn" &&
                   definition.id != current.id;
        }));
    return remainingSpawns == 0;
}

void CheckpointSystem_ApplyMapEditorTypeDefaults(
    CheckpointDefinition& definition,
    const std::string& kind)
{
    if (definition.kind == kind)
    {
        return;
    }
    definition.kind = kind;
    definition.triggerMode = CheckpointTriggerMode::ManualInteract;
    definition.once = true;
    definition.actions.clear();
    definition.metadata.erase("targetStageType");
    definition.metadata.erase("targetCheckpointId");
    definition.metadata.erase("requiresTask");
    definition.metadata.erase("quality");
    if (kind == "supply")
    {
        definition.visible = true;
        CheckpointSystem_ParseColor("#FFFFFFFF", definition.color);
    }
    else if (kind == "extract")
    {
        definition.visible = true;
        CheckpointSystem_ParseColor("#38D6BDFF", definition.color);
    }
    else if (kind == "transfer")
    {
        definition.visible = true;
        CheckpointSystem_ParseColor("#38D6BDFF", definition.color);
        const int targetStage =
            g_checkpointLoadedStageType == 7 ? 11 : 7;
        definition.metadata["targetStageType"] = targetStage;
        definition.metadata["targetCheckpointId"] =
            targetStage == 11
                ? "shopping_mall.spawn.01"
                : "fashion_shop.spawn.01";
    }
    else if (kind == "spawn")
    {
        definition.visible = false;
        definition.triggerMode = CheckpointTriggerMode::None;
        CheckpointSystem_ParseColor("#006B3CFF", definition.color);
        CheckpointActionDefinition action;
        action.handler = "system.select_spawn";
        action.params = nlohmann::json::object();
        definition.actions.push_back(std::move(action));
    }
    else if (kind == "task_random")
    {
        definition.visible = true;
        definition.triggerMode = CheckpointTriggerMode::None;
        CheckpointSystem_ParseColor("#FFE600FF", definition.color);
    }
}

bool CheckpointSystem_CommitMapEditorDraft()
{
    const bool draftWasNew = g_checkpointMapEditorDraftNew;
    const std::string chanceText =
        CheckpointSystem_ReadMapEditorInput(
            g_checkpointMapEditorChanceInput);
    if (!chanceText.empty())
    {
        char* end = nullptr;
        const long parsed = std::strtol(
            chanceText.c_str(),
            &end,
            10);
        if (end != chanceText.c_str())
        {
            g_checkpointMapEditorDraftChance =
                static_cast<int>(
                    std::clamp<long>(parsed, 0, 100));
            CheckpointSystem_SetMapEditorInputText(
                g_checkpointMapEditorChanceInput,
                std::to_string(
                    g_checkpointMapEditorDraftChance));
        }
    }
    g_checkpointMapEditorDraftNote =
        CheckpointSystem_ReadMapEditorInput(
            g_checkpointMapEditorNoteInput);
    if (g_checkpointMapEditorDraftNote.size() > 180)
    {
        g_checkpointMapEditorDraftNote.resize(180);
    }
    const std::string kind = g_checkpointMapEditorDraftKind;
    if (kind != "supply" &&
        kind != "extract" &&
        kind != "transfer" &&
        kind != "spawn" &&
        kind != "task_random")
    {
        CheckpointSystem_SetMapEditorNotice("未知地图点类型");
        return false;
    }
    if (!g_checkpointMapEditorDraftNew &&
        CheckpointSystem_DeletingLastSpawn(
            g_checkpointMapEditorDraftIndex,
            kind))
    {
        CheckpointSystem_SetMapEditorNotice(
            "每张地图至少需要保留一个出生点");
        return false;
    }

    int index = g_checkpointMapEditorDraftIndex;
    if (g_checkpointMapEditorDraftNew)
    {
        CheckpointDefinition created;
        created.id =
            CheckpointSystem_GenerateMapEditorId(kind);
        created.kind = kind;
        created.stageType = g_checkpointLoadedStageType;
        created.x = g_checkpointMapEditorDraftPosition[0];
        created.y = g_checkpointMapEditorDraftPosition[1];
        created.z = g_checkpointMapEditorDraftPosition[2];
        const auto defaults = g_checkpointLoadedConfigRoot.value(
            "defaults",
            nlohmann::json::object());
        const auto defaultVisual = defaults.value(
            "visual",
            nlohmann::json::object());
        const auto defaultTrigger = defaults.value(
            "trigger",
            nlohmann::json::object());
        created.diameter = std::max(
            0.05f,
            defaultVisual.value("diameter", 0.8f));
        created.heightOffset = defaultVisual.value(
            "heightOffset",
            0.02f);
        created.triggerRadius =
            std::max(
                0.1f,
                defaultTrigger.value("radius", 1.1f));
        created.heightTolerance =
            std::max(
                0.5f,
                defaultTrigger.value(
                    "heightTolerance",
                    1.5f));
        created.once = defaultTrigger.value("once", true);
        created.metadata = nlohmann::json::object();
        created.sourceJson = nlohmann::json::object();
        created.kind = "visual";
        CheckpointSystem_ApplyMapEditorTypeDefaults(
            created,
            kind);
        g_checkpointDefinitions.push_back(std::move(created));
        index =
            static_cast<int>(g_checkpointDefinitions.size()) - 1;
    }
    if (index < 0 ||
        index >=
            static_cast<int>(g_checkpointDefinitions.size()))
    {
        return false;
    }

    auto& definition =
        g_checkpointDefinitions[static_cast<size_t>(index)];
    definition.x = std::round(definition.x * 100.0f) / 100.0f;
    definition.y = std::round(definition.y * 100.0f) / 100.0f;
    definition.z = std::round(definition.z * 100.0f) / 100.0f;
    CheckpointSystem_ApplyMapEditorTypeDefaults(
        definition,
        kind);
    definition.deleted = false;
    definition.metadata.erase("quality");
    if (kind == "supply")
    {
        definition.metadata["supplyCount"] =
            g_checkpointMapEditorDraftSupplyCount;
        definition.metadata["supplyQuality"] =
            g_checkpointMapEditorDraftSupplyQuality;
    }
    else
    {
        definition.metadata.erase("supplyCount");
        definition.metadata.erase("supplyQuality");
    }
    definition.metadata["spawnChance"] =
        static_cast<double>(
            g_checkpointMapEditorDraftChance) / 100.0;
    if (g_checkpointMapEditorDraftNote.empty())
    {
        definition.metadata.erase("sourceNote");
    }
    else
    {
        definition.metadata["sourceNote"] =
            g_checkpointMapEditorDraftNote;
    }
    if (!definition.sourceJson.is_object())
    {
        definition.sourceJson = nlohmann::json::object();
    }
    definition.sourceJson["id"] = definition.id;
    definition.sourceJson["kind"] = definition.kind;
    definition.sourceJson["position"] = {
        { "x", definition.x },
        { "y", definition.y },
        { "z", definition.z },
    };
    definition.sourceJson["metadata"] = definition.metadata;
    definition.sourceJson["visual"] =
        definition.sourceJson.value(
            "visual",
            nlohmann::json::object());
    definition.sourceJson["visual"]["visible"] =
        definition.visible;
    definition.sourceJson["visual"]["diameter"] =
        definition.diameter;
    definition.sourceJson["visual"]["heightOffset"] =
        definition.heightOffset;
    char colorText[10]{};
    std::snprintf(
        colorText,
        sizeof(colorText),
        "#%02X%02X%02X%02X",
        static_cast<int>(
            std::clamp(definition.color[0], 0.0f, 1.0f) * 255.0f),
        static_cast<int>(
            std::clamp(definition.color[1], 0.0f, 1.0f) * 255.0f),
        static_cast<int>(
            std::clamp(definition.color[2], 0.0f, 1.0f) * 255.0f),
        static_cast<int>(
            std::clamp(definition.color[3], 0.0f, 1.0f) * 255.0f));
    definition.sourceJson["visual"]["color"] = colorText;
    definition.sourceJson["trigger"] =
        definition.sourceJson.value(
            "trigger",
            nlohmann::json::object());
    definition.sourceJson["trigger"]["mode"] =
        definition.triggerMode == CheckpointTriggerMode::None
            ? "none"
            : "manual_interact";
    definition.sourceJson["trigger"]["radius"] =
        definition.triggerRadius;
    definition.sourceJson["trigger"]["heightTolerance"] =
        definition.heightTolerance;
    definition.sourceJson["trigger"]["once"] =
        definition.once;
    definition.sourceJson["actions"] =
        nlohmann::json::array();
    for (const auto& action : definition.actions)
    {
        definition.sourceJson["actions"].push_back({
            { "handler", action.handler },
            { "params", action.params },
        });
    }

    g_checkpointMapEditorDraftIndex = index;
    g_checkpointMapEditorDraftNew = false;
    g_checkpointMapEditorDirty = true;
    CheckpointSystem_RebuildMapEditorVisuals();
    CheckpointSystem_SetMapEditorNotice(
        "修改已提交");
    LogInfo(
        LogCategory_Checkpoint,
        L"MapEditorCommit",
        L"Map editor draft committed: index=%d new=%d kind=%S id=%S chance=%d count=%d quality=%d.",
        index,
        draftWasNew ? 1 : 0,
        definition.kind.c_str(),
        definition.id.c_str(),
        g_checkpointMapEditorDraftChance,
        g_checkpointMapEditorDraftSupplyCount,
        g_checkpointMapEditorDraftSupplyQuality);
    return true;
}

void CheckpointSystem_DeleteMapEditorDraft()
{
    if (g_checkpointMapEditorDraftNew)
    {
        CheckpointSystem_CloseMapEditorPanel(true);
        return;
    }
    if (g_checkpointMapEditorDraftIndex < 0 ||
        g_checkpointMapEditorDraftIndex >=
            static_cast<int>(g_checkpointDefinitions.size()))
    {
        return;
    }
    auto& definition =
        g_checkpointDefinitions[
            static_cast<size_t>(
                g_checkpointMapEditorDraftIndex)];
    if (definition.kind == "spawn" &&
        CheckpointSystem_DeletingLastSpawn(
            g_checkpointMapEditorDraftIndex,
            "supply"))
    {
        CheckpointSystem_SetMapEditorNotice(
            "每张地图至少需要保留一个出生点");
        return;
    }
    LogInfo(
        LogCategory_Checkpoint,
        L"MapEditorDelete",
        L"Map editor draft deleted: index=%d id=%S kind=%S.",
        g_checkpointMapEditorDraftIndex,
        definition.id.c_str(),
        definition.kind.c_str());
    definition.deleted = true;
    g_checkpointMapEditorDirty = true;
    if (!CheckpointSystem_SaveMapEditorFiles())
    {
        definition.deleted = false;
        CheckpointSystem_RebuildMapEditorVisuals();
        return;
    }
    CheckpointSystem_CloseMapEditorPanel(true);
    CheckpointSystem_RebuildMapEditorVisuals();
    CheckpointSystem_SetMapEditorNotice(
        "地图点已删除并保存");
}

std::wstring CheckpointSystem_TablePathForJson(
    const std::wstring& jsonPath)
{
    std::wstring path = jsonPath;
    const size_t extension = path.find_last_of(L'.');
    if (extension != std::wstring::npos)
    {
        path.resize(extension);
    }
    return path + L".map_points.csv";
}

bool CheckpointSystem_EnsureParentDirectory(
    const std::wstring& path)
{
    const size_t separator = path.find_last_of(L"\\/");
    if (separator == std::wstring::npos)
    {
        return true;
    }
    const std::wstring directory = path.substr(0, separator);
    if (directory.empty())
    {
        return false;
    }
    std::wstring current;
    size_t offset = 0;
    if (directory.size() >= 2 &&
        directory[1] == L':')
    {
        current = directory.substr(0, 2);
        offset = 2;
    }
    while (offset <= directory.size())
    {
        const size_t next = directory.find_first_of(
            L"\\/",
            offset);
        const size_t end = next == std::wstring::npos
            ? directory.size()
            : next;
        if (end > offset)
        {
            if (!current.empty() &&
                current.back() != L'\\' &&
                current.back() != L'/')
            {
                current += L'\\';
            }
            current += directory.substr(
                offset,
                end - offset);
            CreateDirectoryW(current.c_str(), nullptr);
        }
        if (next == std::wstring::npos)
        {
            break;
        }
        offset = next + 1;
    }
    const DWORD attributes =
        GetFileAttributesW(directory.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool CheckpointSystem_WriteUtf8FileAtomic(
    const std::wstring& path,
    const std::string& content)
{
    const std::wstring temporary = path + L".tmp";
    FILE* file = nullptr;
    if (_wfopen_s(&file, temporary.c_str(), L"wb") != 0 ||
        file == nullptr)
    {
        return false;
    }
    const size_t written = std::fwrite(
        content.data(),
        1,
        content.size(),
        file);
    const bool failed =
        written != content.size() ||
        std::ferror(file) != 0;
    std::fclose(file);
    if (failed ||
        !MoveFileExW(
            temporary.c_str(),
            path.c_str(),
            MOVEFILE_REPLACE_EXISTING |
                MOVEFILE_WRITE_THROUGH))
    {
        DeleteFileW(temporary.c_str());
        return false;
    }
    return true;
}

std::string CheckpointSystem_CsvField(
    const std::string& value)
{
    const bool quote =
        value.find_first_of(",\"\r\n") != std::string::npos;
    if (!quote)
    {
        return value;
    }
    std::string escaped = "\"";
    for (const char character : value)
    {
        if (character == '"')
        {
            escaped += "\"\"";
        }
        else
        {
            escaped += character;
        }
    }
    escaped += '"';
    return escaped;
}

bool CheckpointSystem_SaveMapEditorFiles()
{
    nlohmann::json root =
        g_checkpointLoadedConfigRoot.is_object()
            ? g_checkpointLoadedConfigRoot
            : nlohmann::json::object();
    root["schemaVersion"] = 1;
    root["checkpoints"] = nlohmann::json::array();
    for (const auto& definition : g_checkpointDefinitions)
    {
        if (definition.deleted)
        {
            continue;
        }
        nlohmann::json checkpoint =
            definition.sourceJson.is_object()
                ? definition.sourceJson
                : nlohmann::json::object();
        checkpoint["id"] = definition.id;
        checkpoint["kind"] = definition.kind;
        const double roundedX =
            std::round(
                static_cast<double>(definition.x) *
                100.0) / 100.0;
        const double roundedY =
            std::round(
                static_cast<double>(definition.y) *
                100.0) / 100.0;
        const double roundedZ =
            std::round(
                static_cast<double>(definition.z) *
                100.0) / 100.0;
        checkpoint["position"] = {
            { "x", roundedX },
            { "y", roundedY },
            { "z", roundedZ },
        };
        checkpoint["metadata"] = definition.metadata;
        root["checkpoints"].push_back(std::move(checkpoint));
    }
    std::string csv =
        "\xEF\xBB\xBF地图,x,y,z,类型,概率,物资数量,物资品质,备注\r\n";
    const std::string stageName =
        CheckpointSystem_MapEditorStageName();
    for (const auto& definition : g_checkpointDefinitions)
    {
        if (definition.deleted)
        {
            continue;
        }
        char numbers[160]{};
        std::snprintf(
            numbers,
            sizeof(numbers),
            "%.2f,%.2f,%.2f",
            definition.x,
            definition.y,
            definition.z);
        const double chance =
            definition.metadata.value("spawnChance", 1.0);
        char chanceText[16]{};
        if (definition.kind != "spawn")
        {
            std::snprintf(
                chanceText,
                sizeof(chanceText),
                "%.0f",
                std::clamp(chance, 0.0, 1.0) * 100.0);
        }
        char supplyCountText[16]{};
        char supplyQualityText[16]{};
        if (definition.kind == "supply")
        {
            std::snprintf(
                supplyCountText,
                sizeof(supplyCountText),
                "%d",
                CheckpointSystem_GetSupplyCount(
                    definition.metadata));
            std::snprintf(
                supplyQualityText,
                sizeof(supplyQualityText),
                "%d",
                std::clamp(
                    definition.metadata.value(
                        "supplyQuality",
                        0),
                    0,
                    5));
        }
        csv += CheckpointSystem_CsvField(stageName);
        csv += ',';
        csv += numbers;
        csv += ',';
        csv += CheckpointSystem_MapEditorKindDisplay(
            definition.kind);
        csv += ',';
        csv += chanceText;
        csv += ',';
        csv += supplyCountText;
        csv += ',';
        csv += supplyQualityText;
        csv += ',';
        csv += CheckpointSystem_CsvField(
            definition.metadata.value(
                "sourceNote",
                std::string()));
        csv += "\r\n";
    }

    const std::wstring userJsonPath =
        CheckpointSystem_GetUserConfigPath(
            g_checkpointLoadedStageType);
    const std::wstring userTablePath =
        CheckpointSystem_TablePathForJson(
            userJsonPath);
    if (userJsonPath.empty() ||
        !CheckpointSystem_EnsureParentDirectory(
            userJsonPath) ||
        !CheckpointSystem_WriteUtf8FileAtomic(
            userJsonPath,
            root.dump(2) + "\n") ||
        !CheckpointSystem_WriteUtf8FileAtomic(
            userTablePath,
            csv))
    {
        LogError(
            LogCategory_Persistence,
            L"MapEditorSave",
            L"Map point user files could not be written: json=%s table=%s.",
            userJsonPath.c_str(),
            userTablePath.c_str());
        CheckpointSystem_SetMapEditorNotice(
            "用户地图点文件写入失败，标注模式保持开启");
        return false;
    }

    const std::wstring shippedJsonPath =
        CheckpointSystem_GetShippedConfigPath(
            g_checkpointLoadedStageType);
    const std::wstring shippedTablePath =
        CheckpointSystem_TablePathForJson(
            shippedJsonPath);
    const bool shippedJsonSaved =
        !shippedJsonPath.empty() &&
        CheckpointSystem_EnsureParentDirectory(
            shippedJsonPath) &&
        CheckpointSystem_WriteUtf8FileAtomic(
            shippedJsonPath,
            root.dump(2) + "\n");
    const bool shippedTableSaved =
        !shippedTablePath.empty() &&
        CheckpointSystem_WriteUtf8FileAtomic(
            shippedTablePath,
            csv);

    g_checkpointLoadedConfigRoot = root;
    g_checkpointMapEditorDirty = false;
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Map point user files saved: json=%s table=%s.",
        userJsonPath.c_str(),
        userTablePath.c_str());
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Map point shipped mirrors saved: json=%d table=%d.",
        shippedJsonSaved ? 1 : 0,
        shippedTableSaved ? 1 : 0);
    return true;
}

void CheckpointSystem_ExitMapEditorMode()
{
    if (g_checkpointMapEditorUiOpen &&
        !CheckpointSystem_CommitMapEditorDraft())
    {
        return;
    }
    if (!CheckpointSystem_SaveMapEditorFiles())
    {
        return;
    }
    CheckpointSystem_CloseMapEditorPanel(true);
    for (auto& visual : g_checkpointMapEditorVisuals)
    {
        CheckpointSystem_DestroyMapEditorVisual(visual.root);
    }
    g_checkpointMapEditorVisuals.clear();
    for (size_t index = 0;
         index < g_checkpointRuntime.size() &&
         index < g_checkpointMapEditorSavedRuntimeState.size();
         ++index)
    {
        SetWorldGameObjectActive(
            g_checkpointRuntime[index].visualRoot,
            g_checkpointMapEditorSavedRuntimeState[index]);
    }
    g_checkpointMapEditorSavedRuntimeState.clear();
    g_checkpointMapEditorModeActive = false;
    GameplayInput_RestoreMapEditorEyeMaskInputAction();
    g_checkpointMapEditorNearbyIndex = -1;
    g_checkpointMapEditorCandidates.clear();
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorHudRoot.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorHudRoot.gameObject,
            false);
    }
    ExtendedMode_SetCheckpointEditorPrompt(false, "");
    GameplayInventory_CancelPointerInteraction();
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Map point editor mode exited and files were synchronized.");
}

bool CheckpointSystem_TryEnterMapEditorMode()
{
    const int currentStageType =
        ExtendedMode_GetCurrentStageType();
    if (CheckpointSystem_StageKey(currentStageType) == nullptr ||
        g_checkpointLoadedStageType != currentStageType)
    {
        CheckpointSystem_SetMapEditorNotice(
            "当前场景不支持地图点标注");
        return false;
    }
    if (CheckpointSystem_HasAcceptedTask() ||
        g_checkpointActiveTask.active ||
        g_checkpointFailureUiActive.load() ||
        g_gameplayInventoryOpen.load() ||
        g_extendedOriginalSystemMenuVisible.load() ||
        g_extendedOriginalInteractionMenuActive.load() ||
        g_extendedOriginalHandcuffsPanelActive.load() ||
        CheckpointSystem_IsTaskUiOpen())
    {
        CheckpointSystem_SetMapEditorNotice(
            "请先关闭菜单或放弃当前任务");
        return false;
    }
    g_checkpointMapEditorSavedRuntimeState.clear();
    g_checkpointMapEditorSavedRuntimeState.reserve(
        g_checkpointRuntime.size());
    for (auto& runtime : g_checkpointRuntime)
    {
        g_checkpointMapEditorSavedRuntimeState.push_back(
            CheckpointSystem_GetGameObjectActive(
                runtime.visualRoot));
        if (runtime.visualRoot != nullptr)
        {
            SetWorldGameObjectActive(
                runtime.visualRoot,
                false);
        }
    }
    g_checkpointMapEditorModeActive = true;
    g_checkpointMapEditorNearbyIndex = -1;
    CheckpointSystem_EnsureMapEditorHud();
    CheckpointSystem_RebuildMapEditorVisuals();
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorHudRoot.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorHudRoot.gameObject,
            true);
        BringSceneObjectToFront(
            g_checkpointMapEditorHudRoot.gameObject);
    }
    CheckpointSystem_SetMapEditorNotice(
        "已进入地图点标注模式");
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Map point editor mode entered.");
    return true;
}

void CheckpointSystem_ResetMapEditor()
{
    const bool hadOpenPanel =
        g_checkpointMapEditorUiOpen.load();
    if (hadOpenPanel &&
        ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorBackdrop.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorBackdrop.gameObject,
            false);
    }
    g_checkpointMapEditorModeActive = false;
    g_checkpointMapEditorUiOpen = false;
    GameplayInput_RestoreMapEditorEyeMaskInputAction();
    g_checkpointMapEditorTextInputFocused = false;
    g_checkpointMapEditorDirty = false;
    g_checkpointMapEditorDraftIndex = -1;
    g_checkpointMapEditorDraftNew = false;
    g_checkpointMapEditorNearbyIndex = -1;
    g_checkpointMapEditorCandidates.clear();
    g_checkpointMapEditorCandidateCursor = 0;
    g_checkpointMapEditorSavedRuntimeState.clear();
    for (auto& visual : g_checkpointMapEditorVisuals)
    {
        CheckpointSystem_DestroyMapEditorVisual(visual.root);
    }
    g_checkpointMapEditorVisuals.clear();
    g_checkpointMapEditorHudRoot = SceneUiElement{};
    g_checkpointMapEditorStatus = nullptr;
    g_checkpointMapEditorBackdrop = SceneUiElement{};
    g_checkpointMapEditorPanel = SceneUiElement{};
    g_checkpointMapEditorTitle = nullptr;
    g_checkpointMapEditorBody = nullptr;
    g_checkpointMapEditorTypeButton = SceneUiElement{};
    g_checkpointMapEditorChanceDownButton = SceneUiElement{};
    g_checkpointMapEditorChanceUpButton = SceneUiElement{};
    g_checkpointMapEditorChanceInputRoot = SceneUiElement{};
    g_checkpointMapEditorChanceInput = nullptr;
    g_checkpointMapEditorChanceInputText = SceneUiElement{};
    g_checkpointMapEditorSupplyCountLabel = SceneUiElement{};
    g_checkpointMapEditorSupplyCountButton = SceneUiElement{};
    g_checkpointMapEditorSupplyQualityLabel = SceneUiElement{};
    g_checkpointMapEditorSupplyQualityButton = SceneUiElement{};
    for (auto& option : g_checkpointMapEditorSupplyCountOptions)
    {
        option = SceneUiElement{};
    }
    for (auto& option : g_checkpointMapEditorSupplyQualityOptions)
    {
        option = SceneUiElement{};
    }
    g_checkpointMapEditorNoteInputRoot = SceneUiElement{};
    g_checkpointMapEditorNoteInput = nullptr;
    g_checkpointMapEditorNoteInputText = SceneUiElement{};
    g_checkpointMapEditorPasteButton = SceneUiElement{};
    g_checkpointMapEditorClearButton = SceneUiElement{};
    g_checkpointMapEditorDeleteButton = SceneUiElement{};
    g_checkpointMapEditorSaveButton = SceneUiElement{};
    g_checkpointMapEditorCancelButton = SceneUiElement{};
    for (auto& option : g_checkpointMapEditorTypeOptions)
    {
        option = SceneUiElement{};
    }
    g_checkpointMapEditorTypeDropdownOpen = false;
    g_checkpointMapEditorSupplyCountDropdownOpen = false;
    g_checkpointMapEditorSupplyQualityDropdownOpen = false;
    CheckpointSystem_ResetMapEditorPhysicalKeys();
    if (hadOpenPanel)
    {
        ExtendedMode_SetGameplayHudRaycastsEnabled(true);
        ClipCursor(nullptr);
        ExtendedMode_ForceGameplayCursor();
    }
}

void CheckpointSystem_RequestMapEditorToggle()
{
    g_checkpointMapEditorToggleRequested = true;
}

bool CheckpointSystem_IsMapEditorModeActive()
{
    return g_checkpointMapEditorModeActive;
}

bool CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey()
{
    return g_checkpointMapEditorModeActive.load() ||
           g_checkpointMapEditorToggleKeyDown.load() ||
           g_checkpointMapEditorToggleRequested.load();
}

bool CheckpointSystem_IsMapEditorUiOpen()
{
    return g_checkpointMapEditorUiOpen.load();
}

int CheckpointSystem_MapEditorActionForKey(
    int virtualKey,
    bool uiOpen)
{
    if (virtualKey >= 'a' && virtualKey <= 'z')
    {
        virtualKey -= 'a' - 'A';
    }
    if (!uiOpen)
    {
        return -1;
    }
    switch (virtualKey)
    {
    case 'T': return kCheckpointMapEditorCycleType;
    case VK_OEM_MINUS:
    case VK_SUBTRACT:
        return kCheckpointMapEditorChanceDown;
    case VK_OEM_PLUS:
    case VK_ADD:
        return kCheckpointMapEditorChanceUp;
    case 'V': return kCheckpointMapEditorPasteNote;
    case 'X': return kCheckpointMapEditorClearNote;
    case VK_DELETE: return kCheckpointMapEditorDelete;
    case VK_RETURN: return kCheckpointMapEditorSave;
    case 'B': return kCheckpointMapEditorClose;
    default: return -1;
    }
}

void CheckpointSystem_ResetMapEditorPhysicalKeys()
{
    g_checkpointMapEditorToggleKeyDown = false;
    g_checkpointMapEditorEditKeyDown = false;
    g_checkpointMapEditorCycleKeyDown = false;
    for (auto& keyDown : g_checkpointMapEditorActionKeyDown)
    {
        keyDown = false;
    }
}

bool CheckpointSystem_ProcessMapEditorPhysicalKey(
    int virtualKey,
    bool eventDown,
    const wchar_t* source)
{
    if (virtualKey >= 'a' && virtualKey <= 'z')
    {
        virtualKey -= 'a' - 'A';
    }
    const bool uiOpen = g_checkpointMapEditorUiOpen.load();
    const bool modeActive = g_checkpointMapEditorModeActive.load();
    const bool down = eventDown;
    const int action =
        CheckpointSystem_MapEditorActionForKey(
            virtualKey,
            uiOpen);
    if (uiOpen &&
        CheckpointSystem_IsMapEditorTextInputFocused())
    {
        if (!down)
        {
            const int releasedAction =
                CheckpointSystem_MapEditorActionForKey(
                    virtualKey,
                    true);
            if (releasedAction >= 0 && releasedAction < 9)
            {
                g_checkpointMapEditorActionKeyDown[
                    releasedAction] = false;
            }
            g_checkpointMapEditorEditKeyDown = false;
            g_checkpointMapEditorCycleKeyDown = false;
            g_checkpointMapEditorToggleKeyDown = false;
        }
        return false;
    }

    if (virtualKey == 'N')
    {
        if (down)
        {
            bool expected = false;
            if (g_checkpointMapEditorToggleKeyDown
                    .compare_exchange_strong(expected, true))
            {
                CheckpointSystem_RequestMapEditorToggle();
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                    L"Map editor toggle key queued by %s.",
                    source != nullptr ? source : L"input");
            }
        }
        else
        {
            g_checkpointMapEditorToggleKeyDown = false;
        }
        return true;
    }

    if (modeActive && virtualKey == 'B')
    {
        if (!down)
        {
            g_checkpointMapEditorActionKeyDown[
                kCheckpointMapEditorClose] = false;
            g_checkpointMapEditorEditKeyDown = false;
            return true;
        }
        // A B press is either an edit/ open press or a close press. Once
        // classified, late copies of the same physical key-down must not
        // perform the opposite transition after the UI state changes.
        if (g_checkpointMapEditorEditKeyDown.load() ||
            g_checkpointMapEditorActionKeyDown[
                kCheckpointMapEditorClose].load())
        {
            return true;
        }
        if (uiOpen)
        {
            constexpr int closeAction =
                kCheckpointMapEditorClose;
            if (down)
            {
                bool expected = false;
                if (g_checkpointMapEditorActionKeyDown[closeAction]
                        .compare_exchange_strong(expected, true))
                {
                    g_checkpointMapEditorActionRequested =
                        closeAction;
                    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                        L"Map editor B close queued by %s.",
                        source != nullptr ? source : L"input");
                }
            }
            return true;
        }
        bool expected = false;
        if (g_checkpointMapEditorEditKeyDown
                .compare_exchange_strong(expected, true))
        {
            g_checkpointMapEditorEditRequested = true;
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Map editor B key queued by %s.",
                source != nullptr ? source : L"input");
        }
        return true;
    }

    if (modeActive && virtualKey == 'X' && !uiOpen)
    {
        if (down)
        {
            bool expected = false;
            if (g_checkpointMapEditorCycleKeyDown
                    .compare_exchange_strong(expected, true))
            {
                g_checkpointMapEditorActionRequested =
                    kCheckpointMapEditorCycleNearby;
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                    L"Map editor nearby-point cycle queued by %s.",
                    source != nullptr ? source : L"input");
            }
        }
        else
        {
            g_checkpointMapEditorCycleKeyDown = false;
        }
        return true;
    }

    if (action >= 0 && action < 9)
    {
        if (down)
        {
            bool expected = false;
            if (g_checkpointMapEditorActionKeyDown[action]
                    .compare_exchange_strong(expected, true))
            {
                g_checkpointMapEditorActionRequested = action;
                LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                    L"Map editor action %d queued by %s.",
                    action,
                    source != nullptr ? source : L"input");
            }
        }
        else
        {
            g_checkpointMapEditorActionKeyDown[action] = false;
            if (virtualKey == 'B')
            {
                g_checkpointMapEditorEditKeyDown = false;
            }
        }
        return true;
    }
    const int releasedAction =
        CheckpointSystem_MapEditorActionForKey(
            virtualKey,
            true);
    if (!down &&
        releasedAction >= 0 &&
        releasedAction < 9)
    {
        g_checkpointMapEditorActionKeyDown[releasedAction] = false;
        if (virtualKey == 'B')
        {
            g_checkpointMapEditorEditKeyDown = false;
        }
    }
    return false;
}

void CheckpointSystem_ProcessMapEditorPointer()
{
    if (!g_checkpointMapEditorUiOpen.load())
    {
        return;
    }
    const unsigned int releaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    if (releaseSerial ==
        g_checkpointMapEditorObservedReleaseSerial)
    {
        return;
    }
    g_checkpointMapEditorObservedReleaseSerial = releaseSerial;
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
        static_cast<float>(
            clientRect.right - clientRect.left);
    const float height =
        static_cast<float>(
            clientRect.bottom - clientRect.top);
    if (width <= 0.0f || height <= 0.0f)
    {
        return;
    }
    const float x =
        static_cast<float>(point.x) / width;
    const float y =
        1.0f - static_cast<float>(point.y) / height;
    const bool chanceInputHit =
        g_checkpointMapEditorDraftKind != "spawn" &&
        CheckpointSystem_MapEditorPointIn(
            x,
            y,
            CheckpointSystem_MapEditorChildRect(
                0.330f, 0.545f, 0.670f, 0.625f));
    const bool noteInputHit =
        CheckpointSystem_MapEditorPointIn(
            x,
            y,
            CheckpointSystem_MapEditorChildRect(
                0.070f, 0.285f, 0.930f, 0.385f));
    if (!chanceInputHit &&
        !noteInputHit)
    {
        ClearSceneEventSelection();
        g_checkpointMapEditorTextInputFocused = false;
    }
    const bool supplyFieldsVisible =
        g_checkpointMapEditorDraftKind == "supply";
    if (supplyFieldsVisible &&
        g_checkpointMapEditorSupplyCountDropdownOpen.load())
    {
        for (int index = 0; index < 3; ++index)
        {
            const float maxY = 0.410f - index * 0.060f;
            if (CheckpointSystem_MapEditorPointIn(
                    x,
                    y,
                    CheckpointSystem_MapEditorChildRect(
                        0.070f,
                        maxY - 0.050f,
                        0.480f,
                        maxY)))
            {
                CheckpointSystem_SelectSupplyCount(index + 1);
                return;
            }
        }
    }
    if (supplyFieldsVisible &&
        g_checkpointMapEditorSupplyQualityDropdownOpen.load())
    {
        for (int index = 0; index < 6; ++index)
        {
            const float maxY = 0.410f - index * 0.060f;
            if (CheckpointSystem_MapEditorPointIn(
                    x,
                    y,
                    CheckpointSystem_MapEditorChildRect(
                        0.520f,
                        maxY - 0.050f,
                        0.930f,
                        maxY)))
            {
                CheckpointSystem_SelectSupplyQuality(index);
                return;
            }
        }
    }
    if (g_checkpointMapEditorTypeDropdownOpen.load())
    {
        for (int index = 0; index < 5; ++index)
        {
            const float maxY = 0.635f - index * 0.058f;
            const float minY = maxY - 0.048f;
            if (CheckpointSystem_MapEditorPointIn(
                    x,
                    y,
                    CheckpointSystem_MapEditorChildRect(
                        0.070f,
                        minY,
                        0.930f,
                        maxY)))
            {
                CheckpointSystem_SelectMapEditorType(index);
                return;
            }
        }
    }
    int action = -1;
    if (CheckpointSystem_MapEditorPointIn(
            x,
            y,
            CheckpointSystem_MapEditorChildRect(
                0.070f, 0.655f, 0.930f, 0.735f)))
    {
        CheckpointSystem_SetMapEditorTypeDropdownOpen(
            !g_checkpointMapEditorTypeDropdownOpen.load());
        return;
    }
    else if (CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.070f, 0.545f, 0.310f, 0.625f)))
    {
        action = kCheckpointMapEditorChanceDown;
    }
    else if (CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.690f, 0.545f, 0.930f, 0.625f)))
    {
        action = kCheckpointMapEditorChanceUp;
    }
    else if (supplyFieldsVisible &&
             CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.070f, 0.425f, 0.480f, 0.505f)))
    {
        CheckpointSystem_SetSupplyCountDropdownOpen(
            !g_checkpointMapEditorSupplyCountDropdownOpen.load());
        return;
    }
    else if (supplyFieldsVisible &&
             CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.520f, 0.425f, 0.930f, 0.505f)))
    {
        CheckpointSystem_SetSupplyQualityDropdownOpen(
            !g_checkpointMapEditorSupplyQualityDropdownOpen.load());
        return;
    }
    else if (CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.070f, 0.205f, 0.340f, 0.270f)))
    {
        action = kCheckpointMapEditorPasteNote;
    }
    else if (CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.375f, 0.205f, 0.625f, 0.270f)))
    {
        action = kCheckpointMapEditorClearNote;
    }
    else if (CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.070f, 0.055f, 0.285f, 0.150f)))
    {
        action = kCheckpointMapEditorDelete;
    }
    else if (CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.335f, 0.055f, 0.665f, 0.150f)))
    {
        action = kCheckpointMapEditorSave;
    }
    else if (CheckpointSystem_MapEditorPointIn(
                 x,
                 y,
                 CheckpointSystem_MapEditorChildRect(
                     0.715f, 0.055f, 0.930f, 0.150f)))
    {
        action = kCheckpointMapEditorClose;
    }
    if (action >= 0)
    {
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Map editor pointer action=%d queued.",
            action);
        CheckpointSystem_ProcessMapEditorAction(action);
    }
}

void CheckpointSystem_ProcessMapEditorAction(int action)
{
    if (action == kCheckpointMapEditorCycleNearby)
    {
        if (!g_checkpointMapEditorUiOpen &&
            g_checkpointMapEditorCandidates.size() > 1)
        {
            g_checkpointMapEditorCandidateCursor =
                (g_checkpointMapEditorCandidateCursor + 1) %
                g_checkpointMapEditorCandidates.size();
            g_checkpointMapEditorNearbyIndex =
                g_checkpointMapEditorCandidates[
                    g_checkpointMapEditorCandidateCursor];
        }
        return;
    }
    if (!g_checkpointMapEditorUiOpen)
    {
        return;
    }
    if (action == kCheckpointMapEditorCycleType)
    {
        static const char* kinds[] = {
            "supply",
            "extract",
            "transfer",
            "spawn",
            "task_random",
        };
        int current = 0;
        for (int index = 0; index < 5; ++index)
        {
            if (g_checkpointMapEditorDraftKind == kinds[index])
            {
                current = index;
                break;
            }
        }
        g_checkpointMapEditorDraftKind =
            kinds[(current + 1) % 5];
    }
    else if (action == kCheckpointMapEditorChanceDown)
    {
        const int step =
            (GameplayInput_ReadRawAsyncKeyState(VK_SHIFT) & 0x8000) != 0
                ? 1
                : 10;
        g_checkpointMapEditorDraftChance =
            std::max(0, g_checkpointMapEditorDraftChance - step);
    }
    else if (action == kCheckpointMapEditorChanceUp)
    {
        const int step =
            (GameplayInput_ReadRawAsyncKeyState(VK_SHIFT) & 0x8000) != 0
                ? 1
                : 10;
        g_checkpointMapEditorDraftChance =
            std::min(100, g_checkpointMapEditorDraftChance + step);
    }
    else if (action == kCheckpointMapEditorPasteNote)
    {
        const std::string text =
            CheckpointSystem_ReadClipboardText();
        if (text.empty())
        {
            CheckpointSystem_SetMapEditorNotice(
                "剪贴板中没有可用的文本");
        }
        else
        {
            g_checkpointMapEditorDraftNote = text;
            CheckpointSystem_SetMapEditorNotice(
                "备注已从剪贴板粘贴");
        }
    }
    else if (action == kCheckpointMapEditorClearNote)
    {
        g_checkpointMapEditorDraftNote.clear();
    }
    else if (action == kCheckpointMapEditorDelete)
    {
        CheckpointSystem_DeleteMapEditorDraft();
        return;
    }
    else if (action == kCheckpointMapEditorSave)
    {
        if (CheckpointSystem_CommitMapEditorDraft() &&
            CheckpointSystem_SaveMapEditorFiles())
        {
            CheckpointSystem_CloseMapEditorPanel(false);
        }
        return;
    }
    else if (action == kCheckpointMapEditorClose)
    {
        CheckpointSystem_CloseMapEditorPanel(true);
        return;
    }
    if (action == kCheckpointMapEditorChanceDown ||
        action == kCheckpointMapEditorChanceUp)
    {
        CheckpointSystem_RefreshMapEditorInputValues(
            true,
            false,
            false);
    }
    else if (action == kCheckpointMapEditorPasteNote ||
             action == kCheckpointMapEditorClearNote)
    {
        CheckpointSystem_RefreshMapEditorInputValues(
            false,
            false,
            true);
    }
    CheckpointSystem_UpdateMapEditorPanelText();
}

const char* CheckpointSystem_BuildMapEditorPrompt(
    int definitionIndex,
    char* output,
    size_t outputSize)
{
    if (definitionIndex ==
        kCheckpointMapEditorCreateCandidate)
    {
        std::snprintf(
            output,
            outputSize,
            "创建新地图点");
        if (g_checkpointMapEditorCandidates.size() > 1)
        {
            const size_t length = std::strlen(output);
            std::snprintf(
                output + length,
                outputSize > length
                    ? outputSize - length
                    : 0,
                "  [X 切换]");
        }
        return output;
    }
    if (definitionIndex < 0 ||
        definitionIndex >=
            static_cast<int>(g_checkpointDefinitions.size()))
    {
        output[0] = '\0';
        return output;
    }
    const auto& definition =
        g_checkpointDefinitions[
            static_cast<size_t>(definitionIndex)];
    if (definition.kind == "supply")
    {
        const int configuredQuality = std::clamp(
            definition.metadata.value(
                "supplyQuality",
                0),
            0,
            5);
        const int quality = configuredQuality > 0
            ? configuredQuality - 1
            : -1;
        const int chance = std::clamp(
            static_cast<int>(std::round(
                definition.metadata.value(
                    "spawnChance",
                    1.0) * 100.0)),
            0,
            100);
        std::snprintf(
            output,
            outputSize,
            "编辑物资点（%d件，%s，%d%%）",
            CheckpointSystem_GetSupplyCount(
                definition.metadata),
            quality >= 0
                ? CheckpointSystem_QualityDisplayName(quality)
                : "随机品质",
            chance);
    }
    else if (definition.kind == "transfer")
    {
        const int targetStage =
            definition.metadata.value(
                "targetStageType",
                -1);
        std::snprintf(
            output,
            outputSize,
            "编辑转移点（%s）",
            CheckpointSystem_StageDisplayName(targetStage));
    }
    else
    {
        std::snprintf(
            output,
            outputSize,
            "编辑%s",
            CheckpointSystem_MapEditorKindDisplay(
                definition.kind));
    }
    if (g_checkpointMapEditorCandidates.size() > 1)
    {
        const size_t length = std::strlen(output);
        std::snprintf(
            output + length,
            outputSize > length
                ? outputSize - length
                : 0,
            "  [X 切换]");
    }
    return output;
}

void CheckpointSystem_UpdateMapEditor(
    const float player[3],
    bool& ownsPrompt)
{
    ownsPrompt = false;
    if (g_checkpointMapEditorToggleRequested.exchange(false))
    {
        if (!g_checkpointMapEditorModeActive)
        {
            CheckpointSystem_TryEnterMapEditorMode();
        }
        else
        {
            CheckpointSystem_ExitMapEditorMode();
        }
    }
    if (!g_checkpointMapEditorModeActive)
    {
        return;
    }
    CheckpointSystem_ProcessMapEditorPointer();
    if (!g_checkpointMapEditorModeActive)
    {
        return;
    }
    const int action =
        g_checkpointMapEditorActionRequested.exchange(-1);
    if (action >= 0)
    {
        CheckpointSystem_ProcessMapEditorAction(action);
    }

    CheckpointSystem_EnsureMapEditorHud();
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointMapEditorHudRoot.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointMapEditorHudRoot.gameObject,
            true);
        char status[256]{};
        std::snprintf(
            status,
            sizeof(status),
            "地图点标注模式  |  已标注 %u 个点  |  N 退出  |  B 编辑/创建",
            static_cast<unsigned int>(std::count_if(
                g_checkpointDefinitions.begin(),
                g_checkpointDefinitions.end(),
                [](const CheckpointDefinition& definition)
                {
                    return !definition.deleted;
                })));
        SetSceneText(
            g_checkpointMapEditorStatus,
            status,
            15.0f,
            514,
            0.94f,
            0.96f,
            0.98f,
            1.0f);
    }
    if (g_checkpointMapEditorUiOpen)
    {
        CheckpointSystem_UpdateMapEditorTextInputFocus();
        CheckpointSystem_UpdateMapEditorPanelText();
        ExtendedMode_SetCheckpointEditorPrompt(false, "");
        return;
    }

    const bool moved =
        std::fabs(player[0] - g_checkpointMapEditorLastPlayer[0]) > 0.10f ||
        std::fabs(player[1] - g_checkpointMapEditorLastPlayer[1]) > 0.10f ||
        std::fabs(player[2] - g_checkpointMapEditorLastPlayer[2]) > 0.10f;
    if (moved)
    {
        std::memcpy(
            g_checkpointMapEditorLastPlayer,
            player,
            sizeof(g_checkpointMapEditorLastPlayer));
        g_checkpointMapEditorCandidateCursor = 0;
    }
    g_checkpointMapEditorCandidates.clear();
    for (size_t index = 0;
         index < g_checkpointDefinitions.size();
         ++index)
    {
        const auto& definition = g_checkpointDefinitions[index];
        if (definition.deleted)
        {
            continue;
        }
        const float dx = player[0] - definition.x;
        const float dy = player[1] - definition.y;
        const float dz = player[2] - definition.z;
        if (dx * dx + dz * dz <=
                definition.triggerRadius *
                    definition.triggerRadius &&
            std::fabs(dy) <=
                definition.heightTolerance)
        {
            g_checkpointMapEditorCandidates.push_back(
                static_cast<int>(index));
        }
    }
    g_checkpointMapEditorCandidates.push_back(
        kCheckpointMapEditorCreateCandidate);
    if (g_checkpointMapEditorCandidateCursor >=
        g_checkpointMapEditorCandidates.size())
    {
        g_checkpointMapEditorCandidateCursor = 0;
    }
    g_checkpointMapEditorNearbyIndex =
        g_checkpointMapEditorCandidates[
            g_checkpointMapEditorCandidateCursor];
    if (g_checkpointMapEditorEditRequested.exchange(false))
    {
        if (g_checkpointMapEditorNearbyIndex >= 0)
        {
            CheckpointSystem_OpenMapEditorPanel(
                g_checkpointMapEditorNearbyIndex);
        }
        else
        {
            CheckpointSystem_CreateMapEditorPanel(player);
        }
        return;
    }
    char prompt[192]{};
    CheckpointSystem_BuildMapEditorPrompt(
        g_checkpointMapEditorNearbyIndex,
        prompt,
        sizeof(prompt));
    ownsPrompt = true;
    ExtendedMode_SetCheckpointEditorPrompt(
        true,
        prompt);
}
