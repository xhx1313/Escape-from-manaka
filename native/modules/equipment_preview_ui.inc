// Native.cpp module: equipment_preview_ui.inc
// F5 opens an isolated item-preview window. The gameplay avatar is never
// re-layered or re-rendered for this feature; a frozen offscreen clone supplies
// the item geometry and the main camera only sees layer 31.

#include "equipment_catalog_data.inc"

constexpr int kEquipmentPreviewLayer = 30;
constexpr int kEquipmentPreviewTextureSize = 640;
constexpr float kEquipmentPreviewStageX = 20000.0f;
constexpr int kEquipmentPreviewBackgroundCount = 5;

static const char* const
    kEquipmentPreviewBackgroundNames[kEquipmentPreviewBackgroundCount] = {
        "red", "gray", "green", "purple", "gold"
    };

static const wchar_t* const
    kEquipmentPreviewBackgroundDirectories[
        kEquipmentPreviewBackgroundCount] = {
        L"black", L"white", L"green", L"purple", L"gold"
    };

static const float
    kEquipmentPreviewBackgroundColors[
        kEquipmentPreviewBackgroundCount][4] = {
        { 0.420f, 0.035f, 0.045f, 1.0f },
        { 0.720f, 0.720f, 0.720f, 1.0f },
        { 0.055f, 0.330f, 0.075f, 1.0f },
        { 0.280f, 0.045f, 0.380f, 1.0f },
        { 0.520f, 0.310f, 0.035f, 1.0f },
    };

struct EquipmentPreviewRenderer
{
    Il2CppObject* renderer;
    std::wstring name;
};

static Il2CppObject* g_equipmentPreviewPanel = nullptr;
static Il2CppObject* g_equipmentPreviewImageRect = nullptr;
static Il2CppObject* g_equipmentPreviewRawImage = nullptr;
static Il2CppObject* g_equipmentPreviewTitle = nullptr;
static Il2CppObject* g_equipmentPreviewItemLabel = nullptr;
static SceneUiElement g_equipmentPreviewPreviousButton{};
static SceneUiElement g_equipmentPreviewNextButton{};
static SceneUiElement g_equipmentPreviewCaptureButton{};
static SceneUiElement g_equipmentPreviewForceAllButton{};
static SceneUiElement g_equipmentPreviewBatchButton{};
static SceneUiElement g_equipmentPreviewCloseButton{};
static Il2CppObject* g_equipmentPreviewStage = nullptr;
static Il2CppObject* g_equipmentPreviewModel = nullptr;
static Il2CppObject* g_equipmentPreviewStandaloneItem = nullptr;
static Il2CppObject* g_equipmentPreviewCamera = nullptr;
static Il2CppObject* g_equipmentPreviewCameraObject = nullptr;
static Il2CppObject* g_equipmentPreviewRenderTexture = nullptr;
static Il2CppObject* g_equipmentPreviewReferencer = nullptr;
static std::vector<EquipmentPreviewRenderer> g_equipmentPreviewRenderers;
static int g_equipmentPreviewIndex = -1;
static int g_equipmentPreviewCatalogIndex = -1;
static bool g_equipmentPreviewOpen = false;
static bool g_equipmentPreviewBatchActive = false;
static int g_equipmentPreviewBatchWaitFrames = 0;
static int g_equipmentPreviewBatchCaptured = 0;
static int g_equipmentPreviewBatchFailed = 0;
static int g_equipmentPreviewReframeWaitFrames = 0;
static int g_equipmentPreviewBackgroundIndex = 0;
static int g_equipmentDildoCaptureState = 0;
static int g_equipmentDildoCaptureWaitFrames = 0;

bool EquipmentPreviewUi_NameContains(
    const std::wstring& name,
    const std::initializer_list<const wchar_t*>& tokens)
{
    for (const auto* token : tokens)
    {
        if (name.find(token) != std::wstring::npos)
        {
            return true;
        }
    }
    return false;
}

bool EquipmentPreviewUi_IsBodyRenderer(const std::wstring& name)
{
    const bool bodyLike = EquipmentPreviewUi_NameContains(
        name,
        {
            L"Body", L"Face", L"Hair", L"Head", L"Eye", L"Brow", L"Lash",
            L"Cheek", L"Tongue", L"Tooth", L"Manustia", L"Selestia_body",
            L"Tear", L"Tattoo"
        });
    if (!bodyLike)
    {
        return false;
    }
    const bool costumeOverride = EquipmentPreviewUi_NameContains(
        name,
        {
            L"Dress", L"Shirt", L"Skirt", L"Pants", L"Jacket", L"Coat",
            L"Boot", L"Shoe", L"Sock", L"Stocking", L"Glove", L"Hat",
            L"Mask", L"Bra", L"Panty", L"Bikini", L"Swimsuit", L"Leotard",
            L"Collar", L"Necklace", L"Tie", L"Ribbon", L"Hoodie",
            L"Cardigan", L"Sweater", L"Cape", L"FullFace", L"Cosplay"
        });
    return !costumeOverride;
}

std::wstring EquipmentPreviewUi_GetTransformPath(Il2CppObject* gameObject)
{
    if (gameObject == nullptr)
    {
        return std::wstring();
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* transform = GetWorldTransform(gameObject);
    std::vector<std::wstring> parts;
    for (int depth = 0; transform != nullptr && depth < 12; ++depth)
    {
        auto* partObject = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(transform) + 0x10);
        // Component.gameObject is resolved through the provider API below;
        // the raw slot is only a fallback for unnamed transforms.
        auto* componentObject = GetComponentGameObject(transform);
        if (componentObject != nullptr)
        {
            partObject = componentObject;
        }
        const auto partName = GetObjectName(
            partObject != nullptr ? partObject : gameObject);
        if (!partName.empty())
        {
            parts.push_back(partName);
        }
        auto* parentValue = Invoke(
            FindMethod(transformClass, "get_parent", 0),
            transform,
            nullptr);
        transform = parentValue;
    }
    std::wstring result;
    for (auto iterator = parts.rbegin(); iterator != parts.rend(); ++iterator)
    {
        if (!result.empty())
        {
            result += L"/";
        }
        result += *iterator;
    }
    return result;
}

bool EquipmentPreviewUi_IsItemPath(const std::wstring& path)
{
    if (path.empty() ||
        path.find(L"/Armature/") != std::wstring::npos ||
        path.find(L"/Censor/") != std::wstring::npos ||
        path.find(L"/Hair/") != std::wstring::npos)
    {
        return false;
    }
    return path.find(L"/ClothesParent/") != std::wstring::npos ||
           path.find(L"/Clothes/") != std::wstring::npos ||
           path.find(L"/Costume/") != std::wstring::npos ||
           path.find(L"/CostumeParent/") != std::wstring::npos ||
           path.find(L"/Cosplay/") != std::wstring::npos ||
           path.find(L"/Accessories/") != std::wstring::npos ||
           path.find(L"/Equipment/") != std::wstring::npos ||
           path.find(L"/Shoes/") != std::wstring::npos;
}

void EquipmentPreviewUi_DestroyObject(Il2CppObject* object);

void EquipmentPreviewUi_DestroyStandaloneItem()
{
    if (g_equipmentPreviewStandaloneItem == nullptr)
    {
        return;
    }
    if (ExtendedMode_IsUnityObjectAlive(
            g_equipmentPreviewStandaloneItem))
    {
        EquipmentPreviewUi_DestroyObject(
            g_equipmentPreviewStandaloneItem);
    }
    g_equipmentPreviewStandaloneItem = nullptr;
}

void EquipmentPreviewUi_DestroyObject(Il2CppObject* object)
{
    if (object == nullptr)
    {
        return;
    }
    void* parameters[] = { object };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Object"), "Destroy", 1),
        nullptr,
        parameters);
}

void EquipmentPreviewUi_SetRendererEnabled(
    Il2CppObject* renderer,
    bool enabled)
{
    if (renderer == nullptr)
    {
        return;
    }
    void* parameters[] = { &enabled };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Renderer"), "set_enabled", 1),
        renderer,
        parameters);
}

void EquipmentPreviewUi_FreezeClone(Il2CppObject* clone)
{
    if (clone == nullptr)
    {
        return;
    }
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* behaviourClass = FindClass("UnityEngine", "MonoBehaviour");
    if (gameObjectClass == nullptr || animatorClass == nullptr ||
        behaviourClass == nullptr || il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(animatorClass);
    il2cpp_runtime_class_init(behaviourClass);
    bool includeInactive = true;
    auto* animatorType =
        il2cpp_type_get_object(il2cpp_class_get_type(animatorClass));
    void* animatorParameters[] = { animatorType, &includeInactive };
    auto* animators = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        clone,
        animatorParameters);
    if (animators != nullptr)
    {
        const auto count = il2cpp_array_length(animators);
        auto* elements = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(animators) +
            il2cpp_array_object_header_size());
        float speed = 0.0f;
        int updateMode = 0;
        int cullingMode = 0;
        void* speedParameters[] = { &speed };
        void* updateParameters[] = { &updateMode };
        void* cullingParameters[] = { &cullingMode };
        for (uint32_t index = 0; index < count; ++index)
        {
            if (elements[index] == nullptr)
            {
                continue;
            }
            Invoke(
                FindMethod(animatorClass, "set_speed", 1),
                elements[index],
                speedParameters);
            Invoke(
                FindMethod(animatorClass, "set_updateMode", 1),
                elements[index],
                updateParameters);
            Invoke(
                FindMethod(animatorClass, "set_cullingMode", 1),
                elements[index],
                cullingParameters);
        }
    }

    // Clone-local gameplay scripts must not run or write back to the character.
    auto* behaviourType =
        il2cpp_type_get_object(il2cpp_class_get_type(behaviourClass));
    void* behaviourParameters[] = { behaviourType, &includeInactive };
    auto* behaviours = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        clone,
        behaviourParameters);
    if (behaviours != nullptr)
    {
        const auto count = il2cpp_array_length(behaviours);
        auto* elements = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(behaviours) +
            il2cpp_array_object_header_size());
        for (uint32_t index = 0; index < count; ++index)
        {
            ExtendedMode_SetComponentEnabled(elements[index], false);
        }
    }
}

Il2CppObject* EquipmentPreviewUi_GetArrayElement(
    Il2CppObject* array,
    int index)
{
    if (array == nullptr || index < 0 ||
        static_cast<uint32_t>(index) >= il2cpp_array_length(array))
    {
        return nullptr;
    }
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(array) +
        il2cpp_array_object_header_size());
    return elements[index];
}

Il2CppObject* EquipmentPreviewUi_GetReferencerArray(int offset)
{
    return g_equipmentPreviewReferencer != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_equipmentPreviewReferencer) + offset)
        : nullptr;
}

Il2CppObject* EquipmentPreviewUi_GetReferencerObject(int offset)
{
    return EquipmentPreviewUi_GetReferencerArray(offset);
}

