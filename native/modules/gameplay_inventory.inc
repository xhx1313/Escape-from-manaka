// Native.cpp module: gameplay_inventory.inc
// Raid inventory shown with Tab. Equipment and backpack mirror the persistent
// warehouse state; supply and staging areas are intentionally session-local.

constexpr int kGameplayInventoryEquipped = 0;
constexpr int kGameplayInventoryBackpack = 1;
constexpr int kGameplayInventorySupply = 2;
constexpr int kGameplayInventoryOrganizer = 3;
constexpr int kGameplayInventoryStaging = kGameplayInventoryOrganizer;
constexpr int kGameplayInventorySafeBox = 4;
constexpr int kGameplayInventoryPanelCount = 5;
constexpr int kGameplayInventoryColumns = 6;
constexpr int kGameplayInventoryRows = 8;
constexpr int kGameplayInventoryMaxGridCells =
    kGameplayInventoryColumns * kGameplayInventoryRows;
constexpr int kGameplayInventoryEquipmentColumns = 3;
constexpr int kGameplayInventoryEquipmentRows = 3;
constexpr int kGameplayInventoryMaxViews =
    kEquipmentInventoryDefinitionCount + 128;

struct GameplayInventoryTempItem
{
    int definition = -1;
    int quality = 0;
    bool appraised = false;
    unsigned int wearUnits = 0;
    int storage = kGameplayInventorySupply;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
    unsigned int serial = 0;
};

struct GameplayInventoryLootRequest
{
    int definition = -1;
    int quality = 0;
};

struct GameplayInventoryView
{
    int definition = -1;
    int quality = 0;
    bool appraised = false;
    unsigned int wearUnits = 0;
    int itemIndex = -1;
    int storage = -1;
    int equipmentCategory = -1;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
    int tempIndex = -1;
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    SceneUiElement root{};
    SceneUiElement icon{};
    SceneUiElement label{};
    SceneUiElement appraisalBadgeCircle{};
    SceneUiElement appraisalBadgeText{};
};

struct GameplayInventoryGridCell
{
    int storage = -1;
    int column = 0;
    int row = 0;
    SceneUiElement panel{};
};

struct GameplayQuickSlotUi
{
    SceneUiElement panel{};
    SceneUiElement icon{};
    SceneUiElement label{};
    SceneUiElement keyLabel{};
    int currentDefinition = -1;
    int currentQuality = -1;
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
};

static Il2CppObject* g_gameplayInventoryRoot = nullptr;
static Il2CppObject* g_gameplayQuickHudRoot = nullptr;
static Il2CppObject* g_gameplayInventoryStatus = nullptr;
static Il2CppObject* g_gameplayInventoryBackpackLabel = nullptr;
static Il2CppObject* g_gameplayInventorySafeBoxLabel = nullptr;
static SceneUiElement g_gameplayInventoryBackpackSlot{};
static SceneUiElement
    g_gameplayInventoryEquipmentSlots[kInventoryEquipmentCategoryCount]{};
static GameplayQuickSlotUi
    g_gameplayInventoryQuickSlots[kInventoryQuickSlotCount]{};
static GameplayQuickSlotUi
    g_gameplayHudQuickSlots[kInventoryQuickSlotCount]{};
static SceneUiElement g_gameplayInventoryPanels[kGameplayInventoryPanelCount]{};
static Il2CppObject*
    g_gameplayInventoryPanelTitles[kGameplayInventoryPanelCount]{};
static std::vector<GameplayInventoryView> g_gameplayInventoryViews;
static std::vector<GameplayInventoryTempItem> g_gameplayInventoryTempItems;
static GameplayInventoryGridCell
    g_gameplayInventoryGridCells[5][kGameplayInventoryMaxGridCells]{};
static unsigned int g_gameplayInventoryTempSerial = 0;
static int g_gameplayInventoryDraggingView = -1;
static bool g_gameplayInventoryMouseWasDown = false;
static bool g_gameplayInventoryDragMoved = false;
static bool g_gameplayInventoryTaskLayout = false;
static bool g_gameplayInventoryRewardMode = false;
static unsigned int g_gameplayInventoryObservedPressSerial = 0;
static unsigned int g_gameplayInventoryObservedReleaseSerial = 0;
static unsigned int g_gameplayInventoryObservedRightPressSerial = 0;
static float g_gameplayInventoryPressX = 0.0f;
static float g_gameplayInventoryPressY = 0.0f;
static int g_gameplayInventoryDragGrabCellX = 0;
static int g_gameplayInventoryDragGrabCellY = 0;
static float g_gameplayInventoryDragPointerOffsetPixelsX = 0.0f;
static float g_gameplayInventoryDragPointerOffsetPixelsY = 0.0f;
static float g_gameplayInventoryDragVisualWidth = 0.0f;
static float g_gameplayInventoryDragVisualHeight = 0.0f;
static int g_gameplayInventoryHoveredEquipmentCategory = -1;
static int g_gameplayInventoryQuickSlotDragSource = -1;
static bool g_gameplayInventoryRotateKeyDown = false;

void GameplayInventory_Close();

void GameplayInventory_GetPanel(
    int storage,
    float& minX,
    float& minY,
    float& maxX,
    float& maxY)
{
    switch (storage)
    {
    case kGameplayInventoryEquipped:
        minX = 0.010f; minY = 0.644f; maxX = 0.205f; maxY = 0.865f;
        break;
    case kGameplayInventoryBackpack:
        minX = 0.010f; minY = 0.338f; maxX = 0.205f; maxY = 0.632f;
        break;
    case kGameplayInventorySupply:
        minX = 0.700f;
        minY = 0.585f;
        maxX = 0.990f;
        maxY = 0.865f;
        break;
    case kGameplayInventorySafeBox:
        minX = 0.010f;
        minY = 0.105f;
        maxX = 0.205f;
        maxY = 0.326f;
        break;
    default:
        minX = 0.700f;
        minY = g_gameplayInventoryTaskLayout ? 0.115f : 0.115f;
        maxX = 0.990f;
        maxY = g_gameplayInventoryTaskLayout ? 0.575f : 0.865f;
        break;
    }
}

int GameplayInventory_StorageColumns(int storage)
{
    if (storage == kGameplayInventorySafeBox)
    {
        return ExhibitionUi_GetSafeBoxColumns();
    }
    if (storage != kGameplayInventoryBackpack)
    {
        return kGameplayInventoryColumns;
    }
    const int tier = InventoryUi_GetEquippedBackpackTier();
    return tier >= 0 ? kInventoryBackpackTiers[tier].columns : 0;
}

int GameplayInventory_StorageRows(int storage)
{
    if (storage == kGameplayInventorySafeBox)
    {
        return ExhibitionUi_GetSafeBoxRows();
    }
    if (storage != kGameplayInventoryBackpack)
    {
        return kGameplayInventoryRows;
    }
    const int tier = InventoryUi_GetEquippedBackpackTier();
    return tier >= 0 ? kInventoryBackpackTiers[tier].rows : 0;
}

void GameplayInventory_GetGridMetrics(
    int storage,
    float& minX,
    float& minY,
    float& maxX,
    float& maxY,
    float& cellWidth,
    float& cellHeight)
{
    float panelMinX = 0.0f;
    float panelMinY = 0.0f;
    float panelMaxX = 1.0f;
    float panelMaxY = 1.0f;
    GameplayInventory_GetPanel(
        storage,
        panelMinX,
        panelMinY,
        panelMaxX,
        panelMaxY);
    float clientWidth = 1920.0f;
    float clientHeight = 1080.0f;
    InventoryUi_ReadClientSize(clientWidth, clientHeight);
    clientWidth = std::max(1.0f, clientWidth);
    clientHeight = std::max(1.0f, clientHeight);
    const float panelWidth = panelMaxX - panelMinX;
    const float panelHeight = panelMaxY - panelMinY;
    const float insetX = (panelMaxX - panelMinX) * 0.025f;
    const float contentMinX = panelMinX + insetX;
    const float contentMaxX = panelMaxX - insetX;
    const float contentMinY = panelMinY + panelHeight * 0.045f;
    const float contentMaxY = panelMaxY - panelHeight * 0.105f;
    const float areaPixelsX =
        (contentMaxX - contentMinX) * clientWidth;
    const float areaPixelsY =
        (contentMaxY - contentMinY) * clientHeight;
    constexpr float slotGapCells = 0.14f;
    const int layoutColumns =
        storage == kGameplayInventorySafeBox
            ? ExhibitionUi_GetSafeBoxColumns()
            : kGameplayInventoryColumns;
    const int layoutRows =
        storage == kGameplayInventorySafeBox
            ? ExhibitionUi_GetSafeBoxRows()
            : kGameplayInventoryRows;
    const float cellPixels =
        storage == kGameplayInventoryBackpack
            ? std::fmin(
                areaPixelsX /
                    (static_cast<float>(kGameplayInventoryColumns + 1) +
                     slotGapCells),
                areaPixelsY /
                    static_cast<float>(kGameplayInventoryRows))
            : std::fmin(
                areaPixelsX /
                    static_cast<float>(std::max(1, layoutColumns)),
                areaPixelsY /
                    static_cast<float>(std::max(1, layoutRows)));
    cellWidth = std::max(1.0f, cellPixels) / clientWidth;
    cellHeight = std::max(1.0f, cellPixels) / clientHeight;
    const int columns = GameplayInventory_StorageColumns(storage);
    const int rows = GameplayInventory_StorageRows(storage);
    if (storage == kGameplayInventoryBackpack)
    {
        const float gap = slotGapCells * cellWidth;
        const float groupWidth =
            cellWidth +
            gap +
            static_cast<float>(columns) * cellWidth;
        const float centerX = (contentMinX + contentMaxX) * 0.5f;
        minX =
            centerX - groupWidth * 0.5f +
            cellWidth +
            gap;
        maxX = minX + static_cast<float>(columns) * cellWidth;
        maxY = contentMaxY;
        minY = maxY - static_cast<float>(rows) * cellHeight;
        return;
    }
    const float gridWidth =
        static_cast<float>(columns) * cellWidth;
    const float centerX = (contentMinX + contentMaxX) * 0.5f;
    minX = centerX - gridWidth * 0.5f;
    maxX = minX + gridWidth;
    maxY = contentMaxY;
    minY = maxY - static_cast<float>(rows) * cellHeight;
}

void GameplayInventory_GetBackpackSlotRect(
    float& minX,
    float& minY,
    float& maxX,
    float& maxY)
{
    float gridMinX = 0.0f;
    float gridMinY = 0.0f;
    float gridMaxX = 0.0f;
    float gridMaxY = 0.0f;
    float cellWidth = 0.0f;
    float cellHeight = 0.0f;
    GameplayInventory_GetGridMetrics(
        kGameplayInventoryBackpack,
        gridMinX,
        gridMinY,
        gridMaxX,
        gridMaxY,
        cellWidth,
        cellHeight);
    constexpr float slotGapCells = 0.14f;
    maxX = gridMinX - slotGapCells * cellWidth;
    minX = maxX - cellWidth;
    maxY = gridMaxY;
    minY = maxY - cellHeight;
}

void GameplayInventory_GetEquipmentSlotRect(
    int category,
    float& minX,
    float& minY,
    float& maxX,
    float& maxY)
{
    const int column = category % kGameplayInventoryEquipmentColumns;
    const int row = category / kGameplayInventoryEquipmentColumns;
    float panelMinX = 0.0f;
    float panelMinY = 0.0f;
    float panelMaxX = 0.0f;
    float panelMaxY = 0.0f;
    GameplayInventory_GetPanel(
        kGameplayInventoryEquipped,
        panelMinX,
        panelMinY,
        panelMaxX,
        panelMaxY);
    const float panelWidth = panelMaxX - panelMinX;
    const float panelHeight = panelMaxY - panelMinY;
    const float slotMinX = panelMinX + panelWidth * 0.025f;
    const float slotMaxX = panelMaxX - panelWidth * 0.025f;
    const float slotMinY = panelMinY + panelHeight * 0.055f;
    const float slotMaxY = panelMaxY - panelHeight * 0.105f;
    float clientWidth = 1920.0f;
    float clientHeight = 1080.0f;
    InventoryUi_ReadClientSize(clientWidth, clientHeight);
    const float cellPixels = std::fmin(
        (slotMaxX - slotMinX) * clientWidth /
            static_cast<float>(kGameplayInventoryEquipmentColumns),
        (slotMaxY - slotMinY) * clientHeight /
            static_cast<float>(kGameplayInventoryEquipmentRows));
    const float cellWidth =
        std::max(1.0f, cellPixels) / std::max(1.0f, clientWidth);
    const float cellHeight =
        std::max(1.0f, cellPixels) / std::max(1.0f, clientHeight);
    const float gridWidth =
        cellWidth *
        static_cast<float>(kGameplayInventoryEquipmentColumns);
    const float gridCenterX = (slotMinX + slotMaxX) * 0.5f;
    minX =
        gridCenterX - gridWidth * 0.5f +
        static_cast<float>(column) * cellWidth + 0.002f;
    maxX = minX + cellWidth - 0.004f;
    maxY = slotMaxY - static_cast<float>(row) * cellHeight - 0.002f;
    minY = maxY - cellHeight + 0.004f;
}

