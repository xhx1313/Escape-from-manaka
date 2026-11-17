// Native.cpp module: title_hooks.inc
// Included inside the shared implementation namespace by Native.cpp.
void UpdateNativeButtonScreenRect()
{
    auto* rectTransformClass = FindClass("UnityEngine", "RectTransform");
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* utilityClass = FindClass("UnityEngine", "RectTransformUtility");
    auto* buttonRect = GetComponentByName(g_nativeButton, "RectTransform");
    if (buttonRect == nullptr || rectTransformClass == nullptr || utilityClass == nullptr)
    {
        return;
    }

    auto* positionObject = Invoke(FindMethod(transformClass, "get_position", 0), buttonRect, nullptr);
    auto* rectangleObject = Invoke(FindMethod(rectTransformClass, "get_rect", 0), buttonRect, nullptr);
    auto* scaleObject = Invoke(FindMethod(transformClass, "get_lossyScale", 0), buttonRect, nullptr);
    if (positionObject == nullptr || rectangleObject == nullptr || scaleObject == nullptr)
    {
        return;
    }

    const auto* position = static_cast<float*>(il2cpp_object_unbox(positionObject));
    void* worldToScreenParameters[] = { nullptr, const_cast<float*>(position) };
    auto* screenPositionObject = Invoke(
        FindMethod(utilityClass, "WorldToScreenPoint", 2),
        nullptr,
        worldToScreenParameters);
    if (screenPositionObject == nullptr)
    {
        return;
    }

    const auto* screenPosition = static_cast<float*>(il2cpp_object_unbox(screenPositionObject));
    const auto* rectangle = static_cast<float*>(il2cpp_object_unbox(rectangleObject));
    const auto* scale = static_cast<float*>(il2cpp_object_unbox(scaleObject));
    const int width = static_cast<int>(std::abs(rectangle[2] * scale[0]));
    const int height = static_cast<int>(std::abs(rectangle[3] * scale[1]));
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_buttonScreenRect = RECT{
        static_cast<LONG>(screenPosition[0] - width * 0.5f),
        static_cast<LONG>(screenHeight - screenPosition[1] - height * 0.5f),
        static_cast<LONG>(screenPosition[0] + width * 0.5f),
        static_cast<LONG>(screenHeight - screenPosition[1] + height * 0.5f)
    };
    g_buttonRectReady = width > 20 && height > 10;
    if (g_buttonRectReady)
    {
        LogInfo(LogCategory_Hook, L"Hook", 
            L"Native button screen rect: %ld,%ld %ldx%ld",
            g_buttonScreenRect.left,
            g_buttonScreenRect.top,
            g_buttonScreenRect.right - g_buttonScreenRect.left,
            g_buttonScreenRect.bottom - g_buttonScreenRect.top);
    }
}

void* MethodPointer(const Il2CppMethod* method)
{
    if (method == nullptr)
    {
        return nullptr;
    }

    // Unity 2022.3 IL2CPP MethodInfo begins with methodPointer.
    return *reinterpret_cast<void**>(const_cast<Il2CppMethod*>(method));
}

void ResetNativeTitleButtonState()
{
    g_nativeButtonInjected = false;
    g_nativeButtonTitleInstance = nullptr;
    g_nativeButton = nullptr;
    g_nativeButtonComponent = nullptr;
    g_nativeButtonRect = nullptr;
    g_buttonScreenRect = {};
    g_buttonRectReady = false;
}

void CreateNativeTitleButton()
{
    if (g_titleInstance == nullptr)
    {
        return;
    }
    if (g_nativeButtonInjected)
    {
        if (g_nativeButtonTitleInstance == g_titleInstance &&
            g_nativeButton != nullptr &&
            ExtendedMode_IsUnityObjectAlive(g_nativeButton))
        {
            return;
        }
        ResetNativeTitleButtonState();
    }
    LogInfo(LogCategory_Hook, L"Hook", L"Creating native title button.");

    auto* templateButton = FindAnyButtonView();
    if (templateButton == nullptr)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"No loaded UnityEngine.UI.Button could be selected for cloning.");
        return;
    }
    LogInfo(LogCategory_Hook, L"Hook", L"Template UI Button component resolved: %p", templateButton);

    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* rectTransformClass = FindClass("UnityEngine", "RectTransform");
    auto* componentClass = FindClass("UnityEngine", "Component");

    const auto instantiate = FindMethod(objectClass, "Instantiate", 1);
    LogInfo(LogCategory_Hook, L"Hook", 
        L"Native UI methods: Instantiate=%p GameObject.get_transform=%p Transform.get_parent=%p",
        instantiate,
        FindMethod(gameObjectClass, "get_transform", 0),
        FindMethod(transformClass, "get_parent", 0));
    if (instantiate == nullptr)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"UnityEngine.Object.Instantiate(Object) was not resolved.");
        return;
    }
    void* instantiateParameters[] = { templateButton };
    auto* cloneComponent = Invoke(instantiate, nullptr, instantiateParameters);
    if (cloneComponent == nullptr)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"UI Button component clone failed.");
        return;
    }
    g_nativeButtonComponent = cloneComponent;
    auto* clone = Invoke(FindMethod(componentClass, "get_gameObject", 0), cloneComponent, nullptr);
    auto* originalGameObject = Invoke(FindMethod(componentClass, "get_gameObject", 0), templateButton, nullptr);
    if (clone == nullptr || originalGameObject == nullptr)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"Unable to resolve cloned or source title button GameObject.");
        return;
    }
    g_nativeButton = clone;
    LogInfo(LogCategory_Hook, L"Hook", L"Cloned title button GameObject: %p component=%p", clone, cloneComponent);

    auto* cloneTransform = Invoke(FindMethod(gameObjectClass, "get_transform", 0), clone, nullptr);
    auto* originalTransform = Invoke(FindMethod(gameObjectClass, "get_transform", 0), originalGameObject, nullptr);
    auto* parent = Invoke(FindMethod(transformClass, "get_parent", 0), originalTransform, nullptr);
    LogInfo(LogCategory_Hook, L"Hook", L"Transforms: clone=%p original=%p parent=%p", cloneTransform, originalTransform, parent);

    if (cloneTransform != nullptr && parent != nullptr)
    {
        bool keepWorldPosition = false;
        void* setParentParameters[] = { parent, &keepWorldPosition };
        Invoke(FindMethod(transformClass, "SetParent", 2), cloneTransform, setParentParameters);

        auto* siblingResult = Invoke(FindMethod(transformClass, "GetSiblingIndex", 0), originalTransform, nullptr);
        int siblingIndex = 0;
        if (siblingResult != nullptr)
        {
            siblingIndex = *static_cast<int*>(il2cpp_object_unbox(siblingResult)) + 1;
        }
        void* siblingParameters[] = { &siblingIndex };
        Invoke(FindMethod(transformClass, "SetSiblingIndex", 1), cloneTransform, siblingParameters);
    }

    bool active = true;
    void* activeParameters[] = { &active };
    Invoke(FindMethod(gameObjectClass, "SetActive", 1), clone, activeParameters);
    LogInfo(LogCategory_Hook, L"Hook", L"Cloned title button activated.");

    if (g_nativeButtonComponent != nullptr)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"Cloned UI Button component ready: %p", g_nativeButtonComponent);
        SetAllButtonTexts(clone, "Escape from Manaka");
        ClearButtonListeners(g_nativeButtonComponent);
    }
    else
    {
        LogInfo(LogCategory_Hook, L"Hook", L"Cloned UI Button component is unavailable.");
    }
    LogInfo(LogCategory_Hook, L"Hook", L"Native button component stage complete.");

    auto* cloneRect = GetComponentByName(clone, "RectTransform");
    auto* parentGameObject = parent != nullptr
        ? Invoke(
            FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
            parent,
            nullptr)
        : nullptr;
    const bool autoLayout =
        GetComponentByName(parentGameObject, "VerticalLayoutGroup") != nullptr ||
        GetComponentByName(parentGameObject, "HorizontalLayoutGroup") != nullptr ||
        GetComponentByName(parentGameObject, "LayoutGroup") != nullptr;

    if (!autoLayout && cloneRect != nullptr)
    {
        auto* originalRect = GetComponentByName(originalGameObject, "RectTransform");
        auto* positionObject = Invoke(FindMethod(rectTransformClass, "get_anchoredPosition", 0), originalRect, nullptr);
        auto* rectObject = Invoke(FindMethod(rectTransformClass, "get_rect", 0), originalRect, nullptr);
        if (positionObject != nullptr && rectObject != nullptr)
        {
            const auto* originalPosition = static_cast<float*>(il2cpp_object_unbox(positionObject));
            const auto* originalRectangle = static_cast<float*>(il2cpp_object_unbox(rectObject));
            float newPosition[2] = {
                originalPosition[0],
                originalPosition[1] - originalRectangle[3] - 10.0f
            };
            void* positionParameters[] = { newPosition };
            g_nativeButtonRect = cloneRect;
            Invoke(FindMethod(rectTransformClass, "set_anchoredPosition", 1), cloneRect, positionParameters);
        }
    }
    LogInfo(LogCategory_Hook, L"Hook", L"Native button layout stage complete.");

    if (parentGameObject != nullptr)
    {
        auto* layoutRebuilderClass = FindClass("UnityEngine.UI", "LayoutRebuilder");
        auto* parentRect = GetComponentByName(parentGameObject, "RectTransform");
        if (layoutRebuilderClass != nullptr && parentRect != nullptr)
        {
            auto* forceRebuild = FindMethod(layoutRebuilderClass, "ForceRebuildLayoutImmediate", 1);
            void* rebuildParameters[] = { parentRect };
            Invoke(forceRebuild, nullptr, rebuildParameters);
        }
    }
    LogInfo(LogCategory_Hook, L"Hook", L"Native button layout rebuild stage complete.");
    const auto buttonName = GetObjectName(g_nativeButton);
    LogInfo(LogCategory_Hook, L"Hook", L"Native title button created: %s", buttonName.c_str());
    g_nativeButtonTitleInstance = g_titleInstance;
    g_nativeButtonInjected = true;
    SetStatus(L"Escape from Manaka entry added to the title menu.");
}

