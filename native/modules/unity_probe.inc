// Native.cpp module: unity_probe.inc
// Included inside the shared implementation namespace by Native.cpp.
void ProbeCharacterSystems()
{
    static bool probed = false;
    if (probed || g_titleInstance == nullptr)
    {
        return;
    }
    probed = true;

    const auto demoObject = Invoke(
        FindMethod(g_titleClass, "get_demoObject", 0),
        g_titleInstance,
        nullptr);
    LogObjectReference("Title demoObject", demoObject);

    auto* uiPlayerClass = FindClass(
        "ExposureUnnoticed2.Object3D.UIPlayer",
        "UiPlayerController");
    const auto uiPlayerInstance = Invoke(
        FindMethod(uiPlayerClass, "get_Instance", 0),
        nullptr,
        nullptr);
    LogObjectReference("UiPlayerController.Instance", uiPlayerInstance);
    if (uiPlayerInstance == nullptr)
    {
        LogObjectReference(
            "UiPlayerController inactive instance",
            FindFirstComponentOfClass(uiPlayerClass));
    }

    auto* inventoryClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.InventoryPanel",
        "InventoryPanelView");
    const auto inventoryInstance = Invoke(
        FindMethod(inventoryClass, "get_Instance", 0),
        nullptr,
        nullptr);
    LogObjectReference("InventoryPanelView.Instance", inventoryInstance);
    if (inventoryInstance == nullptr)
    {
        LogObjectReference(
            "InventoryPanelView inactive instance",
            FindFirstComponentOfClass(inventoryClass));
    }

    LogMethodRva(
        "UiPlayerController.Initialize",
        "ExposureUnnoticed2.Object3D.UIPlayer",
        "UiPlayerController",
        "Initialize",
        0);
    LogMethodRva(
        "UiPlayerController.Show",
        "ExposureUnnoticed2.Object3D.UIPlayer",
        "UiPlayerController",
        "Show",
        0);
    LogMethodRva(
        "NpcAvatarSelector.CreateAvatar",
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAvatarSelector",
        "CreateAvatar",
        1);
    LogMethodRva(
        "NpcAvatarComponent.Initialize",
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAvatarComponent",
        "Initialize",
        1);
    LogMethodRva(
        "NpcAvatarComponent.InitializeForPlayer",
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAvatarComponent",
        "InitializeForPlayer",
        0);
    LogMethodRva(
        "NpcAvatarComponent.InitializeForPlayerCustom",
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcAvatarComponent",
        "InitializeForPlayerCustom",
        0);
    LogMethodRva(
        "PlayerController.Initialize",
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerController",
        "Initialize",
        5);
    LogMethodRva(
        "NpcManager.Initialize",
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcManager",
        "Initialize",
        1);
    LogMethodRva(
        "Object.Instantiate/Object",
        "UnityEngine",
        "Object",
        "Instantiate",
        1);
    LogMethodRva(
        "Object.Instantiate/ObjectTransform",
        "UnityEngine",
        "Object",
        "Instantiate",
        2);
    LogMethodRva(
        "Object.Instantiate/ObjectPositionRotation",
        "UnityEngine",
        "Object",
        "Instantiate",
        3);
    LogMethodRva(
        "Resources.Load/String",
        "UnityEngine",
        "Resources",
        "Load",
        1);
    LogMethodRva(
        "Resources.Load/StringType",
        "UnityEngine",
        "Resources",
        "Load",
        2);
    LogMethodRva(
        "NpcComponent.get_AvaterObject",
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcComponent",
        "get_AvaterObject",
        0);
    LogMethodRva(
        "PlayerClassAccessor.get_AvatorTransform",
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerClassAccessor",
        "get_AvatorTransform",
        0);

    auto* assetReferencerClass = FindClass(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "AssetReferencer");
    if (assetReferencerClass != nullptr)
    {
        il2cpp_runtime_class_init(assetReferencerClass);
        auto* instanceField = il2cpp_class_get_field_from_name(
            assetReferencerClass,
            "Instance");
        Il2CppObject* assetReferencer = nullptr;
        if (instanceField != nullptr)
        {
            il2cpp_field_static_get_value(instanceField, &assetReferencer);
        }
        LogObjectReference("AssetReferencer.Instance", assetReferencer);
        if (assetReferencer == nullptr)
        {
            assetReferencer = FindFirstComponentOfClass(assetReferencerClass);
            LogObjectReference(
                "AssetReferencer loaded component",
                assetReferencer);
        }
        if (assetReferencer != nullptr)
        {
            auto* uiPlayerPrefab = *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(assetReferencer) + 0x350);
            LogObjectReference("AssetReferencer.UiPlayer prefab", uiPlayerPrefab);
            if (uiPlayerPrefab != nullptr)
            {
                LogInfo(LogCategory_Scene, L"Scene", 
                    L"AssetReferencer.UiPlayer renderers=%d",
                    CountComponentsInChildren(
                        uiPlayerPrefab,
                        "UnityEngine",
                        "SkinnedMeshRenderer"));
            }
        }
    }

    LogLoadedAssetsByClass("UnityEngine", "RuntimeAnimatorController");
    LogLoadedAssetsByClass("UnityEngine", "Avatar");

    auto* resourcesClass = FindClass("UnityEngine", "Resources");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    il2cpp_runtime_class_init(gameObjectClass);
    auto* gameObjectType = il2cpp_type_get_object(
        il2cpp_class_get_type(gameObjectClass));
    auto* assetReferencerName = il2cpp_string_new("AssetReferencer");
    void* assetReferencerLoadParameters[] = {
        assetReferencerName,
        gameObjectType
    };
    auto* resourcesAssetReferencer = Invoke(
        FindMethod(resourcesClass, "Load", 2),
        nullptr,
        assetReferencerLoadParameters);
    LogObjectReference(
        "Resources AssetReferencer",
        resourcesAssetReferencer);
    auto* uiPlayerName = il2cpp_string_new("UiPlayer");
    void* uiPlayerLoadParameters[] = { uiPlayerName, gameObjectType };
    auto* resourcesUiPlayer = Invoke(
        FindMethod(resourcesClass, "Load", 2),
        nullptr,
        uiPlayerLoadParameters);
    LogObjectReference("Resources UiPlayer", resourcesUiPlayer);

    auto* bundleClass = FindClass("UnityEngine", "AssetBundle");
    const auto bundlePath = GetModuleDirectory() +
        L"\\SecretFlasherManaka_Data\\data.unity3d";
    auto* pathString = il2cpp_string_new(WideToUtf8(bundlePath).c_str());
    void* loadBundleParameters[] = { pathString };
    auto* bundle = Invoke(
        FindMethod(bundleClass, "LoadFromFile", 1),
        nullptr,
        loadBundleParameters);
    if (bundle == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Could not open data.unity3d as an AssetBundle.");
        return;
    }

    const auto getAllAssetNames = FindMethod(bundleClass, "GetAllAssetNames", 0);
    auto* names = Invoke(getAllAssetNames, bundle, nullptr);
    if (names == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"AssetBundle contains no readable asset-name inventory.");
        return;
    }

    const auto nameCount = il2cpp_array_length(names);
    auto* nameElements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(names) + il2cpp_array_object_header_size());
    int loggedNames = 0;
    for (uint32_t nameIndex = 0; nameIndex < nameCount && loggedNames < 220; ++nameIndex)
    {
        const auto name = ReadIl2CppString(
            static_cast<Il2CppString*>(nameElements[nameIndex]));
        if (name.find(L"player") != std::wstring::npos ||
            name.find(L"Player") != std::wstring::npos ||
            name.find(L"inventory") != std::wstring::npos ||
            name.find(L"Inventory") != std::wstring::npos ||
            name.find(L"closet") != std::wstring::npos ||
            name.find(L"Closet") != std::wstring::npos ||
            name.find(L"ui") != std::wstring::npos ||
            name.find(L"UI") != std::wstring::npos ||
            name.find(L"menu") != std::wstring::npos ||
            name.find(L"Menu") != std::wstring::npos)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"AssetBundle asset: %s", name.c_str());
            ++loggedNames;
        }
    }
    LogInfo(LogCategory_Scene, L"Scene", L"AssetBundle probe complete: %u names, %d matches.", nameCount, loggedNames);
}

