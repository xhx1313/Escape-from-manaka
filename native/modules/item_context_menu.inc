// Shared right-click menu and item inspection window.

enum ItemContextMenuAction
{
    ItemContextMenu_View = 0,
    ItemContextMenu_Appraise = 1,
    ItemContextMenu_Exhibit = 2,
    ItemContextMenu_Sell = 3,
    ItemContextMenu_Discard = 4,
};

constexpr int kItemContextMenuActionCount = 5;
constexpr float kItemContextMenuWidth = 0.165f;
constexpr float kItemContextMenuHeight = 0.300f;
constexpr int kItemInspectStatCount = 10;
constexpr float kItemInspectStatTooltipMinWidth = 0.120f;
constexpr float kItemInspectStatTooltipMaxWidth = 0.680f;
constexpr float kItemInspectStatTooltipMaxHeight = 0.065f;
constexpr float kItemInspectStatTooltipCursorOffset = 0.018f;
constexpr float kItemInspectStatTooltipEdgePadding = 0.022f;

static Il2CppObject* g_itemContextMenuRoot = nullptr;
static Il2CppObject* g_itemContextMenuCanvas = nullptr;
static SceneUiElement
    g_itemContextMenuRows[kItemContextMenuActionCount]{};
static Il2CppObject*
    g_itemContextMenuLabels[kItemContextMenuActionCount]{};
static float g_itemContextMenuRowMinY[kItemContextMenuActionCount]{};
static float g_itemContextMenuRowMaxY[kItemContextMenuActionCount]{};
static Il2CppObject* g_itemInspectRoot = nullptr;
static Il2CppObject* g_itemInspectCanvas = nullptr;
static SceneUiElement g_itemInspectTitleBar{};
static SceneUiElement g_itemInspectCloseButton{};
static Il2CppObject* g_itemInspectName = nullptr;
static SceneUiElement g_itemInspectImage{};
static Il2CppObject* g_itemInspectStats = nullptr;
static Il2CppObject* g_itemInspectEquipmentStats = nullptr;
static SceneUiElement
    g_itemInspectStatElements[kItemInspectStatCount]{};
static float
    g_itemInspectStatMinX[kItemInspectStatCount]{};
static float
    g_itemInspectStatMaxX[kItemInspectStatCount]{};
static int
    g_itemInspectVisibleStats[kItemInspectStatCount]{};
static int g_itemInspectVisibleStatCount = 0;
static int g_itemInspectHoveredStat = -1;
static int g_itemInspectDefinition = -1;
static SceneUiElement g_itemInspectStatTooltip{};
static SceneUiElement g_itemInspectStatTooltipText{};
static float g_itemInspectStatTooltipWidth =
    kItemInspectStatTooltipMinWidth;
static float g_itemInspectStatTooltipHeight = 0.045f;
static Il2CppObject* g_itemInspectDescription = nullptr;
static SceneUiElement g_itemInspectBadgeCircle{};
static SceneUiElement g_itemInspectBadgeText{};
static bool g_itemContextMenuOpen = false;
static bool g_itemInspectOpen = false;
static bool g_itemInspectDragging = false;
static bool g_itemContextConsumeUntilRelease = false;
static bool g_itemContextInGameplay = false;
static int g_itemContextDefinition = -1;
static int g_itemContextQuality = 0;
static unsigned int g_itemContextObservedRightSerial = 0;
static unsigned int g_itemContextObservedLeftSerial = 0;
static float g_itemContextMinX = 0.0f;
static float g_itemContextMinY = 0.0f;
static float g_itemContextMaxX = 0.0f;
static float g_itemContextMaxY = 0.0f;
static float g_itemInspectX = 0.25f;
static float g_itemInspectY = 0.08f;
static float g_itemInspectWidth = 0.52f;
static float g_itemInspectHeight = 0.82f;
static float g_itemInspectDragOffsetX = 0.0f;
static float g_itemInspectDragOffsetY = 0.0f;
static Il2CppObject* g_itemAppraisalRoot = nullptr;
static Il2CppObject* g_itemAppraisalCanvas = nullptr;
static SceneUiElement g_itemAppraisalTitleBar{};
static SceneUiElement g_itemAppraisalCloseButton{};
static Il2CppObject* g_itemAppraisalName = nullptr;
static SceneUiElement g_itemAppraisalImage{};
static SceneUiElement g_itemAppraisalButton{};
static Il2CppObject* g_itemAppraisalButtonLabel = nullptr;
static bool g_itemAppraisalOpen = false;
static bool g_itemAppraisalDragging = false;
static bool g_itemAppraisalRolling = false;
static int g_itemAppraisalItemIndex = -1;
static unsigned long long g_itemAppraisalStartedTick = 0;
static unsigned long long g_itemAppraisalNextRandomTick = 0;
static unsigned long long g_itemAppraisalNextDigitCheckTick = 0;
static int g_itemAppraisalDigitIndex = 0;
static int g_itemAppraisalFrozenDigitCount = 0;
static float g_itemAppraisalX = 0.29f;
static float g_itemAppraisalY = 0.12f;
static float g_itemAppraisalWidth = 0.42f;
static float g_itemAppraisalHeight = 0.70f;
static float g_itemAppraisalDragOffsetX = 0.0f;
static float g_itemAppraisalDragOffsetY = 0.0f;
static bool g_itemContextCanAppraise = false;
static bool g_itemContextCanExhibit = false;
static bool g_itemContextCanSell = false;
static int g_itemContextItemIndex = -1;
static int g_itemContextGameplayTempIndex = -1;
static Il2CppObject* g_itemDiscardRoot = nullptr;
static Il2CppObject* g_itemDiscardCanvas = nullptr;
static SceneUiElement g_itemDiscardYesButton{};
static SceneUiElement g_itemDiscardNoButton{};
static bool g_itemDiscardOpen = false;
static Il2CppObject* g_itemSaleRoot = nullptr;
static Il2CppObject* g_itemSaleCanvas = nullptr;
static Il2CppObject* g_itemSaleMessage = nullptr;
static SceneUiElement g_itemSaleYesButton{};
static SceneUiElement g_itemSaleNoButton{};
static bool g_itemSaleOpen = false;
static long long g_itemSaleAmount = 0;

bool ItemContextMenu_PointIn(
    float x,
    float y,
    float minX,
    float minY,
    float maxX,
    float maxY)
{
    return x >= minX && x <= maxX &&
           y >= minY && y <= maxY;
}

Il2CppObject* ItemContextMenu_GetCanvas()
{
    if (g_extendedModeState.load() == ExtendedMode_Gameplay &&
        ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        return g_extendedGameplayHudCanvasObject;
    }
    if (g_extendedModeState.load() == ExtendedMode_Active &&
        ExtendedMode_IsUnityObjectAlive(g_extendedCanvas))
    {
        return g_extendedCanvas;
    }
    return nullptr;
}

void ItemContextMenu_CloseMenu()
{
    const bool wasOpen = g_itemContextMenuOpen;
    if (g_itemContextMenuRoot != nullptr)
    {
        ExtendedMode_DestroyUnityObject(g_itemContextMenuRoot);
    }
    g_itemContextMenuCanvas = nullptr;
    for (auto& row : g_itemContextMenuRows)
    {
        row = SceneUiElement{};
    }
    for (auto*& label : g_itemContextMenuLabels)
    {
        label = nullptr;
    }
    g_itemContextMenuOpen = false;
    if (wasOpen)
    {
        LogDebug(
            LogCategory_Inventory,
            L"ContextMenuClose",
            L"Item context menu closed.");
    }
}

void ItemContextMenu_CloseInspect()
{
    const bool wasOpen = g_itemInspectOpen;
    if (g_itemInspectRoot != nullptr)
    {
        ExtendedMode_DestroyUnityObject(g_itemInspectRoot);
    }
    g_itemInspectCanvas = nullptr;
    g_itemInspectTitleBar = SceneUiElement{};
    g_itemInspectCloseButton = SceneUiElement{};
    g_itemInspectName = nullptr;
    g_itemInspectImage = SceneUiElement{};
    g_itemInspectStats = nullptr;
    g_itemInspectEquipmentStats = nullptr;
    for (auto& element : g_itemInspectStatElements)
    {
        element = SceneUiElement{};
    }
    for (int index = 0; index < kItemInspectStatCount; ++index)
    {
        g_itemInspectStatMinX[index] = 0.0f;
        g_itemInspectStatMaxX[index] = 0.0f;
        g_itemInspectVisibleStats[index] = -1;
    }
    g_itemInspectVisibleStatCount = 0;
    g_itemInspectHoveredStat = -1;
    g_itemInspectDefinition = -1;
    g_itemInspectStatTooltip = SceneUiElement{};
    g_itemInspectStatTooltipText = SceneUiElement{};
    g_itemInspectStatTooltipWidth =
        kItemInspectStatTooltipMinWidth;
    g_itemInspectStatTooltipHeight = 0.045f;
    g_itemInspectDescription = nullptr;
    g_itemInspectBadgeCircle = SceneUiElement{};
    g_itemInspectBadgeText = SceneUiElement{};
    g_itemInspectOpen = false;
    g_itemInspectDragging = false;
    if (wasOpen)
    {
        LogDebug(
            LogCategory_Inventory,
            L"InspectClose",
            L"Item inspect window closed.");
    }
}

void ItemContextMenu_CloseAppraisal()
{
    const bool wasOpen = g_itemAppraisalOpen;
    if (g_itemAppraisalRoot != nullptr)
    {
        ExtendedMode_DestroyUnityObject(g_itemAppraisalRoot);
    }
    g_itemAppraisalCanvas = nullptr;
    g_itemAppraisalTitleBar = SceneUiElement{};
    g_itemAppraisalCloseButton = SceneUiElement{};
    g_itemAppraisalName = nullptr;
    g_itemAppraisalImage = SceneUiElement{};
    g_itemAppraisalButton = SceneUiElement{};
    g_itemAppraisalButtonLabel = nullptr;
    g_itemAppraisalOpen = false;
    g_itemAppraisalDragging = false;
    g_itemAppraisalRolling = false;
    g_itemAppraisalItemIndex = -1;
    if (wasOpen)
    {
        LogDebug(
            LogCategory_Inventory,
            L"AppraisalClose",
            L"Item appraisal window closed.");
    }
}

void ItemContextMenu_CloseDiscard()
{
    const bool wasOpen = g_itemDiscardOpen;
    if (g_itemDiscardRoot != nullptr)
    {
        ExtendedMode_DestroyUnityObject(g_itemDiscardRoot);
    }
    g_itemDiscardCanvas = nullptr;
    g_itemDiscardYesButton = SceneUiElement{};
    g_itemDiscardNoButton = SceneUiElement{};
    g_itemDiscardOpen = false;
    if (wasOpen)
    {
        LogDebug(
            LogCategory_Inventory,
            L"DiscardClose",
            L"Item discard confirmation closed.");
    }
}

void ItemContextMenu_CloseSale()
{
    const bool wasOpen = g_itemSaleOpen;
    if (g_itemSaleRoot != nullptr)
    {
        ExtendedMode_DestroyUnityObject(g_itemSaleRoot);
    }
    g_itemSaleCanvas = nullptr;
    g_itemSaleMessage = nullptr;
    g_itemSaleYesButton = SceneUiElement{};
    g_itemSaleNoButton = SceneUiElement{};
    g_itemSaleOpen = false;
    g_itemSaleAmount = 0;
    if (wasOpen)
    {
        LogDebug(
            LogCategory_Inventory,
            L"SaleClose",
            L"Item sale confirmation closed.");
    }
}

