// Native.cpp module: checkpoint_compass.inc
// Included inside the shared implementation namespace by Native.cpp.

enum class CheckpointCompassTargetKind
{
    Supply,
    Extract,
    Transfer,
    Task,
};

struct CheckpointCompassTarget
{
    CheckpointCompassTargetKind kind =
        CheckpointCompassTargetKind::Task;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float angle = 0.0f;
    float distance = 0.0f;
};

struct CheckpointCompassMarker
{
    SceneUiElement badge{};
    SceneUiElement background{};
    Il2CppObject* text = nullptr;
    Il2CppObject* typeText = nullptr;
};

constexpr int kCheckpointCompassMarkerCapacity = 24;
static SceneUiElement g_checkpointCompassRoot{};
static CheckpointCompassMarker
    g_checkpointCompassMarkers[kCheckpointCompassMarkerCapacity]{};
static Il2CppObject* g_checkpointCompassNativeRoot = nullptr;
static unsigned long long g_checkpointCompassNextNativeScan = 0;
static Il2CppObject*
    g_checkpointCompassShapeTextures[3]{};
static size_t g_checkpointCompassLastTargetCount =
    static_cast<size_t>(-1);

void CheckpointCompass_Reset()
{
    // These handles belong to the current gameplay HUD and scene. Clear them
    // without touching the Unity objects; their owners destroy the objects.
    g_checkpointCompassRoot = SceneUiElement{};
    for (auto& marker : g_checkpointCompassMarkers)
    {
        marker = CheckpointCompassMarker{};
    }
    g_checkpointCompassNativeRoot = nullptr;
    g_checkpointCompassNextNativeScan = 0;
    for (auto& texture : g_checkpointCompassShapeTextures)
    {
        texture = nullptr;
    }
    g_checkpointCompassLastTargetCount =
        static_cast<size_t>(-1);
}

Il2CppObject* CheckpointCompass_GetShapeTexture(int shape)
{
    shape = std::clamp(shape, 0, 2);
    Il2CppObject*& cached =
        g_checkpointCompassShapeTextures[shape];
    if (!ExtendedMode_IsUnityObjectAlive(cached))
    {
        const wchar_t* fileName =
            shape == 0
                ? L"compass_up.png"
                : (shape == 1
                    ? L"compass_down.png"
                    : L"compass_square.png");
        cached = LoadScenePngTexture(
            GetEscapeFromManakaSubdirectory(L"Checkpoints") +
            L"\\" +
            fileName);
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint compass mask loaded: shape=%d texture=%p.",
            shape,
            cached);
    }
    return cached;
}

bool CheckpointCompass_GetCameraForward(
    float& x,
    float& z)
{
    Il2CppObject* transform = nullptr;
    if (ExtendedMode_IsUnityObjectAlive(g_extendedCamera))
    {
        transform = GetWorldTransform(
            GetComponentGameObject(g_extendedCamera));
    }
    if (transform == nullptr &&
        ExtendedMode_IsUnityObjectAlive(
            g_extendedPlayerGameObject))
    {
        transform = GetWorldTransform(
            g_extendedPlayerGameObject);
    }
    if (transform == nullptr)
    {
        return false;
    }

    auto* forwardValue = Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "get_forward",
            0),
        transform,
        nullptr);
    if (forwardValue == nullptr)
    {
        return false;
    }
    const auto* forward =
        static_cast<float*>(il2cpp_object_unbox(forwardValue));
    const float length =
        std::sqrt(forward[0] * forward[0] +
                  forward[2] * forward[2]);
    if (length <= 0.0001f)
    {
        return false;
    }
    x = forward[0] / length;
    z = forward[2] / length;
    return true;
}

bool CheckpointCompass_GetPlayerPosition(
    float& x,
    float& y,
    float& z)
{
    if (!ExtendedMode_IsUnityObjectAlive(
            g_extendedPlayerGameObject))
    {
        return false;
    }
    auto* transform = GetWorldTransform(
        g_extendedPlayerGameObject);
    if (transform == nullptr)
    {
        return false;
    }
    auto* positionValue = Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "get_position",
            0),
        transform,
        nullptr);
    if (positionValue == nullptr)
    {
        return false;
    }
    const auto* position =
        static_cast<float*>(il2cpp_object_unbox(positionValue));
    x = position[0];
    y = position[1];
    z = position[2];
    return true;
}

