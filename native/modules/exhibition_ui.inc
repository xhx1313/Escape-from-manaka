// Exhibition cabinet: catalog browsing, red-item display and inventory swaps.

constexpr int kExhibitionCategoryCount = 12;
constexpr int kExhibitionSortCount = 7;
constexpr int kExhibitionSeriesOptionCount = kEquipmentSeriesCount + 1;
constexpr int kExhibitionLevelCount = 10;
constexpr int kExhibitionMaxVisibleCardCount = 80;
constexpr int kExhibitionPickerRowCount = 8;
constexpr float kExhibitionCardPixelWidth = 150.0f;
constexpr float kExhibitionCardPixelHeight = 154.0f;
constexpr float kExhibitionCardPixelGap = 8.0f;
constexpr float kExhibitionContentMaxPixelWidth = 1760.0f;
constexpr float kExhibitionPageMinX = 0.024f;
constexpr float kExhibitionPageMinY = 0.055f;
constexpr float kExhibitionPageMaxX = 0.975f;
constexpr float kExhibitionPageMaxY = 0.884f;

struct ExhibitionEntry
{
    bool occupied = false;
    int quality = 4;
    unsigned int wearUnits = 0;
    unsigned long long addedAt = 0;
};

struct ExhibitionCategoryOption
{
    const char* label;
    uint32_t categoryBit;
};

struct ExhibitionCardUi
{
    SceneUiElement panel{};
    SceneUiElement imageFrame{};
    SceneUiElement image{};
    SceneUiElement mask{};
    SceneUiElement name{};
    SceneUiElement value{};
    SceneUiElement time{};
    int definition = -1;
    float screenMinX = 0.0f;
    float screenMinY = 0.0f;
    float screenMaxX = 0.0f;
    float screenMaxY = 0.0f;
};

struct ExhibitionPickerRowUi
{
    SceneUiElement panel{};
    SceneUiElement imageFrame{};
    SceneUiElement image{};
    SceneUiElement name{};
    SceneUiElement wear{};
    SceneUiElement value{};
    SceneUiElement action{};
    float screenMinX = 0.0f;
    float screenMinY = 0.0f;
    float screenMaxX = 0.0f;
    float screenMaxY = 0.0f;
};

static constexpr ExhibitionCategoryOption kExhibitionCategories[
    kExhibitionCategoryCount] = {
        { "全部",   0u },
        { "头饰",   1u << 0 },
        { "项链",   1u << 1 },
        { "上衣",   1u << 2 },
        { "胸衣",   1u << 3 },
        { "内裤",   1u << 4 },
        { "下衣",   1u << 5 },
        { "袜子",   1u << 6 },
        { "鞋子",   1u << 7 },
        { "饰品",   1u << 8 },
        { "道具",   kInventoryPropCategoryBit },
        { "背包",   kInventoryBackpackCategoryBit },
    };

static constexpr const char* kExhibitionSortLabels[kExhibitionSortCount] = {
    "默认排序",
    "放入时间正序",
    "放入时间倒序",
    "价值正序",
    "价值倒序",
    "磨损度正序",
    "磨损度倒序",
};

static constexpr double kExhibitionLevelThresholds[
    kExhibitionLevelCount] = {
        0.0,
        600000.0,
        4800000.0,
        16000000.0,
        38000000.0,
        74000000.0,
        128000000.0,
        204000000.0,
        304000000.0,
        433200000.0,
};

static constexpr int kExhibitionLevelSafeCells[
    kExhibitionLevelCount] = {
        4, 5, 6, 7, 8, 9, 10, 11, 12, 12,
};

static constexpr int kExhibitionLevelSafeColumns[
    kExhibitionLevelCount] = {
        2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
};

static constexpr int kExhibitionLevelSafeRows[
    kExhibitionLevelCount] = {
        2, 3, 3, 3, 3, 3, 4, 4, 4, 4,
};

static constexpr const char* kExhibitionLevelSafeShapes[
    kExhibitionLevelCount] = {
        "2×2",
        "2×3 5/6",
        "2×3",
        "3×3 7/9",
        "3×3 8/9",
        "3×3",
        "3×4 10/12",
        "3×4 11/12",
        "3×4",
        "3×4",
};

static constexpr int kExhibitionLevelLowPermille[
    kExhibitionLevelCount] = {
        200, 220, 250, 280, 310, 350, 390, 430, 460, 500,
};

static SceneUiElement g_exhibitionPage{};
static SceneUiElement g_exhibitionHeader{};
static Il2CppObject* g_exhibitionPageLabel = nullptr;
static Il2CppObject* g_exhibitionSummaryLabel = nullptr;
static SceneUiElement g_exhibitionTitleElement{};
static SceneUiElement g_exhibitionSummaryElement{};
static SceneUiElement g_exhibitionLevelLabelElement{};
static SceneUiElement g_exhibitionLevelTrack{};
static SceneUiElement g_exhibitionLevelFill{};
static Il2CppObject* g_exhibitionLevelLabel = nullptr;
static float g_exhibitionLevelHitMinX = 0.0f;
static float g_exhibitionLevelHitMinY = 0.0f;
static float g_exhibitionLevelHitMaxX = 0.0f;
static float g_exhibitionLevelHitMaxY = 0.0f;
static float g_exhibitionLevelBarPixelWidth = 1.0f;
static SceneUiElement g_exhibitionSeriesButton{};
static SceneUiElement g_exhibitionCategoryButton{};
static SceneUiElement g_exhibitionSortButton{};
static Il2CppObject* g_exhibitionSeriesLabel = nullptr;
static Il2CppObject* g_exhibitionCategoryLabel = nullptr;
static Il2CppObject* g_exhibitionSortLabel = nullptr;
static SceneUiElement g_exhibitionSeriesLabelElement{};
static SceneUiElement g_exhibitionCategoryLabelElement{};
static SceneUiElement g_exhibitionSortLabelElement{};
static SceneUiElement g_exhibitionSeriesDropdown{};
static SceneUiElement g_exhibitionCategoryDropdown{};
static SceneUiElement g_exhibitionSortDropdown{};
static SceneUiElement
    g_exhibitionSeriesOptions[kExhibitionSeriesOptionCount]{};
static SceneUiElement
    g_exhibitionCategoryOptions[kExhibitionCategoryCount]{};
static SceneUiElement
    g_exhibitionSortOptions[kExhibitionSortCount]{};
static float g_exhibitionSeriesOptionMinX[kExhibitionSeriesOptionCount]{};
static float g_exhibitionSeriesOptionMinY[kExhibitionSeriesOptionCount]{};
static float g_exhibitionSeriesOptionMaxX[kExhibitionSeriesOptionCount]{};
static float g_exhibitionSeriesOptionMaxY[kExhibitionSeriesOptionCount]{};
static float g_exhibitionCategoryOptionMinX[kExhibitionCategoryCount]{};
static float g_exhibitionCategoryOptionMinY[kExhibitionCategoryCount]{};
static float g_exhibitionCategoryOptionMaxX[kExhibitionCategoryCount]{};
static float g_exhibitionCategoryOptionMaxY[kExhibitionCategoryCount]{};
static float g_exhibitionSortOptionMinX[kExhibitionSortCount]{};
static float g_exhibitionSortOptionMinY[kExhibitionSortCount]{};
static float g_exhibitionSortOptionMaxX[kExhibitionSortCount]{};
static float g_exhibitionSortOptionMaxY[kExhibitionSortCount]{};
static float g_exhibitionCategoryButtonMinX = 0.0f;
static float g_exhibitionCategoryButtonMinY = 0.0f;
static float g_exhibitionCategoryButtonMaxX = 0.0f;
static float g_exhibitionCategoryButtonMaxY = 0.0f;
static float g_exhibitionSortButtonMinX = 0.0f;
static float g_exhibitionSortButtonMinY = 0.0f;
static float g_exhibitionSortButtonMaxX = 0.0f;
static float g_exhibitionSortButtonMaxY = 0.0f;
static float g_exhibitionSeriesButtonMinX = 0.0f;
static float g_exhibitionSeriesButtonMinY = 0.0f;
static float g_exhibitionSeriesButtonMaxX = 0.0f;
static float g_exhibitionSeriesButtonMaxY = 0.0f;
static SceneUiElement g_exhibitionPageLabelElement{};
static Il2CppObject* g_exhibitionNoItemsLabel = nullptr;
static SceneUiElement g_exhibitionNoItemsElement{};
static SceneUiElement g_exhibitionPrevPage{};
static SceneUiElement g_exhibitionNextPage{};
static ExhibitionCardUi
    g_exhibitionCards[kExhibitionMaxVisibleCardCount]{};
static std::vector<ExhibitionEntry> g_exhibitionEntries;
static std::vector<int> g_exhibitionVisibleDefinitions;
static int g_exhibitionSeries = 0;
static int g_exhibitionCategory = 0;
static int g_exhibitionSort = 0;
static int g_exhibitionPageIndex = 0;
static int g_exhibitionPageCount = 1;
static int g_exhibitionCachedLevel = 1;
static int g_exhibitionCardsPerPage = 28;
static int g_exhibitionCardColumns = 7;
static int g_exhibitionCardRows = 4;
static float g_exhibitionClientWidth = 1920.0f;
static float g_exhibitionClientHeight = 1080.0f;
static bool g_exhibitionCategoryOpen = false;
static bool g_exhibitionSortOpen = false;
static bool g_exhibitionSeriesOpen = false;
static unsigned int g_exhibitionObservedLeftSerial = 0;
static bool g_exhibitionPageBuilt = false;

static Il2CppObject* g_exhibitionPickerRoot = nullptr;
static SceneUiElement g_exhibitionPickerClose{};
static SceneUiElement g_exhibitionPickerPrev{};
static SceneUiElement g_exhibitionPickerNext{};
static SceneUiElement g_exhibitionPickerScrollTrack{};
static SceneUiElement g_exhibitionPickerScrollThumb{};
static SceneUiElement g_exhibitionPickerUnload{};
static Il2CppObject* g_exhibitionPickerUnloadLabel = nullptr;
static SceneUiElement g_exhibitionPickerTitleElement{};
static SceneUiElement g_exhibitionPickerCurrentElement{};
static SceneUiElement g_exhibitionPickerHintElement{};
static SceneUiElement g_exhibitionPickerEmptyElement{};
static SceneUiElement g_exhibitionPickerPageElement{};
static Il2CppObject* g_exhibitionPickerTitle = nullptr;
static Il2CppObject* g_exhibitionPickerCurrent = nullptr;
static Il2CppObject* g_exhibitionPickerPageLabel = nullptr;
static Il2CppObject* g_exhibitionPickerEmpty = nullptr;
static ExhibitionPickerRowUi
    g_exhibitionPickerRows[kExhibitionPickerRowCount]{};
static std::vector<int> g_exhibitionPickerItems;
static int g_exhibitionPickerDefinition = -1;
static int g_exhibitionPickerPage = 0;
static int g_exhibitionPickerPageCount = 1;
static int g_exhibitionPickerScrollRow = 0;
static bool g_exhibitionPickerScrollDragging = false;
static float g_exhibitionPickerScrollDragOffset = 0.0f;
static float g_exhibitionPickerScrollTrackMinY = 0.0f;
static float g_exhibitionPickerScrollTrackMaxY = 0.0f;
static float g_exhibitionPickerScrollThumbMinY = 0.0f;
static float g_exhibitionPickerScrollThumbMaxY = 0.0f;
static float g_exhibitionPickerUnloadMinX = 0.0f;
static float g_exhibitionPickerUnloadMinY = 0.0f;
static float g_exhibitionPickerUnloadMaxX = 0.0f;
static float g_exhibitionPickerUnloadMaxY = 0.0f;
static std::atomic<int> g_exhibitionPickerWheelDelta{0};
static bool g_exhibitionPickerOpen = false;
static float g_exhibitionPickerContentLeft = 0.0f;
static float g_exhibitionPickerContentTop = 0.0f;
static float g_exhibitionPickerContentWidth = 0.0f;
static float g_exhibitionPickerContentHeight = 0.0f;
static Il2CppObject* g_exhibitionLevelDetailRoot = nullptr;
static SceneUiElement g_exhibitionLevelDetailClose{};
static float g_exhibitionLevelDetailCloseMinX = 0.0f;
static float g_exhibitionLevelDetailCloseMinY = 0.0f;
static float g_exhibitionLevelDetailCloseMaxX = 0.0f;
static float g_exhibitionLevelDetailCloseMaxY = 0.0f;
static float g_exhibitionLevelDetailContentMinX = 0.0f;
static float g_exhibitionLevelDetailContentMinY = 0.0f;
static float g_exhibitionLevelDetailContentMaxX = 0.0f;
static float g_exhibitionLevelDetailContentMaxY = 0.0f;
static SceneUiElement
    g_exhibitionLevelDetailSegmentBackgrounds[kExhibitionLevelCount]{};
static SceneUiElement
    g_exhibitionLevelDetailSegmentFills[kExhibitionLevelCount]{};
static SceneUiElement
    g_exhibitionLevelDetailRows[kExhibitionLevelCount]{};
static bool g_exhibitionLevelDetailOpen = false;

void ExhibitionUi_RefreshCards();
void ExhibitionUi_RefreshLevelProgress();
void ExhibitionUi_ClosePicker();
void ExhibitionUi_CloseLevelDetail();

