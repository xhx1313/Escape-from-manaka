// Native.cpp module: character_animation.inc
// Included inside the shared implementation namespace by Native.cpp.

void ApplyEscapeFromManakaStandingPose(Il2CppObject* modelTransform)
{
    SetSkeletonBoneLocalEuler(modelTransform, L"UpperLeg_L", 0.0f, 180.0f, 180.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"UpperLeg_R", 0.0f, 180.0f, 180.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"LowerLeg_L", 0.0f, 0.0f, 0.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"LowerLeg_R", 0.0f, 0.0f, 0.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"Foot_L", 0.0f, 0.0f, 0.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"Foot_R", 0.0f, 0.0f, 0.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"UpperArm_L", -90.0f, 0.0f, 0.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"UpperArm_R", -90.0f, 0.0f, 0.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"LowerArm_L", 0.0f, 0.0f, 0.0f);
    SetSkeletonBoneLocalEuler(modelTransform, L"LowerArm_R", 0.0f, 0.0f, 0.0f);
}

void EscapeFromManaka_SelectAction(int action)
{
    g_efmSelectedAction = action < 0
        ? 0
        : (action >= kEscapeFromManakaActionCount
            ? kEscapeFromManakaActionCount - 1
            : action);
    LogInfo(LogCategory_Animation, L"Animation", 
        L"EscapeFromManaka action selected: %S.",
        kEscapeFromManakaActions[g_efmSelectedAction].label);
}

int EscapeFromManaka_GetActionTypeId(int action)
{
    // Extracted from the AnyState ActionId conditions in the shipped Player
    // AnimatorController. Entries are ordered from global action index 37.
    static constexpr int actionIds[] = {
        10013, 50000, 50001, 50002, 50003, 50004, 50005, 50006,
        50007, 50009, 50008, 50012, 50011, 50017, 50014, 50015,
        50016, 50010, 50013, 50018, 50019, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        50020, -1, 50021, -1, 50022, -1, 10006, 10007,
        10023, 10008, 10020, 10021, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, 10063, 10030, -1,
        10045, -1, 10055, -1, 10050, -1, 10035, -1,
        100000, -1, 100001, -1
    };
    constexpr int firstAction = 37;
    constexpr int actionCount =
        static_cast<int>(sizeof(actionIds) / sizeof(actionIds[0]));
    return action >= firstAction && action < firstAction + actionCount
        ? actionIds[action - firstAction]
        : -1;
}

bool EscapeFromManaka_ApplyOriginalAction(int action)
{
    if (g_previewAnimator == nullptr)
    {
        return false;
    }

    // Capture the untouched gameplay Animator before the preview menu starts
    // changing layer weights. The snapshot is reused after scene transitions
    // only when the same Animator instance is still alive.
    ExtendedMode_CaptureAnimatorState();

    const auto& definition = kEscapeFromManakaActions[action];
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    // ActionId is the AnyState guard used by the shipped Action layer. The
    // player returns it to -1 while idle, which makes transient action states
    // exit immediately.
    const int mappedActionId = EscapeFromManaka_GetActionTypeId(action);
    int actionId = definition.layer == 0
        ? -1
        : (mappedActionId >= 0 ? mappedActionId : 10000 + action);
    constexpr int kActionIdHash = 1957839355;
    void* actionIdParameters[] = { const_cast<int*>(&kActionIdHash), &actionId };
    Invoke(
        FindMethod(animatorClass, "SetInteger", 2),
        g_previewAnimator,
        actionIdParameters);
    auto* transitionTrigger = il2cpp_string_new("TransAction");
    void* triggerParameters[] = { transitionTrigger };
    Invoke(
        FindMethod(
            animatorClass,
            mappedActionId >= 0 ? "SetTrigger" : "ResetTrigger",
            1),
        g_previewAnimator,
        triggerParameters);
    for (int layerIndex = 0; layerIndex < 12; ++layerIndex)
    {
        float weight = layerIndex == definition.layer ? 1.0f : 0.0f;
        void* parameters[] = { &layerIndex, &weight };
        Invoke(
            FindMethod(animatorClass, "SetLayerWeight", 2),
            g_previewAnimator,
            parameters);
    }

    int layer = definition.layer;
    float normalizedTime = 0.0f;
    auto* state = il2cpp_string_new(definition.statePath);
    void* parameters[] = { state, &layer, &normalizedTime };
    Invoke(
        FindMethod(animatorClass, "Play", 3),
        g_previewAnimator,
        parameters);
    LogInfo(LogCategory_Animation, L"Animation", 
        L"EscapeFromManaka playing original Animator state %S on layer %d.",
        definition.statePath,
        definition.layer);
    return true;
}

void EscapeFromManaka_UpdateModelAction()
{
    if (!g_lobbyUiOpen ||
        g_activeUiTab.load() != 0 ||
        g_previewModel == nullptr)
    {
        return;
    }

    const int pendingAction = g_efmPendingAction.exchange(-1);
    if (pendingAction >= 0)
    {
        EscapeFromManaka_SelectAction(pendingAction);
        if (EscapeFromManaka_ApplyOriginalAction(pendingAction))
        {
            return;
        }
    }

    if (g_previewAnimator != nullptr)
    {
        return;
    }

    auto* modelTransform = GetWorldTransform(g_previewModel);
    ApplyEscapeFromManakaStandingPose(modelTransform);
    const float phase =
        static_cast<float>(GetTickCount64() % 100000) * 0.001f;

    switch (g_efmSelectedAction)
    {
    case 1:
    {
        const float breath = std::sin(phase * 1.4f) * 1.4f;
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"Chest",
            346.0f + breath,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_L",
            -90.0f - breath * 0.35f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_R",
            -90.0f - breath * 0.35f,
            0.0f,
            0.0f);
        break;
    }
    case 2:
    {
        const float swing = std::sin(phase * 5.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperLeg_L",
            swing * 18.0f,
            180.0f,
            180.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperLeg_R",
            -swing * 18.0f,
            180.0f,
            180.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"LowerLeg_L",
            (-swing > 0.0f ? -swing : 0.0f) * 22.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"LowerLeg_R",
            (swing > 0.0f ? swing : 0.0f) * 22.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_L",
            -90.0f - swing * 12.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_R",
            -90.0f + swing * 12.0f,
            0.0f,
            0.0f);
        break;
    }
    case 3:
    {
        const float swing = std::sin(phase * 8.5f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperLeg_L",
            swing * 32.0f,
            180.0f,
            180.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperLeg_R",
            -swing * 32.0f,
            180.0f,
            180.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"LowerLeg_L",
            (-swing > 0.0f ? -swing : 0.0f) * 42.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"LowerLeg_R",
            (swing > 0.0f ? swing : 0.0f) * 42.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_L",
            -90.0f - swing * 26.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_R",
            -90.0f + swing * 26.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"Chest",
            342.0f,
            0.0f,
            0.0f);
        break;
    }
    case 4:
    {
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_L",
            -62.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperArm_R",
            -62.0f,
            0.0f,
            0.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperLeg_L",
            0.0f,
            188.0f,
            180.0f);
        SetSkeletonBoneLocalEuler(
            modelTransform,
            L"UpperLeg_R",
            0.0f,
            172.0f,
            180.0f);
        break;
    }
    default:
        break;
    }
}

