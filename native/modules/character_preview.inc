// Native.cpp module: character_preview.inc
// Included inside the shared implementation namespace by Native.cpp.
Il2CppObject* CreateWorldGameObject(const char* name)
{
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (gameObjectClass == nullptr || il2cpp_object_new == nullptr)
    {
        return nullptr;
    }
    auto* gameObject = il2cpp_object_new(gameObjectClass);
    auto* nameValue = il2cpp_string_new(name);
    void* constructorParameters[] = { nameValue };
    Invoke(
        FindMethod(gameObjectClass, ".ctor", 1),
        gameObject,
        constructorParameters);
    return gameObject;
}

Il2CppObject* GetWorldTransform(Il2CppObject* gameObject)
{
    return gameObject != nullptr
        ? Invoke(
            FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
            gameObject,
            nullptr)
        : nullptr;
}

void SetTransformEuler(Il2CppObject* transform, float x, float y, float z)
{
    if (transform == nullptr)
    {
        return;
    }
    float value[] = { x, y, z };
    void* parameters[] = { value };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "set_eulerAngles", 1),
        transform,
        parameters);
}

void SetTransformLocalEuler(Il2CppObject* transform, float x, float y, float z)
{
    if (transform == nullptr)
    {
        return;
    }
    float value[] = { x, y, z };
    void* parameters[] = { value };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "set_localEulerAngles", 1),
        transform,
        parameters);
}

void SetTransformLocalPosition(Il2CppObject* transform, float x, float y, float z)
{
    if (transform == nullptr)
    {
        return;
    }
    float value[] = { x, y, z };
    void* parameters[] = { value };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "set_localPosition", 1),
        transform,
        parameters);
}

void SetWorldGameObjectActive(Il2CppObject* gameObject, bool active)
{
    if (gameObject == nullptr ||
        !ExtendedMode_IsUnityObjectAlive(gameObject))
    {
        return;
    }
    void* parameters[] = { &active };
    Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "SetActive", 1),
        gameObject,
        parameters);
}

bool NameContainsAny(
    const std::wstring& value,
    const std::initializer_list<const wchar_t*>& tokens)
{
    for (const auto* token : tokens)
    {
        if (value.find(token) != std::wstring::npos)
        {
            return true;
        }
    }
    return false;
}

void HidePreviewClothing(Il2CppObject* rootGameObject)
{
    auto* rendererClass = FindClass("UnityEngine", "SkinnedMeshRenderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (rootGameObject == nullptr || rendererClass == nullptr || gameObjectClass == nullptr)
    {
        return;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(rendererClass);
    }
    auto* rendererType = il2cpp_type_get_object(il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* findParameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        rootGameObject,
        findParameters);
    if (renderers == nullptr)
    {
        return;
    }

    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) + il2cpp_array_object_header_size());
    int hidden = 0;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        if (renderer == nullptr)
        {
            continue;
        }
        auto* gameObject = GetComponentGameObject(renderer);
        const auto name = GetObjectName(gameObject);
        const bool isBodyPart = NameContainsAny(
            name,
            {
                L"Body", L"Face", L"Hair", L"Head", L"Eye", L"Brow",
                L"Lash", L"Cheek", L"Tongue", L"Tooth", L"Manustia",
                L"Selestia_body"
            });
        const bool hide = !isBodyPart;
        if (hide)
        {
            bool enabled = false;
            void* parameters[] = { &enabled };
            Invoke(
                FindMethod(FindClass("UnityEngine", "Renderer"), "set_enabled", 1),
                renderer,
                parameters);
            ++hidden;
        }
    }
    LogInfo(LogCategory_Scene, L"Scene", L"Character preview hid %d clothing/accessory renderer(s).", hidden);
}

void HidePreviewFutanari(Il2CppObject* rootGameObject)
{
    if (rootGameObject == nullptr)
    {
        return;
    }

    auto* modelTransform = GetWorldTransform(rootGameObject);
    auto* futanariRoot = FindChildGameObjectByName(
        modelTransform,
        L"otinpo");
    if (futanariRoot == nullptr)
    {
        futanariRoot = FindChildGameObjectByName(
            modelTransform,
            L"Futanari");
    }
    if (futanariRoot == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Character preview futanari attachment was not found.");
        return;
    }

    SetWorldGameObjectActive(futanariRoot, false);
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Character preview disabled futanari attachment %s.",
        GetObjectName(futanariRoot).c_str());
}

