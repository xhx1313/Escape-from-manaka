// Native.cpp module: gameplay_minimap.inc
// Circular, view-relative gameplay minimap for the Escape HUD.

struct GameplayMinimapFloor
{
    float height = 0.0f;
    float minX = 0.0f;
    float maxX = 1.0f;
    float minZ = 0.0f;
    float maxZ = 1.0f;
    std::wstring imagePath;
};

struct GameplayMinimapState
{
    Il2CppObject* root = nullptr;
    Il2CppObject* rootRect = nullptr;
    Il2CppObject* rotationContainer = nullptr;
    Il2CppObject* mapRect = nullptr;
    Il2CppObject* rawImage = nullptr;
    Il2CppObject* texture = nullptr;
    std::wstring texturePath;
    int stageType = -1;
    int floorIndex = -1;
    std::vector<GameplayMinimapFloor> floors;
    std::vector<CheckpointMapPoint> points;
    Il2CppObject* markerSprite = nullptr;
    Il2CppObject* squareMarkerSprite = nullptr;
    Il2CppObject* starMarkerSprite = nullptr;
    Il2CppObject* edgeArrowSprite = nullptr;
};

struct GameplayMinimapPointMarker
{
    SceneUiElement marker{};
    int runtimeIndex = -1;
    bool usingEdge = false;
};

constexpr int kGameplayWorldMapMaxFloorRows = 16;

struct GameplayWorldMapPointMarker
{
    SceneUiElement marker{};
    int runtimeIndex = -1;
    float visualScale = 1.0f;
};

struct GameplayWorldMapFloorRow
{
    SceneUiElement background{};
    SceneUiElement label{};
};

struct GameplayWorldMapState
{
    Il2CppObject* root = nullptr;
    Il2CppObject* rootRect = nullptr;
    Il2CppObject* mapRect = nullptr;
    Il2CppObject* rawImage = nullptr;
    Il2CppObject* texture = nullptr;
    Il2CppObject* floorLabel = nullptr;
    Il2CppObject* usageLabel = nullptr;
    SceneUiElement dropdownButton{};
    SceneUiElement dropdownLabel{};
    SceneUiElement upButton{};
    SceneUiElement upLabel{};
    SceneUiElement downButton{};
    SceneUiElement downLabel{};
    GameplayWorldMapFloorRow floorRows[
        kGameplayWorldMapMaxFloorRows]{};
    std::wstring texturePath;
    int stageType = -1;
    int floorIndex = -1;
    bool dropdownOpen = false;
    int pressedControl = 0;
    float zoom = 1.0f;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float fitSize = 720.0f;
    float lastPointerX = 0.0f;
    float lastPointerY = 0.0f;
    bool pointerWasDown = false;
    bool dragging = false;
    int hoveredMarkerIndex = -1;
    int pressedMarkerRuntimeIndex = -1;
    std::vector<CheckpointMapPoint> points;
};

constexpr float kGameplayMinimapDiameter = 176.0f;
constexpr float kGameplayMinimapMarginLeft = 18.0f;
constexpr float kGameplayMinimapMarginTop = 18.0f;
constexpr float kGameplayMinimapRadiusMeters = 5.0f;
constexpr float kGameplayWorldMapMinZoom = 0.55f;
constexpr float kGameplayWorldMapMaxZoom = 7.0f;
constexpr int kGameplayMapMarkerCapacity = 64;
constexpr float kGameplayWorldMapMarkerSize = 66.0f;
constexpr int kGameplayWorldMapControlNone = 0;
constexpr int kGameplayWorldMapControlDropdown = 1;
constexpr int kGameplayWorldMapControlUp = 2;
constexpr int kGameplayWorldMapControlDown = 3;
constexpr int kGameplayWorldMapControlConsumed = 4;
constexpr int kGameplayWorldMapControlRowBase = 100;

GameplayMinimapState g_gameplayMinimap;
GameplayWorldMapState g_gameplayWorldMap;
std::atomic<bool> g_gameplayWorldMapPhysicalDown{false};
std::atomic<int> g_gameplayWorldMapWheelDelta{0};
std::atomic<bool> g_gameplayWorldMapPointerDown{false};
std::atomic<unsigned long long> g_gameplayWorldMapNativeInputBlockedUntil{0};
std::atomic<int> g_gameplayTrackedMapPointRuntimeIndex{-1};
GameplayMinimapPointMarker
    g_gameplayMinimapPointMarkers[
        kGameplayMapMarkerCapacity]{};
GameplayWorldMapPointMarker
    g_gameplayWorldMapPointMarkers[
        kGameplayMapMarkerCapacity]{};

void GameplayMinimap_Reset();
void GameplayWorldMap_Reset();
void GameplayWorldMap_Build(Il2CppObject* parentTransform);
void GameplayWorldMap_SetDisplayedFloor(int floorIndex);
void GameplayWorldMap_ReleaseTexture();
bool GameplayWorldMap_ProcessPhysicalKey(
    int virtualKey,
    bool down,
    const wchar_t* source);
bool GameplayWorldMap_ProcessMouseMessage(
    UINT message,
    LONG screenX,
    LONG screenY,
    int wheelDelta);
void GameplayWorldMap_ResetPhysicalInput();
int GameplayMapTracking_GetRuntimeIndex();
void GameplayMapTracking_Toggle(int runtimeIndex);
void GameplayMapTracking_Clear();

std::string GameplayMinimap_ReadUtf8File(const std::wstring& path)
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
        LogInfo(LogCategory_Ui, L"Ui", L"Minimap manifest not found: %s", path.c_str());
        return {};
    }
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) ||
        size.QuadPart <= 0 ||
        size.QuadPart > 1024 * 1024)
    {
        CloseHandle(file);
        LogInfo(LogCategory_Ui, L"Ui", L"Minimap manifest has an invalid size: %s", path.c_str());
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
        LogInfo(LogCategory_Ui, L"Ui", L"Minimap manifest read failed: %s", path.c_str());
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

const wchar_t* GameplayMinimap_SceneDirectory(int stageType)
{
    switch (stageType)
    {
    case 6:
        return L"konbini";
    case 7:
        return L"fashion_shop";
    case 10:
        return L"residence";
    case 11:
        return L"mall";
    case 12:
        return L"station_front";
    case 14:
        return L"park";
    case 15:
        return L"mansion";
    default:
        return nullptr;
    }
}

const char* GameplayMinimap_SceneLabelUtf8(int stageType)
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
    default: return "地图";
    }
}

void GameplayWorldMap_ResetPhysicalInput()
{
    g_gameplayWorldMapPhysicalDown = false;
    g_gameplayWorldMapPointerDown = false;
    g_gameplayWorldMapWheelDelta = 0;
    g_gameplayWorldMapNativeInputBlockedUntil = 0;
}

int GameplayMapTracking_GetRuntimeIndex()
{
    return g_gameplayTrackedMapPointRuntimeIndex.load();
}

void GameplayMapTracking_Toggle(int runtimeIndex)
{
    if (!CheckpointSystem_IsRuntimeMapVisible(runtimeIndex))
    {
        return;
    }
    int previous = g_gameplayTrackedMapPointRuntimeIndex.load();
    while (!g_gameplayTrackedMapPointRuntimeIndex.compare_exchange_weak(
        previous,
        previous == runtimeIndex ? -1 : runtimeIndex))
    {
    }
    LogInfo(LogCategory_Ui, L"Ui", 
        L"World map tracked checkpoint changed: runtime=%d tracked=%d.",
        runtimeIndex,
        g_gameplayTrackedMapPointRuntimeIndex.load());
}

void GameplayMapTracking_Clear()
{
    g_gameplayTrackedMapPointRuntimeIndex = -1;
}

bool GameplayWorldMap_ShouldBlockNativeInput()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }

    const unsigned long long now = GetTickCount64();
    const bool physicalDown =
        g_gameplayWorldMapPhysicalDown.load() ||
        (GameplayInput_ReadRawAsyncKeyState('M') & 0x8000) != 0;
    if (physicalDown)
    {
        g_gameplayWorldMapNativeInputBlockedUntil.store(
            now + 250);
    }
    return g_gameplayWorldMapOpen.load() ||
        g_gameplayWorldMapToggleRequested.load() ||
        physicalDown ||
        now < g_gameplayWorldMapNativeInputBlockedUntil.load();
}

