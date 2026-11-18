// Native.cpp module: gameplay_status_hud.inc
// Compact, state-driven gameplay status list displayed in the lower-left.

constexpr int kGameplayStatusHudMaxRows = 13;
constexpr float kGameplayStatusHudFadeSeconds = 0.22f;
constexpr int kGameplayControlsHintCount = 9;

static SceneUiElement g_gameplayStatusHudPanel{};
static SceneUiElement g_gameplayControlsHintPanel{};
static SceneUiElement g_gameplayControlsHintRows[kGameplayControlsHintCount]{};
static Il2CppObject* g_gameplayControlsHintTexts[kGameplayControlsHintCount]{};
struct GameplayStatusHudRowState
{
    SceneUiElement background;
    SceneUiElement element;
    std::string text;
    std::string renderedText;
    float alpha = 0.0f;
    float targetAlpha = 0.0f;
    float renderedAlpha = -1.0f;
};
static std::vector<GameplayStatusHudRowState> g_gameplayStatusHudRows;
static unsigned long long g_gameplayStatusHudLastUpdateTick = 0;
static std::string g_gameplayStatusHudNotification;
static unsigned long long g_gameplayStatusHudNotificationUntil = 0;
static Il2CppObject* g_gameplayOriginalStaminaGaugeObject = nullptr;
static Il2CppObject* g_gameplayOriginalStaminaGaugeComponent = nullptr;
static unsigned long long g_gameplayOriginalStaminaGaugeLayoutTick = 0;
static float g_gameplayOriginalStaminaGaugeMax = -1.0f;
static bool g_gameplayOriginalStaminaGaugeLayoutLogged = false;
static bool g_gameplayOriginalStaminaGaugeSyncLogged = false;
static SceneUiElement g_gameplayStaminaBarBackground{};
static SceneUiElement g_gameplayStaminaBarFill{};
static float g_gameplayStaminaBarWidth = -1.0f;
static float g_gameplayStaminaBarCapacityMultiplier = -1.0f;

constexpr float kGameplayStaminaBarLeft = 206.0f;
constexpr float kGameplayStaminaBarCenterY = -106.0f;
constexpr float kGameplayStaminaBarWidthMax = 498.0f;
constexpr float kGameplayStaminaBarHeight = 8.0f;

void GameplayStatusHud_SetAnchoredRect(
    Il2CppObject* rect,
    float anchorX,
    float anchorY,
    float pivotX,
    float pivotY,
    float width,
    float height,
    float positionX,
    float positionY);

void GameplayStatusHud_SetRectSize(
    Il2CppObject* rect,
    float width,
    float height)
{
    if (rect == nullptr)
    {
        return;
    }
    float size[] = { width, height };
    void* parameters[] = { size };
    Invoke(
        FindMethod(FindClass("UnityEngine", "RectTransform"), "set_sizeDelta", 1),
        rect,
        parameters);
}

void GameplayStatusHud_BuildOwnedStaminaBar(Il2CppObject* rootTransform)
{
    if (rootTransform == nullptr)
    {
        return;
    }
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayStaminaBarBackground.gameObject) &&
        ExtendedMode_IsUnityObjectAlive(g_gameplayStaminaBarFill.gameObject))
    {
        return;
    }
    g_gameplayStaminaBarBackground = CreateScenePanel(
        "EscapeFromManakaStaminaBarBackground",
        rootTransform,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.80f);
    g_gameplayStaminaBarFill = CreateScenePanel(
        "EscapeFromManakaStaminaBarFill",
        rootTransform,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.257409f,
        0.773000f,
        0.276848f,
        1.0f);
    GameplayStatusHud_SetAnchoredRect(
        g_gameplayStaminaBarBackground.rect,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        500.0f,
        10.0f,
        kGameplayStaminaBarLeft,
        kGameplayStaminaBarCenterY);
    GameplayStatusHud_SetAnchoredRect(
        g_gameplayStaminaBarFill.rect,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        kGameplayStaminaBarWidthMax,
        kGameplayStaminaBarHeight,
        kGameplayStaminaBarLeft + 1.0f,
        kGameplayStaminaBarCenterY);
    g_gameplayStaminaBarWidth = kGameplayStaminaBarWidthMax;
    g_gameplayStaminaBarCapacityMultiplier = 1.0f;
    SetWorldGameObjectActive(
        g_gameplayStaminaBarBackground.gameObject,
        false);
    SetWorldGameObjectActive(g_gameplayStaminaBarFill.gameObject, false);
}

void GameplayStatusHud_UpdateOwnedStaminaBar()
{
    auto* rootTransform = GetWorldTransform(
        g_extendedGameplayHudCanvasObject);
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayStaminaBarBackground.gameObject) ||
        !ExtendedMode_IsUnityObjectAlive(
            g_gameplayStaminaBarFill.gameObject))
    {
        GameplayStatusHud_BuildOwnedStaminaBar(rootTransform);
    }
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayStaminaBarBackground.gameObject) ||
        !ExtendedMode_IsUnityObjectAlive(
            g_gameplayStaminaBarFill.gameObject))
    {
        return;
    }

    float current = 0.0f;
    float max = 0.0f;
    if (!EquipmentStats_ReadStaminaSnapshot(current, max))
    {
        SetWorldGameObjectActive(
            g_gameplayStaminaBarBackground.gameObject,
            false);
        SetWorldGameObjectActive(g_gameplayStaminaBarFill.gameObject, false);
        return;
    }

    const bool visible =
        !g_extendedOriginalSystemMenuVisible.load() &&
        !g_extendedOriginalTutorialActive.load() &&
        !GameplayInput_IsCustomModalUiOpen();
    SetWorldGameObjectActive(
        g_gameplayStaminaBarBackground.gameObject,
        visible);
    SetWorldGameObjectActive(g_gameplayStaminaBarFill.gameObject, visible);
    if (!visible)
    {
        return;
    }

    const float capacityMultiplier =
        EquipmentStats_GetStaminaCapacityMultiplier();
    const bool capacityChanged =
        std::fabs(
            capacityMultiplier -
            g_gameplayStaminaBarCapacityMultiplier) >= 0.0001f;
    if (capacityChanged)
    {
        GameplayStatusHud_SetRectSize(
            g_gameplayStaminaBarBackground.rect,
            500.0f * capacityMultiplier,
            10.0f);
        g_gameplayStaminaBarCapacityMultiplier = capacityMultiplier;
        g_gameplayStaminaBarWidth = -1.0f;
    }
    const float fillMaxWidth =
        kGameplayStaminaBarWidthMax * capacityMultiplier;
    const float width = std::clamp(
        current / max,
        0.0f,
        1.0f) *
        fillMaxWidth;
    if (std::fabs(width - g_gameplayStaminaBarWidth) >= 0.25f)
    {
        GameplayStatusHud_SetRectSize(
            g_gameplayStaminaBarFill.rect,
            width,
            kGameplayStaminaBarHeight);
        g_gameplayStaminaBarWidth = width;
    }
}