void ItemContextMenu_Reset()
{
    ItemContextMenu_CloseMenu();
    ItemContextMenu_CloseInspect();
    ItemContextMenu_CloseAppraisal();
    ItemContextMenu_CloseDiscard();
    ItemContextMenu_CloseSale();
    g_itemContextConsumeUntilRelease = false;
    g_itemContextDefinition = -1;
    g_itemContextQuality = 0;
    g_itemContextItemIndex = -1;
    g_itemContextGameplayTempIndex = -1;
    g_itemContextCanAppraise = false;
    g_itemContextCanExhibit = false;
    g_itemContextCanSell = false;
    g_itemContextObservedRightSerial =
        g_extendedManualRightPressSerial.load();
    g_itemContextObservedLeftSerial =
        g_extendedManualPointerPressSerial.load();
}

bool ItemContextMenu_IsOpen()
{
    return g_itemContextMenuOpen ||
           g_itemInspectOpen ||
           g_itemAppraisalOpen ||
           g_itemDiscardOpen ||
           g_itemSaleOpen;
}

bool ItemContextMenu_ShouldBlockPointer()
{
    return g_itemContextMenuOpen ||
           g_itemInspectOpen ||
           g_itemAppraisalOpen ||
           g_itemDiscardOpen ||
           g_itemSaleOpen ||
           g_itemContextConsumeUntilRelease;
}