void CheckpointCompass_HideNative()
{
    const unsigned long long now = GetTickCount64();
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointCompassNativeRoot))
    {
        if (now >= g_checkpointCompassNextNativeScan)
        {
            g_checkpointCompassNextNativeScan = now + 1000;
            SetWorldGameObjectActive(
                g_checkpointCompassNativeRoot,
                false);
        }
        return;
    }
    g_checkpointCompassNativeRoot = nullptr;
    if (now < g_checkpointCompassNextNativeScan)
    {
        return;
    }
    g_checkpointCompassNextNativeScan = now + 1000;

    auto* compassClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.InGame.Compass",
        "CompassView");
    if (compassClass == nullptr)
    {
        return;
    }
    auto* instance = Invoke(
        FindMethod(compassClass, "get_Instance", 0),
        nullptr,
        nullptr);
    if (instance == nullptr)
    {
        return;
    }
    auto* root = GetComponentGameObject(instance);
    if (root == nullptr)
    {
        return;
    }
    g_checkpointCompassNativeRoot = root;
    SetWorldGameObjectActive(root, false);
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
        L"Checkpoint compass hid native CompassView markers: object=%s.",
        GetObjectName(root).c_str());
}

void CheckpointCompass_EnsureBuilt()
{
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointCompassRoot.gameObject) ||
        !ExtendedMode_IsUnityObjectAlive(
            g_extendedGameplayHudCanvasObject))
    {
        return;
    }

    ExtendedMode_CacheFont();
    g_checkpointCompassRoot = CreateScenePanel(
        "CheckpointCompass",
        GetWorldTransform(g_extendedGameplayHudCanvasObject),
        0.325f,
        0.920f,
        0.675f,
        0.993f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    if (g_checkpointCompassRoot.gameObject == nullptr)
    {
        return;
    }

    const std::wstring squareMaskPath =
        GetEscapeFromManakaSubdirectory(L"Checkpoints") +
        L"\\compass_square.png";
    CreateScenePanel(
        "CheckpointCompassLine",
        g_checkpointCompassRoot.rect,
        0.04f,
        0.475f,
        0.96f,
        0.505f,
        0.70f,
        0.73f,
        0.76f,
        0.55f);

    for (int index = 0;
         index < kCheckpointCompassMarkerCapacity;
         ++index)
    {
        char badgeName[64]{};
        char accentName[64]{};
        char textName[64]{};
        char typeName[64]{};
        std::snprintf(
            badgeName,
            sizeof(badgeName),
            "CheckpointCompassMarker%02d",
            index);
        std::snprintf(
            accentName,
            sizeof(accentName),
            "CheckpointCompassAccent%02d",
            index);
        std::snprintf(
            textName,
            sizeof(textName),
            "CheckpointCompassLabel%02d",
            index);
        std::snprintf(
            typeName,
            sizeof(typeName),
            "CheckpointCompassType%02d",
            index);
        auto& marker = g_checkpointCompassMarkers[index];
        marker.badge.gameObject = CreateSceneGameObject(
            badgeName,
            g_checkpointCompassRoot.rect);
        marker.badge.rect = GetSceneComponentByClass(
            marker.badge.gameObject,
            "UnityEngine",
            "RectTransform");
        SetSceneRect(
            marker.badge.rect,
            0.45f,
            0.03f,
            0.55f,
            0.97f);
        marker.background = CreateSceneRawImageElement(
            accentName,
            marker.badge.rect,
            squareMaskPath,
            0.275f,
            0.08f,
            0.725f,
            0.92f,
            0.55f,
            0.58f,
            0.62f,
            0.88f);
        marker.text = CreateSceneTextElement(
            textName,
            marker.badge.rect,
            "",
            0.0f,
            0.52f,
            1.0f,
            1.0f,
            13.0f,
            514,
            1.0f,
            1.0f,
            1.0f,
            1.0f).text;
        marker.typeText = CreateSceneTextElement(
            typeName,
            marker.badge.rect,
            "",
            0.0f,
            0.0f,
            1.0f,
            0.42f,
            13.0f,
            514,
            1.0f,
            1.0f,
            1.0f,
            1.0f).text;
        SetWorldGameObjectActive(
            marker.badge.gameObject,
            false);
    }
    SetWorldGameObjectActive(
        g_checkpointCompassRoot.gameObject,
        false);
    LogInfo(LogCategory_Checkpoint, L"Checkpoint", L"Checkpoint compass created.");
}