bool GameplayWorldMap_ProcessPhysicalKey(
    int virtualKey,
    bool down,
    const wchar_t* source)
{
    if ((virtualKey != 'M' && virtualKey != 'm') ||
        g_extendedModeState.load() != ExtendedMode_Gameplay ||
        g_extendedOriginalSystemMenuVisible.load())
    {
        return false;
    }
    if (down)
    {
        g_gameplayWorldMapNativeInputBlockedUntil.store(
            GetTickCount64() + 250);
        const bool anotherModalOpen =
            !g_gameplayWorldMapOpen.load() &&
            GameplayInput_IsCustomModalUiOpen();
        const bool wasDown =
            g_gameplayWorldMapPhysicalDown.exchange(true);
        if (!wasDown && !anotherModalOpen)
        {
            if (!g_gameplayWorldMapToggleRequested.exchange(true))
            {
                LogInfo(LogCategory_Ui, L"Ui", 
                    L"World map toggle queued by %s.",
                    source != nullptr ? source : L"input");
            }
        }
        else if (anotherModalOpen)
        {
            g_gameplayWorldMapToggleRequested = false;
        }
    }
    else
    {
        if (g_gameplayWorldMapPhysicalDown.exchange(false))
        {
            g_gameplayWorldMapNativeInputBlockedUntil.store(
                GetTickCount64() + 150);
        }
    }
    return true;
}

bool GameplayWorldMap_ProcessMouseMessage(
    UINT message,
    LONG screenX,
    LONG screenY,
    int wheelDelta)
{
    (void)screenX;
    (void)screenY;
    if (!g_gameplayWorldMapOpen.load())
    {
        return false;
    }
    if (message == WM_LBUTTONDOWN ||
        message == WM_LBUTTONDBLCLK)
    {
        g_gameplayWorldMapPointerDown = true;
        return true;
    }
    if (message == WM_LBUTTONUP ||
        message == WM_CAPTURECHANGED)
    {
        g_gameplayWorldMapPointerDown = false;
        return message != WM_CAPTURECHANGED;
    }
    if (message == WM_MOUSEWHEEL)
    {
        g_gameplayWorldMapWheelDelta.fetch_add(wheelDelta);
        return true;
    }
    return message != WM_MOUSEMOVE;
}

bool GameplayMinimap_LoadStage(int stageType)
{
    g_gameplayMinimap.stageType = stageType;
    g_gameplayMinimap.floorIndex = -1;
    g_gameplayMinimap.texture = nullptr;
    g_gameplayMinimap.floors.clear();
    const std::wstring mapRoot =
        GetEscapeFromManakaSubdirectory(L"Maps");

    const wchar_t* sceneDirectory =
        GameplayMinimap_SceneDirectory(stageType);
    if (sceneDirectory == nullptr)
    {
        return false;
    }
    const std::wstring sceneRoot =
        mapRoot + L"\\" + sceneDirectory;
    const std::string content =
        GameplayMinimap_ReadUtf8File(
            sceneRoot + L"\\manifest.json");
    if (content.empty())
    {
        return false;
    }
    try
    {
        const auto manifest = nlohmann::json::parse(content);
        if (!manifest.contains("floors") ||
            !manifest["floors"].is_array())
        {
            return false;
        }
        for (const auto& floorValue : manifest["floors"])
        {
            GameplayMinimapFloor floor;
            floor.height = floorValue.value("height", 0.0f);
            floor.minX = floorValue.value("minX", 0.0f);
            floor.maxX = floorValue.value("maxX", 1.0f);
            floor.minZ = floorValue.value("minZ", 0.0f);
            floor.maxZ = floorValue.value("maxZ", 1.0f);
            const std::string image =
                floorValue.value("image2560", std::string());
            if (image.empty())
            {
                continue;
            }
            floor.imagePath = sceneRoot + L"\\" +
                Utf8ToWide(image);
            g_gameplayMinimap.floors.push_back(floor);
        }
    }
    catch (const std::exception& exception)
    {
        LogInfo(LogCategory_Ui, L"Ui", 
            L"Minimap stage %d manifest parse failed: %S.",
            stageType,
            exception.what());
        return false;
    }
    return !g_gameplayMinimap.floors.empty();
}

int GameplayMinimap_FindNearestFloor(float height)
{
    int selectedFloor = -1;
    float nearestDistance = 1.0e30f;
    for (int index = 0;
         index <
             static_cast<int>(
                 g_gameplayMinimap.floors.size());
         ++index)
    {
        const float distance = std::abs(
            height -
            g_gameplayMinimap.floors[
                static_cast<size_t>(index)].height);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            selectedFloor = index;
        }
    }
    return selectedFloor;
}

void GameplayMinimap_SetAnchoredRect(
    Il2CppObject* rect,
    float anchorX,
    float anchorY,
    float pivotX,
    float pivotY,
    float width,
    float height,
    float positionX,
    float positionY)
{
    if (rect == nullptr)
    {
        return;
    }
    auto* rectClass = FindClass("UnityEngine", "RectTransform");
    float anchorMin[] = { anchorX, anchorY };
    float anchorMax[] = { anchorX, anchorY };
    float pivot[] = { pivotX, pivotY };
    float size[] = { width, height };
    float position[] = { positionX, positionY };
    void* anchorMinParameters[] = { anchorMin };
    void* anchorMaxParameters[] = { anchorMax };
    void* pivotParameters[] = { pivot };
    void* sizeParameters[] = { size };
    void* positionParameters[] = { position };
    Invoke(
        FindMethod(rectClass, "set_anchorMin", 1),
        rect,
        anchorMinParameters);
    Invoke(
        FindMethod(rectClass, "set_anchorMax", 1),
        rect,
        anchorMaxParameters);
    Invoke(
        FindMethod(rectClass, "set_pivot", 1),
        rect,
        pivotParameters);
    Invoke(
        FindMethod(rectClass, "set_sizeDelta", 1),
        rect,
        sizeParameters);
    Invoke(
        FindMethod(rectClass, "set_anchoredPosition", 1),
        rect,
        positionParameters);
}

void GameplayMinimap_SetImageSprite(
    Il2CppObject* image,
    Il2CppObject* sprite)
{
    if (image == nullptr)
    {
        return;
    }
    void* parameters[] = { sprite };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "Image"),
            "set_sprite",
            1),
        image,
        parameters);
}

Il2CppObject* GameplayMinimap_GetPointSprite(
    CheckpointMapPointKind kind)
{
    switch (kind)
    {
    case CheckpointMapPointKind::Extract:
    case CheckpointMapPointKind::Transfer:
        return g_gameplayMinimap.squareMarkerSprite;
    case CheckpointMapPointKind::Task:
        return g_gameplayMinimap.starMarkerSprite;
    case CheckpointMapPointKind::Supply:
    default:
        return g_gameplayMinimap.markerSprite;
    }
}

Il2CppObject* GameplayMinimap_CreateSprite(
    Il2CppObject* texture,
    float width,
    float height,
    float pivotX,
    float pivotY)
{
    if (texture == nullptr)
    {
        return nullptr;
    }
    auto* spriteClass = FindClass("UnityEngine", "Sprite");
    auto* create = spriteClass != nullptr
        ? FindMethod(spriteClass, "Create", 3)
        : nullptr;
    if (create == nullptr)
    {
        return nullptr;
    }
    float rect[] = { 0.0f, 0.0f, width, height };
    float pivot[] = { pivotX, pivotY };
    void* parameters[] = { texture, rect, pivot };
    return Invoke(create, nullptr, parameters);
}