int GameplayInventory_FindEquipmentCategory(
    int definitionIndex)
{
    if (definitionIndex < 0 ||
        definitionIndex >= kEquipmentInventoryDefinitionCount)
    {
        return -1;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[definitionIndex];
    if (InventoryUi_IsBackpackDefinition(definitionIndex))
    {
        return kInventoryBackpackCategoryIndex;
    }
    if (definition.categories == 0)
    {
        return -1;
    }
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        if ((definition.categories &
             kInventoryEquipmentCategories[category].bit) != 0)
        {
            return category;
        }
    }
    return -1;
}

int GameplayInventory_EquipmentSlotAt(float x, float y)
{
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        GameplayInventory_GetEquipmentSlotRect(
            category,
            minX,
            minY,
            maxX,
            maxY);
        if (x >= minX && x <= maxX && y >= minY && y <= maxY)
        {
            return category;
        }
    }
    float slotMinX = 0.0f;
    float slotMinY = 0.0f;
    float slotMaxX = 0.0f;
    float slotMaxY = 0.0f;
    GameplayInventory_GetBackpackSlotRect(
        slotMinX,
        slotMinY,
        slotMaxX,
        slotMaxY);
    if (x >= slotMinX && x <= slotMaxX &&
        y >= slotMinY && y <= slotMaxY)
    {
        return kInventoryBackpackCategoryIndex;
    }
    return -1;
}

int GameplayInventory_GridAt(
    float x,
    float y,
    int& column,
    int& row)
{
    for (int storage = kGameplayInventoryBackpack;
         storage <= kGameplayInventorySafeBox;
         ++storage)
    {
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        float cellWidth = 0.0f;
        float cellHeight = 0.0f;
        GameplayInventory_GetGridMetrics(
            storage,
            minX,
            minY,
            maxX,
            maxY,
            cellWidth,
            cellHeight);
        if (x < minX || x > maxX || y < minY || y > maxY)
        {
            continue;
        }
        const int columns = GameplayInventory_StorageColumns(storage);
        const int rows = GameplayInventory_StorageRows(storage);
        if (columns <= 0 || rows <= 0)
        {
            continue;
        }
        column = std::clamp(
            static_cast<int>((x - minX) / cellWidth),
            0,
            columns - 1);
        row = std::clamp(
            static_cast<int>((maxY - y) / cellHeight),
            0,
            rows - 1);
        if (storage == kGameplayInventorySafeBox &&
            !ExhibitionUi_IsSafeBoxCellActive(column, row))
        {
            continue;
        }
        return storage;
    }
    return -1;
}