Il2CppObject* GetComponentByName(Il2CppObject* gameObject, const char* typeName)
{
    if (gameObject == nullptr || il2cpp_string_new == nullptr)
    {
        return nullptr;
    }

    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    const auto getComponent = FindMethod(gameObjectClass, "GetComponentByName", 1);
    auto* name = il2cpp_string_new(typeName);
    void* parameters[] = { name };
    return Invoke(getComponent, gameObject, parameters);
}

Il2CppObject* FindAnActiveTitleButton()
{
    const auto managerGetter = FindMethod(g_titleClass, "get_buttonGroupManager", 0);
    auto* manager = Invoke(managerGetter, g_titleInstance, nullptr);
    if (manager == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Title ButtonGroupManager is unavailable.");
        return nullptr;
    }

    auto* managerClass = FindClass("HadakaCoat.ObjectsUi.Common.Button", "ButtonGroupManager");
    auto* buttons = Invoke(FindMethod(managerClass, "get_buttons", 0), manager, nullptr);
    if (buttons == nullptr || il2cpp_array_length == nullptr || il2cpp_array_object_header_size == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Title ButtonGroupManager.buttons is unavailable.");
        return nullptr;
    }

    const auto count = il2cpp_array_length(buttons);
    const auto headerSize = il2cpp_array_object_header_size();
    auto* elements = reinterpret_cast<Il2CppObject**>(reinterpret_cast<char*>(buttons) + headerSize);
    auto* buttonViewClass = FindClass("UnityEngine.UI", "Button");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    const auto getGameObject = FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0);
    const auto getActive = FindMethod(gameObjectClass, "get_activeInHierarchy", 0);

    for (uint32_t index = 0; index < count; ++index)
    {
        auto* buttonView = elements[index];
        if (buttonView == nullptr)
        {
            continue;
        }
        auto* gameObject = Invoke(getGameObject, buttonView, nullptr);
        if (gameObject == nullptr)
        {
            continue;
        }
        auto* activeValue = Invoke(getActive, gameObject, nullptr);
        const bool active = activeValue != nullptr &&
                            il2cpp_object_unbox != nullptr &&
                            *static_cast<bool*>(il2cpp_object_unbox(activeValue));
        if (active || index == 0)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"Using title ButtonGroupManager button index %u.", index);
            return gameObject;
        }
    }

    return nullptr;
}