void GameplayStatusHud_HideResidualGauges()
{
    auto* uiManager = g_extendedInGameUiManager;
    if (!ExtendedMode_IsUnityObjectAlive(uiManager))
    {
        uiManager = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Object3D.IngameManager",
            "InGameUiManager",
            "Instance");
        g_extendedInGameUiManager = uiManager;
    }
    if (uiManager == nullptr)
    {
        return;
    }

    static bool fieldsResolved = false;
    static size_t staminaGaugeOffset = static_cast<size_t>(-1);
    static size_t ecstasyGaugeOffset = static_cast<size_t>(-1);
    static size_t mentalGaugeOffset = static_cast<size_t>(-1);
    static size_t moistureGaugeOffset = static_cast<size_t>(-1);
    static size_t moistureIconOffset = static_cast<size_t>(-1);
    if (!fieldsResolved)
    {
        fieldsResolved = true;
        auto* managerClass = FindClass(
            "ExposureUnnoticed2.Object3D.IngameManager",
            "InGameUiManager");
        const auto resolveOffset =
            [&](const char* fieldName) -> size_t
            {
                auto* field = managerClass != nullptr
                    ? il2cpp_class_get_field_from_name(
                        managerClass,
                        fieldName)
                    : nullptr;
                return field != nullptr
                    ? il2cpp_field_get_offset(field)
                    : static_cast<size_t>(-1);
            };
        staminaGaugeOffset = resolveOffset("staminaGauge");
        ecstasyGaugeOffset = resolveOffset("ecstasyGauge");
        mentalGaugeOffset = resolveOffset("mentalGauge");
        moistureGaugeOffset = resolveOffset("moistureGauge");
        moistureIconOffset = resolveOffset("moistureIcon");
        LogInfo(
            LogCategory_Ui,
            L"OriginalGaugeFields",
            L"Resolved InGameUiManager gauge fields: stamina=0x%llX "
            L"ecstasy=0x%llX mental=0x%llX moisture=0x%llX icon=0x%llX.",
            static_cast<unsigned long long>(staminaGaugeOffset),
            static_cast<unsigned long long>(ecstasyGaugeOffset),
            static_cast<unsigned long long>(mentalGaugeOffset),
            static_cast<unsigned long long>(moistureGaugeOffset),
            static_cast<unsigned long long>(moistureIconOffset));
    }

    const size_t componentOffsets[] = {
        staminaGaugeOffset,
        ecstasyGaugeOffset,
        mentalGaugeOffset,
        moistureGaugeOffset,
    };
    for (const size_t offset : componentOffsets)
    {
        if (offset == static_cast<size_t>(-1))
        {
            continue;
        }
        auto* component = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(uiManager) + offset);
        auto* object = component != nullptr
            ? GetComponentGameObject(component)
            : nullptr;
        if (object != nullptr)
        {
            SetWorldGameObjectActive(object, false);
        }
    }

    // moistureIcon is the FrontImager only. Its BackImage sibling is not
    // referenced by InGameUiManager and would remain visible if the front
    // component's GameObject were hidden alone. The shared parent is Scale.
    if (moistureIconOffset != static_cast<size_t>(-1))
    {
        auto* moistureIcon = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(uiManager) + moistureIconOffset);
        auto* frontObject = moistureIcon != nullptr
            ? GetComponentGameObject(moistureIcon)
            : nullptr;
        SetWorldGameObjectActive(frontObject, false);

        auto* frontTransform = GetWorldTransform(frontObject);
        auto* transformClass = FindClass("UnityEngine", "Transform");
        auto* containerTransform = frontTransform != nullptr &&
                transformClass != nullptr
            ? Invoke(
                FindMethod(transformClass, "get_parent", 0),
                frontTransform,
                nullptr)
            : nullptr;
        auto* containerObject = containerTransform != nullptr
            ? GetComponentGameObject(containerTransform)
            : nullptr;
        if (containerObject != nullptr &&
            GetObjectName(containerObject) == L"Scale")
        {
            SetWorldGameObjectActive(containerObject, false);
            LogOnce(
                "gameplay-original-moisture-icon-hidden",
                LogLevel_Info,
                LogCategory_Ui,
                L"OriginalMoistureIcon",
                L"Hid the original moisture icon container (Scale) so both "
                L"BackImage and FrontImager are removed; field=0x%llX "
                L"front=%p container=%p.",
                static_cast<unsigned long long>(moistureIconOffset),
                frontObject,
                containerObject);
        }
        else
        {
            LogOnce(
                "gameplay-original-moisture-icon-container-missing",
                LogLevel_Warn,
                LogCategory_Ui,
                L"OriginalMoistureIcon",
                L"Hid only the original moisture foreground because the "
                L"expected Scale parent was unavailable; front=%p parent=%p.",
                frontObject,
                containerObject);
        }
    }

    auto* statusInfoObject = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(uiManager) + 0xD0);
    SetWorldGameObjectActive(statusInfoObject, false);
}