bool GameplayInventory_ResolveGridDropCell(
    int storage,
    float cursorX,
    float cursorY,
    int itemWidth,
    int itemHeight,
    int& outputColumn,
    int& outputRow)
{
    const int columns = GameplayInventory_StorageColumns(storage);
    const int rows = GameplayInventory_StorageRows(storage);
    if (columns <= 0 || rows <= 0 ||
        itemWidth <= 0 || itemHeight <= 0 ||
        itemWidth > columns || itemHeight > rows)
    {
        return false;
    }
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float cellWidth = 0.0f;
    float cellHeight = 0.0f;
    GameplayInventory_GetGridMetrics(
        storage,
        minX,
        minY,
        maxX,
        maxY,
        cellWidth,
        cellHeight);
    if (cursorX < minX || cursorX > maxX ||
        cursorY < minY || cursorY > maxY ||
        cellWidth <= 0.0f || cellHeight <= 0.0f)
    {
        return false;
    }
    const int hoveredColumn = std::clamp(
        static_cast<int>((cursorX - minX) / cellWidth),
        0,
        columns - 1);
    const int hoveredRow = std::clamp(
        static_cast<int>((maxY - cursorY) / cellHeight),
        0,
        rows - 1);
    outputColumn = std::clamp(
        hoveredColumn - g_gameplayInventoryDragGrabCellX,
        0,
        columns - itemWidth);
    outputRow = std::clamp(
        hoveredRow - g_gameplayInventoryDragGrabCellY,
        0,
        rows - itemHeight);
    if (storage == kGameplayInventorySafeBox)
    {
        for (int row = outputRow;
             row < outputRow + itemHeight;
             ++row)
        {
            for (int column = outputColumn;
                 column < outputColumn + itemWidth;
                 ++column)
            {
                if (!ExhibitionUi_IsSafeBoxCellActive(column, row))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool GameplayInventory_InsideBackpackPanel(float x, float y)
{
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    GameplayInventory_GetPanel(
        kGameplayInventoryBackpack,
        minX,
        minY,
        maxX,
        maxY);
    return x >= minX && x <= maxX &&
           y >= minY && y <= maxY;
}

void GameplayInventory_SetRectForGrid(
    int storage,
    int x,
    int y,
    int width,
    int height,
    float& minX,
    float& minY,
    float& maxX,
    float& maxY)
{
    float gridMinX = 0.0f;
    float gridMinY = 0.0f;
    float gridMaxX = 0.0f;
    float gridMaxY = 0.0f;
    float cellWidth = 0.0f;
    float cellHeight = 0.0f;
    GameplayInventory_GetGridMetrics(
        storage,
        gridMinX,
        gridMinY,
        gridMaxX,
        gridMaxY,
        cellWidth,
        cellHeight);
    minX = gridMinX + static_cast<float>(x) * cellWidth + 0.002f;
    maxX = gridMinX + static_cast<float>(x + width) * cellWidth - 0.002f;
    maxY = gridMaxY - static_cast<float>(y) * cellHeight - 0.002f;
    minY = gridMaxY - static_cast<float>(y + height) * cellHeight + 0.002f;
}

void GameplayInventory_SetStatus(const char* text)
{
    if (g_gameplayInventoryStatus == nullptr || text == nullptr)
    {
        return;
    }
    SetSceneText(
        g_gameplayInventoryStatus,
        text,
        14.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
}

void GameplayInventory_EnsureState()
{
    if (!g_inventoryStateLoaded)
    {
        InventoryUi_InitializeItems();
        const InventoryStateLoadResult loadResult = InventoryUi_LoadState();
        InventoryUi_EnsureStarterGrant(loadResult);
        InventoryUi_ApplyBackpackCapacity();
    }
}

void GameplayInventory_CreateQuickSlotSet(
    Il2CppObject* parent,
    GameplayQuickSlotUi* slots,
    const char* prefix)
{
    float clientWidth = 1920.0f;
    float clientHeight = 1080.0f;
    InventoryUi_ReadClientSize(clientWidth, clientHeight);
    clientWidth = std::max(1.0f, clientWidth);
    clientHeight = std::max(1.0f, clientHeight);
    const float squarePixels = std::max(
        64.0f,
        clientHeight * 0.078f);
    const float gapPixels = std::max(
        6.0f,
        clientHeight * 0.007f);
    const float groupPixels =
        squarePixels * kInventoryQuickSlotCount +
        gapPixels * (kInventoryQuickSlotCount - 1);
    const float groupMinPixels =
        (clientWidth - groupPixels) * 0.5f;
    const float minY =
        std::strcmp(prefix, "GameplayHudQuickSlot") == 0
            ? 0.025f
            : 0.902f;
    const float width = squarePixels / clientWidth;
    const float height = squarePixels / clientHeight;
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        char name[96]{};
        std::snprintf(
            name,
            sizeof(name),
            "%s_%d",
            prefix,
            index + 1);
        auto& slot = slots[index];
        slot.minX =
            (groupMinPixels +
             index * (squarePixels + gapPixels)) /
            clientWidth;
        slot.maxX = slot.minX + width;
        slot.minY = minY;
        slot.maxY = minY + height;
        slot.panel = CreateScenePanel(
            name,
            parent,
            slot.minX,
            slot.minY,
            slot.maxX,
            slot.maxY,
            0.045f,
            0.052f,
            0.065f,
            0.96f);
        slot.icon = CreateSceneRawImageElement(
            "Icon",
            slot.panel.rect,
            L"",
            0.04f,
            0.24f,
            0.96f,
            0.97f,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        slot.label = CreateSceneTextElement(
            "Label",
            slot.panel.rect,
            "空",
            0.03f,
            0.015f,
            0.97f,
            0.235f,
            8.0f,
            260,
            0.84f,
            0.88f,
            0.93f,
            1.0f);
        char key[4]{};
        std::snprintf(key, sizeof(key), "%d", index + 1);
        slot.keyLabel = CreateSceneTextElement(
            "Key",
            slot.panel.rect,
            key,
            0.035f,
            0.735f,
            0.24f,
            0.985f,
            14.0f,
            257,
            1.0f,
            0.90f,
            0.65f,
            1.0f);
        SetWorldGameObjectActive(slot.icon.gameObject, false);
    }
}

void GameplayInventory_UpdateQuickSlotSet(
    GameplayQuickSlotUi* slots)
{
    for (int quickIndex = 0;
         quickIndex < kInventoryQuickSlotCount;
         ++quickIndex)
    {
        auto& slot = slots[quickIndex];
        const int itemIndex =
            InventoryUi_FindQuickSlotBoundItem(quickIndex);
        if (itemIndex < 0)
        {
            slot.currentDefinition = -1;
            slot.currentQuality = -1;
            SetWorldGameObjectActive(slot.icon.gameObject, false);
            SetSceneText(
                slot.label.text,
                "空",
                8.0f,
                260,
                0.60f,
                0.65f,
                0.70f,
                1.0f);
            SetSceneGraphicColor(
                slot.panel.image,
                0.045f,
                0.052f,
                0.065f,
                0.96f);
            continue;
        }
        const auto& item = g_inventoryItems[itemIndex];
        const auto& definition =
            kEquipmentInventoryDefinitions[item.definition];
        const int quality = std::clamp(
            item.quality,
            0,
            kInventoryQualityCount - 1);
        if (slot.currentDefinition != item.definition ||
            slot.currentQuality != quality)
        {
            slot.currentDefinition = item.definition;
            slot.currentQuality = quality;
            slot.icon.texture = LoadScenePngTexture(
                InventoryUi_EquipmentImagePath(
                    definition.id,
                    quality));
        }
        SetWorldGameObjectActive(slot.icon.gameObject, true);
        SetSceneRawImageTexture(
            slot.icon.rawImage,
            slot.icon.texture);
        SetSceneText(
            slot.label.text,
            definition.name,
            8.0f,
            260,
            0.96f,
            0.94f,
            0.88f,
            1.0f);
        const auto& tint = kInventoryQualityColors[quality];
        SetSceneGraphicColor(
            slot.panel.image,
            0.030f + tint[0] * 0.16f,
            0.034f + tint[1] * 0.16f,
            0.045f + tint[2] * 0.16f,
            0.96f);
    }
}

void GameplayInventory_UpdateQuickSlots()
{
    GameplayInventory_UpdateQuickSlotSet(
        g_gameplayInventoryQuickSlots);
    GameplayInventory_UpdateQuickSlotSet(
        g_gameplayHudQuickSlots);
}

void GameplayInventory_ApplyItemIconRotation(
    const GameplayInventoryView& view)
{
    if (view.icon.rect == nullptr ||
        view.definition < 0 ||
        view.definition >= kEquipmentInventoryDefinitionCount)
    {
        return;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[view.definition];
    const bool rotated =
        definition.width != definition.height &&
        view.width == definition.height &&
        view.height == definition.width;
    constexpr float minX = 0.04f;
    constexpr float minY = 0.20f;
    constexpr float maxX = 0.96f;
    constexpr float maxY = 0.98f;
    if (!rotated)
    {
        SetTransformLocalEuler(view.icon.rect, 0.0f, 0.0f, 0.0f);
        SetSceneRect(view.icon.rect, minX, minY, maxX, maxY);
        return;
    }
    const float centerX = (minX + maxX) * 0.5f;
    const float centerY = (minY + maxY) * 0.5f;
    const float width =
        (maxX - minX) *
        static_cast<float>(view.height) /
        static_cast<float>(view.width);
    const float height =
        (maxY - minY) *
        static_cast<float>(view.width) /
        static_cast<float>(view.height);
    SetTransformLocalEuler(view.icon.rect, 0.0f, 0.0f, 90.0f);
    SetSceneRect(
        view.icon.rect,
        centerX - width * 0.5f,
        centerY - height * 0.5f,
        centerX + width * 0.5f,
        centerY + height * 0.5f);
}

void GameplayInventory_EnsureView(int index)
{
    while (static_cast<int>(g_gameplayInventoryViews.size()) <= index)
    {
        g_gameplayInventoryViews.push_back(GameplayInventoryView{});
    }
    auto& view = g_gameplayInventoryViews[static_cast<size_t>(index)];
    if (view.root.gameObject != nullptr)
    {
        return;
    }
    char name[64]{};
    std::snprintf(name, sizeof(name), "GameplayInventoryItem_%d", index);
    view.root = CreateScenePanel(
        name,
        GetWorldTransform(g_gameplayInventoryRoot),
        0.0f,
        0.0f,
        0.01f,
        0.01f,
        0.055f,
        0.070f,
        0.090f,
        0.98f);
    view.icon = CreateSceneRawImageElement(
        "Icon",
        view.root.rect,
        std::wstring(),
        0.04f,
        0.20f,
        0.96f,
        0.98f,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    view.appraisalBadgeCircle = CreateSceneTextElement(
        "AppraisalBadgeCircle",
        view.root.rect,
        "●",
        0.005f,
        0.780f,
        0.255f,
        0.995f,
        22.0f,
        514,
        0.62f,
        0.65f,
        0.68f,
        1.0f);
    view.appraisalBadgeText = CreateSceneTextElement(
        "AppraisalBadgeText",
        view.appraisalBadgeCircle.rect,
        "",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        10.0f,
        514,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    SetWorldGameObjectActive(
        view.appraisalBadgeCircle.gameObject,
        false);
    SetWorldGameObjectActive(
        view.appraisalBadgeText.gameObject,
        false);
    view.label = CreateSceneTextElement(
        "Label",
        view.root.rect,
        "",
        0.04f,
        0.01f,
        0.96f,
        0.20f,
        10.0f,
        514,
        0.92f,
        0.94f,
        0.97f,
        1.0f);
}

bool GameplayInventory_CanEquipDefinition(
    int definitionIndex,
    int category,
    int excludingPersistentIndex)
{
    (void)excludingPersistentIndex;
    return InventoryUi_CanEquip(definitionIndex, category);
}

void GameplayInventory_UpdateEquipmentHighlights(int hoveredCategory)
{
    int draggedDefinition = -1;
    if (g_gameplayInventoryDraggingView >= 0 &&
        g_gameplayInventoryDraggingView <
            static_cast<int>(g_gameplayInventoryViews.size()))
    {
        draggedDefinition =
            g_gameplayInventoryViews[
                static_cast<size_t>(
                    g_gameplayInventoryDraggingView)].definition;
    }
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        auto& slot = g_gameplayInventoryEquipmentSlots[category];
        if (slot.image == nullptr)
        {
            continue;
        }
        bool eligible = false;
        if (draggedDefinition >= 0)
        {
            const auto& definition =
                kEquipmentInventoryDefinitions[draggedDefinition];
            eligible =
                (definition.categories &
                 kInventoryEquipmentCategories[category].bit) != 0;
        }
        const bool hovered =
            draggedDefinition >= 0 &&
            category == hoveredCategory;
        float red = 0.060f;
        float green = 0.068f;
        float blue = 0.080f;
        if (eligible || hovered)
        {
            const bool canEquip =
                draggedDefinition >= 0 &&
                GameplayInventory_CanEquipDefinition(
                    draggedDefinition,
                    category,
                    draggedDefinition);
            red = canEquip ? (hovered ? 0.24f : 0.16f) : 0.50f;
            green = canEquip ? (hovered ? 0.56f : 0.42f) : 0.10f;
            blue = canEquip ? (hovered ? 0.25f : 0.16f) : 0.10f;
        }
        SetSceneGraphicColor(slot.image, red, green, blue, 0.98f);
    }

    auto& backpackSlot = g_gameplayInventoryBackpackSlot;
    if (backpackSlot.image != nullptr)
    {
        const bool eligible =
            draggedDefinition >= 0 &&
            InventoryUi_IsBackpackDefinition(draggedDefinition);
        const bool hovered =
            draggedDefinition >= 0 &&
            hoveredCategory == kInventoryBackpackCategoryIndex;
        float red = 0.060f;
        float green = 0.068f;
        float blue = 0.080f;
        if (eligible || hovered)
        {
            const bool canEquip =
                draggedDefinition >= 0 &&
                GameplayInventory_CanEquipDefinition(
                    draggedDefinition,
                    kInventoryBackpackCategoryIndex,
                    draggedDefinition);
            red = canEquip ? (hovered ? 0.24f : 0.16f) : 0.50f;
            green = canEquip ? (hovered ? 0.56f : 0.42f) : 0.10f;
            blue = canEquip ? (hovered ? 0.25f : 0.16f) : 0.10f;
        }
        SetSceneGraphicColor(
            backpackSlot.image,
            red,
            green,
            blue,
            0.98f);
    }
}

bool GameplayInventory_InsideEquipmentPanel(float x, float y)
{
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    GameplayInventory_GetPanel(
        kGameplayInventoryEquipped,
        minX,
        minY,
        maxX,
        maxY);
    return x >= minX && x <= maxX &&
           y >= minY && y <= maxY;
}

bool GameplayInventory_CellFree(
    int storage,
    int x,
    int y,
    int width,
    int height,
    int excludingView,
    int excludingTempIndex)
{
    if (x < 0 || y < 0 ||
        x + width > GameplayInventory_StorageColumns(storage) ||
        y + height > GameplayInventory_StorageRows(storage))
    {
        return false;
    }
    if (storage == kGameplayInventorySafeBox)
    {
        for (int row = y; row < y + height; ++row)
        {
            for (int column = x; column < x + width; ++column)
            {
                if (!ExhibitionUi_IsSafeBoxCellActive(column, row))
                {
                    return false;
                }
            }
        }
    }
    for (size_t index = 0;
         index < g_gameplayInventoryViews.size();
         ++index)
    {
        const int viewIndex = static_cast<int>(index);
        if (viewIndex == excludingView)
        {
            continue;
        }
        const auto& other = g_gameplayInventoryViews[index];
        if (other.definition < 0 || other.storage != storage)
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
    (void)excludingTempIndex;
    return true;
}

bool GameplayInventory_FindFreeCell(
    int storage,
    int width,
    int height,
    int& outputX,
    int& outputY)
{
    const int columns = GameplayInventory_StorageColumns(storage);
    const int rows = GameplayInventory_StorageRows(storage);
    if (columns <= 0 || rows <= 0)
    {
        return false;
    }
    for (int y = 0;
         y <= rows - height;
         ++y)
    {
        for (int x = 0;
             x <= columns - width;
             ++x)
        {
            if (GameplayInventory_CellFree(
                    storage,
                    x,
                    y,
                    width,
                    height,
                    -1,
                    -1))
            {
                outputX = x;
                outputY = y;
                return true;
            }
        }
    }
    return false;
}

void GameplayInventory_UpdateGridCells()
{
    for (int storage = 0;
         storage < kGameplayInventoryPanelCount;
         ++storage)
    {
        if (g_gameplayInventoryPanels[storage].gameObject == nullptr)
        {
            continue;
        }
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        GameplayInventory_GetPanel(
            storage,
            minX,
            minY,
            maxX,
            maxY);
        SetSceneRect(
            g_gameplayInventoryPanels[storage].rect,
            minX,
            minY,
            maxX,
            maxY);
        SetWorldGameObjectActive(
            g_gameplayInventoryPanels[storage].gameObject,
            storage != kGameplayInventorySupply ||
                g_gameplayInventoryTaskLayout);
    }
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        auto& slot = g_gameplayInventoryEquipmentSlots[category];
        if (slot.gameObject == nullptr)
        {
            continue;
        }
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        GameplayInventory_GetEquipmentSlotRect(
            category,
            minX,
            minY,
            maxX,
            maxY);
        SetSceneRect(slot.rect, minX, minY, maxX, maxY);
        SetWorldGameObjectActive(slot.gameObject, true);
    }
    for (int storage = kGameplayInventoryBackpack;
         storage < kGameplayInventoryPanelCount;
         ++storage)
    {
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        float cellWidth = 0.0f;
        float cellHeight = 0.0f;
        GameplayInventory_GetGridMetrics(
            storage,
            minX,
            minY,
            maxX,
            maxY,
            cellWidth,
            cellHeight);
        const int columns = GameplayInventory_StorageColumns(storage);
        const int rows = GameplayInventory_StorageRows(storage);
        for (int index = 0;
             index < kGameplayInventoryMaxGridCells;
             ++index)
        {
            auto& cell =
                g_gameplayInventoryGridCells[storage][index];
            if (cell.panel.gameObject == nullptr)
            {
                continue;
            }
            const bool visible =
                columns > 0 && rows > 0 &&
                (storage != kGameplayInventorySupply ||
                 g_gameplayInventoryTaskLayout) &&
                cell.column < columns &&
                cell.row < rows &&
                (storage != kGameplayInventorySafeBox ||
                 ExhibitionUi_IsSafeBoxCellActive(
                     cell.column,
                     cell.row));
            SetWorldGameObjectActive(
                cell.panel.gameObject,
                visible);
            if (!visible)
            {
                continue;
            }
            constexpr float gapPixels = 2.0f;
            float clientWidth = 1920.0f;
            float clientHeight = 1080.0f;
            InventoryUi_ReadClientSize(clientWidth, clientHeight);
            const float gapX =
                gapPixels / std::max(1.0f, clientWidth);
            const float gapY =
                gapPixels / std::max(1.0f, clientHeight);
            const float cellMinX =
                minX + static_cast<float>(cell.column) * cellWidth;
            const float cellMaxX = cellMinX + cellWidth;
            const float cellMinY =
                maxY - static_cast<float>(cell.row + 1) * cellHeight;
            const float cellMaxY = cellMinY + cellHeight;
            SetSceneRect(
                cell.panel.rect,
                cellMinX + gapX,
                cellMinY + gapY,
                cellMaxX - gapX,
                cellMaxY - gapY);
        }
    }
    float slotMinX = 0.0f;
    float slotMinY = 0.0f;
    float slotMaxX = 0.0f;
    float slotMaxY = 0.0f;
    GameplayInventory_GetBackpackSlotRect(
        slotMinX,
        slotMinY,
        slotMaxX,
        slotMaxY);
    SetSceneRect(
        g_gameplayInventoryBackpackSlot.rect,
        slotMinX,
        slotMinY,
        slotMaxX,
        slotMaxY);
}

void GameplayInventory_Build()
{
    if (g_gameplayInventoryRoot != nullptr &&
        ExtendedMode_IsUnityObjectAlive(
            g_gameplayInventoryRoot))
    {
        return;
    }
    g_gameplayInventoryRoot = nullptr;
    if (g_gameplayQuickHudRoot != nullptr &&
        !ExtendedMode_IsUnityObjectAlive(
            g_gameplayQuickHudRoot))
    {
        g_gameplayQuickHudRoot = nullptr;
    }
    if (!ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    // The legacy gameplay canvas is hidden when the black-room preview ends.
    // Attach raid UI to the persistent, visible gameplay HUD instead.
    auto* canvasTransform = GetWorldTransform(
        g_extendedGameplayHudCanvasObject);
    auto root = CreateScenePanel(
        "GameplayInventoryRoot",
        canvasTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.018f,
        0.026f,
        0.040f,
        0.0f);
    g_gameplayInventoryRoot = root.gameObject;
    BringSceneObjectToFront(g_gameplayInventoryRoot);
    auto* rootTransform = root.rect;
    auto quickHudRoot = CreateScenePanel(
        "GameplayQuickBarHudRoot",
        canvasTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    g_gameplayQuickHudRoot = quickHudRoot.gameObject;
    auto* quickHudTransform = quickHudRoot.rect;
    BringSceneObjectToFront(g_gameplayQuickHudRoot);

    CreateScenePanel(
        "GameplayInventoryHeader",
        rootTransform,
        0.015f,
        0.885f,
        0.985f,
        0.985f,
        0.055f,
        0.075f,
        0.105f,
        1.0f);
    CreateSceneTextElement(
        "GameplayInventoryTitle",
        rootTransform,
        "物品栏    TAB 关闭",
        0.030f,
        0.897f,
        0.330f,
        0.974f,
        26.0f,
        513,
        0.95f,
        0.96f,
        0.98f,
        1.0f);
    CreateSceneTextElement(
        "GameplayInventoryHint",
        rootTransform,
        "拖动物品   装备/背包即时存档   整理区仅本局临时",
        0.650f,
        0.904f,
        0.970f,
        0.965f,
        13.0f,
        514,
        0.64f,
        0.70f,
        0.78f,
        1.0f);
    GameplayInventory_CreateQuickSlotSet(
        rootTransform,
        g_gameplayInventoryQuickSlots,
        "GameplayTabQuickSlot");
    GameplayInventory_CreateQuickSlotSet(
        quickHudTransform,
        g_gameplayHudQuickSlots,
        "GameplayHudQuickSlot");
    SetWorldGameObjectActive(g_gameplayQuickHudRoot, true);

    const char* labels[] = {
        "装备",
        "背包",
        "物资区  6×8  ·  本局",
        "整理区  6×8  ·  本局临时",
        "安全箱",
    };
    for (int storage = 0;
         storage < kGameplayInventoryPanelCount;
         ++storage)
    {
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        GameplayInventory_GetPanel(
            storage,
            minX,
            minY,
            maxX,
            maxY);
        auto panel = CreateScenePanel(
            "GameplayInventoryPanel",
            rootTransform,
            minX,
            minY,
            maxX,
            maxY,
            0.035f,
            0.046f,
            0.061f,
            0.98f);
        g_gameplayInventoryPanels[storage] = panel;
        auto title = CreateSceneTextElement(
            "PanelTitle",
            panel.rect,
            labels[storage],
            0.04f,
            0.905f,
            0.96f,
            0.985f,
            17.0f,
            513,
            0.90f,
            0.92f,
            0.96f,
            1.0f);
        g_gameplayInventoryPanelTitles[storage] = title.text;
        if (storage == kGameplayInventoryBackpack)
        {
            g_gameplayInventoryBackpackLabel = title.text;
        }
        else if (storage == kGameplayInventorySafeBox)
        {
            g_gameplayInventorySafeBoxLabel = title.text;
        }
    }

    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        g_gameplayInventoryEquipmentSlots[category] = CreateScenePanel(
            "GameplayInventoryEquipmentSlot",
            rootTransform,
            0.0f,
            0.0f,
            0.001f,
            0.001f,
            0.060f,
            0.068f,
            0.080f,
            0.96f);
    }

    for (int storage = kGameplayInventoryBackpack;
         storage < kGameplayInventoryPanelCount;
         ++storage)
    {
        for (int index = 0;
             index < kGameplayInventoryMaxGridCells;
             ++index)
        {
            auto& cell =
                g_gameplayInventoryGridCells[storage][index];
            cell.storage = storage;
            cell.column = index % kGameplayInventoryColumns;
            cell.row = index / kGameplayInventoryColumns;
            char cellName[80]{};
            std::snprintf(
                cellName,
                sizeof(cellName),
                "GameplayInventoryCell_%d_%d_%d",
                storage,
                cell.column,
                cell.row);
            cell.panel = CreateScenePanel(
                cellName,
                rootTransform,
                0.0f,
                0.0f,
                0.001f,
                0.001f,
                0.082f,
                0.095f,
                0.115f,
                0.98f);
        }
    }
    g_gameplayInventoryBackpackSlot = CreateScenePanel(
        "GameplayInventoryEquippedBackpackSlot",
        rootTransform,
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        0.060f,
        0.070f,
        0.085f,
        0.98f);
    CreateSceneTextElement(
        "EquippedBackpackSlotLabel",
        g_gameplayInventoryBackpackSlot.rect,
        "装备背包",
        0.08f,
        0.08f,
        0.92f,
        0.28f,
        10.0f,
        514,
        0.64f,
        0.70f,
        0.78f,
        1.0f);
    GameplayInventory_UpdateGridCells();

    g_gameplayInventoryStatus = CreateSceneTextElement(
        "GameplayInventoryStatus",
        rootTransform,
        "TAB 关闭物品栏",
        0.025f,
        0.010f,
        0.975f,
        0.060f,
        14.0f,
        513,
        0.76f,
        0.82f,
        0.90f,
        1.0f).text;
    GameplayInventory_UpdateQuickSlots();
    SetWorldGameObjectActive(g_gameplayInventoryRoot, false);
    LogInfo(LogCategory_Inventory, L"Inventory", L"Gameplay Tab inventory shell created.");
}

void GameplayInventory_Refresh()
{
    GameplayInventory_EnsureState();
    GameplayInventory_Build();
    GameplayInventory_UpdateGridCells();
    if (g_gameplayInventoryBackpackLabel != nullptr)
    {
        const int tier = InventoryUi_GetEquippedBackpackTier();
        char label[160]{};
        if (tier >= 0)
        {
            std::snprintf(
                label,
                sizeof(label),
                "背包  %d×%d  ·  %d 格",
                kInventoryBackpackTiers[tier].columns,
                kInventoryBackpackTiers[tier].rows,
                kInventoryBackpackTiers[tier].columns *
                    kInventoryBackpackTiers[tier].rows);
        }
        else
        {
            std::snprintf(
                label,
                sizeof(label),
                "背包  未装备  ·  0×0");
        }
        SetSceneText(
            g_gameplayInventoryBackpackLabel,
            label,
            17.0f,
            513,
            0.90f,
            0.92f,
            0.96f,
            1.0f);
    }
    if (g_gameplayInventorySafeBoxLabel != nullptr)
    {
        char label[160]{};
        std::snprintf(
            label,
            sizeof(label),
            "安全箱  %d×%d  ·  %d 格  ·  Lv.%d",
            ExhibitionUi_GetSafeBoxColumns(),
            ExhibitionUi_GetSafeBoxRows(),
            ExhibitionUi_GetSafeBoxCells(),
            ExhibitionUi_GetCurrentLevel());
        SetSceneText(
            g_gameplayInventorySafeBoxLabel,
            label,
            15.0f,
            513,
            0.94f,
            0.85f,
            0.64f,
            1.0f);
    }
    GameplayInventory_UpdateQuickSlots();
    int viewCount = 0;
    int persistentViewCount = 0;
    auto addPersistentView = [&](int itemIndex, int equipmentCategory)
    {
        GameplayInventory_EnsureView(viewCount);
        auto& view =
            g_gameplayInventoryViews[static_cast<size_t>(viewCount)];
        const auto& item = g_inventoryItems[itemIndex];
        view.definition = item.definition;
        view.quality = item.quality;
        view.appraised = item.appraised;
        view.wearUnits = item.wearUnits;
        view.itemIndex = itemIndex;
        view.storage = item.storage == kInventoryStorageEquipped
            ? kGameplayInventoryEquipped
            : (item.storage == kInventoryStorageSafeBox
                ? kGameplayInventorySafeBox
                : kGameplayInventoryBackpack);
        view.equipmentCategory = equipmentCategory;
        view.x = item.x;
        view.y = item.y;
        view.width = item.width;
        view.height = item.height;
        view.tempIndex = -1;
        ++viewCount;
    };
    for (int itemIndex = 0;
         itemIndex < kInventoryItemSlotCount;
         ++itemIndex)
    {
        const auto& item = g_inventoryItems[itemIndex];
        // The raid panel exposes the live equipped and backpack storage only.
        // Warehouse/stash records stay in the separate warehouse UI.
        if (item.storage != kInventoryStorageEquipped &&
            item.storage != kInventoryStorageBackpack &&
            item.storage != kInventoryStorageSafeBox)
        {
            continue;
        }
        if (item.storage != kInventoryStorageEquipped ||
            InventoryUi_IsBackpackDefinition(item.definition))
        {
            addPersistentView(itemIndex, -1);
            continue;
        }
        const auto& definition =
            kEquipmentInventoryDefinitions[item.definition];
        const uint32_t equipmentBits =
            definition.categories & kInventoryEquipmentCategoryMask;
        if (equipmentBits == 0)
        {
            addPersistentView(itemIndex, -1);
            continue;
        }
        // The warehouse renders one equipped item in every category slot it
        // occupies. Mirror that here so tops covering both upper and lower
        // body slots are displayed twice in the raid Tab panel as well.
        for (int category = 0;
             category < kInventoryEquipmentCategoryCount;
             ++category)
        {
            if ((equipmentBits &
                 kInventoryEquipmentCategories[category].bit) != 0)
            {
                addPersistentView(itemIndex, category);
            }
        }
    }
    persistentViewCount = viewCount;
    for (size_t tempIndex = 0;
         tempIndex < g_gameplayInventoryTempItems.size();
         ++tempIndex)
    {
        const auto& temp = g_gameplayInventoryTempItems[tempIndex];
        if (temp.definition < 0 ||
            temp.definition >= kEquipmentInventoryDefinitionCount)
        {
            continue;
        }
        GameplayInventory_EnsureView(viewCount);
        auto& view =
            g_gameplayInventoryViews[static_cast<size_t>(viewCount)];
        view.definition = temp.definition;
        view.quality = temp.quality;
        view.appraised = temp.appraised;
        view.wearUnits = temp.wearUnits;
        view.itemIndex = -1;
        view.storage = temp.storage;
        view.equipmentCategory = -1;
        view.x = temp.x;
        view.y = temp.y;
        view.width = temp.width;
        view.height = temp.height;
        view.tempIndex = static_cast<int>(tempIndex);
        ++viewCount;
    }

    for (int index = 0; index < viewCount; ++index)
    {
        auto& view =
            g_gameplayInventoryViews[static_cast<size_t>(index)];
        const auto& definition =
            kEquipmentInventoryDefinitions[view.definition];
        if (view.storage == kGameplayInventorySupply &&
            !g_gameplayInventoryTaskLayout)
        {
            SetWorldGameObjectActive(view.root.gameObject, false);
            continue;
        }
        if (view.storage == kGameplayInventoryEquipped)
        {
            if (InventoryUi_IsBackpackDefinition(view.definition))
            {
                GameplayInventory_GetBackpackSlotRect(
                    view.minX,
                    view.minY,
                    view.maxX,
                    view.maxY);
            }
            else
            {
                const int category =
                    view.equipmentCategory >= 0
                        ? view.equipmentCategory
                        : GameplayInventory_FindEquipmentCategory(
                            view.definition);
                if (category < 0 ||
                    category >= kInventoryEquipmentCategoryCount)
                {
                    SetWorldGameObjectActive(
                        view.root.gameObject,
                        false);
                    continue;
                }
                GameplayInventory_GetEquipmentSlotRect(
                    category,
                    view.minX,
                    view.minY,
                    view.maxX,
                    view.maxY);
            }
        }
        else
        {
            GameplayInventory_SetRectForGrid(
                view.storage,
                view.x,
                view.y,
                view.width,
                view.height,
                view.minX,
                view.minY,
                view.maxX,
                view.maxY);
        }
        SetSceneRect(
            view.root.rect,
            view.minX,
            view.minY,
            view.maxX,
            view.maxY);
        GameplayInventory_ApplyItemIconRotation(view);
        if (view.appraisalBadgeCircle.rect != nullptr)
        {
            float clientWidth = g_inventoryLayoutWidth;
            float clientHeight = g_inventoryLayoutHeight;
            InventoryUi_ReadClientSize(
                clientWidth,
                clientHeight);
            const float tilePixelsWidth = std::max(
                1.0f,
                (view.maxX - view.minX) * clientWidth);
            const float tilePixelsHeight = std::max(
                1.0f,
                (view.maxY - view.minY) * clientHeight);
            const float diameterPixels = std::clamp(
                tilePixelsWidth * 0.32f,
                16.0f,
                24.0f);
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
                view.appraisalBadgeCircle.rect,
                insetX,
                1.0f - insetY - badgeHeight,
                insetX + badgeWidth,
                1.0f - insetY);
        }
        SetWorldGameObjectActive(view.root.gameObject, true);
        SetSceneGraphicColor(
            view.root.image,
            kInventoryQualityColors[
                std::clamp(
                    view.quality,
                    0,
                    kInventoryQualityCount - 1)][0],
            kInventoryQualityColors[
                std::clamp(
                    view.quality,
                    0,
                    kInventoryQualityCount - 1)][1],
            kInventoryQualityColors[
                std::clamp(
                    view.quality,
                    0,
                    kInventoryQualityCount - 1)][2],
            1.0f);
        view.icon.texture = LoadScenePngTexture(
            InventoryUi_EquipmentImagePath(
                definition.id,
                view.quality));
        SetSceneRawImageTexture(
            view.icon.rawImage,
            view.icon.texture);
        SetSceneText(
            view.label.text,
            definition.name,
            10.0f,
            514,
            0.96f,
            0.97f,
            0.99f,
            1.0f);
        const bool showAppraisalBadge =
            view.appraised &&
            view.wearUnits != 0;
        SetWorldGameObjectActive(
            view.appraisalBadgeCircle.gameObject,
            showAppraisalBadge);
        SetWorldGameObjectActive(
            view.appraisalBadgeText.gameObject,
            showAppraisalBadge);
        if (showAppraisalBadge)
        {
            const auto tier = InventoryUi_GetAppraisalTier(
                view.wearUnits);
            float badgeRed = 0.0f;
            float badgeGreen = 0.0f;
            float badgeBlue = 0.0f;
            InventoryUi_AppraisalTierColor(
                tier,
                badgeRed,
                badgeGreen,
                badgeBlue);
            SetSceneText(
                view.appraisalBadgeCircle.text,
                "●",
                22.0f,
                514,
                badgeRed,
                badgeGreen,
                badgeBlue,
                1.0f);
            SetSceneText(
                view.appraisalBadgeText.text,
                WideToUtf8(
                    InventoryUi_AppraisalTierLabel(tier)).c_str(),
                10.0f,
                514,
                1.0f,
                1.0f,
                1.0f,
                1.0f);
        }
    }
    for (int index = viewCount;
         index < static_cast<int>(g_gameplayInventoryViews.size());
         ++index)
    {
        auto& view =
            g_gameplayInventoryViews[static_cast<size_t>(index)];
        if (view.root.gameObject != nullptr)
        {
            SetWorldGameObjectActive(view.root.gameObject, false);
        }
        view.definition = -1;
    }
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Gameplay inventory refreshed: persistent=%d temporary=%d.",
        persistentViewCount,
        static_cast<int>(g_gameplayInventoryTempItems.size()));
}

void GameplayInventory_OpenInternal(bool rewardMode)
{
    if (!ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        LogInfo(LogCategory_Inventory, L"Inventory", L"Gameplay inventory cannot open without the gameplay HUD canvas.");
        return;
    }
    GameplayInventory_Build();
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayInventoryRoot))
    {
        LogInfo(LogCategory_Inventory, L"Inventory", L"Gameplay inventory could not build its HUD root.");
        return;
    }
    g_gameplayInventoryRewardMode = rewardMode;
    g_gameplayInventoryTaskLayout = rewardMode;
    g_gameplayInventoryOpen = true;
    GameplayInventory_Refresh();
    SetWorldGameObjectActive(g_gameplayInventoryRoot, true);
    if (g_gameplayQuickHudRoot != nullptr)
    {
        SetWorldGameObjectActive(g_gameplayQuickHudRoot, false);
    }
    BringSceneObjectToFront(g_gameplayInventoryRoot);
    // A previous native or custom modal may have disabled the shared HUD
    // raycaster. The inventory owns the mouse while open, so restore it here.
    ExtendedMode_SetGameplayHudRaycastsEnabled(true);
    ClipCursor(nullptr);
    ExtendedMode_ForceCursor();
    GameplayInventory_SetStatus(
        "TAB 关闭物品栏  ·  拖动到装备/背包会立即保存");
    g_extendedManualPointerDown = false;
    g_extendedManualPointerConsumed = false;
    g_extendedManualPointerPositionValid = false;
    g_gameplayInventoryObservedPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_gameplayInventoryObservedRightPressSerial =
        g_extendedManualRightPressSerial.load();
    g_gameplayInventoryObservedReleaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    g_gameplayInventoryMouseWasDown =
        (GameplayInput_ReadRawAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    g_gameplayInventoryDraggingView = -1;
    g_gameplayInventoryDragMoved = false;
    LogInfo(LogCategory_Inventory, L"Inventory", L"Gameplay Tab inventory opened.");
}

void GameplayInventory_Open()
{
    GameplayInventory_OpenInternal(false);
}

void GameplayInventory_OpenRewardMode()
{
    GameplayInventory_OpenInternal(true);
}

bool GameplayInventory_HasTemporaryStorageItems(int storage)
{
    for (const auto& item : g_gameplayInventoryTempItems)
    {
        if (item.storage == storage)
        {
            return true;
        }
    }
    return false;
}

void GameplayInventory_ClearTemporaryStorage(int storage)
{
    g_gameplayInventoryTempItems.erase(
        std::remove_if(
            g_gameplayInventoryTempItems.begin(),
            g_gameplayInventoryTempItems.end(),
            [storage](const GameplayInventoryTempItem& item)
            {
                return item.storage == storage;
            }),
        g_gameplayInventoryTempItems.end());
}

void GameplayInventory_CloseRewardMode()
{
    GameplayInventory_ClearTemporaryStorage(kGameplayInventorySupply);
    g_gameplayInventoryTaskLayout = false;
    g_gameplayInventoryRewardMode = false;
    GameplayInventory_Close();
    LogInfo(LogCategory_Inventory, L"Inventory", L"Gameplay reward inventory closed and supply memory cleared.");
}

void GameplayInventory_Close()
{
    ItemContextMenu_Reset();
    g_gameplayInventoryOpen = false;
    g_gameplayInventoryRewardMode = false;
    g_gameplayInventoryTaskLayout = false;
    ExtendedMode_ClearGameplayInputMode();
    g_extendedManualPointerDown = false;
    g_extendedManualPointerConsumed = false;
    g_extendedManualPointerPositionValid = false;
    g_gameplayInventoryDraggingView = -1;
    g_gameplayInventoryMouseWasDown = false;
    g_gameplayInventoryDragMoved = false;
    g_gameplayInventoryHoveredEquipmentCategory = -1;
    g_gameplayInventoryDragVisualWidth = 0.0f;
    g_gameplayInventoryDragVisualHeight = 0.0f;
    if (g_gameplayInventoryRoot != nullptr)
    {
        SetWorldGameObjectActive(g_gameplayInventoryRoot, false);
    }
    if (g_gameplayQuickHudRoot != nullptr)
    {
        SetWorldGameObjectActive(g_gameplayQuickHudRoot, true);
    }
    ClipCursor(nullptr);
    ExtendedMode_ForceGameplayCursor();
    ExtendedMode_SetGameplayHudRaycastsEnabled(true);
    LogInfo(LogCategory_Inventory, L"Inventory", L"Gameplay Tab inventory closed.");
}

bool GameplayInventory_IsOpenStateUsable()
{
    if (!g_gameplayInventoryOpen.load() ||
        !ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject) ||
        !ExtendedMode_IsUnityObjectAlive(
            g_gameplayInventoryRoot))
    {
        return false;
    }
    return ExtendedMode_GetGameObjectActiveSelf(
        g_gameplayInventoryRoot);
}

void GameplayInventory_RecoverInvalidOpenState(
    const wchar_t* reason)
{
    if (!g_gameplayInventoryOpen.exchange(false))
    {
        return;
    }

    const bool rootAlive =
        g_gameplayInventoryRoot != nullptr &&
        ExtendedMode_IsUnityObjectAlive(
            g_gameplayInventoryRoot);
    const bool quickHudAlive =
        g_gameplayQuickHudRoot != nullptr &&
        ExtendedMode_IsUnityObjectAlive(
            g_gameplayQuickHudRoot);

    if (rootAlive)
    {
        SetWorldGameObjectActive(
            g_gameplayInventoryRoot,
            false);
    }
    else
    {
        g_gameplayInventoryRoot = nullptr;
    }
    if (quickHudAlive)
    {
        SetWorldGameObjectActive(
            g_gameplayQuickHudRoot,
            true);
    }
    else
    {
        g_gameplayQuickHudRoot = nullptr;
    }

    g_gameplayInventoryRewardMode = false;
    g_gameplayInventoryTaskLayout = false;
    g_extendedManualPointerDown = false;
    g_extendedManualPointerConsumed = false;
    g_extendedManualPointerPositionValid = false;
    g_gameplayInventoryDraggingView = -1;
    g_gameplayInventoryMouseWasDown = false;
    g_gameplayInventoryDragMoved = false;
    g_gameplayInventoryRotateKeyDown = false;
    ExtendedMode_ClearGameplayInputMode();
    ExtendedMode_SetGameplayHudRaycastsEnabled(true);
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Gameplay inventory recovered an invalid open state: %s.",
        reason != nullptr ? reason : L"unknown");
}

void GameplayInventory_EnforceOriginalInventoryClosed()
{
    if (g_extendedOriginalSystemMenuVisible.load())
    {
        return;
    }
    auto* inventoryClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.InventoryPanel",
        "InventoryPanelView");
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (inventoryClass == nullptr ||
        objectClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr ||
        il2cpp_type_get_object == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(inventoryClass);
    auto* managedType = il2cpp_type_get_object(
        il2cpp_class_get_type(inventoryClass));
    bool includeInactive = true;
    void* findParameters[] = { managedType, &includeInactive };
    auto* inventoryPanels = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        findParameters);
    if (inventoryPanels == nullptr ||
        il2cpp_array_length(inventoryPanels) == 0)
    {
        return;
    }
    auto* getActiveSelf = FindMethod(
        gameObjectClass,
        "get_activeSelf",
        0);
    auto* closeMethod = FindMethod(
        inventoryClass,
        "Close",
        0);
    auto* clickBackMethod = FindMethod(
        inventoryClass,
        "OnClickBack",
        0);
    auto* closePanelMethod = FindMethod(
        inventoryClass,
        "ClosePanel",
        0);
    auto* hideMethod = FindMethod(
        inventoryClass,
        "Hide",
        0);
    const auto count = il2cpp_array_length(inventoryPanels);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(inventoryPanels) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* panel = elements[index];
        if (panel == nullptr)
        {
            continue;
        }
        auto* panelObject = GetComponentGameObject(panel);
        auto* activeValue =
            panelObject != nullptr && getActiveSelf != nullptr
                ? Invoke(getActiveSelf, panelObject, nullptr)
                : nullptr;
        const bool active =
            activeValue != nullptr &&
            il2cpp_object_unbox != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(activeValue));
        if (!active)
        {
            continue;
        }
        static bool loggedClose = false;
        if (!loggedClose)
        {
            loggedClose = true;
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Escape gameplay detected active original InventoryPanelView; closing it.");
        }
        if (clickBackMethod != nullptr)
        {
            Invoke(clickBackMethod, panel, nullptr);
        }
        else if (closeMethod != nullptr)
        {
            Invoke(closeMethod, panel, nullptr);
        }
        else if (closePanelMethod != nullptr)
        {
            Invoke(closePanelMethod, panel, nullptr);
        }
        else if (hideMethod != nullptr)
        {
            Invoke(hideMethod, panel, nullptr);
        }
        else if (panelObject != nullptr)
        {
            SetWorldGameObjectActive(panelObject, false);
        }
    }
}