void EquipmentPreviewUi_SetActiveWithAncestors(
    Il2CppObject* gameObject,
    bool active)
{
    if (gameObject == nullptr || g_equipmentPreviewModel == nullptr)
    {
        SetWorldGameObjectActive(gameObject, active);
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* current = GetWorldTransform(gameObject);
    auto* root = GetWorldTransform(g_equipmentPreviewModel);
    while (current != nullptr && current != root)
    {
        SetWorldGameObjectActive(GetComponentGameObject(current), active);
        current = Invoke(
            FindMethod(transformClass, "get_parent", 0),
            current,
            nullptr);
    }
    if (current == root)
    {
        SetWorldGameObjectActive(g_equipmentPreviewModel, active);
    }
    else
    {
        SetWorldGameObjectActive(gameObject, active);
    }
}

void EquipmentPreviewUi_ForceActivateHierarchy(
    Il2CppObject* gameObject,
    int depth = 0)
{
    if (gameObject == nullptr || depth > 32)
    {
        return;
    }
    SetWorldGameObjectActive(gameObject, true);
    auto* renderer = GetSceneComponentByClass(
        gameObject,
        "UnityEngine",
        "Renderer");
    if (renderer != nullptr)
    {
        EquipmentPreviewUi_SetRendererEnabled(renderer, true);
    }
    auto* transform = GetWorldTransform(gameObject);
    if (transform == nullptr)
    {
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* childCountValue = Invoke(
        FindMethod(transformClass, "get_childCount", 0),
        transform,
        nullptr);
    if (childCountValue == nullptr || il2cpp_object_unbox == nullptr)
    {
        return;
    }
    const int childCount = *static_cast<int*>(
        il2cpp_object_unbox(childCountValue));
    for (int index = 0; index < childCount; ++index)
    {
        void* childParameters[] = { &index };
        auto* child = Invoke(
            FindMethod(transformClass, "GetChild", 1),
            transform,
            childParameters);
        auto* childObject = child != nullptr
            ? GetComponentGameObject(child)
            : nullptr;
        EquipmentPreviewUi_ForceActivateHierarchy(
            childObject,
            depth + 1);
    }
}

void EquipmentPreviewUi_SetArrayObjectsActive(int offset, bool active)
{
    auto* array = EquipmentPreviewUi_GetReferencerArray(offset);
    if (array == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(array);
    for (uint32_t index = 0; index < count; ++index)
    {
        SetWorldGameObjectActive(
            EquipmentPreviewUi_GetArrayElement(array, static_cast<int>(index)),
            active);
    }
}

void EquipmentPreviewUi_HideAllRenderers()
{
    g_equipmentPreviewRenderers.clear();
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (g_equipmentPreviewModel == nullptr ||
        rendererClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType =
        il2cpp_type_get_object(il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_equipmentPreviewModel,
        parameters);
    if (renderers == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        EquipmentPreviewUi_SetRendererEnabled(elements[index], false);
    }
}

void EquipmentPreviewUi_ResetCatalogVisualsForRoot(
    Il2CppObject* modelRoot,
    bool keepBodyVisible)
{
    g_equipmentPreviewRenderers.clear();
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (modelRoot == nullptr ||
        rendererClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType =
        il2cpp_type_get_object(il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        modelRoot,
        parameters);
    if (renderers == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        auto* gameObject = renderer != nullptr
            ? GetComponentGameObject(renderer)
            : nullptr;
        const auto name = GetObjectName(gameObject);
        const auto path = EquipmentPreviewUi_GetTransformPath(gameObject);
        // Keep only the visible body/face/hair whitelist. Generic helper and
        // censor meshes must stay disabled; otherwise their placeholder
        // cylinders appear through the equipped outfit.
        const bool body =
            EquipmentPreviewUi_IsBodyRenderer(name) ||
            path.find(L"Hair") != std::wstring::npos;
        EquipmentPreviewUi_SetRendererEnabled(
            renderer,
            keepBodyVisible && body);
    }

    EquipmentPreviewUi_SetArrayObjectsActive(0x288, false); // Costumes
    EquipmentPreviewUi_SetArrayObjectsActive(0x258, false); // Accessorys
    EquipmentPreviewUi_SetArrayObjectsActive(0x270, false); // Panties
    auto* cosplayControllers =
        EquipmentPreviewUi_GetReferencerArray(0x298);
    if (cosplayControllers != nullptr)
    {
        const auto controllerCount = il2cpp_array_length(cosplayControllers);
        for (uint32_t index = 0; index < controllerCount; ++index)
        {
            auto* controller = EquipmentPreviewUi_GetArrayElement(
                cosplayControllers,
                static_cast<int>(index));
            if (controller == nullptr)
            {
                continue;
            }
            auto* parts = *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(controller) + 0x20);
            const auto partCount = parts != nullptr
                ? il2cpp_array_length(parts)
                : 0;
            for (uint32_t partId = 0; partId < partCount; ++partId)
            {
                int id = static_cast<int>(partId);
                bool active = false;
                void* partParameters[] = { &id, &active };
                Invoke(
                    FindMethod(
                        FindClass(
                            "ExposureUnnoticed2.Object3D.Player.Scripts.Costume",
                            "CosplayController"),
                        "SetActiveParts",
                        2),
                    controller,
                    partParameters);
            }
            SetWorldGameObjectActive(
                GetComponentGameObject(controller),
                false);
        }
    }
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x290),
        true); // CosplayParent
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x238),
        false); // ClothesParent
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x240),
        false); // Coat
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x268),
        false); // BraParent
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x260),
        false); // PantsParent
}

void EquipmentPreviewUi_ResetCatalogVisuals()
{
    EquipmentPreviewUi_ResetCatalogVisualsForRoot(
        g_equipmentPreviewModel,
        false);
}

void EquipmentPreviewUi_CollectEnabledRenderersUnder(
    Il2CppObject* rootGameObject)
{
    g_equipmentPreviewRenderers.clear();
    if (rootGameObject == nullptr || g_equipmentPreviewModel == nullptr)
    {
        return;
    }
    const auto rootPath =
        EquipmentPreviewUi_GetTransformPath(rootGameObject);
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (rendererClass == nullptr || gameObjectClass == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType =
        il2cpp_type_get_object(il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_equipmentPreviewModel,
        parameters);
    if (renderers == nullptr)
    {
        return;
    }
    auto* getActiveInHierarchy =
        FindMethod(gameObjectClass, "get_activeInHierarchy", 0);
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        auto* gameObject = renderer != nullptr
            ? GetComponentGameObject(renderer)
            : nullptr;
        if (gameObject == nullptr)
        {
            continue;
        }
        const auto path = EquipmentPreviewUi_GetTransformPath(gameObject);
        if (path != rootPath &&
            path.rfind(rootPath + L"/", 0) != 0)
        {
            continue;
        }
        auto* activeValue = Invoke(
            getActiveInHierarchy,
            gameObject,
            nullptr);
        if (activeValue == nullptr || il2cpp_object_unbox == nullptr ||
            !*static_cast<bool*>(il2cpp_object_unbox(activeValue)))
        {
            continue;
        }
        const auto name = GetObjectName(gameObject);
        EquipmentPreviewUi_SetRendererEnabled(renderer, true);
        g_equipmentPreviewRenderers.push_back({ renderer, name });
    }
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Catalog preview enabled %d renderer(s) below %s.",
        static_cast<int>(g_equipmentPreviewRenderers.size()),
        rootPath.c_str());
}

bool EquipmentPreviewUi_EnableRenderersMatching(
    const wchar_t* token,
    const wchar_t* secondaryToken = nullptr)
{
    if (token == nullptr || g_equipmentPreviewModel == nullptr)
    {
        return false;
    }
    g_equipmentPreviewRenderers.clear();
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (rendererClass == nullptr || gameObjectClass == nullptr)
    {
        return false;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType =
        il2cpp_type_get_object(il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_equipmentPreviewModel,
        parameters);
    if (renderers == nullptr)
    {
        return false;
    }
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        auto* gameObject = renderer != nullptr
            ? GetComponentGameObject(renderer)
            : nullptr;
        if (gameObject == nullptr)
        {
            continue;
        }
        const auto name = GetObjectName(gameObject);
        const auto path = EquipmentPreviewUi_GetTransformPath(gameObject);
        if (path.find(token) == std::wstring::npos &&
            name.find(token) == std::wstring::npos)
        {
            continue;
        }
        if (secondaryToken != nullptr &&
            path.find(secondaryToken) == std::wstring::npos &&
            name.find(secondaryToken) == std::wstring::npos)
        {
            continue;
        }
        EquipmentPreviewUi_SetActiveWithAncestors(gameObject, true);
        EquipmentPreviewUi_SetRendererEnabled(renderer, true);
        g_equipmentPreviewRenderers.push_back({ renderer, name });
    }
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Catalog fallback matched %d renderer(s) for token %s.",
        static_cast<int>(g_equipmentPreviewRenderers.size()),
        token);
    return !g_equipmentPreviewRenderers.empty();
}

Il2CppObject* EquipmentPreviewUi_ActivateCosplayPart(
    int parentType,
    int partId)
{
    auto* controllers =
        EquipmentPreviewUi_GetReferencerArray(0x298);
    auto* controller = EquipmentPreviewUi_GetArrayElement(
        controllers,
        parentType);
    if (controller == nullptr)
    {
        return nullptr;
    }
    auto* root = GetComponentGameObject(controller);
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x290),
        true);
    auto* parts =
        *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(controller) + 0x20);
    auto* partRoot = EquipmentPreviewUi_GetArrayElement(parts, partId);
    EquipmentPreviewUi_SetActiveWithAncestors(
        partRoot != nullptr ? partRoot : root,
        true);
    bool active = true;
    void* parameters[] = { &partId, &active };
    Invoke(
        FindMethod(
            FindClass(
                "ExposureUnnoticed2.Object3D.Player.Scripts.Costume",
                "CosplayController"),
            "SetActiveParts",
            2),
        controller,
        parameters);
    EquipmentPreviewUi_ForceActivateHierarchy(
        partRoot != nullptr ? partRoot : root);
    return partRoot != nullptr ? partRoot : root;
}

Il2CppObject* EquipmentPreviewUi_ActivateCoreCostume(int type)
{
    auto* root = EquipmentPreviewUi_GetArrayElement(
        EquipmentPreviewUi_GetReferencerArray(0x288),
        type);
    EquipmentPreviewUi_SetActiveWithAncestors(root, true);
    EquipmentPreviewUi_ForceActivateHierarchy(root);
    return root;
}