void GameplayMinimap_Build(Il2CppObject* parentTransform)
{
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayMinimap.root))
    {
        SetWorldGameObjectActive(g_gameplayMinimap.root, true);
        return;
    }
    GameplayMinimap_Reset();
    if (parentTransform == nullptr)
    {
        return;
    }

    const std::wstring mapRoot =
        GetEscapeFromManakaSubdirectory(L"Maps");
    auto* circleTexture = LoadScenePngTexture(
        mapRoot + L"\\minimap-circle.png");
    auto* circleSprite =
        GameplayMinimap_CreateSprite(
            circleTexture,
            256.0f,
            256.0f,
            0.5f,
            0.5f);
    auto* arrowTexture = LoadScenePngTexture(
        mapRoot + L"\\minimap-arrow-equilateral.png");
    auto* playerArrowTexture = LoadScenePngTexture(
        mapRoot + L"\\minimap-player-arrow.png");
    auto* markerTexture = LoadScenePngTexture(
        mapRoot + L"\\minimap-marker-dot.png");
    auto* arrowSprite = GameplayMinimap_CreateSprite(
        arrowTexture,
        64.0f,
        56.0f,
        0.5f,
        0.5f);
    auto* playerArrowSprite = GameplayMinimap_CreateSprite(
        playerArrowTexture,
        64.0f,
        64.0f,
        0.5f,
        0.5f);
    auto* markerSprite = GameplayMinimap_CreateSprite(
        markerTexture,
        64.0f,
        64.0f,
        0.5f,
        0.5f);
    auto* squareMarkerTexture = LoadScenePngTexture(
        mapRoot + L"\\minimap-marker-square.png");
    auto* starMarkerTexture = LoadScenePngTexture(
        mapRoot + L"\\minimap-marker-star.png");
    auto* squareMarkerSprite =
        GameplayMinimap_CreateSprite(
            squareMarkerTexture,
            64.0f,
            64.0f,
            0.5f,
            0.5f);
    auto* starMarkerSprite =
        GameplayMinimap_CreateSprite(
            starMarkerTexture,
            64.0f,
            64.0f,
            0.5f,
            0.5f);
    auto* imageClass = FindClass("UnityEngine.UI", "Image");
    auto* maskClass = FindClass("UnityEngine.UI", "Mask");
    if (circleSprite == nullptr ||
        arrowSprite == nullptr ||
        playerArrowSprite == nullptr ||
        markerSprite == nullptr ||
        squareMarkerSprite == nullptr ||
        starMarkerSprite == nullptr ||
        imageClass == nullptr ||
        maskClass == nullptr)
    {
        LogInfo(LogCategory_Ui, L"Ui", L"Minimap circular mask resources are unavailable.");
        return;
    }
    g_gameplayMinimap.markerSprite = markerSprite;
    g_gameplayMinimap.squareMarkerSprite =
        squareMarkerSprite;
    g_gameplayMinimap.starMarkerSprite =
        starMarkerSprite;
    g_gameplayMinimap.edgeArrowSprite = arrowSprite;

    g_gameplayMinimap.root = CreateSceneGameObject(
        "EscapeFromManakaMinimap",
        parentTransform);
    if (g_gameplayMinimap.root == nullptr)
    {
        return;
    }
    g_gameplayMinimap.rootRect = GetSceneComponentByClass(
        g_gameplayMinimap.root,
        "UnityEngine",
        "RectTransform");
    GameplayMinimap_SetAnchoredRect(
        g_gameplayMinimap.rootRect,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        kGameplayMinimapDiameter,
        kGameplayMinimapDiameter,
        kGameplayMinimapMarginLeft,
        -kGameplayMinimapMarginTop);

    auto* maskImage = AddSceneComponentToGameObject(
        g_gameplayMinimap.root,
        "UnityEngine.UI",
        "Image");
    void* spriteParameters[] = { circleSprite };
    Invoke(
        FindMethod(imageClass, "set_sprite", 1),
        maskImage,
        spriteParameters);
    SetSceneGraphicColor(maskImage, 0.06f, 0.08f, 0.09f, 0.95f);
    bool raycastTarget = false;
    void* raycastParameters[] = { &raycastTarget };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "Graphic"),
            "set_raycastTarget",
            1),
        maskImage,
        raycastParameters);
    auto* mask = AddSceneComponentToGameObject(
        g_gameplayMinimap.root,
        "UnityEngine.UI",
        "Mask");
    bool showMaskGraphic = true;
    void* showMaskParameters[] = { &showMaskGraphic };
    Invoke(
        FindMethod(maskClass, "set_showMaskGraphic", 1),
        mask,
        showMaskParameters);

    g_gameplayMinimap.rotationContainer = CreateSceneGameObject(
        "EscapeFromManakaMinimapRotation",
        g_gameplayMinimap.rootRect);
    auto* rotationRect = GetSceneComponentByClass(
        g_gameplayMinimap.rotationContainer,
        "UnityEngine",
        "RectTransform");
    GameplayMinimap_SetAnchoredRect(
        rotationRect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        kGameplayMinimapDiameter,
        kGameplayMinimapDiameter,
        0.0f,
        0.0f);

    auto* mapObject = CreateSceneGameObject(
        "EscapeFromManakaMinimapMap",
        rotationRect);
    g_gameplayMinimap.mapRect = GetSceneComponentByClass(
        mapObject,
        "UnityEngine",
        "RectTransform");
    GameplayMinimap_SetAnchoredRect(
        g_gameplayMinimap.mapRect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        kGameplayMinimapDiameter,
        kGameplayMinimapDiameter,
        0.0f,
        0.0f);
    g_gameplayMinimap.rawImage = AddSceneComponentToGameObject(
        mapObject,
        "UnityEngine.UI",
        "RawImage");
    SetSceneGraphicColor(
        g_gameplayMinimap.rawImage,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "Graphic"),
            "set_raycastTarget",
            1),
        g_gameplayMinimap.rawImage,
        raycastParameters);

    for (int index = 0;
         index < kGameplayMapMarkerCapacity;
         ++index)
    {
        char name[64]{};
        std::snprintf(
            name,
            sizeof(name),
            "EscapeFromManakaMinimapPoint%02d",
            index);
        auto& marker =
            g_gameplayMinimapPointMarkers[index];
        marker = GameplayMinimapPointMarker{};
        marker.marker.gameObject =
            CreateSceneGameObject(
                name,
                g_gameplayMinimap.rootRect);
        marker.marker.rect =
            GetSceneComponentByClass(
                marker.marker.gameObject,
                "UnityEngine",
                "RectTransform");
        marker.marker.image =
            AddSceneComponentToGameObject(
                marker.marker.gameObject,
                "UnityEngine.UI",
                "Image");
        GameplayMinimap_SetImageSprite(
            marker.marker.image,
            markerSprite);
        SetSceneGraphicColor(
            marker.marker.image,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        Invoke(
            FindMethod(
                FindClass("UnityEngine.UI", "Graphic"),
                "set_raycastTarget",
                1),
            marker.marker.image,
            raycastParameters);
        GameplayMinimap_SetAnchoredRect(
            marker.marker.rect,
            0.5f,
            0.5f,
            0.5f,
            0.5f,
            13.0f,
            13.0f,
            0.0f,
            0.0f);
        SetWorldGameObjectActive(
            marker.marker.gameObject,
            false);
    }

    auto* playerArrow = CreateSceneGameObject(
        "EscapeFromManakaMinimapPlayerArrow",
        g_gameplayMinimap.rootRect);
    auto* playerArrowRect = GetSceneComponentByClass(
        playerArrow,
        "UnityEngine",
        "RectTransform");
    GameplayMinimap_SetAnchoredRect(
        playerArrowRect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        20.0f,
        20.0f,
        0.0f,
        0.0f);
    auto* playerArrowImage = AddSceneComponentToGameObject(
        playerArrow,
        "UnityEngine.UI",
        "Image");
    void* arrowSpriteParameters[] = { playerArrowSprite };
    Invoke(
        FindMethod(imageClass, "set_sprite", 1),
        playerArrowImage,
        arrowSpriteParameters);
    SetSceneGraphicColor(
        playerArrowImage,
        1.0f,
        0.94f,
        0.72f,
        1.0f);

    Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "SetAsLastSibling",
            0),
        GetWorldTransform(g_gameplayMinimap.root),
        nullptr);
    LogInfo(LogCategory_Ui, L"Ui", L"Escape gameplay circular minimap HUD created.");
}