void LogSkeletonTransforms(
    Il2CppObject* transform,
    int depth,
    int& remaining)
{
    if (transform == nullptr || remaining <= 0 || depth > 7)
    {
        return;
    }
    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* gameObject = Invoke(
        FindMethod(FindClass("UnityEngine", "Component"), "get_gameObject", 0),
        transform,
        nullptr);
    auto* eulerObject = Invoke(
        FindMethod(transformClass, "get_localEulerAngles", 0),
        transform,
        nullptr);
    auto* localPositionObject = Invoke(
        FindMethod(transformClass, "get_localPosition", 0),
        transform,
        nullptr);
    auto* localScaleObject = Invoke(
        FindMethod(transformClass, "get_localScale", 0),
        transform,
        nullptr);
    if (gameObject != nullptr &&
        eulerObject != nullptr &&
        localPositionObject != nullptr &&
        localScaleObject != nullptr)
    {
        const auto* euler = static_cast<float*>(il2cpp_object_unbox(eulerObject));
        const auto* localPosition = static_cast<float*>(
            il2cpp_object_unbox(localPositionObject));
        const auto* localScale = static_cast<float*>(
            il2cpp_object_unbox(localScaleObject));
        LogTrace(
            LogCategory_Animation,
            L"SkeletonTransform",
            L"Skeleton[%d] %s euler=(%.1f, %.1f, %.1f) localPos=(%.4f, %.4f, %.4f) scale=(%.3f, %.3f, %.3f)",
            depth,
            GetObjectName(gameObject).c_str(),
            euler[0],
            euler[1],
            euler[2],
            localPosition[0],
            localPosition[1],
            localPosition[2],
            localScale[0],
            localScale[1],
            localScale[2]);
        --remaining;
    }

    auto* childCountObject = Invoke(
        FindMethod(transformClass, "get_childCount", 0),
        transform,
        nullptr);
    if (childCountObject == nullptr)
    {
        return;
    }
    const int childCount = *static_cast<int*>(
        il2cpp_object_unbox(childCountObject));
    for (int index = 0; index < childCount && remaining > 0; ++index)
    {
        void* parameters[] = { &index };
        auto* child = Invoke(
            FindMethod(transformClass, "GetChild", 1),
            transform,
            parameters);
        LogSkeletonTransforms(child, depth + 1, remaining);
    }
}

void LogNamedTransformPose(
    const wchar_t* stage,
    const wchar_t* label,
    Il2CppObject* transform)
{
    if (transform == nullptr)
    {
        LogTrace(
            LogCategory_Animation,
            L"LegPose",
            L"LegPose[%s] %s=<null>",
            stage,
            label);
        return;
    }

    auto* transformClass = FindClass("UnityEngine", "Transform");
    auto* eulerObject = Invoke(
        FindMethod(transformClass, "get_localEulerAngles", 0),
        transform,
        nullptr);
    auto* localPositionObject = Invoke(
        FindMethod(transformClass, "get_localPosition", 0),
        transform,
        nullptr);
    auto* localScaleObject = Invoke(
        FindMethod(transformClass, "get_localScale", 0),
        transform,
        nullptr);
    auto* parent = Invoke(
        FindMethod(transformClass, "get_parent", 0),
        transform,
        nullptr);
    if (eulerObject == nullptr ||
        localPositionObject == nullptr ||
        localScaleObject == nullptr ||
        il2cpp_object_unbox == nullptr)
    {
        LogTrace(
            LogCategory_Animation,
            L"LegPose",
            L"LegPose[%s] %s=<unreadable>",
            stage,
            label);
        return;
    }

    const auto* euler = static_cast<float*>(
        il2cpp_object_unbox(eulerObject));
    const auto* localPosition = static_cast<float*>(
        il2cpp_object_unbox(localPositionObject));
    const auto* localScale = static_cast<float*>(
        il2cpp_object_unbox(localScaleObject));
    LogTrace(
        LogCategory_Animation,
        L"LegPose",
        L"LegPose[%s] %s euler=(%.2f, %.2f, %.2f) localPos=(%.4f, %.4f, %.4f) scale=(%.3f, %.3f, %.3f) parent=%s",
        stage,
        label,
        euler[0],
        euler[1],
        euler[2],
        localPosition[0],
        localPosition[1],
        localPosition[2],
        localScale[0],
        localScale[1],
        localScale[2],
        parent != nullptr
            ? GetObjectName(GetComponentGameObject(parent)).c_str()
            : L"<null>");
    auto* worldPositionObject = Invoke(
        FindMethod(transformClass, "get_position", 0),
        transform,
        nullptr);
    if (worldPositionObject != nullptr)
    {
        const auto* worldPosition = static_cast<float*>(
            il2cpp_object_unbox(worldPositionObject));
        LogTrace(
            LogCategory_Animation,
            L"LegPose",
            L"LegPose[%s] %s worldPos=(%.4f, %.4f, %.4f)",
            stage,
            label,
            worldPosition[0],
            worldPosition[1],
            worldPosition[2]);
    }

    if (parent != nullptr)
    {
        auto* parentEulerObject = Invoke(
            FindMethod(transformClass, "get_localEulerAngles", 0),
            parent,
            nullptr);
        auto* parentPositionObject = Invoke(
            FindMethod(transformClass, "get_localPosition", 0),
            parent,
            nullptr);
        auto* parentScaleObject = Invoke(
            FindMethod(transformClass, "get_localScale", 0),
            parent,
            nullptr);
        auto* grandParent = Invoke(
            FindMethod(transformClass, "get_parent", 0),
            parent,
            nullptr);
        if (parentEulerObject != nullptr &&
            parentPositionObject != nullptr &&
            parentScaleObject != nullptr)
        {
            const auto* parentEuler = static_cast<float*>(
                il2cpp_object_unbox(parentEulerObject));
            const auto* parentPosition = static_cast<float*>(
                il2cpp_object_unbox(parentPositionObject));
            const auto* parentScale = static_cast<float*>(
                il2cpp_object_unbox(parentScaleObject));
            LogTrace(
                LogCategory_Animation,
                L"LegPose",
                L"LegPose[%s] %s parentPose euler=(%.2f, %.2f, %.2f) localPos=(%.4f, %.4f, %.4f) scale=(%.3f, %.3f, %.3f) grandParent=%s",
                stage,
                label,
                parentEuler[0],
                parentEuler[1],
                parentEuler[2],
                parentPosition[0],
                parentPosition[1],
                parentPosition[2],
                parentScale[0],
                parentScale[1],
                parentScale[2],
                grandParent != nullptr
                    ? GetObjectName(
                        GetComponentGameObject(grandParent)).c_str()
                    : L"<null>");
        }
    }
}