Il2CppObject* FindButtonInTransform(Il2CppObject* transform, int depth);

Il2CppObject* FindButtonInGameObject(Il2CppObject* gameObject, int depth)
{
    if (gameObject == nullptr || depth > 10)
    {
        return nullptr;
    }

    auto* buttonView = GetComponentByName(gameObject, "Button");
    if (buttonView != nullptr)
    {
        auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
        auto* activeValue = Invoke(FindMethod(gameObjectClass, "get_activeInHierarchy", 0), gameObject, nullptr);
        const bool active = activeValue != nullptr &&
                            il2cpp_object_unbox != nullptr &&
                            *static_cast<bool*>(il2cpp_object_unbox(activeValue));
        if (active)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"Found active UI Button in title hierarchy: %s", GetObjectName(gameObject).c_str());
            return gameObject;
        }
    }

    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* transform = Invoke(FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0), gameObject, nullptr);
    return FindButtonInTransform(transform, depth + 1);
}

Il2CppObject* FindButtonInTransform(Il2CppObject* transform, int depth)
{
    if (transform == nullptr || depth > 10)
    {
        return nullptr;
    }

    auto* childCountValue = Invoke(FindMethod(FindClass("UnityEngine", "Transform"), "get_childCount", 0), transform, nullptr);
    if (childCountValue == nullptr || il2cpp_object_unbox == nullptr)
    {
        return nullptr;
    }

    const int childCount = *static_cast<int*>(il2cpp_object_unbox(childCountValue));
    auto* transformClass = FindClass("UnityEngine", "Transform");
    for (int index = 0; index < childCount; ++index)
    {
        void* childParameters[] = { &index };
        auto* child = Invoke(FindMethod(transformClass, "GetChild", 1), transform, childParameters);
        auto* childGameObject = Invoke(
            FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
            child,
            nullptr);
        auto* found = FindButtonInGameObject(childGameObject, depth);
        if (found != nullptr)
        {
            return found;
        }
    }
    return nullptr;
}

std::wstring GetButtonLabel(Il2CppObject* gameObject)
{
    auto* tmpTextClass = FindClass("TMPro", "TMP_Text");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (tmpTextClass == nullptr || gameObjectClass == nullptr)
    {
        return std::wstring();
    }

    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(tmpTextClass);
    }
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(tmpTextClass));
    bool includeInactive = true;
    void* findParameters[] = { managedType, &includeInactive };
    auto* components = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        gameObject,
        findParameters);
    if (components == nullptr)
    {
        return std::wstring();
    }

    const auto count = il2cpp_array_length(components);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(components) + il2cpp_array_object_header_size());
    const auto getText = FindMethod(tmpTextClass, "get_text", 0);
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* value = Invoke(getText, elements[index], nullptr);
        const auto label = ReadIl2CppString(static_cast<Il2CppString*>(value));
        if (!label.empty())
        {
            return label;
        }
    }
    return std::wstring();
}

