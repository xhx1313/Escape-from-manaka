// Native.cpp module: inventory_ui.inc
// Tarkov-inspired equipment, backpack and stash interface for the black room.

constexpr int kInventoryRouteEquipment = 0;
constexpr int kInventoryRouteLobby = 1;
constexpr int kInventoryRouteMarket = 2;
constexpr int kInventoryRouteCabinet = 3;
constexpr int kInventoryRouteCount = 4;
constexpr int kInventoryStorageBackpack = 0;
constexpr int kInventoryStorageStash = 1;
constexpr int kInventoryStorageSafeBox = 2;
constexpr int kInventoryStorageEquipped = 3;
constexpr int kInventoryGridStorageCount = 3;
constexpr int kInventoryMaxGridCells = 256;
constexpr int kInventoryEquipmentCategoryCount = 9;
constexpr int kInventoryBackpackCategoryIndex = 9;
constexpr int kInventoryQualityCount = 5;
constexpr int kInventoryExtraItemSlotCount = 32;
constexpr int kInventoryQuickSlotCount = 3;
constexpr uint32_t kInventoryEquipmentCategoryMask = (1u << 9) - 1u;
constexpr uint32_t kInventoryPropCategoryBit = 1u << 9;
constexpr uint32_t kInventoryBackpackCategoryBit = 1u << 10;

struct InventoryStorageLayout
{
    int columns;
    int rows;
    int visibleRows;
    float panelMinX;
    float panelMinY;
    float panelMaxX;
    float panelMaxY;
    float gridMinX;
    float gridMinY;
    float gridMaxX;
    float gridMaxY;
};

struct EquipmentInventoryDefinition
{
    const char* id;
    const char* name;
    int width;
    int height;
    uint32_t categories;
    int series;
};

#include "equipment_inventory_data.inc"

constexpr int kInventoryItemSlotCount =
    kEquipmentInventoryDefinitionCount + kInventoryExtraItemSlotCount;

struct InventoryBackpackTier
{
    const char* itemId;
    const char* name;
    int columns;
    int rows;
};

struct InventoryCategoryDefinition
{
    const char* key;
    const char* label;
    uint32_t bit;
};

struct InventoryItemState
{
    int definition;
    int quality;
    bool appraised;
    bool appraisalDebug;
    int quickSlot;
    unsigned int wearUnits;
    int storage;
    int x;
    int y;
    int width;
    int height;
    SceneUiElement element;
    SceneUiElement content;
    SceneUiElement icon;
    SceneUiElement label;
    SceneUiElement appraisalBadgeCircle;
    SceneUiElement appraisalBadgeText;
    float screenMinX;
    float screenMinY;
    float screenMaxX;
    float screenMaxY;
};

struct InventoryDroppedDildoSnapshot
{
    bool active = false;
    std::string itemId;
    int quality = 0;
    bool appraised = false;
    unsigned int wearUnits = 0;
    int stageType = -1;
    int dropItemType = -1;
};

struct InventoryGridCell
{
    int storage;
    int column;
    int row;
    SceneUiElement element;
};

struct InventoryEquipmentSlot
{
    SceneUiElement panel;
    SceneUiElement icon;
    SceneUiElement label;
    std::wstring currentItemId;
    int currentQuality = -1;
    float screenMinX;
    float screenMinY;
    float screenMaxX;
    float screenMaxY;
};

struct InventoryQuickSlot
{
    SceneUiElement panel;
    SceneUiElement icon;
    SceneUiElement label;
    SceneUiElement keyLabel;
    std::wstring currentItemId;
    int currentQuality = -1;
    int itemIndex = -1;
    int definition = -1;
    int quality = -1;
    float screenMinX = 0.0f;
    float screenMinY = 0.0f;
    float screenMaxX = 0.0f;
    float screenMaxY = 0.0f;
};

struct InventoryGridMetrics
{
    float minX;
    float minY;
    float maxX;
    float maxY;
    float cellWidth;
    float cellHeight;
    float cellPixels;
};

static constexpr InventoryBackpackTier kInventoryBackpackTiers[] = {
    { "061-cosplay-05-004-rcosplaypartslegbag", "狱警·腿包",       2, 4 },
    { "120-cosplay-09-006-rcosplaypartslegbag", "战术女仆·腿包",   3, 4 },
    { "190-cosplay-20-005-rcosplaypartsbag",    "校园辣妹·包袋",   4, 5 },
    { "157-cosplay-19-003-rcosplaypartsbag",    "校园连帽·包袋",   5, 6 },
    { "143-cosplay-01-011-rcosplaypartsoriginalrandoseru", "原创·双肩书包", 6, 8 },
};

constexpr int kInventoryBackpackTierCount =
    static_cast<int>(sizeof(kInventoryBackpackTiers) /
                     sizeof(kInventoryBackpackTiers[0]));

static InventoryStorageLayout kInventoryStorageLayouts[] = {
    {
        6, 8, 8,
        0.360f, 0.385f, 0.606f, 0.884f,
        0.035f, 0.055f, 0.965f, 0.825f
    },
    {
        10, 20, 8,
        0.616f, 0.055f, 0.975f, 0.884f, 0.055f, 0.055f, 0.945f, 0.865f
    },
    {
        2, 2, 2,
        0.360f, 0.055f, 0.606f, 0.360f,
        0.080f, 0.040f, 0.920f, 0.690f
    },
};

static constexpr InventoryCategoryDefinition
    kInventoryEquipmentCategories[kInventoryEquipmentCategoryCount] = {
        { "headwear",   "头饰", 1u << 0 },
        { "necklace",   "项链", 1u << 1 },
        { "top",        "上衣", 1u << 2 },
        { "bra",        "胸衣", 1u << 3 },
        { "underpants", "内裤", 1u << 4 },
        { "bottom",     "下衣", 1u << 5 },
        { "socks",      "袜子", 1u << 6 },
        { "shoes",      "鞋子", 1u << 7 },
        { "accessory",  "饰品", 1u << 8 },
    };

static const char* const
    kInventoryQualityLabels[kInventoryQualityCount] = {
        "白", "绿", "紫", "金", "红",
    };

static const wchar_t* const
    kInventoryQualityDirectories[kInventoryQualityCount] = {
        L"white", L"green", L"purple", L"gold", L"black",
    };

static const float
    kInventoryQualityColors[kInventoryQualityCount][3] = {
        { 0.88f, 0.90f, 0.93f },
        { 0.12f, 0.55f, 0.22f },
        { 0.52f, 0.20f, 0.72f },
        { 0.78f, 0.53f, 0.12f },
        { 0.72f, 0.08f, 0.10f },
    };

static const char* const kInventoryRouteLabels[kInventoryRouteCount] = {
    "装备",
    "联机大厅",
    "跳蚤市场",
    "展览柜",
};

static const char* const kInventoryRouteSubtitles[kInventoryRouteCount] = {
    "LOADOUT / INVENTORY",
    "MULTIPLAYER LOBBY",
    "FLEA MARKET",
    "EXHIBITION CABINET",
};

static InventoryItemState
    g_inventoryItems[kInventoryItemSlotCount]{};
static InventoryGridCell
    g_inventoryGridCells[kInventoryGridStorageCount][kInventoryMaxGridCells]{};
static int g_inventoryGridCellCounts[kInventoryGridStorageCount]{};
static InventoryEquipmentSlot
    g_inventoryEquipmentSlots[kInventoryEquipmentCategoryCount]{};
static InventoryEquipmentSlot g_inventoryBackpackSlot{};
static InventoryQuickSlot
    g_inventoryQuickSlots[kInventoryQuickSlotCount]{};
static SceneUiElement g_inventoryRouteButtons[kInventoryRouteCount]{};
static Il2CppObject* g_inventoryEquipmentPage = nullptr;
static Il2CppObject* g_inventoryPlaceholderPage = nullptr;
static Il2CppObject* g_inventoryPlaceholderTitle = nullptr;
static Il2CppObject* g_inventoryStatusLabel = nullptr;
static Il2CppObject* g_inventoryStatsLabel = nullptr;
static Il2CppObject* g_inventoryBackpackSizeLabel = nullptr;
static Il2CppObject* g_inventorySafeBoxSizeLabel = nullptr;
static SceneUiElement g_inventoryStashScrollTrack{};
static SceneUiElement g_inventoryStashScrollThumb{};
static int g_inventoryActiveRoute = kInventoryRouteEquipment;
static int g_inventoryStashScrollRow = 0;
static bool g_inventoryScrollDragging = false;
static float g_inventoryScrollDragOffsetY = 0.0f;
static std::atomic<int> g_inventoryStashWheelDelta{0};
static float g_inventoryStashThumbScreenMinY = 0.0f;
static float g_inventoryStashThumbScreenMaxY = 0.0f;
static int g_inventoryDraggingItem = -1;
static int g_inventoryDragSourceStorage = -1;
static int g_inventoryDragSourceX = 0;
static int g_inventoryDragSourceY = 0;
static int g_inventoryDragSourceWidth = 1;
static int g_inventoryDragSourceHeight = 1;
static int g_inventoryDragGrabCellX = 0;
static int g_inventoryDragGrabCellY = 0;
static int g_inventoryHoveredEquipmentSlot = -1;
static int g_inventoryHoveredQuickSlot = -1;
static int g_inventoryQuickSlotDragSource = -1;
static float g_inventoryDragPointerOffsetPixelsX = 0.0f;
static float g_inventoryDragPointerOffsetPixelsY = 0.0f;
static float g_inventoryDragStartCursorPixelsX = 0.0f;
static float g_inventoryDragStartCursorPixelsY = 0.0f;
static float g_inventoryDragCellWidth = 0.0f;
static float g_inventoryDragCellHeight = 0.0f;
static bool g_inventoryDragActive = false;
static bool g_inventoryDragMoved = false;
static bool g_inventoryRotateKeyDown = false;
static bool g_inventoryLeftButtonWasDown = false;
static bool g_inventoryStateLoaded = false;
static int g_inventoryStarterGrantVersion = 0;
static InventoryDroppedDildoSnapshot g_inventoryDroppedDildo;
static Il2CppObject* g_inventoryBadgeTexture = nullptr;
static Il2CppObject* g_inventoryBadgeSprite = nullptr;
static unsigned long long g_inventoryInputUnlockTick = 0;
static float g_inventoryLayoutWidth = 1920.0f;
static float g_inventoryLayoutHeight = 1080.0f;
static bool g_inventoryLayoutReady = false;

void ExhibitionUi_Build(Il2CppObject* rootTransform);
void ExhibitionUi_SetPageVisible(bool visible);
void ExhibitionUi_Reset();
void ExhibitionUi_Tick();
void ExhibitionUi_UpdateLayout();
bool ExhibitionUi_HandleButtonClick(Il2CppObject* instance);
void ExhibitionUi_LoadState(const nlohmann::json& root);
void ExhibitionUi_WriteState(nlohmann::json& root);
bool ExhibitionUi_ShouldHandleManualPointerAt(float cursorX, float cursorY);
int ExhibitionUi_GetCurrentLevel();
int ExhibitionUi_GetSafeBoxColumns();
int ExhibitionUi_GetSafeBoxRows();
int ExhibitionUi_GetSafeBoxCells();
bool ExhibitionUi_IsSafeBoxCellActive(int column, int row);
void InventoryUi_ApplySafeBoxCapacity();

bool InventoryUi_IsBackpackDefinition(int definitionIndex);
bool InventoryUi_IsDildoDefinition(int definitionIndex);
int InventoryUi_FindBackpackTier(int definitionIndex);
void InventoryUi_SaveState();
void InventoryUi_InitializeItems();

bool InventoryUi_IsItemSlot(int itemIndex)
{
    return itemIndex >= 0 && itemIndex < kInventoryItemSlotCount;
}

bool InventoryUi_ItemSlotIsBackpack(int itemIndex)
{
    return InventoryUi_IsItemSlot(itemIndex) &&
           InventoryUi_IsBackpackDefinition(
               g_inventoryItems[itemIndex].definition);
}

int InventoryUi_ItemSlotBackpackTier(int itemIndex)
{
    return InventoryUi_IsItemSlot(itemIndex)
        ? InventoryUi_FindBackpackTier(
              g_inventoryItems[itemIndex].definition)
        : -1;
}

bool InventoryUi_CanAppraiseItem(int itemIndex)
{
    if (!InventoryUi_IsItemSlot(itemIndex))
    {
        return false;
    }
    const auto& item = g_inventoryItems[itemIndex];
    return item.storage == kInventoryStorageStash &&
           item.quality == kInventoryQualityCount - 1 &&
           !item.appraised;
}

long long InventoryUi_BaseValueForGrid(
    int width,
    int height,
    int quality)
{
    const long long cells = static_cast<long long>(
        std::max(1, width * height));
    const long long qualityLevel = static_cast<long long>(
        std::clamp(
            quality,
            0,
            kInventoryQualityCount - 1)) +
        1ll;
    return cells * qualityLevel * 1000ll;
}

long long InventoryUi_ItemBaseValue(int itemIndex)
{
    if (!InventoryUi_IsItemSlot(itemIndex))
    {
        return 0;
    }
    const auto& item = g_inventoryItems[itemIndex];
    const auto& definition =
        kEquipmentInventoryDefinitions[item.definition];
    return InventoryUi_BaseValueForGrid(
        definition.width,
        definition.height,
        item.quality);
}

double InventoryUi_ItemCurrentValue(int itemIndex)
{
    if (!InventoryUi_IsItemSlot(itemIndex))
    {
        return 0.0;
    }
    const auto& item = g_inventoryItems[itemIndex];
    const double baseValue =
        static_cast<double>(InventoryUi_ItemBaseValue(itemIndex));
    if (!item.appraised || item.wearUnits == 0)
    {
        return baseValue;
    }
    const double wear =
        static_cast<double>(item.wearUnits) / 100000000.0;
    if (item.wearUnits >= 1000000u)
    {
        return baseValue / wear;
    }
    const double denominator =
        0.01 * std::pow(baseValue / 0.01, 0.25);
    return baseValue / denominator;
}

double InventoryUi_InstanceCurrentValue(
    int width,
    int height,
    int quality,
    bool appraised,
    unsigned int wearUnits)
{
    const double baseValue = static_cast<double>(
        InventoryUi_BaseValueForGrid(
            width,
            height,
            quality));
    if (!appraised || wearUnits == 0)
    {
        return baseValue;
    }
    const double wear =
        static_cast<double>(wearUnits) / 100000000.0;
    if (wearUnits >= 1000000u)
    {
        return baseValue / wear;
    }
    const double denominator =
        0.01 * std::pow(baseValue / 0.01, 0.25);
    return baseValue / denominator;
}

std::wstring InventoryUi_FormatWear(unsigned int wearUnits)
{
    wchar_t text[32]{};
    swprintf_s(
        text,
        L"%u.%08u",
        wearUnits / 100000000u,
        wearUnits % 100000000u);
    return text;
}

bool InventoryUi_IsDildoDefinition(int definitionIndex)
{
    return definitionIndex >= 0 &&
           definitionIndex < kEquipmentInventoryDefinitionCount &&
           std::strcmp(
               kEquipmentInventoryDefinitions[definitionIndex].id,
               "217-adult-011-mitemdildo") == 0;
}

void InventoryUi_RecordDroppedDildo(
    int definitionIndex,
    int quality,
    bool appraised,
    unsigned int wearUnits,
    int stageType,
    int dropItemType)
{
    if (!InventoryUi_IsDildoDefinition(definitionIndex))
    {
        return;
    }
    g_inventoryDroppedDildo.active = true;
    g_inventoryDroppedDildo.itemId =
        kEquipmentInventoryDefinitions[definitionIndex].id;
    g_inventoryDroppedDildo.quality = std::clamp(
        quality,
        0,
        kInventoryQualityCount - 1);
    g_inventoryDroppedDildo.appraised =
        appraised && wearUnits != 0;
    g_inventoryDroppedDildo.wearUnits =
        g_inventoryDroppedDildo.appraised ? wearUnits : 0;
    g_inventoryDroppedDildo.stageType = stageType;
    g_inventoryDroppedDildo.dropItemType = dropItemType;
    InventoryUi_SaveState();
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Dildo dropped state saved: quality=%d wear=%u stage=%d type=%d.",
        quality,
        wearUnits,
        stageType,
        dropItemType);
}

bool InventoryUi_TakeDroppedDildo(
    int stageType,
    int dropItemType,
    InventoryDroppedDildoSnapshot& result)
{
    if (!g_inventoryDroppedDildo.active ||
        (stageType >= 0 &&
         g_inventoryDroppedDildo.stageType != stageType) ||
        (dropItemType >= 0 &&
         g_inventoryDroppedDildo.dropItemType != dropItemType))
    {
        return false;
    }
    result = g_inventoryDroppedDildo;
    g_inventoryDroppedDildo = InventoryDroppedDildoSnapshot{};
    InventoryUi_SaveState();
    return true;
}

enum InventoryAppraisalTier
{
    InventoryAppraisalTier_R = 0,
    InventoryAppraisalTier_SR = 1,
    InventoryAppraisalTier_SSR = 2,
    InventoryAppraisalTier_UR = 3,
};

InventoryAppraisalTier InventoryUi_GetAppraisalTier(
    unsigned int wearUnits)
{
    if (wearUnits >= 100000000u)
    {
        return InventoryAppraisalTier_R;
    }
    if (wearUnits >= 50000000u)
    {
        return InventoryAppraisalTier_SR;
    }
    if (wearUnits >= 10000000u)
    {
        return InventoryAppraisalTier_SSR;
    }
    return InventoryAppraisalTier_UR;
}

const wchar_t* InventoryUi_AppraisalTierLabel(
    InventoryAppraisalTier tier)
{
    switch (tier)
    {
    case InventoryAppraisalTier_SR: return L"SR";
    case InventoryAppraisalTier_SSR: return L"SSR";
    case InventoryAppraisalTier_UR: return L"UR";
    default: return L"R";
    }
}

void InventoryUi_AppraisalTierColor(
    InventoryAppraisalTier tier,
    float& red,
    float& green,
    float& blue)
{
    switch (tier)
    {
    case InventoryAppraisalTier_SR:
        red = 0.16f;
        green = 0.68f;
        blue = 0.25f;
        break;
    case InventoryAppraisalTier_SSR:
        red = 0.62f;
        green = 0.24f;
        blue = 0.82f;
        break;
    case InventoryAppraisalTier_UR:
        red = 1.00f;
        green = 0.70f;
        blue = 0.12f;
        break;
    default:
        red = 0.62f;
        green = 0.65f;
        blue = 0.68f;
        break;
    }
}

Il2CppObject* InventoryUi_GetAppraisalBadgeSprite()
{
    if (g_inventoryBadgeSprite != nullptr)
    {
        return g_inventoryBadgeSprite;
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
    void* textureParameters[] = {
        const_cast<int*>(&size),
        const_cast<int*>(&size),
        &textureFormat,
        &mipChain
    };
    Invoke(
        FindMethod(textureClass, ".ctor", 4),
        texture,
        textureParameters);
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
            const float alpha = std::clamp(
                (1.0f - distance) * 20.0f,
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
    void* pixelParameters[] = { pixels, &mipLevel };
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
    g_inventoryBadgeTexture = texture;
    g_inventoryBadgeSprite = sprite;
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Inventory appraisal badge sprite created: texture=%p sprite=%p.",
        texture,
        sprite);
    return g_inventoryBadgeSprite;
}

void InventoryUi_ApplyCircularBadgeSprite(
    SceneUiElement& badge)
{
    if (badge.image == nullptr)
    {
        return;
    }
    auto* sprite = CheckpointSystem_GetRingSprite();
    auto* setSprite = FindMethod(
        FindClass("UnityEngine.UI", "Image"),
        "set_sprite",
        1);
    if (sprite == nullptr || setSprite == nullptr)
    {
        static bool loggedFailure = false;
        if (!loggedFailure)
        {
            loggedFailure = true;
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Unable to configure appraisal badge sprite: sprite=%p method=%p.",
                sprite,
                setSprite);
        }
        return;
    }
    void* parameters[] = { sprite };
    Invoke(
        setSprite,
        badge.image,
        parameters);
}

bool InventoryUi_ReadClientSize(float& width, float& height)
{
    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    if (gameWindow == nullptr || !GetClientRect(gameWindow, &clientRect))
    {
        return false;
    }
    width = static_cast<float>(clientRect.right - clientRect.left);
    height = static_cast<float>(clientRect.bottom - clientRect.top);
    return width > 0.0f && height > 0.0f;
}