bool ExhibitionUi_PointIn(
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

void ExhibitionUi_PageToScreen(
    float localX,
    float localY,
    float& screenX,
    float& screenY)
{
    screenX =
        kExhibitionPageMinX +
        localX * (kExhibitionPageMaxX - kExhibitionPageMinX);
    screenY =
        kExhibitionPageMinY +
        localY * (kExhibitionPageMaxY - kExhibitionPageMinY);
}

void ExhibitionUi_ReadClientSize()
{
    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    if (gameWindow != nullptr &&
        GetClientRect(gameWindow, &clientRect))
    {
        g_exhibitionClientWidth = static_cast<float>(
            std::max<int>(
                1,
                clientRect.right - clientRect.left));
        g_exhibitionClientHeight = static_cast<float>(
            std::max<int>(
                1,
                clientRect.bottom - clientRect.top));
    }
}

void ExhibitionUi_SetPagePixelRect(
    Il2CppObject* rect,
    float left,
    float top,
    float width,
    float height)
{
    const float pageWidth =
        (kExhibitionPageMaxX - kExhibitionPageMinX) *
        g_exhibitionClientWidth;
    const float pageHeight =
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        g_exhibitionClientHeight;
    SetSceneRect(
        rect,
        left / pageWidth,
        1.0f - (top + height) / pageHeight,
        (left + width) / pageWidth,
        1.0f - top / pageHeight);
}

void ExhibitionUi_PixelRectToScreen(
    float pageLeft,
    float pageTop,
    float width,
    float height,
    float& screenMinX,
    float& screenMinY,
    float& screenMaxX,
    float& screenMaxY)
{
    const float pageWidth =
        (kExhibitionPageMaxX - kExhibitionPageMinX) *
        g_exhibitionClientWidth;
    const float pageHeight =
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        g_exhibitionClientHeight;
    const float pageScreenWidth =
        kExhibitionPageMaxX - kExhibitionPageMinX;
    const float pageScreenHeight =
        kExhibitionPageMaxY - kExhibitionPageMinY;
    screenMinX =
        kExhibitionPageMinX +
        pageLeft / pageWidth * pageScreenWidth;
    screenMaxX =
        kExhibitionPageMinX +
        (pageLeft + width) / pageWidth * pageScreenWidth;
    screenMaxY =
        kExhibitionPageMaxY -
        pageTop / pageHeight * pageScreenHeight;
    screenMinY =
        kExhibitionPageMaxY -
        (pageTop + height) / pageHeight * pageScreenHeight;
}

void ExhibitionUi_SetChildPixelRect(
    Il2CppObject* rect,
    float parentWidth,
    float parentHeight,
    float left,
    float top,
    float width,
    float height)
{
    SetSceneRect(
        rect,
        left / parentWidth,
        1.0f - (top + height) / parentHeight,
        (left + width) / parentWidth,
        1.0f - top / parentHeight);
}

void ExhibitionUi_PickerChildRectToScreen(
    float left,
    float top,
    float width,
    float height,
    float& screenMinX,
    float& screenMinY,
    float& screenMaxX,
    float& screenMaxY)
{
    ExhibitionUi_PixelRectToScreen(
        g_exhibitionPickerContentLeft + left,
        g_exhibitionPickerContentTop + top,
        width,
        height,
        screenMinX,
        screenMinY,
        screenMaxX,
        screenMaxY);
}

void ExhibitionUi_LayoutDropdown(
    const SceneUiElement& button,
    SceneUiElement& dropdown,
    SceneUiElement* options,
    float* optionMinX,
    float* optionMinY,
    float* optionMaxX,
    float* optionMaxY,
    int optionCount,
    int optionColumns,
    float dropdownWidth,
    float buttonLeft,
    float buttonTop,
    float buttonWidth,
    float buttonHeight)
{
    constexpr float rowHeight = 32.0f;
    constexpr float padding = 6.0f;
    constexpr float gap = 4.0f;
    const int rows =
        (optionCount + optionColumns - 1) / optionColumns;
    const float height =
        padding * 2.0f +
         rowHeight * static_cast<float>(rows) +
        gap * static_cast<float>(std::max(0, rows - 1));
    float left = buttonLeft + buttonWidth - dropdownWidth;
    const float pageWidth =
        (kExhibitionPageMaxX - kExhibitionPageMinX) *
        g_exhibitionClientWidth;
    left = std::clamp(left, 6.0f, pageWidth - dropdownWidth - 6.0f);
    const float top = buttonTop + buttonHeight + 4.0f;
    ExhibitionUi_SetPagePixelRect(
        dropdown.rect,
        left,
        top,
        dropdownWidth,
        height);
    for (int index = 0; index < optionCount; ++index)
    {
        const int column = index % optionColumns;
        const int row = index / optionColumns;
        const float columnWidth =
            (dropdownWidth - padding * 2.0f -
             gap * static_cast<float>(optionColumns - 1)) /
            static_cast<float>(optionColumns);
        const float optionLeft =
            left + padding +
            static_cast<float>(column) * (columnWidth + gap);
        const float optionTop =
            top + padding +
            static_cast<float>(row) * (rowHeight + gap);
        ExhibitionUi_SetChildPixelRect(
            options[index].rect,
            dropdownWidth,
            height,
            padding +
                static_cast<float>(column) * (columnWidth + gap),
            padding +
                static_cast<float>(row) * (rowHeight + gap),
            columnWidth,
            rowHeight);
        ExhibitionUi_PixelRectToScreen(
            optionLeft,
            optionTop,
            columnWidth,
            rowHeight,
            optionMinX[index],
            optionMinY[index],
            optionMaxX[index],
            optionMaxY[index]);
    }
    (void)button;
}

void ExhibitionUi_ApplyPixelLayout()
{
    if (!g_exhibitionPageBuilt)
    {
        return;
    }
    ExhibitionUi_ReadClientSize();
    const float pageWidth =
        (kExhibitionPageMaxX - kExhibitionPageMinX) *
        g_exhibitionClientWidth;
    const float pageHeight =
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        g_exhibitionClientHeight;
    const float padding = 18.0f;
    const float contentWidth = std::min(
        pageWidth - padding * 2.0f,
        kExhibitionContentMaxPixelWidth);
    const float contentLeft = (pageWidth - contentWidth) * 0.5f;
    constexpr float headerTop = 10.0f;
    constexpr float headerHeight = 68.0f;
    constexpr float footerHeight = 32.0f;
    constexpr float footerBottom = 10.0f;
    constexpr float cardAreaTop =
        headerTop + headerHeight + 12.0f;
    const float cardAreaBottom =
        pageHeight - footerBottom - footerHeight - 8.0f;
    const float availableHeight =
        std::max(
            kExhibitionCardPixelHeight,
            cardAreaBottom - cardAreaTop);

    g_exhibitionCardColumns = std::max(
        1,
        static_cast<int>(std::floor(
            (contentWidth + kExhibitionCardPixelGap) /
            (kExhibitionCardPixelWidth + kExhibitionCardPixelGap))));
    g_exhibitionCardRows = std::clamp(
        static_cast<int>(std::floor(
            (availableHeight + kExhibitionCardPixelGap) /
            (kExhibitionCardPixelHeight + kExhibitionCardPixelGap))),
        1,
        6);
    g_exhibitionCardsPerPage = std::clamp(
        g_exhibitionCardColumns * g_exhibitionCardRows,
        1,
        kExhibitionMaxVisibleCardCount);

    ExhibitionUi_SetPagePixelRect(
        g_exhibitionHeader.rect,
        contentLeft,
        headerTop,
        contentWidth,
        headerHeight);
    const float sortWidth = 250.0f;
    const float categoryWidth = 210.0f;
    const float seriesWidth = 220.0f;
    const float buttonTop = headerTop + 18.0f;
    constexpr float buttonHeight = 32.0f;
    const float sortHeaderLeft =
        contentWidth - 12.0f - sortWidth;
    const float categoryHeaderLeft =
        sortHeaderLeft - 8.0f - categoryWidth;
    const float seriesHeaderLeft =
        categoryHeaderLeft - 8.0f - seriesWidth;
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionSeriesButton.rect,
        contentWidth,
        headerHeight,
        seriesHeaderLeft,
        buttonTop - headerTop,
        seriesWidth,
        buttonHeight);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionCategoryButton.rect,
        contentWidth,
        headerHeight,
        categoryHeaderLeft,
        buttonTop - headerTop,
        categoryWidth,
        buttonHeight);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionSortButton.rect,
        contentWidth,
        headerHeight,
        sortHeaderLeft,
        buttonTop - headerTop,
        sortWidth,
        buttonHeight);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionTitleElement.rect,
        contentWidth,
        headerHeight,
        12.0f,
        7.0f,
        300.0f,
        28.0f);
    const float summaryWidth = std::min(
        300.0f,
        std::max(180.0f, seriesHeaderLeft - 24.0f));
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionSummaryElement.rect,
        contentWidth,
        headerHeight,
        12.0f,
        39.0f,
        summaryWidth,
        20.0f);
    const float progressLeft = 320.0f;
    const float progressRight =
        std::max(progressLeft + 120.0f, seriesHeaderLeft - 20.0f);
    const float progressWidth = progressRight - progressLeft;
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionLevelLabelElement.rect,
        contentWidth,
        headerHeight,
        progressLeft,
        10.0f,
        progressWidth,
        20.0f);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionLevelTrack.rect,
        contentWidth,
        headerHeight,
        progressLeft,
        37.0f,
        progressWidth,
        14.0f);
    g_exhibitionLevelBarPixelWidth = progressWidth;
    ExhibitionUi_PixelRectToScreen(
        contentLeft + progressLeft,
        headerTop + 34.0f,
        progressWidth,
        20.0f,
        g_exhibitionLevelHitMinX,
        g_exhibitionLevelHitMinY,
        g_exhibitionLevelHitMaxX,
        g_exhibitionLevelHitMaxY);
    const float seriesLeft = contentLeft + seriesHeaderLeft;
    const float categoryLeft = contentLeft + categoryHeaderLeft;
    const float sortLeft = contentLeft + sortHeaderLeft;
    ExhibitionUi_PixelRectToScreen(
        seriesLeft,
        buttonTop,
        seriesWidth,
        buttonHeight,
        g_exhibitionSeriesButtonMinX,
        g_exhibitionSeriesButtonMinY,
        g_exhibitionSeriesButtonMaxX,
        g_exhibitionSeriesButtonMaxY);
    ExhibitionUi_PixelRectToScreen(
        categoryLeft,
        buttonTop,
        categoryWidth,
        buttonHeight,
        g_exhibitionCategoryButtonMinX,
        g_exhibitionCategoryButtonMinY,
        g_exhibitionCategoryButtonMaxX,
        g_exhibitionCategoryButtonMaxY);
    ExhibitionUi_PixelRectToScreen(
        sortLeft,
        buttonTop,
        sortWidth,
        buttonHeight,
        g_exhibitionSortButtonMinX,
        g_exhibitionSortButtonMinY,
        g_exhibitionSortButtonMaxX,
        g_exhibitionSortButtonMaxY);

    const float gridWidth =
        static_cast<float>(g_exhibitionCardColumns) *
            kExhibitionCardPixelWidth +
        static_cast<float>(std::max(
            0,
            g_exhibitionCardColumns - 1)) *
            kExhibitionCardPixelGap;
    const float gridLeft =
        contentLeft + (contentWidth - gridWidth) * 0.5f;
    for (int index = 0;
         index < kExhibitionMaxVisibleCardCount;
         ++index)
    {
        auto& card = g_exhibitionCards[index];
        const int column = index % g_exhibitionCardColumns;
        const int row = index / g_exhibitionCardColumns;
        const float left =
            gridLeft +
            static_cast<float>(column) *
                (kExhibitionCardPixelWidth +
                 kExhibitionCardPixelGap);
        const float top =
            cardAreaTop +
            static_cast<float>(row) *
                (kExhibitionCardPixelHeight +
                 kExhibitionCardPixelGap);
        ExhibitionUi_SetPagePixelRect(
            card.panel.rect,
            left,
            top,
            kExhibitionCardPixelWidth,
            kExhibitionCardPixelHeight);
        ExhibitionUi_PixelRectToScreen(
            left,
            top,
            kExhibitionCardPixelWidth,
            kExhibitionCardPixelHeight,
            card.screenMinX,
            card.screenMinY,
            card.screenMaxX,
            card.screenMaxY);
    }

    const float footerTop =
        pageHeight - footerBottom - footerHeight;
    ExhibitionUi_SetPagePixelRect(
        g_exhibitionPrevPage.rect,
        pageWidth * 0.5f - 88.0f,
        footerTop,
        44.0f,
        footerHeight);
    ExhibitionUi_SetPagePixelRect(
        g_exhibitionPageLabelElement.rect,
        pageWidth * 0.5f - 42.0f,
        footerTop,
        84.0f,
        footerHeight);
    ExhibitionUi_SetPagePixelRect(
        g_exhibitionNextPage.rect,
        pageWidth * 0.5f + 44.0f,
        footerTop,
        44.0f,
        footerHeight);
    ExhibitionUi_SetPagePixelRect(
        g_exhibitionNoItemsElement.rect,
        contentLeft,
        cardAreaTop,
        contentWidth,
        availableHeight);

    ExhibitionUi_LayoutDropdown(
        g_exhibitionSeriesButton,
        g_exhibitionSeriesDropdown,
        g_exhibitionSeriesOptions,
        g_exhibitionSeriesOptionMinX,
        g_exhibitionSeriesOptionMinY,
        g_exhibitionSeriesOptionMaxX,
        g_exhibitionSeriesOptionMaxY,
        kExhibitionSeriesOptionCount,
        3,
        480.0f,
        seriesLeft,
        buttonTop,
        seriesWidth,
        buttonHeight);
    ExhibitionUi_LayoutDropdown(
        g_exhibitionCategoryButton,
        g_exhibitionCategoryDropdown,
        g_exhibitionCategoryOptions,
        g_exhibitionCategoryOptionMinX,
        g_exhibitionCategoryOptionMinY,
        g_exhibitionCategoryOptionMaxX,
        g_exhibitionCategoryOptionMaxY,
        kExhibitionCategoryCount,
        2,
        380.0f,
        categoryLeft,
        buttonTop,
        categoryWidth,
        buttonHeight);
    ExhibitionUi_LayoutDropdown(
        g_exhibitionSortButton,
        g_exhibitionSortDropdown,
        g_exhibitionSortOptions,
        g_exhibitionSortOptionMinX,
        g_exhibitionSortOptionMinY,
        g_exhibitionSortOptionMaxX,
        g_exhibitionSortOptionMaxY,
        kExhibitionSortCount,
        2,
        380.0f,
        sortLeft,
        buttonTop,
        sortWidth,
        buttonHeight);
}

void ExhibitionUi_UpdateLayout()
{
    if (!g_exhibitionPageBuilt)
    {
        return;
    }
    ExhibitionUi_ApplyPixelLayout();
    ExhibitionUi_RefreshCards();
}

void ExhibitionUi_EnsureData()
{
    if (g_exhibitionEntries.size() !=
        static_cast<size_t>(kEquipmentInventoryDefinitionCount))
    {
        g_exhibitionEntries.assign(
            static_cast<size_t>(kEquipmentInventoryDefinitionCount),
            ExhibitionEntry{});
    }
}

unsigned long long ExhibitionUi_CurrentEpochSeconds()
{
    FILETIME fileTime{};
    GetSystemTimeAsFileTime(&fileTime);
    ULARGE_INTEGER value{};
    value.LowPart = fileTime.dwLowDateTime;
    value.HighPart = fileTime.dwHighDateTime;
    constexpr unsigned long long kWindowsToUnixSeconds = 11644473600ull;
    return value.QuadPart / 10000000ull - kWindowsToUnixSeconds;
}

std::wstring ExhibitionUi_FormatAddedAt(unsigned long long addedAt)
{
    if (addedAt == 0)
    {
        return L"未记录";
    }
    ULARGE_INTEGER value{};
    value.QuadPart = (addedAt + 11644473600ull) * 10000000ull;
    FILETIME utcTime{};
    utcTime.dwLowDateTime = value.LowPart;
    utcTime.dwHighDateTime = value.HighPart;
    FILETIME localTime{};
    SYSTEMTIME systemTime{};
    if (!FileTimeToLocalFileTime(&utcTime, &localTime) ||
        !FileTimeToSystemTime(&localTime, &systemTime))
    {
        return L"未记录";
    }
    wchar_t text[32]{};
    swprintf_s(
        text,
        L"%04u-%02u-%02u %02u:%02u",
        systemTime.wYear,
        systemTime.wMonth,
        systemTime.wDay,
        systemTime.wHour,
        systemTime.wMinute);
    return text;
}