Il2CppObject* FindAnyButtonView()
{
    if (il2cpp_class_get_type == nullptr || il2cpp_type_get_object == nullptr)
    {
        return nullptr;
    }

    auto* buttonViewClass = FindClass("UnityEngine.UI", "Button");
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    LogInfo(LogCategory_Scene, L"Scene", L"Button class=%p Object class=%p", buttonViewClass, objectClass);
    if (buttonViewClass == nullptr || objectClass == nullptr)
    {
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(buttonViewClass);
        il2cpp_runtime_class_init(objectClass);
    }
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(buttonViewClass));
    const auto findAll = FindMethod(objectClass, "FindObjectsOfType", 2);
    LogInfo(LogCategory_Scene, L"Scene", L"Button managed type=%p FindObjectsOfType=%p", managedType, findAll);
    if (findAll == nullptr)
    {
        void* methodIterator = nullptr;
        while (const auto method = il2cpp_class_get_methods(objectClass, &methodIterator))
        {
            const auto* methodName = il2cpp_method_get_name(method);
            if (methodName != nullptr && std::strstr(methodName, "FindObjects") != nullptr)
            {
                LogInfo(LogCategory_Scene, L"Scene", 
                    L"Object method candidate: %S params=%u generic=%d",
                    methodName,
                    il2cpp_method_get_param_count(method),
                    il2cpp_method_is_generic(method));
            }
        }
    }
    bool includeInactive = true;
    void* findParameters[] = { managedType, &includeInactive };
    auto* buttons = Invoke(findAll, nullptr, findParameters);
    if (buttons == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"UnityEngine.Object.FindObjectsOfType returned no ButtonView array.");
        return nullptr;
    }

    const auto count = il2cpp_array_length(buttons);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(buttons) + il2cpp_array_object_header_size());
    const auto getGameObject = FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0);
    const auto getActive = FindMethod(gameObjectClass, "get_activeInHierarchy", 0);
    Il2CppObject* firstActive = nullptr;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* buttonView = elements[index];
        if (buttonView == nullptr)
        {
            continue;
        }
        auto* gameObject = Invoke(getGameObject, buttonView, nullptr);
        if (gameObject == nullptr)
        {
            continue;
        }
        auto* activeValue = Invoke(getActive, gameObject, nullptr);
        if (activeValue != nullptr &&
            *static_cast<bool*>(il2cpp_object_unbox(activeValue)))
        {
            const auto buttonName = GetObjectName(gameObject);
            const auto label = GetButtonLabel(gameObject);
            LogInfo(LogCategory_Scene, L"Scene", L"Active UI Button: name=%s label=%s", buttonName.c_str(), label.c_str());
            if (firstActive == nullptr)
            {
                firstActive = buttonView;
            }
            g_titleMenuButtons.push_back(buttonView);
            if (g_continueButtonComponent == nullptr && label.find(L"继续") != std::wstring::npos)
            {
                g_continueButtonComponent = buttonView;
            }
            if (label.find(L"从头开始") != std::wstring::npos ||
                label.find(L"开始游戏") != std::wstring::npos ||
                label.find(L"New Game") != std::wstring::npos ||
                label.find(L"NEW GAME") != std::wstring::npos ||
                label.find(L"はじめ") != std::wstring::npos)
            {
                g_startButtonComponent = buttonView;
                LogInfo(LogCategory_Scene, L"Scene", L"Selected title start button: %s", label.c_str());
                return buttonView;
            }
        }
    }

    if (firstActive != nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Using first active UI Button as fallback.");
    }
    return firstActive;
}

Il2CppObject* FindTextInTransform(Il2CppObject* transform, int depth);

Il2CppObject* FindTextInGameObject(Il2CppObject* gameObject, int depth)
{
    if (gameObject == nullptr || depth > 8)
    {
        return nullptr;
    }

    auto* text = GetComponentByName(gameObject, "TextMeshProUGUI");
    if (text == nullptr)
    {
        text = GetComponentByName(gameObject, "TMP_Text");
    }
    if (text != nullptr)
    {
        auto* tmpTextClass = FindClass("TMPro", "TMP_Text");
        auto* currentValue = Invoke(FindMethod(tmpTextClass, "get_text", 0), text, nullptr);
        LogInfo(LogCategory_Scene, L"Scene", 
            L"TMP text candidate in %s: %s",
            GetObjectName(gameObject).c_str(),
            ReadIl2CppString(static_cast<Il2CppString*>(currentValue)).c_str());
        return text;
    }

    auto* transform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        gameObject,
        nullptr);
    return FindTextInTransform(transform, depth + 1);
}