void GameplayStatusHud_SetAnchoredRect(
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

void GameplayStatusHud_SetGraphicAlpha(
    Il2CppObject* graphic,
    float alpha)
{
    if (graphic == nullptr)
    {
        return;
    }
    auto* graphicClass = FindClass("UnityEngine.UI", "Graphic");
    auto* colorObject = Invoke(
        FindMethod(graphicClass, "get_color", 0),
        graphic,
        nullptr);
    if (colorObject == nullptr || il2cpp_object_unbox == nullptr)
    {
        return;
    }
    auto* color =
        static_cast<float*>(il2cpp_object_unbox(colorObject));
    float adjustedColor[] = {
        color[0],
        color[1],
        color[2],
        alpha};
    void* colorParameters[] = { adjustedColor };
    Invoke(
        FindMethod(graphicClass, "set_color", 1),
        graphic,
        colorParameters);
}

void GameplayStatusHud_ForceStaminaGaugeVisible(
    Il2CppObject* gameObject,
    int depth)
{
    if (gameObject == nullptr || depth > 8)
    {
        return;
    }
    SetWorldGameObjectActive(gameObject, true);

    auto* canvasGroup = GetSceneComponentByClass(
        gameObject,
        "UnityEngine",
        "CanvasGroup");
    if (canvasGroup != nullptr)
    {
        float alpha = 1.0f;
        bool interactable = false;
        bool blocksRaycasts = false;
        void* alphaParameters[] = { &alpha };
        void* interactableParameters[] = { &interactable };
        void* blocksParameters[] = { &blocksRaycasts };
        Invoke(
            FindMethod(FindClass("UnityEngine", "CanvasGroup"), "set_alpha", 1),
            canvasGroup,
            alphaParameters);
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "CanvasGroup"),
                "set_interactable",
                1),
            canvasGroup,
            interactableParameters);
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "CanvasGroup"),
                "set_blocksRaycasts",
                1),
            canvasGroup,
            blocksParameters);
    }

    auto* image = GetSceneComponentByClass(
        gameObject,
        "UnityEngine.UI",
        "Image");
    if (image != nullptr)
    {
        ExtendedMode_SetComponentEnabled(image, true);
        GameplayStatusHud_SetGraphicAlpha(image, 1.0f);
    }
    auto* rawImage = GetSceneComponentByClass(
        gameObject,
        "UnityEngine.UI",
        "RawImage");
    if (rawImage != nullptr)
    {
        ExtendedMode_SetComponentEnabled(rawImage, true);
        GameplayStatusHud_SetGraphicAlpha(rawImage, 1.0f);
    }

    auto* transform = GetWorldTransform(gameObject);
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* childCountObject = transform != nullptr
        ? Invoke(
            FindMethod(transformClass, "get_childCount", 0),
            transform,
            nullptr)
        : nullptr;
    const int childCount =
        childCountObject != nullptr && il2cpp_object_unbox != nullptr
            ? *static_cast<int*>(il2cpp_object_unbox(childCountObject))
            : 0;
    for (int index = 0; index < childCount; ++index)
    {
        int childIndex = index;
        void* childParameters[] = { &childIndex };
        auto* childTransform = Invoke(
            FindMethod(transformClass, "GetChild", 1),
            transform,
            childParameters);
        auto* childObject = childTransform != nullptr
            ? Invoke(
                FindMethod(
                    FindClass("UnityEngine", "Component"),
                    "get_gameObject",
                    0),
                childTransform,
                nullptr)
            : nullptr;
        GameplayStatusHud_ForceStaminaGaugeVisible(
            childObject,
            depth + 1);
    }
}