int InventoryUi_FindDefinitionById(const char* id)
{
    if (id == nullptr)
    {
        return -1;
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (std::strcmp(kEquipmentInventoryDefinitions[index].id, id) == 0)
        {
            return index;
        }
    }
    return -1;
}

void InventoryUi_InitializeQuickSlots()
{
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        auto& slot = g_inventoryQuickSlots[index];
        slot.definition = -1;
        slot.quality = -1;
        slot.itemIndex = -1;
        slot.currentItemId.clear();
        slot.currentQuality = -1;
    }
}

bool InventoryUi_IsPropDefinition(int definitionIndex)
{
    return definitionIndex >= 0 &&
           definitionIndex < kEquipmentInventoryDefinitionCount &&
           (kEquipmentInventoryDefinitions[definitionIndex].categories &
            kInventoryPropCategoryBit) != 0;
}

int InventoryUi_GetAdultGoodsType(int definitionIndex)
{
    if (definitionIndex < 0 ||
        definitionIndex >= kEquipmentInventoryDefinitionCount)
    {
        return -1;
    }
    const char* id =
        kEquipmentInventoryDefinitions[definitionIndex].id;
    struct AdultTypeMapping
    {
        const char* id;
        int type;
    };
    static constexpr AdultTypeMapping mappings[] = {
        { "200-adult-003-madultgoodsrecordhandcuff", 3 },
        { "202-adult-001-madultgoodsrecordvibrator", 1 },
        { "201-adult-002-madultgoodsrecordeyemask", 2 },
        { "205-adult-006-madultgoodsrecordvibratortit", 6 },
        { "206-adult-009-madultgoodsrecordpistonmachineanal", 9 },
        { "207-adult-007-madultgoodsrecordvibratorkuri", 7 },
        { "208-adult-008-madultgoodsrecordpistonmachinefuta", 8 },
        { "209-adult-010-madultgoodsrecordpistonmachinepussy", 10 },
    };
    for (const auto& mapping : mappings)
    {
        if (std::strcmp(mapping.id, id) == 0)
        {
            return mapping.type;
        }
    }
    return -1;
}

int InventoryUi_GetAdultGoodsGroup(int adultGoodsType)
{
    switch (adultGoodsType)
    {
    case 1:  // Vibrator
    case 6:  // TitRotor
    case 7:  // KuriRotor
        return 1;
    case 8:  // PistonFuta
    case 9:  // PistonAnal
    case 10: // PistonPussy
        return 2;
    case 2:  // EyeMask
        return 3;
    case 3:  // Handcuff
    case 5:  // TimerHandcuff
        return 4;
    default:
        return -1;
    }
}

bool InventoryUi_IsStandaloneDildoDefinition(
    int definitionIndex)
{
    return definitionIndex >= 0 &&
           definitionIndex < kEquipmentInventoryDefinitionCount &&
           std::strcmp(
               kEquipmentInventoryDefinitions[definitionIndex].id,
               "217-adult-011-mitemdildo") == 0;
}

bool InventoryUi_CanAssignQuickSlot(int itemIndex)
{
    if (!InventoryUi_IsItemSlot(itemIndex))
    {
        return false;
    }
    const auto& item = g_inventoryItems[itemIndex];
    return
        (item.storage == kInventoryStorageBackpack ||
         item.storage == kInventoryStorageSafeBox) &&
        (InventoryUi_GetAdultGoodsType(item.definition) >= 0 ||
         InventoryUi_IsStandaloneDildoDefinition(
             item.definition));
}

int InventoryUi_FindQuickSlotBoundItem(int quickIndex)
{
    if (quickIndex < 0 || quickIndex >= kInventoryQuickSlotCount)
    {
        return -1;
    }
    for (int itemIndex = 0;
         itemIndex < kInventoryItemSlotCount;
         ++itemIndex)
    {
        if (g_inventoryItems[itemIndex].quickSlot == quickIndex &&
            InventoryUi_CanAssignQuickSlot(itemIndex))
        {
            return itemIndex;
        }
    }
    return -1;
}

void InventoryUi_BindQuickSlot(int quickIndex, int itemIndex)
{
    if (quickIndex < 0 || quickIndex >= kInventoryQuickSlotCount)
    {
        return;
    }
    auto& slot = g_inventoryQuickSlots[quickIndex];
    const int previousItem = slot.itemIndex;
    if (previousItem >= 0 &&
        previousItem < kInventoryItemSlotCount &&
        g_inventoryItems[previousItem].quickSlot == quickIndex)
    {
        g_inventoryItems[previousItem].quickSlot = -1;
    }
    if (InventoryUi_CanAssignQuickSlot(itemIndex))
    {
        auto& item = g_inventoryItems[itemIndex];
        const int previousQuickSlot = item.quickSlot;
        if (previousQuickSlot >= 0 &&
            previousQuickSlot < kInventoryQuickSlotCount &&
            previousQuickSlot != quickIndex)
        {
            auto& previousSlot =
                g_inventoryQuickSlots[previousQuickSlot];
            if (previousSlot.itemIndex == itemIndex)
            {
                previousSlot.itemIndex = -1;
                previousSlot.definition = -1;
                previousSlot.quality = -1;
                previousSlot.currentItemId.clear();
                previousSlot.currentQuality = -1;
            }
        }
        item.quickSlot = quickIndex;
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        auto& item = g_inventoryItems[index];
        if (index != itemIndex &&
            item.quickSlot == quickIndex)
        {
            item.quickSlot = -1;
        }
    }
    slot.itemIndex = -1;
    slot.definition = -1;
    slot.quality = -1;
    slot.currentItemId.clear();
    slot.currentQuality = -1;
    if (!InventoryUi_CanAssignQuickSlot(itemIndex))
    {
        return;
    }
    const auto& item = g_inventoryItems[itemIndex];
    slot.itemIndex = itemIndex;
    slot.definition = item.definition;
    slot.quality = item.quality;
    LogInfo(
        LogCategory_Inventory,
        L"QuickSlotBind",
        L"Inventory quick slot %d bound itemSlot=%d definition=%d previousItem=%d.",
        quickIndex + 1,
        itemIndex,
        item.definition,
        previousItem);
}

void InventoryUi_PruneQuickSlotBindings()
{
    for (auto& slot : g_inventoryQuickSlots)
    {
        slot.itemIndex = -1;
        slot.definition = -1;
        slot.quality = -1;
        slot.currentItemId.clear();
        slot.currentQuality = -1;
    }
    for (int itemIndex = 0;
         itemIndex < kInventoryItemSlotCount;
         ++itemIndex)
    {
        auto& item = g_inventoryItems[itemIndex];
        const int quickIndex = item.quickSlot;
        if (quickIndex < 0)
        {
            continue;
        }
        if (quickIndex >= kInventoryQuickSlotCount ||
            !InventoryUi_CanAssignQuickSlot(itemIndex) ||
            g_inventoryQuickSlots[quickIndex].itemIndex >= 0)
        {
            item.quickSlot = -1;
            continue;
        }
        auto& slot = g_inventoryQuickSlots[quickIndex];
        slot.itemIndex = itemIndex;
        slot.definition = item.definition;
        slot.quality = item.quality;
    }
}

bool InventoryUi_IsBackpackDefinition(int definitionIndex)
{
    return definitionIndex >= 0 &&
           definitionIndex < kEquipmentInventoryDefinitionCount &&
           (kEquipmentInventoryDefinitions[definitionIndex].categories &
            kInventoryBackpackCategoryBit) != 0;
}

int InventoryUi_FindBackpackTier(int definitionIndex)
{
    if (!InventoryUi_IsBackpackDefinition(definitionIndex))
    {
        return -1;
    }
    const char* id = kEquipmentInventoryDefinitions[definitionIndex].id;
    for (int tier = 0; tier < kInventoryBackpackTierCount; ++tier)
    {
        if (std::strcmp(kInventoryBackpackTiers[tier].itemId, id) == 0)
        {
            return tier;
        }
    }
    return -1;
}

int InventoryUi_GetEquippedBackpackItem()
{
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (g_inventoryItems[index].storage == kInventoryStorageEquipped &&
            InventoryUi_ItemSlotIsBackpack(index))
        {
            return index;
        }
    }
    return -1;
}

int InventoryUi_GetEquippedBackpackTier()
{
    const int itemIndex = InventoryUi_GetEquippedBackpackItem();
    return itemIndex >= 0
        ? InventoryUi_ItemSlotBackpackTier(itemIndex)
        : -1;
}

bool InventoryUi_BackpackHasItems(int excludingItem)
{
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (index != excludingItem &&
            g_inventoryItems[index].storage == kInventoryStorageBackpack)
        {
            return true;
        }
    }
    return false;
}

bool InventoryUi_BackpackItemsFitTier(
    int tierIndex,
    int excludingItem)
{
    const int columns = tierIndex >= 0
        ? kInventoryBackpackTiers[tierIndex].columns
        : 0;
    const int rows = tierIndex >= 0
        ? kInventoryBackpackTiers[tierIndex].rows
        : 0;
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (index == excludingItem ||
            g_inventoryItems[index].storage != kInventoryStorageBackpack)
        {
            continue;
        }
        const auto& item = g_inventoryItems[index];
        if (item.x < 0 || item.y < 0 ||
            item.x + item.width > columns ||
            item.y + item.height > rows)
        {
            return false;
        }
    }
    return true;
}

bool InventoryUi_StateCellFree(
    int storage,
    int x,
    int y,
    int width,
    int height,
    int excludingItem)
{
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (index == excludingItem)
        {
            continue;
        }
        const auto& other = g_inventoryItems[index];
        if (other.storage != storage)
        {
            continue;
        }
        const bool separate =
            x + width <= other.x ||
            other.x + other.width <= x ||
            y + height <= other.y ||
            other.y + other.height <= y;
        if (!separate)
        {
            return false;
        }
    }
    return true;
}

int InventoryUi_FindFreeExtraItemSlot()
{
    for (int slot = kEquipmentInventoryDefinitionCount;
         slot < kInventoryItemSlotCount;
         ++slot)
    {
        if (g_inventoryItems[slot].storage < 0)
        {
            return slot;
        }
    }
    return -1;
}

int InventoryUi_StashMaxScrollRow();
void InventoryUi_ApplyBackpackCapacity();

int InventoryUi_GetSaveSlotIndex()
{
    auto* gameStateClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameState");
    auto* slotField = gameStateClass != nullptr
        ? il2cpp_class_get_field_from_name(gameStateClass, "SaveDataSlotIndex")
        : nullptr;
    int slot = 0;
    if (slotField != nullptr && il2cpp_field_static_get_value != nullptr)
    {
        il2cpp_field_static_get_value(slotField, &slot);
    }
    return std::max(0, slot);
}

std::wstring InventoryUi_GetStatePath()
{
    wchar_t profile[MAX_PATH]{};
    const DWORD profileLength = GetEnvironmentVariableW(
        L"USERPROFILE",
        profile,
        MAX_PATH);
    if (profileLength == 0 || profileLength >= MAX_PATH)
    {
        return GetEscapeFromManakaSubdirectory(L"Data") +
               L"\\Inventory_" +
               std::to_wstring(InventoryUi_GetSaveSlotIndex()) +
               L".json";
    }
    const std::wstring directory =
        std::wstring(profile) +
        L"\\AppData\\LocalLow\\SheableSoft\\SecretFlasherManaka\\SaveData\\EscapeFromManaka";
    CreateDirectoryW(directory.c_str(), nullptr);
    return directory + L"\\inventory_" +
           std::to_wstring(InventoryUi_GetSaveSlotIndex()) +
           L".json";
}

void InventoryUi_SaveState()
{
    if (!g_inventoryStateLoaded)
    {
        return;
    }
    const std::wstring path = InventoryUi_GetStatePath();
    const std::wstring temporary = path + L".tmp";
    FILE* file = nullptr;
    if (_wfopen_s(&file, temporary.c_str(), L"wb") != 0 || file == nullptr)
    {
        LogError(
            LogCategory_Persistence,
            L"SaveOpen",
            L"Inventory state save failed to open %s.",
            temporary.c_str());
        return;
    }
    nlohmann::json root;
    root["schema"] = 12;
    root["slot"] = InventoryUi_GetSaveSlotIndex();
    root["stash_scroll_row"] = g_inventoryStashScrollRow;
    if (g_inventoryStarterGrantVersion > 0)
    {
        root["starter_grant_version"] = g_inventoryStarterGrantVersion;
    }
    if (g_inventoryDroppedDildo.active)
    {
        nlohmann::json dropped;
        dropped["id"] = g_inventoryDroppedDildo.itemId;
        dropped["quality"] = g_inventoryDroppedDildo.quality;
        dropped["appraised"] = g_inventoryDroppedDildo.appraised;
        dropped["wear_units"] = g_inventoryDroppedDildo.wearUnits;
        dropped["stage_type"] = g_inventoryDroppedDildo.stageType;
        dropped["drop_item_type"] =
            g_inventoryDroppedDildo.dropItemType;
        root["dropped_dildo"] = std::move(dropped);
    }
    root["items"] = nlohmann::json::array();
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        const auto& item = g_inventoryItems[index];
        if (item.storage < 0)
        {
            continue;
        }
        nlohmann::json saved;
        saved["slot"] = index;
        saved["id"] =
            kEquipmentInventoryDefinitions[item.definition].id;
        saved["quality"] = item.quality;
        saved["storage"] = item.storage;
        saved["x"] = item.x;
        saved["y"] = item.y;
        saved["width"] = item.width;
        saved["height"] = item.height;
        saved["appraised"] = item.appraised;
        saved["wear_units"] = item.wearUnits;
        saved["appraisal_debug"] = item.appraisalDebug;
        saved["quick_slot"] = std::clamp(
            item.quickSlot,
            -1,
            kInventoryQuickSlotCount - 1);
        root["items"].push_back(std::move(saved));
    }
    ExhibitionUi_WriteState(root);
    const std::string serialized = root.dump(2) + "\n";
    std::fwrite(
        serialized.data(),
        1,
        serialized.size(),
        file);
    const bool writeFailed = std::ferror(file) != 0;
    std::fclose(file);
    if (writeFailed ||
        !MoveFileExW(
            temporary.c_str(),
            path.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        DeleteFileW(temporary.c_str());
        LogError(
            LogCategory_Persistence,
            L"SaveCommit",
            L"Inventory state save failed for %s.",
            path.c_str());
        return;
    }
    LogDebug(
        LogCategory_Persistence,
        L"SaveCommit",
        L"Inventory state saved to %s.",
        path.c_str());
}

enum class InventoryStateLoadResult
{
    Missing,
    Loaded,
    Corrupt,
};

InventoryStateLoadResult InventoryUi_LoadState()
{
    g_inventoryStarterGrantVersion = 0;
    const std::wstring path = InventoryUi_GetStatePath();
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
        const DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
        {
            LogInfo(LogCategory_Inventory, L"Inventory", L"Inventory state file not found; first-run initialization is required.");
            g_inventoryStateLoaded = true;
            return InventoryStateLoadResult::Missing;
        }
        LogError(
            LogCategory_Persistence,
            L"LoadOpen",
            L"Inventory state open failed with Win32 error %lu; leaving it untouched.",
            error);
        g_inventoryStateLoaded = false;
        return InventoryStateLoadResult::Corrupt;
    }
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) ||
        size.QuadPart <= 0 ||
        size.QuadPart > 4 * 1024 * 1024)
    {
        CloseHandle(file);
        g_inventoryStateLoaded = false;
        return InventoryStateLoadResult::Corrupt;
    }
    std::string content(static_cast<size_t>(size.QuadPart), '\0');
    DWORD read = 0;
    const bool readOk =
        ReadFile(file, content.data(), static_cast<DWORD>(content.size()), &read, nullptr) &&
        read == content.size();
    CloseHandle(file);
    if (!readOk)
    {
        LogError(
            LogCategory_Persistence,
            L"LoadRead",
            L"Inventory state read failed; leaving it untouched.");
        g_inventoryStateLoaded = false;
        return InventoryStateLoadResult::Corrupt;
    }

    const auto root = nlohmann::json::parse(
        content.begin(),
        content.end(),
        nullptr,
        false);
    if (root.is_discarded() ||
        !root.is_object() ||
        !root.contains("items") ||
        !root["items"].is_array())
    {
        LogError(
            LogCategory_Persistence,
            L"LoadParse",
            L"Inventory state JSON is invalid; leaving it untouched.");
        g_inventoryStateLoaded = false;
        return InventoryStateLoadResult::Corrupt;
    }
    const int schemaVersion = root.value("schema", 1);
    g_inventoryStarterGrantVersion =
        root.value("starter_grant_version", 0);
    g_inventoryDroppedDildo = InventoryDroppedDildoSnapshot{};
    if (root.contains("dropped_dildo") &&
        root["dropped_dildo"].is_object())
    {
        const auto& dropped = root["dropped_dildo"];
        if (dropped.contains("id") &&
            dropped["id"].is_string())
        {
            const int definition =
                InventoryUi_FindDefinitionById(
                    dropped["id"].get<std::string>().c_str());
            if (InventoryUi_IsDildoDefinition(definition))
            {
                g_inventoryDroppedDildo.active = true;
                g_inventoryDroppedDildo.itemId =
                    kEquipmentInventoryDefinitions[definition].id;
                g_inventoryDroppedDildo.quality = std::clamp(
                    dropped.value("quality", 0),
                    0,
                    kInventoryQualityCount - 1);
                g_inventoryDroppedDildo.wearUnits =
                    static_cast<unsigned int>(
                        std::clamp(
                            dropped.value("wear_units", 0),
                            0,
                            499999999));
                g_inventoryDroppedDildo.appraised =
                    dropped.value("appraised", false) &&
                    g_inventoryDroppedDildo.wearUnits != 0;
                g_inventoryDroppedDildo.stageType =
                    dropped.value("stage_type", -1);
                g_inventoryDroppedDildo.dropItemType =
                    dropped.value("drop_item_type", -1);
            }
        }
    }
    struct LoadedItem
    {
        int slot;
        int definition;
        int quality;
        int quickSlot;
        bool appraised;
        bool appraisalDebug;
        unsigned int wearUnits;
        int storage;
        int x;
        int y;
        int width;
        int height;
    };
    std::vector<LoadedItem> loaded;
    bool usedSlots[kInventoryItemSlotCount]{};
    const auto findSlot =
        [&](int preferredSlot, int definition)
        -> int
    {
        if (preferredSlot >= 0 &&
            preferredSlot < kInventoryItemSlotCount &&
            !usedSlots[preferredSlot])
        {
            return preferredSlot;
        }
        if (definition >= 0 &&
            definition < kEquipmentInventoryDefinitionCount &&
            !usedSlots[definition])
        {
            return definition;
        }
        for (int slot = kEquipmentInventoryDefinitionCount;
             slot < kInventoryItemSlotCount;
             ++slot)
        {
            if (!usedSlots[slot])
            {
                return slot;
            }
        }
        return -1;
    };
    for (const auto& saved : root["items"])
    {
        if (!saved.is_object() ||
            !saved.contains("id") ||
            !saved["id"].is_string())
        {
            continue;
        }
        const std::string id = saved["id"].get<std::string>();
        const int definition =
            InventoryUi_FindDefinitionById(id.c_str());
        int storage = saved.value("storage", -1);
        int quality = saved.value("quality", 0);
        const int x = saved.value("x", 0);
        const int y = saved.value("y", 0);
        const int width = saved.value("width", 1);
        const int height = saved.value("height", 1);
        const int quickSlot = std::clamp(
            saved.value("quick_slot", -1),
            -1,
            kInventoryQuickSlotCount - 1);
        // Schema 1 used 2 for equipped. Schema 2 introduced safe box at 2
        // and moved equipped to 3.
        if (schemaVersion < 2 && storage == 2)
        {
            storage = kInventoryStorageEquipped;
        }
        if (definition < 0 || storage < 0 ||
            storage > kInventoryStorageEquipped)
        {
            continue;
        }
        const int preferredSlot = saved.value("slot", -1);
        const int slot = findSlot(preferredSlot, definition);
        if (slot < 0)
        {
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Inventory state has no free slot for item %S.",
                id.c_str());
            continue;
        }
        usedSlots[slot] = true;
        bool appraised = saved.value("appraised", false);
        unsigned int wearUnits = static_cast<unsigned int>(
            std::clamp(
                saved.value("wear_units", 0),
                0,
                499999999));
        if (wearUnits == 0)
        {
            appraised = false;
        }
        loaded.push_back({
            slot,
            definition,
            std::clamp(quality, 0, kInventoryQualityCount - 1),
            quickSlot,
            appraised,
            saved.value("appraisal_debug", false),
            appraised ? wearUnits : 0,
            storage,
            x,
            y,
            std::max(1, width),
            std::max(1, height),
        });
    }

    for (auto& item : g_inventoryItems)
    {
        item = InventoryItemState{};
        item.storage = -1;
        item.quickSlot = -1;
    }
    g_inventoryStashScrollRow = std::clamp(
        root.value("stash_scroll_row", 0),
        0,
        InventoryUi_StashMaxScrollRow());
    for (const auto& saved : loaded)
    {
        auto& item = g_inventoryItems[saved.slot];
        item.definition = saved.definition;
        item.quality = saved.quality;
        item.quickSlot = saved.quickSlot;
        item.appraised = saved.appraised;
        item.appraisalDebug = saved.appraisalDebug;
        item.wearUnits = saved.wearUnits;
        item.storage = saved.storage;
        item.x = saved.x;
        item.y = saved.y;
        item.width = saved.width;
        item.height = saved.height;
        item.element = SceneUiElement{};
        item.content = SceneUiElement{};
        item.icon = SceneUiElement{};
        item.label = SceneUiElement{};
    }
    InventoryUi_PruneQuickSlotBindings();
    ExhibitionUi_LoadState(root);
    g_inventoryStateLoaded = true;
    LogInfo(LogCategory_Inventory, L"Inventory", L"Inventory state loaded with %d item record(s); starter grant version=%d.",
        static_cast<int>(loaded.size()),
        g_inventoryStarterGrantVersion);
    return InventoryStateLoadResult::Loaded;
}