Il2CppObject* FindTextInTransform(Il2CppObject* transform, int depth)
{
    if (transform == nullptr || depth > 8)
    {
        return nullptr;
    }

    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* childCountValue = Invoke(FindMethod(transformClass, "get_childCount", 0), transform, nullptr);
    if (childCountValue == nullptr)
    {
        return nullptr;
    }
    const int childCount = *static_cast<int*>(il2cpp_object_unbox(childCountValue));
    for (int index = 0; index < childCount; ++index)
    {
        void* childParameters[] = { &index };
        auto* child = Invoke(FindMethod(transformClass, "GetChild", 1), transform, childParameters);
        auto* childGameObject = Invoke(
            FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
            child,
            nullptr);
        auto* text = FindTextInGameObject(childGameObject, depth);
        if (text != nullptr)
        {
            return text;
        }
    }
    return nullptr;
}

void SetAllButtonTexts(Il2CppObject* gameObject, const char* value, bool verbose = true)
{
    auto* tmpTextClass = FindClass("TMPro", "TMP_Text");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* componentClass = FindClass("UnityEngine", "Component");
    if (tmpTextClass == nullptr || gameObjectClass == nullptr || componentClass == nullptr)
    {
        return;
    }

    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(tmpTextClass);
    }
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(tmpTextClass));
    bool includeInactive = true;
    void* findParameters[] = { managedType, &includeInactive };
    auto* components = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        gameObject,
        findParameters);
    if (components == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"No TMP text components were found below the cloned button.");
        return;
    }

    const auto count = il2cpp_array_length(components);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(components) + il2cpp_array_object_header_size());
    auto* setText = FindMethod(tmpTextClass, "set_text", 1);
    auto* textString = il2cpp_string_new(value);
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* component = elements[index];
        if (component == nullptr)
        {
            continue;
        }
        auto* componentGameObject = Invoke(
            FindMethod(componentClass, "get_gameObject", 0),
            component,
            nullptr);
        if (verbose)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"Updating cloned-button TMP component %u on %s.", index, GetObjectName(componentGameObject).c_str());
        }
        void* textParameters[] = { textString };
        Invoke(setText, component, textParameters);
    }
    if (verbose)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Updated %u TMP text component(s) on the cloned button.", count);
    }
}

void ClearButtonListeners(Il2CppObject* buttonComponent, bool verbose = true)
{
    auto* buttonClass = FindClass("UnityEngine.UI", "Button");
    auto* unityEventClass = FindClass("UnityEngine.Events", "UnityEvent");
    auto* onClick = Invoke(FindMethod(buttonClass, "get_onClick", 0), buttonComponent, nullptr);
    if (onClick != nullptr)
    {
        Invoke(FindMethod(unityEventClass, "RemoveAllListeners", 0), onClick, nullptr);
        if (verbose)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"Cleared inherited title-button click listeners.");
        }
    }
}

Il2CppObject* GetComponentGameObject(Il2CppObject* component)
{
    return Invoke(
        FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
        component,
        nullptr);
}

void SetGameObjectActive(Il2CppObject* component, bool active)
{
    auto* gameObject = GetComponentGameObject(component);
    if (gameObject != nullptr)
    {
        void* parameters[] = { &active };
        Invoke(FindMethod(FindClass("UnityEngine", "GameObject"), "SetActive", 1), gameObject, parameters);
    }
}

void SetButtonText(Il2CppObject* component, const char* text, bool verbose = true)
{
    auto* gameObject = GetComponentGameObject(component);
    if (gameObject != nullptr)
    {
        SetAllButtonTexts(gameObject, text, verbose);
    }
}

Il2CppObject* CloneMenuButton(
    Il2CppObject* templateButton,
    Il2CppObject* parentTransform,
    const char* text,
    bool quiet = false)
{
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* componentClass = FindClass("UnityEngine", "Component");
    auto* transformClass = FindClass("UnityEngine", "Transform");
    void* instantiateParameters[] = { templateButton };
    auto* clone = Invoke(FindMethod(objectClass, "Instantiate", 1), nullptr, instantiateParameters);
    if (clone == nullptr)
    {
        return nullptr;
    }

    auto* cloneTransform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        GetComponentGameObject(clone),
        nullptr);
    if (cloneTransform != nullptr && parentTransform != nullptr)
    {
        bool keepWorldPosition = false;
        void* parentParameters[] = { parentTransform, &keepWorldPosition };
        Invoke(FindMethod(transformClass, "SetParent", 2), cloneTransform, parentParameters);
    }
    if (!quiet)
    {
        SetButtonText(clone, text);
    }
    ClearButtonListeners(clone, !quiet);
    SetGameObjectActive(clone, false);
    return clone;
}