Il2CppObject* CreatePreviewRenderTexture(int width, int height, int depth)
{
    auto* renderTextureClass = FindClass("UnityEngine", "RenderTexture");
    if (renderTextureClass == nullptr || il2cpp_object_new == nullptr)
    {
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(renderTextureClass);
    }
    auto* texture = il2cpp_object_new(renderTextureClass);
    void* constructorParameters[] = { &width, &height, &depth };
    Invoke(
        FindMethod(renderTextureClass, ".ctor", 3),
        texture,
        constructorParameters);
    Invoke(FindMethod(renderTextureClass, "Create", 0), texture, nullptr);
    return texture;
}

Il2CppObject* LoadCharacterPrefabFromBundleCopy()
{
    LogInfo(LogCategory_Scene, L"Scene", L"Runtime character-prefab bundle probing is disabled until a safe scene lifecycle is selected.");
    return nullptr;

    if (g_previewAssetBundle != nullptr)
    {
        return nullptr;
    }

    wchar_t tempDirectory[MAX_PATH]{};
    if (GetTempPathW(MAX_PATH, tempDirectory) == 0)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Unable to resolve the temporary directory for the asset bundle.");
        return nullptr;
    }
    const std::wstring source = GetModuleDirectory() +
        L"\\SecretFlasherManaka_Data\\data.unity3d";
    const std::wstring destination =
        std::wstring(tempDirectory) + L"ManakaMultiplayer.data.unity3d";
    if (!CopyFileW(source.c_str(), destination.c_str(), FALSE))
    {
        LogInfo(LogCategory_Scene, L"Scene", 
            L"Could not copy data.unity3d for preview loading. Win32 error %lu.",
            GetLastError());
        return nullptr;
    }

    auto* pathValue = il2cpp_string_new(WideToUtf8(destination).c_str());
    void* loadParameters[] = { pathValue };
    g_previewAssetBundle = Invoke(
        FindMethod(FindClass("UnityEngine", "AssetBundle"), "LoadFromFile", 1),
        nullptr,
        loadParameters);
    LogInfo(LogCategory_Scene, L"Scene", L"Preview AssetBundle copy: %p", g_previewAssetBundle);
    if (g_previewAssetBundle == nullptr)
    {
        return nullptr;
    }

    auto* npcAvatarClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAvatarComponent");
    if (npcAvatarClass == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"NpcAvatarComponent class is unavailable for asset filtering.");
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(npcAvatarClass);
    }
    auto* npcAvatarType = il2cpp_type_get_object(
        il2cpp_class_get_type(npcAvatarClass));
    void* loadAllParameters[] = { npcAvatarType };
    auto* assets = Invoke(
        FindMethod(FindClass("UnityEngine", "AssetBundle"), "LoadAllAssets", 1),
        g_previewAssetBundle,
        loadAllParameters);
    if (assets == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"LoadAllAssets(NpcAvatarComponent) returned no assets.");
        return nullptr;
    }

    const auto count = il2cpp_array_length(assets);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(assets) + il2cpp_array_object_header_size());
    LogInfo(LogCategory_Scene, L"Scene", L"Loaded %u NpcAvatarComponent prefab asset(s).", count);
    Il2CppObject* fallback = nullptr;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* component = elements[index];
        auto* gameObject = component != nullptr
            ? GetComponentGameObject(component)
            : nullptr;
        if (gameObject == nullptr)
        {
            continue;
        }
        const auto name = GetObjectName(gameObject);
        if (index < 16)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"NPC avatar prefab[%u]: %s", index, name.c_str());
        }
        if (name.find(L"NpcPreviousFemale") != std::wstring::npos ||
            name.find(L"NpcFemale_Assemble") != std::wstring::npos)
        {
            return gameObject;
        }
        if (fallback == nullptr &&
            (name.find(L"Female") != std::wstring::npos ||
             name.find(L"female") != std::wstring::npos))
        {
            fallback = gameObject;
        }
    }
    return fallback;
}