void GameplayStatusHud_PositionOriginalStaminaGauge()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }
    const unsigned long long now = GetTickCount64();
    const bool shouldRelayout =
        g_gameplayOriginalStaminaGaugeLayoutTick == 0 ||
        now - g_gameplayOriginalStaminaGaugeLayoutTick >= 500;
    if (shouldRelayout)
    {
        g_gameplayOriginalStaminaGaugeLayoutTick = now;
    }

    if (!ExtendedMode_IsUnityObjectAlive(g_extendedInGameUiManager))
    {
        g_extendedInGameUiManager = ExtendedMode_GetStaticObject(
            "ExposureUnnoticed2.Object3D.IngameManager",
            "InGameUiManager",
            "Instance");
    }
    auto* uiManager = g_extendedInGameUiManager;
    if (uiManager == nullptr)
    {
        return;
    }
    auto* gauge = g_gameplayOriginalStaminaGaugeComponent;
    auto* gaugeObject = g_gameplayOriginalStaminaGaugeObject;
    if (!ExtendedMode_IsUnityObjectAlive(gaugeObject))
    {
        gaugeObject = nullptr;
        gauge = nullptr;
        auto* sourceGauge = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(uiManager) + 0x28);
        if (sourceGauge == nullptr)
        {
            return;
        }
        auto* objectClass = FindClass("UnityEngine", "Object");
        auto* instantiate = FindMethod(objectClass, "Instantiate", 1);
        if (instantiate == nullptr)
        {
            return;
        }
        void* instantiateParameters[] = { sourceGauge };
        gauge = Invoke(instantiate, nullptr, instantiateParameters);
        gaugeObject = gauge != nullptr
            ? GetComponentGameObject(gauge)
            : nullptr;
        if (gaugeObject == nullptr)
        {
            return;
        }
        g_gameplayOriginalStaminaGaugeComponent = gauge;
        g_gameplayOriginalStaminaGaugeObject = gaugeObject;
        g_gameplayOriginalStaminaGaugeMax = -1.0f;
        g_gameplayOriginalStaminaGaugeLayoutLogged = false;
        LogInfo(
            LogCategory_Ui,
            L"GameplayStaminaGauge",
            L"Cloned the original stamina gauge for the MOD HUD: "
            L"source=%p clone=%p component=%p.",
            sourceGauge,
            gaugeObject,
            gauge);
    }
    SetWorldGameObjectActive(gaugeObject, true);
    if (shouldRelayout)
    {
        auto* gaugeAnimator = GetSceneComponentByClass(
            gaugeObject,
            "UnityEngine",
            "Animator");
        if (gaugeAnimator != nullptr)
        {
            ExtendedMode_SetComponentEnabled(gaugeAnimator, false);
        }
        GameplayStatusHud_ForceStaminaGaugeVisible(gaugeObject, 0);
        if (gauge != nullptr)
        {
            const auto* gaugeRaw =
                reinterpret_cast<const char*>(gauge);
            auto* frontRect = *reinterpret_cast<Il2CppObject* const*>(
                gaugeRaw + 0x20);
            auto* frameRect = *reinterpret_cast<Il2CppObject* const*>(
                gaugeRaw + 0x28);
            auto* frontImage = *reinterpret_cast<Il2CppObject* const*>(
                gaugeRaw + 0x30);
            auto* frontObject = frontRect != nullptr
                ? GetComponentGameObject(frontRect)
                : nullptr;
            auto* frameObject = frameRect != nullptr
                ? GetComponentGameObject(frameRect)
                : nullptr;
            SetWorldGameObjectActive(frontObject, true);
            SetWorldGameObjectActive(frameObject, true);
            if (frontImage != nullptr)
            {
                ExtendedMode_SetComponentEnabled(frontImage, true);
                SetSceneGraphicColor(
                    frontImage,
                    0.86f,
                    0.94f,
                    0.98f,
                    1.0f);
            }
            auto* frameImage = GetSceneComponentByClass(
                frameObject,
                "UnityEngine.UI",
                "Image");
            if (frameImage != nullptr)
            {
                ExtendedMode_SetComponentEnabled(frameImage, true);
                SetSceneGraphicColor(
                    frameImage,
                    0.020f,
                    0.027f,
                    0.035f,
                    0.90f);
            }
        }
        auto* gaugeRect = GetSceneComponentByClass(
            gaugeObject,
            "UnityEngine",
            "RectTransform");
        if (gaugeRect != nullptr)
        {
            auto* gaugeTransform = GetWorldTransform(gaugeObject);
            auto* hudTransform = GetWorldTransform(
                g_extendedGameplayHudCanvasObject);
            if (gaugeTransform != nullptr && hudTransform != nullptr)
            {
                bool keepWorldPosition = false;
                void* parentParameters[] = {
                    hudTransform,
                    &keepWorldPosition};
                Invoke(
                    FindMethod(
                        FindClass("UnityEngine", "Transform"),
                        "SetParent",
                        2),
                    gaugeTransform,
                    parentParameters);
            }

            float width = 280.0f;
            float height = 20.0f;
            auto* rectClass = FindClass("UnityEngine", "RectTransform");
            auto* sizeObject = Invoke(
                FindMethod(rectClass, "get_sizeDelta", 0),
                gaugeRect,
                nullptr);
            if (sizeObject != nullptr && il2cpp_object_unbox != nullptr)
            {
                const auto* size =
                    static_cast<const float*>(
                        il2cpp_object_unbox(sizeObject));
                if (std::isfinite(size[0]) &&
                    std::isfinite(size[1]) &&
                    size[0] >= 40.0f &&
                    size[1] >= 8.0f)
                {
                    width = size[0];
                    height = size[1];
                }
            }

            // The map is anchored at (18, -18), diameter 176. The cloned
            // GaugeView root is 100 wide while its centered frame is 500 wide,
            // so root x=406 makes the actual frame begin at x=206.
            constexpr float gaugeLeft = 406.0f;
            constexpr float gaugeCenterY = -106.0f;
            GameplayStatusHud_SetAnchoredRect(
                gaugeRect,
                0.0f,
                1.0f,
                0.0f,
                0.5f,
                width,
                height,
                gaugeLeft,
                gaugeCenterY);
            if (!g_gameplayOriginalStaminaGaugeLayoutLogged)
            {
                g_gameplayOriginalStaminaGaugeLayoutLogged = true;
                BringSceneObjectToFront(gaugeObject);
                LogInfo(
                    LogCategory_Ui,
                    L"GameplayStaminaGauge",
                    L"Cloned original stamina gauge positioned beside the "
                    L"minimap: x=%.1f centerY=%.1f width=%.1f height=%.1f.",
                    gaugeLeft,
                    gaugeCenterY,
                    width,
                    height);
            }
        }
    }

    float current = 0.0f;
    float max = 0.0f;
    if (EquipmentStats_ReadStaminaSnapshot(current, max))
    {
        auto* gaugeClass = FindClass(
            "Common.CommonUI.CommonGauge",
            "GaugeView");
        auto* setMax = FindMethod(gaugeClass, "SetMax", 1);
        auto* setValue = FindMethod(gaugeClass, "Set", 1);
        if (setMax != nullptr &&
            setValue != nullptr &&
            std::fabs(max - g_gameplayOriginalStaminaGaugeMax) > 0.0001f)
        {
            // GaugeView.SetMax early-outs when the value is unchanged. A clone
            // can carry max=1 while its internal length fields were not
            // initialized by Awake, leaving GaugeFront at width 0. Invalidate
            // the cached max once so SetMax rebuilds lengthScale/length.
            auto* cachedMax = reinterpret_cast<float*>(
                reinterpret_cast<char*>(gauge) + 0x48);
            if (std::fabs(*cachedMax - max) <= 0.0001f)
            {
                *cachedMax = 0.0f;
            }
            void* maxParameters[] = { &max };
            Invoke(setMax, gauge, maxParameters);
            g_gameplayOriginalStaminaGaugeMax = max;
        }
        if (setValue != nullptr)
        {
            void* valueParameters[] = { &current };
            Invoke(setValue, gauge, valueParameters);
        }
        if (!g_gameplayOriginalStaminaGaugeSyncLogged)
        {
            g_gameplayOriginalStaminaGaugeSyncLogged = true;
            LogInfo(
                LogCategory_Ui,
                L"GameplayStaminaGauge",
                L"Cloned stamina gauge synchronized from StaminaManager: "
                L"current=%.4f max=%.4f.",
                current,
                max);
        }
    }
}