std::wstring ExhibitionUi_FormatValue(double value)
{
    wchar_t text[64]{};
    if (value >= 1000000000.0)
    {
        swprintf_s(text, L"%.2fB", value / 1000000000.0);
    }
    else if (value >= 1000000.0)
    {
        swprintf_s(text, L"%.2fM", value / 1000000.0);
    }
    else if (value >= 1000.0)
    {
        swprintf_s(text, L"%.1fK", value / 1000.0);
    }
    else
    {
        swprintf_s(text, L"%.0f", value);
    }
    return text;
}

double ExhibitionUi_ValueForDefinition(int definition)
{
    ExhibitionUi_EnsureData();
    if (definition < 0 ||
        definition >= kEquipmentInventoryDefinitionCount)
    {
        return 0.0;
    }
    const auto& entry =
        g_exhibitionEntries[static_cast<size_t>(definition)];
    if (!entry.occupied)
    {
        return 0.0;
    }
    const auto& item = kEquipmentInventoryDefinitions[definition];
    return InventoryUi_InstanceCurrentValue(
        definition,
        4,
        true,
        entry.wearUnits);
}

double ExhibitionUi_CollectionValueForDefinition(int definition)
{
    return ExhibitionUi_ValueForDefinition(definition);
}

double ExhibitionUi_CollectionTotal()
{
    ExhibitionUi_EnsureData();
    double total = 0.0;
    for (int definition = 0;
         definition < kEquipmentInventoryDefinitionCount;
         ++definition)
    {
        total += ExhibitionUi_CollectionValueForDefinition(definition);
    }
    return total;
}

int ExhibitionUi_LevelForValue(double value)
{
    int level = 1;
    for (int index = 1; index < kExhibitionLevelCount; ++index)
    {
        if (value + 0.5 < kExhibitionLevelThresholds[index])
        {
            break;
        }
        level = index + 1;
    }
    return level;
}

int ExhibitionUi_GetCurrentLevel()
{
    return g_exhibitionCachedLevel;
}

int ExhibitionUi_GetSafeBoxColumns()
{
    return kExhibitionLevelSafeColumns[
        ExhibitionUi_GetCurrentLevel() - 1];
}

int ExhibitionUi_GetSafeBoxRows()
{
    return kExhibitionLevelSafeRows[
        ExhibitionUi_GetCurrentLevel() - 1];
}

int ExhibitionUi_GetSafeBoxCells()
{
    return kExhibitionLevelSafeCells[
        ExhibitionUi_GetCurrentLevel() - 1];
}

bool ExhibitionUi_IsSafeBoxCellActive(int column, int row)
{
    const int levelIndex = ExhibitionUi_GetCurrentLevel() - 1;
    const int columns = kExhibitionLevelSafeColumns[levelIndex];
    const int rows = kExhibitionLevelSafeRows[levelIndex];
    if (column < 0 || row < 0 ||
        column >= columns || row >= rows)
    {
        return false;
    }
    const int index = row * columns + column;
    return index < kExhibitionLevelSafeCells[levelIndex];
}

double ExhibitionUi_ProgressWithinLevel(
    double value,
    int level)
{
    if (level >= kExhibitionLevelCount)
    {
        return 1.0;
    }
    const double previous = kExhibitionLevelThresholds[level - 1];
    const double next = kExhibitionLevelThresholds[level];
    if (next <= previous)
    {
        return 1.0;
    }
    return std::clamp(
        (value - previous) / (next - previous),
        0.0,
        1.0);
}

bool ExhibitionUi_DefinitionMatchesCategory(
    int definition,
    int category)
{
    if (category <= 0 ||
        category >= kExhibitionCategoryCount ||
        definition < 0 ||
        definition >= kEquipmentInventoryDefinitionCount)
    {
        return category == 0;
    }
    const uint32_t bit = kExhibitionCategories[category].categoryBit;
    return (kEquipmentInventoryDefinitions[definition].categories & bit) != 0;
}

bool ExhibitionUi_DefinitionMatchesSeries(
    int definition,
    int series)
{
    if (series <= 0)
    {
        return true;
    }
    if (series > kEquipmentSeriesCount ||
        definition < 0 ||
        definition >= kEquipmentInventoryDefinitionCount)
    {
        return false;
    }
    return kEquipmentInventoryDefinitions[definition].series == series - 1;
}

bool ExhibitionUi_CanDepositItem(int itemIndex)
{
    if (!InventoryUi_IsItemSlot(itemIndex))
    {
        return false;
    }
    const auto& item = g_inventoryItems[itemIndex];
    return item.storage == kInventoryStorageStash &&
           item.definition >= 0 &&
           item.definition < kEquipmentInventoryDefinitionCount &&
           item.quality == kInventoryQualityCount - 1 &&
           item.appraised &&
           item.wearUnits != 0;
}

bool ExhibitionUi_CanSwapItem(int itemIndex)
{
    if (!InventoryUi_IsItemSlot(itemIndex))
    {
        return false;
    }
    const auto& item = g_inventoryItems[itemIndex];
    return item.storage >= 0 &&
           item.storage < kInventoryStorageEquipped &&
           item.definition >= 0 &&
           item.definition < kEquipmentInventoryDefinitionCount &&
           item.quality == kInventoryQualityCount - 1 &&
           item.appraised &&
           item.wearUnits != 0;
}

bool ExhibitionUi_SwapItemIntoSlot(int itemIndex, int expectedDefinition)
{
    if (!ExhibitionUi_CanSwapItem(itemIndex) ||
        expectedDefinition < 0 ||
        expectedDefinition >= kEquipmentInventoryDefinitionCount ||
        g_inventoryItems[itemIndex].definition != expectedDefinition)
    {
        return false;
    }
    ExhibitionUi_EnsureData();
    auto& entry =
        g_exhibitionEntries[static_cast<size_t>(expectedDefinition)];
    auto& item = g_inventoryItems[itemIndex];
    const ExhibitionEntry previous = entry;

    entry.occupied = true;
    entry.quality = kInventoryQualityCount - 1;
    entry.wearUnits = item.wearUnits;
    entry.addedAt = ExhibitionUi_CurrentEpochSeconds();

    if (previous.occupied)
    {
        item.quality = previous.quality;
        item.appraised = true;
        item.appraisalDebug = false;
        item.wearUnits = previous.wearUnits;
    }
    else
    {
        item.storage = -1;
        item.quickSlot = -1;
        item.x = 0;
        item.y = 0;
    }

    ExhibitionUi_RefreshLevelProgress();
    InventoryUi_PruneQuickSlotBindings();
    InventoryUi_RefreshEquipment();
    InventoryUi_SaveState();
    InventoryUi_SetStatus(
        previous.occupied
            ? "展览品已替换  ·  原展品已回到所选物品的位置"
            : "物品已放入展览柜  ·  已自动保存");
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Exhibition slot %d updated from item slot %d; replaced=%d wear=%u.",
        expectedDefinition,
        itemIndex,
        previous.occupied ? 1 : 0,
        entry.wearUnits);
    return true;
}

bool ExhibitionUi_DepositItem(int itemIndex)
{
    if (!ExhibitionUi_CanDepositItem(itemIndex))
    {
        return false;
    }
    return ExhibitionUi_SwapItemIntoSlot(
        itemIndex,
        g_inventoryItems[itemIndex].definition);
}

bool ExhibitionUi_UnloadCurrentItem()
{
    if (!g_exhibitionPickerOpen ||
        g_exhibitionPickerDefinition < 0 ||
        g_exhibitionPickerDefinition >= kEquipmentInventoryDefinitionCount)
    {
        return false;
    }
    ExhibitionUi_EnsureData();
    auto& entry =
        g_exhibitionEntries[
            static_cast<size_t>(g_exhibitionPickerDefinition)];
    if (!entry.occupied)
    {
        InventoryUi_SetStatus("当前展览格没有可卸下的物品");
        return false;
    }
    const auto& definition =
        kEquipmentInventoryDefinitions[
            g_exhibitionPickerDefinition];
    int targetX = -1;
    int targetY = -1;
    const auto& stash =
        kInventoryStorageLayouts[kInventoryStorageStash];
    for (int y = 0;
         y <= stash.rows - definition.height &&
         targetX < 0;
         ++y)
    {
        for (int x = 0;
             x <= stash.columns - definition.width;
             ++x)
        {
            if (InventoryUi_StateCellFree(
                    kInventoryStorageStash,
                    x,
                    y,
                    definition.width,
                    definition.height,
                    -1))
            {
                targetX = x;
                targetY = y;
                break;
            }
        }
    }
    const int itemSlot =
        InventoryUi_FindFreeExtraItemSlot();
    if (targetX < 0 || itemSlot < 0)
    {
        InventoryUi_SetStatus(
            "仓库没有足够空间卸下展览品");
        return false;
    }
    auto& item = g_inventoryItems[itemSlot];
    item = InventoryItemState{};
    item.definition = g_exhibitionPickerDefinition;
    item.quality = kInventoryQualityCount - 1;
    item.appraised = true;
    item.appraisalDebug = false;
    item.quickSlot = -1;
    item.wearUnits = entry.wearUnits;
    item.storage = kInventoryStorageStash;
    item.x = targetX;
    item.y = targetY;
    item.width = definition.width;
    item.height = definition.height;
    entry = ExhibitionEntry{};

    ExhibitionUi_RefreshLevelProgress();
    InventoryUi_PruneQuickSlotBindings();
    InventoryUi_RefreshEquipment();
    InventoryUi_SaveState();
    InventoryUi_SetStatus("展览品已卸下并放回仓库");
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Exhibition item unloaded: definition=%d itemSlot=%d.",
        g_exhibitionPickerDefinition,
        itemSlot);
    ExhibitionUi_ClosePicker();
    ExhibitionUi_RefreshCards();
    return true;
}

void ExhibitionUi_ClosePicker()
{
    const bool wasOpen = g_exhibitionPickerOpen;
    const int previousDefinition = g_exhibitionPickerDefinition;
    if (g_exhibitionPickerRoot != nullptr)
    {
        ExtendedMode_DestroyUnityObject(g_exhibitionPickerRoot);
    }
    g_exhibitionPickerRoot = nullptr;
    g_exhibitionPickerClose = SceneUiElement{};
    g_exhibitionPickerPrev = SceneUiElement{};
    g_exhibitionPickerNext = SceneUiElement{};
    g_exhibitionPickerScrollTrack = SceneUiElement{};
    g_exhibitionPickerScrollThumb = SceneUiElement{};
    g_exhibitionPickerUnload = SceneUiElement{};
    g_exhibitionPickerUnloadLabel = nullptr;
    g_exhibitionPickerTitleElement = SceneUiElement{};
    g_exhibitionPickerCurrentElement = SceneUiElement{};
    g_exhibitionPickerHintElement = SceneUiElement{};
    g_exhibitionPickerEmptyElement = SceneUiElement{};
    g_exhibitionPickerPageElement = SceneUiElement{};
    g_exhibitionPickerTitle = nullptr;
    g_exhibitionPickerCurrent = nullptr;
    g_exhibitionPickerPageLabel = nullptr;
    g_exhibitionPickerEmpty = nullptr;
    for (auto& row : g_exhibitionPickerRows)
    {
        row = ExhibitionPickerRowUi{};
    }
    g_exhibitionPickerItems.clear();
    g_exhibitionPickerDefinition = -1;
    g_exhibitionPickerPage = 0;
    g_exhibitionPickerPageCount = 1;
    g_exhibitionPickerScrollRow = 0;
    g_exhibitionPickerScrollDragging = false;
    g_exhibitionPickerScrollDragOffset = 0.0f;
    g_exhibitionPickerWheelDelta = 0;
    g_exhibitionPickerOpen = false;
    if (wasOpen)
    {
        LogDebug(
            LogCategory_Inventory,
            L"ExhibitionPickerClose",
            L"Exhibition picker closed definition=%d.",
            previousDefinition);
    }
}