void EquipmentPreviewUi_CollectStandaloneRenderers(
    Il2CppObject* rootGameObject)
{
    g_equipmentPreviewRenderers.clear();
    if (rootGameObject == nullptr)
    {
        return;
    }
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (rendererClass == nullptr ||
        gameObjectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType =
        il2cpp_type_get_object(il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        rootGameObject,
        parameters);
    if (renderers == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    auto* getRendererBounds = FindMethod(
        rendererClass,
        "get_bounds",
        0);
    // DildoFloor also ships a black foreign-object proxy, magic circle,
    // collider visualizers and particle renderers. Keep only the authored
    // body pieces so their much larger effect bounds cannot shrink the item.
    for (uint32_t index = 0; index < count; ++index)
    {
        if (elements[index] != nullptr)
        {
            EquipmentPreviewUi_SetRendererEnabled(
                elements[index],
                false);
        }
    }
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        auto* gameObject = renderer != nullptr
            ? GetComponentGameObject(renderer)
            : nullptr;
        if (gameObject == nullptr)
        {
            continue;
        }
        const auto name = GetObjectName(gameObject);
        const bool modelPart =
            name.find(L"otinpov") != std::wstring::npos ||
            name.find(L"tama") != std::wstring::npos ||
            name.find(L"kyuuban") != std::wstring::npos;
        if (!modelPart)
        {
            continue;
        }
        EquipmentPreviewUi_SetActiveWithAncestors(
            gameObject,
            true);
        EquipmentPreviewUi_SetRendererEnabled(renderer, true);
        g_equipmentPreviewRenderers.push_back({ renderer, name });
        float boundsSize[3] = { 0.0f, 0.0f, 0.0f };
        auto* boundsValue = getRendererBounds != nullptr
            ? Invoke(getRendererBounds, renderer, nullptr)
            : nullptr;
        if (boundsValue != nullptr)
        {
            auto* bounds = static_cast<float*>(
                il2cpp_object_unbox(boundsValue));
            boundsSize[0] = bounds[3];
            boundsSize[1] = bounds[4];
            boundsSize[2] = bounds[5];
        }
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Standalone item renderer[%u]: %s bounds=(%.3f, %.3f, %.3f).",
            index,
            name.c_str(),
            boundsSize[0],
            boundsSize[1],
            boundsSize[2]);
    }
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Standalone item preview enabled %d renderer(s).",
        static_cast<int>(g_equipmentPreviewRenderers.size()));
}

Il2CppObject* EquipmentPreviewUi_ActivateStandaloneDroppedItem(
    const EquipmentRenderEntry& entry)
{
    EquipmentPreviewUi_DestroyStandaloneItem();
    auto* assetReferencer = ExtendedMode_GetStaticObject(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "AssetReferencer",
        "Instance");
    if (g_equipmentPreviewStage == nullptr ||
        assetReferencer == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Standalone item preview skipped: stage or asset referencer is unavailable.");
        return nullptr;
    }
    auto* prefabs = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(assetReferencer) +
        0x328);
    if (prefabs != nullptr &&
        il2cpp_array_length != nullptr)
    {
        const auto prefabCount = il2cpp_array_length(prefabs);
        for (uint32_t index = 0;
             index < prefabCount;
             ++index)
        {
            auto* candidate = EquipmentPreviewUi_GetArrayElement(
                prefabs,
                static_cast<int>(index));
            LogInfo(LogCategory_Asset, L"Asset", 
                L"MultipleDroppedPrefabs[%u]: %s.",
                index,
                GetObjectName(candidate).c_str());
        }
    }
    auto* prefab = EquipmentPreviewUi_GetArrayElement(
        prefabs,
        entry.parent);
    if (prefab == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Standalone item preview prefab is unavailable: slug=%S index=%d.",
            entry.slug,
            entry.parent);
        return nullptr;
    }
    void* instantiateParameters[] = { prefab };
    auto* item = Invoke(
        FindMethod(FindClass("UnityEngine", "Object"), "Instantiate", 1),
        nullptr,
        instantiateParameters);
    if (item == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Standalone item preview instantiate failed: slug=%S.",
            entry.slug);
        return nullptr;
    }
    auto* itemTransform = GetWorldTransform(item);
    auto* stageTransform = GetWorldTransform(
        g_equipmentPreviewStage);
    if (itemTransform == nullptr || stageTransform == nullptr)
    {
        EquipmentPreviewUi_DestroyObject(item);
        return nullptr;
    }
    bool keepWorldPosition = false;
    void* parentParameters[] = {
        stageTransform,
        &keepWorldPosition
    };
    Invoke(
        FindMethod(
            FindClass("UnityEngine", "Transform"),
            "SetParent",
            2),
        itemTransform,
        parentParameters);
    SetTransformLocalPosition(itemTransform, 0.0f, 0.0f, 0.0f);
    SetTransformLocalEuler(itemTransform, 0.0f, 180.0f, 0.0f);
    ExtendedMode_SetHierarchyLayer(
        item,
        kEquipmentPreviewLayer);
    auto* dildoClass = FindClass(
        "HadakaCoat.Objects3D.Dildo",
        "DildoController");
    auto* dildoController = FindFirstComponentInChildrenByClass(
        item,
        "HadakaCoat.Objects3D.Dildo",
        "DildoController");
    if (dildoController != nullptr && dildoClass != nullptr)
    {
        auto* start = FindMethod(dildoClass, "Start", 0);
        auto* setTarget = FindMethod(dildoClass, "SetTarget", 1);
        auto* lateUpdate = FindMethod(dildoClass, "LateUpdate", 0);
        if (start != nullptr)
        {
            Invoke(start, dildoController, nullptr);
        }
        if (setTarget != nullptr)
        {
            int target = 0; // Pussy; only stabilizes the standalone mesh.
            void* targetParameters[] = { &target };
            Invoke(
                setTarget,
                dildoController,
                targetParameters);
        }
        if (lateUpdate != nullptr)
        {
            Invoke(lateUpdate, dildoController, nullptr);
        }
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Standalone dildo controller initialized before freezing.");
    }
    EquipmentPreviewUi_FreezeClone(item);
    SetWorldGameObjectActive(item, true);
    EquipmentPreviewUi_CollectStandaloneRenderers(item);
    g_equipmentPreviewStandaloneItem = item;
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Standalone item preview activated: slug=%S prefabIndex=%d renderers=%u.",
        entry.slug,
        entry.parent,
        static_cast<unsigned int>(
            g_equipmentPreviewRenderers.size()));
    return item;
}

Il2CppObject* EquipmentPreviewUi_ActivateCatalogEntry(
    const EquipmentRenderEntry& entry)
{
    EquipmentPreviewUi_DestroyStandaloneItem();
    EquipmentPreviewUi_ResetCatalogVisuals();
    Il2CppObject* root = nullptr;
    switch (entry.kind)
    {
    case 0:
        root = EquipmentPreviewUi_ActivateCoreCostume(entry.type);
        break;
    case 1:
        root = EquipmentPreviewUi_ActivateCosplayPart(
            entry.parent,
            entry.id);
        break;
    case 2:
        root = EquipmentPreviewUi_GetArrayElement(
            EquipmentPreviewUi_GetReferencerArray(0x258),
            entry.type);
        EquipmentPreviewUi_SetActiveWithAncestors(root, true);
        EquipmentPreviewUi_ForceActivateHierarchy(root);
        break;
    case 3:
        if (entry.type == 1)
        {
            SetWorldGameObjectActive(
                EquipmentPreviewUi_GetReferencerObject(0x238),
                true);
            root = EquipmentPreviewUi_GetReferencerObject(0x240);
            SetWorldGameObjectActive(root, true);
        }
        else if (entry.type >= 2)
        {
            // The master-data hoodie record is represented by the packaged
            // SchoolHoodie/Hoodie cosplay mesh in this build.
            root = EquipmentPreviewUi_ActivateCosplayPart(19, 2);
        }
        break;
    case 4:
        root = EquipmentPreviewUi_ActivateCoreCostume(3 + entry.type);
        break;
    case 5:
        root = EquipmentPreviewUi_ActivateCoreCostume(
            std::strstr(entry.slug, "cultist") != nullptr ? 2 : 1);
        break;
    case 6:
        root = EquipmentPreviewUi_ActivateStandaloneDroppedItem(entry);
        break;
    default:
        break;
    }
    if (root != nullptr && entry.kind != 6)
    {
        EquipmentPreviewUi_CollectEnabledRenderersUnder(root);
    }
    if (g_equipmentPreviewRenderers.empty())
    {
        if (std::strstr(entry.slug, "costume-006") != nullptr)
        {
            EquipmentPreviewUi_EnableRenderersMatching(
                L"Selestia_costume_stocking (8)");
        }
        else if (std::strstr(
                     entry.slug,
                     "rcosplaypartsoriginalnippless") != nullptr)
        {
            EquipmentPreviewUi_EnableRenderersMatching(
                L"Nipplessfbx");
        }
        else if (std::strstr(
                     entry.slug,
                     "rcosplaypartsoriginalbansoukoutit") != nullptr)
        {
            EquipmentPreviewUi_EnableRenderersMatching(
                L"BansoukouBreast");
        }
        else if (std::strstr(
                     entry.slug,
                     "rcosplaypartsoriginalofutanip") != nullptr)
        {
            EquipmentPreviewUi_EnableRenderersMatching(
                L"FudaBreastfbx");
        }
        else if (std::strstr(
                     entry.slug,
                     "rcosplaypartsoriginalnippiasu") != nullptr)
        {
            EquipmentPreviewUi_EnableRenderersMatching(
                L"SuzuPiasu");
        }
        else if (std::strstr(
                     entry.slug,
                     "rcosplaypartsoriginalcrossnippiasu") != nullptr)
        {
            EquipmentPreviewUi_EnableRenderersMatching(
                L"CrossNip");
        }
        else if (std::strstr(
                     entry.slug,
                     "rcosplaypartsoriginalgentlehandbreast") != nullptr)
        {
            EquipmentPreviewUi_EnableRenderersMatching(
                L"GentleHandBreast");
        }
        else if (std::strstr(
                     entry.slug,
                     "rcosplaypartsoriginarubbersuit") != nullptr)
        {
            // The rubber suit is a body-material state, not a separate mesh in
            // this build. Render the body mesh as a dark standalone silhouette.
            if (EquipmentPreviewUi_EnableRenderersMatching(L"Selestia_body"))
            {
                for (auto& item : g_equipmentPreviewRenderers)
                {
                    auto* material = Invoke(
                        FindMethod(
                            FindClass("UnityEngine", "Renderer"),
                            "get_material",
                            0),
                        item.renderer,
                        nullptr);
                    if (material != nullptr)
                    {
                        float color[] = { 0.055f, 0.060f, 0.070f, 1.0f };
                        void* colorParameters[] = { color };
                        Invoke(
                            FindMethod(
                                FindClass("UnityEngine", "Material"),
                                "set_color",
                                1),
                            material,
                            colorParameters);
                    }
                }
            }
        }
    }
    return root;
}

Il2CppObject* EquipmentPreviewUi_ActivateCatalogEntryForOutfit(
    const EquipmentRenderEntry& entry)
{
    Il2CppObject* root = nullptr;
    switch (entry.kind)
    {
    case 0:
        root = EquipmentPreviewUi_ActivateCoreCostume(entry.type);
        break;
    case 1:
        root = EquipmentPreviewUi_ActivateCosplayPart(entry.parent, entry.id);
        break;
    case 2:
        root = EquipmentPreviewUi_GetArrayElement(
            EquipmentPreviewUi_GetReferencerArray(0x258),
            entry.type);
        EquipmentPreviewUi_SetActiveWithAncestors(root, true);
        EquipmentPreviewUi_ForceActivateHierarchy(root);
        break;
    case 3:
        if (entry.type == 1)
        {
            SetWorldGameObjectActive(
                EquipmentPreviewUi_GetReferencerObject(0x238),
                true);
            root = EquipmentPreviewUi_GetReferencerObject(0x240);
            EquipmentPreviewUi_SetActiveWithAncestors(root, true);
            EquipmentPreviewUi_ForceActivateHierarchy(root);
        }
        else if (entry.type >= 2)
        {
            root = EquipmentPreviewUi_ActivateCosplayPart(19, 2);
        }
        break;
    case 4:
        root = EquipmentPreviewUi_ActivateCoreCostume(3 + entry.type);
        break;
    case 5:
        root = EquipmentPreviewUi_ActivateCoreCostume(
            std::strstr(entry.slug, "cultist") != nullptr ? 2 : 1);
        break;
    default:
        break;
    }
    return root;
}