void LogEscapeFromManakaLegState(const wchar_t* stage)
{
    LogTrace(
        LogCategory_Animation,
        L"LegPose",
        L"LegPose[%s] begin",
        stage);

    auto* referencer = g_extendedAvatarReferencer;
    if (referencer != nullptr)
    {
        LogNamedTransformPose(
            stage,
            L"Hips",
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0x138));
        LogNamedTransformPose(
            stage,
            L"LegL",
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0xA0));
        LogNamedTransformPose(
            stage,
            L"LegR",
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0xA8));
        LogNamedTransformPose(
            stage,
            L"LowerLegL",
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0xB0));
        LogNamedTransformPose(
            stage,
            L"LowerLegR",
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0xB8));
        LogNamedTransformPose(
            stage,
            L"FootL",
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0x90));
        LogNamedTransformPose(
            stage,
            L"FootR",
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(referencer) + 0x98));
    }

    auto* playerClassAccessor = g_extendedPlayerController != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
        : nullptr;
    if (playerClassAccessor != nullptr)
    {
        auto* knee = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x198);
        auto* kneeValue = knee != nullptr
            ? Invoke(
                FindMethod(
                    FindClass(
                        "ExposureUnnoticed2.Object3D.Player.Scripts",
                        "PlayerKneeAdjuster"),
                    "get_KneeOffsetY",
                    0),
                knee,
                nullptr)
            : nullptr;
        auto* toe = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x108);
        auto* heelValue = toe != nullptr
            ? Invoke(
                FindMethod(
                    FindClass(
                        "ExposureUnnoticed2.Object3D.Player.Scripts",
                        "PlayerToeHeelController"),
                    "get_HeelOffset",
                    0),
                toe,
                nullptr)
            : nullptr;
        if (kneeValue != nullptr &&
            heelValue != nullptr &&
            il2cpp_object_unbox != nullptr)
        {
            LogTrace(
                LogCategory_Animation,
                L"LegPose",
                L"LegPose[%s] KneeOffsetY=%.5f HeelOffset=%.5f",
                stage,
                *static_cast<float*>(il2cpp_object_unbox(kneeValue)),
                *static_cast<float*>(il2cpp_object_unbox(heelValue)));
        }
    }

    if (g_previewAnimator != nullptr)
    {
        auto* animatorClass = FindClass("UnityEngine", "Animator");
        auto* getState = FindMethod(
            animatorClass,
            "GetCurrentAnimatorStateInfo",
            1);
        for (int layer = 0; layer < 2; ++layer)
        {
            void* parameters[] = { &layer };
            auto* stateObject = Invoke(
                getState,
                g_previewAnimator,
                parameters);
            if (stateObject == nullptr || il2cpp_object_unbox == nullptr)
            {
                continue;
            }
            const auto* state = static_cast<const int*>(
                il2cpp_object_unbox(stateObject));
            const auto* stateFloats =
                reinterpret_cast<const float*>(state);
            LogTrace(
                LogCategory_Animation,
                L"AnimatorLayers",
                L"LegPose[%s] Animator layer=%d nameHash=%d pathHash=%d fullPathHash=%d normalized=%.3f length=%.3f speed=%.3f loop=%d",
                stage,
                layer,
                state[0],
                state[1],
                state[2],
                stateFloats[3],
                stateFloats[4],
                stateFloats[5],
                state[8]);
        }
    }
    LogTrace(
        LogCategory_Animation,
        L"LegPose",
        L"LegPose[%s] end",
        stage);
}

int CountComponentsInChildren(
    Il2CppObject* gameObject,
    const char* namespaceName,
    const char* className)
{
    auto* componentClass = FindClass(namespaceName, className);
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (gameObject == nullptr || componentClass == nullptr || gameObjectClass == nullptr)
    {
        return 0;
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
    return components != nullptr ? static_cast<int>(il2cpp_array_length(components)) : 0;
}

void EscapeFromManaka_SilencePreparingAudio()
{
    if (g_efmAudioSilenced.exchange(true))
    {
        return;
    }
    auto* gameManagerName = il2cpp_string_new("InGameManager");
    void* findParameters[] = { gameManagerName };
    auto* gameManager = Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "Find", 1),
        nullptr,
        findParameters);
    if (gameManager == nullptr)
    {
        g_efmAudioSilenced = false;
        return;
    }

    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    il2cpp_runtime_class_init(audioSourceClass);
    auto* audioSourceType = il2cpp_type_get_object(
        il2cpp_class_get_type(audioSourceClass));
    bool includeInactive = true;
    void* sourceFindParameters[] = { audioSourceType, &includeInactive };
    auto* sources = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        gameManager,
        sourceFindParameters);
    if (sources != nullptr)
    {
        const auto sourceCount = il2cpp_array_length(sources);
        auto* elements = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(sources) + il2cpp_array_object_header_size());
        for (uint32_t index = 0; index < sourceCount; ++index)
        {
            auto* source = elements[index];
            if (source == nullptr)
            {
                continue;
            }
            if (MoaningAudio_IsManagedSource(source))
            {
                continue;
            }
            bool muted = true;
            float volume = 0.0f;
            void* muteParameters[] = { &muted };
            void* volumeParameters[] = { &volume };
            Invoke(
                FindMethod(audioSourceClass, "Stop", 0),
                source,
                nullptr);
            Invoke(
                FindMethod(audioSourceClass, "set_mute", 1),
                source,
                muteParameters);
            Invoke(
                FindMethod(audioSourceClass, "set_volume", 1),
                source,
                volumeParameters);
        }
        LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing silenced %u gameplay AudioSource(s).", sourceCount);
    }

    auto* listenerClass = FindClass("UnityEngine", "AudioListener");
    il2cpp_runtime_class_init(listenerClass);
    auto* listenerType = il2cpp_type_get_object(
        il2cpp_class_get_type(listenerClass));
    void* listenerFindParameters[] = { listenerType, &includeInactive };
    auto* listeners = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        gameManager,
        listenerFindParameters);
    if (listeners != nullptr)
    {
        bool enabled = false;
        void* enabledParameters[] = { &enabled };
        const auto listenerCount = il2cpp_array_length(listeners);
        auto* elements = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(listeners) + il2cpp_array_object_header_size());
        for (uint32_t index = 0; index < listenerCount; ++index)
        {
            Invoke(
                FindMethod(FindClass("UnityEngine", "Behaviour"), "set_enabled", 1),
                elements[index],
                enabledParameters);
        }
    }
}

Il2CppObject* FindPreviewAnimatorController(Il2CppObject* gameObject)
{
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (gameObject == nullptr || animatorClass == nullptr || gameObjectClass == nullptr)
    {
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(animatorClass);
    }
    auto* animatorType = il2cpp_type_get_object(
        il2cpp_class_get_type(animatorClass));
    bool includeInactive = true;
    void* findParameters[] = { animatorType, &includeInactive };
    auto* animators = Invoke(
        FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
        gameObject,
        findParameters);
    if (animators == nullptr)
    {
        return nullptr;
    }

    const auto count = il2cpp_array_length(animators);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(animators) + il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* animator = elements[index];
        if (animator == nullptr)
        {
            continue;
        }
        auto* controller = Invoke(
            FindMethod(animatorClass, "get_runtimeAnimatorController", 0),
            animator,
            nullptr);
        LogInfo(LogCategory_Animation, L"Animation", 
            L"EscapeFromManaka source Animator[%u] on %s controller=%s",
            index,
            GetObjectName(GetComponentGameObject(animator)).c_str(),
            GetObjectName(controller).c_str());
        if (controller != nullptr)
        {
            return controller;
        }
    }
    return nullptr;
}

