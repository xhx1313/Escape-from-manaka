// Native.cpp module: title_menu_ui.inc
// Included inside the shared implementation namespace by Native.cpp.
void RebuildTitleButtonLayout(Il2CppObject* buttonComponent)
{
    auto* gameObject = GetComponentGameObject(buttonComponent);
    auto* transform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        gameObject,
        nullptr);
    auto* parent = Invoke(FindMethod(FindClass("UnityEngine", "Transform"), "get_parent", 0), transform, nullptr);
    auto* parentGameObject = Invoke(
        FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
        parent,
        nullptr);
    auto* parentRect = GetComponentByName(parentGameObject, "RectTransform");
    auto* rebuilderClass = FindClass("UnityEngine.UI", "LayoutRebuilder");
    if (parentRect != nullptr && rebuilderClass != nullptr)
    {
        void* parameters[] = { parentRect };
        Invoke(FindMethod(rebuilderClass, "ForceRebuildLayoutImmediate", 1), nullptr, parameters);
    }
}

Il2CppObject* GetComponentTransform(Il2CppObject* component)
{
    auto* gameObject = GetComponentGameObject(component);
    return gameObject != nullptr
        ? Invoke(
            FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
            gameObject,
            nullptr)
        : nullptr;
}

Il2CppObject* FindImageInTransform(Il2CppObject* transform, int depth);

Il2CppObject* FindImageInGameObject(Il2CppObject* gameObject, int depth)
{
    if (gameObject == nullptr || depth > 3)
    {
        return nullptr;
    }

    auto* image = GetComponentByName(gameObject, "Image");
    if (image != nullptr)
    {
        return image;
    }

    auto* transform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        gameObject,
        nullptr);
    return FindImageInTransform(transform, depth + 1);
}

Il2CppObject* FindImageInTransform(Il2CppObject* transform, int depth)
{
    if (transform == nullptr || depth > 3)
    {
        return nullptr;
    }

    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* childCountValue = Invoke(
        FindMethod(transformClass, "get_childCount", 0),
        transform,
        nullptr);
    if (childCountValue == nullptr || il2cpp_object_unbox == nullptr)
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
        auto* childGameObject = Invoke(
            FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
            child,
            nullptr);
        auto* image = FindImageInGameObject(childGameObject, depth);
        if (image != nullptr)
        {
            return image;
        }
    }
    return nullptr;
}

void SetComponentImageColor(
    Il2CppObject* component,
    float red,
    float green,
    float blue,
    float alpha)
{
    auto* image = FindImageInGameObject(GetComponentGameObject(component), 0);
    if (image == nullptr)
    {
        return;
    }

    float color[] = { red, green, blue, alpha };
    void* parameters[] = { color };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Graphic"), "set_color", 1),
        image,
        parameters);
}

void SetComponentRect(
    Il2CppObject* component,
    float minX,
    float minY,
    float maxX,
    float maxY)
{
    auto* rect = GetComponentByName(GetComponentGameObject(component), "RectTransform");
    if (rect == nullptr)
    {
        return;
    }

    auto* rectClass = FindClass("UnityEngine", "RectTransform");
    float anchorMin[] = { minX, minY };
    float anchorMax[] = { maxX, maxY };
    float offsetMin[] = { 0.0f, 0.0f };
    float offsetMax[] = { 0.0f, 0.0f };
    void* anchorMinParameters[] = { anchorMin };
    void* anchorMaxParameters[] = { anchorMax };
    void* offsetMinParameters[] = { offsetMin };
    void* offsetMaxParameters[] = { offsetMax };
    Invoke(FindMethod(rectClass, "set_anchorMin", 1), rect, anchorMinParameters);
    Invoke(FindMethod(rectClass, "set_anchorMax", 1), rect, anchorMaxParameters);
    Invoke(FindMethod(rectClass, "set_offsetMin", 1), rect, offsetMinParameters);
    Invoke(FindMethod(rectClass, "set_offsetMax", 1), rect, offsetMaxParameters);
}

void SetButtonInteractable(Il2CppObject* component, bool interactable)
{
    auto* selectableClass = FindClass("UnityEngine.UI", "Selectable");
    void* parameters[] = { &interactable };
    Invoke(FindMethod(selectableClass, "set_interactable", 1), component, parameters);
}