void GameplayStatusHud_ShowNotification(
    const char* message,
    unsigned int durationMs)
{
    if (message == nullptr || message[0] == '\0')
    {
        return;
    }
    g_gameplayStatusHudNotification = message;
    g_gameplayStatusHudNotificationUntil =
        GetTickCount64() + durationMs;
    LogDebug(
        LogCategory_Ui,
        L"GameplayNotification",
        L"Gameplay notification shown for %u ms: %S.",
        durationMs,
        message);
}

Il2CppObject* GameplayStatusHud_GetPlayerClassAccessor()
{
    return g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
}

Il2CppObject* GameplayStatusHud_GetPlayerState()
{
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    return playerClassAccessor != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x60)
        : nullptr;
}

struct GameplayActionSnapshot
{
    Il2CppObject* actionRecord = nullptr;
    int actionType = -1;
    bool active = false;
};

GameplayActionSnapshot GameplayAction_ReadCurrent()
{
    GameplayActionSnapshot snapshot{};
    auto* playerState = GameplayStatusHud_GetPlayerState();
    if (playerState == nullptr)
    {
        return snapshot;
    }
    snapshot.actionRecord = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerState) + 0x48);
    if (snapshot.actionRecord == nullptr)
    {
        return snapshot;
    }
    snapshot.actionType = *reinterpret_cast<int*>(
        reinterpret_cast<char*>(snapshot.actionRecord) + 0x18);
    snapshot.active = snapshot.actionType >= 0;
    return snapshot;
}

struct GameplayStatusHudMovementState
{
    bool valid = false;
    float moveSpeed = 0.0f;
    bool isDash = false;
    bool isCrouch = false;
};

bool GameplayStatusHud_ReadMovementState(
    GameplayStatusHudMovementState& output)
{
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    auto* playerState = GameplayStatusHud_GetPlayerState();
    if (playerClassAccessor == nullptr || playerState == nullptr)
    {
        return false;
    }

    auto* locomotion = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerClassAccessor) + 0x58);
    auto* moveSpeed = locomotion != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(locomotion) + 0x28)
        : nullptr;
    output.moveSpeed = moveSpeed != nullptr
        ? *reinterpret_cast<float*>(
            reinterpret_cast<char*>(moveSpeed) + 0x10)
        : 0.0f;
    output.isDash = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x19);
    output.isCrouch = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x1B);
    output.valid = true;
    return true;
}

bool GameplayStatusHud_IsAdultGoodsEquipped(int adultGoodsType)
{
    static const Il2CppMethod* isEquipped = nullptr;
    if (isEquipped == nullptr)
    {
        isEquipped = FindMethod(
            FindClass("ExposureUnnoticed2.Scripts.InGame", "GameStateFacade"),
            "IsEquipAdultGoods",
            1);
    }
    if (isEquipped == nullptr)
    {
        return false;
    }
    int type = adultGoodsType;
    void* parameters[] = { &type };
    auto* value = Invoke(isEquipped, nullptr, parameters);
    return value != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(value));
}

bool GameplayStatusHud_IsVibratorStrong()
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (gameStateData == nullptr ||
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(gameStateData) + 0x1B8) != 2)
    {
        return false;
    }
    return
        GameplayStatusHud_IsAdultGoodsEquipped(1) ||
        GameplayStatusHud_IsAdultGoodsEquipped(6) ||
        GameplayStatusHud_IsAdultGoodsEquipped(7);
}

bool GameplayStatusHud_IsPlayerInsideElevator()
{
    auto* elevator = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.ScenePlops.Elevator",
        "ElevatorController",
        "Instance");
    if (elevator == nullptr)
    {
        return false;
    }
    auto* isInside = FindMethod(
        FindClass(
            "ExposureUnnoticed2.Object3D.ScenePlops.Elevator",
            "ElevatorController"),
        "IsInsidePlayer",
        0);
    auto* value = isInside != nullptr
        ? Invoke(isInside, elevator, nullptr)
        : nullptr;
    return value != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(value));
}

const char* GameplayStatusHud_GetActionLabel(int actionType)
{
    for (int index = 0; index < kExtendedPoseMenuOptionCount; ++index)
    {
        if (kExtendedPoseMenuOptions[index].actionType == actionType)
        {
            return kExtendedPoseMenuOptions[index].label;
        }
    }
    switch (actionType)
    {
    case 10000: return "捡大衣";
    case 10001: return "拾取";
    case 10002: return "丢弃";
    case 10003: return "换衣";
    case 10006: return "插入肛塞";
    case 10007: return "取出肛塞";
    case 10008: return "通用装备";
    case 10011: return "使用售货机";
    case 10012: return "喝水";
    case 10019: return "戴手铐";
    case 10020: return "把手铐放到面前";
    case 10021: return "被锁在面前物体";
    case 10022: return "解锁面前手铐";
    case 10023: return "戴眼罩";
    case 10024: return "切换振动器";
    case 10025: return "拾取物品";
    case 10026: return "坐在长椅上";
    case 10027: return "从长椅起身";
    default: return nullptr;
    }
}