void EquipmentPreviewUi_ApplyAdultGoodsAttachment(
    int adultGoodsType,
    bool active)
{
    if (g_extendedAvatarRoot == nullptr ||
        g_extendedAvatarReferencer == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", L"Adult goods attachment skipped: gameplay avatar is unavailable.");
        return;
    }
    auto* previousModel = g_equipmentPreviewModel;
    auto* previousReferencer = g_equipmentPreviewReferencer;
    auto previousRenderers = g_equipmentPreviewRenderers;
    g_equipmentPreviewModel = g_extendedAvatarRoot;
    g_equipmentPreviewReferencer = g_extendedAvatarReferencer;

    Il2CppObject* root = nullptr;
    if (active)
    {
        for (int index = 0;
             index < kEquipmentRenderEntryCount;
             ++index)
        {
            const auto& entry = kEquipmentRenderEntries[index];
            if (entry.kind == 2 &&
                entry.type == adultGoodsType)
            {
                root = EquipmentPreviewUi_ActivateCatalogEntryForOutfit(
                    entry);
                break;
            }
        }
    }
    else
    {
        root = EquipmentPreviewUi_GetArrayElement(
            EquipmentPreviewUi_GetReferencerArray(0x258),
            adultGoodsType);
        if (root != nullptr)
        {
            SetWorldGameObjectActive(root, false);
        }
    }

    g_equipmentPreviewModel = previousModel;
    g_equipmentPreviewReferencer = previousReferencer;
    g_equipmentPreviewRenderers = std::move(previousRenderers);
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Adult goods attachment type=%d rendered with outfit path (active=%d, root=%p).",
        adultGoodsType,
        active ? 1 : 0,
        root);
}

void EquipmentPreviewUi_ApplyOutfitToGameplayAvatar(
    const char* const* itemIds,
    int itemCount)
{
    if (g_extendedAvatarRoot == nullptr ||
        g_extendedAvatarReferencer == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", L"Equipment outfit refresh skipped: gameplay avatar is unavailable.");
        return;
    }

    auto* previousModel = g_equipmentPreviewModel;
    auto* previousReferencer = g_equipmentPreviewReferencer;
    auto previousRenderers = g_equipmentPreviewRenderers;
    g_equipmentPreviewModel = g_extendedAvatarRoot;
    g_equipmentPreviewReferencer = g_extendedAvatarReferencer;
    EquipmentPreviewUi_ResetCatalogVisualsForRoot(
        g_extendedAvatarRoot,
        true);

    int applied = 0;
    for (int itemIndex = 0;
         itemIds != nullptr && itemIndex < itemCount;
         ++itemIndex)
    {
        const char* itemId = itemIds[itemIndex];
        if (itemId == nullptr || itemId[0] == '\0')
        {
            continue;
        }
        const EquipmentRenderEntry* matchingEntry = nullptr;
        for (int renderIndex = 0;
             renderIndex < kEquipmentRenderEntryCount;
             ++renderIndex)
        {
            const auto& candidate = kEquipmentRenderEntries[renderIndex];
            if (std::strstr(itemId, candidate.slug) != nullptr)
            {
                matchingEntry = &candidate;
                break;
            }
        }
        if (matchingEntry == nullptr)
        {
            LogInfo(LogCategory_Asset, L"Asset", L"Equipment outfit has no render mapping for %S.", itemId);
            continue;
        }
        if (EquipmentPreviewUi_ActivateCatalogEntryForOutfit(*matchingEntry) !=
            nullptr)
        {
            ++applied;
        }
    }

    g_equipmentPreviewModel = previousModel;
    g_equipmentPreviewReferencer = previousReferencer;
    g_equipmentPreviewRenderers = std::move(previousRenderers);
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Equipment outfit applied %d/%d visible item(s) to the gameplay avatar.",
        applied,
        itemCount);
}

void EquipmentPreviewUi_FrameSelectedRenderers()
{
    if (g_equipmentPreviewCamera == nullptr)
    {
        return;
    }
    auto* cameraTransform = GetWorldTransform(
        GetComponentGameObject(g_equipmentPreviewCamera));
    const bool earAccessory =
        g_equipmentPreviewCatalogIndex >= 0 &&
        g_equipmentPreviewCatalogIndex < kEquipmentRenderEntryCount &&
        (std::strstr(
             kEquipmentRenderEntries[g_equipmentPreviewCatalogIndex].slug,
             "rcosplaypartsearcat") != nullptr ||
         std::strstr(
             kEquipmentRenderEntries[g_equipmentPreviewCatalogIndex].slug,
             "rcosplaypartsearfox") != nullptr);
    if (earAccessory)
    {
        ExtendedMode_SetTransformPosition(
            cameraTransform,
            kEquipmentPreviewStageX,
            1.45f,
            -2.8f);
        SetTransformEuler(cameraTransform, 0.0f, 0.0f, 0.0f);
        float earSize = 0.28f;
        void* earSizeParameters[] = { &earSize };
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "Camera"),
                "set_orthographicSize",
                1),
            g_equipmentPreviewCamera,
            earSizeParameters);
        return;
    }

    float minX = 1.0e9f;
    float maxX = -1.0e9f;
    float minY = 1.0e9f;
    float maxY = -1.0e9f;
    float minZ = 1.0e9f;
    float maxZ = -1.0e9f;
    for (const auto& item : g_equipmentPreviewRenderers)
    {
        auto* boundsValue = Invoke(
            FindMethod(FindClass("UnityEngine", "Renderer"), "get_bounds", 0),
            item.renderer,
            nullptr);
        if (boundsValue == nullptr || il2cpp_object_unbox == nullptr)
        {
            continue;
        }
        const auto* bounds =
            static_cast<float*>(il2cpp_object_unbox(boundsValue));
        minX = std::fmin(minX, bounds[0] - std::fabs(bounds[3]));
        maxX = std::fmax(maxX, bounds[0] + std::fabs(bounds[3]));
        minY = std::fmin(minY, bounds[1] - std::fabs(bounds[4]));
        maxY = std::fmax(maxY, bounds[1] + std::fabs(bounds[4]));
        minZ = std::fmin(minZ, bounds[2] - std::fabs(bounds[5]));
        maxZ = std::fmax(maxZ, bounds[2] + std::fabs(bounds[5]));
    }
    float size = 0.35f;
    float centerX = kEquipmentPreviewStageX;
    float centerY = 1.0f;
    float centerZ = 0.0f;
    if (maxY >= minY)
    {
        centerX = (minX + maxX) * 0.5f;
        centerY = (minY + maxY) * 0.5f;
        centerZ = (minZ + maxZ) * 0.5f;
        size = std::fmax(
            0.18f,
            std::fmin(
                2.2f,
                std::fmax(
                    (maxY - minY) * 0.70f,
                    (maxX - minX) * 0.70f)));
    }
    ExtendedMode_SetTransformPosition(
        cameraTransform,
        centerX,
        centerY,
        centerZ - 2.8f);
    SetTransformEuler(cameraTransform, 0.0f, 0.0f, 0.0f);
    void* sizeParameters[] = { &size };
    Invoke(
        FindMethod(
            FindClass("UnityEngine", "Camera"),
            "set_orthographicSize",
            1),
        g_equipmentPreviewCamera,
        sizeParameters);
}

void EquipmentPreviewUi_Select(int index)
{
    if (kEquipmentRenderEntryCount <= 0)
    {
        return;
    }
    index %= kEquipmentRenderEntryCount;
    if (index < 0)
    {
        index += kEquipmentRenderEntryCount;
    }
    g_equipmentPreviewCatalogIndex = index;
    const auto& entry = kEquipmentRenderEntries[index];
    EquipmentPreviewUi_ActivateCatalogEntry(entry);
    EquipmentPreviewUi_FrameSelectedRenderers();
    g_equipmentPreviewReframeWaitFrames = 3;

    char label[256]{};
    std::snprintf(
        label,
        sizeof(label),
        "%s  ·  %d / %d   %s",
        kEquipmentPreviewBackgroundNames[g_equipmentPreviewBackgroundIndex],
        index + 1,
        kEquipmentRenderEntryCount,
        entry.label);
    SetSceneText(
        g_equipmentPreviewItemLabel,
        label,
        16.0f,
        514,
        0.93f,
        0.95f,
        0.98f,
        1.0f);
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Equipment catalog selected %d/%d: %s (%S).",
        index + 1,
        kEquipmentRenderEntryCount,
        Utf8ToWide(entry.label).c_str(),
        entry.slug);
}

Il2CppObject* EquipmentPreviewUi_CreateRenderTexture()
{
    auto* renderTextureClass = FindClass("UnityEngine", "RenderTexture");
    if (renderTextureClass == nullptr || il2cpp_object_new == nullptr)
    {
        return nullptr;
    }
    il2cpp_runtime_class_init(renderTextureClass);
    auto* texture = il2cpp_object_new(renderTextureClass);
    int width = kEquipmentPreviewTextureSize;
    int height = kEquipmentPreviewTextureSize;
    int depth = 24;
    void* parameters[] = { &width, &height, &depth };
    Invoke(
        FindMethod(renderTextureClass, ".ctor", 3),
        texture,
        parameters);
    Invoke(FindMethod(renderTextureClass, "Create", 0), texture, nullptr);
    return texture;
}