void GameplayMinimap_UpdatePointMarkers(
    float playerX,
    float playerY,
    float playerZ,
    float yaw)
{
    if (g_gameplayMinimap.floorIndex < 0 ||
        g_gameplayMinimap.floorIndex >=
            static_cast<int>(
                g_gameplayMinimap.floors.size()))
    {
        return;
    }
    for (auto& marker :
         g_gameplayMinimapPointMarkers)
    {
        SetWorldGameObjectActive(
            marker.marker.gameObject,
            false);
        marker.runtimeIndex = -1;
    }

    int trackedRuntime =
        GameplayMapTracking_GetRuntimeIndex();
    CheckpointMapPoint trackedPoint;
    if (trackedRuntime >= 0 &&
        !CheckpointSystem_GetMapPoint(
            trackedRuntime,
            trackedPoint))
    {
        GameplayMapTracking_Clear();
        trackedRuntime = -1;
    }
    CheckpointSystem_CollectMapPoints(
        g_gameplayMinimap.points);
    const auto& floor =
        g_gameplayMinimap.floors[
            static_cast<size_t>(
                g_gameplayMinimap.floorIndex)];
    const float radians =
        yaw * 0.017453292519943295f;
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    const float pixelsPerMeter =
        kGameplayMinimapDiameter /
        (kGameplayMinimapRadiusMeters * 2.0f);
    const float edgeRadius =
        kGameplayMinimapDiameter * 0.5f - 9.0f;
    int markerIndex = 0;

    auto showMarker = [&](const CheckpointMapPoint& point)
    {
        if (markerIndex >= kGameplayMapMarkerCapacity)
        {
            return;
        }
        const int pointFloor =
            GameplayMinimap_FindNearestFloor(point.y);
        const bool sameFloor =
            pointFloor ==
            g_gameplayMinimap.floorIndex;
        const float dx = point.x - playerX;
        const float dz = point.z - playerZ;
        const float horizontalDistance =
            std::sqrt(dx * dx + dz * dz);
        const bool insideRange =
            sameFloor &&
            horizontalDistance <=
                kGameplayMinimapRadiusMeters;
        const bool isTracked =
            trackedRuntime >= 0 &&
            point.runtimeIndex == trackedRuntime;
        if (!insideRange && !isTracked)
        {
            return;
        }
        auto& marker =
            g_gameplayMinimapPointMarkers[
                markerIndex++];

        const float rawX = dx * pixelsPerMeter;
        const float rawY = dz * pixelsPerMeter;
        float displayX =
            rawX * cosine - rawY * sine;
        float displayY =
            rawX * sine + rawY * cosine;
        bool edge = false;
        if (!insideRange)
        {
            edge = true;
            const float displayLength =
                std::sqrt(
                    displayX * displayX +
                    displayY * displayY);
            if (displayLength <= 0.001f)
            {
                displayX = 0.0f;
                displayY = 1.0f;
            }
            else
            {
                displayX =
                    displayX / displayLength *
                    edgeRadius;
                displayY =
                    displayY / displayLength *
                    edgeRadius;
            }
        }

        marker.runtimeIndex =
            point.runtimeIndex;
        marker.usingEdge = edge;
        GameplayMinimap_SetImageSprite(
            marker.marker.image,
            edge
                ? g_gameplayMinimap.edgeArrowSprite
                : GameplayMinimap_GetPointSprite(point.kind));
        GameplayMinimap_SetAnchoredRect(
            marker.marker.rect,
            0.5f,
            0.5f,
            0.5f,
            0.5f,
            edge ? 16.0f : 13.0f,
            edge ? 14.0f : 13.0f,
            displayX,
            displayY);
        const float edgeAngle =
            displayX * displayX +
                    displayY * displayY >
                0.001f
                ? std::atan2(-displayX, displayY) *
                    57.29577951308232f
                : 0.0f;
        SetTransformLocalEuler(
            GetWorldTransform(
                marker.marker.gameObject),
            0.0f,
            0.0f,
            edge ? edgeAngle : 0.0f);

        float red = point.color[0];
        float green = point.color[1];
        float blue = point.color[2];
        if (edge)
        {
            const float levelDelta =
                point.y - floor.height;
            if (levelDelta > 1.0f)
            {
                red = 0.18f;
                green = 0.90f;
                blue = 0.45f;
            }
            else if (levelDelta < -1.0f)
            {
                red = 0.96f;
                green = 0.24f;
                blue = 0.24f;
            }
        }
        SetSceneGraphicColor(
            marker.marker.image,
            red,
            green,
            blue,
            isTracked ? 1.0f : 0.96f);
        SetWorldGameObjectActive(
            marker.marker.gameObject,
            true);
    };

    if (trackedRuntime >= 0)
    {
        showMarker(trackedPoint);
        return;
    }
    for (const auto& point :
         g_gameplayMinimap.points)
    {
        showMarker(point);
    }
}

void GameplayWorldMap_SetOpen(bool open)
{
    if (open &&
        !ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.root))
    {
        g_gameplayWorldMapToggleRequested = false;
        LogInfo(LogCategory_Ui, L"Ui", L"World map could not open because its UI is unavailable.");
        return;
    }
    if (g_gameplayWorldMapOpen.load() == open)
    {
        if (ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.root))
        {
            SetWorldGameObjectActive(g_gameplayWorldMap.root, open);
        }
        return;
    }

    g_gameplayWorldMapOpen = open;
    g_gameplayWorldMapToggleRequested = false;
    g_gameplayWorldMapPointerDown = false;
    g_gameplayWorldMapWheelDelta = 0;
    g_gameplayWorldMap.pointerWasDown = false;
    g_gameplayWorldMap.dragging = false;
    g_gameplayWorldMap.dropdownOpen = false;
    g_gameplayWorldMap.pressedControl =
        kGameplayWorldMapControlNone;
    g_gameplayWorldMap.hoveredMarkerIndex = -1;
    g_gameplayWorldMap.pressedMarkerRuntimeIndex = -1;
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.root))
    {
        SetWorldGameObjectActive(g_gameplayWorldMap.root, open);
        if (open)
        {
            BringSceneObjectToFront(g_gameplayWorldMap.root);
        }
    }
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayMinimap.root))
    {
        SetWorldGameObjectActive(g_gameplayMinimap.root, !open);
    }
    if (open)
    {
        g_gameplayWorldMap.zoom = 1.0f;
        g_gameplayWorldMap.centerX = 0.0f;
        g_gameplayWorldMap.centerY = 0.0f;
        g_gameplayWorldMap.floorIndex = std::clamp(
            g_gameplayMinimap.floorIndex,
            0,
            std::max(
                0,
                static_cast<int>(
                    g_gameplayMinimap.floors.size()) - 1));
        g_gameplayWorldMap.stageType =
            g_gameplayMinimap.stageType;
        ExtendedMode_ForceCursor();
        LogInfo(LogCategory_Ui, L"Ui", L"Escape world floor map opened as a modal UI.");
    }
    else
    {
        g_gameplayWorldMap.dropdownOpen = false;
        g_gameplayWorldMap.pressedControl =
            kGameplayWorldMapControlNone;
        GameplayWorldMap_ReleaseTexture();
        ExtendedMode_ForceGameplayCursor();
        LogInfo(LogCategory_Ui, L"Ui", L"Escape world floor map closed; the gameplay view was left unchanged.");
    }
}

void GameplayWorldMap_Build(Il2CppObject* parentTransform)
{
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.root))
    {
        SetWorldGameObjectActive(g_gameplayWorldMap.root, false);
        return;
    }
    g_gameplayWorldMap = GameplayWorldMapState{};
    g_gameplayWorldMapOpen = false;
    g_gameplayWorldMapToggleRequested = false;
    GameplayWorldMap_ResetPhysicalInput();
    if (parentTransform == nullptr)
    {
        return;
    }

    auto panel = CreateScenePanel(
        "EscapeFromManakaWorldMap",
        parentTransform,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.012f,
        0.016f,
        0.020f,
        0.94f);
    g_gameplayWorldMap.root = panel.gameObject;
    g_gameplayWorldMap.rootRect = panel.rect;
    if (g_gameplayWorldMap.root == nullptr)
    {
        return;
    }

    auto* mapObject = CreateSceneGameObject(
        "EscapeFromManakaWorldMapFloor",
        g_gameplayWorldMap.rootRect);
    g_gameplayWorldMap.mapRect = GetSceneComponentByClass(
        mapObject,
        "UnityEngine",
        "RectTransform");
    g_gameplayWorldMap.rawImage = AddSceneComponentToGameObject(
        mapObject,
        "UnityEngine.UI",
        "RawImage");
    SetSceneGraphicColor(
        g_gameplayWorldMap.rawImage,
        1.0f,
        1.0f,
        1.0f,
        1.0f);
    bool raycastTarget = false;
    void* raycastParameters[] = { &raycastTarget };
    Invoke(
        FindMethod(
            FindClass("UnityEngine.UI", "Graphic"),
            "set_raycastTarget",
            1),
        g_gameplayWorldMap.rawImage,
        raycastParameters);

    for (int index = 0;
         index < kGameplayMapMarkerCapacity;
         ++index)
    {
        char name[64]{};
        std::snprintf(
            name,
            sizeof(name),
            "EscapeFromManakaWorldMapPoint%02d",
            index);
        auto& marker =
            g_gameplayWorldMapPointMarkers[index];
        marker = GameplayWorldMapPointMarker{};
        marker.marker.gameObject =
            CreateSceneGameObject(
                name,
                g_gameplayWorldMap.rootRect);
        marker.marker.rect =
            GetSceneComponentByClass(
                marker.marker.gameObject,
                "UnityEngine",
                "RectTransform");
        marker.marker.image =
            AddSceneComponentToGameObject(
                marker.marker.gameObject,
                "UnityEngine.UI",
                "Image");
        GameplayMinimap_SetImageSprite(
            marker.marker.image,
            g_gameplayMinimap.markerSprite);
        SetSceneGraphicColor(
            marker.marker.image,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        Invoke(
            FindMethod(
                FindClass("UnityEngine.UI", "Graphic"),
                "set_raycastTarget",
                1),
            marker.marker.image,
            raycastParameters);
        GameplayMinimap_SetAnchoredRect(
            marker.marker.rect,
            0.5f,
            0.5f,
            0.5f,
            0.5f,
            kGameplayWorldMapMarkerSize,
            kGameplayWorldMapMarkerSize,
            0.0f,
            0.0f);
        SetWorldGameObjectActive(
            marker.marker.gameObject,
            false);
    }

    g_gameplayWorldMap.floorLabel = CreateSceneTextElement(
        "EscapeFromManakaWorldMapFloorLabel",
        g_gameplayWorldMap.rootRect,
        "地图",
        0.035f,
        0.925f,
        0.52f,
        0.976f,
        25.0f,
        257,
        0.98f,
        0.94f,
        0.82f,
        1.0f).text;
    g_gameplayWorldMap.usageLabel = CreateSceneTextElement(
        "EscapeFromManakaWorldMapUsage",
        g_gameplayWorldMap.rootRect,
        "M 关闭   ·   右上切换楼层   ·   滚轮缩放   ·   悬停地图点放大   ·   单击追踪或取消追踪   ·   按住空白处拖动",
        0.035f,
        0.020f,
        0.88f,
        0.068f,
        16.0f,
        257,
        0.76f,
        0.81f,
        0.84f,
        1.0f).text;

    g_gameplayWorldMap.dropdownButton = CreateScenePanel(
        "EscapeFromManakaWorldMapFloorDropdown",
        g_gameplayWorldMap.rootRect,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.10f,
        0.14f,
        0.17f,
        0.96f);
    g_gameplayWorldMap.dropdownLabel = CreateSceneTextElement(
        "Label",
        g_gameplayWorldMap.dropdownButton.rect,
        "楼层   ▼",
        0.04f,
        0.08f,
        0.96f,
        0.92f,
        18.0f,
        514,
        0.96f,
        0.91f,
        0.76f,
        1.0f);

    g_gameplayWorldMap.upButton = CreateScenePanel(
        "EscapeFromManakaWorldMapFloorUp",
        g_gameplayWorldMap.rootRect,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.10f,
        0.14f,
        0.17f,
        0.96f);
    g_gameplayWorldMap.upLabel = CreateSceneTextElement(
        "Label",
        g_gameplayWorldMap.upButton.rect,
        "▲",
        0.04f,
        0.04f,
        0.96f,
        0.96f,
        28.0f,
        514,
        0.96f,
        0.91f,
        0.76f,
        1.0f);

    g_gameplayWorldMap.downButton = CreateScenePanel(
        "EscapeFromManakaWorldMapFloorDown",
        g_gameplayWorldMap.rootRect,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.10f,
        0.14f,
        0.17f,
        0.96f);
    g_gameplayWorldMap.downLabel = CreateSceneTextElement(
        "Label",
        g_gameplayWorldMap.downButton.rect,
        "▼",
        0.04f,
        0.04f,
        0.96f,
        0.96f,
        28.0f,
        514,
        0.96f,
        0.91f,
        0.76f,
        1.0f);

    for (int index = 0;
         index < kGameplayWorldMapMaxFloorRows;
         ++index)
    {
        char name[80]{};
        std::snprintf(
            name,
            sizeof(name),
            "EscapeFromManakaWorldMapFloorRow%02d",
            index);
        auto& row = g_gameplayWorldMap.floorRows[index];
        row.background = CreateScenePanel(
            name,
            g_gameplayWorldMap.rootRect,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.09f,
            0.13f,
            0.16f,
            0.98f);
        row.label = CreateSceneTextElement(
            "Label",
            row.background.rect,
            "",
            0.06f,
            0.08f,
            0.94f,
            0.92f,
            17.0f,
            514,
            0.91f,
            0.88f,
            0.76f,
            1.0f);
        SetWorldGameObjectActive(
            row.background.gameObject,
            false);
    }
    SetWorldGameObjectActive(
        g_gameplayWorldMap.dropdownButton.gameObject,
        false);
    SetWorldGameObjectActive(
        g_gameplayWorldMap.upButton.gameObject,
        false);
    SetWorldGameObjectActive(
        g_gameplayWorldMap.downButton.gameObject,
        false);
    SetWorldGameObjectActive(g_gameplayWorldMap.root, false);
    BringSceneObjectToFront(g_gameplayWorldMap.root);
    LogInfo(LogCategory_Ui, L"Ui", L"Escape modal world floor map UI created.");
}