const char* const kGameplayControlsHintLabels[kGameplayControlsHintCount] = {
    "<color=#8CE7FF>W/A/S/D</color>  移动",
    "<color=#8CE7FF>Q</color>  切换露出模式",
    "<color=#8CE7FF>E</color>  使用动作",
    "<color=#8CE7FF>F</color>  交互",
    "<color=#8CE7FF>T</color>  搭讪",
    "<color=#8CE7FF>R</color>  切换视角",
    "<color=#8CE7FF>Tab</color>  装备栏",
    "<color=#8CE7FF>M</color>  小地图",
    "<color=#8CE7FF>C</color>  蹲下",
};

void GameplayControlsHint_Build()
{
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    if (ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        SetWorldGameObjectActive(
            g_gameplayControlsHintPanel.gameObject,
            true);
        return;
    }

    g_gameplayControlsHintPanel = SceneUiElement{};
    for (auto& row : g_gameplayControlsHintRows)
    {
        row = SceneUiElement{};
    }
    for (auto*& text : g_gameplayControlsHintTexts)
    {
        text = nullptr;
    }
    if (g_fullScUnityFont == nullptr)
    {
        ExtendedMode_CacheFont();
    }
    auto* rootTransform = GetWorldTransform(
        g_extendedGameplayHudCanvasObject);
    if (rootTransform == nullptr)
    {
        return;
    }

    g_gameplayControlsHintPanel = CreateScenePanel(
        "EscapeFromManakaControlsHint",
        rootTransform,
        0.016f,
        0.020f,
        0.178f,
        0.106f,
        0.0f,
        0.0f,
        0.0f,
        0.0f);
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        return;
    }
    for (int index = 0; index < kGameplayControlsHintCount; ++index)
    {
        const int column = index % 3;
        const int row = index / 3;
        constexpr float cellWidth = 0.309f;
        constexpr float columnStep = 0.327f;
        constexpr float cellHeight = 0.302f;
        constexpr float rowStep = 0.337f;
        const float minX = 0.015f + column * columnStep;
        const float maxX = minX + cellWidth;
        const float maxY = 0.985f - static_cast<float>(row) * rowStep;
        const float minY = maxY - cellHeight;
        char backgroundName[64]{};
        char textName[64]{};
        std::snprintf(
            backgroundName,
            sizeof(backgroundName),
            "ControlsHintRow%02d",
            index);
        std::snprintf(
            textName,
            sizeof(textName),
            "ControlsHintText%02d",
            index);
        g_gameplayControlsHintRows[index] = CreateScenePanel(
            backgroundName,
            g_gameplayControlsHintPanel.rect,
            minX,
            minY,
            maxX,
            maxY,
            0.030f,
            0.043f,
            0.060f,
            0.78f);
        g_gameplayControlsHintTexts[index] = CreateSceneTextElement(
            textName,
            g_gameplayControlsHintRows[index].rect,
            kGameplayControlsHintLabels[index],
            0.035f,
            0.04f,
            0.965f,
            0.96f,
            14.0f,
            257,
            0.90f,
            0.95f,
            1.0f,
            1.0f).text;
    }
    SetWorldGameObjectActive(
        g_gameplayControlsHintPanel.gameObject,
        true);
    LogInfo(LogCategory_Ui, L"Ui", L"Gameplay controls hint created.");
}

void GameplayControlsHint_Reset()
{
    g_gameplayControlsHintPanel = SceneUiElement{};
    for (auto& row : g_gameplayControlsHintRows)
    {
        row = SceneUiElement{};
    }
    for (auto*& text : g_gameplayControlsHintTexts)
    {
        text = nullptr;
    }
}

void GameplayControlsHint_Update()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (ExtendedMode_IsUnityObjectAlive(
                g_gameplayControlsHintPanel.gameObject))
        {
            SetWorldGameObjectActive(
                g_gameplayControlsHintPanel.gameObject,
                false);
        }
        return;
    }
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        GameplayControlsHint_Build();
    }
    if (!ExtendedMode_IsUnityObjectAlive(
            g_gameplayControlsHintPanel.gameObject))
    {
        return;
    }
    const bool hiddenByModal =
        GameplayInput_IsCustomModalUiOpen() ||
        g_extendedOriginalTutorialActive.load() ||
        g_extendedOriginalSystemMenuVisible.load() ||
        g_extendedOriginalInteractionMenuActive.load() ||
        g_extendedOriginalHandcuffsPanelActive.load();
    SetWorldGameObjectActive(
        g_gameplayControlsHintPanel.gameObject,
        !hiddenByModal);
}

void GameplayStatusHud_Build()
{
    if (!ExtendedMode_IsUnityObjectAlive(g_extendedGameplayHudCanvasObject))
    {
        return;
    }
    if (ExtendedMode_IsUnityObjectAlive(g_gameplayStatusHudPanel.gameObject) &&
        static_cast<int>(g_gameplayStatusHudRows.size()) ==
            kGameplayStatusHudMaxRows)
    {
        GameplayStatusHud_BuildOwnedStaminaBar(
            GetWorldTransform(g_extendedGameplayHudCanvasObject));
        SetWorldGameObjectActive(g_gameplayStatusHudPanel.gameObject, true);
        return;
    }

    g_gameplayStatusHudPanel = SceneUiElement{};
    g_gameplayStatusHudRows.clear();
    g_gameplayStatusHudLastUpdateTick = 0;
    auto* rootTransform = GetWorldTransform(g_extendedGameplayHudCanvasObject);
    GameplayStatusHud_BuildOwnedStaminaBar(rootTransform);
    g_gameplayStatusHudPanel = CreateScenePanel(
        "EscapeFromManakaStatusHud",
        rootTransform,
        0.016f,
        0.120f,
        0.305f,
        0.520f,
        0.012f,
        0.018f,
        0.026f,
        0.0f);
    if (g_gameplayStatusHudPanel.gameObject == nullptr)
    {
        return;
    }
    g_gameplayStatusHudRows.reserve(kGameplayStatusHudMaxRows);
    for (int index = 0; index < kGameplayStatusHudMaxRows; ++index)
    {
        char backgroundName[64]{};
        char textName[64]{};
        std::snprintf(
            backgroundName,
            sizeof(backgroundName),
            "StatusRowBackground%02d",
            index);
        std::snprintf(
            textName,
            sizeof(textName),
            "StatusRowText%02d",
            index);
        const float minY = 0.010f + static_cast<float>(index) * 0.071f;
        const float maxY = minY + 0.064f;
        auto background = CreateScenePanel(
            backgroundName,
            g_gameplayStatusHudPanel.rect,
            0.012f,
            minY,
            0.585f,
            maxY,
            0.034f,
            0.048f,
            0.066f,
            0.76f);
        auto row = CreateSceneTextElement(
            textName,
            background.rect,
            "",
            0.055f,
            0.09f,
            0.95f,
            0.91f,
            15.0f,
            257,
            0.88f,
            0.93f,
            0.98f,
            1.0f);
        SetWorldGameObjectActive(background.gameObject, false);
        GameplayStatusHudRowState rowState{};
        rowState.background = background;
        rowState.element = row;
        g_gameplayStatusHudRows.push_back(rowState);
    }
    SetWorldGameObjectActive(g_gameplayStatusHudPanel.gameObject, true);
    LogInfo(LogCategory_Ui, L"Ui", L"Gameplay status listener HUD created.");
}