Il2CppObject* LoadCharacterPrefab()
{
    auto* resourcesClass = FindClass("UnityEngine", "Resources");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* gameObjectType = il2cpp_type_get_object(il2cpp_class_get_type(gameObjectClass));
    for (const char* path : { "NpcPreviousFemale", "NpcFemale_Assemble", "UiPlayer" })
    {
        auto* pathValue = il2cpp_string_new(path);
        void* parameters[] = { pathValue, gameObjectType };
        auto* prefab = Invoke(
            FindMethod(resourcesClass, "Load", 2),
            nullptr,
            parameters);
        LogInfo(LogCategory_Scene, L"Scene", L"Resources.Load(%S): %p", path, prefab);
        if (prefab != nullptr)
        {
            return prefab;
        }
    }
    return LoadCharacterPrefabFromBundleCopy();
}

Il2CppObject* FindFirstComponentInChildrenByClass(
    Il2CppObject* gameObject,
    const char* namespaceName,
    const char* className)
{
    auto* componentClass = FindClass(namespaceName, className);
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (gameObject == nullptr || componentClass == nullptr || gameObjectClass == nullptr)
    {
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(componentClass);
    }
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(componentClass));
    bool includeInactive = true;
    void* parameters[] = { managedType, &includeInactive };
    auto* components = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        gameObject,
        parameters);
    if (components == nullptr || il2cpp_array_length(components) == 0)
    {
        return nullptr;
    }
    return reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(components) + il2cpp_array_object_header_size())[0];
}