void GameplayInventory_EnforceOriginalInGameMenuClosed()
{
    if (g_extendedOriginalSystemMenuVisible.load())
    {
        return;
    }
    auto* menuClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.InGameMenu",
        "InGameMenuView");
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (menuClass == nullptr ||
        objectClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr ||
        il2cpp_type_get_object == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(menuClass);
    auto* managedType = il2cpp_type_get_object(
        il2cpp_class_get_type(menuClass));
    bool includeInactive = true;
    void* findParameters[] = { managedType, &includeInactive };
    auto* menus = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        findParameters);
    if (menus == nullptr || il2cpp_array_length(menus) == 0)
    {
        return;
    }
    auto* getActiveSelf = FindMethod(
        gameObjectClass,
        "get_activeSelf",
        0);
    auto* clickBackMethod = FindMethod(
        menuClass,
        "OnClickBack",
        0);
    auto* closeMethod = FindMethod(menuClass, "Close", 0);
    auto* hideMethod = FindMethod(menuClass, "Hide", 0);
    const auto count = il2cpp_array_length(menus);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(menus) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* menu = elements[index];
        if (menu == nullptr)
        {
            continue;
        }
        auto* menuObject = GetComponentGameObject(menu);
        auto* activeValue =
            menuObject != nullptr && getActiveSelf != nullptr
                ? Invoke(getActiveSelf, menuObject, nullptr)
                : nullptr;
        const bool active =
            activeValue != nullptr &&
            il2cpp_object_unbox != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(activeValue));
        if (!active)
        {
            continue;
        }
        static bool loggedClose = false;
        if (!loggedClose)
        {
            loggedClose = true;
            LogInfo(LogCategory_Inventory, L"Inventory", L"Escape gameplay closed active original InGameMenuView.");
        }
        if (clickBackMethod != nullptr)
        {
            Invoke(clickBackMethod, menu, nullptr);
        }
        else if (closeMethod != nullptr)
        {
            Invoke(closeMethod, menu, nullptr);
        }
        else if (hideMethod != nullptr)
        {
            Invoke(hideMethod, menu, nullptr);
        }
        else if (menuObject != nullptr)
        {
            SetWorldGameObjectActive(menuObject, false);
        }
    }
}