void GameplayStatusHud_Reset()
{
    ExtendedMode_DestroyUnityObject(g_gameplayOriginalStaminaGaugeObject);
    g_gameplayStatusHudPanel = SceneUiElement{};
    g_gameplayStatusHudRows.clear();
    g_gameplayStatusHudLastUpdateTick = 0;
    g_gameplayStatusHudNotification.clear();
    g_gameplayStatusHudNotificationUntil = 0;
    g_gameplayOriginalStaminaGaugeObject = nullptr;
    g_gameplayOriginalStaminaGaugeComponent = nullptr;
    g_gameplayOriginalStaminaGaugeLayoutTick = 0;
    g_gameplayOriginalStaminaGaugeMax = -1.0f;
    g_gameplayOriginalStaminaGaugeLayoutLogged = false;
    g_gameplayOriginalStaminaGaugeSyncLogged = false;
    g_gameplayStaminaBarBackground = SceneUiElement{};
    g_gameplayStaminaBarFill = SceneUiElement{};
    g_gameplayStaminaBarWidth = -1.0f;
    g_gameplayStaminaBarCapacityMultiplier = -1.0f;
    GameplayControlsHint_Reset();
}

void GameplayStatusHud_Update()
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (ExtendedMode_IsUnityObjectAlive(
                g_gameplayStatusHudPanel.gameObject))
        {
            SetWorldGameObjectActive(
                g_gameplayStatusHudPanel.gameObject,
                false);
        }
        return;
    }
    EquipmentStats_UpdateExposureReward();
    GameplayStatusHud_UpdateOwnedStaminaBar();
    GameplayStatusHud_HideResidualGauges();
    GameplayControlsHint_Update();
    if (!ExtendedMode_IsUnityObjectAlive(g_gameplayStatusHudPanel.gameObject))
    {
        GameplayStatusHud_Build();
    }
    auto* playerClassAccessor = GameplayStatusHud_GetPlayerClassAccessor();
    auto* playerState = GameplayStatusHud_GetPlayerState();
    if (playerClassAccessor == nullptr || playerState == nullptr)
    {
        LogOnce(
            "gameplay-status-hud-player-missing",
            LogLevel_Debug,
            LogCategory_Ui,
            L"HudRead",
            L"Gameplay status HUD skipped because player state references are unavailable.");
        return;
    }

    GameplayStatusHudMovementState movementState{};
    if (!GameplayStatusHud_ReadMovementState(movementState))
    {
        LogOnce(
            "gameplay-status-hud-movement-missing",
            LogLevel_Debug,
            LogCategory_Ui,
            L"HudRead",
            L"Gameplay status HUD could not read the locomotion movement state.");
        return;
    }
    const float currentMoveSpeed = movementState.moveSpeed;
    const bool isDash = movementState.isDash;
    const bool isCrouch = movementState.isCrouch;
    const bool nearNpc = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x88);
    const bool ecstasy = *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x50);
    const float streetLightBareyasusa = *reinterpret_cast<float*>(
        reinterpret_cast<char*>(playerState) + 0x9C);
    auto* touchedLights = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerState) + 0x138);
    const int touchedLightCount = touchedLights != nullptr
        ? *reinterpret_cast<int*>(
            reinterpret_cast<char*>(touchedLights) + 0x18)
        : 0;
    const auto currentAction = GameplayAction_ReadCurrent();
    const int currentActionType = currentAction.actionType;
    auto* lastChair = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(playerState) + 0x1A8);
    const bool onChair =
        lastChair != nullptr &&
        ExtendedMode_IsUnityObjectAlive(lastChair) &&
        *reinterpret_cast<int*>(
            reinterpret_cast<char*>(lastChair) + 0x78) > 0;

    auto* gameStateData = ExtendedMode_GetGameStateData();
    const bool handsLocked =
        gameStateData != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A8);
    const bool lockedToObject =
        gameStateData != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(gameStateData) + 0x1A9);
    const int vibrationMode =
        gameStateData != nullptr
            ? *reinterpret_cast<int*>(
                reinterpret_cast<char*>(gameStateData) + 0x1B8)
            : 0;

    std::vector<std::string> lines;
    lines.reserve(kGameplayStatusHudMaxRows);
    lines.emplace_back(
        WideToUtf8(
            L"本局获得金钱：" +
            InventoryUi_FormatCurrency(
                EquipmentStats_GetRaidEarnings())));
    const unsigned long long notificationNow = GetTickCount64();
    if (!g_gameplayStatusHudNotification.empty())
    {
        if (notificationNow < g_gameplayStatusHudNotificationUntil)
        {
            lines.emplace_back(g_gameplayStatusHudNotification);
        }
        else
        {
            g_gameplayStatusHudNotification.clear();
            g_gameplayStatusHudNotificationUntil = 0;
        }
    }
    if (isCrouch)
    {
        lines.emplace_back(
            currentMoveSpeed > 0.02f ? "移动：爬行" : "移动：蹲伏");
    }
    else if (currentMoveSpeed > 0.02f)
    {
        lines.emplace_back(isDash ? "移动：奔跑" : "移动：行走");
    }
    else
    {
        lines.emplace_back("移动：静止");
    }

    lines.emplace_back(
        g_extendedSpecialCoatMode.load() ? "大衣状态" : "裸露状态");
    if (GameplayStatusHud_IsAdultGoodsEquipped(2))
    {
        lines.emplace_back("佩戴眼罩");
    }
    const bool vibrationActive =
        vibrationMode > 0 &&
        (GameplayStatusHud_IsAdultGoodsEquipped(1) ||
         GameplayStatusHud_IsAdultGoodsEquipped(6) ||
         GameplayStatusHud_IsAdultGoodsEquipped(7));
    if (vibrationActive)
    {
        const char* mode = vibrationMode == 1
            ? "弱"
            : (vibrationMode == 2
                ? "强"
                : (vibrationMode == 3 ? "随机" : "未知"));
        char line[96]{};
        std::snprintf(line, sizeof(line), "振动器：%s", mode);
        lines.emplace_back(line);
    }
    if (currentActionType >= 0)
    {
        const char* label = GameplayStatusHud_GetActionLabel(
            currentActionType);
        char line[160]{};
        if (label != nullptr)
        {
            std::snprintf(line, sizeof(line), "动作：%s", label);
        }
        else
        {
            std::snprintf(line, sizeof(line), "动作：#%d", currentActionType);
        }
        lines.emplace_back(line);
    }
    if (handsLocked)
    {
        lines.emplace_back("双手被锁住");
    }
    if (lockedToObject)
    {
        lines.emplace_back("被锁在面前物体");
    }
    if (nearNpc)
    {
        lines.emplace_back("路人身旁");
    }
    if (ecstasy)
    {
        lines.emplace_back("高潮状态");
    }
    if (streetLightBareyasusa > 0.0f || touchedLightCount > 0)
    {
        lines.emplace_back("在光源下");
    }
    if (GameplayStatusHud_IsPlayerInsideElevator())
    {
        lines.emplace_back("在电梯内");
    }
    if (onChair)
    {
        lines.emplace_back("在长椅上");
    }
    if (currentActionType == 10011)
    {
        lines.emplace_back("在使用售货机");
    }

    if (lines.size() > static_cast<size_t>(kGameplayStatusHudMaxRows))
    {
        lines.resize(kGameplayStatusHudMaxRows);
    }
    SetWorldGameObjectActive(g_gameplayStatusHudPanel.gameObject, true);
    const unsigned long long now = GetTickCount64();
    float deltaSeconds = g_gameplayStatusHudLastUpdateTick == 0
        ? 0.0f
        : static_cast<float>(
            now - g_gameplayStatusHudLastUpdateTick) / 1000.0f;
    g_gameplayStatusHudLastUpdateTick = now;
    deltaSeconds = std::clamp(deltaSeconds, 0.0f, 0.1f);
    const float alphaStep =
        deltaSeconds / kGameplayStatusHudFadeSeconds;

    for (int index = 0; index < kGameplayStatusHudMaxRows; ++index)
    {
        auto& row = g_gameplayStatusHudRows[static_cast<size_t>(index)];
        const std::string desiredText =
            index < static_cast<int>(lines.size())
                ? lines[static_cast<size_t>(index)]
                : std::string();

        if (index == 0)
        {
            row.text = desiredText;
            row.alpha = 1.0f;
            row.targetAlpha = 1.0f;
        }
        else if (row.text != desiredText)
        {
            if (row.alpha > 0.0f)
            {
                row.targetAlpha = 0.0f;
            }
            else
            {
                row.text = desiredText;
                row.alpha = 0.0f;
                row.targetAlpha = desiredText.empty() ? 0.0f : 1.0f;
            }
        }
        else
        {
            row.targetAlpha = desiredText.empty() ? 0.0f : 1.0f;
        }

        if (row.alpha < row.targetAlpha)
        {
            row.alpha = std::min(row.targetAlpha, row.alpha + alphaStep);
        }
        else if (row.alpha > row.targetAlpha)
        {
            row.alpha = std::max(row.targetAlpha, row.alpha - alphaStep);
        }

        if (row.alpha <= 0.0f && row.targetAlpha <= 0.0f &&
            row.text != desiredText)
        {
            row.text = desiredText;
            row.targetAlpha = desiredText.empty() ? 0.0f : 1.0f;
        }

        const bool visible = !row.text.empty();
        SetWorldGameObjectActive(row.background.gameObject, visible);
        if (!visible)
        {
            row.renderedText.clear();
            row.renderedAlpha = -1.0f;
            continue;
        }
        if (row.renderedText != row.text ||
            row.renderedAlpha != row.alpha)
        {
            SetSceneGraphicColor(
                row.background.image,
                index == 0 ? 0.045f : 0.055f,
                index == 0 ? 0.220f : 0.260f,
                index == 0 ? 0.085f : 0.105f,
                row.alpha * 0.76f);
            SetSceneText(
                row.element.text,
                row.text.c_str(),
                index == 0 ? 16.0f : 15.0f,
                257,
                index == 0 ? 0.48f : 0.88f,
                index == 0 ? 0.92f : 0.93f,
                index == 0 ? 1.0f : 0.98f,
                row.alpha);
            row.renderedText = row.text;
            row.renderedAlpha = row.alpha;
        }
    }
}