void ProbePlayerScene()
{
    LogInfo(LogCategory_Hook, L"Hook", L"Direct player-scene probing is disabled because the gameplay scene requires a full player lifecycle.");
    return;

    static bool probed = false;
    if (probed)
    {
        return;
    }
    probed = true;

    auto* sceneManager = FindClass(
        "UnityEngine.SceneManagement",
        "SceneManager");
    const auto loadScene = FindMethod(sceneManager, "LoadScene", 2);
    LogInfo(LogCategory_Hook, L"Hook", L"Player-scene probe LoadScene=%p", loadScene);
    if (loadScene == nullptr)
    {
        return;
    }

    auto* sceneName = il2cpp_string_new("Apartment");
    int loadMode = 1;
    void* parameters[] = { sceneName, &loadMode };
    Invoke(loadScene, nullptr, parameters);
    LogInfo(LogCategory_Hook, L"Hook", L"Additive Apartment load requested.");

    auto* uiPlayerClass = FindClass(
        "ExposureUnnoticed2.Object3D.UIPlayer",
        "UiPlayerController");
    auto* instance = Invoke(
        FindMethod(uiPlayerClass, "get_Instance", 0),
        nullptr,
        nullptr);
    LogObjectReference("UiPlayerController after apartment load", instance);
    if (instance == nullptr)
    {
        instance = FindFirstComponentOfClass(uiPlayerClass);
        LogObjectReference("UiPlayerController inactive scan", instance);
    }
    if (instance == nullptr)
    {
        auto* playerPath = il2cpp_string_new("InGameManager/UiPlayer");
        void* findParameters[] = { playerPath };
        auto* playerObject = Invoke(
            FindMethod(FindClass("UnityEngine", "GameObject"), "Find", 1),
            nullptr,
            findParameters);
        LogObjectReference("UiPlayer GameObject.Find", playerObject);
        if (playerObject != nullptr)
        {
            instance = GetSceneComponentByClass(
                playerObject,
                "ExposureUnnoticed2.Object3D.UIPlayer",
                "UiPlayerController");
        }
    }
    if (instance != nullptr)
    {
        LogObjectReference(
            "UiPlayer parent",
            Invoke(FindMethod(uiPlayerClass, "get_parent", 0), instance, nullptr));
        LogObjectReference(
            "UiPlayer contentParent",
            Invoke(FindMethod(uiPlayerClass, "get_contentParent", 0), instance, nullptr));
        LogObjectReference(
            "UiPlayer model host",
            Invoke(FindMethod(uiPlayerClass, "get_uiPlayer", 0), instance, nullptr));
    }
    else
    {
        g_playerSceneProbeFrames = 600;
        LogInfo(LogCategory_Hook, L"Hook", L"UiPlayerController will be rescanned for 600 frames.");
    }
}