Il2CppObject* FindLoadedEscapeAnimatorController()
{
    auto* animatorClass = FindClass("UnityEngine", "Animator");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (animatorClass == nullptr || objectClass == nullptr)
    {
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(animatorClass);
    }
    auto* animatorType = il2cpp_type_get_object(
        il2cpp_class_get_type(animatorClass));
    bool includeInactive = true;
    void* parameters[] = { animatorType, &includeInactive };
    auto* animators = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (animators == nullptr)
    {
        return nullptr;
    }

    const auto count = il2cpp_array_length(animators);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(animators) + il2cpp_array_object_header_size());
    Il2CppObject* fallback = nullptr;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* animator = elements[index];
        if (animator == nullptr)
        {
            continue;
        }
        auto* controller = Invoke(
            FindMethod(animatorClass, "get_runtimeAnimatorController", 0),
            animator,
            nullptr);
        const auto controllerName = GetObjectName(controller);
        if (controller != nullptr && index < 40)
        {
            LogInfo(LogCategory_Animation, L"Animation", 
                L"Loaded Animator[%u] %s controller=%s",
                index,
                GetObjectName(GetComponentGameObject(animator)).c_str(),
                controllerName.c_str());
        }
        if (controller != nullptr &&
            (controllerName.find(L"Npc") != std::wstring::npos ||
             controllerName.find(L"Player") != std::wstring::npos))
        {
            return controller;
        }
    }
    return nullptr;
}

Il2CppObject* FindLoadedEscapeIdleClip()
{
    auto* clipClass = FindClass("UnityEngine", "AnimationClip");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (clipClass == nullptr || objectClass == nullptr)
    {
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(clipClass);
    }
    auto* clipType = il2cpp_type_get_object(il2cpp_class_get_type(clipClass));
    bool includeInactive = true;
    void* parameters[] = { clipType, &includeInactive };
    auto* clips = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (clips == nullptr)
    {
        return nullptr;
    }

    const auto count = il2cpp_array_length(clips);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(clips) + il2cpp_array_object_header_size());
    Il2CppObject* fallback = nullptr;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* clip = elements[index];
        if (clip == nullptr)
        {
            continue;
        }
        const auto name = GetObjectName(clip);
        if (index < 30)
        {
            LogInfo(LogCategory_Animation, L"Animation", L"Loaded AnimationClip[%u]: %s", index, name.c_str());
        }
        if (name.find(L"NpcIdleFemale") != std::wstring::npos ||
            name.find(L"Female_Idle_Base") != std::wstring::npos)
        {
            return clip;
        }
        if (fallback == nullptr &&
            name.find(L"Idle") != std::wstring::npos &&
            name.find(L"Progress") == std::wstring::npos)
        {
            fallback = clip;
        }
    }
    return fallback;
}

void EscapeFromManaka_UnloadPreparingScene()
{
    if (g_efmPreparingScene == nullptr)
    {
        return;
    }
    auto* sceneManager = FindClass(
        "UnityEngine.SceneManagement",
        "SceneManager");
    void* parameters[] = { il2cpp_object_unbox(g_efmPreparingScene) };
    Invoke(
        FindMethod(sceneManager, "UnloadSceneAsync", 1),
        nullptr,
        parameters);
    g_efmPreparingScene = nullptr;
    LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing unloaded the source gameplay scene.");
}

Il2CppObject* FindEscapeFromManakaUiPlayerPrefab()
{
    auto* assetReferencerClass = FindClass(
        "ExposureUnnoticed2.Object3D.IngameManager",
        "AssetReferencer");
    if (assetReferencerClass == nullptr ||
        il2cpp_runtime_class_init == nullptr ||
        il2cpp_class_get_field_from_name == nullptr ||
        il2cpp_field_static_get_value == nullptr)
    {
        return nullptr;
    }

    il2cpp_runtime_class_init(assetReferencerClass);
    auto* instanceField = il2cpp_class_get_field_from_name(
        assetReferencerClass,
        "Instance");
    Il2CppObject* assetReferencer = nullptr;
    if (instanceField != nullptr)
    {
        il2cpp_field_static_get_value(instanceField, &assetReferencer);
    }
    if (assetReferencer == nullptr)
    {
        return nullptr;
    }

    // Prefer the original gameplay player prefab; fall back to UiPlayer.
    auto* playerPrefab = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(assetReferencer) + 0x348);
    if (playerPrefab != nullptr)
    {
        return playerPrefab;
    }
    return *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(assetReferencer) + 0x350);
}

const Il2CppMethod* FindMethodByRva(
    Il2CppClass* klass,
    const char* name,
    int parameterCount,
    uintptr_t rva)
{
    if (klass == nullptr)
    {
        return nullptr;
    }

    const auto gameAssembly = GetModuleHandleW(L"GameAssembly.dll");
    const auto expected = reinterpret_cast<uintptr_t>(gameAssembly) + rva;
    void* iterator = nullptr;
    while (const auto method = il2cpp_class_get_methods(klass, &iterator))
    {
        const auto* methodName = il2cpp_method_get_name(method);
        if (methodName != nullptr &&
            std::strcmp(methodName, name) == 0 &&
            il2cpp_method_get_param_count(method) == parameterCount &&
            reinterpret_cast<uintptr_t>(MethodPointer(method)) == expected)
        {
            return method;
        }
    }
    return nullptr;
}

Il2CppClass* FindRuntimeGameStateClass()
{
    auto* gameAssembly = reinterpret_cast<char*>(
        GetModuleHandleW(L"GameAssembly.dll"));
    if (gameAssembly != nullptr)
    {
        auto* typeInfoSlot = reinterpret_cast<Il2CppClass**>(
            gameAssembly + 0x2D58398);
        if (*typeInfoSlot != nullptr)
        {
            return *typeInfoSlot;
        }
    }
    return FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameState");
}