void ExhibitionUi_RefreshPicker()
{
    if (!g_exhibitionPickerOpen ||
        g_exhibitionPickerDefinition < 0 ||
        g_exhibitionPickerDefinition >= kEquipmentInventoryDefinitionCount)
    {
        return;
    }
    g_exhibitionPickerItems.clear();
    for (int index = 0; index < kInventoryItemSlotCount; ++index)
    {
        if (ExhibitionUi_CanSwapItem(index) &&
            g_inventoryItems[index].definition ==
                g_exhibitionPickerDefinition)
        {
            g_exhibitionPickerItems.push_back(index);
        }
    }
    std::sort(
        g_exhibitionPickerItems.begin(),
        g_exhibitionPickerItems.end(),
        [](int left, int right)
        {
            const auto& leftItem = g_inventoryItems[left];
            const auto& rightItem = g_inventoryItems[right];
            if (leftItem.wearUnits != rightItem.wearUnits)
            {
                return leftItem.wearUnits < rightItem.wearUnits;
            }
            return left < right;
        });
    const int maxScroll = std::max(
        0,
        static_cast<int>(g_exhibitionPickerItems.size()) -
            kExhibitionPickerRowCount);
    g_exhibitionPickerScrollRow = std::clamp(
        g_exhibitionPickerScrollRow,
        0,
        maxScroll);

    const int first = g_exhibitionPickerScrollRow;
    for (int rowIndex = 0;
         rowIndex < kExhibitionPickerRowCount;
         ++rowIndex)
    {
        auto& row = g_exhibitionPickerRows[rowIndex];
        const int listIndex = first + rowIndex;
        const bool visible =
            listIndex < static_cast<int>(g_exhibitionPickerItems.size());
        SetWorldGameObjectActive(row.panel.gameObject, visible);
        if (!visible)
        {
            continue;
        }
        const auto& item =
            g_inventoryItems[g_exhibitionPickerItems[
                static_cast<size_t>(listIndex)]];
        const auto& definition =
            kEquipmentInventoryDefinitions[item.definition];
        auto* texture = LoadScenePngTexture(
            InventoryUi_EquipmentImagePath(definition.id, 4));
        SetSceneRawImageTexture(
            row.image.rawImage,
            texture);
        SetSceneRawImagePreserveAspect(
            row.image.gameObject,
            texture);
        SetSceneText(
            row.name.text,
            definition.name,
            14.0f,
            514,
            0.95f,
            0.96f,
            0.98f,
            1.0f);
        const auto wear = InventoryUi_FormatWear(item.wearUnits);
        SetSceneText(
            row.wear.text,
            WideToUtf8(L"磨损 " + wear).c_str(),
            13.0f,
            514,
            0.73f,
            0.79f,
            0.86f,
            1.0f);
        SetSceneText(
            row.value.text,
            WideToUtf8(
                L"价值 " +
                ExhibitionUi_FormatValue(
                    InventoryUi_ItemCurrentValue(
                        g_exhibitionPickerItems[
                            static_cast<size_t>(listIndex)]))).c_str(),
            13.0f,
            514,
            0.90f,
            0.70f,
            0.32f,
            1.0f);
        SetSceneText(
            row.action.text,
            "点击替换",
            13.0f,
            514,
            0.64f,
            0.83f,
            0.68f,
            1.0f);
    }

    SetWorldGameObjectActive(
        g_exhibitionPickerEmptyElement.gameObject,
        g_exhibitionPickerItems.empty());
    char pickerHint[192]{};
    std::snprintf(
        pickerHint,
        sizeof(pickerHint),
        "同类红色已鉴定物品：%d 件  ·  磨损度由低到高排列",
        static_cast<int>(g_exhibitionPickerItems.size()));
    SetSceneText(
        g_exhibitionPickerHintElement.text,
        pickerHint,
        12.0f,
        514,
        0.62f,
        0.68f,
        0.75f,
        1.0f);
    const auto& entry =
        g_exhibitionEntries[
            static_cast<size_t>(g_exhibitionPickerDefinition)];
    const bool hasScroll =
        g_exhibitionPickerItems.size() >
        static_cast<size_t>(kExhibitionPickerRowCount);
    SetWorldGameObjectActive(
        g_exhibitionPickerScrollTrack.gameObject,
        hasScroll);
    SetWorldGameObjectActive(
        g_exhibitionPickerScrollThumb.gameObject,
        hasScroll);
    if (hasScroll)
    {
        const int maxScrollRows = std::max(1, maxScroll);
        const float trackHeight =
            g_exhibitionPickerScrollTrackMaxY -
            g_exhibitionPickerScrollTrackMinY;
        const float visibleRatio =
            static_cast<float>(kExhibitionPickerRowCount) /
            static_cast<float>(g_exhibitionPickerItems.size());
        const float thumbHeight = std::max(
            24.0f,
            trackHeight * visibleRatio);
        const float progress =
            maxScrollRows > 0
                ? static_cast<float>(g_exhibitionPickerScrollRow) /
                    static_cast<float>(maxScrollRows)
                : 0.0f;
        g_exhibitionPickerScrollThumbMinY =
            g_exhibitionPickerScrollTrackMinY +
            progress * (trackHeight - thumbHeight);
        g_exhibitionPickerScrollThumbMaxY =
            g_exhibitionPickerScrollThumbMinY + thumbHeight;
        ExhibitionUi_SetChildPixelRect(
            g_exhibitionPickerScrollThumb.rect,
            g_exhibitionPickerContentWidth,
            g_exhibitionPickerContentHeight,
            g_exhibitionPickerContentWidth - 30.0f,
            g_exhibitionPickerScrollThumbMinY,
            12.0f,
            thumbHeight);
    }
    const bool canUnload = entry.occupied;
    SetSceneGraphicColor(
        g_exhibitionPickerUnload.image,
        canUnload ? 0.50f : 0.12f,
        canUnload ? 0.14f : 0.14f,
        canUnload ? 0.10f : 0.16f,
        1.0f);
    SetSceneText(
        g_exhibitionPickerUnloadLabel,
        "卸下",
        14.0f,
        514,
        canUnload ? 1.0f : 0.55f,
        canUnload ? 0.88f : 0.58f,
        canUnload ? 0.86f : 0.63f,
        1.0f);
    const std::wstring currentText = entry.occupied
        ? L"当前展品  ·  磨损 " +
            InventoryUi_FormatWear(entry.wearUnits) +
            L"  ·  价值 " +
            ExhibitionUi_FormatValue(
                ExhibitionUi_ValueForDefinition(
                    g_exhibitionPickerDefinition))
        : L"当前展品  ·  未放入";
    SetSceneText(
        g_exhibitionPickerCurrent,
        WideToUtf8(currentText).c_str(),
        14.0f,
        514,
        0.80f,
        0.82f,
        0.86f,
        1.0f);
}

void ExhibitionUi_OpenPicker(int definition)
{
    if (definition < 0 ||
        definition >= kEquipmentInventoryDefinitionCount ||
        g_exhibitionPage.rect == nullptr)
    {
        return;
    }
    ExhibitionUi_ClosePicker();
    ExhibitionUi_EnsureData();
    g_exhibitionPickerDefinition = definition;
    g_exhibitionPickerOpen = true;
    LogInfo(
        LogCategory_Inventory,
        L"ExhibitionPickerOpen",
        L"Exhibition picker opened definition=%d.",
        definition);

    auto root = CreateScenePanel(
        "ExhibitionPickerOverlay",
        g_exhibitionPage.rect,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.84f);
    g_exhibitionPickerRoot = root.gameObject;

    auto content = CreateScenePanel(
        "ExhibitionPickerWindow",
        root.rect,
        0.105f,
        0.060f,
        0.895f,
        0.940f,
        0.035f,
        0.045f,
        0.058f,
        1.0f);
    auto* contentRect = content.rect;
    ExhibitionUi_ReadClientSize();
    const float pagePixelWidth =
        (kExhibitionPageMaxX - kExhibitionPageMinX) *
        g_exhibitionClientWidth;
    const float pagePixelHeight =
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        g_exhibitionClientHeight;
    g_exhibitionPickerContentWidth = std::min(
        1100.0f,
        pagePixelWidth - 80.0f);
    g_exhibitionPickerContentHeight = std::min(
        720.0f,
        pagePixelHeight - 50.0f);
    g_exhibitionPickerContentLeft =
        (pagePixelWidth - g_exhibitionPickerContentWidth) * 0.5f;
    g_exhibitionPickerContentTop =
        (pagePixelHeight - g_exhibitionPickerContentHeight) * 0.5f;
    ExhibitionUi_SetPagePixelRect(
        content.rect,
        g_exhibitionPickerContentLeft,
        g_exhibitionPickerContentTop,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight);
    const auto& definitionData =
        kEquipmentInventoryDefinitions[definition];
    char title[256]{};
    std::snprintf(
        title,
        sizeof(title),
        "选择展览品  ·  %s",
        definitionData.name);
    g_exhibitionPickerTitleElement = CreateSceneTextElement(
        "ExhibitionPickerTitle",
        contentRect,
        title,
        0.035f,
        0.885f,
        0.82f,
        0.965f,
        23.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    g_exhibitionPickerTitle = g_exhibitionPickerTitleElement.text;
    g_exhibitionPickerClose = CreateScenePanel(
        "ExhibitionPickerClose",
        contentRect,
        0.875f,
        0.885f,
        0.965f,
        0.965f,
        0.20f,
        0.07f,
        0.08f,
        1.0f);
    CreateSceneTextElement(
        "ExhibitionPickerCloseText",
        g_exhibitionPickerClose.rect,
        "×",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        22.0f,
        514,
        1.0f,
        0.90f,
        0.90f,
        1.0f);
    g_exhibitionPickerCurrentElement = CreateSceneTextElement(
        "ExhibitionPickerCurrent",
        contentRect,
        "",
        0.035f,
        0.825f,
        0.965f,
        0.878f,
        14.0f,
        514,
        0.80f,
        0.82f,
        0.86f,
        1.0f);
    g_exhibitionPickerCurrent = g_exhibitionPickerCurrentElement.text;
    g_exhibitionPickerHintElement = CreateSceneTextElement(
        "ExhibitionPickerHint",
        contentRect,
        "仅显示同类的红色已鉴定物品  ·  磨损度由低到高排列",
        0.035f,
        0.785f,
        0.965f,
        0.825f,
        12.0f,
        514,
        0.62f,
        0.68f,
        0.75f,
        1.0f);

    constexpr float rowTopPixels = 112.0f;
    constexpr float rowHeightPixels = 48.0f;
    constexpr float rowGapPixels = 6.0f;
    const float rowLeftPixels = 20.0f;
    const float rowWidthPixels =
        g_exhibitionPickerContentWidth - 64.0f;
    for (int index = 0;
         index < kExhibitionPickerRowCount;
         ++index)
    {
        const float rowTop =
            rowTopPixels +
            static_cast<float>(index) *
                (rowHeightPixels + rowGapPixels);
        auto& row = g_exhibitionPickerRows[index];
        row.panel = CreateScenePanel(
            "ExhibitionPickerRow",
            contentRect,
            0.0f,
            0.0f,
            0.001f,
            0.001f,
            0.060f,
            0.075f,
            0.094f,
            0.98f);
        ExhibitionUi_SetChildPixelRect(
            row.panel.rect,
            g_exhibitionPickerContentWidth,
            g_exhibitionPickerContentHeight,
            rowLeftPixels,
            rowTop,
            rowWidthPixels,
            rowHeightPixels);
        row.imageFrame = CreateScenePanel(
            "ExhibitionPickerImageFrame",
            row.panel.rect,
            0.012f,
            0.12f,
            0.105f,
            0.88f,
            0.0f,
            0.0f,
            0.0f,
            0.0f);
        row.image = CreateSceneRawImageElement(
            "ExhibitionPickerIcon",
            row.imageFrame.rect,
            std::wstring(),
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        row.name = CreateSceneTextElement(
            "ExhibitionPickerName",
            row.panel.rect,
            "",
            0.125f,
            0.18f,
            0.50f,
            0.82f,
            14.0f,
            514,
            0.95f,
            0.96f,
            0.98f,
            1.0f);
        row.wear = CreateSceneTextElement(
            "ExhibitionPickerWear",
            row.panel.rect,
            "",
            0.52f,
            0.18f,
            0.68f,
            0.82f,
            13.0f,
            514,
            0.73f,
            0.79f,
            0.86f,
            1.0f);
        row.value = CreateSceneTextElement(
            "ExhibitionPickerValue",
            row.panel.rect,
            "",
            0.69f,
            0.18f,
            0.84f,
            0.82f,
            13.0f,
            514,
            0.90f,
            0.70f,
            0.32f,
            1.0f);
        row.action = CreateSceneTextElement(
            "ExhibitionPickerAction",
            row.panel.rect,
            "点击替换",
            0.85f,
            0.18f,
            0.985f,
            0.82f,
            13.0f,
            514,
            0.64f,
            0.83f,
            0.68f,
            1.0f);
        ExhibitionUi_PickerChildRectToScreen(
            rowLeftPixels,
            rowTop,
            rowWidthPixels,
            rowHeightPixels,
            row.screenMinX,
            row.screenMinY,
            row.screenMaxX,
            row.screenMaxY);
    }

    g_exhibitionPickerEmptyElement = CreateSceneTextElement(
        "ExhibitionPickerEmpty",
        contentRect,
        "仓库中没有该物品的红色已鉴定副本\n请先在仓库完成鉴定，再回来替换。",
        0.12f,
        0.34f,
        0.88f,
        0.68f,
        18.0f,
        514,
        0.68f,
        0.72f,
        0.78f,
        1.0f);
    g_exhibitionPickerEmpty = g_exhibitionPickerEmptyElement.text;
    g_exhibitionPickerScrollTrack = CreateScenePanel(
        "ExhibitionPickerScrollTrack",
        contentRect,
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        0.10f,
        0.14f,
        0.19f,
        1.0f);
    g_exhibitionPickerScrollThumb = CreateScenePanel(
        "ExhibitionPickerScrollThumb",
        g_exhibitionPickerScrollTrack.rect,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.66f,
        0.48f,
        0.22f,
        1.0f);
    g_exhibitionPickerScrollTrackMinY = rowTopPixels;
    g_exhibitionPickerScrollTrackMaxY =
        rowTopPixels +
        static_cast<float>(kExhibitionPickerRowCount) *
            (rowHeightPixels + rowGapPixels) -
        rowGapPixels;
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionPickerScrollTrack.rect,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight,
        g_exhibitionPickerContentWidth - 30.0f,
        g_exhibitionPickerScrollTrackMinY,
        12.0f,
        g_exhibitionPickerScrollTrackMaxY -
            g_exhibitionPickerScrollTrackMinY);
    g_exhibitionPickerUnload = CreateScenePanel(
        "ExhibitionPickerUnload",
        contentRect,
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        0.50f,
        0.14f,
        0.10f,
        1.0f);
    g_exhibitionPickerUnloadLabel = CreateSceneTextElement(
        "ExhibitionPickerUnloadText",
        g_exhibitionPickerUnload.rect,
        "卸下",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        14.0f,
        514,
        1.0f,
        0.88f,
        0.86f,
        1.0f).text;
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionPickerUnload.rect,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight,
        g_exhibitionPickerContentWidth * 0.5f - 70.0f,
        g_exhibitionPickerContentHeight - 38.0f,
        140.0f,
        28.0f);
    ExhibitionUi_PickerChildRectToScreen(
        g_exhibitionPickerContentWidth * 0.5f - 70.0f,
        g_exhibitionPickerContentHeight - 38.0f,
        140.0f,
        28.0f,
        g_exhibitionPickerUnloadMinX,
        g_exhibitionPickerUnloadMinY,
        g_exhibitionPickerUnloadMaxX,
        g_exhibitionPickerUnloadMaxY);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionPickerTitleElement.rect,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight,
        20.0f,
        10.0f,
        g_exhibitionPickerContentWidth - 88.0f,
        30.0f);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionPickerClose.rect,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight,
        g_exhibitionPickerContentWidth - 54.0f,
        10.0f,
        34.0f,
        30.0f);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionPickerCurrentElement.rect,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight,
        20.0f,
        47.0f,
        g_exhibitionPickerContentWidth - 40.0f,
        22.0f);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionPickerHintElement.rect,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight,
        20.0f,
        74.0f,
        g_exhibitionPickerContentWidth - 40.0f,
        20.0f);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionPickerEmptyElement.rect,
        g_exhibitionPickerContentWidth,
        g_exhibitionPickerContentHeight,
        40.0f,
        190.0f,
        g_exhibitionPickerContentWidth - 80.0f,
        90.0f);
    BringSceneObjectToFront(g_exhibitionPickerRoot);
    ExhibitionUi_RefreshPicker();
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Exhibition picker opened: definition=%d eligibleItems=%d.",
        definition,
        static_cast<int>(g_exhibitionPickerItems.size()));
}