void TitleUpdateHook(Il2CppObject* instance, const Il2CppMethod* method)
{
    static unsigned int frameCounter = 0;
    ++frameCounter;
    if (g_extendedWarehouseReentryDelay > 0 &&
        --g_extendedWarehouseReentryDelay == 0)
    {
        g_pendingUiCommand = 1;
        LogInfo(LogCategory_Hook, L"Hook", L"Escape queued the title-page Escape button flow after title stabilization.");
    }

    if (frameCounter == 180)
    {
        wchar_t autoStartValue[8]{};
        const DWORD autoStartLength = GetEnvironmentVariableW(
            L"MANAKA_EFM_AUTOSTART",
            autoStartValue,
            static_cast<DWORD>(
                sizeof(autoStartValue) / sizeof(autoStartValue[0])));
        if (autoStartLength > 0 &&
            autoStartValue[0] != L'0' &&
            autoStartValue[0] != L'\0')
        {
            g_pendingUiCommand = 1;
            LogInfo(LogCategory_Hook, L"Hook", L"Escape from Manaka diagnostic autostart requested.");
        }
    }
    if (frameCounter == 60)
    {
        GameplayActionPursuit_ProbeDependencies();
    }

    const int extendedState = g_extendedModeState.load();
    const bool titleSurfaceActive =
        extendedState == ExtendedMode_Inactive ||
        extendedState == ExtendedMode_Exiting;
    if (instance != nullptr && titleSurfaceActive)
    {
        const bool titleInstanceChanged =
            g_titleInstance != nullptr &&
            g_titleInstance != instance;
        const bool buttonReferenceLost =
            g_nativeButtonInjected &&
            (g_nativeButton == nullptr ||
             !ExtendedMode_IsUnityObjectAlive(g_nativeButton));
        if (titleInstanceChanged || buttonReferenceLost)
        {
            ResetNativeTitleButtonState();
            LogInfo(LogCategory_Hook, L"Hook", L"New title scene detected; recreating the Escape from Manaka entry.");
        }
        g_titleInstance = instance;
    }
    ExtendedMode_TickMainThread(nullptr);

    if (!g_nativeButtonInjected &&
        instance != nullptr &&
        titleSurfaceActive)
    {
        g_titleInstance = instance;
        ProbeCharacterSystems();
        CreateNativeTitleButton();
    }

    if (g_originalTitleUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalTitleUpdate)(instance, method);
    }

    UpdateUi_TickMainThread();
    if (!ExtendedMode_IsRunning() &&
        UpdateUi_ConsumeModEntryPermit())
    {
        ExtendedMode_Begin();
    }

    const int pendingCommand = g_pendingUiCommand.exchange(0);
    if (pendingCommand == 1)
    {
        UpdateUi_RequestModEntry();
    }
    else if (pendingCommand == 2)
    {
        if (ExtendedMode_IsRunning())
        {
            g_extendedExitRequested = true;
        }
    }
    else if (pendingCommand == 30)
    {
        ProbePlayerScene();
    }

    if (g_efmToggleActionListRequested.exchange(false) &&
        g_efmDebugActionPanel != nullptr)
    {
        const bool open = !g_efmActionListOpen.exchange(true);
        if (!open)
        {
            g_efmActionListOpen = false;
            MoaningAudio_Stop();
        }
        SetGameObjectActive(g_efmDebugActionPanel, open);
        RefreshEscapeFromManakaActionPage();
        LogInfo(LogCategory_Hook, L"Hook", 
            open
                ? L"Moaning3 audio debug panel opened."
                : L"Moaning3 audio debug panel closed.");
    }

    EscapeFromManaka_TickPreparing();
    EscapeFromManaka_UpdateModelDrag();
    EscapeFromManaka_UpdateModelAction();
    MoaningAudio_TickDebugPanel();

    int playerProbeFrames = g_playerSceneProbeFrames.load();
    if (playerProbeFrames > 0)
    {
        g_playerSceneProbeFrames = playerProbeFrames - 1;
        if (playerProbeFrames % 30 == 0)
        {
            auto* uiPlayerClass = FindClass(
                "ExposureUnnoticed2.Object3D.UIPlayer",
                "UiPlayerController");
            auto* delayedInstance = Invoke(
                FindMethod(uiPlayerClass, "get_Instance", 0),
                nullptr,
                nullptr);
            LogObjectReference("UiPlayerController delayed scan", delayedInstance);
            if (delayedInstance != nullptr)
            {
                auto* parent = Invoke(
                    FindMethod(uiPlayerClass, "get_parent", 0),
                    delayedInstance,
                    nullptr);
                auto* contentParent = Invoke(
                    FindMethod(uiPlayerClass, "get_contentParent", 0),
                    delayedInstance,
                    nullptr);
                auto* modelHost = Invoke(
                    FindMethod(uiPlayerClass, "get_uiPlayer", 0),
                    delayedInstance,
                    nullptr);
                LogObjectReference("UiPlayer delayed parent", parent);
                LogObjectReference("UiPlayer delayed contentParent", contentParent);
                LogObjectReference("UiPlayer delayed model host", modelHost);
                auto* playerControllerClass = FindClass(
                    "ExposureUnnoticed2.Object3D.Player.Scripts",
                    "PlayerController");
                void* playerControllerValue = nullptr;
                if (playerControllerClass != nullptr &&
                    il2cpp_class_get_field_from_name != nullptr &&
                    il2cpp_field_static_get_value != nullptr)
                {
                    auto* playerInstanceField = il2cpp_class_get_field_from_name(
                        playerControllerClass,
                        "Instance");
                    if (playerInstanceField != nullptr)
                    {
                        il2cpp_field_static_get_value(
                            playerInstanceField,
                            &playerControllerValue);
                    }
                }
                auto* playerController = static_cast<Il2CppObject*>(playerControllerValue);
                LogObjectReference("PlayerController.Instance", playerController);
                if (playerController != nullptr &&
                    il2cpp_class_get_field_from_name != nullptr &&
                    il2cpp_field_get_value != nullptr)
                {
                    auto* avatarReferenceField = il2cpp_class_get_field_from_name(
                        playerControllerClass,
                        "PlayerAvatarObjectReferencer");
                    void* avatarReferenceValue = nullptr;
                    if (avatarReferenceField != nullptr)
                    {
                        il2cpp_field_get_value(
                            avatarReferenceField,
                            playerController,
                            &avatarReferenceValue);
                    }
                    auto* avatarReference = static_cast<Il2CppObject*>(avatarReferenceValue);
                    LogObjectReference(
                        "PlayerAvatarObjectReferencer",
                        avatarReference);
                    LogObjectReference(
                        "PlayerAvatarObjectReferencer GameObject",
                        avatarReference != nullptr
                            ? GetComponentGameObject(avatarReference)
                            : nullptr);
                }
                auto* npcAvatarClass = FindClass(
                    "ExposureUnnoticed2.Object3D.NPC.Script",
                    "NpcAvatarComponent");
                auto* npcAvatar = FindFirstComponentOfClass(npcAvatarClass);
                auto* npcGameObject = npcAvatar != nullptr
                    ? GetComponentGameObject(npcAvatar)
                    : nullptr;
                LogObjectReference("NpcAvatarComponent candidate", npcAvatar);
                LogObjectReference("NpcAvatarComponent GameObject", npcGameObject);
                auto* npcAnimator = npcGameObject != nullptr
                    ? GetSceneComponentByClass(
                        npcGameObject,
                        "UnityEngine",
                        "Animator")
                    : nullptr;
                LogObjectReference("NpcAvatarComponent Animator", npcAnimator);
                auto* npcManagerClass = FindClass(
                    "ExposureUnnoticed2.Object3D.NPC.Script",
                    "NpcManager");
                auto* npcManager = Invoke(
                    FindMethod(npcManagerClass, "get_Instance", 0),
                    nullptr,
                    nullptr);
                LogObjectReference("NpcManager.Instance", npcManager);
                static bool fixedNpcProbeAttempted = false;
                if (npcManager != nullptr && !fixedNpcProbeAttempted)
                {
                    fixedNpcProbeAttempted = true;
                    int fixedType = 2;
                    void* fixedNpcParameters[] = { &fixedType };
                    auto* fixedNpc = Invoke(
                        FindMethod(npcManagerClass, "CreateFixedNpc", 1),
                        npcManager,
                        fixedNpcParameters);
                    LogObjectReference("NpcManager.CreateFixedNpc(Talk)", fixedNpc);
                    if (fixedNpc != nullptr)
                    {
                        auto* fixedNpcGameObject = GetComponentGameObject(fixedNpc);
                        LogObjectReference(
                            "CreateFixedNpc GameObject",
                            fixedNpcGameObject);
                        auto* fixedNpcAnimator = fixedNpcGameObject != nullptr
                            ? GetSceneComponentByClass(
                                fixedNpcGameObject,
                                "UnityEngine",
                                "Animator")
                            : nullptr;
                        LogObjectReference(
                            "CreateFixedNpc Animator",
                            fixedNpcAnimator);
                    }
                }
                auto* femaleModel = FindGameObjectByNameToken(L"NpcPreviousFemale");
                LogObjectReference("NpcPreviousFemale scene object", femaleModel);
                if (femaleModel == nullptr)
                {
                    femaleModel = FindGameObjectByNameToken(L"NpcFemale_Assemble");
                    LogObjectReference("NpcFemale_Assemble scene object", femaleModel);
                }
                if (femaleModel != nullptr)
                {
                    g_playerSceneProbeFrames = 0;
                }
            }
        }
    }

    if (g_lobbyUiOpen && frameCounter % 10 == 0)
    {
        UpdateEscapeFromManakaUiText();
        if (frameCounter % 30 == 0)
        {
            auto* cursorClass = FindClass("UnityEngine", "Cursor");
            int cursorLockState = 0;
            bool cursorVisible = true;
            void* lockParameters[] = { &cursorLockState };
            void* visibleParameters[] = { &cursorVisible };
            Invoke(
                FindMethod(cursorClass, "set_lockState", 1),
                nullptr,
                lockParameters);
            Invoke(
                FindMethod(cursorClass, "set_visible", 1),
                nullptr,
                visibleParameters);
        }
    }
    else if (g_nativeButtonComponent != nullptr && frameCounter % 30 == 0)
    {
        SetButtonText(g_nativeButtonComponent, "Escape from Manaka", false);
    }
}

void ApplicationQuitHook(const Il2CppMethod* method)
{
    Runtime_RequestShutdown(L"UnityEngine.Application.Quit");
    const unsigned long long started = GetTickCount64();
    const unsigned long long deadline = started + 3000;
    while (!g_runtimeThreadStopped.load() &&
           GetTickCount64() < deadline)
    {
        Sleep(1);
    }
    LogInfo(LogCategory_Hook, L"Hook", 
        L"Unity Application.Quit resumed: runtimeStopped=%d waited=%llums.",
        g_runtimeThreadStopped.load() ? 1 : 0,
        GetTickCount64() - started);
    if (g_originalApplicationQuit != nullptr)
    {
        using OriginalFn = void (*)(const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalApplicationQuit)(method);
    }
}

std::wstring GetButtonText(Il2CppObject* buttonComponent)
{
    if (buttonComponent == nullptr)
    {
        return {};
    }
    auto* gameObject = GetComponentGameObject(buttonComponent);
    auto* text = FindFirstTmpTextComponent(gameObject);
    if (text == nullptr)
    {
        return {};
    }
    auto* value = Invoke(
        FindMethod(FindClass("TMPro", "TMP_Text"), "get_text", 0),
        text,
        nullptr);
    return value != nullptr
        ? ReadIl2CppString(static_cast<Il2CppString*>(value))
        : std::wstring{};
}

bool TitleHooks_TryAutoConfirmReturnToTitle()
{
    if (!g_extendedAwaitingReturnConfirmation.load() ||
        g_originalButtonOnClick == nullptr ||
        il2cpp_type_get_object == nullptr)
    {
        return false;
    }
    auto* buttonClass = FindClass("UnityEngine.UI", "Button");
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* unityEventClass = FindClass(
        "UnityEngine.Events",
        "UnityEvent");
    if (buttonClass == nullptr ||
        objectClass == nullptr ||
        gameObjectClass == nullptr ||
        unityEventClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return false;
    }
    il2cpp_runtime_class_init(buttonClass);
    auto* buttonType = il2cpp_type_get_object(
        il2cpp_class_get_type(buttonClass));
    bool includeInactive = true;
    void* parameters[] = { buttonType, &includeInactive };
    auto* objects = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (objects == nullptr)
    {
        return false;
    }
    auto* getActiveInHierarchy = FindMethod(
        gameObjectClass,
        "get_activeInHierarchy",
        0);
    auto* getOnClick = FindMethod(buttonClass, "get_onClick", 0);
    auto* invokeClick = FindMethod(unityEventClass, "Invoke", 0);
    if (getActiveInHierarchy == nullptr ||
        getOnClick == nullptr ||
        invokeClick == nullptr)
    {
        return false;
    }
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(objects) +
        il2cpp_array_object_header_size());
    const uint32_t count = il2cpp_array_length(objects);
    std::vector<std::wstring> activeLabels;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* button = elements[index];
        if (button == nullptr ||
            !ExtendedMode_IsUnityObjectAlive(button))
        {
            continue;
        }
        auto* gameObject = GetComponentGameObject(button);
        auto* activeValue = gameObject != nullptr
            ? Invoke(getActiveInHierarchy, gameObject, nullptr)
            : nullptr;
        if (activeValue == nullptr ||
            !*static_cast<bool*>(il2cpp_object_unbox(activeValue)))
        {
            continue;
        }
        const auto label = GetButtonText(button);
        if (label.empty())
        {
            continue;
        }
        activeLabels.push_back(label);
        std::wstring normalized;
        normalized.reserve(label.size());
        for (const wchar_t character : label)
        {
            if (character != L' ' &&
                character != L'\t' &&
                character != L'\r' &&
                character != L'\n' &&
                character != L'　')
            {
                normalized.push_back(character);
            }
        }
        const bool isYes =
            normalized == L"是" ||
            normalized == L"はい" ||
            _wcsicmp(normalized.c_str(), L"yes") == 0 ||
            _wcsicmp(normalized.c_str(), L"ok") == 0;
        if (!isYes)
        {
            continue;
        }
        auto* onClick = Invoke(getOnClick, button, nullptr);
        if (onClick == nullptr)
        {
            continue;
        }
        Invoke(invokeClick, onClick, nullptr);
        LogInfo(LogCategory_Hook, L"Hook", 
            L"Escape auto-confirmed return to title using button label '%s'.",
            label.c_str());
        return true;
    }
    static unsigned int confirmationScanFrame = 0;
    if (++confirmationScanFrame == 60)
    {
        LogInfo(LogCategory_Hook, L"Hook", 
            L"Escape return confirmation was not found after 60 frames; %zu active button label(s).",
            activeLabels.size());
    }
    return false;
}