int GameplayInventory_ViewAt(float x, float y)
{
    for (int index =
             static_cast<int>(g_gameplayInventoryViews.size()) - 1;
         index >= 0;
         --index)
    {
        const auto& view =
            g_gameplayInventoryViews[static_cast<size_t>(index)];
        if (view.definition >= 0 &&
            view.root.gameObject != nullptr &&
            x >= view.minX && x <= view.maxX &&
            y >= view.minY && y <= view.maxY)
        {
            return index;
        }
    }
    return -1;
}

int GameplayInventory_QuickSlotAt(float x, float y)
{
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        const auto& slot =
            g_gameplayInventoryQuickSlots[index];
        if (x >= slot.minX && x <= slot.maxX &&
            y >= slot.minY && y <= slot.maxY)
        {
            return index;
        }
    }
    return -1;
}

int GameplayInventory_ViewForItem(int itemIndex)
{
    for (int index = 0;
         index < static_cast<int>(g_gameplayInventoryViews.size());
         ++index)
    {
        if (g_gameplayInventoryViews[static_cast<size_t>(index)].itemIndex ==
            itemIndex)
        {
            return index;
        }
    }
    return -1;
}

int GameplayInventory_EnsureQuickDragView(int itemIndex)
{
    int viewIndex = GameplayInventory_ViewForItem(itemIndex);
    if (viewIndex >= 0)
    {
        return viewIndex;
    }
    if (!InventoryUi_CanAssignQuickSlot(itemIndex))
    {
        return -1;
    }
    viewIndex =
        static_cast<int>(g_gameplayInventoryViews.size());
    GameplayInventory_EnsureView(viewIndex);
    auto& view =
        g_gameplayInventoryViews[static_cast<size_t>(viewIndex)];
    const auto& item = g_inventoryItems[itemIndex];
    view.definition = item.definition;
    view.quality = item.quality;
    view.appraised = item.appraised;
    view.wearUnits = item.wearUnits;
    view.itemIndex = itemIndex;
    view.storage = kGameplayInventoryEquipped;
    view.equipmentCategory =
        GameplayInventory_FindEquipmentCategory(item.definition);
    view.x = item.x;
    view.y = item.y;
    view.width = item.width;
    view.height = item.height;
    view.tempIndex = -1;
    return viewIndex;
}