void SetButtonVisual(
    Il2CppObject* component,
    const char* text,
    float fontSize,
    int alignment,
    float red,
    float green,
    float blue,
    float alpha)
{
    auto* gameObject = GetComponentGameObject(component);
    auto* tmpTextClass = FindClass("TMPro", "TMP_Text");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (gameObject == nullptr || tmpTextClass == nullptr || gameObjectClass == nullptr)
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
        return;
    }

    auto* textValue = il2cpp_string_new(text);
    float color[] = { red, green, blue, alpha };
    const auto count = il2cpp_array_length(components);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(components) + il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* textComponent = elements[index];
        if (textComponent == nullptr)
        {
            continue;
        }
        void* textParameters[] = { textValue };
        void* sizeParameters[] = { &fontSize };
        void* alignmentParameters[] = { &alignment };
        void* colorParameters[] = { color };
        Invoke(FindMethod(tmpTextClass, "set_text", 1), textComponent, textParameters);
        Invoke(FindMethod(tmpTextClass, "set_fontSize", 1), textComponent, sizeParameters);
        Invoke(FindMethod(tmpTextClass, "set_alignment", 1), textComponent, alignmentParameters);
        Invoke(FindMethod(tmpTextClass, "set_color", 1), textComponent, colorParameters);
    }
}

Il2CppObject* FindCanvasTransform(Il2CppObject* transform)
{
    auto* transformClass = FindClass("UnityEngine", "Transform");
    const auto getGameObject = FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0);
    const auto getParent = FindMethod(transformClass, "get_parent", 0);
    LogInfo(LogCategory_Ui, L"Ui", 
        L"Canvas search start transform=%p getGameObject=%p getParent=%p",
        transform,
        getGameObject,
        getParent);
    Il2CppObject* bestUiParent = nullptr;
    int depth = 0;
    for (; transform != nullptr && depth < 32; ++depth)
    {
        auto* gameObject = Invoke(
            getGameObject,
            transform,
            nullptr);
        LogInfo(LogCategory_Ui, L"Ui", L"UI ancestor scan %d: transform=%p gameObject=%p", depth, transform, gameObject);
        if (gameObject != nullptr)
        {
            const auto name = GetObjectName(gameObject);
            if (depth < 10)
            {
                LogInfo(LogCategory_Ui, L"Ui", L"UI ancestor %d: %s", depth, name.c_str());
            }
            if (name == L"Canvas" ||
                GetComponentByName(gameObject, "Canvas") != nullptr)
            {
                LogInfo(LogCategory_Ui, L"Ui", L"UI Canvas found at ancestor depth %d.", depth);
                return transform;
            }
            if (GetComponentByName(gameObject, "RectTransform") != nullptr)
            {
                bestUiParent = transform;
            }
        }
        transform = Invoke(
            getParent,
            transform,
            nullptr);
    }
    if (bestUiParent != nullptr)
    {
        LogInfo(LogCategory_Ui, L"Ui", L"Falling back to the highest RectTransform ancestor.");
    }
    return bestUiParent;
}

Il2CppObject* CreateUiElement(
    Il2CppObject* templateButton,
    Il2CppObject* parentTransform,
    const char* text,
    float minX,
    float minY,
    float maxX,
    float maxY,
    float red,
    float green,
    float blue,
    float alpha,
    float fontSize,
    int alignment,
    bool interactable)
{
    auto* element = CloneMenuButton(templateButton, parentTransform, "", true);
    if (element == nullptr)
    {
        return nullptr;
    }

    SetComponentRect(element, minX, minY, maxX, maxY);
    SetComponentImageColor(element, red, green, blue, alpha);
    SetButtonVisual(element, text, fontSize, alignment, 0.92f, 0.94f, 0.96f, 1.0f);
    SetButtonInteractable(element, interactable);
    SetGameObjectActive(element, true);
    if (interactable)
    {
        g_uiButtons.push_back(element);
    }
    return element;
}

void CreateSlotGrid(
    Il2CppObject* templateButton,
    Il2CppObject* parentTransform,
    int columns,
    int rows,
    float top,
    float bottom)
{
    const float availableWidth = 0.92f;
    const float availableHeight = top - bottom;
    const float slotWidth = availableWidth / columns;
    const float slotHeight = availableHeight / rows;
    unsigned int index = 0;
    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            char label[16]{};
            std::snprintf(label, sizeof(label), "%02u", ++index);
            const float minX = 0.04f + slotWidth * column + 0.004f;
            const float maxX = 0.04f + slotWidth * (column + 1) - 0.004f;
            const float minY = bottom + slotHeight * row + 0.006f;
            const float maxY = bottom + slotHeight * (row + 1) - 0.006f;
            CreateUiElement(
                templateButton,
                parentTransform,
                label,
                minX,
                minY,
                maxX,
                maxY,
                0.10f,
                0.12f,
                0.13f,
                0.94f,
                15.0f,
                514,
                false);
        }
    }
}