void ButtonOnClickHook(
    Il2CppObject* instance,
    Il2CppObject* pointerEventData,
    const Il2CppMethod* method)
{
    if (ExtendedMode_HandleButtonClick(instance))
    {
        return;
    }
    // The original pause menu is one of the exceptions to gameplay UI
    // blocking. Its buttons must continue through to the game's own callback;
    // otherwise Esc can open the menu while every menu action is swallowed.
    const bool originalSystemMenuActive =
        ExtendedMode_IsOriginalSystemMenuActive();
    bool allowOriginalMenuButton = false;
    if (ExtendedMode_IsRunning() &&
        (originalSystemMenuActive ||
         g_extendedOriginalSystemMenuOpen.load() ||
         g_extendedModeState.load() == ExtendedMode_Exiting))
    {
        const auto label = GetButtonText(instance);
        const bool returnsToTitle =
            label.find(L"返回标题") != std::wstring::npos ||
            label.find(L"标题") != std::wstring::npos ||
            label.find(L"紧急脱出") != std::wstring::npos ||
            label.find(L"緊急脱出") != std::wstring::npos ||
            label.find(L"タイトル") != std::wstring::npos ||
            label.find(L"Title") != std::wstring::npos ||
            label.find(L"Emergency Escape") != std::wstring::npos;
        if (returnsToTitle)
        {
            if (g_extendedModeState.load() == ExtendedMode_Active)
            {
                g_extendedReturnToWarehouseAfterExit = false;
                g_extendedWarehouseReturnInProgress = false;
                g_extendedWarehouseCoverHideDelayFrames = 0;
                g_extendedPendingDeploymentStageType = -1;
                g_extendedPendingDeploymentDelayFrames = 0;
                g_extendedSuppressPlayerUpdate = true;
                g_extendedExitRequested = true;
                LogInfo(LogCategory_Hook, L"Hook", 
                    L"Warehouse return-to-title requested a real Title scene using label '%s'.",
                    label.c_str());
                return;
            }
            ExtendedMode_RequestReturnToWarehouse(true);
            LogInfo(LogCategory_Hook, L"Hook", 
                L"Escape replaced system-menu return-to-title with warehouse extraction using label '%s'.",
                label.c_str());
            return;
        }
        if (label.find(L"返回游戏") != std::wstring::npos ||
            label.find(L"ゲームに戻る") != std::wstring::npos ||
            label.find(L"Resume") != std::wstring::npos ||
            label.find(L"Continue") != std::wstring::npos)
        {
            auto* systemMenuClass = FindClass(
                "ExposureUnnoticed2.ObjectUI.SystemMenu",
                "SystemMenuView");
            auto* systemMenu = systemMenuClass != nullptr
                ? Invoke(
                    FindMethod(systemMenuClass, "get_Instance", 0),
                    nullptr,
                    nullptr)
                : nullptr;
            auto* resume = FindMethod(
                systemMenuClass,
                "OnClickResume",
                0);
            if (systemMenu != nullptr && resume != nullptr)
            {
                ExtendedMode_RecoverFromInterruptedExit();
                Invoke(resume, systemMenu, nullptr);
                g_extendedOriginalSystemMenuOpen = false;
                ExtendedMode_SetCustomUiBehindSystemMenu(false);
                g_extendedOriginalSystemMenuVisible = false;
                ExtendedMode_ForceCursor();
                LogInfo(LogCategory_Hook, L"Hook", L"Escape from Manaka resumed gameplay from the inventory system menu.");
                return;
            }
        }
        // Confirmation dialogs are children of the pause UI rather than the
        // active SystemMenuView itself. Their Yes/No buttons, plus every other
        // shipped pause-menu action, must go through the original callback.
        allowOriginalMenuButton = true;
    }
    if (allowOriginalMenuButton &&
        g_originalButtonOnClick != nullptr)
    {
        using OriginalFn =
            void (*)(Il2CppObject*, Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalButtonOnClick)(
            instance,
            pointerEventData,
            method);
        return;
    }
    if (ExtendedMode_ShouldBlockUiActions() && !originalSystemMenuActive)
    {
        return;
    }
    if (InventoryUi_HandleButtonClick(instance))
    {
        return;
    }
    if (instance == g_nativeButtonComponent)
    {
        static auto lastNativeClick = std::chrono::steady_clock::time_point{};
        const auto now = std::chrono::steady_clock::now();
        if (lastNativeClick.time_since_epoch().count() != 0 &&
            now - lastNativeClick < std::chrono::milliseconds(500))
        {
            return;
        }
        lastNativeClick = now;
        LogInfo(LogCategory_Hook, L"Hook", L"Escape from Manaka button click received.");
        UpdateUi_RequestModEntry();
        return;
    }
    if (instance == g_extendedPrevPageButton)
    {
        static auto lastExtendedPageClick = std::chrono::steady_clock::time_point{};
        const auto now = std::chrono::steady_clock::now();
        if (lastExtendedPageClick.time_since_epoch().count() != 0 &&
            now - lastExtendedPageClick < std::chrono::milliseconds(500))
        {
            return;
        }
        lastExtendedPageClick = now;
        MoaningAudio_ChangePage(-1);
        ExtendedMode_RefreshActionPage();
        return;
    }
    if (instance == g_extendedNextPageButton)
    {
        static auto lastExtendedPageClick = std::chrono::steady_clock::time_point{};
        const auto now = std::chrono::steady_clock::now();
        if (lastExtendedPageClick.time_since_epoch().count() != 0 &&
            now - lastExtendedPageClick < std::chrono::milliseconds(500))
        {
            return;
        }
        lastExtendedPageClick = now;
        MoaningAudio_ChangePage(1);
        ExtendedMode_RefreshActionPage();
        return;
    }
    for (size_t index = 0; index < g_extendedActionButtons.size(); ++index)
    {
        if (instance == g_extendedActionButtons[index])
        {
            static Il2CppObject* lastExtendedActionButton = nullptr;
            static auto lastExtendedActionClick = std::chrono::steady_clock::time_point{};
            const auto now = std::chrono::steady_clock::now();
            if (instance == lastExtendedActionButton &&
                lastExtendedActionClick.time_since_epoch().count() != 0 &&
                now - lastExtendedActionClick < std::chrono::milliseconds(500))
            {
                return;
            }
            lastExtendedActionButton = instance;
            lastExtendedActionClick = now;
            MoaningAudio_PlayDebugSlot(static_cast<int>(index));
            return;
        }
    }
    if (instance == g_closeUiButton)
    {
        SetEscapeFromManakaOpen(false);
        return;
    }
    if (instance == g_efmPrevActionPageButton)
    {
        static auto lastPageClick = std::chrono::steady_clock::time_point{};
        const auto now = std::chrono::steady_clock::now();
        if (lastPageClick.time_since_epoch().count() != 0 &&
            now - lastPageClick < std::chrono::milliseconds(600))
        {
            return;
        }
        lastPageClick = now;
        MoaningAudio_ChangePage(-1);
        RefreshEscapeFromManakaActionPage();
        return;
    }
    if (instance == g_efmNextActionPageButton)
    {
        static auto lastPageClick = std::chrono::steady_clock::time_point{};
        const auto now = std::chrono::steady_clock::now();
        if (lastPageClick.time_since_epoch().count() != 0 &&
            now - lastPageClick < std::chrono::milliseconds(600))
        {
            return;
        }
        lastPageClick = now;
        MoaningAudio_ChangePage(1);
        RefreshEscapeFromManakaActionPage();
        return;
    }
    for (size_t index = 0; index < g_efmActionButtons.size(); ++index)
    {
        if (instance == g_efmActionButtons[index])
        {
            static Il2CppObject* lastActionButton = nullptr;
            static auto lastActionClick = std::chrono::steady_clock::time_point{};
            const auto now = std::chrono::steady_clock::now();
            if (instance == lastActionButton &&
                lastActionClick.time_since_epoch().count() != 0 &&
                now - lastActionClick < std::chrono::milliseconds(600))
            {
                return;
            }
            lastActionButton = instance;
            lastActionClick = now;
            MoaningAudio_PlayDebugSlot(static_cast<int>(index));
            return;
        }
    }
    if (instance == g_equipmentTabButton)
    {
        SwitchEscapeFromManakaTab(0);
        return;
    }
    if (instance == g_lobbyTabButton)
    {
        SwitchEscapeFromManakaTab(1);
        return;
    }
    if (instance == g_fleaMarketButton)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"Flea market button is reserved for a later build.");
        return;
    }
    if (instance == g_startGameButton)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"Start game button is reserved for a later build.");
        return;
    }
    if (instance == g_hostButtonComponent && g_lobbyUiOpen)
    {
        StartHost();
        g_networkMode = 1;
        SetButtonText(g_hostButtonComponent, "HOST: WAITING");
        return;
    }
    if (instance == g_joinButtonComponent && g_lobbyUiOpen)
    {
        SetButtonText(g_joinButtonComponent, "SEARCHING HOST...");
        g_networkMode = 2;
        StartClientDiscovery();
        return;
    }

    if (g_originalButtonOnClick != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalButtonOnClick)(instance, pointerEventData, method);
    }
}