void CheckpointCompass_AppendTarget(
    std::vector<CheckpointCompassTarget>& targets,
    CheckpointCompassTargetKind kind,
    float playerX,
    float playerY,
    float playerZ,
    float forwardX,
    float forwardZ,
    float targetX,
    float targetY,
    float targetZ)
{
    const float dx = targetX - playerX;
    const float dz = targetZ - playerZ;
    float horizontalLength =
        std::sqrt(dx * dx + dz * dz);
    float normalizedX = 0.0f;
    float normalizedZ = 1.0f;
    if (horizontalLength > 0.0001f)
    {
        normalizedX = dx / horizontalLength;
        normalizedZ = dz / horizontalLength;
    }
    const float dot =
        forwardX * normalizedX +
        forwardZ * normalizedZ;
    const float right =
        forwardZ * normalizedX -
        forwardX * normalizedZ;
    const float verticalDistance =
        std::fabs(targetY - playerY);
    const float distance = std::sqrt(
        horizontalLength * horizontalLength +
        verticalDistance * verticalDistance);

    CheckpointCompassTarget target{};
    target.kind = kind;
    target.x = targetX;
    target.y = targetY;
    target.z = targetZ;
    target.angle =
        std::atan2(right, dot) *
        57.29577951308232f;
    target.distance = distance;
    targets.push_back(target);
}