std::wstring InventoryUi_EquipmentImagePath(const char* id, int quality)
{
    quality = std::clamp(quality, 0, kInventoryQualityCount - 1);
    return GetEscapeFromManakaSubdirectory(L"Assets") +
           L"\\crops\\" +
           kInventoryQualityDirectories[quality] +
           L"\\" +
           Utf8ToWide(id != nullptr ? id : "") +
           L".png";
}

void InventoryUi_ConfigureEquipmentIconScaling(SceneUiElement& image)
{
    if (image.gameObject == nullptr || image.texture == nullptr)
    {
        return;
    }
    auto* aspectFitter = GetSceneComponentByClass(
        image.gameObject,
        "UnityEngine.UI",
        "AspectRatioFitter");
    if (aspectFitter == nullptr)
    {
        aspectFitter = AddSceneComponentToGameObject(
            image.gameObject,
            "UnityEngine.UI",
            "AspectRatioFitter");
    }
    if (aspectFitter == nullptr)
    {
        return;
    }
    auto* textureClass = FindClass("UnityEngine", "Texture");
    auto* widthValue = Invoke(
        FindMethod(textureClass, "get_width", 0),
        image.texture,
        nullptr);
    auto* heightValue = Invoke(
        FindMethod(textureClass, "get_height", 0),
        image.texture,
        nullptr);
    if (widthValue == nullptr || heightValue == nullptr)
    {
        return;
    }
    const int width = *static_cast<int*>(
        il2cpp_object_unbox(widthValue));
    const int height = *static_cast<int*>(
        il2cpp_object_unbox(heightValue));
    if (width <= 0 || height <= 0)
    {
        return;
    }
    float aspectRatio =
        static_cast<float>(width) / static_cast<float>(height);
    int aspectMode = 3; // AspectMode.FitInParent
    void* aspectParameters[] = { &aspectRatio };
    void* modeParameters[] = { &aspectMode };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "AspectRatioFitter"),
            "set_aspectRatio",
            1),
        aspectFitter,
        aspectParameters);
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "AspectRatioFitter"),
            "set_aspectMode",
            1),
        aspectFitter,
        modeParameters);
}

bool InventoryUi_IsItemIconRotated(
    const InventoryItemState& item)
{
    if (item.definition < 0 ||
        item.definition >= kEquipmentInventoryDefinitionCount)
    {
        return false;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[item.definition];
    return definition.width != definition.height &&
        item.width == definition.height &&
        item.height == definition.width;
}

void InventoryUi_ApplyItemIconRotation(
    const InventoryItemState& item)
{
    if (item.icon.rect == nullptr ||
        item.definition < 0 ||
        item.definition >= kEquipmentInventoryDefinitionCount)
    {
        return;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[item.definition];
    const bool rotated = InventoryUi_IsItemIconRotated(item);
    if (!rotated)
    {
        SetTransformLocalEuler(item.icon.rect, 0.0f, 0.0f, 0.0f);
        SetSceneRect(item.icon.rect, 0.0f, 0.0f, 1.0f, 1.0f);
        return;
    }
    const float width =
        static_cast<float>(item.height) /
        static_cast<float>(item.width);
    const float height =
        static_cast<float>(item.width) /
        static_cast<float>(item.height);
    SetTransformLocalEuler(item.icon.rect, 0.0f, 0.0f, 90.0f);
    SetSceneRect(
        item.icon.rect,
        (1.0f - width) * 0.5f,
        (1.0f - height) * 0.5f,
        (1.0f + width) * 0.5f,
        (1.0f + height) * 0.5f);
}

void InventoryUi_ItemColor(
    const EquipmentInventoryDefinition& definition,
    int quality,
    float& red,
    float& green,
    float& blue)
{
    (void)definition;
    quality = std::clamp(quality, 0, kInventoryQualityCount - 1);
    red = kInventoryQualityColors[quality][0];
    green = kInventoryQualityColors[quality][1];
    blue = kInventoryQualityColors[quality][2];
}

float InventoryUi_BackpackCellPixels(
    float clientWidth,
    float clientHeight)
{
    const auto& layout =
        kInventoryStorageLayouts[kInventoryStorageBackpack];
    const float panelWidth = layout.panelMaxX - layout.panelMinX;
    const float panelHeight = layout.panelMaxY - layout.panelMinY;
    const float areaPixelsX =
        (layout.gridMaxX - layout.gridMinX) *
        panelWidth *
        clientWidth;
    const float areaPixelsY =
        (layout.gridMaxY - layout.gridMinY) *
        panelHeight *
        clientHeight;
    constexpr float slotGapCells = 0.14f;
    const int widestColumns =
        kInventoryBackpackTiers[kInventoryBackpackTierCount - 1].columns;
    const int tallestRows =
        kInventoryBackpackTiers[kInventoryBackpackTierCount - 1].rows;
    return std::max(
        1.0f,
        std::fmin(
            areaPixelsX /
                (static_cast<float>(widestColumns + 1) +
                 slotGapCells),
            areaPixelsY / static_cast<float>(tallestRows)));
}

void InventoryUi_CalculateGridMetrics(
    int storage,
    float clientWidth,
    float clientHeight,
    InventoryGridMetrics& metrics)
{
    if (storage < 0 || storage >= kInventoryGridStorageCount)
    {
        metrics = InventoryGridMetrics{};
        return;
    }
    const auto& layout = kInventoryStorageLayouts[storage];
    if (layout.columns <= 0 || layout.rows <= 0 ||
        layout.visibleRows <= 0)
    {
        metrics = InventoryGridMetrics{};
        return;
    }
    const float panelWidth = layout.panelMaxX - layout.panelMinX;
    const float panelHeight = layout.panelMaxY - layout.panelMinY;
    const float gridAreaMinX =
        layout.panelMinX + layout.gridMinX * panelWidth;
    const float gridAreaMinY =
        layout.panelMinY + layout.gridMinY * panelHeight;
    const float gridAreaMaxX =
        layout.panelMinX + layout.gridMaxX * panelWidth;
    const float gridAreaMaxY =
        layout.panelMinY + layout.gridMaxY * panelHeight;
    const float areaPixelsX =
        (gridAreaMaxX - gridAreaMinX) * clientWidth;
    const float areaPixelsY =
        (gridAreaMaxY - gridAreaMinY) * clientHeight;
    if (storage == kInventoryStorageBackpack)
    {
        metrics.cellPixels =
            InventoryUi_BackpackCellPixels(clientWidth, clientHeight);
        metrics.cellWidth = metrics.cellPixels / clientWidth;
        metrics.cellHeight = metrics.cellPixels / clientHeight;
        constexpr float slotGapCells = 0.14f;
        metrics.minX =
            gridAreaMinX +
            (1.0f + slotGapCells) * metrics.cellWidth;
        metrics.maxX =
            metrics.minX +
            static_cast<float>(layout.columns) * metrics.cellWidth;
        metrics.maxY = gridAreaMaxY;
        metrics.minY =
            metrics.maxY -
            static_cast<float>(layout.visibleRows) * metrics.cellHeight;
        return;
    }
    metrics.cellPixels =
        storage == kInventoryStorageSafeBox
            ? std::fmin(
                areaPixelsX / static_cast<float>(layout.columns),
                areaPixelsY /
                    static_cast<float>(layout.visibleRows))
            : std::fmin(
                areaPixelsX / static_cast<float>(layout.columns),
                areaPixelsY / static_cast<float>(layout.visibleRows));
    if (metrics.cellPixels < 1.0f)
    {
        metrics.cellPixels = 1.0f;
    }
    const float gridPixelsX =
        metrics.cellPixels * static_cast<float>(layout.columns);
    const float gridPixelsY =
        metrics.cellPixels * static_cast<float>(layout.visibleRows);
    const float gridCenterPixelsX =
        (gridAreaMinX + gridAreaMaxX) * 0.5f * clientWidth;
    const float gridCenterPixelsY =
        (gridAreaMinY + gridAreaMaxY) * 0.5f * clientHeight;
    metrics.minX =
        (gridCenterPixelsX - gridPixelsX * 0.5f) / clientWidth;
    metrics.maxX =
        (gridCenterPixelsX + gridPixelsX * 0.5f) / clientWidth;
    metrics.minY =
        (gridCenterPixelsY - gridPixelsY * 0.5f) / clientHeight;
    metrics.maxY =
        (gridCenterPixelsY + gridPixelsY * 0.5f) / clientHeight;
    metrics.cellWidth = metrics.cellPixels / clientWidth;
    metrics.cellHeight = metrics.cellPixels / clientHeight;
}

void InventoryUi_UpdateGridCell(const InventoryGridCell& cell)
{
    if (cell.element.rect == nullptr)
    {
        return;
    }
    InventoryGridMetrics metrics{};
    InventoryUi_CalculateGridMetrics(
        cell.storage,
        g_inventoryLayoutWidth,
        g_inventoryLayoutHeight,
        metrics);
    if (metrics.cellWidth <= 0.0f ||
        metrics.cellHeight <= 0.0f)
    {
        SetWorldGameObjectActive(cell.element.gameObject, false);
        return;
    }
    const auto& layout = kInventoryStorageLayouts[cell.storage];
    if (cell.column >= layout.columns ||
        cell.row >= layout.rows)
    {
        SetWorldGameObjectActive(cell.element.gameObject, false);
        return;
    }
    if (cell.storage == kInventoryStorageSafeBox &&
        !ExhibitionUi_IsSafeBoxCellActive(
            cell.column,
            cell.row))
    {
        SetWorldGameObjectActive(cell.element.gameObject, false);
        return;
    }
    int visibleRow = cell.row;
    if (cell.storage == kInventoryStorageStash)
    {
        visibleRow -= g_inventoryStashScrollRow;
    }
    if (visibleRow < 0 || visibleRow >= layout.visibleRows)
    {
        SetWorldGameObjectActive(cell.element.gameObject, false);
        return;
    }
    SetWorldGameObjectActive(cell.element.gameObject, true);
    const float gapX = 2.0f / g_inventoryLayoutWidth;
    const float gapY = 2.0f / g_inventoryLayoutHeight;
    const float cellMinX =
        metrics.minX + static_cast<float>(cell.column) * metrics.cellWidth;
    const float cellMaxX = cellMinX + metrics.cellWidth;
    const float cellMinY =
        metrics.maxY -
        static_cast<float>(visibleRow + 1) * metrics.cellHeight;
    const float cellMaxY = cellMinY + metrics.cellHeight;
    SetSceneRect(
        cell.element.rect,
        cellMinX + gapX,
        cellMinY + gapY,
        cellMaxX - gapX,
        cellMaxY - gapY);
    if (cell.storage == kInventoryStorageSafeBox)
    {
        static bool loggedSafeBoxCell[4]{};
        const int logIndex = std::clamp(
            cell.row * layout.columns + cell.column,
            0,
            3);
        if (!loggedSafeBoxCell[logIndex])
        {
            loggedSafeBoxCell[logIndex] = true;
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Safe-box cell %d,%d rect=(%.3f,%.3f)-(%.3f,%.3f) active=%d.",
                cell.column,
                cell.row,
                cellMinX + gapX,
                cellMinY + gapY,
                cellMaxX - gapX,
                cellMaxY - gapY,
                cell.element.gameObject != nullptr ? 1 : 0);
        }
    }
}

int InventoryUi_StashMaxScrollRow()
{
    const auto& layout = kInventoryStorageLayouts[kInventoryStorageStash];
    return std::max(0, layout.rows - layout.visibleRows);
}

void InventoryUi_UpdateItemRect(int itemIndex);

void InventoryUi_UpdateStashScrollbar()
{
    const auto& layout = kInventoryStorageLayouts[kInventoryStorageStash];
    constexpr float trackMinY = 0.10f;
    constexpr float trackMaxY = 0.86f;
    const float trackHeight = trackMaxY - trackMinY;
    const float visibleRatio =
        static_cast<float>(layout.visibleRows) /
        static_cast<float>(layout.rows);
    const float thumbHeight = trackHeight * visibleRatio;
    const int maxScroll = InventoryUi_StashMaxScrollRow();
    const float progress = maxScroll > 0
        ? static_cast<float>(g_inventoryStashScrollRow) /
            static_cast<float>(maxScroll)
        : 0.0f;
    const float thumbMinY =
        trackMaxY - thumbHeight -
        progress * (trackHeight - thumbHeight);
    constexpr float panelMinY = 0.055f;
    constexpr float panelMaxY = 0.884f;
    const float panelHeight = panelMaxY - panelMinY;
    g_inventoryStashThumbScreenMinY = panelMinY + thumbMinY * panelHeight;
    g_inventoryStashThumbScreenMaxY =
        panelMinY + (thumbMinY + thumbHeight) * panelHeight;
    SetSceneRect(
        g_inventoryStashScrollThumb.rect,
        0.956f,
        thumbMinY,
        0.994f,
        thumbMinY + thumbHeight);
    SetSceneGraphicColor(
        g_inventoryStashScrollThumb.image,
        0.64f,
        0.49f,
        0.29f,
        1.0f);
}

void InventoryUi_SetStashScrollRow(int row)
{
    const int clamped = std::clamp(
        row,
        0,
        InventoryUi_StashMaxScrollRow());
    if (clamped == g_inventoryStashScrollRow)
    {
        InventoryUi_UpdateStashScrollbar();
        return;
    }
    g_inventoryStashScrollRow = clamped;
    for (int index = 0;
         index < g_inventoryGridCellCounts[kInventoryStorageStash];
         ++index)
    {
        InventoryUi_UpdateGridCell(
            g_inventoryGridCells[kInventoryStorageStash][index]);
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (g_inventoryItems[index].storage == kInventoryStorageStash &&
            (!g_inventoryDragActive ||
             index != g_inventoryDraggingItem))
        {
            InventoryUi_UpdateItemRect(index);
        }
    }
    InventoryUi_UpdateStashScrollbar();
}

bool InventoryUi_StashScrollbarHit(
    float cursorX,
    float cursorY,
    bool& onThumb)
{
    constexpr float panelMinX = 0.616f;
    constexpr float panelMaxX = 0.975f;
    constexpr float panelMinY = 0.055f;
    constexpr float panelMaxY = 0.884f;
    const float panelWidth = panelMaxX - panelMinX;
    const float panelHeight = panelMaxY - panelMinY;
    const float minX = panelMinX + 0.956f * panelWidth;
    const float maxX = panelMinX + 0.994f * panelWidth;
    const float minY = panelMinY + 0.10f * panelHeight;
    const float maxY = panelMinY + 0.86f * panelHeight;
    if (cursorX < minX || cursorX > maxX ||
        cursorY < minY || cursorY > maxY)
    {
        return false;
    }
    onThumb =
        cursorY >= g_inventoryStashThumbScreenMinY &&
        cursorY <= g_inventoryStashThumbScreenMaxY;
    return true;
}

void InventoryUi_UpdateStashScrollFromCursor(float cursorY)
{
    constexpr float panelMinY = 0.055f;
    constexpr float panelMaxY = 0.884f;
    const float panelHeight = panelMaxY - panelMinY;
    const float trackMinY = panelMinY + 0.10f * panelHeight;
    const float trackMaxY = panelMinY + 0.86f * panelHeight;
    const float thumbHeight =
        g_inventoryStashThumbScreenMaxY -
        g_inventoryStashThumbScreenMinY;
    const float available = trackMaxY - trackMinY - thumbHeight;
    if (available <= 0.0f)
    {
        InventoryUi_SetStashScrollRow(0);
        return;
    }
    const float thumbMinY = std::clamp(
        cursorY - g_inventoryScrollDragOffsetY,
        trackMinY,
        trackMaxY - thumbHeight);
    const float progress =
        (trackMaxY - thumbHeight - thumbMinY) / available;
    const int maxScroll = InventoryUi_StashMaxScrollRow();
    InventoryUi_SetStashScrollRow(
        static_cast<int>(std::round(progress * maxScroll)));
}

bool InventoryUi_IsInStashColumn(float cursorX, float cursorY)
{
    const auto& layout =
        kInventoryStorageLayouts[kInventoryStorageStash];
    return cursorX >= layout.panelMinX &&
           cursorX <= layout.panelMaxX &&
           cursorY >= layout.panelMinY &&
           cursorY <= layout.panelMaxY;
}

void InventoryUi_QueueStashWheel(int wheelDelta)
{
    if (wheelDelta == 0)
    {
        return;
    }
    const int current =
        g_inventoryStashWheelDelta.load(std::memory_order_relaxed);
    const int combined = std::clamp(
        current + wheelDelta,
        -WHEEL_DELTA * 8,
        WHEEL_DELTA * 8);
    g_inventoryStashWheelDelta.store(
        combined,
        std::memory_order_relaxed);
}

void InventoryUi_TickStashWheel()
{
    const int wheelDelta =
        g_inventoryStashWheelDelta.exchange(0);
    if (wheelDelta == 0 ||
        g_extendedModeState.load() != kExtendedModeActiveStateValue ||
        g_inventoryActiveRoute != kInventoryRouteEquipment ||
        g_extendedCanvas == nullptr ||
        g_extendedActionListOpen ||
        g_extendedOriginalSystemMenuVisible.load() ||
        ItemContextMenu_ShouldBlockPointer())
    {
        return;
    }

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
    const float cursorX =
        static_cast<float>(cursor.x - clientRect.left) / width;
    const float cursorY =
        1.0f -
        static_cast<float>(cursor.y - clientRect.top) / height;
    if (!InventoryUi_IsInStashColumn(cursorX, cursorY))
    {
        return;
    }

    const int notches = std::max(
        1,
        std::abs(wheelDelta) / WHEEL_DELTA);
    const int direction = wheelDelta > 0 ? -1 : 1;
    InventoryUi_SetStashScrollRow(
        g_inventoryStashScrollRow + direction * notches);
    InventoryUi_SaveState();
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Warehouse stash wheel scroll: delta=%d row=%d.",
        wheelDelta,
        g_inventoryStashScrollRow);
}

bool InventoryUi_IsInEquipmentColumn(float cursorX, float cursorY)
{
    return cursorX >= 0.024f && cursorX <= 0.350f &&
           cursorY >= 0.055f && cursorY <= 0.884f;
}

bool InventoryUi_IsInBackpackColumn(float cursorX, float cursorY)
{
    const auto& layout =
        kInventoryStorageLayouts[kInventoryStorageBackpack];
    return cursorX >= layout.panelMinX &&
           cursorX <= layout.panelMaxX &&
           cursorY >= layout.panelMinY &&
           cursorY <= layout.panelMaxY;
}

bool InventoryUi_CanEquip(int itemIndex, int targetCategory);