bool ExhibitionUi_LessForSort(
    int leftDefinition,
    int rightDefinition,
    int sortMode)
{
    if (sortMode == 0)
    {
        return leftDefinition < rightDefinition;
    }
    const auto& leftEntry =
        g_exhibitionEntries[static_cast<size_t>(leftDefinition)];
    const auto& rightEntry =
        g_exhibitionEntries[static_cast<size_t>(rightDefinition)];
    if (leftEntry.occupied != rightEntry.occupied)
    {
        return leftEntry.occupied;
    }
    if (!leftEntry.occupied)
    {
        return leftDefinition < rightDefinition;
    }
    switch (sortMode)
    {
    case 1:
        if (leftEntry.addedAt != rightEntry.addedAt)
        {
            return leftEntry.addedAt < rightEntry.addedAt;
        }
        break;
    case 2:
        if (leftEntry.addedAt != rightEntry.addedAt)
        {
            return leftEntry.addedAt > rightEntry.addedAt;
        }
        break;
    case 3:
    case 4:
    {
        const double leftValue =
            ExhibitionUi_ValueForDefinition(leftDefinition);
        const double rightValue =
            ExhibitionUi_ValueForDefinition(rightDefinition);
        if (leftValue != rightValue)
        {
            return sortMode == 3
                ? leftValue < rightValue
                : leftValue > rightValue;
        }
        break;
    }
    case 5:
        if (leftEntry.wearUnits != rightEntry.wearUnits)
        {
            return leftEntry.wearUnits < rightEntry.wearUnits;
        }
        break;
    case 6:
        if (leftEntry.wearUnits != rightEntry.wearUnits)
        {
            return leftEntry.wearUnits > rightEntry.wearUnits;
        }
        break;
    default:
        break;
    }
    return leftDefinition < rightDefinition;
}

void ExhibitionUi_RefreshLevelProgress()
{
    const double total = ExhibitionUi_CollectionTotal();
    const int level = ExhibitionUi_LevelForValue(total);
    g_exhibitionCachedLevel = level;
    const double progress =
        ExhibitionUi_ProgressWithinLevel(total, level);
    char label[192]{};
    if (level >= kExhibitionLevelCount)
    {
        std::snprintf(
            label,
            sizeof(label),
            "展览柜等级 Lv.10  ·  MAX  ·  %s",
            WideToUtf8(
                ExhibitionUi_FormatValue(total)).c_str());
    }
    else
    {
        std::snprintf(
            label,
            sizeof(label),
            "展览柜等级 Lv.%d  ·  %s / %s  ·  %d%%",
            level,
            WideToUtf8(
                ExhibitionUi_FormatValue(total)).c_str(),
            WideToUtf8(
                ExhibitionUi_FormatValue(
                    kExhibitionLevelThresholds[level])).c_str(),
            static_cast<int>(std::floor(progress * 100.0)));
    }
    SetSceneText(
        g_exhibitionLevelLabel,
        label,
        14.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    SetSceneRect(
        g_exhibitionLevelFill.rect,
        0.0f,
        0.0f,
        static_cast<float>(std::max(0.0, progress)),
        1.0f);
}

void ExhibitionUi_RefreshCards()
{
    if (!g_exhibitionPageBuilt)
    {
        return;
    }
    ExhibitionUi_EnsureData();
    g_exhibitionVisibleDefinitions.clear();
    for (int definition = 0;
         definition < kEquipmentInventoryDefinitionCount;
         ++definition)
    {
        if (ExhibitionUi_DefinitionMatchesCategory(
                definition,
                g_exhibitionCategory) &&
            ExhibitionUi_DefinitionMatchesSeries(
                definition,
                g_exhibitionSeries))
        {
            g_exhibitionVisibleDefinitions.push_back(definition);
        }
    }
    std::sort(
        g_exhibitionVisibleDefinitions.begin(),
        g_exhibitionVisibleDefinitions.end(),
        [](int left, int right)
        {
            return ExhibitionUi_LessForSort(
                left,
                right,
                g_exhibitionSort);
        });
    g_exhibitionPageCount = std::max(
        1,
        (static_cast<int>(g_exhibitionVisibleDefinitions.size()) +
         g_exhibitionCardsPerPage - 1) /
            g_exhibitionCardsPerPage);
    g_exhibitionPageIndex = std::clamp(
        g_exhibitionPageIndex,
        0,
        g_exhibitionPageCount - 1);
    const int first =
        g_exhibitionPageIndex * g_exhibitionCardsPerPage;
    int occupiedCount = 0;
    double totalValue = 0.0;
    for (const auto& entry : g_exhibitionEntries)
    {
        if (!entry.occupied)
        {
            continue;
        }
        ++occupiedCount;
    }
    for (int index = 0;
         index < kExhibitionMaxVisibleCardCount;
         ++index)
    {
        auto& card = g_exhibitionCards[index];
        const int listIndex = first + index;
        const bool visible =
            index < g_exhibitionCardsPerPage &&
            listIndex <
                static_cast<int>(g_exhibitionVisibleDefinitions.size());
        SetWorldGameObjectActive(card.panel.gameObject, visible);
        if (!visible)
        {
            card.definition = -1;
            continue;
        }
        const int definition =
            g_exhibitionVisibleDefinitions[
                static_cast<size_t>(listIndex)];
        const auto& itemDefinition =
            kEquipmentInventoryDefinitions[definition];
        const auto& entry =
            g_exhibitionEntries[static_cast<size_t>(definition)];
        if (card.definition != definition)
        {
            card.definition = definition;
            auto* texture = LoadScenePngTexture(
                InventoryUi_EquipmentImagePath(
                    itemDefinition.id,
                    4));
            SetSceneRawImageTexture(
                card.image.rawImage,
                texture);
            SetSceneRawImagePreserveAspect(
                card.image.gameObject,
                texture);
            SetSceneText(
                card.name.text,
                itemDefinition.name,
                12.0f,
                514,
                0.93f,
                0.95f,
                0.98f,
                1.0f);
        }
        SetWorldGameObjectActive(card.mask.gameObject, !entry.occupied);
        if (entry.occupied)
        {
            const auto value =
                ExhibitionUi_CollectionValueForDefinition(definition);
            totalValue += value;
            SetSceneText(
                card.value.text,
                WideToUtf8(
                    L"馆藏 " +
                    ExhibitionUi_FormatValue(value)).c_str(),
                11.0f,
                514,
                0.94f,
                0.73f,
                0.34f,
                1.0f);
            SetSceneText(
                card.time.text,
                WideToUtf8(
                    ExhibitionUi_FormatAddedAt(entry.addedAt)).c_str(),
                10.0f,
                514,
                0.68f,
                0.74f,
                0.81f,
                1.0f);
            SetSceneGraphicColor(
                card.image.rawImage,
                1.0f,
                1.0f,
                1.0f,
                1.0f);
        }
        else
        {
            SetSceneText(
                card.value.text,
                "价值 --",
                11.0f,
                514,
                0.62f,
                0.66f,
                0.71f,
                1.0f);
            SetSceneText(
                card.time.text,
                "点击放入",
                10.0f,
                514,
                0.62f,
                0.66f,
                0.71f,
                1.0f);
            SetSceneGraphicColor(
                card.image.rawImage,
                0.62f,
                0.64f,
                0.68f,
                1.0f);
        }
    }
    char pageText[64]{};
    std::snprintf(
        pageText,
        sizeof(pageText),
        "%d / %d",
        g_exhibitionPageIndex + 1,
        g_exhibitionPageCount);
    SetSceneText(
        g_exhibitionPageLabelElement.text,
        pageText,
        12.0f,
        514,
        0.72f,
        0.77f,
        0.84f,
        1.0f);
    char summary[192]{};
    std::snprintf(
        summary,
        sizeof(summary),
        "已收藏 %d/%d  ·  页值 %s",
        occupiedCount,
        kEquipmentInventoryDefinitionCount,
        WideToUtf8(
            ExhibitionUi_FormatValue(totalValue)).c_str());
    SetSceneText(
        g_exhibitionSummaryLabel,
        summary,
        13.0f,
        257,
        0.72f,
        0.77f,
        0.84f,
        1.0f);
    SetWorldGameObjectActive(
        g_exhibitionNoItemsElement.gameObject,
        g_exhibitionVisibleDefinitions.empty());
    ExhibitionUi_RefreshLevelProgress();
}

void ExhibitionUi_RefreshLabels()
{
    if (g_exhibitionSeriesLabel == nullptr ||
        g_exhibitionCategoryLabel == nullptr ||
        g_exhibitionSortLabel == nullptr)
    {
        return;
    }
    char seriesText[160]{};
    std::snprintf(
        seriesText,
        sizeof(seriesText),
        "系列：%s  %s",
        g_exhibitionSeries == 0
            ? "全部系列"
            : kEquipmentSeriesNames[g_exhibitionSeries - 1],
        g_exhibitionSeriesOpen ? "▲" : "▼");
    SetSceneText(
        g_exhibitionSeriesLabel,
        seriesText,
        15.0f,
        514,
        0.91f,
        0.93f,
        0.96f,
        1.0f);
    char categoryText[128]{};
    std::snprintf(
        categoryText,
        sizeof(categoryText),
        "类别：%s  %s",
        kExhibitionCategories[g_exhibitionCategory].label,
        g_exhibitionCategoryOpen ? "▲" : "▼");
    SetSceneText(
        g_exhibitionCategoryLabel,
        categoryText,
        15.0f,
        514,
        0.91f,
        0.93f,
        0.96f,
        1.0f);
    char sortText[128]{};
    std::snprintf(
        sortText,
        sizeof(sortText),
        "排序：%s  %s",
        kExhibitionSortLabels[g_exhibitionSort],
        g_exhibitionSortOpen ? "▲" : "▼");
    SetSceneText(
        g_exhibitionSortLabel,
        sortText,
        15.0f,
        514,
        0.91f,
        0.93f,
        0.96f,
        1.0f);
}

void ExhibitionUi_SetDropdownState(
    bool seriesOpen,
    bool categoryOpen,
    bool sortOpen)
{
    g_exhibitionSeriesOpen = seriesOpen;
    g_exhibitionCategoryOpen = categoryOpen;
    g_exhibitionSortOpen = sortOpen;
    SetWorldGameObjectActive(
        g_exhibitionSeriesDropdown.gameObject,
        seriesOpen);
    SetWorldGameObjectActive(
        g_exhibitionCategoryDropdown.gameObject,
        categoryOpen);
    SetWorldGameObjectActive(
        g_exhibitionSortDropdown.gameObject,
        sortOpen);
    if (seriesOpen)
    {
        BringSceneObjectToFront(
            g_exhibitionSeriesDropdown.gameObject);
    }
    if (categoryOpen)
    {
        BringSceneObjectToFront(
            g_exhibitionCategoryDropdown.gameObject);
    }
    if (sortOpen)
    {
        BringSceneObjectToFront(
            g_exhibitionSortDropdown.gameObject);
    }
    ExhibitionUi_RefreshLabels();
}

void ExhibitionUi_BuildDropdown(
    const char* rootName,
    const char* optionName,
    Il2CppObject* parent,
    float buttonMinX,
    float buttonMinY,
    float buttonMaxX,
    int optionCount,
    int optionColumns,
    const char* const* labels,
    SceneUiElement* dropdown,
    SceneUiElement* options,
    float* optionMinX,
    float* optionMinY,
    float* optionMaxX,
    float* optionMaxY)
{
    constexpr float rowHeight = 0.043f;
    constexpr float padding = 0.008f;
    const int rows =
        (optionCount + optionColumns - 1) / optionColumns;
    const float dropdownMinX = buttonMinX;
    const float dropdownMaxX = buttonMaxX;
    const float dropdownMaxY = buttonMinY - 0.004f;
    const float dropdownMinY =
        dropdownMaxY -
        padding * 2.0f -
        rowHeight * static_cast<float>(rows);
    *dropdown = CreateScenePanel(
        rootName,
        parent,
        dropdownMinX,
        dropdownMinY,
        dropdownMaxX,
        dropdownMaxY,
        0.045f,
        0.055f,
        0.070f,
        1.0f);
    for (int index = 0; index < optionCount; ++index)
    {
        const int column = index % optionColumns;
        const int row = index / optionColumns;
        const float columnWidth =
            (1.0f - padding * 2.0f) /
            static_cast<float>(optionColumns);
        const float maxY =
            dropdownMaxY - padding -
            rowHeight * static_cast<float>(row);
        const float minY = maxY - rowHeight + 0.002f;
        options[index] = CreateSceneButton(
            optionName,
            dropdown->rect,
            labels[index],
            padding + columnWidth * static_cast<float>(column),
            minY,
            padding + columnWidth * static_cast<float>(column + 1),
            maxY,
            0.075f,
            0.090f,
            0.115f,
            1.0f,
            13.0f,
            514,
            true);
        SetButtonInteractable(options[index].button, false);
        const float contentWidth =
            kExhibitionPageMaxX - kExhibitionPageMinX;
        const float contentHeight =
            kExhibitionPageMaxY - kExhibitionPageMinY;
        optionMinX[index] =
            kExhibitionPageMinX +
            (dropdownMinX + padding) * contentWidth;
        optionMaxX[index] =
            kExhibitionPageMinX +
            (dropdownMaxX - padding) * contentWidth;
        optionMinY[index] =
            kExhibitionPageMinY + minY * contentHeight;
        optionMaxY[index] =
            kExhibitionPageMinY + maxY * contentHeight;
    }
    SetWorldGameObjectActive(dropdown->gameObject, false);
}

void ExhibitionUi_Build(Il2CppObject* rootTransform)
{
    if (rootTransform == nullptr || g_exhibitionPageBuilt)
    {
        return;
    }
    ExhibitionUi_EnsureData();
    g_exhibitionPage = CreateScenePanel(
        "ExhibitionPage",
        rootTransform,
        kExhibitionPageMinX,
        kExhibitionPageMinY,
        kExhibitionPageMaxX,
        kExhibitionPageMaxY,
        0.018f,
        0.025f,
        0.034f,
        1.0f);
    g_exhibitionHeader = CreateScenePanel(
        "ExhibitionHeader",
        g_exhibitionPage.rect,
        0.014f,
        0.835f,
        0.986f,
        0.982f,
        0.030f,
        0.040f,
        0.053f,
        1.0f);
    g_exhibitionTitleElement = CreateSceneTextElement(
        "ExhibitionTitle",
        g_exhibitionHeader.rect,
        "展览柜",
        0.020f,
        0.545f,
        0.285f,
        0.945f,
        20.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    g_exhibitionPageLabel = g_exhibitionTitleElement.text;
    g_exhibitionSummaryElement = CreateSceneTextElement(
        "ExhibitionSummary",
        g_exhibitionHeader.rect,
        "",
        0.020f,
        0.075f,
        0.285f,
        0.465f,
        11.0f,
        257,
        0.72f,
        0.77f,
        0.84f,
        1.0f);
    g_exhibitionSummaryLabel = g_exhibitionSummaryElement.text;
    g_exhibitionLevelLabelElement = CreateSceneTextElement(
        "ExhibitionLevelLabel",
        g_exhibitionHeader.rect,
        "展览柜等级 Lv.1",
        0.220f,
        0.620f,
        0.500f,
        0.900f,
        14.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    g_exhibitionLevelLabel = g_exhibitionLevelLabelElement.text;
    g_exhibitionLevelTrack = CreateScenePanel(
        "ExhibitionLevelTrack",
        g_exhibitionHeader.rect,
        0.220f,
        0.220f,
        0.500f,
        0.480f,
        0.12f,
        0.15f,
        0.19f,
        1.0f);
    g_exhibitionLevelFill = CreateScenePanel(
        "ExhibitionLevelFill",
        g_exhibitionLevelTrack.rect,
        0.0f,
        0.0f,
        0.34f,
        1.0f,
        0.75f,
        0.48f,
        0.16f,
        1.0f);
    g_exhibitionSeriesButton = CreateSceneButton(
        "ExhibitionSeriesButton",
        g_exhibitionHeader.rect,
        "系列：全部系列 ▼",
        0.160f,
        0.080f,
        0.290f,
        0.920f,
        0.100f,
        0.115f,
        0.145f,
        1.0f,
        15.0f,
        514,
        true);
    SetButtonInteractable(g_exhibitionSeriesButton.button, false);
    g_exhibitionSeriesLabelElement = g_exhibitionSeriesButton;
    g_exhibitionSeriesLabel = g_exhibitionSeriesLabelElement.text;
    g_exhibitionCategoryButton = CreateSceneButton(
        "ExhibitionCategoryButton",
        g_exhibitionHeader.rect,
        "类别：全部 ▼",
        0.300f,
        0.080f,
        0.600f,
        0.920f,
        0.100f,
        0.115f,
        0.145f,
        1.0f,
        15.0f,
        514,
        true);
    SetButtonInteractable(g_exhibitionCategoryButton.button, false);
    g_exhibitionCategoryLabelElement = g_exhibitionCategoryButton;
    g_exhibitionCategoryLabel = g_exhibitionCategoryLabelElement.text;
    g_exhibitionSortButton = CreateSceneButton(
        "ExhibitionSortButton",
        g_exhibitionHeader.rect,
        "排序：默认排序 ▼",
        0.620f,
        0.080f,
        0.975f,
        0.920f,
        0.100f,
        0.115f,
        0.145f,
        1.0f,
        15.0f,
        514,
        true);
    SetButtonInteractable(g_exhibitionSortButton.button, false);
    g_exhibitionSortLabelElement = g_exhibitionSortButton;
    g_exhibitionSortLabel = g_exhibitionSortLabelElement.text;

    const float buttonCategoryMinX =
        0.014f + 0.300f * (0.986f - 0.014f);
    const float buttonCategoryMaxX =
        0.014f + 0.600f * (0.986f - 0.014f);
    const float buttonSortMinX =
        0.014f + 0.620f * (0.986f - 0.014f);
    const float buttonSortMaxX =
        0.014f + 0.975f * (0.986f - 0.014f);
    const float buttonMinY =
        0.835f + 0.080f * (0.982f - 0.835f);
    const float buttonMaxY =
        0.835f + 0.920f * (0.982f - 0.835f);
    ExhibitionUi_PageToScreen(
        buttonCategoryMinX,
        buttonMinY,
        g_exhibitionCategoryButtonMinX,
        g_exhibitionCategoryButtonMinY);
    ExhibitionUi_PageToScreen(
        buttonCategoryMaxX,
        buttonMaxY,
        g_exhibitionCategoryButtonMaxX,
        g_exhibitionCategoryButtonMaxY);
    ExhibitionUi_PageToScreen(
        buttonSortMinX,
        buttonMinY,
        g_exhibitionSortButtonMinX,
        g_exhibitionSortButtonMinY);
    ExhibitionUi_PageToScreen(
        buttonSortMaxX,
        buttonMaxY,
        g_exhibitionSortButtonMaxX,
        g_exhibitionSortButtonMaxY);

    for (int index = 0;
         index < kExhibitionMaxVisibleCardCount;
         ++index)
    {
        auto& card = g_exhibitionCards[index];
        card.panel = CreateScenePanel(
            "ExhibitionCard",
            g_exhibitionPage.rect,
            0.0f,
            0.0f,
            0.001f,
            0.001f,
            0.047f,
            0.058f,
            0.075f,
            0.98f);
        card.imageFrame = CreateScenePanel(
            "ExhibitionCardImageFrame",
            card.panel.rect,
            0.045f,
            0.290f,
            0.955f,
            0.975f,
            0.0f,
            0.0f,
            0.0f,
            0.0f);
        card.image = CreateSceneRawImageElement(
            "ExhibitionCardImage",
            card.imageFrame.rect,
            std::wstring(),
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        card.mask = CreateScenePanel(
            "ExhibitionCardMask",
            card.panel.rect,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.50f,
            0.52f,
            0.56f,
            0.62f);
        card.name = CreateSceneTextElement(
            "ExhibitionCardName",
            card.panel.rect,
            "",
            0.035f,
            0.205f,
            0.965f,
            0.295f,
            13.0f,
            514,
            0.93f,
            0.95f,
            0.98f,
            1.0f);
        card.value = CreateSceneTextElement(
            "ExhibitionCardValue",
            card.panel.rect,
            "",
            0.035f,
            0.110f,
            0.965f,
            0.205f,
            11.0f,
            514,
            0.94f,
            0.73f,
            0.34f,
            1.0f);
        card.time = CreateSceneTextElement(
            "ExhibitionCardTime",
            card.panel.rect,
            "",
            0.035f,
            0.010f,
            0.965f,
            0.105f,
            10.0f,
            514,
            0.68f,
            0.74f,
            0.81f,
            1.0f);
    }

    g_exhibitionNoItemsElement = CreateSceneTextElement(
        "ExhibitionNoItems",
        g_exhibitionPage.rect,
        "该类别下没有物品",
        0.30f,
        0.35f,
        0.70f,
        0.55f,
        18.0f,
        514,
        0.67f,
        0.72f,
        0.79f,
        1.0f);
    g_exhibitionNoItemsLabel = g_exhibitionNoItemsElement.text;
    g_exhibitionPrevPage = CreateSceneButton(
        "ExhibitionPrevPage",
        g_exhibitionPage.rect,
        "<",
        0.755f,
        0.025f,
        0.820f,
        0.080f,
        0.11f,
        0.15f,
        0.21f,
        1.0f,
        16.0f,
        514,
        true);
    SetButtonInteractable(g_exhibitionPrevPage.button, false);
    g_exhibitionPageLabelElement = CreateSceneTextElement(
        "ExhibitionPageLabel",
        g_exhibitionPage.rect,
        "1 / 1",
        0.820f,
        0.025f,
        0.890f,
        0.080f,
        12.0f,
        514,
        0.72f,
        0.77f,
        0.84f,
        1.0f);
    g_exhibitionNextPage = CreateSceneButton(
        "ExhibitionNextPage",
        g_exhibitionPage.rect,
        ">",
        0.890f,
        0.025f,
        0.955f,
        0.080f,
        0.11f,
        0.15f,
        0.21f,
        1.0f,
        16.0f,
        514,
        true);
    SetButtonInteractable(g_exhibitionNextPage.button, false);

    const char* seriesLabels[kExhibitionSeriesOptionCount]{};
    seriesLabels[0] = "全部系列";
    for (int index = 0;
         index < kEquipmentSeriesCount;
         ++index)
    {
        seriesLabels[index + 1] = kEquipmentSeriesNames[index];
    }
    ExhibitionUi_BuildDropdown(
        "ExhibitionSeriesDropdown",
        "ExhibitionSeriesOption",
        g_exhibitionPage.rect,
        0.160f,
        0.840f,
        0.290f,
        kExhibitionSeriesOptionCount,
        3,
        seriesLabels,
        &g_exhibitionSeriesDropdown,
        g_exhibitionSeriesOptions,
        g_exhibitionSeriesOptionMinX,
        g_exhibitionSeriesOptionMinY,
        g_exhibitionSeriesOptionMaxX,
        g_exhibitionSeriesOptionMaxY);

    const char* categoryLabels[kExhibitionCategoryCount]{};
    for (int index = 0;
         index < kExhibitionCategoryCount;
         ++index)
    {
        categoryLabels[index] =
            kExhibitionCategories[index].label;
    }
    ExhibitionUi_BuildDropdown(
        "ExhibitionCategoryDropdown",
        "ExhibitionCategoryOption",
        g_exhibitionPage.rect,
        buttonCategoryMinX,
        buttonMinY,
        buttonCategoryMaxX,
        kExhibitionCategoryCount,
        2,
        categoryLabels,
        &g_exhibitionCategoryDropdown,
        g_exhibitionCategoryOptions,
        g_exhibitionCategoryOptionMinX,
        g_exhibitionCategoryOptionMinY,
        g_exhibitionCategoryOptionMaxX,
        g_exhibitionCategoryOptionMaxY);
    ExhibitionUi_BuildDropdown(
        "ExhibitionSortDropdown",
        "ExhibitionSortOption",
        g_exhibitionPage.rect,
        buttonSortMinX,
        buttonMinY,
        buttonSortMaxX,
        kExhibitionSortCount,
        2,
        kExhibitionSortLabels,
        &g_exhibitionSortDropdown,
        g_exhibitionSortOptions,
        g_exhibitionSortOptionMinX,
        g_exhibitionSortOptionMinY,
        g_exhibitionSortOptionMaxX,
        g_exhibitionSortOptionMaxY);

    g_exhibitionPageBuilt = true;
    ExhibitionUi_ApplyPixelLayout();
    g_exhibitionObservedLeftSerial =
        g_extendedManualPointerPressSerial.load();
    SetWorldGameObjectActive(g_exhibitionPage.gameObject, false);
    ExhibitionUi_SetDropdownState(false, false, false);
    ExhibitionUi_RefreshCards();
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Exhibition cabinet UI built with %d catalog slots.",
        kEquipmentInventoryDefinitionCount);
}

void ExhibitionUi_SetPageVisible(bool visible)
{
    if (g_exhibitionPage.gameObject != nullptr)
    {
        SetWorldGameObjectActive(
            g_exhibitionPage.gameObject,
            visible);
    }
    if (!visible)
    {
        ExhibitionUi_ClosePicker();
        ExhibitionUi_CloseLevelDetail();
        ExhibitionUi_SetDropdownState(false, false, false);
    }
    else
    {
        ExhibitionUi_ApplyPixelLayout();
        ExhibitionUi_RefreshCards();
        g_exhibitionObservedLeftSerial =
            g_extendedManualPointerPressSerial.load();
    }
}

bool ExhibitionUi_HandleButtonClick(Il2CppObject* instance)
{
    if (!g_exhibitionPageBuilt ||
        instance == nullptr ||
        g_inventoryActiveRoute != kInventoryRouteCabinet ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (instance == g_exhibitionSeriesButton.button)
    {
        ExhibitionUi_SetDropdownState(
            !g_exhibitionSeriesOpen,
            false,
            false);
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition series button clicked: open=%d.",
            g_exhibitionSeriesOpen ? 1 : 0);
        return true;
    }
    if (instance == g_exhibitionCategoryButton.button)
    {
        ExhibitionUi_SetDropdownState(
            false,
            !g_exhibitionCategoryOpen,
            false);
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition category button clicked: open=%d.",
            g_exhibitionCategoryOpen ? 1 : 0);
        return true;
    }
    for (int index = 0;
         index < kExhibitionSeriesOptionCount;
         ++index)
    {
        if (instance == g_exhibitionSeriesOptions[index].button)
        {
            g_exhibitionSeries = index;
            g_exhibitionPageIndex = 0;
            ExhibitionUi_SetDropdownState(false, false, false);
            ExhibitionUi_RefreshCards();
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Exhibition series selected: index=%d label=%S.",
                index,
                index == 0
                    ? "全部系列"
                    : kEquipmentSeriesNames[index - 1]);
            return true;
        }
    }
    if (instance == g_exhibitionSortButton.button)
    {
        ExhibitionUi_SetDropdownState(
            false,
            false,
            !g_exhibitionSortOpen);
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition sort button clicked: open=%d.",
            g_exhibitionSortOpen ? 1 : 0);
        return true;
    }
    for (int index = 0;
         index < kExhibitionCategoryCount;
         ++index)
    {
        if (instance == g_exhibitionCategoryOptions[index].button)
        {
            g_exhibitionCategory = index;
            g_exhibitionPageIndex = 0;
            ExhibitionUi_SetDropdownState(false, false, false);
            ExhibitionUi_RefreshCards();
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Exhibition category selected: index=%d label=%S.",
                index,
                kExhibitionCategories[index].label);
            return true;
        }
    }
    for (int index = 0;
         index < kExhibitionSortCount;
         ++index)
    {
        if (instance == g_exhibitionSortOptions[index].button)
        {
            g_exhibitionSort = index;
            g_exhibitionPageIndex = 0;
            ExhibitionUi_SetDropdownState(false, false, false);
            ExhibitionUi_RefreshCards();
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Exhibition sort selected: index=%d label=%S.",
                index,
                kExhibitionSortLabels[index]);
            return true;
        }
    }
    if (instance == g_exhibitionPrevPage.button)
    {
        if (g_exhibitionPageIndex > 0)
        {
            --g_exhibitionPageIndex;
            ExhibitionUi_RefreshCards();
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Exhibition page changed: page=%d/%d.",
                g_exhibitionPageIndex + 1,
                g_exhibitionPageCount);
        }
        return true;
    }
    if (instance == g_exhibitionNextPage.button)
    {
        if (g_exhibitionPageIndex + 1 < g_exhibitionPageCount)
        {
            ++g_exhibitionPageIndex;
            ExhibitionUi_RefreshCards();
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Exhibition page changed: page=%d/%d.",
                g_exhibitionPageIndex + 1,
                g_exhibitionPageCount);
        }
        return true;
    }
    return false;
}