bool GameplayInventory_ApplyDrop(
    int sourceView,
    int targetStorage,
    int targetColumn,
    int targetRow,
    int targetCategory)
{
    if (sourceView < 0 ||
        sourceView >=
            static_cast<int>(g_gameplayInventoryViews.size()))
    {
        return false;
    }
    auto& source =
        g_gameplayInventoryViews[static_cast<size_t>(sourceView)];
    const int sourceDefinition = source.definition;
    const int sourceQuality = source.quality;
    const bool sourceAppraised = source.appraised;
    const unsigned int sourceWearUnits = source.wearUnits;
    const int sourceTempIndex = source.tempIndex;
    if (source.storage == kGameplayInventoryEquipped ||
        source.storage == kGameplayInventoryBackpack ||
        source.storage == kGameplayInventorySafeBox)
    {
        if (!InventoryUi_IsItemSlot(source.itemIndex))
        {
            return false;
        }
        auto& persistent = g_inventoryItems[source.itemIndex];
        if (source.storage == kGameplayInventoryEquipped &&
            InventoryUi_IsBackpackDefinition(sourceDefinition) &&
            targetCategory != kInventoryBackpackCategoryIndex &&
            InventoryUi_BackpackHasItems(source.itemIndex))
        {
            GameplayInventory_SetStatus(
                "背包内仍有物品，暂时不能卸下");
            return false;
        }
        if (targetCategory >= 0)
        {
            if (!GameplayInventory_CanEquipDefinition(
                    sourceDefinition,
                    targetCategory,
                    sourceDefinition))
            {
                GameplayInventory_SetStatus(
                    "该装备槽不可用或已被占用");
                return false;
            }
            persistent.storage = kInventoryStorageEquipped;
            persistent.x = 0;
            persistent.y = 0;
            persistent.width = source.width;
            persistent.height = source.height;
            InventoryUi_RefreshEquipment();
            InventoryUi_SaveState();
            GameplayInventory_SetStatus(
                "装备已更新并立即写入存档");
            return true;
        }
        if (!GameplayInventory_CellFree(
                targetStorage,
                targetColumn,
                targetRow,
                source.width,
                source.height,
                sourceView,
                sourceTempIndex))
        {
            GameplayInventory_SetStatus("目标空间不足");
            return false;
        }
        if (targetStorage == kGameplayInventoryBackpack ||
            targetStorage == kGameplayInventorySafeBox)
        {
            persistent.storage =
                targetStorage == kGameplayInventorySafeBox
                    ? kInventoryStorageSafeBox
                    : kInventoryStorageBackpack;
            persistent.x = targetColumn;
            persistent.y = targetRow;
            persistent.width = source.width;
            persistent.height = source.height;
            if (source.storage == kGameplayInventoryEquipped)
            {
                InventoryUi_RefreshCharacterModel();
            }
            InventoryUi_SaveState();
            GameplayInventory_SetStatus(
                targetStorage == kGameplayInventorySafeBox
                    ? "安全箱已更新并立即写入存档"
                    : "背包已更新并立即写入存档");
            return true;
        }
        persistent.storage = -1;
        persistent.element = SceneUiElement{};
        persistent.icon = SceneUiElement{};
        persistent.label = SceneUiElement{};
        GameplayInventoryTempItem temp;
        temp.definition = sourceDefinition;
        temp.quality = sourceQuality;
        temp.appraised = sourceAppraised;
        temp.wearUnits = sourceWearUnits;
        temp.storage = targetStorage;
        temp.x = targetColumn;
        temp.y = targetRow;
        temp.width = source.width;
        temp.height = source.height;
        temp.serial = ++g_gameplayInventoryTempSerial;
        g_gameplayInventoryTempItems.push_back(temp);
        if (source.storage == kGameplayInventoryEquipped)
        {
            InventoryUi_RefreshCharacterModel();
        }
        InventoryUi_SaveState();
        GameplayInventory_SetStatus(
            "物品已移入本局临时区域，装备/背包存档已同步");
        return true;
    }

    if (sourceTempIndex < 0 ||
        sourceTempIndex >=
            static_cast<int>(g_gameplayInventoryTempItems.size()))
    {
        return false;
    }
    auto& temp =
        g_gameplayInventoryTempItems[
            static_cast<size_t>(sourceTempIndex)];
    if (targetCategory >= 0 ||
        targetStorage == kGameplayInventoryBackpack ||
        targetStorage == kGameplayInventorySafeBox)
    {
        const int persistentSlot =
            InventoryUi_FindFreeExtraItemSlot();
        if (persistentSlot < 0)
        {
            GameplayInventory_SetStatus(
                "没有可用的持久库存槽位");
            return false;
        }
        auto& persistent =
            g_inventoryItems[persistentSlot];
        if (targetCategory >= 0)
        {
            if (!GameplayInventory_CanEquipDefinition(
                    sourceDefinition,
                    targetCategory,
                    sourceDefinition))
            {
                GameplayInventory_SetStatus(
                    "该装备槽不可用或已被占用");
                return false;
            }
            persistent.storage = kInventoryStorageEquipped;
            persistent.x = 0;
            persistent.y = 0;
        }
        else
        {
            int excludingView = sourceView;
            for (size_t viewIndex = 0;
                 viewIndex < g_gameplayInventoryViews.size();
                 ++viewIndex)
            {
                const auto& existing =
                    g_gameplayInventoryViews[viewIndex];
                if (static_cast<int>(viewIndex) != sourceView &&
                    existing.definition == sourceDefinition &&
                    existing.storage == targetStorage)
                {
                    excludingView = static_cast<int>(viewIndex);
                    break;
                }
            }
            if (!GameplayInventory_CellFree(
                    targetStorage,
                    targetColumn,
                    targetRow,
                    source.width,
                    source.height,
                    excludingView,
                    sourceTempIndex))
            {
                GameplayInventory_SetStatus("背包空间不足");
                return false;
            }
            persistent.storage =
                targetStorage == kGameplayInventorySafeBox
                    ? kInventoryStorageSafeBox
                    : kInventoryStorageBackpack;
            persistent.x = targetColumn;
            persistent.y = targetRow;
        }
        persistent.quality = sourceQuality;
        persistent.appraised = sourceAppraised;
        persistent.wearUnits =
            sourceAppraised ? sourceWearUnits : 0;
        persistent.width = source.width;
        persistent.height = source.height;
        persistent.definition = sourceDefinition;
        g_gameplayInventoryTempItems.erase(
            g_gameplayInventoryTempItems.begin() + sourceTempIndex);
        InventoryUi_RefreshEquipment();
        InventoryUi_SaveState();
        GameplayInventory_SetStatus(
            targetCategory >= 0
                ? "已装备并立即写入存档"
                : (targetStorage == kGameplayInventorySafeBox
                    ? "已放入安全箱并立即写入存档"
                    : "已放入背包并立即写入存档"));
        return true;
    }

    if (!GameplayInventory_CellFree(
            targetStorage,
            targetColumn,
            targetRow,
            source.width,
            source.height,
            sourceView,
            sourceTempIndex))
    {
        GameplayInventory_SetStatus("目标临时空间不足");
        return false;
    }
    temp.storage = targetStorage;
    temp.x = targetColumn;
    temp.y = targetRow;
    GameplayInventory_SetStatus(
        targetStorage == kGameplayInventoryStaging
            ? "物品已移入整理区（本局）"
            : "物品已移入物资区（本局）");
    return true;
}

void GameplayInventory_FinishDrop(
    int sourceView,
    float cursorX,
    float cursorY)
{
    const int quickTarget =
        GameplayInventory_QuickSlotAt(cursorX, cursorY);
    if (quickTarget >= 0)
    {
        const auto& source =
            g_gameplayInventoryViews[
                static_cast<size_t>(sourceView)];
        if (InventoryUi_CanAssignQuickSlot(source.itemIndex))
        {
            InventoryUi_BindQuickSlot(
                quickTarget,
                source.itemIndex);
            LogInfo(
                LogCategory_Inventory,
                L"QuickSlotBind",
                L"Gameplay quick slot %d bound itemSlot=%d definition=%d.",
                quickTarget + 1,
                source.itemIndex,
                source.definition);
            GameplayInventory_SetStatus(
                "快捷格已绑定实际物品  ·  原物品仍在背包或安全箱");
            InventoryUi_SaveState();
        }
        else
        {
            GameplayInventory_SetStatus(
                "快捷格只接受背包或安全箱中的成人用品");
        }
        g_gameplayInventoryQuickSlotDragSource = -1;
        GameplayInventory_Refresh();
        return;
    }
    int column = 0;
    int row = 0;
    const int category =
        GameplayInventory_EquipmentSlotAt(cursorX, cursorY);
    const int storage = GameplayInventory_GridAt(
        cursorX,
        cursorY,
        column,
        row);
    const auto& dragged =
        g_gameplayInventoryViews[static_cast<size_t>(sourceView)];
    int resolvedCategory = category;
    if ((resolvedCategory < 0 ||
         !GameplayInventory_CanEquipDefinition(
             dragged.definition,
             resolvedCategory,
             dragged.definition)) &&
        GameplayInventory_InsideEquipmentPanel(cursorX, cursorY))
    {
        const int automaticCategory =
            InventoryUi_FindAutoEquipCategory(dragged.definition);
        if (automaticCategory >= 0)
        {
            resolvedCategory = automaticCategory;
        }
    }
    if (resolvedCategory < 0 &&
        InventoryUi_IsBackpackDefinition(dragged.definition) &&
        GameplayInventory_InsideBackpackPanel(cursorX, cursorY))
    {
        resolvedCategory = kInventoryBackpackCategoryIndex;
    }
    bool applied = false;
    if (resolvedCategory >= 0)
    {
        applied = GameplayInventory_ApplyDrop(
            sourceView,
            -1,
            0,
            0,
            resolvedCategory);
    }
    else if (storage >= 0)
    {
        int targetColumn = column;
        int targetRow = row;
        if (!GameplayInventory_ResolveGridDropCell(
                storage,
                cursorX,
                cursorY,
                dragged.width,
                dragged.height,
                targetColumn,
                targetRow))
        {
            targetColumn = column;
            targetRow = row;
        }
        applied = GameplayInventory_ApplyDrop(
            sourceView,
            storage,
            targetColumn,
            targetRow,
            -1);
        column = targetColumn;
        row = targetRow;
    }
    if (!applied)
    {
        GameplayInventory_SetStatus(
            "移动未完成，物品已回到原位置");
    }
    else
    {
        if (g_gameplayInventoryQuickSlotDragSource >= 0)
        {
            InventoryUi_BindQuickSlot(
                g_gameplayInventoryQuickSlotDragSource,
                -1);
        }
        InventoryUi_PruneQuickSlotBindings();
        InventoryUi_SaveState();
    }
    g_gameplayInventoryQuickSlotDragSource = -1;
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Gameplay inventory drop view=%d category=%d storage=%d column=%d row=%d applied=%d.",
        sourceView,
        resolvedCategory,
        storage,
        column,
        row,
        applied ? 1 : 0);
    GameplayInventory_Refresh();
}