void EquipmentPreviewUi_CreateStage()
{
    if (g_equipmentPreviewStage != nullptr ||
        g_extendedAvatarRoot == nullptr)
    {
        return;
    }

    g_equipmentPreviewRenderTexture =
        EquipmentPreviewUi_CreateRenderTexture();
    if (g_equipmentPreviewRenderTexture == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", L"Equipment preview RenderTexture creation failed.");
        return;
    }

    g_equipmentPreviewStage =
        CreateWorldGameObject("EquipmentItemPreviewStage");
    auto* stageTransform = GetWorldTransform(g_equipmentPreviewStage);
    ExtendedMode_SetTransformPosition(
        stageTransform,
        kEquipmentPreviewStageX,
        0.0f,
        0.0f);

    void* instantiateParameters[] = { g_extendedAvatarRoot };
    g_equipmentPreviewModel = Invoke(
        FindMethod(FindClass("UnityEngine", "Object"), "Instantiate", 1),
        nullptr,
        instantiateParameters);
    if (g_equipmentPreviewModel == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", L"Equipment preview clone creation failed.");
        EquipmentPreviewUi_DestroyObject(g_equipmentPreviewStage);
        g_equipmentPreviewStage = nullptr;
        return;
    }

    auto* modelTransform = GetWorldTransform(g_equipmentPreviewModel);
    bool keepWorldPosition = false;
    void* parentParameters[] = { stageTransform, &keepWorldPosition };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Transform"), "SetParent", 2),
        modelTransform,
        parentParameters);
    SetTransformLocalPosition(modelTransform, 0.0f, 0.0f, 0.0f);
    SetTransformLocalEuler(modelTransform, 0.0f, 180.0f, 0.0f);
    ExtendedMode_SetHierarchyLayer(
        g_equipmentPreviewModel,
        kEquipmentPreviewLayer);
    EquipmentPreviewUi_FreezeClone(g_equipmentPreviewModel);
    g_equipmentPreviewReferencer = FindFirstComponentInChildrenByClass(
        g_equipmentPreviewModel,
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAvatarObjectReferencer");
    if (g_equipmentPreviewReferencer == nullptr)
    {
        LogInfo(LogCategory_Asset, L"Asset", L"Equipment preview avatar object referencer is unavailable.");
    }

    auto* cameraObject =
        CreateWorldGameObject("EquipmentItemPreviewCamera");
    g_equipmentPreviewCameraObject = cameraObject;
    g_equipmentPreviewCamera = AddSceneComponentToGameObject(
        cameraObject,
        "UnityEngine",
        "Camera");
    ExtendedMode_SetGameObjectLayer(cameraObject, kEquipmentPreviewLayer);
    auto* cameraTransform = GetWorldTransform(cameraObject);
    ExtendedMode_SetTransformPosition(
        cameraTransform,
        kEquipmentPreviewStageX,
        1.0f,
        -2.8f);
    auto* cameraClass = FindClass("UnityEngine", "Camera");
    int cullingMask = 1 << kEquipmentPreviewLayer;
    int clearFlags = 2;
    float nearClip = 0.01f;
    float farClip = 12.0f;
    bool orthographic = true;
    float orthographicSize = 0.8f;
    float depth = -40.0f;
    float backgroundColor[] = { 0.105f, 0.120f, 0.145f, 1.0f };
    void* maskParameters[] = { &cullingMask };
    void* clearParameters[] = { &clearFlags };
    void* nearParameters[] = { &nearClip };
    void* farParameters[] = { &farClip };
    void* orthographicParameters[] = { &orthographic };
    void* sizeParameters[] = { &orthographicSize };
    void* depthParameters[] = { &depth };
    void* colorParameters[] = { backgroundColor };
    void* textureParameters[] = { g_equipmentPreviewRenderTexture };
    Invoke(
        FindMethod(cameraClass, "set_cullingMask", 1),
        g_equipmentPreviewCamera,
        maskParameters);
    Invoke(
        FindMethod(cameraClass, "set_clearFlags", 1),
        g_equipmentPreviewCamera,
        clearParameters);
    Invoke(
        FindMethod(cameraClass, "set_nearClipPlane", 1),
        g_equipmentPreviewCamera,
        nearParameters);
    Invoke(
        FindMethod(cameraClass, "set_farClipPlane", 1),
        g_equipmentPreviewCamera,
        farParameters);
    Invoke(
        FindMethod(cameraClass, "set_orthographic", 1),
        g_equipmentPreviewCamera,
        orthographicParameters);
    Invoke(
        FindMethod(cameraClass, "set_orthographicSize", 1),
        g_equipmentPreviewCamera,
        sizeParameters);
    Invoke(
        FindMethod(cameraClass, "set_depth", 1),
        g_equipmentPreviewCamera,
        depthParameters);
    Invoke(
        FindMethod(cameraClass, "set_backgroundColor", 1),
        g_equipmentPreviewCamera,
        colorParameters);
    Invoke(
        FindMethod(cameraClass, "set_targetTexture", 1),
        g_equipmentPreviewCamera,
        textureParameters);

    for (int lightIndex = 0; lightIndex < 2; ++lightIndex)
    {
        auto* lightObject = CreateWorldGameObject(
            lightIndex == 0
                ? "EquipmentPreviewKeyLight"
                : "EquipmentPreviewFillLight");
        auto* lightTransform = GetWorldTransform(lightObject);
        bool keepLightWorldPosition = false;
        void* lightParentParameters[] = {
            stageTransform,
            &keepLightWorldPosition
        };
        Invoke(
            FindMethod(FindClass("UnityEngine", "Transform"), "SetParent", 2),
            lightTransform,
            lightParentParameters);
        SetTransformLocalPosition(
            lightTransform,
            lightIndex == 0 ? -1.2f : 1.2f,
            lightIndex == 0 ? 1.8f : 0.7f,
            -1.6f);
        ExtendedMode_SetGameObjectLayer(
            lightObject,
            kEquipmentPreviewLayer);
        auto* light = AddSceneComponentToGameObject(
            lightObject,
            "UnityEngine",
            "Light");
        auto* lightClass = FindClass("UnityEngine", "Light");
        int lightType = lightIndex == 0 ? 1 : 1;
        float intensity = lightIndex == 0 ? 1.55f : 0.85f;
        float range = 5.0f;
        float color[] = {
            lightIndex == 0 ? 1.0f : 0.58f,
            lightIndex == 0 ? 0.94f : 0.72f,
            lightIndex == 0 ? 0.87f : 1.0f,
            1.0f
        };
        int lightMask = 1 << kEquipmentPreviewLayer;
        void* typeParameters[] = { &lightType };
        void* intensityParameters[] = { &intensity };
        void* rangeParameters[] = { &range };
        void* colorParameters2[] = { color };
        void* lightMaskParameters[] = { &lightMask };
        Invoke(
            FindMethod(lightClass, "set_type", 1),
            light,
            typeParameters);
        Invoke(
            FindMethod(lightClass, "set_intensity", 1),
            light,
            intensityParameters);
        Invoke(
            FindMethod(lightClass, "set_range", 1),
            light,
            rangeParameters);
        Invoke(
            FindMethod(lightClass, "set_color", 1),
            light,
            colorParameters2);
        Invoke(
            FindMethod(lightClass, "set_cullingMask", 1),
            light,
            lightMaskParameters);
    }

    EquipmentPreviewUi_HideAllRenderers();
}

void EquipmentPreviewUi_UpdateLayout()
{
    if (!g_equipmentPreviewOpen ||
        g_equipmentPreviewImageRect == nullptr)
    {
        return;
    }
    float width = 0.0f;
    float height = 0.0f;
    if (!InventoryUi_ReadClientSize(width, height))
    {
        return;
    }
    constexpr float panelMinX = 0.250f;
    constexpr float panelMinY = 0.180f;
    constexpr float panelMaxX = 0.750f;
    constexpr float panelMaxY = 0.840f;
    const float panelPixelsX = (panelMaxX - panelMinX) * width;
    const float panelPixelsY = (panelMaxY - panelMinY) * height;
    const float availablePixelsX = panelPixelsX * 0.84f;
    const float availablePixelsY = panelPixelsY * 0.56f;
    const float squarePixels =
        std::fmin(availablePixelsX, availablePixelsY);
    const float centerX = (panelMinX + panelMaxX) * 0.5f;
    const float centerY = panelMinY + panelPixelsY * 0.56f / height;
    const float halfX = squarePixels * 0.5f / width;
    const float halfY = squarePixels * 0.5f / height;
    SetSceneRect(
        g_equipmentPreviewImageRect,
        centerX - halfX,
        centerY - halfY,
        centerX + halfX,
        centerY + halfY);
}

void EquipmentPreviewUi_EnsureUi()
{
    if (g_equipmentPreviewPanel != nullptr ||
        g_extendedCanvas == nullptr)
    {
        return;
    }
    auto* rootTransform = GetWorldTransform(g_extendedCanvas);
    auto panel = CreateScenePanel(
        "EquipmentItemPreviewWindow",
        rootTransform,
        0.250f,
        0.180f,
        0.750f,
        0.840f,
        0.025f,
        0.030f,
        0.038f,
        0.995f);
    g_equipmentPreviewPanel = panel.gameObject;
    g_equipmentPreviewTitle = CreateSceneTextElement(
        "EquipmentPreviewTitle",
        panel.rect,
        "装备模型预览  /  ISOLATED ITEM",
        0.045f,
        0.915f,
        0.955f,
        0.982f,
        18.0f,
        257,
        0.96f,
        0.97f,
        0.99f,
        1.0f).text;

    auto imageObject = CreateSceneGameObject(
        "EquipmentPreviewImage",
        panel.rect);
    g_equipmentPreviewImageRect = GetSceneComponentByClass(
        imageObject,
        "UnityEngine",
        "RectTransform");
    auto* rawImage = AddSceneComponentToGameObject(
        imageObject,
        "UnityEngine.UI",
        "RawImage");
    g_equipmentPreviewRawImage = rawImage;
    SetSceneRect(
        g_equipmentPreviewImageRect,
        0.08f,
        0.22f,
        0.92f,
        0.88f);
    if (rawImage != nullptr)
    {
        SetSceneGraphicColor(rawImage, 1.0f, 1.0f, 1.0f, 1.0f);
        bool raycastTarget = false;
        void* raycastParameters[] = { &raycastTarget };
        Invoke(
            FindMethod(
                FindClass("UnityEngine.UI", "Graphic"),
                "set_raycastTarget",
                1),
            rawImage,
            raycastParameters);
    }

    g_equipmentPreviewItemLabel = CreateSceneTextElement(
        "EquipmentPreviewItemLabel",
        panel.rect,
        "WAITING FOR ITEM",
        0.045f,
        0.165f,
        0.955f,
        0.215f,
        15.0f,
        514,
        0.86f,
        0.90f,
        0.95f,
        1.0f).text;
    char previewHint[160]{};
    std::snprintf(
        previewHint,
        sizeof(previewHint),
        "%d 件 × 5 背景  ·  F6 依次导出 黑 / 白 / 绿 / 紫 / 金",
        kEquipmentRenderEntryCount);
    CreateSceneTextElement(
        "EquipmentPreviewHint",
        panel.rect,
        previewHint,
        0.045f,
        0.125f,
        0.955f,
        0.165f,
        11.0f,
        514,
        0.61f,
        0.68f,
        0.76f,
        1.0f);

    g_equipmentPreviewPreviousButton = CreateSceneButton(
        "EquipmentPreviewPrevious",
        panel.rect,
        "◀",
        0.030f,
        0.035f,
        0.145f,
        0.105f,
        0.10f,
        0.13f,
        0.18f,
        1.0f,
        13.0f,
        514,
        true);
    g_equipmentPreviewNextButton = CreateSceneButton(
        "EquipmentPreviewNext",
        panel.rect,
        "▶",
        0.155f,
        0.035f,
        0.270f,
        0.105f,
        0.10f,
        0.13f,
        0.18f,
        1.0f,
        13.0f,
        514,
        true);
    g_equipmentPreviewCaptureButton = CreateSceneButton(
        "EquipmentPreviewCapture",
        panel.rect,
        "保存",
        0.280f,
        0.035f,
        0.415f,
        0.105f,
        0.21f,
        0.31f,
        0.19f,
        1.0f,
        12.0f,
        514,
        true);
    g_equipmentPreviewForceAllButton = CreateSceneButton(
        "EquipmentPreviewForceAll",
        panel.rect,
        "全开调试",
        0.425f,
        0.035f,
        0.580f,
        0.105f,
        0.24f,
        0.18f,
        0.10f,
        1.0f,
        12.0f,
        514,
        true);
    g_equipmentPreviewBatchButton = CreateSceneButton(
        "EquipmentPreviewBatch",
        panel.rect,
        "全部截图",
        0.590f,
        0.035f,
        0.755f,
        0.105f,
        0.12f,
        0.38f,
        0.22f,
        1.0f,
        12.0f,
        514,
        true);
    g_equipmentPreviewCloseButton = CreateSceneButton(
        "EquipmentPreviewClose",
        panel.rect,
        "关闭 F5",
        0.765f,
        0.035f,
        0.965f,
        0.105f,
        0.13f,
        0.15f,
        0.18f,
        1.0f,
        12.0f,
        514,
        true);
    SetWorldGameObjectActive(g_equipmentPreviewPanel, false);
}