void ExhibitionUi_LoadState(const nlohmann::json& root)
{
    ExhibitionUi_EnsureData();
    std::fill(
        g_exhibitionEntries.begin(),
        g_exhibitionEntries.end(),
        ExhibitionEntry{});
    g_exhibitionCachedLevel = 1;
    if (!root.is_object() ||
        !root.contains("exhibition") ||
        !root["exhibition"].is_array())
    {
        return;
    }
    std::unordered_set<int> loadedDefinitions;
    int loadedCount = 0;
    for (const auto& saved : root["exhibition"])
    {
        if (!saved.is_object() ||
            !saved.contains("id") ||
            !saved["id"].is_string())
        {
            continue;
        }
        const int definition = InventoryUi_FindDefinitionById(
            saved["id"].get<std::string>().c_str());
        if (definition < 0 ||
            definition >= kEquipmentInventoryDefinitionCount ||
            !loadedDefinitions.insert(definition).second)
        {
            continue;
        }
        const bool appraised = saved.value("appraised", true);
        const int quality = saved.value("quality", 4);
        const unsigned int wearUnits = static_cast<unsigned int>(
            std::clamp(
                saved.value("wear_units", 0),
                0,
                499999999));
        if (!appraised ||
            quality != kInventoryQualityCount - 1 ||
            wearUnits == 0)
        {
            continue;
        }
        auto& entry =
            g_exhibitionEntries[static_cast<size_t>(definition)];
        entry.occupied = true;
        entry.quality = kInventoryQualityCount - 1;
        entry.wearUnits = wearUnits;
        entry.addedAt = saved.value(
            "added_at",
            static_cast<unsigned long long>(0));
        ++loadedCount;
    }
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Exhibition state loaded with %d item(s).",
        loadedCount);
    g_exhibitionCachedLevel = ExhibitionUi_LevelForValue(
        ExhibitionUi_CollectionTotal());
}