void SwitchEscapeFromManakaTab(int tab)
{
    if (g_equipmentPage == nullptr || g_lobbyPage == nullptr)
    {
        return;
    }

    g_activeUiTab = tab;
    const bool equipment = tab == 0;
    SetGameObjectActive(g_equipmentPage, equipment);
    SetGameObjectActive(g_lobbyPage, !equipment);
    SetComponentImageColor(
        g_equipmentTabButton,
        equipment ? 0.62f : 0.12f,
        equipment ? 0.36f : 0.14f,
        equipment ? 0.18f : 0.15f,
        0.96f);
    SetComponentImageColor(
        g_lobbyTabButton,
        equipment ? 0.12f : 0.62f,
        equipment ? 0.14f : 0.36f,
        equipment ? 0.15f : 0.18f,
        0.96f);
}

void BuildNativeUi()
{
    if (g_uiRoot != nullptr ||
        g_nativeButtonComponent == nullptr ||
        g_nativeButton == nullptr)
    {
        return;
    }

    auto* buttonTransform = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "get_transform", 0),
        g_nativeButton,
        nullptr);
    LogInfo(LogCategory_Ui, L"Ui", L"Resolving native UI parent from title button transform %p.", buttonTransform);
    auto* canvasTransform = FindCanvasTransform(buttonTransform);
    if (canvasTransform == nullptr)
    {
        LogInfo(LogCategory_Ui, L"Ui", L"Unable to find the title Canvas for the native multiplayer UI.");
        return;
    }

    auto* templateButton = g_nativeButtonComponent;
    g_uiRoot = CreateUiElement(
        templateButton,
        canvasTransform,
        "",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        16.0f,
        514,
        false);
    if (g_uiRoot == nullptr)
    {
        return;
    }

    auto* rootTransform = GetComponentTransform(g_uiRoot);
    g_uiBackdrop = CreateUiElement(
        templateButton,
        rootTransform,
        "",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.015f,
        0.018f,
        0.022f,
        0.78f,
        16.0f,
        514,
        false);

    auto* header = CreateUiElement(
        templateButton,
        rootTransform,
        "",
        0.018f,
        0.892f,
        0.982f,
        0.977f,
        0.045f,
        0.050f,
        0.055f,
        0.98f,
        16.0f,
        514,
        false);
    auto* headerTransform = GetComponentTransform(header);
    CreateUiElement(
        templateButton,
        headerTransform,
        "MULTIPLAYER OPERATIONS",
        0.018f,
        0.18f,
        0.30f,
        0.88f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        27.0f,
        257,
        false);

    g_equipmentTabButton = CreateUiElement(
        templateButton,
        headerTransform,
        "装备",
        0.31f,
        0.20f,
        0.405f,
        0.82f,
        0.62f,
        0.36f,
        0.18f,
        0.96f,
        20.0f,
        514,
        true);
    g_lobbyTabButton = CreateUiElement(
        templateButton,
        headerTransform,
        "联机大厅",
        0.412f,
        0.20f,
        0.515f,
        0.82f,
        0.12f,
        0.14f,
        0.15f,
        0.96f,
        20.0f,
        514,
        true);
    g_fleaMarketButton = CreateUiElement(
        templateButton,
        headerTransform,
        "跳蚤市场",
        0.665f,
        0.20f,
        0.775f,
        0.82f,
        0.12f,
        0.14f,
        0.15f,
        0.96f,
        19.0f,
        514,
        true);
    g_startGameButton = CreateUiElement(
        templateButton,
        headerTransform,
        "开始游戏",
        0.782f,
        0.20f,
        0.892f,
        0.82f,
        0.12f,
        0.14f,
        0.15f,
        0.96f,
        19.0f,
        514,
        true);
    g_closeUiButton = CreateUiElement(
        templateButton,
        headerTransform,
        "关闭",
        0.905f,
        0.20f,
        0.982f,
        0.82f,
        0.34f,
        0.11f,
        0.10f,
        0.96f,
        18.0f,
        514,
        true);

    auto* content = CreateUiElement(
        templateButton,
        rootTransform,
        "",
        0.018f,
        0.035f,
        0.982f,
        0.875f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        16.0f,
        514,
        false);
    auto* contentTransform = GetComponentTransform(content);

    g_equipmentPage = CreateUiElement(
        templateButton,
        contentTransform,
        "",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        16.0f,
        514,
        false);
    auto* equipmentTransform = GetComponentTransform(g_equipmentPage);
    auto* leftPanel = CreateUiElement(
        templateButton,
        equipmentTransform,
        "装备 / LOADOUT",
        0.0f,
        0.0f,
        0.315f,
        1.0f,
        0.04f,
        0.05f,
        0.06f,
        0.18f,
        24.0f,
        257,
        false);
    auto* leftTransform = GetComponentTransform(leftPanel);
    CreateSlotGrid(templateButton, leftTransform, 2, 4, 0.31f, 0.03f);
    CreateUiElement(
        templateButton,
        leftTransform,
        "角色视图\n复用 UiPlayerController",
        0.06f,
        0.38f,
        0.94f,
        0.72f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        18.0f,
        514,
        false);

    auto* middlePanel = CreateUiElement(
        templateButton,
        equipmentTransform,
        "背包 / BACKPACK",
        0.32f,
        0.0f,
        0.655f,
        1.0f,
        0.055f,
        0.062f,
        0.068f,
        0.94f,
        24.0f,
        257,
        false);
    CreateSlotGrid(
        templateButton,
        GetComponentTransform(middlePanel),
        5,
        5,
        0.83f,
        0.035f);

    auto* rightPanel = CreateUiElement(
        templateButton,
        equipmentTransform,
        "仓库 / STASH",
        0.66f,
        0.0f,
        1.0f,
        1.0f,
        0.050f,
        0.056f,
        0.062f,
        0.94f,
        24.0f,
        257,
        false);
    CreateSlotGrid(
        templateButton,
        GetComponentTransform(rightPanel),
        6,
        6,
        0.84f,
        0.035f);

    g_lobbyPage = CreateUiElement(
        templateButton,
        contentTransform,
        "",
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        16.0f,
        514,
        false);
    auto* lobbyTransform = GetComponentTransform(g_lobbyPage);
    auto* lobbyPanel = CreateUiElement(
        templateButton,
        lobbyTransform,
        "联机大厅 / ONLINE LOBBY",
        0.15f,
        0.06f,
        0.85f,
        0.95f,
        0.055f,
        0.062f,
        0.068f,
        0.96f,
        28.0f,
        257,
        false);
    auto* lobbyPanelTransform = GetComponentTransform(lobbyPanel);
    g_hostButtonComponent = CreateUiElement(
        templateButton,
        lobbyPanelTransform,
        "创建房间 / HOST ROOM",
        0.18f,
        0.56f,
        0.82f,
        0.70f,
        0.19f,
        0.42f,
        0.26f,
        0.98f,
        23.0f,
        514,
        true);
    g_joinButtonComponent = CreateUiElement(
        templateButton,
        lobbyPanelTransform,
        "加入房间 / JOIN ROOM",
        0.18f,
        0.38f,
        0.82f,
        0.52f,
        0.16f,
        0.28f,
        0.50f,
        0.98f,
        23.0f,
        514,
        true);
    g_lobbyStatusLabel = CreateUiElement(
        templateButton,
        lobbyPanelTransform,
        "等待操作 · TCP/UDP 12121",
        0.08f,
        0.15f,
        0.92f,
        0.31f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        19.0f,
        514,
        false);

    SwitchEscapeFromManakaTab(0);
    auto* transformClass = FindClass("UnityEngine", "Transform");
    Invoke(
        FindMethod(transformClass, "SetAsLastSibling", 0),
        rootTransform,
        nullptr);
    SetGameObjectActive(g_uiRoot, false);
    LogInfo(LogCategory_Ui, L"Ui", L"Native Unity multiplayer inventory UI created.");
}

struct SceneUiElement
{
    Il2CppObject* gameObject = nullptr;
    Il2CppObject* rect = nullptr;
    Il2CppObject* image = nullptr;
    Il2CppObject* rawImage = nullptr;
    Il2CppObject* texture = nullptr;
    Il2CppObject* button = nullptr;
    Il2CppObject* text = nullptr;
};

Il2CppObject* AddSceneComponentToGameObject(
    Il2CppObject* gameObject,
    const char* namespaceName,
    const char* className);