void InstallNativeHooks()
{
    if (g_nativeHooksInstalled)
    {
        return;
    }

    auto* buttonViewClass = FindClass("UnityEngine.UI", "Button");
    const auto titleUpdate = FindMethod(g_titleClass, "Update", 0);
    const auto buttonOnClick = FindMethod(buttonViewClass, "OnPointerClick", 1);
    auto* playerControllerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerController");
    const auto playerUpdate = FindMethod(
        playerControllerClass,
        "OnUpdate",
        0);
    g_playerControllerUpdateMethod = playerUpdate;
    const auto playerOnFoundStrangerEvent = FindMethod(
        playerControllerClass,
        "OnFoundStrangerEvent",
        1);
    auto* npcManagerClass = FindClass(
        "ExposureUnnoticed2.Object3D.NPC.Script",
        "NpcManager");
    const auto npcManagerUpdate = FindMethod(
        npcManagerClass,
        "OnUpdate",
        0);
    auto* playerActionClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAction");
    const auto playerActionUpdate = FindMethod(
        playerActionClass,
        "OnUpdate",
        1);
    const auto playerActionChangeAction = FindMethod(
        playerActionClass,
        "ChangeAction",
        1);
    const auto playerActionOnFrameEnd = FindMethod(
        playerActionClass,
        "OnFrameEnd",
        0);
    auto* playerAnimationManagerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationManager");
    const auto playerAnimationManagerUpdate = FindMethod(
        playerAnimationManagerClass,
        "OnUpdate",
        1);
    const auto playerAnimationTransAction = FindMethod(
        playerAnimationManagerClass,
        "TransAction",
        1);
    const auto playerAnimationSetMoveSpeed = FindMethod(
        playerAnimationManagerClass,
        "SetMoveSpeed",
        1);
    const auto playerAnimationSetGamanBaibu = FindMethod(
        playerAnimationManagerClass,
        "SetGamanBaibu",
        1);
    const auto playerAnimationSetEcstasy = FindMethod(
        playerAnimationManagerClass,
        "SetEcstasy",
        1);
    auto* playerAdultGoodsControllerClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAdultGoodsController");
    const auto playerAdultGoodsControllerUpdate = FindMethod(
        playerAdultGoodsControllerClass,
        "OnUpdate",
        0);
    auto* playerClothesActionClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerClothesAction");
    const auto playerClothesActionUpdate = FindMethod(
        playerClothesActionClass,
        "OnUpdate",
        1);
    auto* playerClothesClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerClothes");
    const auto playerClothesGetExposedBodyParts = FindMethod(
        playerClothesClass,
        "GetExposedBodyParts",
        0);
    auto* playerHoldInHandClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerHoldInHand");
    const auto playerHoldInHandHold = FindMethod(
        playerHoldInHandClass,
        "Hold",
        2);
    auto* playerAnimationReceiverClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationReceiver");
    const auto playerAnimationDropClothes = FindMethod(
        playerAnimationReceiverClass,
        "OnAnimDropClothes",
        0);
    const auto playerAnimationEndPickingCoat = FindMethod(
        playerAnimationReceiverClass,
        "OnAnimEndPickingCoat",
        0);
    const auto playerAnimationCommonStartAnim = FindMethod(
        playerAnimationReceiverClass,
        "CommonStartAnim",
        0);
    const auto playerAnimationCommonOnEndAnim = FindMethod(
        playerAnimationReceiverClass,
        "CommonOnEndAnim",
        0);
    const auto playerAnimationChangeVibrator = FindMethod(
        playerAnimationReceiverClass,
        "ChangeVibrator",
        0);
    const auto playerAnimationEndEcstasy = FindMethod(
        playerAnimationReceiverClass,
        "OnAnimEndEcstasy",
        0);
    auto* inputManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "InputManager");
    const auto inputManagerUpdate = FindMethod(
        inputManagerClass,
        "Update",
        0);
    const auto inputManagerUpdateProcess = FindMethod(
        inputManagerClass,
        "UpdateProcess",
        0);
    const auto inputManagerIsDown = FindMethod(
        inputManagerClass,
        "IsDown",
        1);
    const auto inputManagerIsDownFrame = FindMethod(
        inputManagerClass,
        "IsDownFrame",
        1);
    const auto inputManagerIsUpFrame = FindMethod(
        inputManagerClass,
        "IsUpFrame",
        1);
    const auto inputManagerGetInputAction = FindMethod(
        inputManagerClass,
        "GetInputAction",
        1);
    auto* inputResolverClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "InputResolver");
    const auto inputResolverResolve = FindMethod(
        inputResolverClass,
        "Resolve",
        0);
    auto* unityInputClass = FindClass("UnityEngine", "Input");
    auto* unityApplicationClass = FindClass(
        "UnityEngine",
        "Application");
    auto* unityCursorClass = FindClass("UnityEngine", "Cursor");
    const auto unityApplicationQuit = FindMethod(
        unityApplicationClass,
        "Quit",
        0);
    const auto unityInputGetKey = FindMethod(
        unityInputClass,
        "GetKey",
        1);
    const auto unityInputGetKeyDown = FindMethod(
        unityInputClass,
        "GetKeyDown",
        1);
    const auto unityInputGetKeyUp = FindMethod(
        unityInputClass,
        "GetKeyUp",
        1);
    const auto unityCursorSetLockState = FindMethod(
        unityCursorClass,
        "set_lockState",
        1);
    const auto unityCursorSetVisible = FindMethod(
        unityCursorClass,
        "set_visible",
        1);
    auto* originalInventoryPanelClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.InventoryPanel",
        "InventoryPanelView");
    const auto originalInventoryPanelOpen = FindMethod(
        originalInventoryPanelClass,
        "Open",
        2);
    auto* inGameManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "InGameManager");
    const auto inGameManagerUpdate = FindMethod(
        inGameManagerClass,
        "OnUpdate",
        0);
    auto* eventMethodManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "EventMethodManager");
    const auto eventMethodManagerUpdate = FindMethod(
        eventMethodManagerClass,
        "Update",
        0);
    auto* baseCameraClass = FindClass(
        "ExposureUnnoticed2.Object3D.Camera",
        "BaseCameraController");
    const auto baseCameraLateUpdate = FindMethod(
        baseCameraClass,
        "OnLateUpdate",
        0);
    auto* tpCameraClass = FindClass(
        "ExposureUnnoticed2.Object3D.Camera",
        "TPCameraController");
    const auto tpCameraLateUpdate = FindMethod(
        tpCameraClass,
        "OnLateUpdate",
        0);
    auto* inventoryCameraClass = FindClass(
        "ExposureUnnoticed2.Object3D.Camera",
        "InventoryCameraController");
    const auto inventoryCameraLateUpdate = FindMethod(
        inventoryCameraClass,
        "OnLateUpdate",
        0);
    auto* fpCameraClass = FindClass(
        "ExposureUnnoticed2.Object3D.Camera",
        "FPCameraController");
    const auto fpCameraLateUpdate = FindMethod(
        fpCameraClass,
        "OnLateUpdate",
        0);
    auto* saveManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "SaveManager");
    const auto saveManagerSave = FindMethod(
        saveManagerClass,
        "Save",
        3);
    g_saveManagerSaveMethod = saveManagerSave;
    auto* interactManagerClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "InteractManager");
    const auto interactManagerRegister = FindMethod(
        interactManagerClass,
        "Register",
        2);
    const auto interactManagerUnregister = FindMethod(
        interactManagerClass,
        "Unregister",
        2);
    auto* systemMenuClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.SystemMenu",
        "SystemMenuView");
    const auto systemMenuOpen = FindMethod(
        systemMenuClass,
        "Open",
        0);
    const auto systemMenuResume = FindMethod(
        systemMenuClass,
        "OnClickResume",
        0);
    const auto systemMenuSaveAndGoToTitle = FindMethod(
        systemMenuClass,
        "OnClickSaveAndGoToTitle",
        0);
    auto* tutorialPanelClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.TutorialPanel",
        "TutorialPanelView");
    const auto tutorialPanelOpen = FindMethod(
        tutorialPanelClass,
        "Open",
        1);
    auto* missionFailedPanelClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.MissionFailedPanel",
        "MissionFailedPanelView");
    const auto missionFailedLoad = FindMethod(
        missionFailedPanelClass,
        "OnClickLoad",
        0);
    const auto missionFailedGoToTitle = FindMethod(
        missionFailedPanelClass,
        "OnClickGoToTitle",
        0);
    const auto missionFailedOpen = FindMethod(
        missionFailedPanelClass,
        "Open",
        0);
    auto* titleUpdatePointer = MethodPointer(titleUpdate);
    auto* buttonOnClickPointer = MethodPointer(buttonOnClick);
    auto* playerUpdatePointer = MethodPointer(playerUpdate);
    auto* playerOnFoundStrangerEventPointer = MethodPointer(
        playerOnFoundStrangerEvent);
    auto* playerActionUpdatePointer = MethodPointer(playerActionUpdate);
    auto* playerActionChangeActionPointer =
        MethodPointer(playerActionChangeAction);
    auto* playerActionOnFrameEndPointer =
        MethodPointer(playerActionOnFrameEnd);
    auto* npcManagerUpdatePointer = MethodPointer(
        npcManagerUpdate);
    auto* playerAnimationManagerUpdatePointer = MethodPointer(
        playerAnimationManagerUpdate);
    auto* playerAnimationTransActionPointer =
        MethodPointer(playerAnimationTransAction);
    auto* playerAnimationSetMoveSpeedPointer =
        MethodPointer(playerAnimationSetMoveSpeed);
    auto* playerAnimationSetGamanBaibuPointer =
        MethodPointer(playerAnimationSetGamanBaibu);
    auto* playerAnimationSetEcstasyPointer =
        MethodPointer(playerAnimationSetEcstasy);
    auto* playerAdultGoodsControllerUpdatePointer =
        MethodPointer(playerAdultGoodsControllerUpdate);
    auto* playerClothesActionUpdatePointer = MethodPointer(
        playerClothesActionUpdate);
    auto* playerClothesGetExposedBodyPartsPointer = MethodPointer(
        playerClothesGetExposedBodyParts);
    auto* playerHoldInHandHoldPointer = MethodPointer(
        playerHoldInHandHold);
    auto* playerAnimationDropClothesPointer = MethodPointer(
        playerAnimationDropClothes);
    auto* playerAnimationEndPickingCoatPointer = MethodPointer(
        playerAnimationEndPickingCoat);
    auto* playerAnimationCommonStartAnimPointer =
        MethodPointer(playerAnimationCommonStartAnim);
    auto* playerAnimationCommonOnEndAnimPointer =
        MethodPointer(playerAnimationCommonOnEndAnim);
    auto* playerAnimationChangeVibratorPointer =
        MethodPointer(playerAnimationChangeVibrator);
    auto* playerAnimationEndEcstasyPointer =
        MethodPointer(playerAnimationEndEcstasy);
    auto* inputManagerUpdatePointer = MethodPointer(inputManagerUpdate);
    auto* inputManagerUpdateProcessPointer = MethodPointer(
        inputManagerUpdateProcess);
    auto* inputManagerIsDownPointer = MethodPointer(
        inputManagerIsDown);
    auto* inputManagerIsDownFramePointer = MethodPointer(
        inputManagerIsDownFrame);
    auto* inputManagerIsUpFramePointer = MethodPointer(
        inputManagerIsUpFrame);
    auto* inputManagerGetInputActionPointer = MethodPointer(
        inputManagerGetInputAction);
    auto* inputResolverResolvePointer = MethodPointer(
        inputResolverResolve);
    auto* unityInputGetKeyPointer = MethodPointer(
        unityInputGetKey);
    auto* unityInputGetKeyDownPointer = MethodPointer(
        unityInputGetKeyDown);
    auto* unityInputGetKeyUpPointer = MethodPointer(
        unityInputGetKeyUp);
    auto* unityCursorSetLockStatePointer = MethodPointer(
        unityCursorSetLockState);
    auto* unityCursorSetVisiblePointer = MethodPointer(
        unityCursorSetVisible);
    auto* unityApplicationQuitPointer = MethodPointer(
        unityApplicationQuit);
    auto* originalInventoryPanelOpenPointer = MethodPointer(
        originalInventoryPanelOpen);
    auto* eventMethodManagerUpdatePointer = MethodPointer(
        eventMethodManagerUpdate);
    auto* inGameManagerUpdatePointer = MethodPointer(inGameManagerUpdate);
    auto* baseCameraLateUpdatePointer = MethodPointer(baseCameraLateUpdate);
    auto* tpCameraLateUpdatePointer = MethodPointer(tpCameraLateUpdate);
    auto* inventoryCameraLateUpdatePointer = MethodPointer(
        inventoryCameraLateUpdate);
    auto* fpCameraLateUpdatePointer = MethodPointer(fpCameraLateUpdate);
    auto* saveManagerSavePointer = MethodPointer(saveManagerSave);
    auto* interactManagerRegisterPointer =
        MethodPointer(interactManagerRegister);
    auto* interactManagerUnregisterPointer =
        MethodPointer(interactManagerUnregister);
    auto* systemMenuOpenPointer = MethodPointer(systemMenuOpen);
    auto* systemMenuResumePointer = MethodPointer(systemMenuResume);
    auto* systemMenuSaveAndGoToTitlePointer = MethodPointer(
        systemMenuSaveAndGoToTitle);
    auto* tutorialPanelOpenPointer = MethodPointer(tutorialPanelOpen);
    auto* missionFailedLoadPointer = MethodPointer(
        missionFailedLoad);
    auto* missionFailedGoToTitlePointer = MethodPointer(
        missionFailedGoToTitle);
    auto* missionFailedOpenPointer = MethodPointer(
        missionFailedOpen);
    LogInfo(LogCategory_Hook, L"Hook", 
        L"Hook targets: input=%p inputProcess=%p actionDown=%p actionDownFrame=%p actionUpFrame=%p actionGet=%p appQuit=%p inputResolver=%p eventMethod=%p inGame=%p player=%p npcFound=%p playerAnimation=%p playerClothes=%p holdInHand=%p coatDrop=%p coatPickEnd=%p baseCamera=%p tpCamera=%p inventoryCamera=%p fpCamera=%p save=%p interactRegister=%p interactUnregister=%p.",
        inputManagerUpdatePointer,
        inputManagerUpdateProcessPointer,
        inputManagerIsDownPointer,
        inputManagerIsDownFramePointer,
        inputManagerIsUpFramePointer,
        inputManagerGetInputActionPointer,
        unityApplicationQuitPointer,
        inputResolverResolvePointer,
        eventMethodManagerUpdatePointer,
        inGameManagerUpdatePointer,
        playerUpdatePointer,
        playerOnFoundStrangerEventPointer,
        playerAnimationManagerUpdatePointer,
        playerClothesActionUpdatePointer,
        playerHoldInHandHoldPointer,
        playerAnimationDropClothesPointer,
        playerAnimationEndPickingCoatPointer,
        baseCameraLateUpdatePointer,
        tpCameraLateUpdatePointer,
        inventoryCameraLateUpdatePointer,
        fpCameraLateUpdatePointer,
        saveManagerSavePointer,
        interactManagerRegisterPointer,
        interactManagerUnregisterPointer);
    if (titleUpdatePointer == nullptr ||
        buttonOnClickPointer == nullptr ||
        inputManagerUpdatePointer == nullptr ||
        inputManagerUpdateProcessPointer == nullptr ||
        inputManagerIsDownPointer == nullptr ||
        inputManagerIsDownFramePointer == nullptr ||
        inputManagerIsUpFramePointer == nullptr ||
        inputManagerGetInputActionPointer == nullptr ||
        inputResolverResolvePointer == nullptr ||
        unityInputGetKeyPointer == nullptr ||
        unityInputGetKeyDownPointer == nullptr ||
        unityInputGetKeyUpPointer == nullptr ||
        unityCursorSetLockStatePointer == nullptr ||
        unityCursorSetVisiblePointer == nullptr ||
        unityApplicationQuitPointer == nullptr ||
        originalInventoryPanelOpenPointer == nullptr ||
        playerUpdatePointer == nullptr ||
        playerAnimationManagerUpdatePointer == nullptr ||
        playerClothesActionUpdatePointer == nullptr ||
        playerClothesGetExposedBodyPartsPointer == nullptr ||
        playerHoldInHandHoldPointer == nullptr ||
        playerAnimationDropClothesPointer == nullptr ||
        playerAnimationEndPickingCoatPointer == nullptr ||
        eventMethodManagerUpdatePointer == nullptr ||
        inGameManagerUpdatePointer == nullptr ||
        baseCameraLateUpdatePointer == nullptr ||
        tpCameraLateUpdatePointer == nullptr ||
        inventoryCameraLateUpdatePointer == nullptr ||
        fpCameraLateUpdatePointer == nullptr ||
        saveManagerSavePointer == nullptr ||
        interactManagerRegisterPointer == nullptr ||
        interactManagerUnregisterPointer == nullptr ||
        systemMenuOpenPointer == nullptr ||
        systemMenuResumePointer == nullptr ||
        tutorialPanelOpenPointer == nullptr)
    {
        LogInfo(LogCategory_Hook, L"Hook", 
            L"Unable to resolve lifecycle hooks: title=%p button=%p input=%p inputProcess=%p inputResolver=%p player=%p npcFound=%p playerAnimation=%p eventMethod=%p inGame=%p baseCamera=%p tpCamera=%p inventoryCamera=%p fpCamera=%p save=%p systemMenu=%p.",
            titleUpdatePointer,
            buttonOnClickPointer,
            inputManagerUpdatePointer,
            inputManagerUpdateProcessPointer,
            inputResolverResolvePointer,
            playerUpdatePointer,
            playerOnFoundStrangerEventPointer,
            playerAnimationManagerUpdatePointer,
            eventMethodManagerUpdatePointer,
            inGameManagerUpdatePointer,
            baseCameraLateUpdatePointer,
            tpCameraLateUpdatePointer,
            inventoryCameraLateUpdatePointer,
            fpCameraLateUpdatePointer,
            saveManagerSavePointer,
            systemMenuOpenPointer);
        g_fallbackVisible = true;
        return;
    }

    if (MH_Initialize() != MH_OK)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"MinHook initialization failed.");
        g_fallbackVisible = true;
        return;
    }

    HMODULE user32Module = GetModuleHandleW(L"user32.dll");
    void* getAsyncKeyStatePointer = user32Module != nullptr
        ? reinterpret_cast<void*>(
            GetProcAddress(user32Module, "GetAsyncKeyState"))
        : nullptr;
    void* getKeyStatePointer = user32Module != nullptr
        ? reinterpret_cast<void*>(
            GetProcAddress(user32Module, "GetKeyState"))
        : nullptr;
    void* setCursorPosPointer = user32Module != nullptr
        ? reinterpret_cast<void*>(
            GetProcAddress(user32Module, "SetCursorPos"))
        : nullptr;
    void* clipCursorPointer = user32Module != nullptr
        ? reinterpret_cast<void*>(
            GetProcAddress(user32Module, "ClipCursor"))
        : nullptr;
    if (getAsyncKeyStatePointer == nullptr ||
        getKeyStatePointer == nullptr ||
        setCursorPosPointer == nullptr ||
        clipCursorPointer == nullptr)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"Unable to resolve native Windows keyboard-state APIs.");
        g_fallbackVisible = true;
        return;
    }

    if (playerOnFoundStrangerEventPointer != nullptr &&
        MH_CreateHook(
            playerOnFoundStrangerEventPointer,
            reinterpret_cast<LPVOID>(&PlayerControllerOnFoundStrangerEventHook),
            &g_originalPlayerOnFoundStrangerEvent) != MH_OK)
    {
        g_originalPlayerOnFoundStrangerEvent = nullptr;
        LogInfo(LogCategory_Hook, L"Hook", L"NPC pursuit found-event hook could not be created; pursuit feature disabled.");
    }

    if (npcManagerUpdatePointer != nullptr &&
        MH_CreateHook(
            npcManagerUpdatePointer,
            reinterpret_cast<LPVOID>(&NpcManagerUpdateHook),
            &g_originalNpcManagerUpdate) == MH_OK)
    {
        g_gameplayActionPursuitPostUpdateHookReady = true;
        LogInfo(LogCategory_Hook, L"Hook", L"NPC pursuit post-update hold hook installed: %p.", npcManagerUpdatePointer);
    }
    else
    {
        g_originalNpcManagerUpdate = nullptr;
        g_gameplayActionPursuitPostUpdateHookReady = false;
        LogInfo(LogCategory_Hook, L"Hook", L"NPC pursuit post-update hold hook could not be installed.");
    }

    if (MH_CreateHook(
            titleUpdatePointer,
            reinterpret_cast<LPVOID>(&TitleUpdateHook),
            &g_originalTitleUpdate) != MH_OK ||
        MH_CreateHook(
            buttonOnClickPointer,
            reinterpret_cast<LPVOID>(&ButtonOnClickHook),
            &g_originalButtonOnClick) != MH_OK ||
        MH_CreateHook(
            inputManagerUpdatePointer,
            reinterpret_cast<LPVOID>(&InputManagerUpdateHook),
            &g_originalInputManagerUpdate) != MH_OK ||
        MH_CreateHook(
            inputManagerUpdateProcessPointer,
            reinterpret_cast<LPVOID>(&InputManagerUpdateProcessHook),
            &g_originalInputManagerUpdateProcess) != MH_OK ||
        MH_CreateHook(
            inputManagerIsDownPointer,
            reinterpret_cast<LPVOID>(&GameplayInput_InputManagerIsDownHook),
            &g_originalInputManagerIsDown) != MH_OK ||
        MH_CreateHook(
            inputManagerIsDownFramePointer,
            reinterpret_cast<LPVOID>(
                &GameplayInput_InputManagerIsDownFrameHook),
            &g_originalInputManagerIsDownFrame) != MH_OK ||
        MH_CreateHook(
            inputManagerIsUpFramePointer,
            reinterpret_cast<LPVOID>(
                &GameplayInput_InputManagerIsUpFrameHook),
            &g_originalInputManagerIsUpFrame) != MH_OK ||
        MH_CreateHook(
            inputManagerGetInputActionPointer,
            reinterpret_cast<LPVOID>(
                &GameplayInput_InputManagerGetInputActionHook),
            &g_originalInputManagerGetInputAction) != MH_OK ||
        MH_CreateHook(
            inputResolverResolvePointer,
            reinterpret_cast<LPVOID>(&InputResolverResolveHook),
            &g_originalInputResolverResolve) != MH_OK ||
        MH_CreateHook(
            unityInputGetKeyPointer,
            reinterpret_cast<LPVOID>(&GameplayInput_GetKeyHook),
            &g_originalUnityInputGetKey) != MH_OK ||
        MH_CreateHook(
            unityInputGetKeyDownPointer,
            reinterpret_cast<LPVOID>(&GameplayInput_GetKeyDownHook),
            &g_originalUnityInputGetKeyDown) != MH_OK ||
        MH_CreateHook(
            unityInputGetKeyUpPointer,
            reinterpret_cast<LPVOID>(&GameplayInput_GetKeyUpHook),
            &g_originalUnityInputGetKeyUp) != MH_OK ||
        MH_CreateHook(
            unityCursorSetLockStatePointer,
            reinterpret_cast<LPVOID>(&CursorSetLockStateHook),
            &g_originalCursorSetLockState) != MH_OK ||
        MH_CreateHook(
            unityCursorSetVisiblePointer,
            reinterpret_cast<LPVOID>(&CursorSetVisibleHook),
            &g_originalCursorSetVisible) != MH_OK ||
        MH_CreateHook(
            setCursorPosPointer,
            reinterpret_cast<LPVOID>(&SetCursorPosHook),
            &g_originalSetCursorPos) != MH_OK ||
        MH_CreateHook(
            clipCursorPointer,
            reinterpret_cast<LPVOID>(&ClipCursorHook),
            &g_originalClipCursor) != MH_OK ||
        MH_CreateHook(
            unityApplicationQuitPointer,
            reinterpret_cast<LPVOID>(&ApplicationQuitHook),
            &g_originalApplicationQuit) != MH_OK ||
        MH_CreateHook(
            originalInventoryPanelOpenPointer,
            reinterpret_cast<LPVOID>(
                &GameplayOriginalInventoryOpenHook),
            &g_originalInventoryPanelOpen) != MH_OK ||
        MH_CreateHook(
            playerUpdatePointer,
            reinterpret_cast<LPVOID>(&PlayerControllerUpdateHook),
            &g_originalPlayerUpdate) != MH_OK ||
        MH_CreateHook(
            playerAnimationManagerUpdatePointer,
            reinterpret_cast<LPVOID>(&PlayerAnimationManagerUpdateHook),
            &g_originalPlayerAnimationManagerUpdate) != MH_OK ||
        MH_CreateHook(
            playerClothesActionUpdatePointer,
            reinterpret_cast<LPVOID>(&PlayerClothesActionUpdateHook),
            &g_originalPlayerClothesActionUpdate) != MH_OK ||
        MH_CreateHook(
            playerClothesGetExposedBodyPartsPointer,
            reinterpret_cast<LPVOID>(
                &PlayerClothesGetExposedBodyPartsHook),
            &g_originalPlayerClothesGetExposedBodyParts) != MH_OK ||
        MH_CreateHook(
            playerHoldInHandHoldPointer,
            reinterpret_cast<LPVOID>(&PlayerHoldInHandHoldHook),
            &g_originalPlayerHoldInHandHold) != MH_OK ||
        MH_CreateHook(
            playerAnimationDropClothesPointer,
            reinterpret_cast<LPVOID>(&PlayerAnimationDropClothesHook),
            &g_originalPlayerAnimationDropClothes) != MH_OK ||
        MH_CreateHook(
            playerAnimationEndPickingCoatPointer,
            reinterpret_cast<LPVOID>(
                &PlayerAnimationEndPickingCoatHook),
            &g_originalPlayerAnimationEndPickingCoat) != MH_OK ||
        MH_CreateHook(
            eventMethodManagerUpdatePointer,
            reinterpret_cast<LPVOID>(&EventMethodManagerUpdateHook),
            &g_originalEventMethodManagerUpdate) != MH_OK ||
        MH_CreateHook(
            inGameManagerUpdatePointer,
            reinterpret_cast<LPVOID>(&InGameManagerUpdateHook),
            &g_originalInGameManagerUpdate) != MH_OK ||
        MH_CreateHook(
            baseCameraLateUpdatePointer,
            reinterpret_cast<LPVOID>(&BaseCameraLateUpdateHook),
            &g_originalBaseCameraLateUpdate) != MH_OK ||
        MH_CreateHook(
            tpCameraLateUpdatePointer,
            reinterpret_cast<LPVOID>(&TpCameraLateUpdateHook),
            &g_originalTpCameraLateUpdate) != MH_OK ||
        MH_CreateHook(
            inventoryCameraLateUpdatePointer,
            reinterpret_cast<LPVOID>(&InventoryCameraLateUpdateHook),
            &g_originalInventoryCameraLateUpdate) != MH_OK ||
        MH_CreateHook(
            fpCameraLateUpdatePointer,
            reinterpret_cast<LPVOID>(&FpCameraLateUpdateHook),
            &g_originalFpCameraLateUpdate) != MH_OK ||
        MH_CreateHook(
            saveManagerSavePointer,
            reinterpret_cast<LPVOID>(&SaveManagerSaveHook),
            &g_originalSaveManagerSave) != MH_OK ||
        MH_CreateHook(
            interactManagerRegisterPointer,
            reinterpret_cast<LPVOID>(&InteractManagerRegisterHook),
            &g_originalInteractManagerRegister) != MH_OK ||
        MH_CreateHook(
            interactManagerUnregisterPointer,
            reinterpret_cast<LPVOID>(&InteractManagerUnregisterHook),
            &g_originalInteractManagerUnregister) != MH_OK ||
        MH_CreateHook(
            systemMenuOpenPointer,
            reinterpret_cast<LPVOID>(&SystemMenuOpenHook),
            &g_originalSystemMenuOpen) != MH_OK ||
        MH_CreateHook(
            systemMenuResumePointer,
            reinterpret_cast<LPVOID>(&SystemMenuResumeHook),
            &g_originalSystemMenuResume) != MH_OK ||
        MH_CreateHook(
            systemMenuSaveAndGoToTitlePointer,
            reinterpret_cast<LPVOID>(&SystemMenuSaveAndGoToTitleHook),
            &g_originalSystemMenuSaveAndGoToTitle) != MH_OK ||
        MH_CreateHook(
            tutorialPanelOpenPointer,
            reinterpret_cast<LPVOID>(&TutorialPanelOpenHook),
            &g_originalTutorialPanelOpen) != MH_OK ||
        MH_CreateHook(
            missionFailedLoadPointer,
            reinterpret_cast<LPVOID>(&MissionFailedLoadHook),
            &g_originalMissionFailedLoad) != MH_OK ||
        MH_CreateHook(
            missionFailedGoToTitlePointer,
            reinterpret_cast<LPVOID>(&MissionFailedGoToTitleHook),
            &g_originalMissionFailedGoToTitle) != MH_OK ||
        MH_CreateHook(
            missionFailedOpenPointer,
            reinterpret_cast<LPVOID>(&MissionFailedOpenHook),
            &g_originalMissionFailedOpen) != MH_OK ||
        MH_CreateHook(
            getAsyncKeyStatePointer,
            reinterpret_cast<LPVOID>(
                &GameplayInput_GetAsyncKeyStateHook),
            &g_originalGetAsyncKeyState) != MH_OK ||
        MH_CreateHook(
            getKeyStatePointer,
            reinterpret_cast<LPVOID>(
                &GameplayInput_GetKeyStateHook),
            &g_originalGetKeyState) != MH_OK ||
        MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        LogInfo(LogCategory_Hook, L"Hook", L"Native title/gameplay hooks could not be enabled.");
        g_fallbackVisible = true;
        return;
    }

    const auto installOptionalHook =
        [](void* target, LPVOID detour, void** original, const wchar_t* name)
        {
            if (target == nullptr || detour == nullptr || original == nullptr)
            {
                LogWarn(
                    LogCategory_Hook,
                    L"OptionalHookMissing",
                    L"Optional logging hook target is unavailable: %s.",
                    name);
                return false;
            }
            if (MH_CreateHook(target, detour, original) != MH_OK ||
                MH_EnableHook(target) != MH_OK)
            {
                *original = nullptr;
                LogWarn(
                    LogCategory_Hook,
                    L"OptionalHookFailed",
                    L"Optional logging hook could not be installed: %s.",
                    name);
                return false;
            }
            return true;
        };
    int optionalHooksInstalled = 0;
    optionalHooksInstalled += installOptionalHook(
        playerActionUpdatePointer,
        reinterpret_cast<LPVOID>(&PlayerActionUpdateHook),
        &g_originalPlayerActionUpdate,
        L"PlayerAction.OnUpdate") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerActionChangeActionPointer,
        reinterpret_cast<LPVOID>(&PlayerActionChangeActionHook),
        &g_originalPlayerActionChangeAction,
        L"PlayerAction.ChangeAction") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerActionOnFrameEndPointer,
        reinterpret_cast<LPVOID>(&PlayerActionOnFrameEndHook),
        &g_originalPlayerActionOnFrameEnd,
        L"PlayerAction.OnFrameEnd") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationTransActionPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationTransActionHook),
        &g_originalPlayerAnimationTransAction,
        L"PlayerAnimationManager.TransAction") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationSetMoveSpeedPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationSetMoveSpeedHook),
        &g_originalPlayerAnimationSetMoveSpeed,
        L"PlayerAnimationManager.SetMoveSpeed") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationSetGamanBaibuPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationSetGamanBaibuHook),
        &g_originalPlayerAnimationSetGamanBaibu,
        L"PlayerAnimationManager.SetGamanBaibu") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationSetEcstasyPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationSetEcstasyHook),
        &g_originalPlayerAnimationSetEcstasy,
        L"PlayerAnimationManager.SetEcstasy") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAdultGoodsControllerUpdatePointer,
        reinterpret_cast<LPVOID>(&PlayerAdultGoodsControllerUpdateHook),
        &g_originalPlayerAdultGoodsControllerUpdate,
        L"PlayerAdultGoodsController.OnUpdate") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationCommonStartAnimPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationCommonStartAnimHook),
        &g_originalPlayerAnimationCommonStartAnim,
        L"PlayerAnimationReceiver.CommonStartAnim") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationCommonOnEndAnimPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationCommonOnEndAnimHook),
        &g_originalPlayerAnimationCommonOnEndAnim,
        L"PlayerAnimationReceiver.CommonOnEndAnim") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationChangeVibratorPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationChangeVibratorHook),
        &g_originalPlayerAnimationChangeVibrator,
        L"PlayerAnimationReceiver.ChangeVibrator") ? 1 : 0;
    optionalHooksInstalled += installOptionalHook(
        playerAnimationEndEcstasyPointer,
        reinterpret_cast<LPVOID>(&PlayerAnimationEndEcstasyHook),
        &g_originalPlayerAnimationEndEcstasy,
        L"PlayerAnimationReceiver.OnAnimEndEcstasy") ? 1 : 0;
    LogInfo(
        LogCategory_Hook,
        L"ActionHooks",
        L"Action and animation logging hooks installed: %d/12.",
        optionalHooksInstalled);

    g_nativeHooksInstalled = true;
    LogInfo(LogCategory_Hook, L"Hook", L"Native title, player lifecycle, camera, save, and system-menu hooks enabled.");
}

void RefreshTitleState()
{
    if (g_titleProbeComplete)
    {
        return;
    }

    if (!g_titleApiReady && !ResolveIl2CppApi())
    {
        return;
    }

    Il2CppObject* instance = nullptr;
    if (g_titleInstanceField != nullptr)
    {
        il2cpp_field_static_get_value(g_titleInstanceField, &instance);
    }

    g_titlePresent = instance != nullptr;
    if (g_titlePresent)
    {
        g_titleProbeComplete = true;
        g_titleInstance = instance;
        InstallNativeHooks();
        SetStatus(L"TitleSceneView detected. Native menu integration armed.");
    }
}