void CheckpointCompass_Update()
{
    CheckpointCompass_HideNative();
    CheckpointCompass_EnsureBuilt();
    if (!ExtendedMode_IsUnityObjectAlive(
            g_checkpointCompassRoot.gameObject))
    {
        return;
    }

    const bool visible =
        g_extendedModeState.load() ==
            ExtendedMode_Gameplay &&
        !CheckpointSystem_TaskGameplayPaused();
    SetWorldGameObjectActive(
        g_checkpointCompassRoot.gameObject,
        visible);
    if (!visible)
    {
        return;
    }

    float playerX = 0.0f;
    float playerY = 0.0f;
    float playerZ = 0.0f;
    float forwardX = 0.0f;
    float forwardZ = 1.0f;
    if (!CheckpointCompass_GetPlayerPosition(
            playerX,
            playerY,
            playerZ) ||
        !CheckpointCompass_GetCameraForward(
            forwardX,
            forwardZ))
    {
        static bool loggedUnavailable = false;
        if (!loggedUnavailable)
        {
            loggedUnavailable = true;
            LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
                L"Checkpoint compass player or camera direction is unavailable.");
        }
        return;
    }

    std::vector<CheckpointCompassTarget> targets;
    targets.reserve(1);
    const int trackedRuntimeIndex =
        GameplayMapTracking_GetRuntimeIndex();
    CheckpointMapPoint trackedPoint;
    if (trackedRuntimeIndex >= 0 &&
        CheckpointSystem_GetMapPoint(
            trackedRuntimeIndex,
            trackedPoint))
    {
        CheckpointCompassTargetKind targetKind =
            CheckpointCompassTargetKind::Supply;
        switch (trackedPoint.kind)
        {
        case CheckpointMapPointKind::Task:
            targetKind =
                CheckpointCompassTargetKind::Task;
            break;
        case CheckpointMapPointKind::Extract:
            targetKind =
                CheckpointCompassTargetKind::Extract;
            break;
        case CheckpointMapPointKind::Transfer:
            targetKind =
                CheckpointCompassTargetKind::Transfer;
            break;
        case CheckpointMapPointKind::Supply:
        default:
            targetKind =
                CheckpointCompassTargetKind::Supply;
            break;
        }
        CheckpointCompass_AppendTarget(
            targets,
            targetKind,
            playerX,
            playerY,
            playerZ,
            forwardX,
            forwardZ,
            trackedPoint.x,
            trackedPoint.y,
            trackedPoint.z);
    }

    std::sort(
        targets.begin(),
        targets.end(),
        [](const CheckpointCompassTarget& left,
           const CheckpointCompassTarget& right)
        {
            return std::fabs(left.angle) <
                   std::fabs(right.angle);
        });
    if (targets.size() >
        kCheckpointCompassMarkerCapacity)
    {
        targets.resize(
            kCheckpointCompassMarkerCapacity);
    }
    if (targets.size() != g_checkpointCompassLastTargetCount)
    {
        g_checkpointCompassLastTargetCount = targets.size();
        LogInfo(LogCategory_Checkpoint, L"Checkpoint", 
            L"Checkpoint compass target count changed: count=%u player=(%.2f, %.2f, %.2f).",
            static_cast<unsigned int>(targets.size()),
            playerX,
            playerY,
            playerZ);
    }

    float screenAspect = 21.0f / 9.0f;
    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    if (gameWindow != nullptr &&
        GetClientRect(gameWindow, &clientRect) &&
        clientRect.right > clientRect.left &&
        clientRect.bottom > clientRect.top)
    {
        screenAspect =
            static_cast<float>(
                clientRect.right - clientRect.left) /
            static_cast<float>(
                clientRect.bottom - clientRect.top);
    }
    // Root width=.35 screen, height=.073 screen. Badge is .12 of the
    // root width and .94 of its height. The mask occupies .55 of the
    // badge height, so this keeps squares square and triangles from
    // being stretched on ultrawide displays.
    constexpr float rootWidth = 0.35f;
    constexpr float rootHeight = 0.073f;
    constexpr float badgeWidth = 0.12f;
    constexpr float badgeHeight = 0.94f;
    constexpr float maskHeight = 0.84f;
    const float maskHalfWidth = 0.5f *
        maskHeight * (rootHeight * badgeHeight) /
        (rootWidth * badgeWidth * screenAspect);

    for (int index = 0;
         index < kCheckpointCompassMarkerCapacity;
         ++index)
    {
        auto& marker = g_checkpointCompassMarkers[index];
        if (index >= static_cast<int>(targets.size()))
        {
            SetWorldGameObjectActive(
                marker.badge.gameObject,
                false);
            continue;
        }
        const auto& target =
            targets[static_cast<size_t>(index)];
        const float normalizedAngle =
            std::clamp(
                target.angle / 68.0f,
                -1.0f,
                1.0f);
        const float centerX =
            0.5f + normalizedAngle * 0.46f;
        const float halfWidth = 0.060f;
        SetSceneRect(
            marker.badge.rect,
            std::clamp(
                centerX - halfWidth,
                0.005f,
                0.99f - halfWidth * 2.0f),
            0.04f,
            std::clamp(
                centerX + halfWidth,
                halfWidth * 2.0f + 0.005f,
                0.995f),
            0.96f);

        const char* label = "地图点";
        float red = 0.56f;
        float green = 0.59f;
        float blue = 0.63f;
        int shape = 2;
        if (target.kind ==
            CheckpointCompassTargetKind::Supply)
        {
            label = "物资点";
        }
        else if (target.kind ==
            CheckpointCompassTargetKind::Extract)
        {
            label = "撤离点";
        }
        else if (target.kind ==
                 CheckpointCompassTargetKind::Transfer)
        {
            label = "转移点";
        }
        else if (target.kind ==
                 CheckpointCompassTargetKind::Task)
        {
            label = "任务点";
        }
        const float heightDelta = target.y - playerY;
        if (heightDelta > 1.0f)
        {
            red = 0.18f;
            green = 0.76f;
            blue = 0.36f;
            shape = 0;
        }
        else if (heightDelta < -1.0f)
        {
            red = 0.90f;
            green = 0.20f;
            blue = 0.20f;
            shape = 1;
        }
        SetSceneRect(
            marker.background.rect,
            std::clamp(
                0.5f - maskHalfWidth,
                0.0f,
                1.0f),
            0.08f,
            std::clamp(
                0.5f + maskHalfWidth,
                0.0f,
                1.0f),
            0.92f);
        SetSceneRawImageTexture(
            marker.background.rawImage,
            CheckpointCompass_GetShapeTexture(shape));
        SetSceneGraphicColor(
            marker.background.rawImage,
            red,
            green,
            blue,
            0.88f);
        char distanceLabel[32]{};
        std::snprintf(
            distanceLabel,
            sizeof(distanceLabel),
            "%.0fm",
            target.distance);
        SetSceneText(
            marker.text,
            distanceLabel,
            13.0f,
            514,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        SetSceneText(
            marker.typeText,
            label,
            13.0f,
            514,
            1.0f,
            1.0f,
            1.0f,
            1.0f);
        SetWorldGameObjectActive(
            marker.badge.gameObject,
            true);
    }
}

void CheckpointCompass_Hide()
{
    if (ExtendedMode_IsUnityObjectAlive(
            g_checkpointCompassRoot.gameObject))
    {
        SetWorldGameObjectActive(
            g_checkpointCompassRoot.gameObject,
            false);
    }
}