int InventoryUi_FindAutoEquipCategory(int itemIndex)
{
    if (itemIndex < 0 ||
        itemIndex >= kInventoryItemSlotCount)
    {
        return -1;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[g_inventoryItems[itemIndex].definition];
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        if ((definition.categories &
             kInventoryEquipmentCategories[category].bit) != 0 &&
            InventoryUi_CanEquip(itemIndex, category))
        {
            return category;
        }
    }
    return -1;
}

bool InventoryUi_CanPlace(int itemIndex, int storage, int x, int y)
{
    if (storage < 0 || storage >= kInventoryGridStorageCount ||
        itemIndex < 0 || itemIndex >= kInventoryItemSlotCount)
    {
        return false;
    }
    const auto& layout = kInventoryStorageLayouts[storage];
    const auto& item = g_inventoryItems[itemIndex];
    if (x < 0 || y < 0 ||
        x + item.width > layout.columns ||
        y + item.height > layout.rows)
    {
        return false;
    }
    if (storage == kInventoryStorageSafeBox)
    {
        for (int row = y; row < y + item.height; ++row)
        {
            for (int column = x; column < x + item.width; ++column)
            {
                if (!ExhibitionUi_IsSafeBoxCellActive(column, row))
                {
                    return false;
                }
            }
        }
    }
    for (int otherIndex = 0;
         otherIndex < kInventoryItemSlotCount;
         ++otherIndex)
    {
        if (otherIndex == itemIndex)
        {
            continue;
        }
        const auto& other = g_inventoryItems[otherIndex];
        if (other.storage != storage)
        {
            continue;
        }
        const bool separate =
            x + item.width <= other.x ||
            other.x + other.width <= x ||
            y + item.height <= other.y ||
            other.y + other.height <= y;
        if (!separate)
        {
            return false;
        }
    }
    return true;
}

int InventoryUi_FindEquippedItem(int categoryIndex)
{
    if (categoryIndex == kInventoryBackpackCategoryIndex)
    {
        for (int index = 0;
             index < kInventoryItemSlotCount;
             ++index)
        {
            if (g_inventoryItems[index].storage ==
                    kInventoryStorageEquipped &&
                InventoryUi_ItemSlotIsBackpack(index))
            {
                return index;
            }
        }
        return -1;
    }
    if (categoryIndex < 0 ||
        categoryIndex >= kInventoryEquipmentCategoryCount)
    {
        return -1;
    }
    const uint32_t categoryBit =
        kInventoryEquipmentCategories[categoryIndex].bit;
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        const auto& item = g_inventoryItems[index];
        if (item.storage == kInventoryStorageEquipped &&
            (kEquipmentInventoryDefinitions[item.definition].categories &
             categoryBit) != 0)
        {
            return index;
        }
    }
    return -1;
}

bool InventoryUi_CanEquip(int itemIndex, int targetCategory)
{
    if (targetCategory == kInventoryBackpackCategoryIndex)
    {
        if (itemIndex < 0 ||
            itemIndex >= kInventoryItemSlotCount ||
            !InventoryUi_ItemSlotIsBackpack(itemIndex))
        {
            return false;
        }
        const int occupant = InventoryUi_FindEquippedItem(
            kInventoryBackpackCategoryIndex);
        if (occupant >= 0 && occupant != itemIndex)
        {
            return false;
        }
        const int tier = InventoryUi_ItemSlotBackpackTier(itemIndex);
        return tier >= 0 &&
            InventoryUi_BackpackItemsFitTier(tier, itemIndex);
    }
    if (itemIndex < 0 ||
        itemIndex >= kInventoryItemSlotCount ||
        targetCategory < 0 ||
        targetCategory >= kInventoryEquipmentCategoryCount)
    {
        return false;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[g_inventoryItems[itemIndex].definition];
    const uint32_t targetBit =
        kInventoryEquipmentCategories[targetCategory].bit;
    if ((definition.categories & targetBit) == 0)
    {
        return false;
    }
    const uint32_t equipmentBits =
        definition.categories & kInventoryEquipmentCategoryMask;
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        if ((equipmentBits &
             kInventoryEquipmentCategories[category].bit) == 0)
        {
            continue;
        }
        const int occupant = InventoryUi_FindEquippedItem(category);
        if (occupant >= 0 && occupant != itemIndex)
        {
            return false;
        }
    }
    return equipmentBits != 0;
}

void InventoryUi_UpdateItemRect(int itemIndex)
{
    if (itemIndex < 0 ||
        itemIndex >= kInventoryItemSlotCount)
    {
        return;
    }
    auto& item = g_inventoryItems[itemIndex];
    if (item.element.rect == nullptr)
    {
        return;
    }
    if (item.storage == kInventoryStorageEquipped)
    {
        SetWorldGameObjectActive(item.element.gameObject, false);
        return;
    }
    if (item.storage < 0 || item.storage >= kInventoryGridStorageCount)
    {
        SetWorldGameObjectActive(item.element.gameObject, false);
        return;
    }

    SetWorldGameObjectActive(item.element.gameObject, true);
    InventoryGridMetrics metrics{};
    InventoryUi_CalculateGridMetrics(
        item.storage,
        g_inventoryLayoutWidth,
        g_inventoryLayoutHeight,
        metrics);
    if (metrics.cellWidth <= 0.0f ||
        metrics.cellHeight <= 0.0f)
    {
        SetWorldGameObjectActive(item.element.gameObject, false);
        return;
    }
    int displayedY = item.y;
    int displayedHeight = item.height;
    float topCropFraction = 0.0f;
    float bottomCropFraction = 0.0f;
    if (item.storage == kInventoryStorageStash)
    {
        const auto& layout = kInventoryStorageLayouts[kInventoryStorageStash];
        const int firstVisible = g_inventoryStashScrollRow;
        const int lastVisible = firstVisible + layout.visibleRows;
        if (item.y >= lastVisible || item.y + item.height <= firstVisible)
        {
            SetWorldGameObjectActive(item.element.gameObject, false);
            return;
        }
        const int firstDrawn = std::max(item.y, firstVisible);
        const int lastDrawn = std::min(item.y + item.height, lastVisible);
        displayedY = firstDrawn - firstVisible;
        displayedHeight = lastDrawn - firstDrawn;
        const int croppedFromTop = firstDrawn - item.y;
        const int croppedFromBottom =
            item.y + item.height - lastDrawn;
        topCropFraction =
            static_cast<float>(croppedFromTop) /
            static_cast<float>(displayedHeight);
        bottomCropFraction =
            static_cast<float>(croppedFromBottom) /
            static_cast<float>(displayedHeight);
    }
    const float gapX = 2.0f / g_inventoryLayoutWidth;
    const float gapY = 2.0f / g_inventoryLayoutHeight;
    item.screenMinX =
        metrics.minX +
        static_cast<float>(item.x) * metrics.cellWidth +
        gapX;
    item.screenMaxX =
        metrics.minX +
        static_cast<float>(item.x + item.width) * metrics.cellWidth -
        gapX;
    item.screenMinY =
        metrics.maxY -
        static_cast<float>(displayedY + displayedHeight) * metrics.cellHeight +
        gapY;
    item.screenMaxY =
        metrics.maxY -
        static_cast<float>(displayedY) * metrics.cellHeight -
        gapY;
    SetSceneRect(
        item.element.rect,
        item.screenMinX,
        item.screenMinY,
        item.screenMaxX,
        item.screenMaxY);
    if (item.content.rect != nullptr)
    {
        SetSceneRect(
            item.content.rect,
            0.0f,
            -bottomCropFraction,
            1.0f,
            1.0f + topCropFraction);
    }
    InventoryUi_ApplyItemIconRotation(item);
    if (item.appraisalBadgeCircle.rect != nullptr)
    {
        const float tilePixelsWidth = std::max(
            1.0f,
            (item.screenMaxX - item.screenMinX) *
                g_inventoryLayoutWidth);
        const float tilePixelsHeight = std::max(
            1.0f,
            (item.screenMaxY - item.screenMinY) *
                g_inventoryLayoutHeight);
        const float diameterPixels = std::clamp(
            tilePixelsWidth * 0.32f,
            18.0f,
            26.0f);
        const float insetPixels = 1.0f;
        const float badgeWidth =
            diameterPixels / tilePixelsWidth;
        const float badgeHeight =
            diameterPixels / tilePixelsHeight;
        const float insetX =
            insetPixels / tilePixelsWidth;
        const float insetY =
            insetPixels / tilePixelsHeight;
        SetSceneRect(
            item.appraisalBadgeCircle.rect,
            insetX,
            1.0f - insetY - badgeHeight,
            insetX + badgeWidth,
            1.0f - insetY);
    }

    const auto& definition =
        kEquipmentInventoryDefinitions[item.definition];
    SetSceneText(
        item.label.text,
        definition.name,
        12.0f,
        260,
        0.94f,
        0.95f,
        0.97f,
        1.0f);
    const bool showBadge = item.appraised;
    if (item.appraisalBadgeCircle.text != nullptr)
    {
        SetWorldGameObjectActive(
            item.appraisalBadgeCircle.gameObject,
            showBadge);
    }
    if (item.appraisalBadgeText.text != nullptr)
    {
        SetWorldGameObjectActive(
            item.appraisalBadgeText.gameObject,
            showBadge);
    }
    if (showBadge)
    {
        const auto tier =
            InventoryUi_GetAppraisalTier(item.wearUnits);
        float badgeRed = 0.0f;
        float badgeGreen = 0.0f;
        float badgeBlue = 0.0f;
        InventoryUi_AppraisalTierColor(
            tier,
            badgeRed,
            badgeGreen,
            badgeBlue);
        SetSceneText(
            item.appraisalBadgeCircle.text,
            "●",
            26.0f,
            514,
            badgeRed,
            badgeGreen,
            badgeBlue,
            1.0f);
        SetSceneText(
            item.appraisalBadgeText.text,
            WideToUtf8(
                InventoryUi_AppraisalTierLabel(tier)).c_str(),
            12.0f,
            514,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
    }
}

void InventoryUi_RefreshItems()
{
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        InventoryUi_UpdateItemRect(index);
    }
}

void InventoryUi_SetStatus(const char* text)
{
    if (g_inventoryStatusLabel == nullptr)
    {
        return;
    }
    SetSceneText(
        g_inventoryStatusLabel,
        text,
        14.0f,
        514,
        0.78f,
        0.83f,
        0.90f,
        1.0f);
}

void InventoryUi_Warn(const char* text)
{
    InventoryUi_SetStatus(text);
    MessageBeep(MB_ICONWARNING);
}

void InventoryUi_UpdateCategoryScreenRect(int categoryIndex)
{
    if (categoryIndex < 0 ||
        categoryIndex >= kInventoryEquipmentCategoryCount)
    {
        return;
    }
    constexpr float panelMinX = 0.024f;
    constexpr float panelMinY = 0.055f;
    constexpr float panelMaxX = 0.350f;
    constexpr float panelMaxY = 0.884f;
    constexpr float slotMinX[kInventoryEquipmentCategoryCount] = {
        0.570f, 0.705f, 0.840f,
        0.570f, 0.705f, 0.840f,
        0.570f, 0.705f, 0.840f,
    };
    constexpr float slotMinY[kInventoryEquipmentCategoryCount] = {
        0.665f, 0.665f, 0.665f,
        0.490f, 0.490f, 0.490f,
        0.315f, 0.315f, 0.315f,
    };
    constexpr float slotWidth = 0.125f;
    constexpr float slotHeight = 0.145f;
    const float panelWidth = panelMaxX - panelMinX;
    const float panelHeight = panelMaxY - panelMinY;
    auto& slot = g_inventoryEquipmentSlots[categoryIndex];
    slot.screenMinX = panelMinX + slotMinX[categoryIndex] * panelWidth;
    slot.screenMaxX =
        panelMinX + (slotMinX[categoryIndex] + slotWidth) * panelWidth;
    slot.screenMinY = panelMinY + slotMinY[categoryIndex] * panelHeight;
    slot.screenMaxY =
        panelMinY + (slotMinY[categoryIndex] + slotHeight) * panelHeight;
}

void InventoryUi_GetQuickSlotLocalRect(
    int index,
    float& minX,
    float& minY,
    float& maxX,
    float& maxY)
{
    constexpr float panelMinX = 0.024f;
    constexpr float panelMinY = 0.055f;
    constexpr float panelMaxX = 0.350f;
    constexpr float panelMaxY = 0.884f;
    const float panelWidth = panelMaxX - panelMinX;
    const float panelHeight = panelMaxY - panelMinY;
    const float clientWidth = std::max(
        1.0f,
        g_inventoryLayoutWidth);
    const float clientHeight = std::max(
        1.0f,
        g_inventoryLayoutHeight);
    const float squarePixels = std::max(
        64.0f,
        clientHeight * 0.078f);
    const float gapPixels = std::max(
        6.0f,
        clientHeight * 0.008f);
    const float groupWidth =
        squarePixels * kInventoryQuickSlotCount +
        gapPixels * (kInventoryQuickSlotCount - 1);
    constexpr float equipmentColumnSlotMinX = 0.570f;
    constexpr float equipmentColumnSlotWidth = 0.125f;
    const float equipmentColumnCenterScreenX =
        panelMinX +
        (equipmentColumnSlotMinX +
         equipmentColumnSlotWidth * 0.5f) *
            panelWidth;
    const float screenMinX =
        equipmentColumnCenterScreenX * clientWidth -
        groupWidth * 0.5f +
        index * (squarePixels + gapPixels);
    const float screenMinY = clientHeight * 0.135f;
    minX =
        (screenMinX / clientWidth - panelMinX) /
        panelWidth;
    maxX = minX + squarePixels / clientWidth / panelWidth;
    minY =
        (screenMinY / clientHeight - panelMinY) /
        panelHeight;
    maxY = minY + squarePixels / clientHeight / panelHeight;
}

void InventoryUi_UpdateQuickSlotScreenRect(int index)
{
    if (index < 0 || index >= kInventoryQuickSlotCount)
    {
        return;
    }
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    InventoryUi_GetQuickSlotLocalRect(
        index,
        minX,
        minY,
        maxX,
        maxY);
    auto& slot = g_inventoryQuickSlots[index];
    constexpr float panelMinX = 0.024f;
    constexpr float panelMinY = 0.055f;
    constexpr float panelMaxX = 0.350f;
    constexpr float panelMaxY = 0.884f;
    const float panelWidth = panelMaxX - panelMinX;
    const float panelHeight = panelMaxY - panelMinY;
    slot.screenMinX = panelMinX + minX * panelWidth;
    slot.screenMaxX =
        panelMinX + maxX * panelWidth;
    slot.screenMinY = panelMinY + minY * panelHeight;
    slot.screenMaxY = panelMinY + maxY * panelHeight;
    if (slot.panel.rect != nullptr)
    {
        SetSceneRect(
            slot.panel.rect,
            minX,
            minY,
            maxX,
            maxY);
    }
}

void InventoryUi_UpdateEquipmentSlotHighlights()
{
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        auto& slot = g_inventoryEquipmentSlots[category];
        const int occupiedBy = InventoryUi_FindEquippedItem(category);
        float red = 0.060f;
        float green = 0.068f;
        float blue = 0.080f;
        (void)occupiedBy;

        bool eligible = false;
        if (g_inventoryDragActive && g_inventoryDraggingItem >= 0)
        {
            const auto& definition =
                kEquipmentInventoryDefinitions[
                    g_inventoryItems[g_inventoryDraggingItem].definition];
            eligible =
                (definition.categories &
                 kInventoryEquipmentCategories[category].bit) != 0;
        }
        const bool hovered =
            g_inventoryDragActive &&
            category == g_inventoryHoveredEquipmentSlot;
        if (eligible || hovered)
        {
            const bool canEquip = InventoryUi_CanEquip(
                g_inventoryDraggingItem,
                category);
            red = canEquip ? (hovered ? 0.24f : 0.16f) : 0.50f;
            green = canEquip ? (hovered ? 0.56f : 0.42f) : 0.10f;
            blue = canEquip ? (hovered ? 0.25f : 0.16f) : 0.10f;
        }
        SetSceneGraphicColor(
            slot.panel.image,
            red,
            green,
            blue,
            0.98f);
    }

    auto& backpackSlot = g_inventoryBackpackSlot;
    if (backpackSlot.panel.image != nullptr)
    {
        const bool eligible =
            g_inventoryDragActive &&
            g_inventoryDraggingItem >= 0 &&
        InventoryUi_ItemSlotIsBackpack(
                g_inventoryDraggingItem);
        const bool hovered =
            g_inventoryDragActive &&
            g_inventoryHoveredEquipmentSlot ==
                kInventoryBackpackCategoryIndex;
        float red = 0.060f;
        float green = 0.068f;
        float blue = 0.080f;
        if (eligible || hovered)
        {
            const bool canEquip = InventoryUi_CanEquip(
                g_inventoryDraggingItem,
                kInventoryBackpackCategoryIndex);
            red = canEquip ? (hovered ? 0.24f : 0.16f) : 0.50f;
            green = canEquip ? (hovered ? 0.56f : 0.42f) : 0.10f;
            blue = canEquip ? (hovered ? 0.25f : 0.16f) : 0.10f;
        }
        SetSceneGraphicColor(
            backpackSlot.panel.image,
            red,
            green,
            blue,
            0.98f);
    }
}

void InventoryUi_UpdateEquipmentSlots()
{
    int occupiedCount = 0;
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        auto& slot = g_inventoryEquipmentSlots[category];
        const int itemIndex = InventoryUi_FindEquippedItem(category);
        const bool occupied = itemIndex >= 0;
        if (occupied)
        {
            ++occupiedCount;
            const auto& item = g_inventoryItems[itemIndex];
            const auto& definition =
                kEquipmentInventoryDefinitions[item.definition];
            SetWorldGameObjectActive(slot.icon.gameObject, true);
            if (slot.currentItemId != Utf8ToWide(definition.id) ||
                slot.currentQuality != item.quality)
            {
                slot.currentItemId = Utf8ToWide(definition.id);
                slot.currentQuality = item.quality;
                slot.icon.texture = LoadScenePngTexture(
                    InventoryUi_EquipmentImagePath(
                        definition.id,
                        item.quality));
            }
            SetSceneRawImageTexture(
                slot.icon.rawImage,
                slot.icon.texture);
            InventoryUi_ConfigureEquipmentIconScaling(slot.icon);
            SetSceneText(
                slot.label.text,
                definition.name,
                11.0f,
                260,
                1.0f,
                0.95f,
                0.82f,
                1.0f);
        }
        else
        {
            slot.currentItemId.clear();
            slot.currentQuality = -1;
            SetWorldGameObjectActive(slot.icon.gameObject, false);
            char label[96]{};
            std::snprintf(
                label,
                sizeof(label),
                "%s\n空",
                kInventoryEquipmentCategories[category].label);
            SetSceneText(
                slot.label.text,
                label,
                10.0f,
                514,
                0.72f,
                0.77f,
                0.83f,
                1.0f);
        }
        bool eligibleForDraggedItem = false;
        if (g_inventoryDragActive &&
            g_inventoryDraggingItem >= 0)
        {
            const auto& draggedDefinition =
                kEquipmentInventoryDefinitions[
                    g_inventoryItems[g_inventoryDraggingItem].definition];
            eligibleForDraggedItem =
                (draggedDefinition.categories &
                 kInventoryEquipmentCategories[category].bit) != 0;
        }
        const bool hovered =
            category == g_inventoryHoveredEquipmentSlot &&
            g_inventoryDragActive;
        (void)eligibleForDraggedItem;
        (void)hovered;
    }

    auto& backpackSlot = g_inventoryBackpackSlot;
    const int backpackItemIndex = InventoryUi_FindEquippedItem(
        kInventoryBackpackCategoryIndex);
    if (backpackItemIndex >= 0)
    {
        const auto& backpackItem =
            g_inventoryItems[backpackItemIndex];
        const auto& definition =
            kEquipmentInventoryDefinitions[backpackItem.definition];
        SetWorldGameObjectActive(backpackSlot.icon.gameObject, true);
        if (backpackSlot.currentItemId != Utf8ToWide(definition.id) ||
            backpackSlot.currentQuality != backpackItem.quality)
        {
            backpackSlot.currentItemId = Utf8ToWide(definition.id);
            backpackSlot.currentQuality = backpackItem.quality;
            backpackSlot.icon.texture = LoadScenePngTexture(
                InventoryUi_EquipmentImagePath(
                    definition.id,
                    backpackItem.quality));
        }
        SetSceneRawImageTexture(
            backpackSlot.icon.rawImage,
            backpackSlot.icon.texture);
        SetSceneText(
            backpackSlot.label.text,
            definition.name,
            11.0f,
            260,
            1.0f,
            0.95f,
            0.82f,
            1.0f);
    }
    else
    {
        backpackSlot.currentItemId.clear();
        backpackSlot.currentQuality = -1;
        SetWorldGameObjectActive(backpackSlot.icon.gameObject, false);
        SetSceneText(
            backpackSlot.label.text,
            "装备背包\n空",
            11.0f,
            514,
            0.72f,
            0.77f,
            0.83f,
            1.0f);
    }
    InventoryUi_UpdateEquipmentSlotHighlights();
    if (g_inventoryStatsLabel != nullptr)
    {
        char countLabel[128]{};
        std::snprintf(
            countLabel,
            sizeof(countLabel),
            "已装备 %d / %d 分类",
            occupiedCount,
            kInventoryEquipmentCategoryCount);
        SetSceneText(
            g_inventoryStatsLabel,
            countLabel,
            12.0f,
            257,
            0.77f,
            0.80f,
            0.84f,
            1.0f);
    }
}