Il2CppObject* EscapeFromManaka_LoadClosetSaveData()
{
    auto* gameStateClass = FindRuntimeGameStateClass();
    auto* playerCustomField = gameStateClass != nullptr
        ? il2cpp_class_get_field_from_name(gameStateClass, "PlayerCustomData")
        : nullptr;
    Il2CppObject* playerCustomData = nullptr;
    if (playerCustomField != nullptr)
    {
        il2cpp_field_static_get_value(
            playerCustomField,
            &playerCustomData);
    }
    LogInfo(LogCategory_Animation, L"Animation", L"Initial GameState.PlayerCustomData: %p", playerCustomData);

    auto* playerCustomClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "PlayerCustomData");
    auto* saveRapperClass = FindClass(
        "ExposureUnnoticed2.Scripts.Base",
        "SaveRapper");
    auto* es3Class = FindClass("", "ES3");
    if (playerCustomClass != nullptr &&
        saveRapperClass != nullptr &&
        es3Class != nullptr &&
        il2cpp_object_new != nullptr &&
        il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(playerCustomClass);
        il2cpp_runtime_class_init(saveRapperClass);
        il2cpp_runtime_class_init(es3Class);
        auto* defaults = il2cpp_object_new(playerCustomClass);
        if (defaults != nullptr)
        {
            Invoke(
                FindMethod(playerCustomClass, ".ctor", 0),
                defaults,
                nullptr);

            const int saveType = 3;
            const int slot = 0;
            void* pathParameters[] = {
                const_cast<int*>(&saveType),
                const_cast<int*>(&slot)
            };
            auto* savePath = Invoke(
                FindMethod(saveRapperClass, "GetSpecifiedSaveDataPath", 2),
                nullptr,
                pathParameters);
            LogInfo(LogCategory_Animation, L"Animation", 
                L"Closet save path: %s",
                ReadIl2CppString(static_cast<Il2CppString*>(savePath)).c_str());

            auto* loadDefault = FindMethodByRva(
                es3Class,
                "Load",
                3,
                0x3BD610);
            auto* key = il2cpp_string_new("PlayerCustomData");
            void* loadParameters[] = {
                key,
                savePath,
                defaults
            };
            auto* loaded = loadDefault != nullptr
                ? Invoke(loadDefault, nullptr, loadParameters)
                : nullptr;
            LogInfo(LogCategory_Animation, L"Animation", 
                L"Selective closet load: methodInfo=%p method=%p defaults=%p result=%p.",
                loadDefault,
                MethodPointer(loadDefault),
                defaults,
                loaded);
            if (loaded != nullptr && playerCustomField != nullptr)
            {
                playerCustomData = loaded;
                il2cpp_field_static_set_value =
                    reinterpret_cast<FieldStaticSetValueFn>(
                        GetProcAddress(
                            GetModuleHandleW(L"GameAssembly.dll"),
                            "il2cpp_field_static_set_value"));
                if (il2cpp_field_static_set_value != nullptr)
                {
                    auto* staticFields = *reinterpret_cast<char**>(
                        reinterpret_cast<char*>(gameStateClass) + 0xB8);
                    const auto customOffset =
                        il2cpp_field_get_offset(playerCustomField);
                    il2cpp_field_static_set_value(
                        playerCustomField,
                        &playerCustomData);
                    if (staticFields != nullptr)
                    {
                        *reinterpret_cast<Il2CppObject**>(
                            staticFields + customOffset) =
                            playerCustomData;
                    }
                    LogInfo(LogCategory_Animation, L"Animation", 
                        L"PlayerCustomData assigned to GameState: %p raw Slot=%p value=%p.",
                        playerCustomData,
                        staticFields != nullptr
                            ? staticFields + customOffset
                            : nullptr,
                        staticFields != nullptr
                            ? *reinterpret_cast<Il2CppObject**>(
                                staticFields + customOffset)
                            : nullptr);
                    if (il2cpp_gchandle_new != nullptr)
                    {
                        if (g_efmSavedPlayerCustomDataHandle != 0 &&
                            il2cpp_gchandle_free != nullptr)
                        {
                            il2cpp_gchandle_free(
                                g_efmSavedPlayerCustomDataHandle);
                        }
                        g_efmSavedPlayerCustomDataHandle =
                            il2cpp_gchandle_new(playerCustomData, false);
                        LogInfo(LogCategory_Animation, L"Animation", 
                            L"PlayerCustomData strong root: handle=%u",
                            g_efmSavedPlayerCustomDataHandle);
                    }
                }
                else
                {
                    LogInfo(LogCategory_Animation, L"Animation", L"Selective closet load could not assign GameState.PlayerCustomData.");
                }
            }

        }
    }

    LogInfo(LogCategory_Animation, L"Animation", L"Loaded GameState.PlayerCustomData: %p", playerCustomData);
    if (playerCustomData == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"Selective closet load unavailable; keeping the prefab appearance.");
        return nullptr;
    }

    LogInfo(LogCategory_Animation, L"Animation", L"Closet customization object loaded; reading saved fields.");
    struct SavedField
    {
        const char* name;
        size_t offset;
    };
    const SavedField fields[] = {
        { "CustomizeHeight", 0x10 },
        { "CustomizeHead", 0x14 },
        { "CustomizeNeckThick", 0x1C },
        { "CustomizeBoobs", 0x20 },
        { "CustomizeWaist", 0x34 },
        { "CustomizeThighs", 0x3C },
        { "CustomizeArm", 0x5C },
        { "CustomizeShoulder", 0x60 },
        { "CurrentHair", 0x104 }
    };
    for (const auto& field : fields)
    {
        const int value = *reinterpret_cast<int*>(
            reinterpret_cast<char*>(playerCustomData) + field.offset);
        LogInfo(LogCategory_Animation, L"Animation", L"Closet save %S=%d", field.name, value);
    }
    LogInfo(LogCategory_Animation, L"Animation", L"Closet customization fields read successfully.");
    return playerCustomData;
}

bool SetManagedObjectField(
    Il2CppClass* klass,
    Il2CppObject* instance,
    const char* fieldName,
    Il2CppObject* value)
{
    if (klass == nullptr ||
        instance == nullptr ||
        il2cpp_class_get_field_from_name == nullptr ||
        il2cpp_field_get_offset == nullptr ||
        il2cpp_gc_wbarrier_set_field == nullptr)
    {
        return false;
    }
    auto* field = il2cpp_class_get_field_from_name(klass, fieldName);
    if (field == nullptr)
    {
        return false;
    }
    auto** target = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(instance) +
        il2cpp_field_get_offset(field));
    il2cpp_gc_wbarrier_set_field(
        instance,
        reinterpret_cast<void**>(target),
        value);
    return true;
}

void EscapeFromManaka_PlayerTitChangeDetour(
    Il2CppObject* instance,
    Il2CppObject* evt,
    const void* method)
{
    if (g_efmSkipPlayerTitChange.load())
    {
        return;
    }
    if (g_originalPlayerTitChange != nullptr)
    {
        reinterpret_cast<PlayerTitChangeFn>(
            g_originalPlayerTitChange)(instance, evt, method);
    }
}

bool EnsureEscapeFromManakaPlayerTitHook()
{
    if (g_originalPlayerTitChange != nullptr)
    {
        return true;
    }
    auto* gameAssembly = reinterpret_cast<char*>(
        GetModuleHandleW(L"GameAssembly.dll"));
    if (gameAssembly == nullptr)
    {
        return false;
    }
    return MH_CreateHook(
        gameAssembly + 0x663270,
        reinterpret_cast<LPVOID>(
            &EscapeFromManaka_PlayerTitChangeDetour),
        &g_originalPlayerTitChange) == MH_OK &&
        MH_EnableHook(gameAssembly + 0x663270) == MH_OK;
}

void EscapeFromManaka_Il2CppNullDetour()
{
    if (g_efmTraceBodyNull.load())
    {
        CONTEXT context{};
        context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
        RtlCaptureContext(&context);
        const auto caller = reinterpret_cast<uintptr_t>(
            __builtin_return_address(0));
        const auto gameAssembly = reinterpret_cast<uintptr_t>(
            GetModuleHandleW(L"GameAssembly.dll"));
        LogInfo(LogCategory_Animation, L"Animation", 
            L"Saved closet null check caller RVA: 0x%llX rcx=%p rdx=%p r8=%p r9=%p.",
            static_cast<unsigned long long>(caller - gameAssembly),
            reinterpret_cast<void*>(context.Rcx),
            reinterpret_cast<void*>(context.Rdx),
            reinterpret_cast<void*>(context.R8),
            reinterpret_cast<void*>(context.R9));
    }
    if (g_originalIl2CppThrowNull != nullptr)
    {
        reinterpret_cast<Il2CppThrowFn>(
            g_originalIl2CppThrowNull)();
    }
}

