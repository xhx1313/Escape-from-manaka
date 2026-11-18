// Native.cpp module: interaction_filter.inc
// Preserve the game's complete InteractArea pipeline. Map portals are handled
// separately below because Escape mode controls scene entry and extraction.

constexpr int kInteractTypeUseBuyMachine = 10;
constexpr int kInteractTypeSitChair = 17;
constexpr int kInteractTypeElevatorCallButton = 18;
constexpr int kInteractTypePinpon = 22;

void* g_originalInteractManagerRegister = nullptr;
void* g_originalInteractManagerUnregister = nullptr;
bool g_interactionWhitelistLogged[32]{};

bool InteractionFilter_IsAllowedType(int type)
{
    (void)type;
    return true;
}

std::wstring InteractionFilter_ObjectPath(Il2CppObject* gameObject)
{
    std::wstring path;
    auto* current = gameObject;
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* componentClass = FindClass("UnityEngine", "Component");
    auto* getTransform = gameObject != nullptr
        ? FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0)
        : nullptr;
    auto* getParent = FindMethod(transformClass, "get_parent", 0);
    auto* getGameObject = FindMethod(componentClass, "get_gameObject", 0);
    for (int depth = 0; current != nullptr && depth < 8; ++depth)
    {
        const auto name = GetObjectName(current);
        path = path.empty() ? name : name + L"/" + path;
        auto* transform = Invoke(getTransform, current, nullptr);
        auto* parent = transform != nullptr ? Invoke(getParent, transform, nullptr) : nullptr;
        current = parent != nullptr ? Invoke(getGameObject, parent, nullptr) : nullptr;
    }
    return path;
}

Il2CppObject* InteractionFilter_FindVisualRoot(Il2CppObject* controller)
{
    auto* gameObject = GetComponentGameObject(controller);
    if (gameObject == nullptr)
    {
        return nullptr;
    }
    auto* selected = gameObject;
    auto* current = gameObject;
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* componentClass = FindClass("UnityEngine", "Component");
    auto* getTransform = FindMethod(
        FindClass("UnityEngine", "GameObject"),
        "get_transform",
        0);
    auto* getParent = FindMethod(transformClass, "get_parent", 0);
    auto* getGameObject = FindMethod(componentClass, "get_gameObject", 0);
    // Interaction colliders are frequently children of a controller child while
    // the renderable prop is a sibling under the room/container transform.
    // Climb a little farther so toilets, doors, counters and NPC roots are
    // hidden with their interaction trigger. A scene root has no parent and is
    // never selected.
    for (int depth = 0; depth < 8 && current != nullptr; ++depth)
    {
        auto* transform = Invoke(getTransform, current, nullptr);
        auto* parent = transform != nullptr
            ? Invoke(getParent, transform, nullptr)
            : nullptr;
        auto* parentObject = parent != nullptr
            ? Invoke(getGameObject, parent, nullptr)
            : nullptr;
        if (parentObject == nullptr)
        {
            break;
        }
        const int rendererCount = CountComponentsInChildren(
            current,
            "UnityEngine",
            "Renderer");
        if (rendererCount > 0)
        {
            selected = current;
            break;
        }
        current = parentObject;
    }
    return selected;
}

void InteractionFilter_HideDeniedController(
    int type,
    Il2CppObject* controller)
{
    auto* controllerObject = GetComponentGameObject(controller);
    auto* visualRoot = InteractionFilter_FindVisualRoot(controller);
    bool logged = false;
    if (type >= 0 && type < 32)
    {
        logged = g_interactionWhitelistLogged[type];
        g_interactionWhitelistLogged[type] = true;
    }
    if (!logged)
    {
        const auto controllerPath =
            InteractionFilter_ObjectPath(controllerObject);
        const auto visualPath =
            InteractionFilter_ObjectPath(visualRoot);
        LogInfo(LogCategory_Scene, L"Scene", 
            L"Escape interaction filter denied type=%d controller=%s visual=%s.",
            type,
            controllerPath.c_str(),
            visualPath.c_str());
    }
    if (visualRoot != nullptr)
    {
        SetWorldGameObjectActive(visualRoot, false);
    }
    if (controllerObject != nullptr &&
        controllerObject != visualRoot)
    {
        SetWorldGameObjectActive(controllerObject, false);
    }
}