void InventoryUi_UpdateQuickSlots()
{
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        auto& slot = g_inventoryQuickSlots[index];
        if (InventoryUi_CanAssignQuickSlot(slot.itemIndex))
        {
            const auto& item = g_inventoryItems[slot.itemIndex];
            slot.definition = item.definition;
            slot.quality = item.quality;
            const auto& definition =
                kEquipmentInventoryDefinitions[item.definition];
            const int quality = std::clamp(
                item.quality,
                0,
                kInventoryQualityCount - 1);
            SetWorldGameObjectActive(slot.icon.gameObject, true);
            if (slot.currentItemId != Utf8ToWide(definition.id) ||
                slot.currentQuality != quality)
            {
                slot.currentItemId = Utf8ToWide(definition.id);
                slot.currentQuality = quality;
                slot.icon.texture = LoadScenePngTexture(
                    InventoryUi_EquipmentImagePath(
                        definition.id,
                        quality));
            }
            SetSceneRawImageTexture(
                slot.icon.rawImage,
                slot.icon.texture);
            SetSceneText(
                slot.label.text,
                definition.name,
                9.0f,
                260,
                0.98f,
                0.95f,
                0.88f,
                1.0f);
        }
        else
        {
            if (slot.itemIndex >= 0)
            {
                InventoryUi_BindQuickSlot(index, -1);
            }
            slot.currentItemId.clear();
            slot.currentQuality = -1;
            slot.definition = -1;
            slot.quality = -1;
            SetWorldGameObjectActive(slot.icon.gameObject, false);
            SetSceneText(
                slot.label.text,
                "空",
                10.0f,
                514,
                0.62f,
                0.67f,
                0.72f,
                1.0f);
        }

        const bool hovered =
            g_inventoryDragActive &&
            g_inventoryHoveredQuickSlot == index;
        const bool valid =
            hovered &&
            InventoryUi_CanAssignQuickSlot(g_inventoryDraggingItem);
        if (hovered)
        {
            SetSceneGraphicColor(
                slot.panel.image,
                valid ? 0.08f : 0.42f,
                valid ? 0.40f : 0.08f,
                valid ? 0.18f : 0.07f,
                0.98f);
        }
        else
        {
            const int quality = std::clamp(
                slot.quality,
                0,
                kInventoryQualityCount - 1);
            const auto& tint = kInventoryQualityColors[quality];
            SetSceneGraphicColor(
                slot.panel.image,
                0.025f + tint[0] * 0.16f,
                0.030f + tint[1] * 0.16f,
                0.042f + tint[2] * 0.16f,
                0.98f);
        }
    }
}

void InventoryUi_RefreshCharacterModel()
{
    const char* itemIds[kInventoryEquipmentCategoryCount + 1]{};
    int itemCount = 0;
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (g_inventoryItems[index].storage != kInventoryStorageEquipped)
        {
            continue;
        }
        const char* id =
            kEquipmentInventoryDefinitions[g_inventoryItems[index].definition].id;
        bool duplicate = false;
        for (int existing = 0; existing < itemCount; ++existing)
        {
            if (itemIds[existing] == id)
            {
                duplicate = true;
                break;
            }
        }
        if (!duplicate &&
            itemCount < kInventoryEquipmentCategoryCount + 1)
        {
            itemIds[itemCount++] = id;
        }
    }
    EquipmentPreviewUi_ApplyOutfitToGameplayAvatar(itemIds, itemCount);
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Inventory character model refresh completed with %d outfit item(s).",
        itemCount);
}

bool InventoryUi_IsCoatEquipped()
{
    const int coatDefinition = InventoryUi_FindDefinitionById(
        "211-clothes-001-mclothesrecordcoat");
    return coatDefinition >= 0 &&
        coatDefinition < kEquipmentInventoryDefinitionCount &&
        g_inventoryItems[coatDefinition].storage ==
            kInventoryStorageEquipped;
}

void InventoryUi_RefreshEquipment()
{
    if (g_inventoryEquipmentPage != nullptr)
    {
        InventoryUi_ApplyBackpackCapacity();
        InventoryUi_ApplySafeBoxCapacity();
        InventoryUi_RefreshItems();
        InventoryUi_UpdateEquipmentSlots();
        InventoryUi_UpdateQuickSlots();
    }
    InventoryUi_RefreshCharacterModel();
}

void InventoryUi_UpdateDynamicLayout()
{
    float width = g_inventoryLayoutWidth;
    float height = g_inventoryLayoutHeight;
    if (!InventoryUi_ReadClientSize(width, height))
    {
        return;
    }
    const bool changed =
        !g_inventoryLayoutReady ||
        std::fabs(width - g_inventoryLayoutWidth) > 0.25f ||
        std::fabs(height - g_inventoryLayoutHeight) > 0.25f;
    if (!changed)
    {
        return;
    }
    g_inventoryLayoutWidth = width;
    g_inventoryLayoutHeight = height;
    g_inventoryLayoutReady = true;
    InventoryUi_ApplySafeBoxCapacity();
    for (int storage = 0;
         storage < kInventoryGridStorageCount;
         ++storage)
    {
        for (int index = 0;
             index < g_inventoryGridCellCounts[storage];
             ++index)
        {
            InventoryUi_UpdateGridCell(g_inventoryGridCells[storage][index]);
        }
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (!g_inventoryDragActive || index != g_inventoryDraggingItem)
        {
            InventoryUi_UpdateItemRect(index);
        }
    }
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        InventoryUi_UpdateCategoryScreenRect(category);
    }
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        InventoryUi_UpdateQuickSlotScreenRect(index);
    }
    InventoryUi_UpdateStashScrollbar();
    ExhibitionUi_UpdateLayout();
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Inventory square-grid layout adapted to %.0fx%.0f.",
        width,
        height);
}

void InventoryUi_CreateGrid(
    int storage,
    Il2CppObject* equipmentTransform)
{
    if (storage < 0 || storage >= kInventoryGridStorageCount)
    {
        return;
    }
    const auto& layout = kInventoryStorageLayouts[storage];
    const int columns = storage == kInventoryStorageBackpack
        ? kInventoryBackpackTiers[kInventoryBackpackTierCount - 1].columns
        : (storage == kInventoryStorageSafeBox ? 3 : layout.columns);
    const int rows = storage == kInventoryStorageBackpack
        ? kInventoryBackpackTiers[kInventoryBackpackTierCount - 1].rows
        : (storage == kInventoryStorageSafeBox ? 4 : layout.rows);
    g_inventoryGridCellCounts[storage] = 0;
    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            if (g_inventoryGridCellCounts[storage] >= kInventoryMaxGridCells)
            {
                return;
            }
            char name[64]{};
            std::snprintf(
                name,
                sizeof(name),
                "InventoryCell_%d_%d_%d",
                storage,
                column,
                row);
            auto& cell =
                g_inventoryGridCells[storage]
                    [g_inventoryGridCellCounts[storage]++];
            cell.storage = storage;
            cell.column = column;
            cell.row = row;
            cell.element = CreateScenePanel(
                name,
                equipmentTransform,
                0.0f,
                0.0f,
                0.001f,
                0.001f,
                0.095f,
                0.108f,
                0.130f,
                0.94f);
            InventoryUi_UpdateGridCell(cell);
        }
    }
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Inventory grid storage=%d created with %d cell(s), grid=%dx%d.",
        storage,
        g_inventoryGridCellCounts[storage],
        columns,
        rows);
}

void InventoryUi_SetBackpackSlotGeometry()
{
    const auto& layout =
        kInventoryStorageLayouts[kInventoryStorageBackpack];
    const float panelWidth = layout.panelMaxX - layout.panelMinX;
    const float panelHeight = layout.panelMaxY - layout.panelMinY;
    constexpr float slotGapCells = 0.14f;
    const float cellPixels = InventoryUi_BackpackCellPixels(
        g_inventoryLayoutWidth,
        g_inventoryLayoutHeight);
    const float cellWidth = cellPixels / g_inventoryLayoutWidth;
    const float cellHeight = cellPixels / g_inventoryLayoutHeight;
    const float slotLocalMinX = layout.gridMinX;
    const float slotLocalMaxX =
        slotLocalMinX + cellWidth / panelWidth;
    const float slotLocalMaxY = layout.gridMaxY;
    const float slotLocalMinY =
        slotLocalMaxY - cellHeight / panelHeight;
    g_inventoryBackpackSlot.screenMinX =
        layout.panelMinX + slotLocalMinX * panelWidth;
    g_inventoryBackpackSlot.screenMaxX =
        layout.panelMinX + slotLocalMaxX * panelWidth;
    g_inventoryBackpackSlot.screenMaxY =
        layout.panelMinY + slotLocalMaxY * panelHeight;
    g_inventoryBackpackSlot.screenMinY =
        layout.panelMinY + slotLocalMinY * panelHeight;
    SetSceneRect(
        g_inventoryBackpackSlot.panel.rect,
        slotLocalMinX,
        slotLocalMinY,
        slotLocalMaxX,
        slotLocalMaxY);
}

void InventoryUi_ApplyBackpackCapacity()
{
    auto& layout =
        kInventoryStorageLayouts[kInventoryStorageBackpack];
    const int previousColumns = layout.columns;
    const int previousRows = layout.rows;
    const int tier = InventoryUi_GetEquippedBackpackTier();
    layout.columns = tier >= 0
        ? kInventoryBackpackTiers[tier].columns
        : 0;
    layout.rows = tier >= 0
        ? kInventoryBackpackTiers[tier].rows
        : 0;
    layout.visibleRows = layout.rows;
    InventoryUi_SetBackpackSlotGeometry();

    for (int index = 0;
         index < g_inventoryGridCellCounts[kInventoryStorageBackpack];
         ++index)
    {
        InventoryUi_UpdateGridCell(
            g_inventoryGridCells[kInventoryStorageBackpack][index]);
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        InventoryUi_UpdateItemRect(index);
    }

    if (g_inventoryBackpackSizeLabel != nullptr)
    {
        char text[160]{};
        if (tier >= 0)
        {
            std::snprintf(
                text,
                sizeof(text),
                "%s  ·  %d × %d  ·  %d 格",
                kInventoryBackpackTiers[tier].name,
                layout.columns,
                layout.rows,
                layout.columns * layout.rows);
        }
        else
        {
            std::snprintf(
                text,
                sizeof(text),
                "未装备背包  ·  0 × 0  ·  0 格");
        }
        SetSceneText(
            g_inventoryBackpackSizeLabel,
            text,
            12.0f,
            514,
            0.71f,
            0.75f,
            0.80f,
            1.0f);
    }
    if (layout.columns != previousColumns ||
        layout.rows != previousRows)
    {
        LogInfo(
            LogCategory_Inventory,
            L"BackpackCapacity",
            L"Backpack capacity changed %dx%d -> %dx%d tier=%d.",
            previousColumns,
            previousRows,
            layout.columns,
            layout.rows,
            tier);
    }
}

void InventoryUi_NormalizeSafeBoxItems()
{
    std::vector<int> safeItems;
    safeItems.reserve(kInventoryItemSlotCount);
    bool invalid = false;
    for (int index = 0; index < kInventoryItemSlotCount; ++index)
    {
        const auto& item = g_inventoryItems[index];
        if (item.storage != kInventoryStorageSafeBox)
        {
            continue;
        }
        safeItems.push_back(index);
        if (item.x < 0 || item.y < 0 ||
            item.x + item.width >
                kInventoryStorageLayouts[kInventoryStorageSafeBox].columns ||
            item.y + item.height >
                kInventoryStorageLayouts[kInventoryStorageSafeBox].rows)
        {
            invalid = true;
            continue;
        }
        for (int row = item.y; row < item.y + item.height; ++row)
        {
            for (int column = item.x;
                 column < item.x + item.width;
                 ++column)
            {
                if (!ExhibitionUi_IsSafeBoxCellActive(column, row))
                {
                    invalid = true;
                }
            }
        }
    }
    for (size_t left = 0; left < safeItems.size() && !invalid; ++left)
    {
        for (size_t right = left + 1; right < safeItems.size(); ++right)
        {
            const auto& a = g_inventoryItems[safeItems[left]];
            const auto& b = g_inventoryItems[safeItems[right]];
            const bool separate =
                a.x + a.width <= b.x ||
                b.x + b.width <= a.x ||
                a.y + a.height <= b.y ||
                b.y + b.height <= a.y;
            if (!separate)
            {
                invalid = true;
                break;
            }
        }
    }
    if (!invalid || safeItems.empty())
    {
        return;
    }
    for (const int index : safeItems)
    {
        g_inventoryItems[index].storage = -1;
    }
    for (const int index : safeItems)
    {
        auto& item = g_inventoryItems[index];
        const int originalWidth = item.width;
        const int originalHeight = item.height;
        bool placed = false;
        for (int orientation = 0; orientation < 2 && !placed; ++orientation)
        {
            const int width = orientation == 0
                ? originalWidth
                : originalHeight;
            const int height = orientation == 0
                ? originalHeight
                : originalWidth;
            if (width == originalWidth && orientation == 1)
            {
                break;
            }
            const auto& layout =
                kInventoryStorageLayouts[kInventoryStorageSafeBox];
            for (int y = 0;
                 y <= layout.rows - height && !placed;
                 ++y)
            {
                for (int x = 0;
                     x <= layout.columns - width && !placed;
                     ++x)
                {
                    bool active = true;
                    for (int row = y; row < y + height && active; ++row)
                    {
                        for (int column = x;
                             column < x + width;
                             ++column)
                        {
                            if (!ExhibitionUi_IsSafeBoxCellActive(column, row))
                            {
                                active = false;
                                break;
                            }
                        }
                    }
                    if (!active)
                    {
                        continue;
                    }
                    if (InventoryUi_StateCellFree(
                            kInventoryStorageSafeBox,
                            x,
                            y,
                            width,
                            height,
                            index))
                    {
                        item.storage = kInventoryStorageSafeBox;
                        item.x = x;
                        item.y = y;
                        item.width = width;
                        item.height = height;
                        placed = true;
                    }
                }
            }
        }
        if (!placed)
        {
            item.storage = -1;
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Safe-box item %d could not be repacked after capacity change.",
                index);
        }
    }
}

void InventoryUi_ApplySafeBoxCapacity()
{
    auto& layout =
        kInventoryStorageLayouts[kInventoryStorageSafeBox];
    const int previousColumns = layout.columns;
    const int previousRows = layout.rows;
    layout.columns = ExhibitionUi_GetSafeBoxColumns();
    layout.rows = ExhibitionUi_GetSafeBoxRows();
    layout.visibleRows = layout.rows;
    InventoryUi_NormalizeSafeBoxItems();
    if (layout.columns != previousColumns ||
        layout.rows != previousRows)
    {
        LogInfo(
            LogCategory_Inventory,
            L"SafeBoxCapacity",
            L"Safe-box capacity changed %dx%d -> %dx%d cells=%d level=%d.",
            previousColumns,
            previousRows,
            layout.columns,
            layout.rows,
            ExhibitionUi_GetSafeBoxCells(),
            ExhibitionUi_GetCurrentLevel());
    }

    for (int index = 0;
         index < g_inventoryGridCellCounts[kInventoryStorageSafeBox];
         ++index)
    {
        InventoryUi_UpdateGridCell(
            g_inventoryGridCells[kInventoryStorageSafeBox][index]);
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        if (g_inventoryItems[index].storage ==
            kInventoryStorageSafeBox)
        {
            InventoryUi_UpdateItemRect(index);
        }
    }
    if (g_inventorySafeBoxSizeLabel != nullptr)
    {
        char text[160]{};
        std::snprintf(
            text,
            sizeof(text),
            "%d × %d  ·  %d 格  ·  展览柜 Lv.%d",
            layout.columns,
            layout.rows,
            ExhibitionUi_GetSafeBoxCells(),
            ExhibitionUi_GetCurrentLevel());
        SetSceneText(
            g_inventorySafeBoxSizeLabel,
            text,
            10.0f,
            514,
            0.78f,
            0.76f,
            0.72f,
            1.0f);
    }
}

void InventoryUi_SetRoute(int route)
{
    if (route < 0 || route >= kInventoryRouteCount)
    {
        route = kInventoryRouteEquipment;
    }
    g_inventoryActiveRoute = route;
    g_activeUiTab = route;
    if (route != kInventoryRouteEquipment && EquipmentPreviewUi_IsOpen())
    {
        EquipmentPreviewUi_Close();
    }
    const bool equipment = route == kInventoryRouteEquipment;
    const bool exhibition = route == kInventoryRouteCabinet;
    SetWorldGameObjectActive(g_inventoryEquipmentPage, equipment);
    SetWorldGameObjectActive(
        g_inventoryPlaceholderPage,
        !equipment && !exhibition);
    ExhibitionUi_SetPageVisible(exhibition);
    if (!equipment && !exhibition)
    {
        g_extendedActionListOpen = false;
        if (g_extendedActionPanel != nullptr)
        {
            SetWorldGameObjectActive(g_extendedActionPanel, false);
        }
        char placeholder[192]{};
        std::snprintf(
            placeholder,
            sizeof(placeholder),
            "%s\n\n模块入口已预留，此阶段仅实现装备页。\n选择顶部“装备”返回角色与物品管理。",
            kInventoryRouteLabels[route]);
        SetSceneText(
            g_inventoryPlaceholderTitle,
            placeholder,
            25.0f,
            514,
            0.82f,
            0.86f,
            0.92f,
            1.0f);
    }
    else
    {
        g_extendedReframeCountdown = 2;
    }
    for (int index = 0; index < kInventoryRouteCount; ++index)
    {
        const bool active = index == route;
        SceneUiElement& button = g_inventoryRouteButtons[index];
        SetSceneGraphicColor(
            button.image,
            active ? 0.61f : 0.075f,
            active ? 0.39f : 0.090f,
            active ? 0.15f : 0.110f,
            0.98f);
        SetSceneText(
            button.text,
            kInventoryRouteLabels[index],
            18.0f,
            514,
            active ? 1.0f : 0.76f,
            active ? 1.0f : 0.82f,
            active ? 1.0f : 0.89f,
            1.0f);
    }
    LogInfo(LogCategory_Inventory, L"Inventory", L"Inventory route selected: %S.", kInventoryRouteLabels[route]);
}

int InventoryUi_EquipmentSlotAt(float cursorX, float cursorY);
int InventoryUi_QuickSlotAt(float cursorX, float cursorY);

bool InventoryUi_HandleManualPointerPress(float cursorX, float cursorY)
{
    constexpr float headerMinX = 0.024f;
    constexpr float headerMinY = 0.902f;
    constexpr float headerMaxX = 0.976f;
    constexpr float headerMaxY = 0.974f;
    if (cursorX < headerMinX || cursorX > headerMaxX ||
        cursorY < headerMinY || cursorY > headerMaxY)
    {
        return false;
    }
    const float localX =
        (cursorX - headerMinX) / (headerMaxX - headerMinX);
    const float localY =
        (cursorY - headerMinY) / (headerMaxY - headerMinY);
    if (localY < 0.16f || localY > 0.84f)
    {
        return true;
    }
    constexpr float routeX[kInventoryRouteCount] = {
        0.255f, 0.385f, 0.515f, 0.645f,
    };
    for (int index = 0; index < kInventoryRouteCount; ++index)
    {
        if (localX >= routeX[index] &&
            localX <= routeX[index] + 0.120f)
        {
            InventoryUi_SetRoute(index);
            return true;
        }
    }
    return true;
}