void GameplayInventory_CancelPointerInteraction()
{
    g_gameplayInventoryObservedPressSerial =
        g_extendedManualPointerPressSerial.load();
    g_gameplayInventoryObservedReleaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    g_gameplayInventoryObservedRightPressSerial =
        g_extendedManualRightPressSerial.load();
    g_gameplayInventoryDraggingView = -1;
    g_gameplayInventoryQuickSlotDragSource = -1;
    g_gameplayInventoryMouseWasDown = false;
    g_gameplayInventoryDragMoved = false;
    g_gameplayInventoryQuickSlotDragSource = -1;
    g_gameplayInventoryRotateKeyDown = false;
    g_gameplayInventoryHoveredEquipmentCategory = -1;
    g_gameplayInventoryDragVisualWidth = 0.0f;
    g_gameplayInventoryDragVisualHeight = 0.0f;
}

void GameplayInventory_ProcessMouse()
{
    if (ItemContextMenu_ShouldBlockPointer())
    {
        return;
    }
    const HWND window = FindGameWindow();
    RECT clientRect{};
    if (window == nullptr ||
        !GetClientRect(window, &clientRect))
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

    POINT cursor{};
    if (g_extendedManualPointerPositionValid.load())
    {
        cursor.x = g_extendedManualPointerCurrentX.load();
        cursor.y = g_extendedManualPointerCurrentY.load();
    }
    else if (!GetCursorPos(&cursor))
    {
        return;
    }
    if (!ScreenToClient(window, &cursor))
    {
        return;
    }
    const float cursorX = static_cast<float>(cursor.x) / width;
    const float cursorY = 1.0f - static_cast<float>(cursor.y) / height;

    const unsigned int rightPressSerial =
        g_extendedManualRightPressSerial.load();
    if (rightPressSerial !=
        g_gameplayInventoryObservedRightPressSerial)
    {
        g_gameplayInventoryObservedRightPressSerial =
            rightPressSerial;
        POINT pressPoint{
            g_extendedManualPointerPressX.load(),
            g_extendedManualPointerPressY.load()};
        ScreenToClient(window, &pressPoint);
        const int quickSlot =
            GameplayInventory_QuickSlotAt(
                static_cast<float>(pressPoint.x) / width,
                1.0f -
                    static_cast<float>(pressPoint.y) / height);
        if (quickSlot >= 0)
        {
            InventoryUi_BindQuickSlot(quickSlot, -1);
            InventoryUi_SaveState();
            GameplayInventory_SetStatus(
                "快捷格已解除绑定  ·  原物品仍保留");
            GameplayInventory_Refresh();
            return;
        }
    }

    const unsigned int pressSerial =
        g_extendedManualPointerPressSerial.load();
    if (pressSerial != g_gameplayInventoryObservedPressSerial)
    {
        g_gameplayInventoryObservedPressSerial = pressSerial;
        g_gameplayInventoryMouseWasDown = true;
        g_gameplayInventoryDragMoved = false;
        POINT pressPoint{
            g_extendedManualPointerPressX.load(),
            g_extendedManualPointerPressY.load()};
        ScreenToClient(window, &pressPoint);
        g_gameplayInventoryPressX =
            static_cast<float>(pressPoint.x) / width;
        g_gameplayInventoryPressY =
            1.0f - static_cast<float>(pressPoint.y) / height;
        g_gameplayInventoryDraggingView =
            GameplayInventory_ViewAt(
                g_gameplayInventoryPressX,
                g_gameplayInventoryPressY);
        g_gameplayInventoryQuickSlotDragSource =
            GameplayInventory_QuickSlotAt(
                g_gameplayInventoryPressX,
                g_gameplayInventoryPressY);
        if (g_gameplayInventoryDraggingView < 0 &&
            g_gameplayInventoryQuickSlotDragSource >= 0)
        {
            const int boundItem =
                InventoryUi_FindQuickSlotBoundItem(
                    g_gameplayInventoryQuickSlotDragSource);
            g_gameplayInventoryDraggingView =
                GameplayInventory_EnsureQuickDragView(boundItem);
        }
        g_gameplayInventoryHoveredEquipmentCategory = -1;
        g_gameplayInventoryDragGrabCellX = 0;
        g_gameplayInventoryDragGrabCellY = 0;
        g_gameplayInventoryDragPointerOffsetPixelsX = 0.0f;
        g_gameplayInventoryDragPointerOffsetPixelsY = 0.0f;
        g_gameplayInventoryDragVisualWidth = 0.0f;
        g_gameplayInventoryDragVisualHeight = 0.0f;
        g_gameplayInventoryRotateKeyDown =
            (GameplayInput_ReadRawAsyncKeyState('R') & 0x8000) != 0;
        if (g_gameplayInventoryDraggingView >= 0)
        {
            const auto& view =
                g_gameplayInventoryViews[
                    static_cast<size_t>(
                        g_gameplayInventoryDraggingView)];
            const float pressPixelsX =
                static_cast<float>(pressPoint.x);
            const float pressPixelsY =
                static_cast<float>(pressPoint.y);
            g_gameplayInventoryDragVisualWidth =
                view.maxX - view.minX;
            g_gameplayInventoryDragVisualHeight =
                view.maxY - view.minY;
            g_gameplayInventoryDragPointerOffsetPixelsX =
                pressPixelsX - view.minX * width;
            g_gameplayInventoryDragPointerOffsetPixelsY =
                (1.0f - view.maxY) * height - pressPixelsY;
            if (view.storage != kGameplayInventoryEquipped)
            {
                float gridMinX = 0.0f;
                float gridMinY = 0.0f;
                float gridMaxX = 0.0f;
                float gridMaxY = 0.0f;
                float cellWidth = 0.0f;
                float cellHeight = 0.0f;
                GameplayInventory_GetGridMetrics(
                    view.storage,
                    gridMinX,
                    gridMinY,
                    gridMaxX,
                    gridMaxY,
                    cellWidth,
                    cellHeight);
                if (cellWidth > 0.0f && cellHeight > 0.0f)
                {
                    const int hoveredColumn = std::clamp(
                        static_cast<int>(
                            (g_gameplayInventoryPressX -
                             gridMinX) /
                            cellWidth),
                        0,
                        std::max(
                            0,
                            GameplayInventory_StorageColumns(
                                view.storage) - 1));
                    const int hoveredRow = std::clamp(
                        static_cast<int>(
                            (gridMaxY -
                             g_gameplayInventoryPressY) /
                            cellHeight),
                        0,
                        std::max(
                            0,
                            GameplayInventory_StorageRows(
                                view.storage) - 1));
                    g_gameplayInventoryDragGrabCellX = std::clamp(
                        hoveredColumn - view.x,
                        0,
                        std::max(0, view.width - 1));
                    g_gameplayInventoryDragGrabCellY = std::clamp(
                        hoveredRow - view.y,
                        0,
                        std::max(0, view.height - 1));
                }
            }
            if (g_gameplayInventoryQuickSlotDragSource >= 0)
            {
                const auto& quickSlot =
                    g_gameplayInventoryQuickSlots[
                        g_gameplayInventoryQuickSlotDragSource];
                g_gameplayInventoryDragPointerOffsetPixelsX =
                    static_cast<float>(pressPoint.x) -
                    quickSlot.minX * width;
                g_gameplayInventoryDragPointerOffsetPixelsY =
                    (1.0f - quickSlot.maxY) * height -
                    static_cast<float>(pressPoint.y);
                if (view.storage != kGameplayInventoryEquipped)
                {
                    float gridMinX = 0.0f;
                    float gridMinY = 0.0f;
                    float gridMaxX = 0.0f;
                    float gridMaxY = 0.0f;
                    float cellWidth = 0.0f;
                    float cellHeight = 0.0f;
                    GameplayInventory_GetGridMetrics(
                        view.storage,
                        gridMinX,
                        gridMinY,
                        gridMaxX,
                        gridMaxY,
                        cellWidth,
                        cellHeight);
                    g_gameplayInventoryDragVisualWidth =
                        view.width * cellWidth;
                    g_gameplayInventoryDragVisualHeight =
                        view.height * cellHeight;
                }
                SetWorldGameObjectActive(
                    view.root.gameObject,
                    true);
            }
        }
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Gameplay inventory pointer down at %.3f,%.3f hitView=%d.",
            g_gameplayInventoryPressX,
            g_gameplayInventoryPressY,
            g_gameplayInventoryDraggingView);
    }

    if (g_gameplayInventoryDraggingView >= 0)
    {
        const float deltaX =
            std::fabs(cursorX - g_gameplayInventoryPressX);
        const float deltaY =
            std::fabs(cursorY - g_gameplayInventoryPressY);
        if (deltaX * width > 4.0f ||
            deltaY * height > 4.0f)
        {
            g_gameplayInventoryDragMoved = true;
        }
        if (g_gameplayInventoryDragMoved)
        {
            auto& view = g_gameplayInventoryViews[
                static_cast<size_t>(g_gameplayInventoryDraggingView)];
            const bool rotateDown =
                (GameplayInput_ReadRawAsyncKeyState('R') & 0x8000) != 0;
            if (rotateDown && !g_gameplayInventoryRotateKeyDown)
            {
                const int oldWidth = view.width;
                view.width = view.height;
                view.height = oldWidth;
                if (view.storage != kGameplayInventoryEquipped)
                {
                    float gridMinX = 0.0f;
                    float gridMinY = 0.0f;
                    float gridMaxX = 0.0f;
                    float gridMaxY = 0.0f;
                    float cellWidth = 0.0f;
                    float cellHeight = 0.0f;
                    GameplayInventory_GetGridMetrics(
                        view.storage,
                        gridMinX,
                        gridMinY,
                        gridMaxX,
                        gridMaxY,
                        cellWidth,
                        cellHeight);
                    g_gameplayInventoryDragVisualWidth =
                        view.width * cellWidth;
                    g_gameplayInventoryDragVisualHeight =
                        view.height * cellHeight;
                }
                else
                {
                    const float unit = std::max(
                        32.0f,
                        height * 0.055f);
                    g_gameplayInventoryDragVisualWidth =
                        view.width * unit / width;
                    g_gameplayInventoryDragVisualHeight =
                        view.height * unit / height;
                }
                GameplayInventory_SetStatus("物品已旋转");
            }
            g_gameplayInventoryRotateKeyDown = rotateDown;
            const int hoveredQuickSlot =
                GameplayInventory_QuickSlotAt(cursorX, cursorY);
            int hoveredColumn = 0;
            int hoveredRow = 0;
            int hoveredStorage = GameplayInventory_GridAt(
                cursorX,
                cursorY,
                hoveredColumn,
                hoveredRow);
            int hoveredCategory =
                GameplayInventory_EquipmentSlotAt(cursorX, cursorY);
            if ((hoveredCategory < 0 ||
                 !GameplayInventory_CanEquipDefinition(
                     view.definition,
                     hoveredCategory,
                     view.definition)) &&
                GameplayInventory_InsideEquipmentPanel(
                    cursorX,
                    cursorY))
            {
                const int automaticCategory =
                    InventoryUi_FindAutoEquipCategory(
                        view.definition);
                if (automaticCategory >= 0)
                {
                    hoveredCategory = automaticCategory;
                }
            }
            if (InventoryUi_IsBackpackDefinition(view.definition) &&
                GameplayInventory_InsideBackpackPanel(
                    cursorX,
                    cursorY) &&
                GameplayInventory_CanEquipDefinition(
                    view.definition,
                    kInventoryBackpackCategoryIndex,
                    view.definition))
            {
                hoveredStorage = -1;
                hoveredCategory = kInventoryBackpackCategoryIndex;
            }
            g_gameplayInventoryHoveredEquipmentCategory =
                hoveredCategory;
            GameplayInventory_UpdateEquipmentHighlights(
                hoveredCategory);

            if (hoveredQuickSlot >= 0)
            {
                const auto& quickSlot =
                    g_gameplayInventoryQuickSlots[hoveredQuickSlot];
                view.minX = quickSlot.minX;
                view.minY = quickSlot.minY;
                view.maxX = quickSlot.maxX;
                view.maxY = quickSlot.maxY;
                SetSceneRect(
                    view.root.rect,
                    view.minX,
                    view.minY,
                    view.maxX,
                    view.maxY);
                const bool canBind =
                    InventoryUi_CanAssignQuickSlot(view.itemIndex);
                SetSceneGraphicColor(
                    view.root.image,
                    canBind ? 0.14f : 0.55f,
                    canBind ? 0.58f : 0.10f,
                    canBind ? 0.25f : 0.10f,
                    1.0f);
                GameplayInventory_SetStatus(
                    canBind
                        ? "快捷格只绑定成人用品  ·  原物品不会移动"
                        : "快捷格仅接受背包或安全箱中的成人用品");
            }
            else if (hoveredStorage >= 0 &&
                GameplayInventory_ResolveGridDropCell(
                    hoveredStorage,
                    cursorX,
                    cursorY,
                    view.width,
                    view.height,
                    hoveredColumn,
                    hoveredRow))
            {
                int excludingView = g_gameplayInventoryDraggingView;
                for (size_t otherIndex = 0;
                     otherIndex < g_gameplayInventoryViews.size();
                     ++otherIndex)
                {
                    const auto& other =
                        g_gameplayInventoryViews[otherIndex];
                    if (static_cast<int>(otherIndex) !=
                            g_gameplayInventoryDraggingView &&
                        other.definition == view.definition &&
                        other.storage == hoveredStorage)
                    {
                        excludingView =
                            static_cast<int>(otherIndex);
                        break;
                    }
                }
                const bool canPlace =
                    GameplayInventory_CellFree(
                        hoveredStorage,
                        hoveredColumn,
                        hoveredRow,
                        view.width,
                        view.height,
                        excludingView,
                        view.tempIndex);
                GameplayInventory_SetRectForGrid(
                    hoveredStorage,
                    hoveredColumn,
                    hoveredRow,
                    view.width,
                    view.height,
                    view.minX,
                    view.minY,
                    view.maxX,
                    view.maxY);
                SetSceneRect(
                    view.root.rect,
                    view.minX,
                    view.minY,
                    view.maxX,
                    view.maxY);
                SetSceneGraphicColor(
                    view.root.image,
                    canPlace ? 0.14f : 0.55f,
                    canPlace ? 0.58f : 0.10f,
                    canPlace ? 0.25f : 0.10f,
                    1.0f);
            }
            else if (hoveredCategory >= 0)
            {
                if (hoveredCategory ==
                    kInventoryBackpackCategoryIndex)
                {
                    GameplayInventory_GetBackpackSlotRect(
                        view.minX,
                        view.minY,
                        view.maxX,
                        view.maxY);
                }
                else
                {
                    GameplayInventory_GetEquipmentSlotRect(
                        hoveredCategory,
                        view.minX,
                        view.minY,
                        view.maxX,
                        view.maxY);
                }
                SetSceneRect(
                    view.root.rect,
                    view.minX,
                    view.minY,
                    view.maxX,
                    view.maxY);
                const bool canEquip =
                    GameplayInventory_CanEquipDefinition(
                        view.definition,
                        hoveredCategory,
                        view.definition);
                SetSceneGraphicColor(
                    view.root.image,
                    canEquip ? 0.14f : 0.55f,
                    canEquip ? 0.58f : 0.10f,
                    canEquip ? 0.25f : 0.10f,
                    1.0f);
            }
            else
            {
                const float dragWidthPixels =
                    g_gameplayInventoryDragVisualWidth * width;
                const float dragHeightPixels =
                    g_gameplayInventoryDragVisualHeight * height;
                const float dragMinPixelsX =
                    static_cast<float>(cursor.x) -
                    g_gameplayInventoryDragPointerOffsetPixelsX;
                const float dragTopPixelsY =
                    static_cast<float>(cursor.y) +
                    g_gameplayInventoryDragPointerOffsetPixelsY;
                SetSceneRect(
                    view.root.rect,
                    dragMinPixelsX / width,
                    1.0f -
                        (dragTopPixelsY + dragHeightPixels) /
                            height,
                    (dragMinPixelsX + dragWidthPixels) / width,
                    1.0f - dragTopPixelsY / height);
            }
            GameplayInventory_ApplyItemIconRotation(view);
            BringSceneObjectToFront(view.root.gameObject);
        }
    }

    const unsigned int releaseSerial =
        g_extendedManualPointerReleaseSerial.load();
    if (releaseSerial != g_gameplayInventoryObservedReleaseSerial)
    {
        g_gameplayInventoryObservedReleaseSerial = releaseSerial;
        const int sourceView = g_gameplayInventoryDraggingView;
        g_gameplayInventoryDraggingView = -1;
        if (sourceView >= 0 && g_gameplayInventoryDragMoved)
        {
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Gameplay inventory drag released for view=%d.",
                sourceView);
            GameplayInventory_FinishDrop(
                sourceView,
                cursorX,
                cursorY);
        }
        g_gameplayInventoryHoveredEquipmentCategory = -1;
        GameplayInventory_UpdateEquipmentHighlights(-1);
        g_gameplayInventoryMouseWasDown = false;
        g_gameplayInventoryDragMoved = false;
        g_gameplayInventoryQuickSlotDragSource = -1;
        g_gameplayInventoryRotateKeyDown = false;
    }
}