void InteractionFilter_DisablePortalTriggers(
    bool hideObjects,
    bool forceLog)
{
    auto* portalClass = FindClass(
        "ExposureUnnoticed2.Object3D.Portal",
        "PortalController");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (portalClass == nullptr ||
        objectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr ||
        il2cpp_type_get_object == nullptr ||
        il2cpp_array_length == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        return;
    }

    il2cpp_runtime_class_init(portalClass);
    auto* portalType = il2cpp_type_get_object(
        il2cpp_class_get_type(portalClass));
    bool includeInactive = true;
    void* portalParameters[] = { portalType, &includeInactive };
    auto* portals = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        portalParameters);
    if (portals == nullptr)
    {
        return;
    }

    auto* colliderClass = FindClass("UnityEngine", "Collider");
    auto* setColliderEnabled = colliderClass != nullptr
        ? FindMethod(colliderClass, "set_enabled", 1)
        : nullptr;
    auto* portalElements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(portals) +
        il2cpp_array_object_header_size());
    const auto portalCount = il2cpp_array_length(portals);
    int disabledTriggers = 0;
    int hiddenObjects = 0;
    for (uint32_t index = 0; index < portalCount; ++index)
    {
        auto* portal = portalElements[index];
        auto* portalObject = portal != nullptr
            ? GetComponentGameObject(portal)
            : nullptr;
        if (portalObject == nullptr)
        {
            continue;
        }
        if (setColliderEnabled != nullptr)
        {
            auto* collider = GetSceneComponentByClass(
                portalObject,
                "UnityEngine",
                "Collider");
            if (collider != nullptr)
            {
                bool enabled = false;
                void* enabledParameters[] = { &enabled };
                Invoke(setColliderEnabled, collider, enabledParameters);
                ++disabledTriggers;
            }
        }
        if (hideObjects)
        {
            SetWorldGameObjectActive(portalObject, false);
            ++hiddenObjects;
        }
    }

    static int lastDisabledTriggers = -1;
    static int lastHiddenObjects = -1;
    int& lastCount = hideObjects
        ? lastHiddenObjects
        : lastDisabledTriggers;
    const int currentCount = hideObjects
        ? hiddenObjects
        : disabledTriggers;
    if (forceLog || currentCount != lastCount)
    {
        lastCount = currentCount;
        LogInfo(
            LogCategory_Scene,
            L"Scene",
            L"Escape interaction filter pre-disabled %d portal trigger(s); "
            L"hidden portal object(s)=%d.",
            disabledTriggers,
            hiddenObjects);
    }
}

void InteractionFilter_ApplySceneWhitelist()
{
    auto* controllerClass = FindClass(
        "ExposureUnnoticed2.Object3D.InteractArea",
        "InteractAreaController");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (controllerClass == nullptr ||
        objectClass == nullptr ||
        il2cpp_runtime_class_init == nullptr ||
        il2cpp_type_get_object == nullptr)
    {
        return;
    }
    il2cpp_runtime_class_init(controllerClass);
    auto* managedType = il2cpp_type_get_object(
        il2cpp_class_get_type(controllerClass));
    bool includeInactive = true;
    void* parameters[] = { managedType, &includeInactive };
    auto* controllers = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (controllers == nullptr)
    {
        return;
    }
    auto* getType = FindMethod(controllerClass, "get_Type", 0);
    if (getType == nullptr)
    {
        return;
    }
    const auto count = il2cpp_array_length(controllers);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(controllers) +
        il2cpp_array_object_header_size());
    int deniedCount = 0;
    int allowedCount = 0;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* controller = elements[index];
        if (controller == nullptr)
        {
            continue;
        }
        auto* typeValue = Invoke(getType, controller, nullptr);
        if (typeValue == nullptr ||
            il2cpp_object_unbox == nullptr)
        {
            continue;
        }
        const int type = *static_cast<int*>(
            il2cpp_object_unbox(typeValue));
        if (InteractionFilter_IsAllowedType(type))
        {
            ++allowedCount;
            if (type >= 0 &&
                type < 32 &&
                !g_interactionWhitelistLogged[type])
            {
                g_interactionWhitelistLogged[type] = true;
                LogInfo(LogCategory_Scene, L"Scene", 
                    L"Escape interaction filter kept type=%d at %s.",
                    type,
                    InteractionFilter_ObjectPath(
                        GetComponentGameObject(controller)).c_str());
            }
            continue;
        }
        InteractionFilter_HideDeniedController(type, controller);
        ++deniedCount;
    }

    // Disable the portal trigger before any later scene code can accept an
    // entry. The spawn path calls this helper earlier, before moving the
    // player, so a spawn overlapping the portal cannot teleport immediately.
    InteractionFilter_DisablePortalTriggers(true, false);

    static unsigned int lastDenied = 0xFFFFFFFFu;
    static unsigned int lastAllowed = 0xFFFFFFFFu;
    if (deniedCount != static_cast<int>(lastDenied) ||
        allowedCount != static_cast<int>(lastAllowed))
    {
        lastDenied = static_cast<unsigned int>(deniedCount);
        lastAllowed = static_cast<unsigned int>(allowedCount);
        LogInfo(LogCategory_Scene, L"Scene", 
            L"Escape interaction scene scan: kept=%d hidden=%d.",
            allowedCount,
            deniedCount);
    }
}

void InteractManagerRegisterHook(
    int type,
    Il2CppObject* controller,
    const Il2CppMethod* method)
{
    if (!InteractionFilter_IsAllowedType(type))
    {
        InteractionFilter_HideDeniedController(type, controller);
        return;
    }
    if (g_originalInteractManagerRegister != nullptr)
    {
        using OriginalFn =
            void (*)(int, Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalInteractManagerRegister)(
            type,
            controller,
            method);
    }
}

void InteractManagerUnregisterHook(
    int type,
    Il2CppObject* controller,
    const Il2CppMethod* method)
{
    if (g_originalInteractManagerUnregister != nullptr)
    {
        using OriginalFn =
            void (*)(int, Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalInteractManagerUnregister)(
            type,
            controller,
            method);
    }
}