bool EnsureEscapeFromManakaNullTraceHook()
{
    if (g_originalIl2CppThrowNull != nullptr)
    {
        return true;
    }
    auto* gameAssembly = reinterpret_cast<char*>(
        GetModuleHandleW(L"GameAssembly.dll"));
    if (gameAssembly == nullptr)
    {
        return false;
    }
    return MH_CreateHook(
        gameAssembly + 0x31DAD0,
        reinterpret_cast<LPVOID>(
            &EscapeFromManaka_Il2CppNullDetour),
        &g_originalIl2CppThrowNull) == MH_OK &&
        MH_EnableHook(gameAssembly + 0x31DAD0) == MH_OK;
}

void EscapeFromManaka_ApplyClosetHairCustomization()
{
    auto* gameStateClass = FindRuntimeGameStateClass();
    auto* playerCustomField = gameStateClass != nullptr
        ? il2cpp_class_get_field_from_name(
            gameStateClass,
            "PlayerCustomData")
        : nullptr;
    Il2CppObject* playerCustomData = nullptr;
    if (g_efmSavedPlayerCustomDataHandle != 0 &&
        il2cpp_gchandle_get_target != nullptr)
    {
        playerCustomData = il2cpp_gchandle_get_target(
            g_efmSavedPlayerCustomDataHandle);
    }
    else if (playerCustomField != nullptr)
    {
        il2cpp_field_static_get_value(
            playerCustomField,
            &playerCustomData);
    }
    if (playerCustomData == nullptr || g_previewModel == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", 
            L"Closet hair customization skipped: data=%p model=%p.",
            playerCustomData,
            g_previewModel);
        return;
    }
    if (playerCustomField != nullptr &&
        il2cpp_field_static_set_value != nullptr)
    {
        il2cpp_field_static_set_value(
            playerCustomField,
            &playerCustomData);
    }

    auto* avatarReferencer = FindFirstComponentInChildrenByClass(
        g_previewModel,
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAvatarObjectReferencer");
    if (avatarReferencer == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"Closet hair customization skipped: avatar referencer is unavailable.");
        return;
    }

    auto* hairs = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(avatarReferencer) + 0x280);
    const int savedHair = *reinterpret_cast<int*>(
        reinterpret_cast<char*>(playerCustomData) + 0x104);
    if (hairs != nullptr && il2cpp_array_length != nullptr)
    {
        const auto hairCount = il2cpp_array_length(hairs);
        auto* hairObjects = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(hairs) +
            il2cpp_array_object_header_size());
        for (uint32_t index = 0; index < hairCount; ++index)
        {
            if (hairObjects[index] != nullptr)
            {
                SetWorldGameObjectActive(hairObjects[index], false);
            }
        }

        int appliedHair = savedHair;
        if (appliedHair < 0 ||
            static_cast<uint32_t>(appliedHair) >= hairCount ||
            hairObjects[appliedHair] == nullptr)
        {
            appliedHair = -1;
            for (uint32_t index = 0; index < hairCount; ++index)
            {
                if (hairObjects[index] != nullptr)
                {
                    appliedHair = static_cast<int>(index);
                    break;
                }
            }
        }

        if (appliedHair >= 0)
        {
            auto* selectedHair = hairObjects[appliedHair];
            SetWorldGameObjectActive(selectedHair, true);

            auto* rendererClass = FindClass("UnityEngine", "Renderer");
            auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
            if (rendererClass != nullptr &&
                gameObjectClass != nullptr &&
                il2cpp_type_get_object != nullptr)
            {
                il2cpp_runtime_class_init(rendererClass);
                auto* rendererType = il2cpp_type_get_object(
                    il2cpp_class_get_type(rendererClass));
                bool includeInactive = true;
                void* rendererParameters[] = {
                    rendererType,
                    &includeInactive
                };
                auto* renderers = Invoke(
                    FindMethod(
                        gameObjectClass,
                        "GetComponentsInChildren",
                        2),
                    selectedHair,
                    rendererParameters);
                if (renderers != nullptr)
                {
                    bool rendererEnabled = true;
                    const auto rendererCount =
                        il2cpp_array_length(renderers);
                    auto* rendererElements =
                        reinterpret_cast<Il2CppObject**>(
                            reinterpret_cast<char*>(renderers) +
                            il2cpp_array_object_header_size());
                    void* enabledParameters[] = { &rendererEnabled };
                    for (uint32_t index = 0;
                         index < rendererCount;
                         ++index)
                    {
                        if (rendererElements[index] == nullptr)
                        {
                            continue;
                        }
                        Invoke(
                            FindMethod(
                                rendererClass,
                                "set_enabled",
                                1),
                            rendererElements[index],
                            enabledParameters);
                    }
                }
            }
        }

        LogInfo(LogCategory_Animation, L"Animation", 
            L"Closet hair index requested=%d applied=%d across %u object(s).",
            savedHair,
            appliedHair,
            hairCount);
    }

    auto* hairCustomClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts.Other",
        "HairCustomAdjuster");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (hairCustomClass != nullptr &&
        gameObjectClass != nullptr &&
        il2cpp_class_get_type != nullptr &&
        il2cpp_type_get_object != nullptr)
    {
        il2cpp_runtime_class_init(hairCustomClass);
        auto* managedType = il2cpp_type_get_object(
            il2cpp_class_get_type(hairCustomClass));
        bool includeInactive = true;
        void* findParameters[] = { managedType, &includeInactive };
        auto* components = Invoke(
            FindMethod(gameObjectClass, "GetComponentsInChildren", 2),
            g_previewModel,
            findParameters);
        if (components != nullptr)
        {
            const auto count = il2cpp_array_length(components);
            auto* elements = reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(components) +
                il2cpp_array_object_header_size());
            auto* onChangeBody = FindMethod(
                hairCustomClass,
                "OnChangeBody",
                1);
            if (onChangeBody == nullptr)
            {
                LogInfo(LogCategory_Animation, L"Animation", L"Closet hair customization skipped: OnChangeBody is unavailable.");
                return;
            }
            int applied = 0;
            for (uint32_t index = 0; index < count; ++index)
            {
                if (elements[index] == nullptr)
                {
                    continue;
                }
                void* hairChangeParameters[] = { nullptr };
                Invoke(
                    onChangeBody,
                    elements[index],
                    hairChangeParameters);
                ++applied;
            }
            LogInfo(LogCategory_Animation, L"Animation", 
                L"Closet hair details applied through %d HairCustomAdjuster component(s).",
                applied);
        }
    }
}