void BuildCharacterPreview(
    Il2CppObject* leftPanelTransform,
    Il2CppObject* providedModel = nullptr)
{
    if (leftPanelTransform == nullptr || g_previewStageComponent != nullptr)
    {
        return;
    }

    g_previewStageComponent = CreateWorldGameObject("CharacterPreviewStage");
    auto* stageTransform = GetWorldTransform(g_previewStageComponent);
    SetTransformLocalPosition(stageTransform, 10000.0f, 0.0f, 0.0f);

    if (providedModel != nullptr)
    {
        g_previewModel = providedModel;
        SetWorldGameObjectActive(g_previewModel, true);
    }
    else
    {
        auto* prefab = LoadCharacterPrefab();
        if (prefab == nullptr)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"No character prefab could be loaded from Resources.");
            return;
        }
        void* instantiateParameters[] = { prefab };
        g_previewModel = Invoke(
            FindMethod(FindClass("UnityEngine", "Object"), "Instantiate", 1),
            nullptr,
            instantiateParameters);
        if (g_previewModel == nullptr)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"Character prefab instantiation failed.");
            return;
        }
        SetWorldGameObjectActive(g_previewModel, true);
    }
    auto* modelTransform = GetWorldTransform(g_previewModel);
    bool keepWorldPosition = false;
    void* parentParameters[] = { stageTransform, &keepWorldPosition };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "SetParent", 2),
        modelTransform,
        parentParameters);
    SetTransformLocalPosition(modelTransform, 0.0f, 0.0f, 0.0f);
    SetTransformEuler(modelTransform, 0.0f, 180.0f, 0.0f);

    if (g_efmPreviewIdleClip != nullptr)
    {
        float sampleTime = 0.1f;
        void* sampleParameters[] = { g_previewModel, &sampleTime };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "AnimationClip"),
                "SampleAnimation",
                2),
            g_efmPreviewIdleClip,
            sampleParameters);
        LogInfo(LogCategory_Scene, L"Scene", 
            L"EscapeFromManaka_preparing sampled idle clip %s at %.2f.",
            GetObjectName(g_efmPreviewIdleClip).c_str(),
            sampleTime);
    }

    int remainingSkeletonNodes = 90;
    LogSkeletonTransforms(modelTransform, 0, remainingSkeletonNodes);

    HidePreviewClothing(g_previewModel);
    HidePreviewFutanari(g_previewModel);

    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(animatorClass);
    }
    auto* animatorType = il2cpp_type_get_object(
        il2cpp_class_get_type(animatorClass));
    bool includeInactiveAnimators = true;
    void* animatorFindParameters[] = {
        animatorType,
        &includeInactiveAnimators
    };
    auto* animators = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_previewModel,
        animatorFindParameters);
    const auto animatorCount = animators != nullptr
        ? il2cpp_array_length(animators)
        : 0;
    if (animatorCount > 0)
    {
        auto* animatorElements = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(animators) +
            il2cpp_array_object_header_size());
        bool applyRootMotion = false;
        int cullingMode = 0;
        float speed = 1.0f;
        float deltaTime = 0.016f;
        void* rootMotionParameters[] = { &applyRootMotion };
        void* cullingParameters[] = { &cullingMode };
        void* speedParameters[] = { &speed };
        void* updateParameters[] = { &deltaTime };
        for (uint32_t index = 0; index < animatorCount; ++index)
        {
            auto* animator = animatorElements[index];
            if (animator == nullptr)
            {
                continue;
            }
            if (g_previewAnimator == nullptr)
            {
                g_previewAnimator = animator;
            }
            const auto animatorObjectName = GetObjectName(
                GetComponentGameObject(animator));
            if (index == 0)
            {
                LogObjectReference(
                    "Character preview existing avatar",
                    Invoke(
                        FindMethod(animatorClass, "get_avatar", 0),
                        animator,
                        nullptr));
            }
            auto* runtimeController = Invoke(
                FindMethod(animatorClass, "get_runtimeAnimatorController", 0),
                animator,
                nullptr);
            LogObjectReference(
                "Character preview runtime controller",
                runtimeController);
            int layerIndex = 0;
            void* layerNameParameters[] = { &layerIndex };
            auto* layerNameValue = Invoke(
                FindMethod(animatorClass, "GetLayerName", 1),
                animator,
                layerNameParameters);
            LogInfo(LogCategory_Scene, L"Scene", 
                L"Character preview Animator[%u] on %s layer0=%s",
                index,
                animatorObjectName.c_str(),
                ReadIl2CppString(static_cast<Il2CppString*>(layerNameValue)).c_str());
            Invoke(
                FindMethod(animatorClass, "set_applyRootMotion", 1),
                animator,
                rootMotionParameters);
            Invoke(
                FindMethod(animatorClass, "set_cullingMode", 1),
                animator,
                cullingParameters);
            Invoke(
                FindMethod(animatorClass, "set_speed", 1),
                animator,
                speedParameters);
            Invoke(
                FindMethod(animatorClass, "Update", 1),
                animator,
                updateParameters);
            LogInfo(LogCategory_Scene, L"Scene", 
                L"Character preview Animator[%u] on %s set to standing idle.",
                index,
                GetObjectName(GetComponentGameObject(animator)).c_str());
        }
        LogInfo(LogCategory_Scene, L"Scene", L"Character preview initialized %u Animator component(s).", animatorCount);
    }
    else
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Character prefab has no Animator component.");
    }

    if (g_efmPreviewController == nullptr)
    {
        ApplyEscapeFromManakaStandingPose(modelTransform);
    }

    g_previewRenderTexture = CreatePreviewRenderTexture(768, 1024, 24);
    if (g_previewRenderTexture == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Character preview RenderTexture creation failed.");
        return;
    }

    auto* previewSurface = CreateSceneGameObject(
        "CharacterPreviewSurface",
        leftPanelTransform);
    auto* previewRect = GetSceneComponentByClass(
        previewSurface,
        "UnityEngine",
        "RectTransform");
    auto* rawImage = AddSceneComponentToGameObject(
        previewSurface,
        "UnityEngine.UI",
        "RawImage");
    SetSceneRect(previewRect, 0.03f, 0.30f, 0.97f, 0.86f);
    void* textureParameters[] = { g_previewRenderTexture };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "RawImage"), "set_texture", 1),
        rawImage,
        textureParameters);
    SetSceneGraphicColor(rawImage, 1.0f, 1.0f, 1.0f, 1.0f);
    bool raycastTarget = false;
    void* raycastParameters[] = { &raycastTarget };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Graphic"), "set_raycastTarget", 1),
        rawImage,
        raycastParameters);

    auto* cameraObject = CreateWorldGameObject("CharacterPreviewCamera");
    g_previewCameraComponent = AddSceneComponentToGameObject(
        cameraObject,
        "UnityEngine",
        "Camera");
    auto* cameraTransform = GetWorldTransform(cameraObject);
    SetTransformLocalPosition(cameraTransform, 10000.0f, 0.95f, -3.2f);
    SetTransformEuler(cameraTransform, 0.0f, 0.0f, 0.0f);

    auto* cameraClass = FindClass("UnityEngine", "Camera");
    bool orthographic = true;
    float orthographicSize = 0.92f;
    float nearClip = 0.01f;
    float farClip = 20.0f;
    float cameraDepth = -50.0f;
    float backgroundColor[] = { 0.035f, 0.048f, 0.070f, 1.0f };
    int clearFlags = 2;
    void* orthographicParameters[] = { &orthographic };
    void* sizeParameters[] = { &orthographicSize };
    void* nearParameters[] = { &nearClip };
    void* farParameters[] = { &farClip };
    void* depthParameters[] = { &cameraDepth };
    void* colorParameters[] = { backgroundColor };
    void* clearParameters[] = { &clearFlags };
    void* renderTextureParameters[] = { g_previewRenderTexture };
    Invoke(
        FindMethod(cameraClass, "set_orthographic", 1),
        g_previewCameraComponent,
        orthographicParameters);
    Invoke(
        FindMethod(cameraClass, "set_orthographicSize", 1),
        g_previewCameraComponent,
        sizeParameters);
    Invoke(
        FindMethod(cameraClass, "set_nearClipPlane", 1),
        g_previewCameraComponent,
        nearParameters);
    Invoke(
        FindMethod(cameraClass, "set_farClipPlane", 1),
        g_previewCameraComponent,
        farParameters);
    Invoke(
        FindMethod(cameraClass, "set_depth", 1),
        g_previewCameraComponent,
        depthParameters);
    Invoke(
        FindMethod(cameraClass, "set_backgroundColor", 1),
        g_previewCameraComponent,
        colorParameters);
    Invoke(
        FindMethod(cameraClass, "set_clearFlags", 1),
        g_previewCameraComponent,
        clearParameters);
    Invoke(
        FindMethod(cameraClass, "set_targetTexture", 1),
        g_previewCameraComponent,
        renderTextureParameters);

    auto* keyLightObject = CreateWorldGameObject("CharacterPreviewKeyLight");
    auto* keyLight = AddSceneComponentToGameObject(
        keyLightObject,
        "UnityEngine",
        "Light");
    auto* keyLightTransform = GetWorldTransform(keyLightObject);
    SetTransformLocalPosition(keyLightTransform, 10000.0f, 2.5f, -1.5f);
    SetTransformEuler(keyLightTransform, 50.0f, -25.0f, 0.0f);
    int lightType = 1;
    float intensity = 0.65f;
    float lightColor[] = { 1.0f, 0.95f, 0.90f, 1.0f };
    void* lightTypeParameters[] = { &lightType };
    void* intensityParameters[] = { &intensity };
    void* lightColorParameters[] = { lightColor };
    auto* lightClass = FindClass("UnityEngine", "Light");
    Invoke(
        FindMethod(lightClass, "set_type", 1),
        keyLight,
        lightTypeParameters);
    Invoke(
        FindMethod(lightClass, "set_intensity", 1),
        keyLight,
        intensityParameters);
    Invoke(
        FindMethod(lightClass, "set_color", 1),
        keyLight,
        lightColorParameters);

    auto* fillLightObject = CreateWorldGameObject("CharacterPreviewFillLight");
    auto* fillLight = AddSceneComponentToGameObject(
        fillLightObject,
        "UnityEngine",
        "Light");
    auto* fillLightTransform = GetWorldTransform(fillLightObject);
    SetTransformLocalPosition(fillLightTransform, 9998.0f, 1.3f, -1.0f);
    lightType = 2;
    intensity = 0.35f;
    float fillColor[] = { 0.55f, 0.70f, 1.0f, 1.0f };
    lightTypeParameters[0] = &lightType;
    intensityParameters[0] = &intensity;
    lightColorParameters[0] = fillColor;
    Invoke(
        FindMethod(lightClass, "set_type", 1),
        fillLight,
        lightTypeParameters);
    Invoke(
        FindMethod(lightClass, "set_intensity", 1),
        fillLight,
        intensityParameters);
    Invoke(
        FindMethod(lightClass, "set_color", 1),
        fillLight,
        lightColorParameters);

    LogInfo(LogCategory_Scene, L"Scene", L"Character preview built on an offscreen 3D stage.");
}