void EquipmentPreviewUi_BindTexture()
{
    if (g_equipmentPreviewRawImage == nullptr ||
        g_equipmentPreviewRenderTexture == nullptr)
    {
        return;
    }
    void* textureParameters[] = { g_equipmentPreviewRenderTexture };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "RawImage"), "set_texture", 1),
        g_equipmentPreviewRawImage,
        textureParameters);
}

void EquipmentPreviewUi_SetBackground(int index)
{
    if (index < 0 || index >= kEquipmentPreviewBackgroundCount ||
        g_equipmentPreviewCamera == nullptr)
    {
        return;
    }
    g_equipmentPreviewBackgroundIndex = index;
    float color[] = {
        kEquipmentPreviewBackgroundColors[index][0],
        kEquipmentPreviewBackgroundColors[index][1],
        kEquipmentPreviewBackgroundColors[index][2],
        1.0f
    };
    void* parameters[] = { color };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Camera"), "set_backgroundColor", 1),
        g_equipmentPreviewCamera,
        parameters);
}

void EquipmentPreviewUi_MakeBackgroundTransparent(
    Il2CppObject* texture,
    int width,
    int height)
{
    if (texture == nullptr || width <= 0 || height <= 0)
    {
        return;
    }
    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* raw = Invoke(
        FindMethod(textureClass, "GetRawTextureData", 0),
        texture,
        nullptr);
    if (raw == nullptr)
    {
        return;
    }
    const int pixelCount = width * height;
    const DWORD byteCount = il2cpp_array_length(raw);
    if (byteCount < static_cast<DWORD>(pixelCount * 4))
    {
        return;
    }
    auto* bytes = reinterpret_cast<unsigned char*>(
        reinterpret_cast<char*>(raw) +
        il2cpp_array_object_header_size());
    const unsigned char backgroundRed = bytes[0];
    const unsigned char backgroundGreen = bytes[1];
    const unsigned char backgroundBlue = bytes[2];
    std::vector<unsigned char> visited(
        static_cast<size_t>(pixelCount),
        0);
    std::vector<int> pending;
    pending.reserve(65536);
    auto tryPush = [&](int x, int y)
    {
        if (x < 0 || x >= width || y < 0 || y >= height)
        {
            return;
        }
        const int pixel = y * width + x;
        if (visited[pixel] != 0)
        {
            return;
        }
        const int offset = pixel * 4;
        const int distance = std::max(
            std::abs(static_cast<int>(bytes[offset]) - backgroundRed),
            std::max(
                std::abs(
                    static_cast<int>(bytes[offset + 1]) -
                    backgroundGreen),
                std::abs(
                    static_cast<int>(bytes[offset + 2]) -
                    backgroundBlue)));
        if (distance > 3)
        {
            return;
        }
        visited[pixel] = 1;
        pending.push_back(pixel);
    };
    for (int x = 0; x < width; ++x)
    {
        tryPush(x, 0);
        tryPush(x, height - 1);
    }
    for (int y = 0; y < height; ++y)
    {
        tryPush(0, y);
        tryPush(width - 1, y);
    }
    while (!pending.empty())
    {
        const int pixel = pending.back();
        pending.pop_back();
        bytes[pixel * 4 + 3] = 0;
        const int x = pixel % width;
        const int y = pixel / width;
        tryPush(x - 1, y);
        tryPush(x + 1, y);
        tryPush(x, y - 1);
        tryPush(x, y + 1);
    }
    auto isBackground = [&](int x, int y)
    {
        return x < 0 || x >= width || y < 0 || y >= height ||
            visited[y * width + x] != 0;
    };
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const int pixel = y * width + x;
            if (visited[pixel] != 0 ||
                (!isBackground(x - 1, y) &&
                 !isBackground(x + 1, y) &&
                 !isBackground(x, y - 1) &&
                 !isBackground(x, y + 1)))
            {
                continue;
            }
            const int offset = pixel * 4;
            const int red = bytes[offset];
            const int green = bytes[offset + 1];
            const int blue = bytes[offset + 2];
            const int minRedBlue = std::min(red, blue);
            if (minRedBlue <= 24)
            {
                continue;
            }
            const float keyMix = std::fmax(
                0.0f,
                static_cast<float>(minRedBlue - green) /
                    static_cast<float>(minRedBlue));
            if (keyMix <= 0.04f)
            {
                continue;
            }
            const float alpha = std::fmax(0.0f, 1.0f - keyMix);
            if (alpha <= 0.02f)
            {
                bytes[offset + 3] = 0;
                continue;
            }
            bytes[offset] = static_cast<unsigned char>(std::clamp(
                static_cast<int>(
                    (red - keyMix * 255.0f) / alpha),
                0,
                255));
            bytes[offset + 1] = static_cast<unsigned char>(std::clamp(
                static_cast<int>(
                    static_cast<float>(green) / alpha),
                0,
                255));
            bytes[offset + 2] = static_cast<unsigned char>(std::clamp(
                static_cast<int>(
                    (blue - keyMix * 255.0f) / alpha),
                0,
                255));
            bytes[offset + 3] = static_cast<unsigned char>(
                std::clamp(
                    static_cast<int>(alpha * 255.0f),
                    0,
                    255));
        }
    }
    void* loadParameters[] = { raw };
    Invoke(
        FindMethod(textureClass, "LoadRawTextureData", 1),
        texture,
        loadParameters);
    Invoke(FindMethod(textureClass, "Apply", 0), texture, nullptr);
}

void EquipmentPreviewUi_ForceTextureOpaque(
    Il2CppObject* texture,
    int width,
    int height)
{
    if (texture == nullptr || width <= 0 || height <= 0)
    {
        return;
    }
    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* raw = Invoke(
        FindMethod(textureClass, "GetRawTextureData", 0),
        texture,
        nullptr);
    if (raw == nullptr)
    {
        return;
    }
    const DWORD expected = static_cast<DWORD>(width * height * 4);
    if (il2cpp_array_length(raw) < expected)
    {
        return;
    }
    auto* bytes = reinterpret_cast<unsigned char*>(
        reinterpret_cast<char*>(raw) +
        il2cpp_array_object_header_size());
    for (DWORD index = 3; index < expected; index += 4)
    {
        bytes[index] = 255;
    }
    void* loadParameters[] = { raw };
    Invoke(
        FindMethod(textureClass, "LoadRawTextureData", 1),
        texture,
        loadParameters);
    Invoke(FindMethod(textureClass, "Apply", 0), texture, nullptr);
}

bool EquipmentPreviewUi_SaveScreenshot()
{
    if (g_equipmentPreviewRenderTexture == nullptr ||
        g_equipmentPreviewCatalogIndex < 0 ||
        g_equipmentPreviewCatalogIndex >= kEquipmentRenderEntryCount)
    {
        return false;
    }
    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* renderTextureClass = FindClass("UnityEngine", "RenderTexture");
    auto* imageConversionClass =
        FindClass("UnityEngine", "ImageConversion");
    if (textureClass == nullptr || renderTextureClass == nullptr ||
        imageConversionClass == nullptr || il2cpp_object_new == nullptr)
    {
        return false;
    }
    il2cpp_runtime_class_init(textureClass);
    const auto previousValue = Invoke(
        FindMethod(renderTextureClass, "get_active", 0),
        nullptr,
        nullptr);
    void* activeParameters[] = { g_equipmentPreviewRenderTexture };
    Invoke(
        FindMethod(renderTextureClass, "set_active", 1),
        nullptr,
        activeParameters);

    auto* texture = il2cpp_object_new(textureClass);
    int width = kEquipmentPreviewTextureSize;
    int height = kEquipmentPreviewTextureSize;
    // The two-argument overload is unambiguous and creates a readable RGBA32
    // texture. Texture2D has several four-argument constructors, so selecting
    // only by parameter count can bind the wrong graphics-format overload.
    void* constructorParameters[] = { &width, &height };
    Invoke(
        FindMethod(textureClass, ".ctor", 2),
        texture,
        constructorParameters);
    float pixelRect[] = {
        0.0f,
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };
    int destX = 0;
    int destY = 0;
    bool recalculateMipMaps = false;
    void* readParameters[] = {
        pixelRect,
        &destX,
        &destY,
        &recalculateMipMaps
    };
    Invoke(
        FindMethod(textureClass, "ReadPixels", 4),
        texture,
        readParameters);
    Invoke(FindMethod(textureClass, "Apply", 0), texture, nullptr);
    EquipmentPreviewUi_ForceTextureOpaque(texture, width, height);
    void* encodeParameters[] = { texture };
    auto* png = Invoke(
        FindMethod(imageConversionClass, "EncodeToPNG", 1),
        nullptr,
        encodeParameters);

    bool written = false;
    if (png != nullptr)
    {
        const std::wstring rootDirectory =
            GetEscapeFromManakaSubdirectory(L"EquipmentColorCaptures");
        CreateDirectoryW(rootDirectory.c_str(), nullptr);
        std::wstring directory =
            rootDirectory + L"\\" +
            kEquipmentPreviewBackgroundDirectories[
                g_equipmentPreviewBackgroundIndex];
        CreateDirectoryW(directory.c_str(), nullptr);
        std::wstring fileName = Utf8ToWide(
            kEquipmentRenderEntries[g_equipmentPreviewCatalogIndex].slug);
        for (auto& character : fileName)
        {
            if (character == L'<' || character == L'>' ||
                character == L':' || character == L'"' ||
                character == L'/' || character == L'\\' ||
                character == L'|' || character == L'?' ||
                character == L'*')
            {
                character = L'_';
            }
            else if (character == L' ')
            {
                character = L'-';
            }
        }
        if (fileName.size() > 100)
        {
            fileName.resize(100);
        }
        char indexText[32]{};
        std::snprintf(
            indexText,
            sizeof(indexText),
            "%03d-",
            g_equipmentPreviewCatalogIndex + 1);
        const std::wstring path =
            directory + L"\\" + Utf8ToWide(indexText) + fileName + L".png";
        const DWORD length = il2cpp_array_length(png);
        auto* bytes = reinterpret_cast<unsigned char*>(
            reinterpret_cast<char*>(png) +
            il2cpp_array_object_header_size());
        HANDLE file = CreateFileW(
            path.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file != INVALID_HANDLE_VALUE)
        {
            DWORD totalWritten = 0;
            written = WriteFile(
                file,
                bytes,
                length,
                &totalWritten,
                nullptr) != FALSE &&
                totalWritten == length;
            CloseHandle(file);
            if (written)
            {
                LogInfo(LogCategory_Asset, L"Asset", L"Equipment preview wrote %s.", path.c_str());
            }
        }
    }
    EquipmentPreviewUi_DestroyObject(texture);
    void* restoreParameters[] = { previousValue };
    Invoke(
        FindMethod(renderTextureClass, "set_active", 1),
        nullptr,
        restoreParameters);
    return written;
}