void ExhibitionUi_WriteState(nlohmann::json& root)
{
    ExhibitionUi_EnsureData();
    root["exhibition"] = nlohmann::json::array();
    for (int definition = 0;
         definition < kEquipmentInventoryDefinitionCount;
         ++definition)
    {
        const auto& entry =
            g_exhibitionEntries[static_cast<size_t>(definition)];
        if (!entry.occupied)
        {
            continue;
        }
        nlohmann::json saved;
        saved["id"] =
            kEquipmentInventoryDefinitions[definition].id;
        saved["quality"] = kInventoryQualityCount - 1;
        saved["appraised"] = true;
        saved["wear_units"] = entry.wearUnits;
        saved["added_at"] = entry.addedAt;
        root["exhibition"].push_back(std::move(saved));
    }
}

void ExhibitionUi_Reset()
{
    ExhibitionUi_ClosePicker();
    ExhibitionUi_CloseLevelDetail();
    g_exhibitionPage = SceneUiElement{};
    g_exhibitionHeader = SceneUiElement{};
    g_exhibitionPageLabel = nullptr;
    g_exhibitionSummaryLabel = nullptr;
    g_exhibitionTitleElement = SceneUiElement{};
    g_exhibitionSummaryElement = SceneUiElement{};
    g_exhibitionLevelLabelElement = SceneUiElement{};
    g_exhibitionLevelTrack = SceneUiElement{};
    g_exhibitionLevelFill = SceneUiElement{};
    g_exhibitionLevelLabel = nullptr;
    g_exhibitionLevelBarPixelWidth = 1.0f;
    g_exhibitionCategoryButton = SceneUiElement{};
    g_exhibitionSortButton = SceneUiElement{};
    g_exhibitionSeriesButton = SceneUiElement{};
    g_exhibitionSeriesLabel = nullptr;
    g_exhibitionCategoryLabel = nullptr;
    g_exhibitionSortLabel = nullptr;
    g_exhibitionSeriesLabelElement = SceneUiElement{};
    g_exhibitionCategoryLabelElement = SceneUiElement{};
    g_exhibitionSortLabelElement = SceneUiElement{};
    g_exhibitionSeriesDropdown = SceneUiElement{};
    g_exhibitionCategoryDropdown = SceneUiElement{};
    g_exhibitionSortDropdown = SceneUiElement{};
    for (auto& option : g_exhibitionSeriesOptions)
    {
        option = SceneUiElement{};
    }
    for (auto& option : g_exhibitionCategoryOptions)
    {
        option = SceneUiElement{};
    }
    for (auto& option : g_exhibitionSortOptions)
    {
        option = SceneUiElement{};
    }
    for (auto& card : g_exhibitionCards)
    {
        card = ExhibitionCardUi{};
    }
    g_exhibitionPageLabelElement = SceneUiElement{};
    g_exhibitionNoItemsLabel = nullptr;
    g_exhibitionNoItemsElement = SceneUiElement{};
    g_exhibitionPrevPage = SceneUiElement{};
    g_exhibitionNextPage = SceneUiElement{};
    g_exhibitionVisibleDefinitions.clear();
    g_exhibitionSeries = 0;
    g_exhibitionCategory = 0;
    g_exhibitionSort = 0;
    g_exhibitionPageIndex = 0;
    g_exhibitionPageCount = 1;
    g_exhibitionSeriesOpen = false;
    g_exhibitionCategoryOpen = false;
    g_exhibitionSortOpen = false;
    g_exhibitionPageBuilt = false;
}

bool ExhibitionUi_ShouldHandleManualPointerAt(
    float cursorX,
    float cursorY)
{
    return g_exhibitionPageBuilt &&
           g_inventoryActiveRoute == kInventoryRouteCabinet &&
           ExhibitionUi_PointIn(
               cursorX,
               cursorY,
               kExhibitionPageMinX,
               kExhibitionPageMinY,
               kExhibitionPageMaxX,
               kExhibitionPageMaxY);
}

bool ExhibitionUi_HandlePickerClick(float cursorX, float cursorY)
{
    if (!g_exhibitionPickerOpen)
    {
        return false;
    }
    float closeMinX = 0.0f;
    float closeMinY = 0.0f;
    float closeMaxX = 0.0f;
    float closeMaxY = 0.0f;
    ExhibitionUi_PickerChildRectToScreen(
        g_exhibitionPickerContentWidth - 54.0f,
        10.0f,
        34.0f,
        30.0f,
        closeMinX,
        closeMinY,
        closeMaxX,
        closeMaxY);
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            closeMinX,
            closeMinY,
            closeMaxX,
            closeMaxY))
    {
        ExhibitionUi_ClosePicker();
        return true;
    }

    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            g_exhibitionPickerUnloadMinX,
            g_exhibitionPickerUnloadMinY,
            g_exhibitionPickerUnloadMaxX,
            g_exhibitionPickerUnloadMaxY))
    {
        ExhibitionUi_UnloadCurrentItem();
        return true;
    }

    float trackMinX = 0.0f;
    float trackMinY = 0.0f;
    float trackMaxX = 0.0f;
    float trackMaxY = 0.0f;
    ExhibitionUi_PickerChildRectToScreen(
        g_exhibitionPickerContentWidth - 30.0f,
        g_exhibitionPickerScrollTrackMinY,
        12.0f,
        g_exhibitionPickerScrollTrackMaxY -
            g_exhibitionPickerScrollTrackMinY,
        trackMinX,
        trackMinY,
        trackMaxX,
        trackMaxY);
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            trackMinX,
            trackMinY,
            trackMaxX,
            trackMaxY) &&
        g_exhibitionPickerItems.size() >
            static_cast<size_t>(kExhibitionPickerRowCount))
    {
        const float thumbHeight =
            g_exhibitionPickerScrollThumbMaxY -
            g_exhibitionPickerScrollThumbMinY;
        const float relativeY =
            (trackMaxY - cursorY) /
            std::max(0.001f, trackMaxY - trackMinY);
        const float contentY =
            g_exhibitionPickerScrollTrackMaxY -
            relativeY *
                (g_exhibitionPickerScrollTrackMaxY -
                 g_exhibitionPickerScrollTrackMinY);
        const bool onThumb =
            contentY >= g_exhibitionPickerScrollThumbMinY &&
            contentY <= g_exhibitionPickerScrollThumbMaxY;
        g_exhibitionPickerScrollDragging = true;
        g_exhibitionPickerScrollDragOffset =
            onThumb
                ? contentY - g_exhibitionPickerScrollThumbMinY
                : thumbHeight * 0.5f;
        const float movableTrack =
            g_exhibitionPickerScrollTrackMaxY -
            g_exhibitionPickerScrollTrackMinY -
            thumbHeight;
        const float thumbTop = std::clamp(
            contentY - g_exhibitionPickerScrollDragOffset,
            g_exhibitionPickerScrollTrackMinY,
            g_exhibitionPickerScrollTrackMaxY - thumbHeight);
        const int maxScroll = std::max(
            1,
            static_cast<int>(g_exhibitionPickerItems.size()) -
                kExhibitionPickerRowCount);
        g_exhibitionPickerScrollRow = static_cast<int>(std::round(
            (thumbTop - g_exhibitionPickerScrollTrackMinY) /
            std::max(0.001f, movableTrack) *
            static_cast<float>(maxScroll)));
        ExhibitionUi_RefreshPicker();
        return true;
    }

    const int first = g_exhibitionPickerScrollRow;
    for (int index = 0;
         index < kExhibitionPickerRowCount;
         ++index)
    {
        const int listIndex = first + index;
        if (listIndex >=
                static_cast<int>(g_exhibitionPickerItems.size()) ||
            !ExhibitionUi_PointIn(
                cursorX,
                cursorY,
                g_exhibitionPickerRows[index].screenMinX,
                g_exhibitionPickerRows[index].screenMinY,
                g_exhibitionPickerRows[index].screenMaxX,
                g_exhibitionPickerRows[index].screenMaxY))
        {
            continue;
        }
        const int itemIndex =
            g_exhibitionPickerItems[static_cast<size_t>(listIndex)];
        if (ExhibitionUi_SwapItemIntoSlot(
                itemIndex,
                g_exhibitionPickerDefinition))
        {
            ExhibitionUi_ClosePicker();
            ExhibitionUi_RefreshCards();
        }
        return true;
    }
    return true;
}

void ExhibitionUi_CloseLevelDetail()
{
    const bool wasOpen = g_exhibitionLevelDetailOpen;
    if (g_exhibitionLevelDetailRoot != nullptr)
    {
        ExtendedMode_DestroyUnityObject(g_exhibitionLevelDetailRoot);
    }
    g_exhibitionLevelDetailRoot = nullptr;
    g_exhibitionLevelDetailClose = SceneUiElement{};
    for (auto& segment : g_exhibitionLevelDetailSegmentBackgrounds)
    {
        segment = SceneUiElement{};
    }
    for (auto& fill : g_exhibitionLevelDetailSegmentFills)
    {
        fill = SceneUiElement{};
    }
    for (auto& row : g_exhibitionLevelDetailRows)
    {
        row = SceneUiElement{};
    }
    g_exhibitionLevelDetailOpen = false;
    if (wasOpen)
    {
        LogDebug(
            LogCategory_Inventory,
            L"ExhibitionLevelClose",
            L"Exhibition level detail closed.");
    }
}

void ExhibitionUi_OpenLevelDetail()
{
    if (!g_exhibitionPageBuilt || g_exhibitionPage.rect == nullptr)
    {
        return;
    }
    ExhibitionUi_CloseLevelDetail();
    ExhibitionUi_ReadClientSize();
    const float pagePixelWidth =
        (kExhibitionPageMaxX - kExhibitionPageMinX) *
        g_exhibitionClientWidth;
    const float pagePixelHeight =
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        g_exhibitionClientHeight;
    const float contentWidth = std::min(1200.0f, pagePixelWidth - 80.0f);
    const float contentHeight = std::min(860.0f, pagePixelHeight - 30.0f);
    const float contentLeft = (pagePixelWidth - contentWidth) * 0.5f;
    const float contentTop = (pagePixelHeight - contentHeight) * 0.5f;
    ExhibitionUi_PixelRectToScreen(
        contentLeft,
        contentTop,
        contentWidth,
        contentHeight,
        g_exhibitionLevelDetailContentMinX,
        g_exhibitionLevelDetailContentMinY,
        g_exhibitionLevelDetailContentMaxX,
        g_exhibitionLevelDetailContentMaxY);

    auto root = CreateScenePanel(
        "ExhibitionLevelDetailOverlay",
        g_exhibitionPage.rect,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.88f);
    g_exhibitionLevelDetailRoot = root.gameObject;
    auto content = CreateScenePanel(
        "ExhibitionLevelDetail",
        root.rect,
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        0.032f,
        0.042f,
        0.055f,
        1.0f);
    ExhibitionUi_SetPagePixelRect(
        content.rect,
        contentLeft,
        contentTop,
        contentWidth,
        contentHeight);

    const double total = ExhibitionUi_CollectionTotal();
    const int currentLevel = ExhibitionUi_LevelForValue(total);
    const double currentProgress =
        ExhibitionUi_ProgressWithinLevel(total, currentLevel);
    auto titleElement = CreateSceneTextElement(
        "ExhibitionLevelDetailTitle",
        content.rect,
        "展览柜等级",
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        22.0f,
        514,
        0.96f,
        0.97f,
        0.99f,
        1.0f);
    ExhibitionUi_SetChildPixelRect(
        titleElement.rect,
        contentWidth,
        contentHeight,
        20.0f,
        10.0f,
        340.0f,
        30.0f);
    char currentText[256]{};
    std::snprintf(
        currentText,
        sizeof(currentText),
        "当前 Lv.%d  ·  馆藏 %s  ·  本级进度 %d%%",
        currentLevel,
        WideToUtf8(
            ExhibitionUi_FormatValue(total)).c_str(),
        static_cast<int>(std::floor(currentProgress * 100.0)));
    auto currentElement = CreateSceneTextElement(
        "ExhibitionLevelDetailCurrent",
        content.rect,
        currentText,
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        14.0f,
        514,
        0.82f,
        0.86f,
        0.92f,
        1.0f);
    ExhibitionUi_SetChildPixelRect(
        currentElement.rect,
        contentWidth,
        contentHeight,
        20.0f,
        48.0f,
        contentWidth - 40.0f,
        24.0f);
    g_exhibitionLevelDetailClose = CreateScenePanel(
        "ExhibitionLevelDetailClose",
        content.rect,
        0.0f,
        0.0f,
        0.001f,
        0.001f,
        0.20f,
        0.07f,
        0.08f,
        1.0f);
    ExhibitionUi_SetChildPixelRect(
        g_exhibitionLevelDetailClose.rect,
        contentWidth,
        contentHeight,
        contentWidth - 62.0f,
        10.0f,
        44.0f,
        32.0f);
    CreateSceneTextElement(
        "ExhibitionLevelDetailCloseText",
        g_exhibitionLevelDetailClose.rect,
        "×",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        22.0f,
        514,
        1.0f,
        0.90f,
        0.90f,
        1.0f);
    ExhibitionUi_PixelRectToScreen(
        contentLeft + contentWidth - 72.0f,
        contentTop + 4.0f,
        64.0f,
        44.0f,
        g_exhibitionLevelDetailCloseMinX,
        g_exhibitionLevelDetailCloseMinY,
        g_exhibitionLevelDetailCloseMaxX,
        g_exhibitionLevelDetailCloseMaxY);

    constexpr float barLeft = 20.0f;
    constexpr float barWidth = 14.0f;
    const float barTop = 88.0f;
    const float barHeight = contentHeight - 106.0f;
    constexpr float segmentGap = 4.0f;
    const float segmentHeight =
        (barHeight - segmentGap * (kExhibitionLevelCount - 1)) /
        static_cast<float>(kExhibitionLevelCount);
    const float rowLeft = 54.0f;
    const float rowWidth = contentWidth - rowLeft - 18.0f;
    for (int index = 0; index < kExhibitionLevelCount; ++index)
    {
        const int displayRow = index;
        const float segmentTop =
            barTop +
            static_cast<float>(displayRow) *
                (segmentHeight + segmentGap);
        auto& background =
            g_exhibitionLevelDetailSegmentBackgrounds[index];
        background = CreateScenePanel(
            "ExhibitionLevelSegment",
            content.rect,
            0.0f,
            0.0f,
            0.001f,
            0.001f,
            0.12f,
            0.15f,
            0.19f,
            1.0f);
        ExhibitionUi_SetChildPixelRect(
            background.rect,
            contentWidth,
            contentHeight,
            barLeft,
            segmentTop,
            barWidth,
            segmentHeight);
        auto& fill =
            g_exhibitionLevelDetailSegmentFills[index];
        fill = CreateScenePanel(
            "ExhibitionLevelSegmentFill",
            background.rect,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.78f,
            0.50f,
            0.16f,
            1.0f);
        float fillRatio = 0.0f;
        if (index < currentLevel)
        {
            fillRatio = 1.0f;
        }
        else if (index == currentLevel)
        {
            fillRatio = static_cast<float>(currentProgress);
        }
        const bool showFill = fillRatio > 0.0f;
        SetWorldGameObjectActive(
            fill.gameObject,
            showFill);
        if (showFill)
        {
            SetSceneRect(
                fill.rect,
                0.0f,
                1.0f - std::clamp(fillRatio, 0.001f, 1.0f),
                1.0f,
                1.0f);
        }

        auto& row = g_exhibitionLevelDetailRows[index];
        row = CreateScenePanel(
            "ExhibitionLevelRow",
            content.rect,
            0.0f,
            0.0f,
            0.001f,
            0.001f,
            index == currentLevel
                ? 0.13f
                : 0.060f,
            index == currentLevel
                ? 0.11f
                : 0.075f,
            index == currentLevel
                ? 0.12f
                : 0.094f,
            0.98f);
        ExhibitionUi_SetChildPixelRect(
            row.rect,
            contentWidth,
            contentHeight,
            rowLeft,
            segmentTop,
            rowWidth,
            segmentHeight);
        const int low = kExhibitionLevelLowPermille[index];
        const double r = (1000.0 - low) / 10.0;
        const double sr = low / 20.0;
        const double ssr = low / 25.0;
        const double ur = low / 100.0;
        char line1[256]{};
        std::snprintf(
            line1,
            sizeof(line1),
            "Lv.%d   累计 %s   安全箱 %d 格",
            index + 1,
            WideToUtf8(
                ExhibitionUi_FormatValue(
                    kExhibitionLevelThresholds[index])).c_str(),
            kExhibitionLevelSafeCells[index]);
        CreateSceneTextElement(
            "ExhibitionLevelRequirement",
            row.rect,
            line1,
            0.018f,
            0.50f,
            0.982f,
            0.95f,
            13.0f,
            514,
            0.93f,
            0.95f,
            0.98f,
            1.0f);
        char line2[256]{};
        std::snprintf(
            line2,
            sizeof(line2),
            "R %.1f%%   SR %.1f%%   SSR %.1f%%   UR %.1f%%",
            r,
            sr,
            ssr,
            ur);
        CreateSceneTextElement(
            "ExhibitionLevelProbability",
            row.rect,
            line2,
            0.018f,
            0.08f,
            0.982f,
            0.49f,
            12.0f,
            514,
            0.72f,
            0.78f,
            0.85f,
            1.0f);
    }
    BringSceneObjectToFront(
        g_exhibitionLevelDetailClose.gameObject);
    BringSceneObjectToFront(g_exhibitionLevelDetailRoot);
    g_exhibitionLevelDetailOpen = true;
    LogInfo(
        LogCategory_Inventory,
        L"ExhibitionLevelOpen",
        L"Exhibition level detail opened level=%d value=%.0f.",
        ExhibitionUi_GetCurrentLevel(),
        ExhibitionUi_CollectionTotal());
}