void GameplayWorldMap_ReleaseTexture()
{
    if (g_gameplayWorldMap.texturePath.empty())
    {
        g_gameplayWorldMap.texture = nullptr;
        return;
    }
    if (g_gameplayWorldMap.texturePath !=
        g_gameplayMinimap.texturePath)
    {
        ReleaseScenePngTexture(
            g_gameplayWorldMap.texturePath);
    }
    g_gameplayWorldMap.texture = nullptr;
    g_gameplayWorldMap.texturePath.clear();
}

void GameplayWorldMap_SetDisplayedFloor(int floorIndex)
{
    const int floorCount = static_cast<int>(
        g_gameplayMinimap.floors.size());
    if (floorCount <= 0)
    {
        return;
    }
    floorIndex = std::clamp(floorIndex, 0, floorCount - 1);
    const auto& floor =
        g_gameplayMinimap.floors[
            static_cast<size_t>(floorIndex)];
    if (g_gameplayWorldMap.floorIndex == floorIndex &&
        g_gameplayWorldMap.texturePath == floor.imagePath)
    {
        return;
    }

    auto* texture = LoadScenePngTexture(floor.imagePath);
    if (texture == nullptr)
    {
        LogInfo(LogCategory_Ui, L"Ui", 
            L"World map floor image is unavailable: %s",
            floor.imagePath.c_str());
        return;
    }
    if (!g_gameplayWorldMap.texturePath.empty() &&
        g_gameplayWorldMap.texturePath != floor.imagePath &&
        g_gameplayWorldMap.texturePath !=
            g_gameplayMinimap.texturePath)
    {
        ReleaseScenePngTexture(
            g_gameplayWorldMap.texturePath);
    }
    g_gameplayWorldMap.floorIndex = floorIndex;
    g_gameplayWorldMap.texture = texture;
    g_gameplayWorldMap.texturePath = floor.imagePath;
    SetSceneRawImageTexture(
        g_gameplayWorldMap.rawImage,
        texture);
    g_gameplayWorldMap.zoom = 1.0f;
    g_gameplayWorldMap.centerX = 0.0f;
    g_gameplayWorldMap.centerY = 0.0f;
    LogInfo(
        LogCategory_Ui,
        L"WorldMapFloor",
        L"World map floor displayed: index=%d image=%s.",
        floorIndex,
        floor.imagePath.c_str());

    char label[128]{};
    std::snprintf(
        label,
        sizeof(label),
        "%s   ·   %dF",
        GameplayMinimap_SceneLabelUtf8(
            g_gameplayMinimap.stageType),
        floorIndex + 1);
    SetSceneText(
        g_gameplayWorldMap.floorLabel,
        label,
        25.0f,
        257,
        0.98f,
        0.94f,
        0.82f,
        1.0f);
}