bool InventoryUi_ShouldHandleManualPointerAt(float cursorX, float cursorY)
{
    if (ExhibitionUi_ShouldHandleManualPointerAt(cursorX, cursorY))
    {
        return true;
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        const auto& item = g_inventoryItems[index];
        if (item.storage < 0 ||
            item.storage >= kInventoryGridStorageCount)
        {
            continue;
        }
        if (cursorX >= item.screenMinX && cursorX <= item.screenMaxX &&
            cursorY >= item.screenMinY && cursorY <= item.screenMaxY)
        {
            return true;
        }
    }
    const int category = InventoryUi_EquipmentSlotAt(cursorX, cursorY);
    if (category >= 0 && InventoryUi_FindEquippedItem(category) >= 0)
    {
        return true;
    }
    const int quickSlot = InventoryUi_QuickSlotAt(cursorX, cursorY);
    if (quickSlot >= 0 &&
        InventoryUi_CanAssignQuickSlot(
            g_inventoryQuickSlots[quickSlot].itemIndex))
    {
        return true;
    }
    bool thumb = false;
    return InventoryUi_StashScrollbarHit(cursorX, cursorY, thumb);
}

bool InventoryUi_HandleButtonClick(Il2CppObject* instance)
{
    if (ExhibitionUi_HandleButtonClick(instance))
    {
        return true;
    }
    if (EquipmentPreviewUi_HandleButtonClick(instance))
    {
        return true;
    }
    for (int index = 0; index < kInventoryRouteCount; ++index)
    {
        if (instance == g_inventoryRouteButtons[index].button)
        {
            InventoryUi_SetRoute(index);
            return true;
        }
    }
    return false;
}

bool InventoryUi_IsDragging()
{
    return g_inventoryDragActive;
}

bool InventoryUi_CanDragModel()
{
    return g_inventoryActiveRoute == kInventoryRouteEquipment &&
           !g_inventoryDragActive &&
           !g_extendedActionListOpen &&
           !EquipmentPreviewUi_IsOpen();
}

void InventoryUi_CreateEquipmentSlot(
    int categoryIndex,
    Il2CppObject* loadoutTransform)
{
    constexpr float slotMinX[kInventoryEquipmentCategoryCount] = {
        0.570f, 0.705f, 0.840f,
        0.570f, 0.705f, 0.840f,
        0.570f, 0.705f, 0.840f,
    };
    constexpr float slotMinY[kInventoryEquipmentCategoryCount] = {
        0.665f, 0.665f, 0.665f,
        0.490f, 0.490f, 0.490f,
        0.315f, 0.315f, 0.315f,
    };
    constexpr float slotWidth = 0.125f;
    constexpr float slotHeight = 0.145f;
    char name[64]{};
    std::snprintf(
        name,
        sizeof(name),
        "EquipmentCategory_%d",
        categoryIndex);
    auto& slot = g_inventoryEquipmentSlots[categoryIndex];
    slot.panel = CreateScenePanel(
        name,
        loadoutTransform,
        slotMinX[categoryIndex],
        slotMinY[categoryIndex],
        slotMinX[categoryIndex] + slotWidth,
        slotMinY[categoryIndex] + slotHeight,
        0.060f,
        0.068f,
        0.080f,
        0.98f);
    slot.icon = CreateSceneRawImageElement(
        "Icon",
        slot.panel.rect,
        L"",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    slot.label = CreateSceneTextElement(
        "Label",
        slot.panel.rect,
        kInventoryEquipmentCategories[categoryIndex].label,
        0.28f,
        0.78f,
        0.97f,
        0.97f,
        11.0f,
        260,
        0.72f,
        0.77f,
        0.83f,
        1.0f);
    SetWorldGameObjectActive(slot.icon.gameObject, false);
    InventoryUi_UpdateCategoryScreenRect(categoryIndex);
}

void InventoryUi_CreateQuickSlots(Il2CppObject* loadoutTransform)
{
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        auto& slot = g_inventoryQuickSlots[index];
        char name[64]{};
        std::snprintf(
            name,
            sizeof(name),
            "QuickItemSlot_%d",
            index + 1);
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        InventoryUi_GetQuickSlotLocalRect(
            index,
            minX,
            minY,
            maxX,
            maxY);
        slot.panel = CreateScenePanel(
            name,
            loadoutTransform,
            minX,
            minY,
            maxX,
            maxY,
            0.060f,
            0.068f,
            0.080f,
            0.98f);
        slot.icon = CreateSceneRawImageElement(
            "Icon",
            slot.panel.rect,
            L"",
            0.075f,
            0.255f,
            0.925f,
            0.955f,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        slot.label = CreateSceneTextElement(
            "Label",
            slot.panel.rect,
            "空",
            0.035f,
            0.015f,
            0.965f,
            0.255f,
            9.0f,
            260,
            0.72f,
            0.77f,
            0.83f,
            1.0f);
        char keyText[4]{};
        std::snprintf(keyText, sizeof(keyText), "%d", index + 1);
        slot.keyLabel = CreateSceneTextElement(
            "Key",
            slot.panel.rect,
            keyText,
            0.035f,
            0.735f,
            0.29f,
            0.985f,
            18.0f,
            257,
            1.0f,
            0.93f,
            0.72f,
            1.0f);
        SetWorldGameObjectActive(slot.icon.gameObject, false);
        InventoryUi_UpdateQuickSlotScreenRect(index);
    }
}

void InventoryUi_CreateBackpackSlot(
    Il2CppObject* backpackTransform)
{
    auto& slot = g_inventoryBackpackSlot;
    slot.panel = CreateScenePanel(
        "EquippedBackpackSlot",
        backpackTransform,
        0.035f,
        0.690f,
        0.220f,
        0.855f,
        0.060f,
        0.068f,
        0.080f,
        0.98f);
    slot.icon = CreateSceneRawImageElement(
        "Icon",
        slot.panel.rect,
        L"",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    slot.label = CreateSceneTextElement(
        "Label",
        slot.panel.rect,
        "装备背包\n空",
        0.25f,
        0.76f,
        0.97f,
        0.97f,
        11.0f,
        514,
        0.72f,
        0.77f,
        0.83f,
        1.0f);
    CreateSceneTextElement(
        "BackpackSlotHint",
        backpackTransform,
        "背包装备位\n拖入包袋改变容量",
        0.035f,
        0.008f,
        0.965f,
        0.042f,
        13.0f,
        514,
        0.78f,
        0.82f,
        0.88f,
        1.0f);
    SetWorldGameObjectActive(slot.icon.gameObject, false);
    InventoryUi_SetBackpackSlotGeometry();
}

void InventoryUi_CreateItemTiles(Il2CppObject* equipmentTransform)
{
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        auto& item = g_inventoryItems[index];
        if (item.storage < 0)
        {
            continue;
        }
        const auto& definition =
            kEquipmentInventoryDefinitions[item.definition];
        constexpr float red = 0.025f;
        constexpr float green = 0.035f;
        constexpr float blue = 0.045f;
        char name[64]{};
        std::snprintf(name, sizeof(name), "InventoryItem_%d", index);
        item.element = CreateScenePanel(
            name,
            equipmentTransform,
            0.0f,
            0.0f,
            0.01f,
            0.01f,
            red,
            green,
            blue,
            0.98f);
        AddSceneComponentToGameObject(
            item.element.gameObject,
            "UnityEngine.UI",
            "RectMask2D");
        item.content.gameObject = CreateSceneGameObject(
            "Content",
            item.element.rect);
        item.content.rect = GetSceneComponentByClass(
            item.content.gameObject,
            "UnityEngine",
            "RectTransform");
        SetSceneRect(
            item.content.rect,
            0.0f,
            0.0f,
            1.0f,
            1.0f);
        item.icon = CreateSceneRawImageElement(
            "Icon",
            item.content.rect,
            InventoryUi_EquipmentImagePath(
                definition.id,
                item.quality),
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        item.appraisalBadgeCircle = CreateSceneTextElement(
            "AppraisalBadgeCircle",
            item.content.rect,
            "●",
            0.010f,
            0.795f,
            0.245f,
            1.000f,
            26.0f,
            514,
            0.62f,
            0.65f,
            0.68f,
            1.0f);
        item.appraisalBadgeText = CreateSceneTextElement(
            "AppraisalBadgeText",
            item.appraisalBadgeCircle.rect,
            "",
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            12.0f,
            514,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        SetWorldGameObjectActive(
            item.appraisalBadgeCircle.gameObject,
            false);
        SetWorldGameObjectActive(
            item.appraisalBadgeText.gameObject,
            false);
        item.label = CreateSceneTextElement(
            "Label",
            item.content.rect,
            definition.name,
            0.30f,
            0.79f,
            0.975f,
            0.975f,
            12.0f,
            260,
            0.94f,
            0.95f,
            0.97f,
            1.0f);
        Invoke(
            FindMethod(FindClass("UnityEngine", "Transform"), "SetAsLastSibling", 0),
            item.element.rect,
            nullptr);
        InventoryUi_UpdateItemRect(index);
    }
}

bool InventoryUi_EnsureStarterGrant(
    InventoryStateLoadResult loadResult)
{
    struct StarterItem
    {
        const char* id;
        int quality;
    };
    constexpr int kBaseStarterItemCount = 6;
    constexpr int kBonusStarterItemCount = 2;
    constexpr int kFixedStarterItemCount =
        kBaseStarterItemCount + kBonusStarterItemCount;
    constexpr int kRedStarterItemCount = 10;
    constexpr int kStarterItemCount =
        kFixedStarterItemCount + kRedStarterItemCount;
    constexpr int kCurrentStarterGrantVersion = 2;
    constexpr StarterItem bonusStarterItems[kBonusStarterItemCount] = {
        { "143-cosplay-01-011-rcosplaypartsoriginalrandoseru", 3 },
        { "061-cosplay-05-004-rcosplaypartslegbag", 2 },
    };

    if (loadResult != InventoryStateLoadResult::Missing)
    {
        if (loadResult == InventoryStateLoadResult::Loaded &&
            g_inventoryStarterGrantVersion > 0 &&
            g_inventoryStarterGrantVersion <
                kCurrentStarterGrantVersion)
        {
            for (const auto& starter : bonusStarterItems)
            {
                const int definition =
                    InventoryUi_FindDefinitionById(starter.id);
                if (definition < 0)
                {
                    LogInfo(LogCategory_Inventory, L"Inventory", 
                        L"Bonus starter item is missing from the catalog: %S.",
                        starter.id);
                    return false;
                }
                bool alreadyOwned = false;
                for (const auto& current : g_inventoryItems)
                {
                    if (current.definition == definition &&
                        current.storage >= 0)
                    {
                        alreadyOwned = true;
                        break;
                    }
                }
                if (alreadyOwned)
                {
                    continue;
                }

                const auto& itemDefinition =
                    kEquipmentInventoryDefinitions[definition];
                auto& item = g_inventoryItems[definition];
                bool placed = false;
                for (int y = 0;
                     y <=
                         kInventoryStorageLayouts[kInventoryStorageStash].rows -
                             itemDefinition.height &&
                     !placed;
                     ++y)
                {
                    for (int x = 0;
                         x <=
                             kInventoryStorageLayouts[kInventoryStorageStash].columns -
                                 itemDefinition.width &&
                         !placed;
                         ++x)
                    {
                        if (!InventoryUi_StateCellFree(
                                kInventoryStorageStash,
                                x,
                                y,
                                itemDefinition.width,
                                itemDefinition.height,
                                definition))
                        {
                            continue;
                        }
                        item.definition = definition;
                        item.quality = starter.quality;
                        item.appraised = false;
                        item.appraisalDebug = false;
                        item.wearUnits = 0;
                        item.quickSlot = -1;
                        item.storage = kInventoryStorageStash;
                        item.x = x;
                        item.y = y;
                        item.width = itemDefinition.width;
                        item.height = itemDefinition.height;
                        placed = true;
                    }
                }
                if (!placed)
                {
                    LogInfo(LogCategory_Inventory, L"Inventory", 
                        L"Bonus starter inventory could not place item %S.",
                        starter.id);
                    return false;
                }
            }
            g_inventoryStarterGrantVersion =
                kCurrentStarterGrantVersion;
            InventoryUi_SaveState();
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Starter inventory migrated to version 2 with gold shoulder bag and purple guard leg bag.");
            return true;
        }
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Starter inventory was not injected; existing state result=%d grant=%d.",
            static_cast<int>(loadResult),
            g_inventoryStarterGrantVersion);
        return false;
    }
    if (g_inventoryStarterGrantVersion >=
        kCurrentStarterGrantVersion)
    {
        return false;
    }

    StarterItem starterItems[kStarterItemCount] = {
        { "200-adult-003-madultgoodsrecordhandcuff", 0 },
        { "201-adult-002-madultgoodsrecordeyemask", 0 },
        { "202-adult-001-madultgoodsrecordvibrator", 0 },
        { "217-adult-011-mitemdildo", 0 },
        { "211-clothes-001-mclothesrecordcoat", 1 },
        { "003-costume-000-rcostumeshoes", 1 },
        { "143-cosplay-01-011-rcosplaypartsoriginalrandoseru", 3 },
        { "061-cosplay-05-004-rcosplaypartslegbag", 2 },
    };

    std::vector<int> redCandidates;
    redCandidates.reserve(kEquipmentInventoryDefinitionCount);
    for (int definition = 0;
         definition < kEquipmentInventoryDefinitionCount;
         ++definition)
    {
        const char* id = kEquipmentInventoryDefinitions[definition].id;
        bool alreadyIncluded = false;
        for (int starter = 0;
             starter < kFixedStarterItemCount;
             ++starter)
        {
            if (std::strcmp(starterItems[starter].id, id) == 0)
            {
                alreadyIncluded = true;
                break;
            }
        }
        if (!alreadyIncluded)
        {
            redCandidates.push_back(definition);
        }
    }

    std::mt19937 redGenerator(
        std::random_device{}() ^
        static_cast<unsigned int>(GetTickCount64()));
    std::shuffle(redCandidates.begin(), redCandidates.end(), redGenerator);
    for (int index = 0; index < kRedStarterItemCount; ++index)
    {
        starterItems[kFixedStarterItemCount + index] = {
            kEquipmentInventoryDefinitions[redCandidates[index]].id,
            kInventoryQualityCount - 1,
        };
    }

    for (const auto& starter : starterItems)
    {
        const int definition =
            InventoryUi_FindDefinitionById(starter.id);
        if (definition < 0)
        {
            LogInfo(LogCategory_Inventory, L"Inventory", L"Starter item is missing from the catalog: %S.", starter.id);
            InventoryUi_InitializeItems();
            g_inventoryStateLoaded = false;
            return false;
        }

        auto& item = g_inventoryItems[definition];
        const auto& itemDefinition =
            kEquipmentInventoryDefinitions[definition];
        const int columns =
            kInventoryStorageLayouts[kInventoryStorageStash].columns;
        const int rows =
            kInventoryStorageLayouts[kInventoryStorageStash].rows;
        bool placed = false;
        for (int y = 0; y <= rows - itemDefinition.height && !placed; ++y)
        {
            for (int x = 0; x <= columns - itemDefinition.width; ++x)
            {
                if (!InventoryUi_StateCellFree(
                        kInventoryStorageStash,
                        x,
                        y,
                        itemDefinition.width,
                        itemDefinition.height,
                        definition))
                {
                    continue;
                }
                item.definition = definition;
                item.quality = starter.quality;
                item.appraised = false;
                item.appraisalDebug = false;
                item.wearUnits = 0;
                item.quickSlot = -1;
                item.storage = kInventoryStorageStash;
                item.x = x;
                item.y = y;
                item.width = itemDefinition.width;
                item.height = itemDefinition.height;
                placed = true;
                break;
            }
        }
        if (!placed)
        {
            LogInfo(LogCategory_Inventory, L"Inventory", L"Starter inventory could not place item %S.", starter.id);
            InventoryUi_InitializeItems();
            g_inventoryStateLoaded = false;
            return false;
        }
    }

    g_inventoryStarterGrantVersion =
        kCurrentStarterGrantVersion;
    InventoryUi_SaveState();
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Starter inventory granted once with eighteen warehouse items; ten are red, one shoulder bag is gold, and one leg bag is purple.");
    return true;
}

void InventoryUi_InitializeItems()
{
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        auto& item = g_inventoryItems[index];
        item = InventoryItemState{};
        item.storage = -1;
        item.quickSlot = -1;
        if (index < kEquipmentInventoryDefinitionCount)
        {
            item.definition = index;
            item.width = kEquipmentInventoryDefinitions[index].width;
            item.height = kEquipmentInventoryDefinitions[index].height;
        }
        else
        {
            item.definition = 0;
            item.width = 1;
            item.height = 1;
        }
    }
}

void InventoryUi_Build(
    Il2CppObject* rootTransform,
    Il2CppObject* headerTransform)
{
    g_inventoryActiveRoute = kInventoryRouteEquipment;
    g_inventoryDraggingItem = -1;
    g_inventoryDragActive = false;
    g_inventoryRotateKeyDown = false;
    g_inventoryHoveredEquipmentSlot = -1;
    g_inventoryHoveredQuickSlot = -1;
    g_inventoryQuickSlotDragSource = -1;
    g_inventoryStashScrollRow = 0;
    g_inventoryStashWheelDelta = 0;
    g_inventoryScrollDragging = false;
    g_inventoryLeftButtonWasDown = false;
    g_inventoryInputUnlockTick = GetTickCount64() + 700;
    InventoryUi_InitializeItems();
    InventoryUi_InitializeQuickSlots();
    const InventoryStateLoadResult loadResult = InventoryUi_LoadState();
    InventoryUi_EnsureStarterGrant(loadResult);
    InventoryUi_ApplySafeBoxCapacity();

    const float routeX[kInventoryRouteCount] = {
        0.255f, 0.385f, 0.515f, 0.645f,
    };
    for (int index = 0; index < kInventoryRouteCount; ++index)
    {
        char name[64]{};
        std::snprintf(name, sizeof(name), "InventoryRoute_%d", index);
        g_inventoryRouteButtons[index] = CreateSceneButton(
            name,
            headerTransform,
            kInventoryRouteLabels[index],
            routeX[index],
            0.16f,
            routeX[index] + 0.120f,
            0.84f,
            0.075f,
            0.090f,
            0.110f,
            0.98f,
            18.0f,
            514,
            true);
    }

    auto equipmentPage = CreateScenePanel(
        "EquipmentPage",
        rootTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    g_inventoryEquipmentPage = equipmentPage.gameObject;
    auto* equipmentTransform = equipmentPage.rect;

    auto loadoutPanel = CreateScenePanel(
        "EquipmentColumn",
        equipmentTransform,
        0.024f,
        0.055f,
        0.350f,
        0.884f,
        0.025f,
        0.030f,
        0.039f,
        0.63f);
    auto* loadoutTransform = loadoutPanel.rect;
    CreateSceneTextElement(
        "EquipmentTitle",
        loadoutTransform,
        "装备  /  EQUIPMENT",
        0.035f,
        0.925f,
        0.965f,
        0.985f,
        22.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    CreateSceneTextElement(
        "EquipmentSubtitle",
        loadoutTransform,
        "SURVIVAL OPERATOR  ·  TACTICAL LOADOUT",
        0.045f,
        0.875f,
        0.955f,
        0.918f,
        12.0f,
        257,
        0.67f,
        0.72f,
        0.78f,
        1.0f);

    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        InventoryUi_CreateEquipmentSlot(category, loadoutTransform);
    }
    CreateSceneTextElement(
        "QuickItemTitle",
        loadoutTransform,
        "成人用品快捷键  /  ADULT GOODS",
        0.285f,
        0.254f,
        0.715f,
        0.298f,
        11.0f,
        514,
        0.81f,
        0.73f,
        0.55f,
        1.0f);
    InventoryUi_CreateQuickSlots(loadoutTransform);

    CreateSceneTextElement(
        "EquipmentHint",
        loadoutTransform,
        "拖动物品至分类槽穿戴  ·  拖回背包或仓库卸下  ·  R 旋转",
        0.035f,
        0.018f,
        0.965f,
        0.064f,
        11.0f,
        257,
        0.66f,
        0.70f,
        0.75f,
        1.0f);
    g_inventoryStatsLabel = CreateSceneTextElement(
        "EquipmentStats",
        loadoutTransform,
        "",
        0.035f,
        0.072f,
        0.965f,
        0.120f,
        12.0f,
        257,
        0.77f,
        0.80f,
        0.84f,
        1.0f).text;

    auto backpackPanel = CreateScenePanel(
        "BackpackColumn",
        equipmentTransform,
        kInventoryStorageLayouts[kInventoryStorageBackpack].panelMinX,
        kInventoryStorageLayouts[kInventoryStorageBackpack].panelMinY,
        kInventoryStorageLayouts[kInventoryStorageBackpack].panelMaxX,
        kInventoryStorageLayouts[kInventoryStorageBackpack].panelMaxY,
        0.032f,
        0.038f,
        0.047f,
        0.96f);
    CreateSceneTextElement(
        "BackpackTitle",
        backpackPanel.rect,
        "背包  /  BACKPACK",
        0.035f,
        0.915f,
        0.965f,
        0.975f,
        21.0f,
        257,
        0.95f,
        0.96f,
        0.98f,
        1.0f);
    char backpackSizeText[160]{};
    std::snprintf(
        backpackSizeText,
        sizeof(backpackSizeText),
        "%s  ·  %d x %d  ·  %d CELLS",
        "未装备背包",
        kInventoryStorageLayouts[kInventoryStorageBackpack].columns,
        kInventoryStorageLayouts[kInventoryStorageBackpack].rows,
        kInventoryStorageLayouts[kInventoryStorageBackpack].columns *
            kInventoryStorageLayouts[kInventoryStorageBackpack].rows);
    g_inventoryBackpackSizeLabel = CreateSceneTextElement(
        "BackpackSize",
        backpackPanel.rect,
        backpackSizeText,
        0.035f,
        0.868f,
        0.965f,
        0.910f,
        12.0f,
        514,
        0.71f,
        0.75f,
        0.80f,
        1.0f).text;
    InventoryUi_CreateBackpackSlot(backpackPanel.rect);

    auto safeBoxPanel = CreateScenePanel(
        "SafeBoxColumn",
        equipmentTransform,
        kInventoryStorageLayouts[kInventoryStorageSafeBox].panelMinX,
        kInventoryStorageLayouts[kInventoryStorageSafeBox].panelMinY,
        kInventoryStorageLayouts[kInventoryStorageSafeBox].panelMaxX,
        kInventoryStorageLayouts[kInventoryStorageSafeBox].panelMaxY,
        0.028f,
        0.050f,
        0.062f,
        0.96f);
    CreateSceneTextElement(
        "SafeBoxTitle",
        safeBoxPanel.rect,
        "安全箱  /  SECURE CONTAINER",
        0.035f,
        0.760f,
        0.760f,
        0.965f,
        15.0f,
        257,
        0.94f,
        0.86f,
        0.66f,
        1.0f);
    auto safeBoxSize = CreateSceneTextElement(
        "SafeBoxSize",
        safeBoxPanel.rect,
        "2 × 2  ·  固定",
        0.775f,
        0.760f,
        0.965f,
        0.965f,
        11.0f,
        514,
        0.78f,
        0.76f,
        0.72f,
        1.0f);
    g_inventorySafeBoxSizeLabel = safeBoxSize.text;

    auto stashPanel = CreateScenePanel(
        "StashColumn",
        equipmentTransform,
        kInventoryStorageLayouts[kInventoryStorageStash].panelMinX,
        kInventoryStorageLayouts[kInventoryStorageStash].panelMinY,
        kInventoryStorageLayouts[kInventoryStorageStash].panelMaxX,
        kInventoryStorageLayouts[kInventoryStorageStash].panelMaxY,
        0.032f,
        0.038f,
        0.047f,
        0.96f);
    CreateSceneTextElement(
        "StashTitle",
        stashPanel.rect,
        "仓库  /  STASH",
        0.025f,
        0.915f,
        0.760f,
        0.975f,
        21.0f,
        257,
        0.95f,
        0.96f,
        0.98f,
        1.0f);
    CreateSceneTextElement(
        "StashSize",
        stashPanel.rect,
        "10 x 20  ·  200 CELLS",
        0.765f,
        0.915f,
        0.975f,
        0.975f,
        12.0f,
        514,
        0.71f,
        0.75f,
        0.80f,
        1.0f);
    g_inventoryStashScrollTrack = CreateScenePanel(
        "StashScrollTrack",
        stashPanel.rect,
        0.956f,
        0.10f,
        0.994f,
        0.86f,
        0.045f,
        0.052f,
        0.062f,
        0.98f);
    g_inventoryStashScrollThumb = CreateScenePanel(
        "StashScrollThumb",
        stashPanel.rect,
        0.956f,
        0.556f,
        0.994f,
        0.86f,
        0.64f,
        0.49f,
        0.29f,
        1.0f);

    InventoryUi_CreateGrid(
        kInventoryStorageBackpack,
        equipmentTransform);
    InventoryUi_CreateGrid(
        kInventoryStorageSafeBox,
        equipmentTransform);
    InventoryUi_CreateGrid(
        kInventoryStorageStash,
        equipmentTransform);
    InventoryUi_CreateItemTiles(equipmentTransform);
    g_inventoryLayoutReady = false;
    InventoryUi_UpdateDynamicLayout();

    g_inventoryStatusLabel = CreateSceneTextElement(
        "InventoryStatus",
        equipmentTransform,
        "默认穿着基础外衣·外套  ·  外套同时占用上衣与下衣",
        0.620f,
        0.012f,
        0.975f,
        0.047f,
        14.0f,
        514,
        0.78f,
        0.83f,
        0.90f,
        1.0f).text;

    auto placeholder = CreateScenePanel(
        "RoutePlaceholderPage",
        rootTransform,
        0.024f,
        0.055f,
        0.975f,
        0.884f,
        0.020f,
        0.025f,
        0.034f,
        0.99f);
    g_inventoryPlaceholderPage = placeholder.gameObject;
    g_inventoryPlaceholderTitle = CreateSceneTextElement(
        "RoutePlaceholderTitle",
        placeholder.rect,
        "",
        0.10f,
        0.28f,
        0.90f,
        0.72f,
        25.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f).text;

    ExhibitionUi_Build(rootTransform);
    InventoryUi_SetRoute(kInventoryRouteEquipment);
    InventoryUi_RefreshEquipment();
    ClearSceneEventSelection();
    LogInfo(LogCategory_Inventory, L"Inventory", L"Inventory equipment categories, backpack and stash UI created.");
}

void InventoryUi_PrepareForSceneTransition()
{
    InventoryUi_SaveState();
    ExhibitionUi_Reset();
    ClearScenePngTextureCache();
    g_inventoryBadgeTexture = nullptr;
    g_inventoryBadgeSprite = nullptr;
    g_inventoryDragActive = false;
    g_inventoryDragMoved = false;
    g_inventoryDraggingItem = -1;
    g_inventoryHoveredEquipmentSlot = -1;
    g_inventoryHoveredQuickSlot = -1;
    g_inventoryQuickSlotDragSource = -1;
    g_inventoryScrollDragging = false;
    g_inventoryLeftButtonWasDown = false;
    g_inventoryEquipmentPage = nullptr;
    g_inventoryPlaceholderPage = nullptr;
    g_inventoryPlaceholderTitle = nullptr;
    g_inventoryStatusLabel = nullptr;
    g_inventoryStatsLabel = nullptr;
    g_inventoryBackpackSizeLabel = nullptr;
    g_inventorySafeBoxSizeLabel = nullptr;
    g_inventoryStashScrollTrack = SceneUiElement{};
    g_inventoryStashScrollThumb = SceneUiElement{};
    g_inventoryBackpackSlot = InventoryEquipmentSlot{};
    for (int index = 0; index < kInventoryRouteCount; ++index)
    {
        g_inventoryRouteButtons[index] = SceneUiElement{};
    }
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        g_inventoryEquipmentSlots[category] = InventoryEquipmentSlot{};
    }
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        g_inventoryQuickSlots[index] = InventoryQuickSlot{};
    }
    for (int storage = 0;
         storage < kInventoryGridStorageCount;
         ++storage)
    {
        for (int index = 0;
             index < g_inventoryGridCellCounts[storage];
             ++index)
        {
            g_inventoryGridCells[storage][index].element = SceneUiElement{};
        }
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        g_inventoryItems[index].element = SceneUiElement{};
        g_inventoryItems[index].icon = SceneUiElement{};
        g_inventoryItems[index].label = SceneUiElement{};
        g_inventoryItems[index].appraisalBadgeCircle =
            SceneUiElement{};
        g_inventoryItems[index].appraisalBadgeText =
            SceneUiElement{};
    }
    LogInfo(LogCategory_Inventory, L"Inventory", L"Inventory UI references released before scene transition.");
}