void EscapeFromManaka_ApplyClosetCustomization()
{
    auto* gameStateClass = FindRuntimeGameStateClass();
    auto* playerCustomField = gameStateClass != nullptr
        ? il2cpp_class_get_field_from_name(gameStateClass, "PlayerCustomData")
        : nullptr;
    Il2CppObject* playerCustomData = nullptr;
    if (g_efmSavedPlayerCustomDataHandle != 0 &&
        il2cpp_gchandle_get_target != nullptr)
    {
        playerCustomData = il2cpp_gchandle_get_target(
            g_efmSavedPlayerCustomDataHandle);
    }
    else if (playerCustomField != nullptr)
    {
        il2cpp_field_static_get_value(playerCustomField, &playerCustomData);
    }
    if (playerCustomData != nullptr &&
        playerCustomField != nullptr &&
        il2cpp_field_static_set_value != nullptr)
    {
        il2cpp_field_static_set_value(
            playerCustomField,
            &playerCustomData);
    }
    if (playerCustomData == nullptr || g_previewModel == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"Saved closet customization skipped: data=%p model=%p.", playerCustomData, g_previewModel);
        return;
    }

    auto* avatarReferencer = FindFirstComponentInChildrenByClass(
        g_previewModel,
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAvatarObjectReferencer");
    auto* animationReceiver = FindFirstComponentInChildrenByClass(
        g_previewModel,
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerAnimationReceiver");
    auto* animator = g_previewAnimator != nullptr
        ? g_previewAnimator
        : FindFirstComponentInChildrenByClass(
            g_previewModel,
            "UnityEngine",
            "Animator");
    LogInfo(LogCategory_Animation, L"Animation", 
        L"Saved closet dependencies: avatarRef=%p animationReceiver=%p animator=%p.",
        avatarReferencer,
        animationReceiver,
        animator);
    if (avatarReferencer == nullptr || animationReceiver == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"Saved closet customization skipped: avatar components are unavailable.");
        return;
    }

    auto* pcaClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerClassAccessor");
    auto* bodyClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerBodyCustomizeAdjuster");
    if (pcaClass == nullptr ||
        bodyClass == nullptr ||
        il2cpp_object_new == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"Saved closet customization skipped: required classes are unavailable.");
        return;
    }
    il2cpp_runtime_class_init(pcaClass);
    il2cpp_runtime_class_init(bodyClass);

    auto* pca = il2cpp_object_new(pcaClass);
    if (il2cpp_gchandle_new != nullptr)
    {
        g_efmPreviewPlayerClassAccessorHandle =
            il2cpp_gchandle_new(pca, false);
    }
    auto* bodyAdjuster = il2cpp_object_new(bodyClass);
    if (il2cpp_gchandle_new != nullptr)
    {
        g_efmPreviewBodyAdjusterHandle =
            il2cpp_gchandle_new(bodyAdjuster, false);
    }
    LogInfo(LogCategory_Animation, L"Animation", L"Saved closet allocated accessor=%p bodyAdjuster=%p.", pca, bodyAdjuster);
    Invoke(FindMethod(bodyClass, ".ctor", 0), bodyAdjuster, nullptr);
    LogInfo(LogCategory_Animation, L"Animation", L"Saved closet body adjuster constructed.");

    auto* componentClass = FindClass("UnityEngine", "Component");
    auto* avatarTransform = Invoke(
        FindMethod(componentClass, "get_transform", 0),
        avatarReferencer,
        nullptr);
    auto* modelTransform = GetWorldTransform(g_previewModel);
    SetManagedObjectField(pcaClass, pca, "<Animator>k__BackingField", animator);
    SetManagedObjectField(pcaClass, pca, "<Transform>k__BackingField", modelTransform);
    SetManagedObjectField(pcaClass, pca, "<AvatorTransform>k__BackingField", avatarTransform);
    SetManagedObjectField(pcaClass, pca, "<GameObject>k__BackingField", g_previewModel);
    SetManagedObjectField(
        pcaClass,
        pca,
        "<PlayerAvatarObjectReferencer>k__BackingField",
        avatarReferencer);
    SetManagedObjectField(
        pcaClass,
        pca,
        "<PlayerAnimationReceiver>k__BackingField",
        animationReceiver);
    SetManagedObjectField(
        pcaClass,
        pca,
        "<PlayerBodyCustomizeAdjuster>k__BackingField",
        bodyAdjuster);
    LogInfo(LogCategory_Animation, L"Animation", L"Saved closet minimal accessor references assigned.");

    void* initializeParameters[] = { pca };
    Invoke(
        FindMethod(bodyClass, "Initialize", 1),
        bodyAdjuster,
        initializeParameters);
    LogInfo(LogCategory_Animation, L"Animation", L"Saved closet body adjuster initialized.");
    auto* bodyRenderer = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(avatarReferencer) + 0x208);
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    if (bodyRenderer != nullptr && rendererClass != nullptr)
    {
        auto* materials = Invoke(
            FindMethod(rendererClass, "get_materials", 0),
            bodyRenderer,
            nullptr);
        if (materials != nullptr)
        {
            const auto materialCount = il2cpp_array_length(materials);
            auto* materialElements = reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(materials) +
                il2cpp_array_object_header_size());
            LogInfo(LogCategory_Animation, L"Animation", L"Saved closet body renderer materials: %u", materialCount);
            for (uint32_t index = 0;
                 index < materialCount && index < 12;
                 ++index)
            {
                LogInfo(LogCategory_Animation, L"Animation", 
                    L"Saved closet body material[%u]: %p %s",
                    index,
                    materialElements[index],
                    materialElements[index] != nullptr
                        ? GetObjectName(materialElements[index]).c_str()
                        : L"<null>");
            }
        }
    }

    auto* titClass = FindClass(
        "ExposureUnnoticed2.Object3D.Player.Scripts",
        "PlayerTitController");
    if (titClass != nullptr)
    {
        il2cpp_runtime_class_init(titClass);
        auto* titController = il2cpp_object_new(titClass);
        Invoke(
            FindMethod(titClass, ".ctor", 0),
            titController,
            nullptr);
        SetManagedObjectField(
            pcaClass,
            pca,
            "<PlayerTitController>k__BackingField",
            titController);
        if (il2cpp_gchandle_new != nullptr)
        {
            g_efmPreviewTitControllerHandle =
                il2cpp_gchandle_new(titController, false);
        }
        LogInfo(LogCategory_Animation, L"Animation", L"Saved closet inert PlayerTitController: %p", titController);
    }
    if (il2cpp_gchandle_get_target != nullptr &&
        g_efmSavedPlayerCustomDataHandle != 0 &&
        playerCustomField != nullptr)
    {
        playerCustomData = il2cpp_gchandle_get_target(
            g_efmSavedPlayerCustomDataHandle);
        if (playerCustomData != nullptr &&
            il2cpp_field_static_set_value != nullptr)
        {
            il2cpp_field_static_set_value(
                playerCustomField,
                &playerCustomData);
            auto* staticFields = *reinterpret_cast<char**>(
                reinterpret_cast<char*>(gameStateClass) + 0xB8);
            if (staticFields != nullptr)
            {
                *reinterpret_cast<Il2CppObject**>(
                    staticFields +
                    il2cpp_field_get_offset(playerCustomField)) =
                    playerCustomData;
            }
        }
        Il2CppObject* readback = nullptr;
        il2cpp_field_static_get_value(playerCustomField, &readback);
        LogInfo(LogCategory_Animation, L"Animation", 
            L"Saved closet refreshed PlayerCustomData: root=%p readback=%p.",
            playerCustomData,
            readback);
    }
    auto* colorClass = *reinterpret_cast<Il2CppClass**>(
        reinterpret_cast<char*>(GetModuleHandleW(L"GameAssembly.dll")) +
        0x2D5C1C8);
    if (colorClass == nullptr)
    {
        colorClass = FindClass(
            "ExposureUnnoticed2.Master.Common",
            "MCustomizeColor");
    }
    if (colorClass != nullptr)
    {
        il2cpp_runtime_class_init(colorClass);
        auto* colorInstance = Invoke(
            FindMethod(colorClass, "get_Instance", 0),
            nullptr,
            nullptr);
        if (colorInstance == nullptr)
        {
            auto* resourcesClass = FindClass("UnityEngine", "Resources");
            auto* colorType = il2cpp_type_get_object(
                il2cpp_class_get_type(colorClass));
            void* assetParameters[] = { colorType };
            auto* assets = Invoke(
                FindMethod(resourcesClass, "FindObjectsOfTypeAll", 1),
                nullptr,
                assetParameters);
            if (assets != nullptr && il2cpp_array_length(assets) > 0)
            {
                auto* assetElements = reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<char*>(assets) +
                    il2cpp_array_object_header_size());
                colorInstance = assetElements[0];
                auto* instanceField = il2cpp_class_get_field_from_name(
                    colorClass,
                    "instance");
                if (instanceField != nullptr &&
                    il2cpp_field_static_set_value != nullptr)
                {
                    il2cpp_field_static_set_value(
                        instanceField,
                        &colorInstance);
                    auto* staticFields = *reinterpret_cast<char**>(
                        reinterpret_cast<char*>(colorClass) + 0xB8);
                    if (staticFields != nullptr)
                    {
                        *reinterpret_cast<Il2CppObject**>(staticFields) =
                            colorInstance;
                    }
                }
            }
        }
        LogInfo(LogCategory_Animation, L"Animation", 
            L"Saved closet MCustomizeColor instance: %p",
            colorInstance);
    }
    const bool titHookReady = EnsureEscapeFromManakaPlayerTitHook();
    LogInfo(LogCategory_Animation, L"Animation", L"Saved closet PlayerTit bypass hook ready: %s.", titHookReady ? L"yes" : L"no");
    const bool nullTraceReady = EnsureEscapeFromManakaNullTraceHook();
    const auto diagnostics = AddVectoredExceptionHandler(
        1,
        [](PEXCEPTION_POINTERS pointers) -> LONG
        {
            if (pointers == nullptr ||
                pointers->ExceptionRecord == nullptr ||
                pointers->ExceptionRecord->ExceptionCode !=
                    EXCEPTION_ACCESS_VIOLATION)
            {
                return EXCEPTION_CONTINUE_SEARCH;
            }
            const auto base = reinterpret_cast<uintptr_t>(
                GetModuleHandleW(L"GameAssembly.dll"));
            const auto address = reinterpret_cast<uintptr_t>(
                pointers->ExceptionRecord->ExceptionAddress);
            if (base != 0 && address >= base && address < base + 0x4000000)
            {
                const auto* context = pointers->ContextRecord;
                LogInfo(LogCategory_Animation, L"Animation", 
                    L"Saved closet body AV: rva=0x%llX rcx=%p rdx=0x%llX r8=%p r9=%p.",
                    static_cast<unsigned long long>(address - base),
                    reinterpret_cast<void*>(context->Rcx),
                    static_cast<unsigned long long>(context->Rdx),
                    reinterpret_cast<void*>(context->R8),
                    reinterpret_cast<void*>(context->R9));
                const auto* stack =
                    reinterpret_cast<const uintptr_t*>(context->Rsp);
                for (size_t index = 0; index < 0x100; ++index)
                {
                    const auto candidate = stack[index];
                    if (candidate >= base && candidate < base + 0x4000000)
                    {
                        LogInfo(LogCategory_Animation, L"Animation", 
                            L"Saved closet body stack[%u]: rva=0x%llX",
                            static_cast<unsigned>(index),
                            static_cast<unsigned long long>(candidate - base));
                    }
                }
            }
            return EXCEPTION_CONTINUE_SEARCH;
        });
    void* bodyChangeParameters[] = { nullptr };
    g_efmSkipPlayerTitChange = titHookReady;
    g_efmTraceBodyNull = nullTraceReady;
    Invoke(
        FindMethod(bodyClass, "OnChangeBody", 1),
        bodyAdjuster,
        bodyChangeParameters);
    g_efmTraceBodyNull = false;
    g_efmSkipPlayerTitChange = false;
    if (diagnostics != nullptr)
    {
        RemoveVectoredExceptionHandler(diagnostics);
    }
    LogInfo(LogCategory_Animation, L"Animation", L"Saved body proportions applied with a minimal PlayerClassAccessor.");

    EscapeFromManaka_ApplyClosetHairCustomization();
}