std::wstring ItemContextMenu_CategoryText(int definitionIndex)
{
    if (definitionIndex < 0 ||
        definitionIndex >= kEquipmentInventoryDefinitionCount)
    {
        return L"未知";
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[definitionIndex];
    std::wstring text;
    for (int category = 0;
         category < kInventoryEquipmentCategoryCount;
         ++category)
    {
        if ((definition.categories &
             kInventoryEquipmentCategories[category].bit) == 0)
        {
            continue;
        }
        if (!text.empty())
        {
            text += L" / ";
        }
        text += Utf8ToWide(
            kInventoryEquipmentCategories[category].label);
    }
    if ((definition.categories &
         kInventoryBackpackCategoryBit) != 0)
    {
        if (!text.empty())
        {
            text += L" / ";
        }
        text += L"背包";
    }
    return text.empty() ? L"其他" : text;
}

int ItemContextMenu_FindItemAt(
    float x,
    float y,
    int& quality,
    int& selectedItemIndex,
    int& selectedGameplayTempIndex)
{
    quality = 0;
    selectedItemIndex = -1;
    selectedGameplayTempIndex = -1;
    if (g_gameplayInventoryOpen.load())
    {
        for (int index =
                 static_cast<int>(g_gameplayInventoryViews.size()) - 1;
             index >= 0;
             --index)
        {
            const auto& view =
                g_gameplayInventoryViews[static_cast<size_t>(index)];
            if (view.definition >= 0 &&
                ItemContextMenu_PointIn(
                    x,
                    y,
                    view.minX,
                    view.minY,
                    view.maxX,
                    view.maxY))
            {
                quality = view.quality;
                selectedItemIndex = view.itemIndex;
                selectedGameplayTempIndex = view.tempIndex;
                return view.definition;
            }
        }
        return -1;
    }
    if (g_extendedModeState.load() != ExtendedMode_Active ||
        g_inventoryActiveRoute != kInventoryRouteEquipment)
    {
        return -1;
    }
    for (int index = kInventoryItemSlotCount - 1;
         index >= 0;
         --index)
    {
        const auto& item = g_inventoryItems[index];
        if (item.storage < 0 ||
            item.storage >= kInventoryGridStorageCount ||
            !ItemContextMenu_PointIn(
                x,
                y,
                item.screenMinX,
                item.screenMinY,
                item.screenMaxX,
                item.screenMaxY))
        {
            continue;
        }
        quality = item.quality;
        selectedItemIndex = index;
        return item.definition;
    }
    const int category = InventoryUi_EquipmentSlotAt(x, y);
    if (category >= 0)
    {
        const int itemIndex = InventoryUi_FindEquippedItem(category);
        if (itemIndex >= 0)
        {
            quality = g_inventoryItems[itemIndex].quality;
            selectedItemIndex = itemIndex;
            return g_inventoryItems[itemIndex].definition;
        }
    }
    return -1;
}

void ItemContextMenu_BuildMenu(
    Il2CppObject* canvas,
    float cursorX,
    float cursorY)
{
    if (canvas == nullptr)
    {
        return;
    }
    if (g_itemContextMenuRoot != nullptr &&
        g_itemContextMenuCanvas != canvas)
    {
        ItemContextMenu_CloseMenu();
    }
    if (g_itemContextMenuRoot != nullptr)
    {
        SetWorldGameObjectActive(g_itemContextMenuRoot, false);
    }
    const float width = kItemContextMenuWidth;
    const float height = kItemContextMenuHeight;
    float minX = cursorX + 0.008f;
    if (minX + width > 0.992f)
    {
        minX = cursorX - width - 0.008f;
    }
    minX = std::clamp(minX, 0.006f, 0.994f - width);
    float minY = cursorY - height * 0.35f;
    minY = std::clamp(minY, 0.006f, 0.994f - height);
    const float maxX = minX + width;
    const float maxY = minY + height;
    g_itemContextMinX = minX;
    g_itemContextMinY = minY;
    g_itemContextMaxX = maxX;
    g_itemContextMaxY = maxY;

    constexpr float padding = 0.018f;
    constexpr float rowGap = 0.012f;
    const bool visibleActions[kItemContextMenuActionCount] = {
        true,
        g_itemContextCanAppraise,
        g_itemContextCanExhibit,
        g_itemContextCanSell,
        true,
    };
    int visibleCount = 0;
    for (bool visible : visibleActions)
    {
        visibleCount += visible ? 1 : 0;
    }
    const float rowHeight =
        (1.0f -
         padding * 2.0f -
         rowGap * static_cast<float>(visibleCount - 1)) /
        static_cast<float>(visibleCount);

    if (g_itemContextMenuRoot == nullptr)
    {
        auto root = CreateScenePanel(
            "ItemContextMenu",
            GetWorldTransform(canvas),
            minX,
            minY,
            maxX,
            maxY,
            0.022f,
            0.027f,
            0.036f,
            0.985f);
        g_itemContextMenuRoot = root.gameObject;
        g_itemContextMenuCanvas = canvas;
        const char* labels[] = {
            "查看",
            "鉴定",
            "放入展览柜",
            "出售",
            "丢弃",
        };
        for (int index = 0;
             index < kItemContextMenuActionCount;
             ++index)
        {
            const float relativeMaxY =
                1.0f - padding -
                static_cast<float>(index) * (rowHeight + rowGap);
            const float relativeMinY = relativeMaxY - rowHeight;
            auto row = CreateScenePanel(
                "ItemContextMenuAction",
                root.rect,
                padding,
                relativeMinY,
                1.0f - padding,
                relativeMaxY,
                index == ItemContextMenu_View ? 0.075f : 0.045f,
                index == ItemContextMenu_View ? 0.125f : 0.055f,
                index == ItemContextMenu_View ? 0.170f : 0.070f,
                0.99f);
            g_itemContextMenuRows[index] = row;
            g_itemContextMenuLabels[index] = CreateSceneTextElement(
                "ItemContextMenuLabel",
                row.rect,
                labels[index],
                0.10f,
                0.10f,
                0.90f,
                0.90f,
                18.0f,
                514,
                index == ItemContextMenu_View ? 0.96f :
                    (index == ItemContextMenu_Exhibit ? 0.68f :
                        (index == ItemContextMenu_Discard ? 1.0f : 0.58f)),
                index == ItemContextMenu_View ? 0.98f :
                    (index == ItemContextMenu_Exhibit ? 0.86f :
                        (index == ItemContextMenu_Discard ? 0.42f : 0.61f)),
                index == ItemContextMenu_View ? 1.0f :
                    (index == ItemContextMenu_Exhibit ? 0.82f :
                        (index == ItemContextMenu_Discard ? 0.36f : 0.66f)),
                1.0f).text;
        }
    }
    else
    {
        SetSceneRect(
            GetSceneComponentByClass(
                g_itemContextMenuRoot,
                "UnityEngine",
                "RectTransform"),
            minX,
            minY,
            maxX,
            maxY);
        // Row controls use normalized coordinates under the root, so their
        // geometry follows the moved root without another rebuild.
    }
    int visibleRow = 0;
    for (int index = 0;
         index < kItemContextMenuActionCount;
         ++index)
    {
        const bool visible = visibleActions[index];
        SetWorldGameObjectActive(
            g_itemContextMenuRows[index].gameObject,
            visible);
        if (!visible)
        {
            g_itemContextMenuRowMinY[index] = 0.0f;
            g_itemContextMenuRowMaxY[index] = 0.0f;
            continue;
        }
        const float relativeMaxY =
            1.0f - padding -
            static_cast<float>(visibleRow) * (rowHeight + rowGap);
        const float relativeMinY = relativeMaxY - rowHeight;
        SetSceneRect(
            g_itemContextMenuRows[index].rect,
            padding,
            relativeMinY,
            1.0f - padding,
            relativeMaxY);
        g_itemContextMenuRowMinY[index] =
            minY + height * relativeMinY;
        g_itemContextMenuRowMaxY[index] =
            minY + height * relativeMaxY;
        ++visibleRow;
    }
    SetWorldGameObjectActive(g_itemContextMenuRoot, true);
    BringSceneObjectToFront(g_itemContextMenuRoot);
    g_itemContextMenuOpen = true;
}

void ItemContextMenu_BuildInspectWindow(Il2CppObject* canvas)
{
    if (canvas == nullptr)
    {
        return;
    }
    if (g_itemInspectRoot != nullptr &&
        g_itemInspectCanvas != canvas)
    {
        ItemContextMenu_CloseInspect();
    }
    if (g_itemInspectRoot != nullptr)
    {
        return;
    }
    g_itemInspectX = std::clamp(
        g_itemInspectX,
        0.008f,
        0.992f - g_itemInspectWidth);
    g_itemInspectY = std::clamp(
        g_itemInspectY,
        0.008f,
        0.992f - g_itemInspectHeight);
    auto root = CreateScenePanel(
        "ItemInspectWindow",
        GetWorldTransform(canvas),
        g_itemInspectX,
        g_itemInspectY,
        g_itemInspectX + g_itemInspectWidth,
        g_itemInspectY + g_itemInspectHeight,
        0.018f,
        0.022f,
        0.030f,
        0.995f);
    g_itemInspectRoot = root.gameObject;
    g_itemInspectCanvas = canvas;
    g_itemInspectTitleBar = CreateScenePanel(
        "ItemInspectTitleBar",
        root.rect,
        0.0f,
        0.895f,
        1.0f,
        1.0f,
        0.060f,
        0.073f,
        0.088f,
        0.99f);
    g_itemInspectName = CreateSceneTextElement(
        "ItemInspectName",
        g_itemInspectTitleBar.rect,
        "",
        0.025f,
        0.10f,
        0.925f,
        0.90f,
        22.0f,
        513,
        0.96f,
        0.97f,
        0.99f,
        1.0f).text;
    g_itemInspectCloseButton = CreateScenePanel(
        "ItemInspectClose",
        g_itemInspectTitleBar.rect,
        0.935f,
        0.12f,
        0.985f,
        0.88f,
        0.44f,
        0.08f,
        0.08f,
        1.0f);
    CreateSceneTextElement(
        "ItemInspectCloseLabel",
        g_itemInspectCloseButton.rect,
        "×",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        22.0f,
        514,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    auto previewBackground = CreateScenePanel(
        "ItemInspectPreviewBackground",
        root.rect,
        0.045f,
        0.480f,
        0.955f,
        0.855f,
        0.035f,
        0.040f,
        0.050f,
        1.0f);
    g_itemInspectImage = CreateSceneRawImageElement(
        "ItemInspectImage",
        previewBackground.rect,
        L"",
        0.08f,
        0.08f,
        0.92f,
        0.92f,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    AddSceneComponentToGameObject(
        g_itemInspectImage.gameObject,
        "UnityEngine.UI",
        "AspectRatioFitter");
    g_itemInspectBadgeCircle = CreateSceneTextElement(
        "ItemInspectBadgeCircle",
        previewBackground.rect,
        "●",
        0.010f,
        0.790f,
        0.180f,
        0.990f,
        30.0f,
        514,
        0.62f,
        0.65f,
        0.68f,
        1.0f);
    g_itemInspectBadgeText = CreateSceneTextElement(
        "ItemInspectBadgeText",
        g_itemInspectBadgeCircle.rect,
        "",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        15.0f,
        514,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    SetWorldGameObjectActive(
        g_itemInspectBadgeCircle.gameObject,
        false);
    SetWorldGameObjectActive(
        g_itemInspectBadgeText.gameObject,
        false);
    g_itemInspectStats = CreateSceneTextElement(
        "ItemInspectStats",
        root.rect,
        "",
        0.055f,
        0.365f,
        0.945f,
        0.465f,
        14.0f,
        514,
        0.78f,
        0.86f,
        0.96f,
        1.0f).text;
    g_itemInspectEquipmentStats = CreateSceneTextElement(
        "ItemInspectEquipmentStats",
        root.rect,
        "",
        0.055f,
        0.315f,
        0.945f,
        0.355f,
        13.0f,
        514,
        0.86f,
        0.92f,
        1.0f,
        1.0f).text;
    for (int index = 0; index < kItemInspectStatCount; ++index)
    {
        char objectName[64]{};
        sprintf_s(
            objectName,
            "ItemInspectStat%02d",
            index);
        auto element = CreateSceneTextElement(
            objectName,
            root.rect,
            "",
            0.055f,
            0.315f,
            0.055f,
            0.355f,
            12.0f,
            513,
            0.86f,
            0.92f,
            1.0f,
            1.0f);
        g_itemInspectStatElements[index] = element;
        SetWorldGameObjectActive(element.gameObject, false);
        g_itemInspectStatMinX[index] = 0.0f;
        g_itemInspectStatMaxX[index] = 0.0f;
        g_itemInspectVisibleStats[index] = -1;
    }
    CreateScenePanel(
        "ItemInspectDivider",
        root.rect,
        0.055f,
        0.300f,
        0.945f,
        0.303f,
        0.18f,
        0.22f,
        0.27f,
        1.0f);
    g_itemInspectDescription = CreateSceneTextElement(
        "ItemInspectDescription",
        root.rect,
        "",
        0.055f,
        0.165f,
        0.945f,
        0.285f,
        15.0f,
        513,
        0.82f,
        0.85f,
        0.89f,
        1.0f).text;
    g_itemInspectStatTooltip = CreateScenePanel(
        "ItemInspectStatTooltip",
        root.rect,
        kItemInspectStatTooltipEdgePadding,
        kItemInspectStatTooltipEdgePadding,
        kItemInspectStatTooltipEdgePadding +
            g_itemInspectStatTooltipWidth,
        kItemInspectStatTooltipEdgePadding +
            g_itemInspectStatTooltipHeight,
        0.035f,
        0.048f,
        0.070f,
        0.990f);
    g_itemInspectStatTooltipText = CreateSceneTextElement(
        "ItemInspectStatTooltipText",
        g_itemInspectStatTooltip.rect,
        "",
        0.025f,
        0.10f,
        0.975f,
        0.90f,
        12.0f,
        513,
        0.90f,
        0.94f,
        1.0f,
        1.0f);
    SetWorldGameObjectActive(
        g_itemInspectStatTooltip.gameObject,
        false);
    SetWorldGameObjectActive(
        g_itemInspectStatTooltipText.gameObject,
        false);
    SetWorldGameObjectActive(g_itemInspectRoot, false);
}

void ItemContextMenu_SetPreviewTexture(
    SceneUiElement& image,
    int definition,
    int quality)
{
    if (definition < 0 ||
        definition >= kEquipmentInventoryDefinitionCount)
    {
        return;
    }
    const auto& itemDefinition =
        kEquipmentInventoryDefinitions[definition];
    const auto texture = LoadScenePngTexture(
        InventoryUi_EquipmentImagePath(
            itemDefinition.id,
            quality));
    image.texture = texture;
    SetSceneRawImageTexture(image.rawImage, texture);
    auto* aspectFitter = GetSceneComponentByClass(
        image.gameObject,
        "UnityEngine.UI",
        "AspectRatioFitter");
    if (aspectFitter == nullptr || texture == nullptr)
    {
        return;
    }
    auto* textureClass = FindClass("UnityEngine", "Texture");
    auto* textureWidthValue = Invoke(
        FindMethod(textureClass, "get_width", 0),
        texture,
        nullptr);
    auto* textureHeightValue = Invoke(
        FindMethod(textureClass, "get_height", 0),
        texture,
        nullptr);
    if (textureWidthValue == nullptr ||
        textureHeightValue == nullptr)
    {
        return;
    }
    const int textureWidth =
        *static_cast<int*>(
            il2cpp_object_unbox(textureWidthValue));
    const int textureHeight =
        *static_cast<int*>(
            il2cpp_object_unbox(textureHeightValue));
    if (textureWidth <= 0 || textureHeight <= 0)
    {
        return;
    }
    float aspectRatio =
        static_cast<float>(textureWidth) /
        static_cast<float>(textureHeight);
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

void ItemContextMenu_BuildAppraisalWindow(Il2CppObject* canvas)
{
    if (canvas == nullptr)
    {
        return;
    }
    if (g_itemAppraisalRoot != nullptr &&
        g_itemAppraisalCanvas != canvas)
    {
        ItemContextMenu_CloseAppraisal();
    }
    if (g_itemAppraisalRoot != nullptr)
    {
        return;
    }
    g_itemAppraisalX = std::clamp(
        g_itemAppraisalX,
        0.008f,
        0.992f - g_itemAppraisalWidth);
    g_itemAppraisalY = std::clamp(
        g_itemAppraisalY,
        0.008f,
        0.992f - g_itemAppraisalHeight);
    auto root = CreateScenePanel(
        "ItemAppraisalWindow",
        GetWorldTransform(canvas),
        g_itemAppraisalX,
        g_itemAppraisalY,
        g_itemAppraisalX + g_itemAppraisalWidth,
        g_itemAppraisalY + g_itemAppraisalHeight,
        0.018f,
        0.022f,
        0.030f,
        0.995f);
    g_itemAppraisalRoot = root.gameObject;
    g_itemAppraisalCanvas = canvas;
    g_itemAppraisalTitleBar = CreateScenePanel(
        "ItemAppraisalTitleBar",
        root.rect,
        0.0f,
        0.880f,
        1.0f,
        1.0f,
        0.060f,
        0.073f,
        0.088f,
        0.99f);
    g_itemAppraisalName = CreateSceneTextElement(
        "ItemAppraisalName",
        g_itemAppraisalTitleBar.rect,
        "",
        0.030f,
        0.10f,
        0.925f,
        0.90f,
        22.0f,
        513,
        0.96f,
        0.97f,
        0.99f,
        1.0f).text;
    g_itemAppraisalCloseButton = CreateScenePanel(
        "ItemAppraisalClose",
        g_itemAppraisalTitleBar.rect,
        0.935f,
        0.12f,
        0.985f,
        0.88f,
        0.44f,
        0.08f,
        0.08f,
        1.0f);
    CreateSceneTextElement(
        "ItemAppraisalCloseLabel",
        g_itemAppraisalCloseButton.rect,
        "×",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        22.0f,
        514,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    auto previewBackground = CreateScenePanel(
        "ItemAppraisalPreviewBackground",
        root.rect,
        0.055f,
        0.285f,
        0.945f,
        0.850f,
        0.035f,
        0.040f,
        0.050f,
        1.0f);
    g_itemAppraisalImage = CreateSceneRawImageElement(
        "ItemAppraisalImage",
        previewBackground.rect,
        L"",
        0.08f,
        0.08f,
        0.92f,
        0.92f,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    AddSceneComponentToGameObject(
        g_itemAppraisalImage.gameObject,
        "UnityEngine.UI",
        "AspectRatioFitter");
    g_itemAppraisalButton = CreateScenePanel(
        "ItemAppraisalButton",
        root.rect,
        0.120f,
        0.055f,
        0.880f,
        0.215f,
        0.075f,
        0.125f,
        0.170f,
        1.0f);
    g_itemAppraisalButtonLabel = CreateSceneTextElement(
        "ItemAppraisalButtonLabel",
        g_itemAppraisalButton.rect,
        "鉴定",
        0.04f,
        0.08f,
        0.96f,
        0.92f,
        20.0f,
        514,
        0.96f,
        0.98f,
        1.0f,
        1.0f).text;
    SetWorldGameObjectActive(g_itemAppraisalRoot, false);
}

void ItemContextMenu_UpdateAppraisalButtonText(const std::wstring& text)
{
    if (g_itemAppraisalButtonLabel == nullptr)
    {
        return;
    }
    const auto utf8 = WideToUtf8(text);
    SetSceneText(
        g_itemAppraisalButtonLabel,
        utf8.c_str(),
        20.0f,
        514,
        0.96f,
        0.98f,
        1.0f,
        1.0f);
}

void ItemContextMenu_OpenAppraisal(int itemIndex)
{
    if (!InventoryUi_CanAppraiseItem(itemIndex))
    {
        return;
    }
    auto* canvas = ItemContextMenu_GetCanvas();
    if (canvas == nullptr)
    {
        return;
    }
    ItemContextMenu_BuildAppraisalWindow(canvas);
    if (g_itemAppraisalRoot == nullptr)
    {
        return;
    }
    const auto& item = g_inventoryItems[itemIndex];
    const auto& definition =
        kEquipmentInventoryDefinitions[item.definition];
    SetSceneText(
        g_itemAppraisalName,
        definition.name,
        22.0f,
        513,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    ItemContextMenu_SetPreviewTexture(
        g_itemAppraisalImage,
        item.definition,
        item.quality);
    ItemContextMenu_UpdateAppraisalButtonText(L"鉴定");
    SetSceneRect(
        GetSceneComponentByClass(
            g_itemAppraisalRoot,
            "UnityEngine",
            "RectTransform"),
        g_itemAppraisalX,
        g_itemAppraisalY,
        g_itemAppraisalX + g_itemAppraisalWidth,
        g_itemAppraisalY + g_itemAppraisalHeight);
    SetWorldGameObjectActive(g_itemAppraisalRoot, true);
    BringSceneObjectToFront(g_itemAppraisalRoot);
    g_itemAppraisalOpen = true;
    g_itemAppraisalDragging = false;
    g_itemAppraisalRolling = false;
    g_itemAppraisalItemIndex = itemIndex;
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Item appraisal window opened: definition=%d itemSlot=%d.",
        item.definition,
        itemIndex);
}

void ItemContextMenu_StartAppraisal()
{
    if (!g_itemAppraisalOpen ||
        !InventoryUi_CanAppraiseItem(g_itemAppraisalItemIndex))
    {
        return;
    }
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<unsigned int> distribution(
        1u,
        499999999u);
    auto& item = g_inventoryItems[g_itemAppraisalItemIndex];
    item.appraised = true;
    item.wearUnits = distribution(generator);
    InventoryUi_UpdateItemRect(g_itemAppraisalItemIndex);
    g_itemAppraisalStartedTick = GetTickCount64();
    g_itemAppraisalNextRandomTick = g_itemAppraisalStartedTick;
    g_itemAppraisalDigitIndex = 0;
    g_itemAppraisalFrozenDigitCount = 0;
    g_itemAppraisalNextDigitCheckTick =
        g_itemAppraisalStartedTick + 1000ull;
    g_itemAppraisalRolling = true;
    ItemContextMenu_UpdateAppraisalButtonText(L"0.00000000");
    InventoryUi_SaveState();
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Item appraisal started: itemSlot=%d wearUnits=%u firstCheckDigit=%d.",
        g_itemAppraisalItemIndex,
        item.wearUnits,
        g_itemAppraisalDigitIndex);
}

void ItemContextMenu_UpdateAppraisalRollingText(
    const InventoryItemState& item)
{
    constexpr unsigned int divisors[9] = {
        100000000u,
        10000000u,
        1000000u,
        100000u,
        10000u,
        1000u,
        100u,
        10u,
        1u,
    };
    static std::mt19937 rollingGenerator(std::random_device{}());
    std::uniform_int_distribution<int> rollingDigit(0, 9);
    wchar_t rolling[16]{};
    int offset = 0;
    for (int digit = 0; digit < 9; ++digit)
    {
        if (digit == 1)
        {
            rolling[offset++] = L'.';
        }
        const bool frozen =
            digit < g_itemAppraisalFrozenDigitCount;
        const int value = frozen
            ? static_cast<int>(
                (item.wearUnits / divisors[digit]) % 10u)
            : rollingDigit(rollingGenerator);
        rolling[offset++] =
            static_cast<wchar_t>(L'0' + value);
    }
    rolling[offset] = L'\0';
    ItemContextMenu_UpdateAppraisalButtonText(rolling);
}

void ItemContextMenu_UpdateAppraisalAnimation()
{
    if (!g_itemAppraisalRolling ||
        !InventoryUi_IsItemSlot(g_itemAppraisalItemIndex))
    {
        return;
    }
    const auto& item = g_inventoryItems[g_itemAppraisalItemIndex];
    const unsigned long long now = GetTickCount64();
    if (now >= g_itemAppraisalNextDigitCheckTick)
    {
        constexpr unsigned int divisors[9] = {
            100000000u,
            10000000u,
            1000000u,
            100000u,
            10000u,
            1000u,
            100u,
            10u,
            1u,
        };
        const int digitIndex = std::clamp(
            g_itemAppraisalDigitIndex,
            0,
            8);
        const unsigned int digit =
            (item.wearUnits / divisors[digitIndex]) % 10u;
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Item appraisal digit check: itemSlot=%d digitIndex=%d digit=%u.",
            g_itemAppraisalItemIndex,
            digitIndex,
            digit);
        if (digit != 0u)
        {
            ItemContextMenu_UpdateAppraisalButtonText(
                InventoryUi_FormatWear(item.wearUnits));
            g_itemAppraisalRolling = false;
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Item appraisal revealed: itemSlot=%d wearUnits=%u checkedDigit=%d.",
                g_itemAppraisalItemIndex,
                item.wearUnits,
                digitIndex);
            return;
        }
        g_itemAppraisalFrozenDigitCount = digitIndex + 1;
        ++g_itemAppraisalDigitIndex;
        g_itemAppraisalNextDigitCheckTick = now + 1000ull;
        g_itemAppraisalNextRandomTick = now;
        ItemContextMenu_UpdateAppraisalRollingText(item);
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Item appraisal zero frozen: itemSlot=%d digitIndex=%d frozenDigits=%d nextCheckMs=1000.",
            g_itemAppraisalItemIndex,
            digitIndex,
            g_itemAppraisalFrozenDigitCount);
        return;
    }
    if (now < g_itemAppraisalNextRandomTick)
    {
        return;
    }
    g_itemAppraisalNextRandomTick = now + 45ull;
    ItemContextMenu_UpdateAppraisalRollingText(item);
}

void ItemContextMenu_BuildDiscardWindow(Il2CppObject* canvas)
{
    if (canvas == nullptr)
    {
        return;
    }
    if (g_itemDiscardRoot != nullptr &&
        g_itemDiscardCanvas != canvas)
    {
        ItemContextMenu_CloseDiscard();
    }
    if (g_itemDiscardRoot != nullptr)
    {
        return;
    }
    constexpr float minX = 0.250f;
    constexpr float minY = 0.345f;
    constexpr float maxX = 0.750f;
    constexpr float maxY = 0.655f;
    auto root = CreateScenePanel(
        "ItemDiscardConfirmation",
        GetWorldTransform(canvas),
        minX,
        minY,
        maxX,
        maxY,
        0.055f,
        0.060f,
        0.070f,
        0.995f);
    g_itemDiscardRoot = root.gameObject;
    g_itemDiscardCanvas = canvas;
    CreateSceneTextElement(
        "ItemDiscardTitle",
        root.rect,
        "警告",
        0.055f,
        0.720f,
        0.945f,
        0.925f,
        24.0f,
        514,
        0.98f,
        0.72f,
        0.24f,
        1.0f);
    CreateSceneTextElement(
        "ItemDiscardMessage",
        root.rect,
        "这样做此物品会永久消失，\n你确定要这么做吗？",
        0.080f,
        0.330f,
        0.920f,
        0.700f,
        19.0f,
        514,
        0.94f,
        0.95f,
        0.97f,
        1.0f);
    g_itemDiscardYesButton = CreateSceneButton(
        "ItemDiscardYes",
        root.rect,
        "是",
        0.170f,
        0.105f,
        0.455f,
        0.275f,
        0.12f,
        0.42f,
        0.20f,
        1.0f,
        19.0f,
        514,
        true);
    g_itemDiscardNoButton = CreateSceneButton(
        "ItemDiscardNo",
        root.rect,
        "否",
        0.545f,
        0.105f,
        0.830f,
        0.275f,
        0.12f,
        0.14f,
        0.18f,
        1.0f,
        19.0f,
        514,
        true);
    SetWorldGameObjectActive(g_itemDiscardRoot, false);
}

void ItemContextMenu_OpenDiscardConfirmation()
{
    auto* canvas = ItemContextMenu_GetCanvas();
    if (canvas == nullptr)
    {
        return;
    }
    ItemContextMenu_BuildDiscardWindow(canvas);
    if (g_itemDiscardRoot == nullptr)
    {
        return;
    }
    SetWorldGameObjectActive(g_itemDiscardRoot, true);
    BringSceneObjectToFront(g_itemDiscardRoot);
    g_itemDiscardOpen = true;
    LogInfo(
        LogCategory_Inventory,
        L"DiscardOpen",
        L"Item discard confirmation opened: definition=%d itemSlot=%d.",
        g_itemContextDefinition,
        g_itemContextItemIndex);
}

void ItemContextMenu_DiscardSelectedItem()
{
    if (g_itemContextInGameplay &&
        g_itemContextGameplayTempIndex >= 0 &&
        g_itemContextGameplayTempIndex <
            static_cast<int>(
                g_gameplayInventoryTempItems.size()))
    {
        g_gameplayInventoryTempItems.erase(
            g_gameplayInventoryTempItems.begin() +
            g_itemContextGameplayTempIndex);
        GameplayInventory_Refresh();
        GameplayInventory_SetStatus("物品已永久丢弃");
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Gameplay temporary item discarded: definition=%d tempIndex=%d.",
            g_itemContextDefinition,
            g_itemContextGameplayTempIndex);
        return;
    }
    if (!InventoryUi_IsItemSlot(g_itemContextItemIndex))
    {
        return;
    }
    auto& item = g_inventoryItems[g_itemContextItemIndex];
    if (item.element.gameObject != nullptr)
    {
        SetWorldGameObjectActive(item.element.gameObject, false);
    }
    const int definition = item.definition;
    const int storage = item.storage;
    item = InventoryItemState{};
    item.storage = -1;
    InventoryUi_RefreshEquipment();
    InventoryUi_SaveState();
    if (g_itemContextInGameplay)
    {
        GameplayInventory_Refresh();
        GameplayInventory_SetStatus("物品已永久丢弃");
    }
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Persistent item discarded: definition=%d slot=%d storage=%d gameplay=%d.",
        definition,
        g_itemContextItemIndex,
        storage,
        g_itemContextInGameplay ? 1 : 0);
}

void ItemContextMenu_BuildSaleWindow(Il2CppObject* canvas)
{
    if (canvas == nullptr)
    {
        return;
    }
    if (g_itemSaleRoot != nullptr &&
        g_itemSaleCanvas != canvas)
    {
        ItemContextMenu_CloseSale();
    }
    if (g_itemSaleRoot != nullptr)
    {
        return;
    }
    constexpr float minX = 0.250f;
    constexpr float minY = 0.345f;
    constexpr float maxX = 0.750f;
    constexpr float maxY = 0.655f;
    auto root = CreateScenePanel(
        "ItemSaleConfirmation",
        GetWorldTransform(canvas),
        minX,
        minY,
        maxX,
        maxY,
        0.055f,
        0.060f,
        0.070f,
        0.995f);
    g_itemSaleRoot = root.gameObject;
    g_itemSaleCanvas = canvas;
    CreateSceneTextElement(
        "ItemSaleTitle",
        root.rect,
        "出售物品",
        0.055f,
        0.720f,
        0.945f,
        0.925f,
        24.0f,
        514,
        0.96f,
        0.82f,
        0.40f,
        1.0f);
    g_itemSaleMessage = CreateSceneTextElement(
        "ItemSaleMessage",
        root.rect,
        "",
        0.075f,
        0.300f,
        0.925f,
        0.700f,
        18.0f,
        514,
        0.94f,
        0.95f,
        0.97f,
        1.0f).text;
    g_itemSaleYesButton = CreateSceneButton(
        "ItemSaleYes",
        root.rect,
        "确认出售",
        0.115f,
        0.105f,
        0.485f,
        0.275f,
        0.32f,
        0.24f,
        0.10f,
        1.0f,
        18.0f,
        514,
        true);
    g_itemSaleNoButton = CreateSceneButton(
        "ItemSaleNo",
        root.rect,
        "取消",
        0.515f,
        0.105f,
        0.885f,
        0.275f,
        0.12f,
        0.14f,
        0.18f,
        1.0f,
        18.0f,
        514,
        true);
    SetWorldGameObjectActive(g_itemSaleRoot, false);
}

void ItemContextMenu_OpenSaleConfirmation()
{
    if (g_itemContextInGameplay ||
        !InventoryUi_IsItemSlot(g_itemContextItemIndex) ||
        g_inventoryItems[g_itemContextItemIndex].storage < 0)
    {
        return;
    }
    auto* canvas = ItemContextMenu_GetCanvas();
    const double currentValue =
        InventoryUi_ItemCurrentValue(g_itemContextItemIndex);
    if (canvas == nullptr || currentValue < 0.5)
    {
        return;
    }
    g_itemSaleAmount = std::max(
        1ll,
        static_cast<long long>(std::llround(currentValue)));
    ItemContextMenu_BuildSaleWindow(canvas);
    if (g_itemSaleRoot == nullptr || g_itemSaleMessage == nullptr)
    {
        return;
    }
    const auto& item = g_inventoryItems[g_itemContextItemIndex];
    const auto& definition =
        kEquipmentInventoryDefinitions[item.definition];
    wchar_t message[512]{};
    swprintf_s(
        message,
        L"确定出售“%s”吗？\n\n本次可获得 %s。\n"
        L"确认后物品将从仓库永久移除。",
        Utf8ToWide(definition.name).c_str(),
        InventoryUi_FormatCurrency(g_itemSaleAmount).c_str());
    const auto messageUtf8 = WideToUtf8(message);
    SetSceneText(
        g_itemSaleMessage,
        messageUtf8.c_str(),
        18.0f,
        514,
        0.94f,
        0.95f,
        0.97f,
        1.0f);
    SetWorldGameObjectActive(g_itemSaleRoot, true);
    BringSceneObjectToFront(g_itemSaleRoot);
    g_itemSaleOpen = true;
    LogInfo(
        LogCategory_Inventory,
        L"SaleOpen",
        L"Item sale confirmation opened: definition=%d itemSlot=%d "
        L"amount=%lld.",
        item.definition,
        g_itemContextItemIndex,
        g_itemSaleAmount);
}

void ItemContextMenu_SellSelectedItem()
{
    if (g_itemContextInGameplay ||
        !InventoryUi_IsItemSlot(g_itemContextItemIndex))
    {
        return;
    }
    auto& item = g_inventoryItems[g_itemContextItemIndex];
    if (item.storage < 0 || g_itemSaleAmount <= 0)
    {
        LogInfo(
            LogCategory_Inventory,
            L"Sale",
            L"Item sale skipped because the selected item is no longer "
            L"available: itemSlot=%d storage=%d amount=%lld.",
            g_itemContextItemIndex,
            item.storage,
            g_itemSaleAmount);
        return;
    }
    const int definition = item.definition;
    const int storage = item.storage;
    const int quality = item.quality;
    const long long amount = g_itemSaleAmount;
    if (item.element.gameObject != nullptr)
    {
        SetWorldGameObjectActive(item.element.gameObject, false);
    }
    item = InventoryItemState{};
    item.storage = -1;
    InventoryUi_PruneQuickSlotBindings();
    const long long balance =
        InventoryUi_AddBalance(amount, L"item sale");
    InventoryUi_RefreshEquipment();
    InventoryUi_SaveState();
    InventoryUi_SetStatus("物品已出售");
    LogInfo(
        LogCategory_Inventory,
        L"Sale",
        L"Item sold: definition=%d slot=%d storage=%d quality=%d "
        L"amount=%lld balance=%lld.",
        definition,
        g_itemContextItemIndex,
        storage,
        quality,
        amount,
        balance);
}

void ItemContextMenu_UpdateMenuHover(float cursorX, float cursorY)
{
    for (int index = 0;
         index < kItemContextMenuActionCount;
         ++index)
    {
        auto& row = g_itemContextMenuRows[index];
        if (row.image == nullptr)
        {
            continue;
        }
        const bool hovered =
            ItemContextMenu_PointIn(
                cursorX,
                cursorY,
                g_itemContextMinX,
                g_itemContextMenuRowMinY[index],
                g_itemContextMaxX,
                g_itemContextMenuRowMaxY[index]);
        const bool actionable =
            index == ItemContextMenu_View ||
            index == ItemContextMenu_Discard ||
            (index == ItemContextMenu_Appraise &&
             g_itemContextCanAppraise) ||
            (index == ItemContextMenu_Exhibit &&
             g_itemContextCanExhibit);
        const bool discard = index == ItemContextMenu_Discard;
        SetSceneGraphicColor(
            row.image,
            discard
                ? (hovered ? 0.420f : 0.250f)
                : (hovered && actionable ? 0.075f :
                    (actionable ? 0.050f : 0.030f)),
            discard
                ? (hovered ? 0.105f : 0.065f)
                : (hovered && actionable ? 0.250f :
                    (actionable ? 0.110f : 0.035f)),
            discard
                ? (hovered ? 0.075f : 0.050f)
                : (hovered && actionable ? 0.390f :
                    (actionable ? 0.160f : 0.045f)),
            0.99f);
    }
}

const wchar_t* ItemContextMenu_StatLabel(int index)
{
    static const wchar_t* labels[kItemInspectStatCount] = {
        L"感知力",
        L"保守度",
        L"正常度",
        L"忍耐力",
        L"速度",
        L"高潮速度",
        L"体力上限",
        L"体力恢复",
        L"手铐熟练度",
        L"奖励值",
    };
    if (index < 0 || index >= kItemInspectStatCount)
    {
        return L"属性";
    }
    return labels[index];
}

const wchar_t* ItemContextMenu_StatDescription(int index)
{
    static const wchar_t* descriptions[kItemInspectStatCount] = {
        L"眼罩状态下的可视范围。",
        L"NPC 正向怀疑值的增长效率，数值越高越不容易被发现。",
        L"怀疑值开始下降前的等待时间和下降速度，数值越高下降越快。",
        L"爬行和振动器强档状态下的移动速度。",
        L"行走和奔跑状态下的移动速度。",
        L"普通高潮值的累计速度与自然衰减速度，数值越高累计越快、衰减越慢。",
        L"最大体力，以及体力开始恢复前的等待时间。",
        L"体力消耗倍率和体力恢复速度。",
        L"普通手铐的解锁效率与解锁动画时间。",
        L"暴露状态下的金钱收益倍率。",
    };
    if (index < 0 || index >= kItemInspectStatCount)
    {
        return L"该属性会影响对应的游戏机制。";
    }
    return descriptions[index];
}

void ItemContextMenu_HideInspectStatTooltip()
{
    if (g_itemInspectHoveredStat >= 0)
    {
        LogDebug(
            LogCategory_Inventory,
            L"InspectStatTooltip",
            L"Inspect stat tooltip hidden: definition=%d stat=%d.",
            g_itemInspectDefinition,
            g_itemInspectHoveredStat);
    }
    g_itemInspectHoveredStat = -1;
    if (g_itemInspectStatTooltip.gameObject != nullptr)
    {
        SetWorldGameObjectActive(
            g_itemInspectStatTooltip.gameObject,
            false);
    }
    if (g_itemInspectStatTooltipText.gameObject != nullptr)
    {
        SetWorldGameObjectActive(
            g_itemInspectStatTooltipText.gameObject,
            false);
    }
}

void ItemContextMenu_ResetInspectStatLayout(const wchar_t* message)
{
    ItemContextMenu_HideInspectStatTooltip();
    g_itemInspectVisibleStatCount = 0;
    for (int index = 0; index < kItemInspectStatCount; ++index)
    {
        g_itemInspectVisibleStats[index] = -1;
        g_itemInspectStatMinX[index] = 0.0f;
        g_itemInspectStatMaxX[index] = 0.0f;
        if (g_itemInspectStatElements[index].gameObject != nullptr)
        {
            SetWorldGameObjectActive(
                g_itemInspectStatElements[index].gameObject,
                false);
        }
    }
    if (g_itemInspectEquipmentStats != nullptr)
    {
        const auto text = WideToUtf8(
            message != nullptr ? message : L"无属性加成");
        SetSceneText(
            g_itemInspectEquipmentStats,
            text.c_str(),
            13.0f,
            514,
            0.86f,
            0.92f,
            1.0f,
            1.0f);
        SetWorldGameObjectActive(
            g_itemInspectEquipmentStats,
            true);
    }
}

void ItemContextMenu_LayoutInspectStats(const int values[kItemInspectStatCount])
{
    ItemContextMenu_HideInspectStatTooltip();
    g_itemInspectVisibleStatCount = 0;
    double totalUnits = 0.0;
    for (int index = 0; index < kItemInspectStatCount; ++index)
    {
        if (values[index] == 0)
        {
            g_itemInspectVisibleStats[index] = -1;
            continue;
        }
        const int digitCount =
            std::abs(values[index]) >= 10 ? 2 : 1;
        totalUnits +=
            static_cast<double>(
                wcslen(ItemContextMenu_StatLabel(index))) +
            3.0 +
            static_cast<double>(digitCount);
        g_itemInspectVisibleStats[
            g_itemInspectVisibleStatCount] = index;
        ++g_itemInspectVisibleStatCount;
    }
    if (g_itemInspectVisibleStatCount == 0 || totalUnits <= 0.0)
    {
        ItemContextMenu_ResetInspectStatLayout(L"无属性加成");
        return;
    }

    if (g_itemInspectEquipmentStats != nullptr)
    {
        SetWorldGameObjectActive(
            g_itemInspectEquipmentStats,
            false);
    }
    constexpr double rowMinX = 0.055;
    constexpr double rowMaxX = 0.945;
    constexpr double columnGap = 0.012;
    const double availableWidth =
        rowMaxX -
        rowMinX -
        columnGap *
            static_cast<double>(
                g_itemInspectVisibleStatCount - 1);
    const float fontSize =
        g_itemInspectVisibleStatCount >= 7 ? 11.0f :
        (g_itemInspectVisibleStatCount >= 5 ? 12.0f : 13.0f);
    double cursorX = rowMinX;
    for (int visibleIndex = 0;
         visibleIndex < g_itemInspectVisibleStatCount;
         ++visibleIndex)
    {
        const int statIndex =
            g_itemInspectVisibleStats[visibleIndex];
        const int value = values[statIndex];
        const int digitCount = std::abs(value) >= 10 ? 2 : 1;
        const double units =
            static_cast<double>(
                wcslen(ItemContextMenu_StatLabel(statIndex))) +
            3.0 +
            static_cast<double>(digitCount);
        const double width =
            availableWidth * units / totalUnits;
        const double maxX = cursorX + width;
        auto& element = g_itemInspectStatElements[visibleIndex];
        SetSceneRect(
            element.rect,
            static_cast<float>(cursorX),
            0.315f,
            static_cast<float>(maxX),
            0.355f);
        g_itemInspectStatMinX[visibleIndex] =
            static_cast<float>(cursorX);
        g_itemInspectStatMaxX[visibleIndex] =
            static_cast<float>(maxX);
        wchar_t valueText[32]{};
        swprintf_s(
            valueText,
            value > 0
                ? L"<color=#66B3FF>%+d</color>"
                : L"<color=#FF6666>%+d</color>",
            value);
        std::wstring line = ItemContextMenu_StatLabel(statIndex);
        line += L"：";
        line += valueText;
        const auto lineUtf8 = WideToUtf8(line);
        SetSceneText(
            element.text,
            lineUtf8.c_str(),
            fontSize,
            513,
            0.86f,
            0.92f,
            1.0f,
            1.0f);
        SetWorldGameObjectActive(element.gameObject, true);
        cursorX =
            maxX + (visibleIndex + 1 < g_itemInspectVisibleStatCount
                ? columnGap
                : 0.0);
    }
    for (int visibleIndex = g_itemInspectVisibleStatCount;
         visibleIndex < kItemInspectStatCount;
         ++visibleIndex)
    {
        g_itemInspectVisibleStats[visibleIndex] = -1;
        g_itemInspectStatMinX[visibleIndex] = 0.0f;
        g_itemInspectStatMaxX[visibleIndex] = 0.0f;
        if (g_itemInspectStatElements[visibleIndex].gameObject != nullptr)
        {
            SetWorldGameObjectActive(
                g_itemInspectStatElements[visibleIndex].gameObject,
                false);
        }
    }
}

void ItemContextMenu_PositionInspectStatTooltip(
    float cursorX,
    float cursorY)
{
    if (g_itemInspectStatTooltip.rect == nullptr ||
        g_itemInspectWidth <= 0.0f ||
        g_itemInspectHeight <= 0.0f)
    {
        return;
    }
    const float relativeX =
        (cursorX - g_itemInspectX) / g_itemInspectWidth;
    const float relativeY =
        (cursorY - g_itemInspectY) / g_itemInspectHeight;
    float minX =
        relativeX + kItemInspectStatTooltipCursorOffset;
    if (minX + g_itemInspectStatTooltipWidth >
        1.0f - kItemInspectStatTooltipEdgePadding)
    {
        minX =
            relativeX -
            g_itemInspectStatTooltipWidth -
            kItemInspectStatTooltipCursorOffset;
    }
    minX = std::clamp(
        minX,
        kItemInspectStatTooltipEdgePadding,
        1.0f -
            kItemInspectStatTooltipEdgePadding -
            g_itemInspectStatTooltipWidth);
    float minY =
        relativeY + kItemInspectStatTooltipCursorOffset;
    if (minY + g_itemInspectStatTooltipHeight >
        1.0f - kItemInspectStatTooltipEdgePadding)
    {
        minY =
            relativeY -
            g_itemInspectStatTooltipHeight -
            kItemInspectStatTooltipCursorOffset;
    }
    minY = std::clamp(
        minY,
        kItemInspectStatTooltipEdgePadding,
        1.0f -
            kItemInspectStatTooltipEdgePadding -
            g_itemInspectStatTooltipHeight);
    SetSceneRect(
        g_itemInspectStatTooltip.rect,
        minX,
        minY,
        minX + g_itemInspectStatTooltipWidth,
        minY + g_itemInspectStatTooltipHeight);
}

void ItemContextMenu_UpdateInspectStatHover(
    float cursorX,
    float cursorY)
{
    if (!g_itemInspectOpen ||
        g_itemInspectVisibleStatCount <= 0 ||
        g_itemInspectWidth <= 0.0f ||
        g_itemInspectHeight <= 0.0f)
    {
        ItemContextMenu_HideInspectStatTooltip();
        return;
    }
    const float relativeX =
        (cursorX - g_itemInspectX) / g_itemInspectWidth;
    const float relativeY =
        (cursorY - g_itemInspectY) / g_itemInspectHeight;
    if (relativeX < 0.0f || relativeX > 1.0f ||
        relativeY < 0.300f || relativeY > 0.365f)
    {
        ItemContextMenu_HideInspectStatTooltip();
        return;
    }
    int hoveredVisual = -1;
    for (int visibleIndex = 0;
         visibleIndex < g_itemInspectVisibleStatCount;
         ++visibleIndex)
    {
        if (relativeX >= g_itemInspectStatMinX[visibleIndex] &&
            relativeX <= g_itemInspectStatMaxX[visibleIndex])
        {
            hoveredVisual = visibleIndex;
            break;
        }
    }
    if (hoveredVisual < 0)
    {
        ItemContextMenu_HideInspectStatTooltip();
        return;
    }
    const int statIndex =
        g_itemInspectVisibleStats[hoveredVisual];
    if (statIndex < 0)
    {
        ItemContextMenu_HideInspectStatTooltip();
        return;
    }
    if (g_itemInspectHoveredStat == statIndex)
    {
        ItemContextMenu_PositionInspectStatTooltip(
            cursorX,
            cursorY);
        return;
    }
    if (g_itemInspectHoveredStat >= 0)
    {
        LogDebug(
            LogCategory_Inventory,
            L"InspectStatTooltip",
            L"Inspect stat tooltip switched: definition=%d from=%d to=%d.",
            g_itemInspectDefinition,
            g_itemInspectHoveredStat,
            statIndex);
    }
    else
    {
        LogDebug(
            LogCategory_Inventory,
            L"InspectStatTooltip",
            L"Inspect stat tooltip shown: definition=%d stat=%d.",
            g_itemInspectDefinition,
            statIndex);
    }
    g_itemInspectHoveredStat = statIndex;
    if (g_itemInspectStatTooltip.gameObject != nullptr)
    {
        SetWorldGameObjectActive(
            g_itemInspectStatTooltip.gameObject,
            true);
    }
    if (g_itemInspectStatTooltipText.gameObject != nullptr)
    {
        SetWorldGameObjectActive(
            g_itemInspectStatTooltipText.gameObject,
            true);
    }
    if (g_itemInspectStatTooltipText.text != nullptr)
    {
        std::wstring tooltipText = L"<color=#8FB4D9>作用：</color>";
        tooltipText += ItemContextMenu_StatDescription(statIndex);
        const auto tooltipTextUtf8 = WideToUtf8(tooltipText);
        SetSceneText(
            g_itemInspectStatTooltipText.text,
            tooltipTextUtf8.c_str(),
            12.0f,
            513,
            0.90f,
            0.94f,
            1.0f,
            1.0f);
        int horizontalOverflow = 1;
        void* overflowParameters[] = { &horizontalOverflow };
        Invoke(
            FindMethod(
                FindClass("UnityEngine.UI", "Text"),
                "set_horizontalOverflow",
                1),
            g_itemInspectStatTooltipText.text,
            overflowParameters);
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "Canvas"),
                "ForceUpdateCanvases",
                0),
            nullptr,
            nullptr);

        auto* screenClass = FindClass("UnityEngine", "Screen");
        auto* widthValue = Invoke(
            FindMethod(screenClass, "get_width", 0),
            nullptr,
            nullptr);
        auto* heightValue = Invoke(
            FindMethod(screenClass, "get_height", 0),
            nullptr,
            nullptr);
        const float screenWidth =
            widthValue != nullptr && il2cpp_object_unbox != nullptr
                ? static_cast<float>(
                    *static_cast<int*>(
                        il2cpp_object_unbox(widthValue)))
                : 0.0f;
        const float screenHeight =
            heightValue != nullptr && il2cpp_object_unbox != nullptr
                ? static_cast<float>(
                    *static_cast<int*>(
                        il2cpp_object_unbox(heightValue)))
                : 0.0f;
        if (screenWidth > 0.0f &&
            screenHeight > 0.0f &&
            g_itemInspectWidth > 0.0f &&
            g_itemInspectHeight > 0.0f)
        {
            const float rootPixelWidth =
                screenWidth * g_itemInspectWidth;
            const float rootPixelHeight =
                screenHeight * g_itemInspectHeight;
            const float preferredWidth =
                ExtendedMode_GetTextPreferredWidth(
                    g_itemInspectStatTooltipText.text);
            const float desiredWidthPixels =
                std::max(96.0f, preferredWidth + 24.0f);
            constexpr float desiredHeightPixels = 38.0f;
            g_itemInspectStatTooltipWidth = std::clamp(
                desiredWidthPixels / rootPixelWidth,
                kItemInspectStatTooltipMinWidth,
                kItemInspectStatTooltipMaxWidth);
            g_itemInspectStatTooltipHeight = std::clamp(
                desiredHeightPixels / rootPixelHeight,
                0.034f,
                kItemInspectStatTooltipMaxHeight);
            const float horizontalInset = std::clamp(
                10.0f / rootPixelWidth,
                0.012f,
                0.060f);
            SetSceneRect(
                g_itemInspectStatTooltipText.rect,
                horizontalInset,
                0.12f,
                1.0f - horizontalInset,
                0.88f);
        }
    }
    ItemContextMenu_PositionInspectStatTooltip(
        cursorX,
        cursorY);
}