void EquipmentPreviewUi_ForceAllActive()
{
    if (g_equipmentPreviewModel == nullptr ||
        g_equipmentPreviewReferencer == nullptr)
    {
        return;
    }
    g_equipmentPreviewBatchActive = false;
    EquipmentPreviewUi_ResetCatalogVisuals();
    EquipmentPreviewUi_SetArrayObjectsActive(0x288, true); // Costumes
    EquipmentPreviewUi_SetArrayObjectsActive(0x258, true); // Accessorys
    EquipmentPreviewUi_SetArrayObjectsActive(0x270, true); // Panties
    auto* cosplayControllers =
        EquipmentPreviewUi_GetReferencerArray(0x298);
    if (cosplayControllers != nullptr)
    {
        const auto count = il2cpp_array_length(cosplayControllers);
        for (uint32_t index = 0; index < count; ++index)
        {
            auto* controller = EquipmentPreviewUi_GetArrayElement(
                cosplayControllers,
                static_cast<int>(index));
            if (controller == nullptr)
            {
                continue;
            }
            SetWorldGameObjectActive(GetComponentGameObject(controller), true);
            auto* parts =
                *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<char*>(controller) + 0x20);
            if (parts == nullptr)
            {
                continue;
            }
            const auto partCount = il2cpp_array_length(parts);
            for (uint32_t partId = 0; partId < partCount; ++partId)
            {
                int id = static_cast<int>(partId);
                bool active = true;
                void* parameters[] = { &id, &active };
                Invoke(
                    FindMethod(
                        FindClass(
                            "ExposureUnnoticed2.Object3D.Player.Scripts.Costume",
                            "CosplayController"),
                        "SetActiveParts",
                        2),
                    controller,
                    parameters);
            }
        }
    }
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x238),
        true);
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x240),
        true);
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x268),
        true);
    SetWorldGameObjectActive(
        EquipmentPreviewUi_GetReferencerObject(0x260),
        true);

    EquipmentPreviewUi_HideAllRenderers();
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (rendererClass == nullptr || gameObjectClass == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType =
        il2cpp_type_get_object(il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* parameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        g_equipmentPreviewModel,
        parameters);
    if (renderers == nullptr)
    {
        return;
    }
    auto* getActiveInHierarchy =
        FindMethod(gameObjectClass, "get_activeInHierarchy", 0);
    const auto count = il2cpp_array_length(renderers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(renderers) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* renderer = elements[index];
        auto* gameObject = renderer != nullptr
            ? GetComponentGameObject(renderer)
            : nullptr;
        if (gameObject == nullptr)
        {
            continue;
        }
        const auto name = GetObjectName(gameObject);
        const auto path = EquipmentPreviewUi_GetTransformPath(gameObject);
        if (EquipmentPreviewUi_IsBodyRenderer(name) ||
            path.find(L"/Hair/") != std::wstring::npos ||
            path.find(L"/Censor/") != std::wstring::npos)
        {
            continue;
        }
        auto* activeValue = Invoke(
            getActiveInHierarchy,
            gameObject,
            nullptr);
        if (activeValue == nullptr || il2cpp_object_unbox == nullptr ||
            !*static_cast<bool*>(il2cpp_object_unbox(activeValue)))
        {
            continue;
        }
        EquipmentPreviewUi_SetRendererEnabled(renderer, true);
        g_equipmentPreviewRenderers.push_back({ renderer, name });
    }
    EquipmentPreviewUi_FrameSelectedRenderers();
    SetSceneText(
        g_equipmentPreviewItemLabel,
        "ALL EQUIPMENT VISUAL NODES FORCED ACTIVE",
        15.0f,
        514,
        1.0f,
        0.74f,
        0.38f,
        1.0f);
    InventoryUi_SetStatus("调试：全部装备节点已强制启用");
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Equipment force-all enabled %d renderer(s).",
        static_cast<int>(g_equipmentPreviewRenderers.size()));
}

void EquipmentPreviewUi_WriteManifest()
{
    const std::wstring directory =
        GetEscapeFromManakaSubdirectory(L"EquipmentColorCaptures");
    CreateDirectoryW(directory.c_str(), nullptr);
    const std::wstring path = directory + L"\\manifest.csv";
    std::string text = "index,slug,label\r\n";
    for (int index = 0; index < kEquipmentRenderEntryCount; ++index)
    {
        char line[384]{};
        std::snprintf(
            line,
            sizeof(line),
            "%d,%s,%s\r\n",
            index + 1,
            kEquipmentRenderEntries[index].slug,
            kEquipmentRenderEntries[index].label);
        text += line;
    }
    HANDLE file = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return;
    }
    DWORD written = 0;
    WriteFile(
        file,
        text.data(),
        static_cast<DWORD>(text.size()),
        &written,
        nullptr);
    CloseHandle(file);
}

void EquipmentPreviewUi_StartBatch()
{
    if (!g_equipmentPreviewOpen || g_equipmentPreviewRenderTexture == nullptr)
    {
        return;
    }
    g_equipmentPreviewBatchActive = true;
    // Give dependent meshes and material updates enough frames to settle
    // before the first capture. Three frames caused the first background to
    // omit delayed pieces on some layered outfits.
    g_equipmentPreviewBatchWaitFrames = 8;
    g_equipmentPreviewBatchCaptured = 0;
    g_equipmentPreviewBatchFailed = 0;
    g_equipmentPreviewReframeWaitFrames = 0;
    EquipmentPreviewUi_SetBackground(0);
    EquipmentPreviewUi_WriteManifest();
    EquipmentPreviewUi_Select(0);
    SetSceneText(
        g_equipmentPreviewItemLabel,
        "BATCH 1/5 BLACK  ·  1 / 208",
        15.0f,
        514,
        1.0f,
        0.78f,
        0.42f,
        1.0f);
    InventoryUi_SetStatus("正在批量导出 5 种背景 × 208 件装备");
    LogInfo(LogCategory_Asset, L"Asset", L"Equipment catalog batch capture started.");
}

void EquipmentPreviewUi_UpdateBatch()
{
    if (!g_equipmentPreviewBatchActive)
    {
        return;
    }
    if (g_equipmentPreviewBatchWaitFrames > 0)
    {
        --g_equipmentPreviewBatchWaitFrames;
        return;
    }
    if (EquipmentPreviewUi_SaveScreenshot())
    {
        ++g_equipmentPreviewBatchCaptured;
    }
    else
    {
        ++g_equipmentPreviewBatchFailed;
    }
    const int next = g_equipmentPreviewCatalogIndex + 1;
    if (next >= kEquipmentRenderEntryCount)
    {
        if (g_equipmentPreviewBackgroundIndex + 1 <
            kEquipmentPreviewBackgroundCount)
        {
            EquipmentPreviewUi_SetBackground(
                g_equipmentPreviewBackgroundIndex + 1);
            EquipmentPreviewUi_Select(0);
            g_equipmentPreviewBatchWaitFrames = 8;
            char nextBackground[160]{};
            std::snprintf(
                nextBackground,
                sizeof(nextBackground),
                "BATCH %d/%d %s  ·  1 / %d",
                g_equipmentPreviewBackgroundIndex + 1,
                kEquipmentPreviewBackgroundCount,
                kEquipmentPreviewBackgroundNames[
                    g_equipmentPreviewBackgroundIndex],
                kEquipmentRenderEntryCount);
            SetSceneText(
                g_equipmentPreviewItemLabel,
                nextBackground,
                14.0f,
                514,
                1.0f,
                0.78f,
                0.42f,
                1.0f);
            return;
        }
        g_equipmentPreviewBatchActive = false;
        char done[256]{};
        std::snprintf(
            done,
            sizeof(done),
            "5 组背景导出完成  ·  成功 %d  ·  失败 %d",
            g_equipmentPreviewBatchCaptured,
            g_equipmentPreviewBatchFailed);
        SetSceneText(
            g_equipmentPreviewItemLabel,
            done,
            15.0f,
            514,
            0.55f,
            1.0f,
            0.62f,
            1.0f);
        InventoryUi_SetStatus(done);
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Equipment catalog batch capture finished: ok=%d failed=%d.",
            g_equipmentPreviewBatchCaptured,
            g_equipmentPreviewBatchFailed);
        return;
    }
    EquipmentPreviewUi_Select(next);
    g_equipmentPreviewBatchWaitFrames = 8;
    char progress[128]{};
    std::snprintf(
        progress,
        sizeof(progress),
        "BATCH %d/%d %s  ·  %d / %d  ·  failed %d",
        g_equipmentPreviewBackgroundIndex + 1,
        kEquipmentPreviewBackgroundCount,
        kEquipmentPreviewBackgroundNames[g_equipmentPreviewBackgroundIndex],
        next + 1,
        kEquipmentRenderEntryCount,
        g_equipmentPreviewBatchFailed);
    SetSceneText(
        g_equipmentPreviewItemLabel,
        progress,
        14.0f,
        514,
        1.0f,
        0.78f,
        0.42f,
        1.0f);
}

void EquipmentPreviewUi_Close()
{
    if (!g_equipmentPreviewOpen && g_equipmentPreviewPanel == nullptr)
    {
        return;
    }
    g_equipmentPreviewBatchActive = false;
    g_equipmentPreviewOpen = false;
    if (g_equipmentPreviewPanel != nullptr)
    {
        SetWorldGameObjectActive(g_equipmentPreviewPanel, false);
    }
    g_equipmentPreviewRenderers.clear();
    g_equipmentPreviewIndex = -1;
    g_equipmentPreviewCatalogIndex = -1;
    g_equipmentPreviewReferencer = nullptr;
    g_equipmentPreviewBackgroundIndex = 0;
    EquipmentPreviewUi_DestroyStandaloneItem();
    EquipmentPreviewUi_DestroyObject(g_equipmentPreviewStage);
    g_equipmentPreviewStage = nullptr;
    g_equipmentPreviewModel = nullptr;
    g_equipmentPreviewStandaloneItem = nullptr;
    g_equipmentPreviewCamera = nullptr;
    EquipmentPreviewUi_DestroyObject(g_equipmentPreviewCameraObject);
    g_equipmentPreviewCameraObject = nullptr;
    if (g_equipmentPreviewRenderTexture != nullptr)
    {
        Invoke(
            FindMethod(FindClass("UnityEngine", "RenderTexture"), "Release", 0),
            g_equipmentPreviewRenderTexture,
            nullptr);
        EquipmentPreviewUi_DestroyObject(g_equipmentPreviewRenderTexture);
        g_equipmentPreviewRenderTexture = nullptr;
    }
    LogInfo(LogCategory_Asset, L"Asset", L"Equipment item preview closed and its clone released.");
}