void GameplayWorldMap_Update()
{
    if (!g_gameplayWorldMapOpen.load())
    {
        return;
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.root) ||
        !ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.mapRect))
    {
        g_gameplayWorldMapOpen = false;
        g_gameplayWorldMapToggleRequested = false;
        return;
    }

    SetWorldGameObjectActive(g_gameplayWorldMap.root, true);
    ExtendedMode_ForceCursor();

    if (g_gameplayWorldMap.stageType !=
        g_gameplayMinimap.stageType)
    {
        GameplayWorldMap_ReleaseTexture();
        g_gameplayWorldMap.stageType =
            g_gameplayMinimap.stageType;
        g_gameplayWorldMap.floorIndex = -1;
        g_gameplayWorldMap.dropdownOpen = false;
        g_gameplayWorldMap.pressedControl =
            kGameplayWorldMapControlNone;
    }
    int worldFloorCount = static_cast<int>(
        g_gameplayMinimap.floors.size());
    if (worldFloorCount > 0)
    {
        const int initialFloor = std::clamp(
            g_gameplayWorldMap.floorIndex >= 0
                ? g_gameplayWorldMap.floorIndex
                : g_gameplayMinimap.floorIndex,
            0,
            worldFloorCount - 1);
        GameplayWorldMap_SetDisplayedFloor(initialFloor);
    }
    if (g_gameplayWorldMap.floorIndex < 0 ||
        g_gameplayWorldMap.floorIndex >= worldFloorCount)
    {
        g_gameplayWorldMapOpen = false;
        g_gameplayWorldMapToggleRequested = false;
        return;
    }

    auto* rectValue = Invoke(
        FindMethod(
            FindClass("UnityEngine", "RectTransform"),
            "get_rect",
            0),
        g_gameplayWorldMap.rootRect,
        nullptr);
    float canvasWidth = 1920.0f;
    float canvasHeight = 1080.0f;
    if (rectValue != nullptr && il2cpp_object_unbox != nullptr)
    {
        const auto* rect = static_cast<float*>(
            il2cpp_object_unbox(rectValue));
        if (rect[2] > 1.0f && rect[3] > 1.0f)
        {
            canvasWidth = rect[2];
            canvasHeight = rect[3];
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
    const float clientWidth =
        static_cast<float>(clientRect.right - clientRect.left);
    const float clientHeight =
        static_cast<float>(clientRect.bottom - clientRect.top);
    if (clientWidth <= 0.0f || clientHeight <= 0.0f)
    {
        return;
    }
    const float pointerX =
        (static_cast<float>(cursor.x) / clientWidth - 0.5f) *
        canvasWidth;
    const float pointerY =
        (0.5f - static_cast<float>(cursor.y) / clientHeight) *
        canvasHeight;

    const int floorRowCount = std::min(
        worldFloorCount,
        kGameplayWorldMapMaxFloorRows);
    const bool hasFloorSwitcher = floorRowCount > 1;
    constexpr float controlMargin = 24.0f;
    constexpr float dropdownHeight = 42.0f;
    const float dropdownWidth = std::clamp(
        canvasWidth * 0.14f,
        190.0f,
        260.0f);
    const float dropdownCenterX =
        canvasWidth * 0.5f -
        controlMargin -
        dropdownWidth * 0.5f;
    const float dropdownCenterY =
        canvasHeight * 0.5f -
        controlMargin -
        dropdownHeight * 0.5f;
    const float rowHeight = std::clamp(
        canvasHeight * 0.036f,
        24.0f,
        34.0f);
    constexpr float rowGap = 2.0f;
    const float dropdownBottom =
        dropdownCenterY - dropdownHeight * 0.5f;
    const float rowTop = dropdownBottom - 6.0f;

    GameplayMinimap_SetAnchoredRect(
        g_gameplayWorldMap.dropdownButton.rect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        dropdownWidth,
        dropdownHeight,
        dropdownCenterX,
        dropdownCenterY);

    constexpr float sideButtonSize = 64.0f;
    constexpr float sideButtonGap = 12.0f;
    const float sideButtonX =
        canvasWidth * 0.5f -
        controlMargin -
        sideButtonSize * 0.5f;
    const float upButtonY = -52.0f;
    const float downButtonY =
        upButtonY - sideButtonSize - sideButtonGap;
    const bool showUpButton =
        hasFloorSwitcher &&
        g_gameplayWorldMap.floorIndex <
            floorRowCount - 1;
    const bool showDownButton =
        hasFloorSwitcher &&
        g_gameplayWorldMap.floorIndex > 0;
    GameplayMinimap_SetAnchoredRect(
        g_gameplayWorldMap.upButton.rect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        sideButtonSize,
        sideButtonSize,
        sideButtonX,
        upButtonY);
    GameplayMinimap_SetAnchoredRect(
        g_gameplayWorldMap.downButton.rect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        sideButtonSize,
        sideButtonSize,
        sideButtonX,
        downButtonY);

    auto contains = [](
        float x,
        float y,
        float centerX,
        float centerY,
        float width,
        float height)
    {
        return x >= centerX - width * 0.5f &&
               x <= centerX + width * 0.5f &&
               y >= centerY - height * 0.5f &&
               y <= centerY + height * 0.5f;
    };
    const bool dropdownHovered = contains(
        pointerX,
        pointerY,
        dropdownCenterX,
        dropdownCenterY,
        dropdownWidth,
        dropdownHeight);
    const bool upHovered = showUpButton && contains(
        pointerX,
        pointerY,
        sideButtonX,
        upButtonY,
        sideButtonSize,
        sideButtonSize);
    const bool downHovered = showDownButton && contains(
        pointerX,
        pointerY,
        sideButtonX,
        downButtonY,
        sideButtonSize,
        sideButtonSize);

    float rowMinY[kGameplayWorldMapMaxFloorRows]{};
    float rowMaxY[kGameplayWorldMapMaxFloorRows]{};
    int hoveredFloorRow = -1;
    for (int rowIndex = 0;
         rowIndex < kGameplayWorldMapMaxFloorRows;
         ++rowIndex)
    {
        auto& row = g_gameplayWorldMap.floorRows[rowIndex];
        const bool hasRow = rowIndex < floorRowCount;
        const bool visible =
            hasRow && g_gameplayWorldMap.dropdownOpen;
        const float centerY =
            rowTop -
            rowHeight * 0.5f -
            static_cast<float>(rowIndex) *
                (rowHeight + rowGap);
        rowMinY[rowIndex] = centerY - rowHeight * 0.5f;
        rowMaxY[rowIndex] = centerY + rowHeight * 0.5f;
        GameplayMinimap_SetAnchoredRect(
            row.background.rect,
            0.5f,
            0.5f,
            0.5f,
            0.5f,
            dropdownWidth,
            rowHeight,
            dropdownCenterX,
            centerY);
        SetWorldGameObjectActive(
            row.background.gameObject,
            visible);
        if (!hasRow)
        {
            continue;
        }
        const int floorForRow =
            floorRowCount - 1 - rowIndex;
        const bool hovered = visible && contains(
            pointerX,
            pointerY,
            dropdownCenterX,
            centerY,
            dropdownWidth,
            rowHeight);
        if (hovered)
        {
            hoveredFloorRow = rowIndex;
        }
        const bool selected =
            floorForRow == g_gameplayWorldMap.floorIndex;
        char rowLabel[32]{};
        std::snprintf(
            rowLabel,
            sizeof(rowLabel),
            "%dF",
            floorForRow + 1);
        SetSceneText(
            row.label.text,
            rowLabel,
            17.0f,
            514,
            selected
                ? 0.98f
                : (hovered ? 0.94f : 0.82f),
            selected
                ? 0.91f
                : (hovered ? 0.90f : 0.84f),
            selected
                ? 0.72f
                : (hovered ? 0.76f : 0.74f),
            1.0f);
        SetSceneGraphicColor(
            row.background.image,
            selected ? 0.24f : (hovered ? 0.18f : 0.09f),
            selected ? 0.29f : (hovered ? 0.22f : 0.13f),
            selected ? 0.26f : (hovered ? 0.22f : 0.16f),
            0.98f);
    }

    const bool pointerInsideDropdownList =
        g_gameplayWorldMap.dropdownOpen &&
        pointerX >= dropdownCenterX - dropdownWidth * 0.5f &&
        pointerX <= dropdownCenterX + dropdownWidth * 0.5f &&
        pointerY <= rowTop &&
        pointerY >=
            rowTop -
            static_cast<float>(floorRowCount) *
                (rowHeight + rowGap);
    const bool pointerOverControls =
        (hasFloorSwitcher && dropdownHovered) ||
        upHovered ||
        downHovered ||
        pointerInsideDropdownList;

    SetWorldGameObjectActive(
        g_gameplayWorldMap.dropdownButton.gameObject,
        hasFloorSwitcher);
    SetWorldGameObjectActive(
        g_gameplayWorldMap.upButton.gameObject,
        showUpButton);
    SetWorldGameObjectActive(
        g_gameplayWorldMap.downButton.gameObject,
        showDownButton);
    if (hasFloorSwitcher)
    {
        char dropdownText[48]{};
        std::snprintf(
            dropdownText,
            sizeof(dropdownText),
            "%dF   %s",
            g_gameplayWorldMap.floorIndex + 1,
            g_gameplayWorldMap.dropdownOpen ? "▲" : "▼");
        SetSceneText(
            g_gameplayWorldMap.dropdownLabel.text,
            dropdownText,
            18.0f,
            514,
            0.96f,
            0.91f,
            0.76f,
            1.0f);
        SetSceneGraphicColor(
            g_gameplayWorldMap.dropdownButton.image,
            dropdownHovered ? 0.18f : 0.10f,
            dropdownHovered ? 0.23f : 0.14f,
            dropdownHovered ? 0.23f : 0.17f,
            0.98f);
    }
    if (showUpButton)
    {
        SetSceneGraphicColor(
            g_gameplayWorldMap.upButton.image,
            upHovered ? 0.20f : 0.10f,
            upHovered ? 0.25f : 0.14f,
            upHovered ? 0.24f : 0.17f,
            0.98f);
    }
    if (showDownButton)
    {
        SetSceneGraphicColor(
            g_gameplayWorldMap.downButton.image,
            downHovered ? 0.20f : 0.10f,
            downHovered ? 0.25f : 0.14f,
            downHovered ? 0.24f : 0.17f,
            0.98f);
    }

    const int wheelDelta =
        g_gameplayWorldMapWheelDelta.exchange(0);
    if (wheelDelta != 0 && !pointerOverControls)
    {
        const float previousZoom =
            g_gameplayWorldMap.zoom;
        const float zoomFactor = std::pow(
            1.15f,
            static_cast<float>(wheelDelta) /
                static_cast<float>(WHEEL_DELTA));
        const float nextZoom = std::clamp(
            previousZoom * zoomFactor,
            kGameplayWorldMapMinZoom,
            kGameplayWorldMapMaxZoom);
        if (nextZoom != previousZoom)
        {
            const float ratio = nextZoom / previousZoom;
            g_gameplayWorldMap.centerX =
                pointerX -
                (pointerX - g_gameplayWorldMap.centerX) * ratio;
            g_gameplayWorldMap.centerY =
                pointerY -
                (pointerY - g_gameplayWorldMap.centerY) * ratio;
            g_gameplayWorldMap.zoom = nextZoom;
        }
    }

    g_gameplayWorldMap.fitSize =
        std::min(canvasWidth, canvasHeight) * 0.86f;
    float mapSize =
        g_gameplayWorldMap.fitSize *
        g_gameplayWorldMap.zoom;

    int trackedRuntime =
        GameplayMapTracking_GetRuntimeIndex();
    CheckpointMapPoint trackedPoint;
    if (trackedRuntime >= 0 &&
        !CheckpointSystem_GetMapPoint(
            trackedRuntime,
            trackedPoint))
    {
        GameplayMapTracking_Clear();
        trackedRuntime = -1;
    }

    for (auto& marker :
         g_gameplayWorldMapPointMarkers)
    {
        SetWorldGameObjectActive(
            marker.marker.gameObject,
            false);
        marker.runtimeIndex = -1;
    }
    g_gameplayWorldMap.points.clear();
    if (g_gameplayWorldMap.floorIndex >= 0 &&
        g_gameplayWorldMap.floorIndex <
            static_cast<int>(
                g_gameplayMinimap.floors.size()))
    {
        CheckpointSystem_CollectMapPoints(
            g_gameplayWorldMap.points);
        const auto iterator = std::remove_if(
            g_gameplayWorldMap.points.begin(),
            g_gameplayWorldMap.points.end(),
            [](const CheckpointMapPoint& point)
            {
                return GameplayMinimap_FindNearestFloor(
                           point.y) !=
                       g_gameplayWorldMap.floorIndex;
            });
        g_gameplayWorldMap.points.erase(
            iterator,
            g_gameplayWorldMap.points.end());
    }
    const auto& floor =
        g_gameplayMinimap.floors[
            static_cast<size_t>(
                g_gameplayWorldMap.floorIndex)];
    const float worldWidth =
        std::max(
            floor.maxX - floor.minX,
            0.01f);
    const float worldDepth =
        std::max(
            floor.maxZ - floor.minZ,
            0.01f);

    int markerCount = 0;
    int hoveredMarkerIndex = -1;
    float hoveredDistance =
        kGameplayWorldMapMarkerSize * 0.5f + 7.0f;
    for (const auto& point :
         g_gameplayWorldMap.points)
    {
        if (markerCount >= kGameplayMapMarkerCapacity)
        {
            break;
        }
        auto& marker =
            g_gameplayWorldMapPointMarkers[
                markerCount];
        marker.runtimeIndex = point.runtimeIndex;
        const float normalizedX =
            (point.x - floor.minX) / worldWidth;
        const float normalizedY =
            (point.z - floor.minZ) / worldDepth;
        const float markerX =
            (normalizedX - 0.5f) * mapSize;
        const float markerY =
            (normalizedY - 0.5f) * mapSize;
        const float screenX =
            g_gameplayWorldMap.centerX + markerX;
        const float screenY =
            g_gameplayWorldMap.centerY + markerY;
        const float distance = std::sqrt(
            (pointerX - screenX) *
                (pointerX - screenX) +
            (pointerY - screenY) *
                (pointerY - screenY));
        if (!pointerOverControls &&
            distance < hoveredDistance)
        {
            hoveredDistance = distance;
            hoveredMarkerIndex = markerCount;
        }
        ++markerCount;
    }
    g_gameplayWorldMap.hoveredMarkerIndex =
        hoveredMarkerIndex;

    const bool pointerDown =
        g_gameplayWorldMapPointerDown.load() ||
        (GameplayInput_ReadRawAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (pointerDown)
    {
        if (!g_gameplayWorldMap.pointerWasDown)
        {
            int pressedControl =
                kGameplayWorldMapControlNone;
            if (hasFloorSwitcher && dropdownHovered)
            {
                pressedControl =
                    kGameplayWorldMapControlDropdown;
            }
            else if (showUpButton && upHovered)
            {
                pressedControl =
                    kGameplayWorldMapControlUp;
            }
            else if (showDownButton && downHovered)
            {
                pressedControl =
                    kGameplayWorldMapControlDown;
            }
            else if (
                g_gameplayWorldMap.dropdownOpen &&
                hoveredFloorRow >= 0)
            {
                pressedControl =
                    kGameplayWorldMapControlRowBase +
                    hoveredFloorRow;
            }
            else if (g_gameplayWorldMap.dropdownOpen)
            {
                g_gameplayWorldMap.dropdownOpen = false;
                pressedControl =
                    kGameplayWorldMapControlConsumed;
            }
            g_gameplayWorldMap.pressedControl =
                pressedControl;

            if (pressedControl !=
                kGameplayWorldMapControlNone)
            {
                g_gameplayWorldMap.pressedMarkerRuntimeIndex = -1;
                g_gameplayWorldMap.dragging = false;
            }
            else if (hoveredMarkerIndex >= 0)
            {
                g_gameplayWorldMap.pressedMarkerRuntimeIndex =
                    g_gameplayWorldMapPointMarkers[
                        hoveredMarkerIndex].runtimeIndex;
                g_gameplayWorldMap.dragging = false;
            }
            else
            {
                g_gameplayWorldMap.pressedMarkerRuntimeIndex = -1;
                g_gameplayWorldMap.dragging = true;
                g_gameplayWorldMap.lastPointerX = pointerX;
                g_gameplayWorldMap.lastPointerY = pointerY;
            }
        }
        else if (g_gameplayWorldMap.dragging)
        {
            g_gameplayWorldMap.centerX +=
                pointerX - g_gameplayWorldMap.lastPointerX;
            g_gameplayWorldMap.centerY +=
                pointerY - g_gameplayWorldMap.lastPointerY;
            g_gameplayWorldMap.lastPointerX = pointerX;
            g_gameplayWorldMap.lastPointerY = pointerY;
        }
        g_gameplayWorldMap.pointerWasDown = true;
    }
    else
    {
        const int pressedControl =
            g_gameplayWorldMap.pressedControl;
        if (g_gameplayWorldMap.pointerWasDown &&
            pressedControl != kGameplayWorldMapControlNone)
        {
            if (pressedControl ==
                    kGameplayWorldMapControlDropdown &&
                hasFloorSwitcher &&
                dropdownHovered)
            {
                g_gameplayWorldMap.dropdownOpen =
                    !g_gameplayWorldMap.dropdownOpen;
            }
            else if (pressedControl ==
                         kGameplayWorldMapControlUp &&
                     showUpButton &&
                     upHovered)
            {
                GameplayWorldMap_SetDisplayedFloor(
                    g_gameplayWorldMap.floorIndex + 1);
                g_gameplayWorldMap.dropdownOpen = false;
            }
            else if (pressedControl ==
                         kGameplayWorldMapControlDown &&
                     showDownButton &&
                     downHovered)
            {
                GameplayWorldMap_SetDisplayedFloor(
                    g_gameplayWorldMap.floorIndex - 1);
                g_gameplayWorldMap.dropdownOpen = false;
            }
            else if (
                pressedControl >=
                    kGameplayWorldMapControlRowBase &&
                hoveredFloorRow ==
                    pressedControl -
                        kGameplayWorldMapControlRowBase)
            {
                const int floorForRow =
                    floorRowCount - 1 - hoveredFloorRow;
                GameplayWorldMap_SetDisplayedFloor(
                    floorForRow);
                g_gameplayWorldMap.dropdownOpen = false;
            }
        }
        else if (g_gameplayWorldMap.pointerWasDown &&
            g_gameplayWorldMap.pressedMarkerRuntimeIndex >= 0 &&
            hoveredMarkerIndex >= 0 &&
            g_gameplayWorldMapPointMarkers[
                hoveredMarkerIndex].runtimeIndex ==
                g_gameplayWorldMap.pressedMarkerRuntimeIndex)
        {
            GameplayMapTracking_Toggle(
                g_gameplayWorldMap.pressedMarkerRuntimeIndex);
        }
        g_gameplayWorldMap.pressedControl =
            kGameplayWorldMapControlNone;
        g_gameplayWorldMap.pressedMarkerRuntimeIndex = -1;
        g_gameplayWorldMap.pointerWasDown = false;
        g_gameplayWorldMap.dragging = false;
    }

    mapSize =
        g_gameplayWorldMap.fitSize *
        g_gameplayWorldMap.zoom;
    const float dragLimitX =
        std::max(0.0f, mapSize * 0.5f - 64.0f);
    const float dragLimitY =
        std::max(0.0f, mapSize * 0.5f - 64.0f);
    g_gameplayWorldMap.centerX = std::clamp(
        g_gameplayWorldMap.centerX,
        -dragLimitX,
        dragLimitX);
    g_gameplayWorldMap.centerY = std::clamp(
        g_gameplayWorldMap.centerY,
        -dragLimitY,
        dragLimitY);
    GameplayMinimap_SetAnchoredRect(
        g_gameplayWorldMap.mapRect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        mapSize,
        mapSize,
        g_gameplayWorldMap.centerX,
        g_gameplayWorldMap.centerY);

    const float pulse =
        (std::sin(
             static_cast<float>(GetTickCount64()) *
             0.004f) +
         1.0f) *
        0.5f;
    for (int index = 0; index < markerCount; ++index)
    {
        auto& marker =
            g_gameplayWorldMapPointMarkers[index];
        const auto& point =
            g_gameplayWorldMap.points[
                static_cast<size_t>(index)];
        const bool hovered =
            index == hoveredMarkerIndex;
        const bool tracked =
            point.runtimeIndex == trackedRuntime;
        const float targetScale =
            hovered
                ? 1.16f
                : (tracked ? 1.06f + pulse * 0.03f : 1.0f);
        marker.visualScale +=
            (targetScale - marker.visualScale) * 0.24f;
        const float markerSize =
            kGameplayWorldMapMarkerSize *
            marker.visualScale;
        const float normalizedX =
            (point.x - floor.minX) / worldWidth;
        const float normalizedY =
            (point.z - floor.minZ) / worldDepth;
        GameplayMinimap_SetAnchoredRect(
            marker.marker.rect,
            0.5f,
            0.5f,
            0.5f,
            0.5f,
            markerSize,
            markerSize,
            g_gameplayWorldMap.centerX +
                (normalizedX - 0.5f) * mapSize,
            g_gameplayWorldMap.centerY +
                (normalizedY - 0.5f) * mapSize);
        GameplayMinimap_SetImageSprite(
            marker.marker.image,
            GameplayMinimap_GetPointSprite(point.kind));
        SetSceneGraphicColor(
            marker.marker.image,
            point.color[0],
            point.color[1],
            point.color[2],
            hovered || tracked ? 1.0f : 0.92f);
        SetWorldGameObjectActive(
            marker.marker.gameObject,
            true);
    }
}

void GameplayMinimap_Reset()
{
    GameplayWorldMap_Reset();
    g_gameplayMinimap = GameplayMinimapState{};
    GameplayMapTracking_Clear();
    for (auto& marker :
         g_gameplayMinimapPointMarkers)
    {
        marker = GameplayMinimapPointMarker{};
    }
}

void GameplayWorldMap_Reset()
{
    GameplayWorldMap_ReleaseTexture();
    g_gameplayWorldMap = GameplayWorldMapState{};
    for (auto& marker :
         g_gameplayWorldMapPointMarkers)
    {
        marker = GameplayWorldMapPointMarker{};
    }
    g_gameplayWorldMapOpen = false;
    g_gameplayWorldMapToggleRequested = false;
    GameplayWorldMap_ResetPhysicalInput();
}

void GameplayMinimap_Update()
{
    if (!GameplayWorldMap_ShouldBlockNativeInput() &&
        !g_gameplayDisabledInputActions.empty())
    {
        GameplayInput_RestoreDisabledActions();
    }
    if (!ExtendedMode_IsUnityObjectAlive(g_gameplayMinimap.root))
    {
        GameplayMinimap_Reset();
        return;
    }
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        SetWorldGameObjectActive(g_gameplayMinimap.root, false);
        if (ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.root))
        {
            SetWorldGameObjectActive(g_gameplayWorldMap.root, false);
        }
        g_gameplayWorldMapOpen = false;
        GameplayWorldMap_ResetPhysicalInput();
        return;
    }
    if (g_gameplayWorldMapToggleRequested.exchange(false))
    {
        const bool shouldOpen = !g_gameplayWorldMapOpen.load();
        if (!shouldOpen ||
            !GameplayInput_IsCustomModalUiOpen())
        {
            GameplayWorldMap_SetOpen(shouldOpen);
        }
    }
    const bool worldMapOpen =
        g_gameplayWorldMapOpen.load();
    SetWorldGameObjectActive(
        g_gameplayMinimap.root,
        !worldMapOpen);
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayWorldMap.root))
    {
        SetWorldGameObjectActive(
            g_gameplayWorldMap.root,
            worldMapOpen);
    }

    const int stageType = ExtendedMode_GetCurrentStageType();
    if (stageType != g_gameplayMinimap.stageType)
    {
        if (!GameplayMinimap_LoadStage(stageType))
        {
            SetWorldGameObjectActive(g_gameplayMinimap.root, false);
            if (worldMapOpen)
            {
                GameplayWorldMap_SetOpen(false);
            }
            LogInfo(LogCategory_Ui, L"Ui", L"Minimap has no atlas for stage %d.", stageType);
            return;
        }
    }
    if (g_gameplayMinimap.floors.empty())
    {
        return;
    }

    auto* playerObject = g_extendedPlayerGameObject != nullptr
        ? g_extendedPlayerGameObject
        : g_extendedAvatarRoot;
    auto* playerTransform = GetWorldTransform(playerObject);
    if (playerTransform == nullptr)
    {
        return;
    }
    auto* positionValue = Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "get_position",
            0),
        playerTransform,
        nullptr);
    if (positionValue == nullptr || il2cpp_object_unbox == nullptr)
    {
        return;
    }
    const auto* position = static_cast<float*>(
        il2cpp_object_unbox(positionValue));

    int selectedFloor = 0;
    float nearestDistance = 1.0e30f;
    for (int index = 0;
         index < static_cast<int>(g_gameplayMinimap.floors.size());
         ++index)
    {
        const float distance =
            std::abs(position[1] - g_gameplayMinimap.floors[index].height);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            selectedFloor = index;
        }
    }
    if (selectedFloor != g_gameplayMinimap.floorIndex)
    {
        const auto& floor = g_gameplayMinimap.floors[selectedFloor];
        if (!g_gameplayMinimap.texturePath.empty())
        {
            ReleaseScenePngTexture(g_gameplayMinimap.texturePath);
        }
        auto* texture = LoadScenePngTexture(floor.imagePath);
        if (texture == nullptr)
        {
            SetWorldGameObjectActive(g_gameplayMinimap.root, false);
            if (g_gameplayWorldMapOpen.load())
            {
                GameplayWorldMap_SetOpen(false);
            }
            LogInfo(LogCategory_Ui, L"Ui", 
                L"Minimap stage %d floor %d image is unavailable.",
                stageType,
                selectedFloor + 1);
            return;
        }
        g_gameplayMinimap.texture = texture;
        g_gameplayMinimap.texturePath = floor.imagePath;
        g_gameplayMinimap.floorIndex = selectedFloor;
        SetSceneRawImageTexture(
            g_gameplayMinimap.rawImage,
            texture);
    }

    const auto& floor =
        g_gameplayMinimap.floors[g_gameplayMinimap.floorIndex];
    const float worldWidth = std::max(floor.maxX - floor.minX, 0.01f);
    const float worldDepth = std::max(floor.maxZ - floor.minZ, 0.01f);
    const float pixelsPerMeter =
        kGameplayMinimapDiameter /
        (kGameplayMinimapRadiusMeters * 2.0f);
    const float mapWidth = worldWidth * pixelsPerMeter;
    const float mapHeight = worldDepth * pixelsPerMeter;
    const float playerImageX =
        (position[0] - floor.minX) * pixelsPerMeter;
    const float playerImageY =
        (position[2] - floor.minZ) * pixelsPerMeter;
    GameplayMinimap_SetAnchoredRect(
        g_gameplayMinimap.mapRect,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        mapWidth,
        mapHeight,
        mapWidth * 0.5f - playerImageX,
        mapHeight * 0.5f - playerImageY);

    float forwardX = 0.0f;
    float forwardZ = 1.0f;
    if (g_extendedCamera != nullptr &&
        ExtendedMode_IsUnityObjectAlive(g_extendedCamera))
    {
        auto* cameraTransform = GetWorldTransform(
            GetComponentGameObject(g_extendedCamera));
        auto* forwardValue = cameraTransform != nullptr
            ? Invoke(
                FindMethod(
                    FindClass("UnityEngine", "Transform"),
                    "get_forward",
                    0),
                cameraTransform,
                nullptr)
            : nullptr;
        if (forwardValue != nullptr)
        {
            const auto* forward =
                static_cast<float*>(il2cpp_object_unbox(forwardValue));
            forwardX = forward[0];
            forwardZ = forward[2];
        }
    }
    const float horizontalLength =
        std::sqrt(forwardX * forwardX + forwardZ * forwardZ);
    constexpr float radiansToDegrees = 57.29577951308232f;
    const float yaw = horizontalLength > 0.001f
        ? std::atan2(forwardX, forwardZ) * radiansToDegrees
        : 0.0f;
    SetTransformLocalEuler(
        GetWorldTransform(g_gameplayMinimap.rotationContainer),
        0.0f,
        0.0f,
        yaw);
    GameplayMinimap_UpdatePointMarkers(
        position[0],
        position[1],
        position[2],
        yaw);
    GameplayWorldMap_Update();
}