bool ExhibitionUi_HandleLevelDetailClick(float cursorX, float cursorY)
{
    if (!g_exhibitionLevelDetailOpen)
    {
        return false;
    }
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            g_exhibitionLevelDetailCloseMinX,
            g_exhibitionLevelDetailCloseMinY,
            g_exhibitionLevelDetailCloseMaxX,
            g_exhibitionLevelDetailCloseMaxY))
    {
        ExhibitionUi_CloseLevelDetail();
    }
    else if (!ExhibitionUi_PointIn(
                 cursorX,
                 cursorY,
                 g_exhibitionLevelDetailContentMinX,
                 g_exhibitionLevelDetailContentMinY,
                 g_exhibitionLevelDetailContentMaxX,
                 g_exhibitionLevelDetailContentMaxY))
    {
        ExhibitionUi_CloseLevelDetail();
    }
    return true;
}

void ExhibitionUi_UpdatePickerScrollDrag(float cursorY)
{
    if (!g_exhibitionPickerOpen ||
        !g_exhibitionPickerScrollDragging ||
        g_exhibitionPickerItems.size() <=
            static_cast<size_t>(kExhibitionPickerRowCount))
    {
        return;
    }
    ExhibitionUi_ReadClientSize();
    const float pagePixelHeight =
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        g_exhibitionClientHeight;
    const float contentY =
        (kExhibitionPageMaxY - cursorY) /
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        pagePixelHeight -
        g_exhibitionPickerContentTop;
    const float thumbHeight =
        g_exhibitionPickerScrollThumbMaxY -
        g_exhibitionPickerScrollThumbMinY;
    const float thumbTop = std::clamp(
        contentY - g_exhibitionPickerScrollDragOffset,
        g_exhibitionPickerScrollTrackMinY,
        g_exhibitionPickerScrollTrackMaxY - thumbHeight);
    const float movableTrack =
        g_exhibitionPickerScrollTrackMaxY -
        g_exhibitionPickerScrollTrackMinY -
        thumbHeight;
    const int maxScroll = std::max(
        1,
        static_cast<int>(g_exhibitionPickerItems.size()) -
            kExhibitionPickerRowCount);
    const int scrollRow = std::clamp(
        static_cast<int>(std::round(
            (thumbTop - g_exhibitionPickerScrollTrackMinY) /
            std::max(0.001f, movableTrack) *
            static_cast<float>(maxScroll))),
        0,
        maxScroll);
    if (scrollRow != g_exhibitionPickerScrollRow)
    {
        g_exhibitionPickerScrollRow = scrollRow;
        ExhibitionUi_RefreshPicker();
    }
}

bool ExhibitionUi_IsPickerOpen()
{
    return g_exhibitionPickerOpen;
}

void ExhibitionUi_QueuePickerWheel(int delta)
{
    if (g_exhibitionPickerOpen && delta != 0)
    {
        g_exhibitionPickerWheelDelta.fetch_add(delta);
    }
}

void ExhibitionUi_Tick()
{
    if (!g_exhibitionPageBuilt ||
        g_extendedModeState.load() != ExtendedMode_Active ||
        g_inventoryActiveRoute != kInventoryRouteCabinet)
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
    const float cursorX = static_cast<float>(cursor.x) / width;
    const float cursorY = 1.0f - static_cast<float>(cursor.y) / height;
    const int wheelDelta = g_exhibitionPickerWheelDelta.exchange(0);
    if (g_exhibitionPickerOpen && wheelDelta != 0)
    {
        const int maxScroll = std::max(
            0,
            static_cast<int>(g_exhibitionPickerItems.size()) -
                kExhibitionPickerRowCount);
        const int rows =
            -wheelDelta / WHEEL_DELTA;
        const int next = std::clamp(
            g_exhibitionPickerScrollRow + rows,
            0,
            maxScroll);
        if (next != g_exhibitionPickerScrollRow)
        {
            g_exhibitionPickerScrollRow = next;
            ExhibitionUi_RefreshPicker();
        }
    }
    if (g_exhibitionPickerOpen &&
        g_exhibitionPickerScrollDragging)
    {
        if (g_extendedManualPointerDown.load())
        {
            ExhibitionUi_UpdatePickerScrollDrag(cursorY);
        }
        else
        {
            g_exhibitionPickerScrollDragging = false;
        }
        return;
    }
    const unsigned int leftSerial =
        g_extendedManualPointerPressSerial.load();
    const bool leftPressed =
        leftSerial != g_exhibitionObservedLeftSerial;
    if (leftPressed)
    {
        g_exhibitionObservedLeftSerial = leftSerial;
    }
    if (!leftPressed ||
        g_extendedManualPointerConsumed.load())
    {
        return;
    }
    if (g_exhibitionPickerOpen)
    {
        ExhibitionUi_HandlePickerClick(cursorX, cursorY);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (g_exhibitionLevelDetailOpen)
    {
        ExhibitionUi_HandleLevelDetailClick(cursorX, cursorY);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            g_exhibitionLevelHitMinX,
            g_exhibitionLevelHitMinY,
            g_exhibitionLevelHitMaxX,
            g_exhibitionLevelHitMaxY))
    {
        ExhibitionUi_OpenLevelDetail();
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            g_exhibitionSeriesButtonMinX,
            g_exhibitionSeriesButtonMinY,
            g_exhibitionSeriesButtonMaxX,
            g_exhibitionSeriesButtonMaxY))
    {
        ExhibitionUi_SetDropdownState(
            !g_exhibitionSeriesOpen,
            false,
            false);
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition series button clicked: open=%d.",
            g_exhibitionSeriesOpen ? 1 : 0);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            g_exhibitionCategoryButtonMinX,
            g_exhibitionCategoryButtonMinY,
            g_exhibitionCategoryButtonMaxX,
            g_exhibitionCategoryButtonMaxY))
    {
        ExhibitionUi_SetDropdownState(
            false,
            !g_exhibitionCategoryOpen,
            false);
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition category button clicked: open=%d.",
            g_exhibitionCategoryOpen ? 1 : 0);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            g_exhibitionSortButtonMinX,
            g_exhibitionSortButtonMinY,
            g_exhibitionSortButtonMaxX,
            g_exhibitionSortButtonMaxY))
    {
        ExhibitionUi_SetDropdownState(
            false,
            false,
            !g_exhibitionSortOpen);
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition sort button clicked: open=%d.",
            g_exhibitionSortOpen ? 1 : 0);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (g_exhibitionSeriesOpen)
    {
        for (int index = 0;
             index < kExhibitionSeriesOptionCount;
             ++index)
        {
            if (ExhibitionUi_PointIn(
                    cursorX,
                    cursorY,
                    g_exhibitionSeriesOptionMinX[index],
                    g_exhibitionSeriesOptionMinY[index],
                    g_exhibitionSeriesOptionMaxX[index],
                    g_exhibitionSeriesOptionMaxY[index]))
            {
                g_exhibitionSeries = index;
                g_exhibitionPageIndex = 0;
                ExhibitionUi_SetDropdownState(false, false, false);
                ExhibitionUi_RefreshCards();
                g_extendedManualPointerConsumed = true;
                return;
            }
        }
        ExhibitionUi_SetDropdownState(false, false, false);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (g_exhibitionCategoryOpen)
    {
        for (int index = 0;
             index < kExhibitionCategoryCount;
             ++index)
        {
            if (ExhibitionUi_PointIn(
                    cursorX,
                    cursorY,
                    g_exhibitionCategoryOptionMinX[index],
                    g_exhibitionCategoryOptionMinY[index],
                    g_exhibitionCategoryOptionMaxX[index],
                    g_exhibitionCategoryOptionMaxY[index]))
            {
                g_exhibitionCategory = index;
                g_exhibitionPageIndex = 0;
                ExhibitionUi_SetDropdownState(false, false, false);
                ExhibitionUi_RefreshCards();
                g_extendedManualPointerConsumed = true;
                return;
            }
        }
        ExhibitionUi_SetDropdownState(false, false, false);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (g_exhibitionSortOpen)
    {
        for (int index = 0;
             index < kExhibitionSortCount;
             ++index)
        {
            if (ExhibitionUi_PointIn(
                    cursorX,
                    cursorY,
                    g_exhibitionSortOptionMinX[index],
                    g_exhibitionSortOptionMinY[index],
                    g_exhibitionSortOptionMaxX[index],
                    g_exhibitionSortOptionMaxY[index]))
            {
                g_exhibitionSort = index;
                g_exhibitionPageIndex = 0;
                ExhibitionUi_SetDropdownState(false, false, false);
                ExhibitionUi_RefreshCards();
                g_extendedManualPointerConsumed = true;
                return;
            }
        }
        ExhibitionUi_SetDropdownState(false, false, false);
        g_extendedManualPointerConsumed = true;
        return;
    }

    ExhibitionUi_ReadClientSize();
    const float pageWidth =
        (kExhibitionPageMaxX - kExhibitionPageMinX) *
        g_exhibitionClientWidth;
    const float pageHeight =
        (kExhibitionPageMaxY - kExhibitionPageMinY) *
        g_exhibitionClientHeight;
    const float footerTop = pageHeight - 10.0f - 32.0f;
    float prevMinX = 0.0f;
    float prevMinY = 0.0f;
    float prevMaxX = 0.0f;
    float prevMaxY = 0.0f;
    ExhibitionUi_PixelRectToScreen(
        pageWidth * 0.5f - 88.0f,
        footerTop,
        44.0f,
        32.0f,
        prevMinX,
        prevMinY,
        prevMaxX,
        prevMaxY);
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            prevMinX,
            prevMinY,
            prevMaxX,
            prevMaxY) &&
        g_exhibitionPageIndex > 0)
    {
        --g_exhibitionPageIndex;
        ExhibitionUi_RefreshCards();
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition page changed: page=%d/%d.",
            g_exhibitionPageIndex + 1,
            g_exhibitionPageCount);
        g_extendedManualPointerConsumed = true;
        return;
    }
    float nextMinX = 0.0f;
    float nextMinY = 0.0f;
    float nextMaxX = 0.0f;
    float nextMaxY = 0.0f;
    ExhibitionUi_PixelRectToScreen(
        pageWidth * 0.5f + 44.0f,
        footerTop,
        44.0f,
        32.0f,
        nextMinX,
        nextMinY,
        nextMaxX,
        nextMaxY);
    if (ExhibitionUi_PointIn(
            cursorX,
            cursorY,
            nextMinX,
            nextMinY,
            nextMaxX,
            nextMaxY) &&
        g_exhibitionPageIndex + 1 < g_exhibitionPageCount)
    {
        ++g_exhibitionPageIndex;
        ExhibitionUi_RefreshCards();
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Exhibition page changed: page=%d/%d.",
            g_exhibitionPageIndex + 1,
            g_exhibitionPageCount);
        g_extendedManualPointerConsumed = true;
        return;
    }
    if (g_exhibitionSeriesOpen ||
        g_exhibitionCategoryOpen ||
        g_exhibitionSortOpen)
    {
        ExhibitionUi_SetDropdownState(false, false, false);
        g_extendedManualPointerConsumed = true;
        return;
    }
    for (int index = 0;
         index < g_exhibitionCardsPerPage;
         ++index)
    {
        const auto& card = g_exhibitionCards[index];
        if (card.definition >= 0 &&
            ExhibitionUi_PointIn(
                cursorX,
                cursorY,
                card.screenMinX,
                card.screenMinY,
                card.screenMaxX,
                card.screenMaxY))
        {
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Exhibition card clicked: definition=%d occupied=%d.",
                card.definition,
                g_exhibitionEntries[
                    static_cast<size_t>(card.definition)].occupied
                    ? 1
                    : 0);
            ExhibitionUi_OpenPicker(card.definition);
            g_extendedManualPointerConsumed = true;
            return;
        }
    }
}