void EscapeFromManaka_UpdateModelDrag()
{
    if (!g_lobbyUiOpen ||
        g_activeUiTab.load() != 0 ||
        g_previewModel == nullptr)
    {
        g_efmModelDragging = false;
        return;
    }

    const HWND gameWindow = FindGameWindow();
    RECT clientRect{};
    if (gameWindow == nullptr ||
        !GetClientRect(gameWindow, &clientRect))
    {
        g_efmModelDragging = false;
        return;
    }

    const int width = clientRect.right - clientRect.left;
    const int height = clientRect.bottom - clientRect.top;
    if (width <= 0 || height <= 0)
    {
        g_efmModelDragging = false;
        return;
    }

    POINT cursor{};
    if (!GetCursorPos(&cursor) ||
        !ScreenToClient(gameWindow, &cursor))
    {
        g_efmModelDragging = false;
        return;
    }

    const float normalizedX =
        static_cast<float>(cursor.x - clientRect.left) /
        static_cast<float>(width);
    const float normalizedY =
        static_cast<float>(cursor.y - clientRect.top) /
        static_cast<float>(height);
    const bool insideModelArea =
        normalizedX >= 0.025f && normalizedX <= 0.365f &&
        normalizedY >= 0.220f && normalizedY <= 0.740f;
    const bool leftButtonDown =
        (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    if (!leftButtonDown)
    {
        if (g_efmModelDragging)
        {
            LogInfo(LogCategory_Scene, L"Scene", 
                L"Model rotation drag ended at %.1f degrees.",
                g_efmModelYaw);
        }
        g_efmModelDragging = false;
        return;
    }

    if (!g_efmModelDragging)
    {
        if (!insideModelArea)
        {
            return;
        }
        g_efmModelDragging = true;
        g_efmModelDragPoint = cursor;
        LogInfo(LogCategory_Scene, L"Scene", L"Model rotation drag started.");
        return;
    }

    const int deltaX = cursor.x - g_efmModelDragPoint.x;
    if (deltaX != 0)
    {
        g_efmModelYaw -=
            static_cast<float>(deltaX) * 360.0f /
            static_cast<float>(width);
        if (g_efmModelYaw > 180.0f)
        {
            g_efmModelYaw -= 360.0f;
        }
        else if (g_efmModelYaw < -180.0f)
        {
            g_efmModelYaw += 360.0f;
        }

        auto* modelTransform = GetWorldTransform(g_previewModel);
        SetTransformEuler(
            modelTransform,
            0.0f,
            180.0f + g_efmModelYaw,
            0.0f);
    }
    g_efmModelDragPoint = cursor;
}

Il2CppObject* FindChildGameObjectByName(
    Il2CppObject* transform,
    const wchar_t* targetName,
    int depth)
{
    if (transform == nullptr || targetName == nullptr || depth > 12)
    {
        return nullptr;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* gameObject = Invoke(
        FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
        transform,
        nullptr);
    if (gameObject != nullptr && GetObjectName(gameObject) == targetName)
    {
        return gameObject;
    }

    auto* childCountValue = Invoke(
        FindMethod(transformClass, "get_childCount", 0),
        transform,
        nullptr);
    if (childCountValue == nullptr)
    {
        return nullptr;
    }
    const int childCount = *static_cast<int*>(il2cpp_object_unbox(childCountValue));
    for (int index = 0; index < childCount; ++index)
    {
        void* parameters[] = { &index };
        auto* child = Invoke(
            FindMethod(transformClass, "GetChild", 1),
            transform,
            parameters);
        auto* found = FindChildGameObjectByName(child, targetName, depth + 1);
        if (found != nullptr)
        {
            return found;
        }
    }
    return nullptr;
}

void SetSkeletonBoneLocalEuler(
    Il2CppObject* modelTransform,
    const wchar_t* boneName,
    float x,
    float y,
    float z)
{
    auto* boneObject = FindChildGameObjectByName(modelTransform, boneName);
    if (boneObject == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Standing pose bone not found: %s", boneName);
        return;
    }
    auto* boneTransform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        boneObject,
        nullptr);
    SetTransformLocalEuler(boneTransform, x, y, z);
}