void ItemContextMenu_OpenInspect(
    int definition,
    int quality,
    int itemIndex,
    bool appraised,
    unsigned int wearUnits)
{
    auto* canvas = ItemContextMenu_GetCanvas();
    if (canvas == nullptr ||
        definition < 0 ||
        definition >= kEquipmentInventoryDefinitionCount)
    {
        return;
    }
    ItemContextMenu_BuildInspectWindow(canvas);
    if (g_itemInspectRoot == nullptr)
    {
        return;
    }
    const auto& itemDefinition =
        kEquipmentInventoryDefinitions[definition];
    g_itemInspectDefinition = definition;
    const std::wstring name = Utf8ToWide(itemDefinition.name);
    const std::wstring category =
        ItemContextMenu_CategoryText(definition);
    const auto qualityText = Utf8ToWide(
        kInventoryQualityLabels[
            std::clamp(quality, 0, kInventoryQualityCount - 1)]);
    SetSceneText(
        g_itemInspectName,
        itemDefinition.name,
        22.0f,
        513,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    wchar_t stats[768]{};
    double value = 0.0;
    if (InventoryUi_IsItemSlot(itemIndex))
    {
        const auto& item = g_inventoryItems[itemIndex];
        appraised = item.appraised;
        wearUnits = item.wearUnits;
        value = InventoryUi_ItemCurrentValue(itemIndex);
    }
    else
    {
        value = InventoryUi_InstanceCurrentValue(
            definition,
            quality,
            appraised,
            wearUnits);
    }
    const long long qualityGrade =
        InventoryUi_QualityGrade(quality);
    const double wearFactor =
        InventoryUi_WearFactor(
            quality,
            appraised,
            wearUnits);
    std::wstring wearText;
    if (appraised)
    {
        wchar_t wearLine[128]{};
        swprintf_s(
            wearLine,
            L"\n磨损度：%s    磨损系数：%.3f",
            InventoryUi_FormatWear(wearUnits).c_str(),
            wearFactor);
        wearText = wearLine;
    }
    swprintf_s(
        stats,
        L"类别：%s    尺寸：%d × %d 格\n"
        L"品质：%s × %lld    参考价：￥%d\n"
        L"装备价值：￥%.2f%s",
        category.c_str(),
        itemDefinition.width,
        itemDefinition.height,
        qualityText.c_str(),
        qualityGrade,
        itemDefinition.referencePrice,
        value,
        wearText.c_str());
    const auto statsUtf8 = WideToUtf8(stats);
    SetSceneText(
        g_itemInspectStats,
        statsUtf8.c_str(),
        14.0f,
        514,
        0.78f,
        0.86f,
        0.96f,
        1.0f);
    const bool isProp =
        (itemDefinition.categories &
         kInventoryPropCategoryBit) != 0;
    if (isProp)
    {
        ItemContextMenu_ResetInspectStatLayout(
            L"道具类装备不提供属性加成");
    }
    else
    {
        const auto scaledStat =
            [qualityGrade](int base) -> int
            {
                return std::clamp(
                    static_cast<int>(
                        static_cast<long long>(base) * qualityGrade),
                    -25,
                    25);
            };
        const int values[] = {
            scaledStat(itemDefinition.perceptionBase),
            scaledStat(itemDefinition.conservatismBase),
            scaledStat(itemDefinition.normalityBase),
            scaledStat(itemDefinition.strengthBase),
            scaledStat(itemDefinition.speedBase),
            scaledStat(itemDefinition.climaxSpeedBase),
            scaledStat(itemDefinition.staminaCapacityBase),
            scaledStat(itemDefinition.staminaRecoveryBase),
            scaledStat(itemDefinition.handcuffProficiencyBase),
            scaledStat(itemDefinition.rewardBase),
        };
        static_assert(
            std::size(values) ==
                static_cast<size_t>(kItemInspectStatCount));
        ItemContextMenu_LayoutInspectStats(values);
        LogDebug(
            LogCategory_Inventory,
            L"InspectStatLayout",
            L"Inspect stat row laid out: definition=%d visible=%d.",
            definition,
            g_itemInspectVisibleStatCount);
    }
    ItemContextMenu_HideInspectStatTooltip();
    wchar_t description[512]{};
    swprintf_s(
        description,
        L"%s\n\n该物品属于“%s”类别，可在装备页面中搭配，或收纳于背包、物资区与整理区。",
        name.c_str(),
        category.c_str());
    const auto descriptionUtf8 = WideToUtf8(description);
    SetSceneText(
        g_itemInspectDescription,
        descriptionUtf8.c_str(),
        15.0f,
        513,
        0.82f,
        0.85f,
        0.89f,
        1.0f);
    SetWorldGameObjectActive(
        g_itemInspectBadgeCircle.gameObject,
        appraised);
    SetWorldGameObjectActive(
        g_itemInspectBadgeText.gameObject,
        appraised);
    if (appraised)
    {
        const auto tier = InventoryUi_GetAppraisalTier(wearUnits);
        float badgeRed = 0.0f;
        float badgeGreen = 0.0f;
        float badgeBlue = 0.0f;
        InventoryUi_AppraisalTierColor(
            tier,
            badgeRed,
            badgeGreen,
            badgeBlue);
        SetSceneText(
            g_itemInspectBadgeCircle.text,
            "●",
            30.0f,
            514,
            badgeRed,
            badgeGreen,
            badgeBlue,
            1.0f);
        SetSceneText(
            g_itemInspectBadgeText.text,
            WideToUtf8(
                InventoryUi_AppraisalTierLabel(tier)).c_str(),
            15.0f,
            514,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
    }
    if (g_itemInspectBadgeCircle.rect != nullptr)
    {
        float clientWidth = g_inventoryLayoutWidth;
        float clientHeight = g_inventoryLayoutHeight;
        InventoryUi_ReadClientSize(clientWidth, clientHeight);
        const float previewPixelsWidth =
            clientWidth * g_itemInspectWidth * 0.91f;
        const float previewPixelsHeight =
            clientHeight * g_itemInspectHeight * 0.45f;
        constexpr float diameterPixels = 30.0f;
        constexpr float insetPixels = 2.0f;
        const float badgeWidth =
            diameterPixels / previewPixelsWidth;
        const float badgeHeight =
            diameterPixels / previewPixelsHeight;
        const float insetX =
            insetPixels / previewPixelsWidth;
        const float insetY =
            insetPixels / previewPixelsHeight;
        SetSceneRect(
            g_itemInspectBadgeCircle.rect,
            insetX,
            1.0f - insetY - badgeHeight,
            insetX + badgeWidth,
            1.0f - insetY);
    }
    const auto texture = LoadScenePngTexture(
        InventoryUi_EquipmentImagePath(
            itemDefinition.id,
            quality));
    g_itemInspectImage.texture = texture;
    SetSceneRawImageTexture(g_itemInspectImage.rawImage, texture);
    auto* aspectFitter = GetSceneComponentByClass(
        g_itemInspectImage.gameObject,
        "UnityEngine.UI",
        "AspectRatioFitter");
    if (aspectFitter != nullptr && texture != nullptr)
    {
        auto* textureClass = FindClass("UnityEngine", "Texture");
        auto* textureWidthValue = Invoke(
            FindMethod(textureClass, "get_width", 0),
            texture,
            nullptr);
        auto* textureHeightValue = Invoke(
            FindMethod(textureClass, "get_height", 0),
            texture,
            nullptr);
        if (textureWidthValue != nullptr &&
            textureHeightValue != nullptr)
        {
            const int textureWidth =
                *static_cast<int*>(
                    il2cpp_object_unbox(textureWidthValue));
            const int textureHeight =
                *static_cast<int*>(
                    il2cpp_object_unbox(textureHeightValue));
            if (textureWidth > 0 && textureHeight > 0)
            {
                float aspectRatio =
                    static_cast<float>(textureWidth) /
                    static_cast<float>(textureHeight);
                int aspectMode = 3; // AspectMode.FitInParent
                void* aspectParameters[] = { &aspectRatio };
                void* modeParameters[] = { &aspectMode };
                Invoke(
                    FindMethod(
                        FindClass(
                            "UnityEngine.UI",
                            "AspectRatioFitter"),
                        "set_aspectRatio",
                        1),
                    aspectFitter,
                    aspectParameters);
                Invoke(
                    FindMethod(
                        FindClass(
                            "UnityEngine.UI",
                            "AspectRatioFitter"),
                        "set_aspectMode",
                        1),
                    aspectFitter,
                    modeParameters);
            }
        }
    }
    SetSceneRect(
        GetSceneComponentByClass(
            g_itemInspectRoot,
            "UnityEngine",
            "RectTransform"),
        g_itemInspectX,
        g_itemInspectY,
        g_itemInspectX + g_itemInspectWidth,
        g_itemInspectY + g_itemInspectHeight);
    SetWorldGameObjectActive(g_itemInspectRoot, true);
    BringSceneObjectToFront(g_itemInspectRoot);
    g_itemInspectOpen = true;
    g_itemInspectDragging = false;
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Item inspect opened: definition=%d quality=%d itemSlot=%d "
        L"referencePrice=%d wearFactor=%.3f value=%.2f.",
        definition,
        quality,
        itemIndex,
        itemDefinition.referencePrice,
        wearFactor,
        value);
}

void ItemContextMenu_HandleRightClick(
    float cursorX,
    float cursorY)
{
    if (!g_gameplayInventoryOpen.load() &&
        g_extendedModeState.load() == ExtendedMode_Active)
    {
        const int quickSlot = InventoryUi_QuickSlotAt(
            cursorX,
            cursorY);
        if (quickSlot >= 0)
        {
            InventoryUi_UnbindQuickSlot(quickSlot);
            ItemContextMenu_CloseMenu();
            return;
        }
    }
    int quality = 0;
    int itemIndex = -1;
    int gameplayTempIndex = -1;
    const int definition =
        ItemContextMenu_FindItemAt(
            cursorX,
            cursorY,
            quality,
            itemIndex,
            gameplayTempIndex);
    if (definition < 0)
    {
        ItemContextMenu_CloseMenu();
        return;
    }
    auto* canvas = ItemContextMenu_GetCanvas();
    if (canvas == nullptr)
    {
        return;
    }
    g_itemContextDefinition = definition;
    g_itemContextQuality = quality;
    g_itemContextItemIndex = itemIndex;
    g_itemContextGameplayTempIndex = gameplayTempIndex;
    g_itemContextInGameplay = g_gameplayInventoryOpen.load();
    g_itemContextCanAppraise =
        !g_itemContextInGameplay &&
        InventoryUi_CanAppraiseItem(itemIndex);
    g_itemContextCanExhibit =
        !g_itemContextInGameplay &&
        ExhibitionUi_CanDepositItem(itemIndex);
    g_itemContextCanSell =
        !g_itemContextInGameplay &&
        InventoryUi_IsItemSlot(itemIndex) &&
        g_inventoryItems[itemIndex].storage >= 0;
    ItemContextMenu_BuildMenu(canvas, cursorX, cursorY);
    g_itemContextConsumeUntilRelease = true;
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Item context menu opened: definition=%d quality=%d gameplay=%d "
        L"itemSlot=%d appraisable=%d exhibitable=%d sellable=%d.",
        definition,
        quality,
        g_itemContextInGameplay ? 1 : 0,
        itemIndex,
        g_itemContextCanAppraise ? 1 : 0,
        g_itemContextCanExhibit ? 1 : 0,
        g_itemContextCanSell ? 1 : 0);
}