int InventoryUi_EquipmentSlotAt(float cursorX, float cursorY)
{
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        const auto& slot = g_inventoryEquipmentSlots[category];
        if (cursorX >= slot.screenMinX && cursorX <= slot.screenMaxX &&
            cursorY >= slot.screenMinY && cursorY <= slot.screenMaxY)
        {
            return category;
        }
    }
    const auto& backpackSlot = g_inventoryBackpackSlot;
    if (cursorX >= backpackSlot.screenMinX &&
        cursorX <= backpackSlot.screenMaxX &&
        cursorY >= backpackSlot.screenMinY &&
        cursorY <= backpackSlot.screenMaxY)
    {
        return kInventoryBackpackCategoryIndex;
    }
    return -1;
}

int InventoryUi_QuickSlotAt(float cursorX, float cursorY)
{
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        const auto& slot = g_inventoryQuickSlots[index];
        if (cursorX >= slot.screenMinX &&
            cursorX <= slot.screenMaxX &&
            cursorY >= slot.screenMinY &&
            cursorY <= slot.screenMaxY)
        {
            return index;
        }
    }
    return -1;
}

bool InventoryUi_UnbindQuickSlot(int index)
{
    if (index < 0 || index >= kInventoryQuickSlotCount)
    {
        return false;
    }
    InventoryUi_BindQuickSlot(index, -1);
    InventoryUi_UpdateQuickSlots();
    InventoryUi_SetStatus(
        "快捷格已解除绑定  ·  原物品仍保留在背包或安全箱");
    InventoryUi_SaveState();
    LogInfo(LogCategory_Inventory, L"Inventory", L"Quick slot %d unbound from its inventory item.", index + 1);
    return true;
}

int InventoryUi_GridAt(
    float cursorX,
    float cursorY,
    float width,
    float height,
    InventoryGridMetrics& metrics)
{
    for (int storage = 0;
         storage < kInventoryGridStorageCount;
         ++storage)
    {
        InventoryUi_CalculateGridMetrics(storage, width, height, metrics);
        if (metrics.cellWidth <= 0.0f ||
            metrics.cellHeight <= 0.0f)
        {
            continue;
        }
        if (cursorX >= metrics.minX && cursorX <= metrics.maxX &&
            cursorY >= metrics.minY && cursorY <= metrics.maxY)
        {
            return storage;
        }
    }
    return -1;
}

void InventoryUi_SetTileColor(int itemIndex, bool valid, bool dragging)
{
    if (itemIndex < 0 ||
        itemIndex >= kInventoryItemSlotCount)
    {
        return;
    }
    auto& item = g_inventoryItems[itemIndex];
    if (item.element.image == nullptr)
    {
        return;
    }
    if (dragging)
    {
        SetSceneGraphicColor(
            item.element.image,
            valid ? 0.12f : 0.58f,
            valid ? 0.38f : 0.12f,
            valid ? 0.28f : 0.10f,
            1.0f);
        return;
    }
    SetSceneGraphicColor(
        item.element.image,
        0.025f,
        0.035f,
        0.045f,
        0.98f);
}

void InventoryUi_SetDraggedTileToEquipmentSlot(
    int itemIndex,
    int category)
{
    auto& item = g_inventoryItems[itemIndex];
    const auto& slot = category == kInventoryBackpackCategoryIndex
        ? g_inventoryBackpackSlot
        : g_inventoryEquipmentSlots[category];
    SetSceneRect(
        item.element.rect,
        slot.screenMinX + 0.002f,
        slot.screenMinY + 0.002f,
        slot.screenMaxX - 0.002f,
        slot.screenMaxY - 0.002f);
}

void InventoryUi_RestoreDraggedItem()
{
    if (g_inventoryDraggingItem < 0)
    {
        return;
    }
    auto& item = g_inventoryItems[g_inventoryDraggingItem];
    item.storage = g_inventoryDragSourceStorage;
    item.x = g_inventoryDragSourceX;
    item.y = g_inventoryDragSourceY;
    item.width = g_inventoryDragSourceWidth;
    item.height = g_inventoryDragSourceHeight;
    InventoryUi_SetTileColor(g_inventoryDraggingItem, true, false);
    g_inventoryDragActive = false;
    g_inventoryDragMoved = false;
    g_inventoryHoveredEquipmentSlot = -1;
    g_inventoryHoveredQuickSlot = -1;
    g_inventoryQuickSlotDragSource = -1;
    g_inventoryDraggingItem = -1;
    InventoryUi_RefreshEquipment();
}

void InventoryUi_CancelUnmovedDrag()
{
    if (g_inventoryDraggingItem >= 0)
    {
        auto& item = g_inventoryItems[g_inventoryDraggingItem];
        item.storage = g_inventoryDragSourceStorage;
        item.x = g_inventoryDragSourceX;
        item.y = g_inventoryDragSourceY;
        item.width = g_inventoryDragSourceWidth;
        item.height = g_inventoryDragSourceHeight;
    }
    // A click without movement never touched any Unity object, so avoid all
    // UI/model refresh calls on this path.
    g_inventoryDragActive = false;
    g_inventoryDragMoved = false;
    g_inventoryHoveredEquipmentSlot = -1;
    g_inventoryHoveredQuickSlot = -1;
    g_inventoryQuickSlotDragSource = -1;
    g_inventoryDraggingItem = -1;
}