void EquipmentPreviewUi_Open()
{
    if (g_equipmentPreviewOpen ||
        g_extendedModeState.load() != ExtendedMode_Active ||
        g_inventoryActiveRoute != kInventoryRouteEquipment ||
        g_extendedAvatarRoot == nullptr)
    {
        return;
    }
    g_extendedActionListOpen = false;
    if (g_extendedActionPanel != nullptr)
    {
        SetWorldGameObjectActive(g_extendedActionPanel, false);
    }
    EquipmentPreviewUi_CreateStage();
    EquipmentPreviewUi_EnsureUi();
    if (g_equipmentPreviewReferencer == nullptr ||
        g_equipmentPreviewPanel == nullptr)
    {
        InventoryUi_SetStatus("装备目录节点不可用，无法打开完整预览");
        EquipmentPreviewUi_Close();
        return;
    }
    g_equipmentPreviewOpen = true;
    g_equipmentPreviewBackgroundIndex = 0;
    EquipmentPreviewUi_BindTexture();
    SetWorldGameObjectActive(g_equipmentPreviewPanel, true);
    EquipmentPreviewUi_UpdateLayout();
    EquipmentPreviewUi_Select(0);
    InventoryUi_SetStatus("F5 完整装备目录已打开  ·  208 件");
    LogInfo(LogCategory_Asset, L"Asset", L"Equipment item preview opened.");
}

void EquipmentPreviewUi_Toggle()
{
    if (g_equipmentPreviewOpen)
    {
        EquipmentPreviewUi_Close();
        InventoryUi_SetStatus("F5 装备预览已关闭");
        return;
    }
    EquipmentPreviewUi_Open();
}

bool EquipmentPreviewUi_DildoCaptureRequested()
{
    static int requested = -1;
    if (requested < 0)
    {
        wchar_t value[16]{};
        const DWORD length = GetEnvironmentVariableW(
            L"MANAKA_CAPTURE_DILDO_ONCE",
            value,
            static_cast<DWORD>(
                sizeof(value) / sizeof(value[0])));
        requested =
            length > 0 &&
            (value[0] == L'1' ||
             value[0] == L't' ||
             value[0] == L'T')
                ? 1
                : 0;
    }
    return requested == 1;
}

void EquipmentPreviewUi_TickDildoAutoCapture()
{
    if (!EquipmentPreviewUi_DildoCaptureRequested() ||
        g_equipmentDildoCaptureState >= 2)
    {
        return;
    }
    if (g_equipmentDildoCaptureState == 0)
    {
        if (g_extendedModeState.load() != ExtendedMode_Active ||
            g_inventoryActiveRoute != kInventoryRouteEquipment ||
            g_extendedAvatarRoot == nullptr)
        {
            return;
        }
        EquipmentPreviewUi_Open();
        if (!g_equipmentPreviewOpen)
        {
            return;
        }
        int dildoIndex = -1;
        for (int index = 0;
             index < kEquipmentRenderEntryCount;
             ++index)
        {
            if (kEquipmentRenderEntries[index].kind == 6 &&
                std::strstr(
                    kEquipmentRenderEntries[index].slug,
                    "dildo") != nullptr)
            {
                dildoIndex = index;
                break;
            }
        }
        if (dildoIndex < 0)
        {
            g_equipmentDildoCaptureState = 2;
            LogInfo(LogCategory_Asset, L"Asset", 
                L"Automatic dildo capture failed: catalog entry is unavailable.");
            return;
        }
        g_equipmentPreviewBatchActive = false;
        EquipmentPreviewUi_SetBackground(0);
        EquipmentPreviewUi_Select(dildoIndex);
        g_equipmentDildoCaptureWaitFrames = 16;
        g_equipmentDildoCaptureState = 1;
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Automatic dildo capture started: catalogIndex=%d.",
            dildoIndex);
        return;
    }

    if (!g_equipmentPreviewOpen)
    {
        g_equipmentDildoCaptureState = 2;
        LogInfo(LogCategory_Asset, L"Asset", L"Automatic dildo capture stopped: preview closed.");
        return;
    }
    if (g_equipmentDildoCaptureWaitFrames > 0)
    {
        --g_equipmentDildoCaptureWaitFrames;
        return;
    }
    if (!EquipmentPreviewUi_SaveScreenshot())
    {
        g_equipmentDildoCaptureState = 2;
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Automatic dildo capture failed at background=%d.",
            g_equipmentPreviewBackgroundIndex);
        return;
    }
    const int nextBackground =
        g_equipmentPreviewBackgroundIndex + 1;
    if (nextBackground < kEquipmentPreviewBackgroundCount)
    {
        EquipmentPreviewUi_SetBackground(nextBackground);
        g_equipmentDildoCaptureWaitFrames = 16;
        return;
    }
    EquipmentPreviewUi_Close();
    g_equipmentDildoCaptureState = 2;
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Automatic dildo capture completed for five quality backgrounds.");
}

bool EquipmentPreviewUi_IsOpen()
{
    return g_equipmentPreviewOpen;
}

bool EquipmentPreviewUi_HandleButtonClick(Il2CppObject* instance)
{
    if (instance == g_equipmentPreviewPreviousButton.button)
    {
        g_equipmentPreviewBatchActive = false;
        EquipmentPreviewUi_Select(g_equipmentPreviewCatalogIndex - 1);
        return true;
    }
    if (instance == g_equipmentPreviewNextButton.button)
    {
        g_equipmentPreviewBatchActive = false;
        EquipmentPreviewUi_Select(g_equipmentPreviewCatalogIndex + 1);
        return true;
    }
    if (instance == g_equipmentPreviewCaptureButton.button)
    {
        InventoryUi_SetStatus(
            EquipmentPreviewUi_SaveScreenshot()
                ? "装备预览 PNG 已保存到 EscapeFromManaka\\EquipmentColorCaptures"
                : "装备预览截图失败，请查看日志");
        return true;
    }
    if (instance == g_equipmentPreviewForceAllButton.button)
    {
        EquipmentPreviewUi_ForceAllActive();
        return true;
    }
    if (instance == g_equipmentPreviewBatchButton.button)
    {
        if (g_equipmentPreviewBatchActive)
        {
            g_equipmentPreviewBatchActive = false;
            InventoryUi_SetStatus("装备批量截图已停止");
        }
        else
        {
            EquipmentPreviewUi_StartBatch();
        }
        return true;
    }
    if (instance == g_equipmentPreviewCloseButton.button)
    {
        EquipmentPreviewUi_Close();
        InventoryUi_SetStatus("F5 装备预览已关闭");
        return true;
    }
    return false;
}

bool EquipmentPreviewUi_HandleManualPointer(float cursorX, float cursorY)
{
    if (!g_equipmentPreviewOpen)
    {
        return false;
    }
    constexpr float panelMinX = 0.250f;
    constexpr float panelMinY = 0.180f;
    constexpr float panelMaxX = 0.750f;
    constexpr float panelMaxY = 0.840f;
    if (cursorX < panelMinX || cursorX > panelMaxX ||
        cursorY < panelMinY || cursorY > panelMaxY)
    {
        return true;
    }
    const float localX =
        (cursorX - panelMinX) / (panelMaxX - panelMinX);
    const float localY =
        (cursorY - panelMinY) / (panelMaxY - panelMinY);
    if (localY < 0.035f || localY > 0.105f)
    {
        return true;
    }
    if (localX >= 0.030f && localX <= 0.145f)
    {
        g_equipmentPreviewBatchActive = false;
        EquipmentPreviewUi_Select(g_equipmentPreviewCatalogIndex - 1);
    }
    else if (localX >= 0.155f && localX <= 0.270f)
    {
        g_equipmentPreviewBatchActive = false;
        EquipmentPreviewUi_Select(g_equipmentPreviewCatalogIndex + 1);
    }
    else if (localX >= 0.280f && localX <= 0.415f)
    {
        InventoryUi_SetStatus(
            EquipmentPreviewUi_SaveScreenshot()
                ? "装备预览 PNG 已保存到 EscapeFromManaka\\EquipmentColorCaptures"
                : "装备预览截图失败，请查看日志");
    }
    else if (localX >= 0.425f && localX <= 0.580f)
    {
        EquipmentPreviewUi_ForceAllActive();
    }
    else if (localX >= 0.590f && localX <= 0.755f)
    {
        if (g_equipmentPreviewBatchActive)
        {
            g_equipmentPreviewBatchActive = false;
            InventoryUi_SetStatus("装备批量截图已停止");
        }
        else
        {
            EquipmentPreviewUi_StartBatch();
        }
    }
    else if (localX >= 0.765f && localX <= 0.965f)
    {
        EquipmentPreviewUi_Close();
        InventoryUi_SetStatus("F5 装备预览已关闭");
    }
    return true;
}

void EquipmentPreviewUi_TickMainThread()
{
    if (!g_equipmentPreviewOpen)
    {
        return;
    }
    if (g_extendedModeState.load() != ExtendedMode_Active ||
        g_inventoryActiveRoute != kInventoryRouteEquipment)
    {
        EquipmentPreviewUi_Close();
        return;
    }
    if (g_extendedToggleEquipmentCatalogBatchRequested.exchange(false) &&
        !g_equipmentPreviewBatchActive)
    {
        EquipmentPreviewUi_StartBatch();
    }
    EquipmentPreviewUi_UpdateLayout();
    if (g_equipmentPreviewReframeWaitFrames > 0 &&
        --g_equipmentPreviewReframeWaitFrames == 0)
    {
        EquipmentPreviewUi_FrameSelectedRenderers();
    }
    EquipmentPreviewUi_UpdateBatch();
}

void EquipmentPreviewUi_Reset()
{
    g_equipmentPreviewOpen = false;
    g_equipmentPreviewPanel = nullptr;
    g_equipmentPreviewImageRect = nullptr;
    g_equipmentPreviewRawImage = nullptr;
    g_equipmentPreviewTitle = nullptr;
    g_equipmentPreviewItemLabel = nullptr;
    g_equipmentPreviewPreviousButton = SceneUiElement{};
    g_equipmentPreviewNextButton = SceneUiElement{};
    g_equipmentPreviewCaptureButton = SceneUiElement{};
    g_equipmentPreviewForceAllButton = SceneUiElement{};
    g_equipmentPreviewBatchButton = SceneUiElement{};
    g_equipmentPreviewCloseButton = SceneUiElement{};
    g_equipmentPreviewStage = nullptr;
    g_equipmentPreviewModel = nullptr;
    g_equipmentPreviewStandaloneItem = nullptr;
    g_equipmentPreviewCamera = nullptr;
    g_equipmentPreviewCameraObject = nullptr;
    g_equipmentPreviewRenderTexture = nullptr;
    g_equipmentPreviewReferencer = nullptr;
    g_equipmentPreviewRenderers.clear();
    g_equipmentPreviewIndex = -1;
    g_equipmentPreviewCatalogIndex = -1;
    g_equipmentPreviewBackgroundIndex = 0;
    g_equipmentPreviewBatchActive = false;
    g_equipmentPreviewBatchWaitFrames = 0;
    g_equipmentPreviewBatchCaptured = 0;
    g_equipmentPreviewBatchFailed = 0;
    g_equipmentPreviewReframeWaitFrames = 0;
}