void ItemContextMenu_Tick()
{
    const int state = g_extendedModeState.load();
    if ((state != ExtendedMode_Active &&
         state != ExtendedMode_Gameplay) ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        if (g_extendedOriginalSystemMenuVisible.load())
        {
            ItemContextMenu_CloseMenu();
        }
        return;
    }
    const HWND window = FindGameWindow();
    RECT clientRect{};
    if (window == nullptr || !GetClientRect(window, &clientRect))
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
    ItemContextMenu_UpdateAppraisalAnimation();
    POINT cursor{};
    if (GetCursorPos(&cursor))
    {
        // Use the current desktop position for menu and window interaction.
        // The manual pointer cache is intentionally not used here because the
        // warehouse path does not refresh it while merely moving the mouse.
    }
    else if (g_extendedManualPointerPositionValid.load())
    {
        cursor.x = g_extendedManualPointerCurrentX.load();
        cursor.y = g_extendedManualPointerCurrentY.load();
    }
    else
    {
        return;
    }
    if (!ScreenToClient(window, &cursor))
    {
        return;
    }
    const float cursorX = static_cast<float>(cursor.x) / width;
    const float cursorY =
        1.0f - static_cast<float>(cursor.y) / height;

    const unsigned int rightSerial =
        g_extendedManualRightPressSerial.load();
    if (rightSerial != g_itemContextObservedRightSerial)
    {
        g_itemContextObservedRightSerial = rightSerial;
        POINT pressPoint{
            g_extendedManualPointerPressX.load(),
            g_extendedManualPointerPressY.load()};
        ScreenToClient(window, &pressPoint);
        if (!InventoryUi_RequestDraggedItemRotation() &&
            !GameplayInventory_RequestDraggedItemRotation())
        {
            ItemContextMenu_HandleRightClick(
                static_cast<float>(pressPoint.x) / width,
                1.0f - static_cast<float>(pressPoint.y) / height);
        }
    }

    const bool leftDown = g_extendedManualPointerDown.load();
    const unsigned int leftSerial =
        g_extendedManualPointerPressSerial.load();
    const bool leftPressed =
        leftSerial != g_itemContextObservedLeftSerial;
    if (leftPressed)
    {
        g_itemContextObservedLeftSerial = leftSerial;
    }
    bool openedDiscardThisTick = false;
    bool openedSaleThisTick = false;

    if (g_itemContextMenuOpen)
    {
        ItemContextMenu_UpdateMenuHover(cursorX, cursorY);
        if (leftPressed)
        {
            int action = -1;
            for (int index = 0;
                 index < kItemContextMenuActionCount;
                 ++index)
            {
                if (g_itemContextMenuRowMaxY[index] <=
                    g_itemContextMenuRowMinY[index])
                {
                    continue;
                }
                if (ItemContextMenu_PointIn(
                        cursorX,
                        cursorY,
                        g_itemContextMinX,
                        g_itemContextMenuRowMinY[index],
                        g_itemContextMaxX,
                        g_itemContextMenuRowMaxY[index]))
                {
                    action = index;
                    break;
                }
            }
            if (action == ItemContextMenu_View)
            {
                bool appraised = false;
                unsigned int wearUnits = 0;
                if (g_itemContextItemIndex >= 0)
                {
                    const auto& item = g_inventoryItems[
                        g_itemContextItemIndex];
                    appraised = item.appraised;
                    wearUnits = item.wearUnits;
                }
                else if (g_itemContextGameplayTempIndex >= 0 &&
                         g_itemContextGameplayTempIndex <
                             static_cast<int>(
                                 g_gameplayInventoryTempItems.size()))
                {
                    const auto& temp =
                        g_gameplayInventoryTempItems[
                            static_cast<size_t>(
                                g_itemContextGameplayTempIndex)];
                    appraised = temp.appraised;
                    wearUnits = temp.wearUnits;
                }
                ItemContextMenu_OpenInspect(
                    g_itemContextDefinition,
                    g_itemContextQuality,
                    g_itemContextItemIndex,
                    appraised,
                    wearUnits);
            }
            else if (action == ItemContextMenu_Appraise &&
                     g_itemContextCanAppraise)
            {
                ItemContextMenu_OpenAppraisal(
                    g_itemContextItemIndex);
            }
            else if (action == ItemContextMenu_Exhibit &&
                     g_itemContextCanExhibit)
            {
                ExhibitionUi_DepositItem(
                    g_itemContextItemIndex);
            }
            else if (action == ItemContextMenu_Sell &&
                     g_itemContextCanSell)
            {
                ItemContextMenu_OpenSaleConfirmation();
                openedSaleThisTick = true;
            }
            else if (action == ItemContextMenu_Discard)
            {
                ItemContextMenu_OpenDiscardConfirmation();
                openedDiscardThisTick = true;
            }
            ItemContextMenu_CloseMenu();
            g_itemContextConsumeUntilRelease = true;
        }
    }

    if (g_itemSaleOpen &&
        !openedSaleThisTick &&
        leftPressed)
    {
        constexpr float rootMinX = 0.250f;
        constexpr float rootMinY = 0.345f;
        constexpr float rootWidth = 0.500f;
        constexpr float rootHeight = 0.310f;
        const float rootMaxX = rootMinX + rootWidth;
        const float rootMaxY = rootMinY + rootHeight;
        const float yesMinX =
            rootMinX + rootWidth * 0.115f;
        const float yesMaxX =
            rootMinX + rootWidth * 0.485f;
        const float noMinX =
            rootMinX + rootWidth * 0.515f;
        const float noMaxX =
            rootMinX + rootWidth * 0.885f;
        const float buttonMinY =
            rootMinY + rootHeight * 0.105f;
        const float buttonMaxY =
            rootMinY + rootHeight * 0.275f;
        const bool confirmed = ItemContextMenu_PointIn(
            cursorX,
            cursorY,
            yesMinX,
            buttonMinY,
            yesMaxX,
            buttonMaxY);
        const bool canceled = ItemContextMenu_PointIn(
            cursorX,
            cursorY,
            noMinX,
            buttonMinY,
            noMaxX,
            buttonMaxY);
        if (confirmed)
        {
            ItemContextMenu_SellSelectedItem();
        }
        else if (canceled ||
                 !ItemContextMenu_PointIn(
                     cursorX,
                     cursorY,
                     rootMinX,
                     rootMinY,
                     rootMaxX,
                     rootMaxY))
        {
            LogInfo(
                LogCategory_Inventory,
                L"SaleCancel",
                L"Item sale confirmation canceled; no inventory or balance "
                L"change was made.");
        }
        ItemContextMenu_CloseSale();
        g_itemContextConsumeUntilRelease = true;
    }

    if (g_itemDiscardOpen &&
        !openedDiscardThisTick &&
        leftPressed)
    {
        constexpr float rootMinX = 0.250f;
        constexpr float rootMinY = 0.345f;
        constexpr float rootWidth = 0.500f;
        constexpr float rootHeight = 0.310f;
        const float rootMaxX = rootMinX + rootWidth;
        const float rootMaxY = rootMinY + rootHeight;
        const float yesMinX =
            rootMinX + rootWidth * 0.170f;
        const float yesMaxX =
            rootMinX + rootWidth * 0.455f;
        const float noMinX =
            rootMinX + rootWidth * 0.545f;
        const float noMaxX =
            rootMinX + rootWidth * 0.830f;
        const float buttonMinY =
            rootMinY + rootHeight * 0.105f;
        const float buttonMaxY =
            rootMinY + rootHeight * 0.275f;
        const bool confirmed = ItemContextMenu_PointIn(
            cursorX,
            cursorY,
            yesMinX,
            buttonMinY,
            yesMaxX,
            buttonMaxY);
        const bool canceled = ItemContextMenu_PointIn(
            cursorX,
            cursorY,
            noMinX,
            buttonMinY,
            noMaxX,
            buttonMaxY);
        if (confirmed)
        {
            ItemContextMenu_DiscardSelectedItem();
        }
        else if (canceled ||
                 !ItemContextMenu_PointIn(
                     cursorX,
                     cursorY,
                     rootMinX,
                     rootMinY,
                     rootMaxX,
                     rootMaxY))
        {
            // Cancellation performs no inventory mutation.
        }
        ItemContextMenu_CloseDiscard();
        g_itemContextConsumeUntilRelease = true;
    }

    if (g_itemInspectOpen)
    {
        const float rootMinX = g_itemInspectX;
        const float rootMaxX = g_itemInspectX + g_itemInspectWidth;
        const float rootMinY = g_itemInspectY;
        const float rootMaxY = g_itemInspectY + g_itemInspectHeight;
        const float titleMinY =
            rootMinY + g_itemInspectHeight * 0.895f;
        const float closeMinX =
            rootMinX + g_itemInspectWidth * 0.935f;
        const float closeMaxX =
            rootMinX + g_itemInspectWidth * 0.985f;
        ItemContextMenu_UpdateInspectStatHover(
            cursorX,
            cursorY);
        if (leftPressed)
        {
            if (ItemContextMenu_PointIn(
                    cursorX,
                    cursorY,
                    closeMinX,
                    titleMinY,
                    closeMaxX,
                    rootMaxY))
            {
                ItemContextMenu_CloseInspect();
            }
            else if (ItemContextMenu_PointIn(
                         cursorX,
                         cursorY,
                         rootMinX,
                         titleMinY,
                         rootMaxX,
                         rootMaxY))
            {
                g_itemInspectDragging = true;
                g_itemInspectDragOffsetX = cursorX - g_itemInspectX;
                g_itemInspectDragOffsetY = cursorY - g_itemInspectY;
            }
            g_itemContextConsumeUntilRelease = true;
        }
        if (g_itemInspectDragging && leftDown)
        {
            g_itemInspectX = std::clamp(
                cursorX - g_itemInspectDragOffsetX,
                0.008f,
                0.992f - g_itemInspectWidth);
            g_itemInspectY = std::clamp(
                cursorY - g_itemInspectDragOffsetY,
                0.008f,
                0.992f - g_itemInspectHeight);
            SetSceneRect(
                GetSceneComponentByClass(
                    g_itemInspectRoot,
                    "UnityEngine",
                    "RectTransform"),
                g_itemInspectX,
                g_itemInspectY,
                g_itemInspectX + g_itemInspectWidth,
                g_itemInspectY + g_itemInspectHeight);
            BringSceneObjectToFront(g_itemInspectRoot);
        }
        if (!leftDown)
        {
            g_itemInspectDragging = false;
        }
    }

    if (g_itemAppraisalOpen)
    {
        const float rootMinX = g_itemAppraisalX;
        const float rootMaxX =
            g_itemAppraisalX + g_itemAppraisalWidth;
        const float rootMinY = g_itemAppraisalY;
        const float rootMaxY =
            g_itemAppraisalY + g_itemAppraisalHeight;
        const float titleMinY =
            rootMinY + g_itemAppraisalHeight * 0.880f;
        const float closeMinX =
            rootMinX + g_itemAppraisalWidth * 0.935f;
        const float closeMaxX =
            rootMinX + g_itemAppraisalWidth * 0.985f;
        const float buttonMinX =
            rootMinX + g_itemAppraisalWidth * 0.120f;
        const float buttonMaxX =
            rootMinX + g_itemAppraisalWidth * 0.880f;
        const float buttonMinY =
            rootMinY + g_itemAppraisalHeight * 0.055f;
        const float buttonMaxY =
            rootMinY + g_itemAppraisalHeight * 0.215f;
        if (leftPressed)
        {
            if (ItemContextMenu_PointIn(
                    cursorX,
                    cursorY,
                    closeMinX,
                    titleMinY,
                    closeMaxX,
                    rootMaxY))
            {
                ItemContextMenu_CloseAppraisal();
            }
            else if (!g_itemAppraisalRolling &&
                     ItemContextMenu_PointIn(
                         cursorX,
                         cursorY,
                         buttonMinX,
                         buttonMinY,
                         buttonMaxX,
                         buttonMaxY))
            {
                ItemContextMenu_StartAppraisal();
            }
            else if (ItemContextMenu_PointIn(
                         cursorX,
                         cursorY,
                         rootMinX,
                         titleMinY,
                         rootMaxX,
                         rootMaxY))
            {
                g_itemAppraisalDragging = true;
                g_itemAppraisalDragOffsetX =
                    cursorX - g_itemAppraisalX;
                g_itemAppraisalDragOffsetY =
                    cursorY - g_itemAppraisalY;
            }
            g_itemContextConsumeUntilRelease = true;
        }
        if (g_itemAppraisalDragging && leftDown)
        {
            g_itemAppraisalX = std::clamp(
                cursorX - g_itemAppraisalDragOffsetX,
                0.008f,
                0.992f - g_itemAppraisalWidth);
            g_itemAppraisalY = std::clamp(
                cursorY - g_itemAppraisalDragOffsetY,
                0.008f,
                0.992f - g_itemAppraisalHeight);
            SetSceneRect(
                GetSceneComponentByClass(
                    g_itemAppraisalRoot,
                    "UnityEngine",
                    "RectTransform"),
                g_itemAppraisalX,
                g_itemAppraisalY,
                g_itemAppraisalX + g_itemAppraisalWidth,
                g_itemAppraisalY + g_itemAppraisalHeight);
            BringSceneObjectToFront(g_itemAppraisalRoot);
        }
        if (!leftDown)
        {
            g_itemAppraisalDragging = false;
        }
    }
    if (!leftDown && !g_itemContextMenuOpen)
    {
        g_itemContextConsumeUntilRelease = false;
    }
}