void InventoryUi_UpdateDrag()
{
    if (ItemContextMenu_ShouldBlockPointer())
    {
        return;
    }
    const bool leftDown =
        g_extendedModeState.load() == kExtendedModeActiveStateValue
            ? (g_extendedManualPointerDown.load() &&
               !g_extendedManualPointerConsumed.load())
            : ((GameplayInput_ReadRawAsyncKeyState(VK_LBUTTON) &
                0x8000) != 0);
    const bool leftPressed = leftDown && !g_inventoryLeftButtonWasDown;
    g_inventoryLeftButtonWasDown = leftDown;
    if (g_extendedModeState.load() == kExtendedModeActiveStateValue &&
        !g_inventoryDragActive &&
        !g_extendedManualPointerInventoryEligible.load())
    {
        g_inventoryLeftButtonWasDown = leftDown;
        return;
    }
    if (GetTickCount64() < g_inventoryInputUnlockTick)
    {
        return;
    }
    if (EquipmentPreviewUi_IsOpen())
    {
        return;
    }
    if (g_inventoryActiveRoute != kInventoryRouteEquipment ||
        g_extendedCanvas == nullptr ||
        g_extendedActionListOpen)
    {
        if (!g_inventoryDragActive)
        {
            return;
        }
    }
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
    const float cursorPixelsX =
        static_cast<float>(cursor.x - clientRect.left);
    const float cursorPixelsY =
        static_cast<float>(cursor.y - clientRect.top);
    const float cursorX = cursorPixelsX / width;
    const float cursorY = 1.0f - cursorPixelsY / height;
    const bool rotateDown =
        (GameplayInput_ReadRawAsyncKeyState('R') & 0x8000) != 0;

    bool scrollbarThumb = false;
    const bool overStashScrollbar = InventoryUi_StashScrollbarHit(
        cursorX,
        cursorY,
        scrollbarThumb);
    if (g_inventoryScrollDragging)
    {
        if (leftDown)
        {
            InventoryUi_UpdateStashScrollFromCursor(cursorY);
        }
        else
        {
            g_inventoryScrollDragging = false;
            InventoryUi_SaveState();
        }
        return;
    }
    if (!leftDown)
    {
        g_inventoryScrollDragging = false;
    }
    if (overStashScrollbar && leftPressed && !g_inventoryDragActive)
    {
        const float thumbHeight =
            g_inventoryStashThumbScreenMaxY -
            g_inventoryStashThumbScreenMinY;
        g_inventoryScrollDragOffsetY = scrollbarThumb
            ? cursorY - g_inventoryStashThumbScreenMinY
            : thumbHeight * 0.5f;
        g_inventoryScrollDragging = true;
        InventoryUi_UpdateStashScrollFromCursor(cursorY);
        return;
    }

    if (!leftDown)
    {
        if (!g_inventoryDragActive)
        {
            g_inventoryRotateKeyDown = rotateDown;
            g_inventoryHoveredEquipmentSlot = -1;
            g_inventoryHoveredQuickSlot = -1;
            return;
        }
        if (!g_inventoryDragMoved)
        {
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Inventory mouse released without movement: item=%d.",
                g_inventoryDraggingItem);
            InventoryUi_CancelUnmovedDrag();
            return;
        }

        auto& item = g_inventoryItems[g_inventoryDraggingItem];
        InventoryGridMetrics gridMetrics{};
        int targetStorage = InventoryUi_GridAt(
            cursorX,
            cursorY,
            width,
            height,
            gridMetrics);
        int targetCategory = InventoryUi_EquipmentSlotAt(
            cursorX,
            cursorY);
        const int targetQuickSlot = InventoryUi_QuickSlotAt(
            cursorX,
            cursorY);
        if (targetQuickSlot >= 0)
        {
            if (InventoryUi_CanAssignQuickSlot(
                    g_inventoryDraggingItem))
            {
                InventoryUi_BindQuickSlot(
                    targetQuickSlot,
                    g_inventoryDraggingItem);
                InventoryUi_RestoreDraggedItem();
                InventoryUi_SetStatus(
                    "快捷格已绑定实际物品  ·  原物品仍在背包或安全箱");
                InventoryUi_SaveState();
            }
            else
            {
                InventoryUi_RestoreDraggedItem();
                InventoryUi_Warn(
                    "快捷格只接受背包或安全箱中的成人用品；物品未移动");
            }
            return;
        }
        if ((targetCategory < 0 ||
             !InventoryUi_CanEquip(
                 g_inventoryDraggingItem,
                 targetCategory)) &&
            InventoryUi_IsInEquipmentColumn(cursorX, cursorY))
        {
            const int automaticCategory =
                InventoryUi_FindAutoEquipCategory(
                g_inventoryDraggingItem);
            if (automaticCategory >= 0)
            {
                targetCategory = automaticCategory;
            }
        }
        if (g_inventoryDragSourceStorage != kInventoryStorageEquipped &&
            InventoryUi_ItemSlotIsBackpack(g_inventoryDraggingItem) &&
            InventoryUi_IsInBackpackColumn(cursorX, cursorY) &&
            InventoryUi_CanEquip(
                g_inventoryDraggingItem,
                kInventoryBackpackCategoryIndex))
        {
            targetStorage = -1;
            targetCategory = kInventoryBackpackCategoryIndex;
            InventoryUi_SetStatus(
                "检测到背包区域  ·  松开鼠标自动装备");
        }
        if (targetStorage >= 0 &&
            g_inventoryDragSourceStorage == kInventoryStorageEquipped &&
            InventoryUi_ItemSlotIsBackpack(g_inventoryDraggingItem) &&
            InventoryUi_BackpackHasItems(g_inventoryDraggingItem))
        {
            InventoryUi_RestoreDraggedItem();
            InventoryUi_Warn(
                "背包内仍有物品，必须清空后才能卸下背包");
            return;
        }
        if (targetStorage >= 0)
        {
            const auto& layout = kInventoryStorageLayouts[targetStorage];
            int hoveredColumn = static_cast<int>(std::floor(
                (cursorX - gridMetrics.minX) / gridMetrics.cellWidth));
            int hoveredRow = static_cast<int>(std::floor(
                (gridMetrics.maxY - cursorY) / gridMetrics.cellHeight));
            hoveredColumn = std::clamp(
                hoveredColumn,
                0,
                layout.columns - 1);
            const int scrollOffset =
                targetStorage == kInventoryStorageStash
                    ? g_inventoryStashScrollRow
                    : 0;
            hoveredRow = std::clamp(
                hoveredRow + scrollOffset,
                0,
                layout.rows - 1);
            int targetX = hoveredColumn - g_inventoryDragGrabCellX;
            int targetY = hoveredRow - g_inventoryDragGrabCellY;
            targetX = targetX < 0
                ? 0
                : (targetX + item.width > layout.columns
                    ? layout.columns - item.width
                    : targetX);
            targetY = targetY < 0
                ? 0
                : (targetY + item.height > layout.rows
                    ? layout.rows - item.height
                    : targetY);
            if (targetStorage == kInventoryStorageStash)
            {
                targetY = std::clamp(
                    targetY,
                    g_inventoryStashScrollRow,
                    g_inventoryStashScrollRow +
                        layout.visibleRows - item.height);
            }
            if (InventoryUi_CanPlace(
                    g_inventoryDraggingItem,
                    targetStorage,
                    targetX,
                    targetY))
            {
                const bool wasEquipped =
                    g_inventoryDragSourceStorage == kInventoryStorageEquipped;
                item.storage = targetStorage;
                item.x = targetX;
                item.y = targetY;
                InventoryUi_SetTileColor(
                    g_inventoryDraggingItem,
                    true,
                    false);
                const int movedItemIndex = g_inventoryDraggingItem;
                g_inventoryDragActive = false;
                g_inventoryDragMoved = false;
                g_inventoryHoveredEquipmentSlot = -1;
                g_inventoryDraggingItem = -1;
                if (g_inventoryQuickSlotDragSource >= 0)
                {
                    InventoryUi_BindQuickSlot(
                        g_inventoryQuickSlotDragSource,
                        -1);
                    g_inventoryQuickSlotDragSource = -1;
                }
                InventoryUi_PruneQuickSlotBindings();
                InventoryUi_SetStatus(
                    wasEquipped
                        ? "装备已卸下并放入储物区  ·  已自动保存"
                        : (targetStorage == kInventoryStorageBackpack
                            ? "已放入背包  ·  已自动保存"
                            : "已放入仓库  ·  已自动保存"));
                InventoryUi_RefreshEquipment();
                InventoryUi_SaveState();
                LogInfo(
                    LogCategory_Inventory,
                    L"ItemMoved",
                    L"Inventory item moved itemSlot=%d storage=%d x=%d y=%d fromEquipped=%d.",
                    movedItemIndex,
                    targetStorage,
                    targetX,
                    targetY,
                    wasEquipped ? 1 : 0);
            }
            else
            {
                InventoryUi_RestoreDraggedItem();
                InventoryUi_Warn("空间不足，物品已回到原位");
            }
            return;
        }
        if (targetCategory >= 0)
        {
            if (InventoryUi_CanEquip(
                    g_inventoryDraggingItem,
                    targetCategory))
            {
                item.storage = kInventoryStorageEquipped;
                InventoryUi_SetTileColor(
                    g_inventoryDraggingItem,
                    true,
                    false);
                const int equippedItemIndex = g_inventoryDraggingItem;
                g_inventoryDragActive = false;
                g_inventoryDragMoved = false;
                g_inventoryHoveredEquipmentSlot = -1;
                g_inventoryDraggingItem = -1;
                if (g_inventoryQuickSlotDragSource >= 0)
                {
                    InventoryUi_BindQuickSlot(
                        g_inventoryQuickSlotDragSource,
                        -1);
                    g_inventoryQuickSlotDragSource = -1;
                }
                InventoryUi_PruneQuickSlotBindings();
                InventoryUi_SetStatus("装备已穿戴，角色模型已刷新  ·  已自动保存");
                InventoryUi_RefreshEquipment();
                InventoryUi_SaveState();
                LogInfo(
                    LogCategory_Inventory,
                    L"ItemEquipped",
                    L"Inventory item equipped itemSlot=%d category=%d.",
                    equippedItemIndex,
                    targetCategory);
            }
            else
            {
                InventoryUi_RestoreDraggedItem();
                InventoryUi_Warn(
                    "该分类已被占用，或物品不支持此分类；物品已回到原位");
            }
            return;
        }
        if (InventoryUi_IsInEquipmentColumn(cursorX, cursorY))
        {
            InventoryUi_RestoreDraggedItem();
            InventoryUi_Warn(
                "左侧没有可用空位：该分类已被占用或物品不支持当前分类");
            return;
        }
        InventoryUi_RestoreDraggedItem();
        InventoryUi_Warn("请将物品放入分类槽、背包或仓库");
        return;
    }

    if (!g_inventoryDragActive && leftPressed)
    {
        const int pressedQuickSlot =
            InventoryUi_QuickSlotAt(cursorX, cursorY);
        if (pressedQuickSlot >= 0)
        {
            auto& quickSlot =
                g_inventoryQuickSlots[pressedQuickSlot];
            if (!InventoryUi_CanAssignQuickSlot(
                    quickSlot.itemIndex))
            {
                InventoryUi_Warn(
                    "该快捷格没有绑定背包或安全箱中的成人用品");
                return;
            }
            auto& item =
                g_inventoryItems[quickSlot.itemIndex];
            g_inventoryDraggingItem = quickSlot.itemIndex;
            g_inventoryDragSourceStorage = item.storage;
            g_inventoryDragSourceX = item.x;
            g_inventoryDragSourceY = item.y;
            g_inventoryDragSourceWidth = item.width;
            g_inventoryDragSourceHeight = item.height;
            g_inventoryDragGrabCellX = 0;
            g_inventoryDragGrabCellY = 0;
            g_inventoryDragPointerOffsetPixelsX =
                cursorPixelsX - quickSlot.screenMinX * width;
            g_inventoryDragPointerOffsetPixelsY =
                (1.0f - quickSlot.screenMaxY) * height - cursorPixelsY;
            g_inventoryDragStartCursorPixelsX = cursorPixelsX;
            g_inventoryDragStartCursorPixelsY = cursorPixelsY;
            InventoryGridMetrics metrics{};
            InventoryUi_CalculateGridMetrics(
                item.storage,
                width,
                height,
                metrics);
            g_inventoryDragCellWidth = metrics.cellWidth;
            g_inventoryDragCellHeight = metrics.cellHeight;
            g_inventoryDragActive = true;
            g_inventoryDragMoved = false;
            g_inventoryQuickSlotDragSource = pressedQuickSlot;
            g_inventoryRotateKeyDown = rotateDown;
            return;
        }
        g_inventoryQuickSlotDragSource = -1;
        for (int index = kInventoryItemSlotCount - 1;
             index >= 0;
             --index)
        {
            const auto& item = g_inventoryItems[index];
            if (item.storage < 0 ||
                item.storage >= kInventoryGridStorageCount ||
                cursorX < item.screenMinX || cursorX > item.screenMaxX ||
                cursorY < item.screenMinY || cursorY > item.screenMaxY)
            {
                continue;
            }
            g_inventoryDraggingItem = index;
            g_inventoryDragSourceStorage = item.storage;
            g_inventoryDragSourceX = item.x;
            g_inventoryDragSourceY = item.y;
            g_inventoryDragSourceWidth = item.width;
            g_inventoryDragSourceHeight = item.height;
            InventoryGridMetrics metrics{};
            InventoryUi_CalculateGridMetrics(
                item.storage,
                width,
                height,
                metrics);
            const auto& layout = kInventoryStorageLayouts[item.storage];
            int hoveredColumn = static_cast<int>(std::floor(
                (cursorX - metrics.minX) / metrics.cellWidth));
            int hoveredRow = static_cast<int>(std::floor(
                (metrics.maxY - cursorY) / metrics.cellHeight));
            hoveredColumn = std::clamp(
                hoveredColumn,
                0,
                layout.columns - 1);
            hoveredRow = std::clamp(
                hoveredRow +
                    (item.storage == kInventoryStorageStash
                        ? g_inventoryStashScrollRow
                        : 0),
                0,
                layout.rows - 1);
            g_inventoryDragGrabCellX = std::clamp(
                hoveredColumn - item.x,
                0,
                item.width - 1);
            g_inventoryDragGrabCellY = std::clamp(
                hoveredRow - item.y,
                0,
                item.height - 1);
            g_inventoryDragPointerOffsetPixelsX =
                cursorPixelsX - item.screenMinX * width;
            g_inventoryDragPointerOffsetPixelsY =
                (1.0f - item.screenMaxY) * height - cursorPixelsY;
            g_inventoryDragStartCursorPixelsX = cursorPixelsX;
            g_inventoryDragStartCursorPixelsY = cursorPixelsY;
            g_inventoryDragCellWidth = metrics.cellWidth;
            g_inventoryDragCellHeight = metrics.cellHeight;
            g_inventoryDragActive = true;
            g_inventoryDragMoved = false;
            g_inventoryRotateKeyDown = rotateDown;
            return;
        }

        const int category = InventoryUi_EquipmentSlotAt(cursorX, cursorY);
        if (category >= 0)
        {
            const int itemIndex = InventoryUi_FindEquippedItem(category);
            if (itemIndex >= 0)
            {
                auto& item = g_inventoryItems[itemIndex];
                g_inventoryDraggingItem = itemIndex;
                g_inventoryDragSourceStorage = kInventoryStorageEquipped;
                g_inventoryDragSourceX = item.x;
                g_inventoryDragSourceY = item.y;
                g_inventoryDragSourceWidth = item.width;
                g_inventoryDragSourceHeight = item.height;
                g_inventoryDragGrabCellX = 0;
                g_inventoryDragGrabCellY = 0;
                g_inventoryDragPointerOffsetPixelsX = 0.0f;
                g_inventoryDragPointerOffsetPixelsY = 0.0f;
                g_inventoryDragStartCursorPixelsX = cursorPixelsX;
                g_inventoryDragStartCursorPixelsY = cursorPixelsY;
                InventoryGridMetrics metrics{};
                InventoryUi_CalculateGridMetrics(
                    kInventoryStorageStash,
                    width,
                    height,
                    metrics);
                g_inventoryDragCellWidth = metrics.cellWidth;
                g_inventoryDragCellHeight = metrics.cellHeight;
                g_inventoryDragActive = true;
                g_inventoryDragMoved = false;
                g_inventoryRotateKeyDown = rotateDown;
                return;
            }
        }
        return;
    }

    auto& item = g_inventoryItems[g_inventoryDraggingItem];
    if (std::fabs(cursorPixelsX - g_inventoryDragStartCursorPixelsX) > 4.0f ||
        std::fabs(cursorPixelsY - g_inventoryDragStartCursorPixelsY) > 4.0f)
    {
        g_inventoryDragMoved = true;
    }
    if (!g_inventoryDragMoved)
    {
        return;
    }
    SetWorldGameObjectActive(item.element.gameObject, true);
    if (item.content.rect != nullptr)
    {
        SetSceneRect(item.content.rect, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    InventoryUi_SetTileColor(g_inventoryDraggingItem, true, true);
    InventoryUi_UpdateEquipmentSlotHighlights();
    InventoryUi_SetStatus("正在移动物品  ·  按 R 旋转  ·  左侧区域自动装备");
    if (rotateDown && !g_inventoryRotateKeyDown)
    {
        const int oldWidth = item.width;
        item.width = item.height;
        item.height = oldWidth;
        g_inventoryDragMoved = true;
        g_inventoryDragGrabCellX = std::clamp(
            g_inventoryDragGrabCellX,
            0,
            item.width - 1);
        g_inventoryDragGrabCellY = std::clamp(
            g_inventoryDragGrabCellY,
            0,
            item.height - 1);
        InventoryUi_SetStatus("物品已旋转");
    }
    g_inventoryRotateKeyDown = rotateDown;
    InventoryUi_ApplyItemIconRotation(item);

    InventoryGridMetrics hoveredMetrics{};
    int hoveredStorage = InventoryUi_GridAt(
        cursorX,
        cursorY,
        width,
        height,
        hoveredMetrics);
    int hoveredCategory = InventoryUi_EquipmentSlotAt(
        cursorX,
        cursorY);
    const int hoveredQuickSlot = InventoryUi_QuickSlotAt(
        cursorX,
        cursorY);
    g_inventoryHoveredQuickSlot = hoveredQuickSlot;
    if (hoveredQuickSlot >= 0)
    {
        g_inventoryHoveredEquipmentSlot = -1;
        InventoryUi_UpdateEquipmentSlotHighlights();
        InventoryUi_UpdateQuickSlots();
        const auto& quickSlot =
            g_inventoryQuickSlots[hoveredQuickSlot];
        SetSceneRect(
            item.element.rect,
            quickSlot.screenMinX + 0.002f,
            quickSlot.screenMinY + 0.002f,
            quickSlot.screenMaxX - 0.002f,
            quickSlot.screenMaxY - 0.002f);
        InventoryUi_SetTileColor(
            g_inventoryDraggingItem,
            InventoryUi_CanAssignQuickSlot(
                g_inventoryDraggingItem),
            true);
        InventoryUi_SetStatus(
            InventoryUi_CanAssignQuickSlot(
                g_inventoryDraggingItem)
                ? "快捷格只展示成人用品，不会移动原物品"
                : "仅背包或安全箱中的成人用品可放入快捷格");
        return;
    }
    if (g_inventoryDragSourceStorage != kInventoryStorageEquipped &&
        InventoryUi_ItemSlotIsBackpack(g_inventoryDraggingItem) &&
        InventoryUi_IsInBackpackColumn(cursorX, cursorY) &&
        InventoryUi_CanEquip(
            g_inventoryDraggingItem,
            kInventoryBackpackCategoryIndex))
    {
        hoveredStorage = -1;
        hoveredCategory = kInventoryBackpackCategoryIndex;
        InventoryUi_SetStatus(
            "检测到背包区域  ·  松开鼠标自动装备");
    }
    if ((hoveredCategory < 0 ||
         !InventoryUi_CanEquip(
             g_inventoryDraggingItem,
             hoveredCategory)) &&
        InventoryUi_IsInEquipmentColumn(cursorX, cursorY))
    {
        const int automaticCategory =
            InventoryUi_FindAutoEquipCategory(
            g_inventoryDraggingItem);
        if (automaticCategory >= 0)
        {
            hoveredCategory = automaticCategory;
            InventoryUi_SetStatus(
                "左侧检测到可用分类  ·  松开鼠标自动装备");
        }
    }
    g_inventoryHoveredEquipmentSlot = hoveredCategory;
    InventoryUi_UpdateEquipmentSlotHighlights();

    if (hoveredStorage >= 0)
    {
        const auto& layout = kInventoryStorageLayouts[hoveredStorage];
        int hoveredColumn = static_cast<int>(std::floor(
            (cursorX - hoveredMetrics.minX) / hoveredMetrics.cellWidth));
        int hoveredRow = static_cast<int>(std::floor(
            (hoveredMetrics.maxY - cursorY) / hoveredMetrics.cellHeight));
        hoveredColumn = std::clamp(
            hoveredColumn,
            0,
            layout.columns - 1);
        const int scrollOffset =
            hoveredStorage == kInventoryStorageStash
                ? g_inventoryStashScrollRow
                : 0;
        hoveredRow = std::clamp(
            hoveredRow + scrollOffset,
            0,
            layout.rows - 1);
        int targetX = hoveredColumn - g_inventoryDragGrabCellX;
        int targetY = hoveredRow - g_inventoryDragGrabCellY;
        targetX = targetX < 0
            ? 0
            : (targetX + item.width > layout.columns
                ? layout.columns - item.width
                : targetX);
        targetY = targetY < 0
            ? 0
            : (targetY + item.height > layout.rows
                ? layout.rows - item.height
                : targetY);
        if (hoveredStorage == kInventoryStorageStash)
        {
            targetY = std::clamp(
                targetY,
                g_inventoryStashScrollRow,
                g_inventoryStashScrollRow +
                    layout.visibleRows - item.height);
        }
        const int visibleTargetY = targetY - scrollOffset;
        const float gapX = 2.0f / width;
        const float gapY = 2.0f / height;
        SetSceneRect(
            item.element.rect,
            hoveredMetrics.minX + targetX * hoveredMetrics.cellWidth + gapX,
            hoveredMetrics.maxY -
                (visibleTargetY + item.height) * hoveredMetrics.cellHeight + gapY,
            hoveredMetrics.minX +
                (targetX + item.width) * hoveredMetrics.cellWidth - gapX,
            hoveredMetrics.maxY -
                visibleTargetY * hoveredMetrics.cellHeight - gapY);
        InventoryUi_SetTileColor(
            g_inventoryDraggingItem,
            InventoryUi_CanPlace(
                g_inventoryDraggingItem,
                hoveredStorage,
                targetX,
                targetY),
            true);
        return;
    }
    if (hoveredCategory >= 0)
    {
        InventoryUi_SetDraggedTileToEquipmentSlot(
            g_inventoryDraggingItem,
            hoveredCategory);
        InventoryUi_SetTileColor(
            g_inventoryDraggingItem,
            InventoryUi_CanEquip(
                g_inventoryDraggingItem,
                hoveredCategory),
            true);
        return;
    }

    const float dragWidthPixels =
        static_cast<float>(item.width) * g_inventoryDragCellWidth * width;
    const float dragHeightPixels =
        static_cast<float>(item.height) * g_inventoryDragCellHeight * height;
    const float dragMinPixelsX =
        cursorPixelsX - g_inventoryDragPointerOffsetPixelsX;
    const float dragTopPixelsY =
        cursorPixelsY + g_inventoryDragPointerOffsetPixelsY;
    SetSceneRect(
        item.element.rect,
        dragMinPixelsX / width,
        1.0f - (dragTopPixelsY + dragHeightPixels) / height,
        (dragMinPixelsX + dragWidthPixels) / width,
        1.0f - dragTopPixelsY / height);
    InventoryUi_SetTileColor(g_inventoryDraggingItem, false, true);
}

void InventoryUi_Reset()
{
    ExhibitionUi_Reset();
    g_inventoryEquipmentPage = nullptr;
    g_inventoryPlaceholderPage = nullptr;
    g_inventoryPlaceholderTitle = nullptr;
    g_inventoryStatusLabel = nullptr;
    g_inventoryStatsLabel = nullptr;
    g_inventoryBackpackSizeLabel = nullptr;
    g_inventorySafeBoxSizeLabel = nullptr;
    g_inventoryStashScrollTrack = SceneUiElement{};
    g_inventoryStashScrollThumb = SceneUiElement{};
    g_inventoryActiveRoute = kInventoryRouteEquipment;
    g_inventoryStashScrollRow = 0;
    g_inventoryStashWheelDelta = 0;
    g_inventoryScrollDragging = false;
    g_inventoryScrollDragOffsetY = 0.0f;
    g_inventoryStashThumbScreenMinY = 0.0f;
    g_inventoryStashThumbScreenMaxY = 0.0f;
    g_inventoryDraggingItem = -1;
    g_inventoryDragSourceStorage = -1;
    g_inventoryDragSourceX = 0;
    g_inventoryDragSourceY = 0;
    g_inventoryDragSourceWidth = 1;
    g_inventoryDragSourceHeight = 1;
    g_inventoryDragGrabCellX = 0;
    g_inventoryDragGrabCellY = 0;
    g_inventoryHoveredEquipmentSlot = -1;
    g_inventoryHoveredQuickSlot = -1;
    g_inventoryQuickSlotDragSource = -1;
    g_inventoryDragPointerOffsetPixelsX = 0.0f;
    g_inventoryDragPointerOffsetPixelsY = 0.0f;
    g_inventoryDragStartCursorPixelsX = 0.0f;
    g_inventoryDragStartCursorPixelsY = 0.0f;
    g_inventoryDragActive = false;
    g_inventoryDragMoved = false;
    g_inventoryRotateKeyDown = false;
    g_inventoryLeftButtonWasDown = false;
    g_inventoryStateLoaded = false;
    g_inventoryInputUnlockTick = 0;
    g_inventoryLayoutWidth = 1920.0f;
    g_inventoryLayoutHeight = 1080.0f;
    g_inventoryLayoutReady = false;
    for (int storage = 0;
         storage < kInventoryGridStorageCount;
         ++storage)
    {
        g_inventoryGridCellCounts[storage] = 0;
        for (int index = 0; index < kInventoryMaxGridCells; ++index)
        {
            g_inventoryGridCells[storage][index] = InventoryGridCell{};
        }
    }
    for (int index = 0; index < kInventoryRouteCount; ++index)
    {
        g_inventoryRouteButtons[index] = SceneUiElement{};
    }
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        g_inventoryEquipmentSlots[category] = InventoryEquipmentSlot{};
    }
    g_inventoryBackpackSlot = InventoryEquipmentSlot{};
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        g_inventoryQuickSlots[index] = InventoryQuickSlot{};
    }
    for (int index = 0;
         index < kInventoryItemSlotCount;
         ++index)
    {
        g_inventoryItems[index] = InventoryItemState{};
    }
}