void GameplayInventory_Toggle()
{
    if (g_gameplayInventoryOpen.load())
    {
        if (g_gameplayInventoryRewardMode)
        {
            CheckpointSystem_RequestRewardInventoryClose();
        }
        else
        {
            GameplayInventory_Close();
        }
    }
    else
    {
        GameplayInventory_Open();
    }
}

void GameplayInventory_TickMainThread()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        g_extendedToggleGameplayInventoryRequested = false;
        if (g_gameplayInventoryOpen.load())
        {
            GameplayInventory_Close();
        }
        return;
    }
    if (g_gameplayInventoryOpen.load() &&
        !GameplayInventory_IsOpenStateUsable())
    {
        GameplayInventory_RecoverInvalidOpenState(
            L"the inventory root or HUD is unavailable");
    }
    GameplayInventory_EnsureState();
    GameplayInventory_Build();
    static unsigned int quickHudRefreshFrame = 0;
    if (!g_gameplayInventoryOpen.load() &&
        ++quickHudRefreshFrame % 15 == 0)
    {
        GameplayInventory_UpdateQuickSlots();
    }
    static unsigned int originalPanelCheckFrame = 0;
    if (++originalPanelCheckFrame % 10 == 0)
    {
        GameplayInventory_EnforceOriginalInventoryClosed();
        GameplayInventory_EnforceOriginalInGameMenuClosed();
    }
    if (g_extendedToggleGameplayInventoryRequested.exchange(false))
    {
        GameplayInventory_Toggle();
    }
    if (g_gameplayInventoryOpen.load())
    {
        static unsigned int inventoryRaycastCheckFrame = 0;
        if (++inventoryRaycastCheckFrame % 15 == 0)
        {
            ExtendedMode_SetGameplayHudRaycastsEnabled(true);
        }
        if (!CheckpointSystem_IsTaskUiOpen())
        {
            GameplayInventory_ProcessMouse();
        }
    }
}

bool GameplayInventory_AddLootEx(
    int definitionIndex,
    int quality,
    bool appraised,
    unsigned int wearUnits,
    bool preferStaging,
    const std::string& source)
{
    if (definitionIndex < 0 ||
        definitionIndex >= kEquipmentInventoryDefinitionCount)
    {
        return false;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[definitionIndex];
    // Refresh occupancy from the live persistent/temporary state before
    // choosing a cell. Without this, multiple loot rolls while Tab is closed
    // can reuse the same stale view slot.
    GameplayInventory_Refresh();
    int storage = preferStaging
        ? kGameplayInventoryStaging
        : kGameplayInventorySupply;
    int x = 0;
    int y = 0;
    if (!GameplayInventory_FindFreeCell(
            storage,
            definition.width,
            definition.height,
            x,
            y))
    {
        const int fallback = storage == kGameplayInventorySupply
            ? kGameplayInventoryStaging
            : kGameplayInventorySupply;
        if (!GameplayInventory_FindFreeCell(
                fallback,
                definition.width,
                definition.height,
                x,
                y))
        {
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Gameplay inventory could not receive loot %S from %S: no free cell.",
                definition.id,
                source.c_str());
            return false;
        }
        storage = fallback;
    }
    GameplayInventoryTempItem item;
    item.definition = definitionIndex;
    item.quality = std::clamp(
        quality,
        0,
        kInventoryQualityCount - 1);
    item.appraised = appraised && wearUnits != 0;
    item.wearUnits =
        item.appraised ? wearUnits : 0;
    item.storage = storage;
    item.x = x;
    item.y = y;
    item.width = definition.width;
    item.height = definition.height;
    item.serial = ++g_gameplayInventoryTempSerial;
    g_gameplayInventoryTempItems.push_back(item);
    if (storage == kGameplayInventoryStaging)
    {
        g_gameplayInventoryTaskLayout = true;
    }
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Gameplay inventory accepted loot: id=%S quality=%S storage=%S source=%S totalTemporary=%u.",
        definition.id,
        kInventoryQualityLabels[item.quality],
        storage == kGameplayInventorySupply
            ? "supply"
            : "staging",
        source.c_str(),
        static_cast<unsigned int>(
            g_gameplayInventoryTempItems.size()));
    if (g_gameplayInventoryOpen.load())
    {
        GameplayInventory_Refresh();
    }
    return true;
}

bool GameplayInventory_AddLoot(
    int definitionIndex,
    int quality,
    bool preferStaging,
    const std::string& source)
{
    return GameplayInventory_AddLootEx(
        definitionIndex,
        quality,
        false,
        0,
        preferStaging,
        source);
}

bool GameplayInventory_AddLootBatch(
    const std::vector<GameplayInventoryLootRequest>& requests,
    const std::string& source)
{
    if (requests.empty())
    {
        return false;
    }
    const auto itemsSnapshot = g_gameplayInventoryTempItems;
    const bool layoutSnapshot =
        g_gameplayInventoryTaskLayout;
    for (const auto& request : requests)
    {
        if (!GameplayInventory_AddLoot(
                request.definition,
                request.quality,
                false,
                source))
        {
            g_gameplayInventoryTempItems = itemsSnapshot;
            g_gameplayInventoryTaskLayout = layoutSnapshot;
            GameplayInventory_Refresh();
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Gameplay inventory rolled back a loot batch from %S after an allocation failure.",
                source.c_str());
            return false;
        }
    }
    return true;
}

void GameplayInventory_Reset()
{
    g_gameplayInventoryRoot = nullptr;
    g_gameplayQuickHudRoot = nullptr;
    g_gameplayInventoryStatus = nullptr;
    g_gameplayInventoryBackpackLabel = nullptr;
    g_gameplayInventoryBackpackSlot = SceneUiElement{};
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        g_gameplayInventoryEquipmentSlots[category] =
            SceneUiElement{};
    }
    for (int index = 0;
         index < kInventoryQuickSlotCount;
         ++index)
    {
        g_gameplayInventoryQuickSlots[index] =
            GameplayQuickSlotUi{};
        g_gameplayHudQuickSlots[index] =
            GameplayQuickSlotUi{};
    }
    g_gameplayInventoryTaskLayout = false;
    g_gameplayInventoryRewardMode = false;
    g_gameplayInventoryViews.clear();
    g_gameplayInventoryTempItems.clear();
    for (int storage = 0;
         storage < kGameplayInventoryPanelCount;
         ++storage)
    {
        g_gameplayInventoryPanels[storage] = SceneUiElement{};
        g_gameplayInventoryPanelTitles[storage] = nullptr;
        for (int index = 0;
             index < kGameplayInventoryMaxGridCells;
             ++index)
        {
            g_gameplayInventoryGridCells[storage][index] =
                GameplayInventoryGridCell{};
        }
    }
    g_gameplayInventoryTempSerial = 0;
    g_gameplayInventoryDraggingView = -1;
    g_gameplayInventoryMouseWasDown = false;
    g_gameplayInventoryDragMoved = false;
    g_gameplayInventoryHoveredEquipmentCategory = -1;
    g_gameplayInventoryDragVisualWidth = 0.0f;
    g_gameplayInventoryDragVisualHeight = 0.0f;
    g_gameplayInventoryOpen = false;
    g_extendedToggleGameplayInventoryRequested = false;
}