void EscapeFromManaka_preparing()
{
    if (g_efmPreparingState.load() != 0)
    {
        return;
    }
    g_efmPreparingState = 1;
    g_efmPreparingFrames = 0;
    LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing started.");

    EscapeFromManaka_LoadClosetSaveData();

    if (g_efmLoadoutPanelTransform == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing has no loadout panel.");
        g_efmPreparingState = 4;
        return;
    }

    auto* uiPlayerPrefab = FindEscapeFromManakaUiPlayerPrefab();
    if (uiPlayerPrefab == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing could not resolve AssetReferencer.UiPlayer.");
        g_efmPreparingState = 4;
        return;
    }

    LogInfo(LogCategory_Animation, L"Animation", 
        L"EscapeFromManaka_preparing resolved player prefab %s with %d renderer(s).",
        GetObjectName(uiPlayerPrefab).c_str(),
        CountComponentsInChildren(
            uiPlayerPrefab,
            "UnityEngine",
            "SkinnedMeshRenderer"));

    void* instantiateParameters[] = { uiPlayerPrefab };
    auto* modelClone = Invoke(
        FindMethod(FindClass("UnityEngine", "Object"), "Instantiate", 1),
        nullptr,
        instantiateParameters);
    if (modelClone == nullptr)
    {
        LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing UiPlayer instantiation failed.");
        g_efmPreparingState = 4;
        return;
    }

    g_efmModelDragging = false;
    g_efmModelYaw = 0.0f;
    g_efmPreviewController = FindLoadedAssetByName(
        "UnityEngine",
        "RuntimeAnimatorController",
        L"Player");
    if (g_efmPreviewController == nullptr)
    {
        g_efmPreviewController = FindPreviewAnimatorController(modelClone);
    }
    if (g_efmPreviewController == nullptr)
    {
        g_efmPreviewController = FindLoadedEscapeAnimatorController();
    }
    g_efmPreviewIdleClip = FindLoadedEscapeIdleClip();
    BuildCharacterPreview(g_efmLoadoutPanelTransform, modelClone);
    EscapeFromManaka_ApplyClosetHairCustomization();
    // Do not call PlayerBodyCustomizeAdjuster.OnChangeBody outside the full
    // gameplay object graph. It requires live master data and event lifecycle.
    // EscapeFromManaka_ApplyClosetCustomization();
    g_efmSelectedAction = 0;
    LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing created the model without loading gameplay.");
    g_efmPreparingState = 2;
}

void EscapeFromManaka_TickPreparing()
{
    const int state = g_efmPreparingState.load();
    if (state == 2)
    {
        g_efmPreparingState = 3;
        LogInfo(LogCategory_Animation, L"Animation", L"EscapeFromManaka_preparing completed.");
    }
}

