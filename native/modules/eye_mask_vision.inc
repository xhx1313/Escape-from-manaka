// Native.cpp module: eye_mask_vision.inc
// Maps one signed vision value to the original eye-mask distance fog.

constexpr float kEyeMaskVisionMinimum = -225.0f;
constexpr float kEyeMaskVisionMaximum = 225.0f;
constexpr float kEyeMaskVisionDensityAtOneMeter = 1.7308184f;
constexpr float kEyeMaskVisionDefaultValue = 0.0f;
constexpr float kEyeMaskVisionVignetteResponsePower = 6.0f;
constexpr unsigned long long kEyeMaskVisionConfigPollMs = 500;

struct EyeMaskVisionSettings
{
    bool enabled = true;
    bool useConfigValue = true;
    float value = kEyeMaskVisionDefaultValue;
    float minDistance = 1.0f;
    float maxDistance = 5.5f;
    float responseSeconds = 0.20f;
    float vignetteAtMinDistance = 1.0f;
    float vignetteAtMaxDistance = 0.5f;
};

EyeMaskVisionSettings g_eyeMaskVisionSettings;
std::atomic<float> g_eyeMaskVisionExternalValue{
    kEyeMaskVisionDefaultValue};
std::atomic<bool> g_eyeMaskVisionUseExternalValue{false};
bool g_eyeMaskVisionForceThirdPersonOnRemoval = false;
bool g_eyeMaskVisionPreviousActive = false;
bool g_eyeMaskVisionPreviousViewWasFirstPerson = false;
bool g_eyeMaskVisionPreviousViewCaptured = false;

std::wstring EyeMaskVision_GetConfigPath()
{
    return GetEscapeFromManakaSubdirectory(L"Config") +
        L"\\eye_mask_vision.json";
}

bool EyeMaskVision_WriteUtf8File(
    const std::wstring& path,
    const std::string& content)
{
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
        return false;
    }
    DWORD written = 0;
    const bool ok =
        content.size() <= MAXDWORD &&
        WriteFile(
            file,
            content.data(),
            static_cast<DWORD>(content.size()),
            &written,
            nullptr) != FALSE &&
        written == content.size();
    CloseHandle(file);
    return ok;
}

void EyeMaskVision_CreateDefaultConfig(const std::wstring& path)
{
    const nlohmann::json root = {
        { "schemaVersion", 2 },
        { "enabled", true },
        { "useConfigValue", true },
        { "value", kEyeMaskVisionDefaultValue },
        { "valueRange", { -225.0f, 225.0f } },
        { "distanceMeters", { 1.0f, 5.5f } },
        { "responseSeconds", 0.20f },
        { "vignette", {
            { "atMinDistance", 1.0f },
            { "atMaxDistance", 0.5f },
        } },
    };
    if (!EyeMaskVision_WriteUtf8File(path, root.dump(2) + "\n"))
    {
        LogInfo(LogCategory_Action, L"Action", L"Eye-mask vision could not create its default config.");
    }
    else
    {
        LogInfo(LogCategory_Action, L"Action", 
            L"Eye-mask vision created default config: %s",
            path.c_str());
    }
}

void EyeMaskVision_LoadConfig()
{
    const std::wstring path = EyeMaskVision_GetConfigPath();
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        EyeMaskVision_CreateDefaultConfig(path);
    }

    const std::string content = CheckpointSystem_ReadUtf8File(path);
    if (content.empty())
    {
        return;
    }

    try
    {
        const auto root = nlohmann::json::parse(content);
        if (!root.is_object())
        {
            return;
        }
        const int schemaVersion = root.value("schemaVersion", 1);
        EyeMaskVisionSettings settings = g_eyeMaskVisionSettings;
        settings.enabled = root.value(
            "enabled",
            settings.enabled);
        settings.useConfigValue = root.value(
            "useConfigValue",
            settings.useConfigValue);
        settings.value = root.value(
            "value",
            settings.value);
        if (root.contains("valueRange") &&
            root["valueRange"].is_array() &&
            root["valueRange"].size() == 2)
        {
            settings.value = std::clamp(
                settings.value,
                root["valueRange"][0].get<float>(),
                root["valueRange"][1].get<float>());
        }
        if (root.contains("distanceMeters") &&
            root["distanceMeters"].is_array() &&
            root["distanceMeters"].size() == 2)
        {
            settings.minDistance =
                root["distanceMeters"][0].get<float>();
            settings.maxDistance =
                root["distanceMeters"][1].get<float>();
        }
        settings.responseSeconds = root.value(
            "responseSeconds",
            settings.responseSeconds);
        if (root.contains("vignette") &&
            root["vignette"].is_object())
        {
            settings.vignetteAtMinDistance =
                root["vignette"].value(
                    "atMinDistance",
                    settings.vignetteAtMinDistance);
            settings.vignetteAtMaxDistance =
                root["vignette"].value(
                    "atMaxDistance",
                    settings.vignetteAtMaxDistance);
        }
        settings.minDistance = std::max(
            settings.minDistance,
            0.10f);
        settings.maxDistance = std::max(
            settings.maxDistance,
            settings.minDistance);
        settings.responseSeconds = std::max(
            settings.responseSeconds,
            0.01f);
        settings.vignetteAtMinDistance = std::clamp(
            settings.vignetteAtMinDistance,
            0.0f,
            1.0f);
        settings.vignetteAtMaxDistance = std::clamp(
            settings.vignetteAtMaxDistance,
            0.0f,
            1.0f);
        if (schemaVersion < 2)
        {
            settings.minDistance = 1.0f;
            settings.maxDistance = 5.5f;
            settings.vignetteAtMinDistance = 1.0f;
            settings.vignetteAtMaxDistance = 0.5f;
        }
        g_eyeMaskVisionSettings = settings;
        if (schemaVersion < 2)
        {
            const nlohmann::json migrated = {
                { "schemaVersion", 2 },
                { "enabled", settings.enabled },
                { "useConfigValue", settings.useConfigValue },
                { "value", settings.value },
                { "valueRange", { -225.0f, 225.0f } },
                { "distanceMeters", { 1.0f, 5.5f } },
                { "responseSeconds", settings.responseSeconds },
                { "vignette", {
                    { "atMinDistance", 1.0f },
                    { "atMaxDistance", 0.5f },
                } },
            };
            EyeMaskVision_WriteUtf8File(
                path,
                migrated.dump(2) + "\n");
            LogInfo(LogCategory_Action, L"Action", L"Eye-mask vision config migrated to schema version 2.");
        }
    }
    catch (const std::exception& exception)
    {
        static unsigned long long lastErrorLogAt = 0;
        const unsigned long long now = GetTickCount64();
        if (now - lastErrorLogAt >= 5000)
        {
            lastErrorLogAt = now;
            LogInfo(LogCategory_Action, L"Action", 
                L"Eye-mask vision JSON parse failed: %S",
                exception.what());
        }
    }
}

float EyeMaskVision_GetValue()
{
    float baseValue = 0.0f;
    if (g_eyeMaskVisionUseExternalValue.load())
    {
        baseValue = g_eyeMaskVisionExternalValue.load();
    }
    else
    {
        baseValue = g_eyeMaskVisionSettings.value;
    }
    return std::clamp(
        baseValue +
            static_cast<float>(EquipmentStats_GetEffectivePerception()),
        kEyeMaskVisionMinimum,
        kEyeMaskVisionMaximum);
}

void EyeMaskVision_SetValue(float value)
{
    g_eyeMaskVisionExternalValue =
        std::clamp(
            value,
            kEyeMaskVisionMinimum,
            kEyeMaskVisionMaximum);
    g_eyeMaskVisionUseExternalValue = true;
}

void EyeMaskVision_UseConfigValue()
{
    g_eyeMaskVisionUseExternalValue = false;
}

float EyeMaskVision_ComputeTargetDistance(float value)
{
    const float t = std::clamp(
        (value - kEyeMaskVisionMinimum) /
            (kEyeMaskVisionMaximum - kEyeMaskVisionMinimum),
        0.0f,
        1.0f);
    return g_eyeMaskVisionSettings.minDistance +
        (g_eyeMaskVisionSettings.maxDistance -
            g_eyeMaskVisionSettings.minDistance) * t;
}

bool EyeMaskVision_IsEyeMaskActive()
{
    auto* playerClassAccessor =
        g_extendedPlayerController != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
            : nullptr;
    auto* adultGoodsController =
        playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) + 0x100)
            : nullptr;
    if (adultGoodsController == nullptr)
    {
        return false;
    }

    static const Il2CppMethod* isActiveGetter = nullptr;
    if (isActiveGetter == nullptr)
    {
        isActiveGetter = FindMethod(
            FindClass(
                "ExposureUnnoticed2.Object3D.Player.Scripts",
                "PlayerAdultGoodsController"),
            "get_IsActiveEyeMask",
            0);
    }
    auto* result = Invoke(
        isActiveGetter,
        adultGoodsController,
        nullptr);
    return result != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

bool EyeMaskVision_IsPlayerFirstPerson()
{
    auto* playerClassAccessor =
        g_extendedPlayerController != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
            : nullptr;
    auto* playerState =
        playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) + 0x60)
            : nullptr;
    return playerState != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(playerState) + 0x1A);
}

void EyeMaskVision_SetPlayerFirstPerson(bool firstPerson)
{
    auto* playerClassAccessor =
        g_extendedPlayerController != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedPlayerController) + 0x40)
            : nullptr;
    auto* playerState =
        playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) + 0x60)
            : nullptr;
    if (playerState == nullptr)
    {
        return;
    }
    *reinterpret_cast<bool*>(
        reinterpret_cast<char*>(playerState) + 0x1A) =
        firstPerson;
    LogInfo(LogCategory_Action, L"Action", 
        L"Eye-mask camera state set to %s.",
        firstPerson ? L"first-person" : L"third-person");
}

void EyeMaskVision_ForceThirdPersonAfterRemoval()
{
    g_eyeMaskVisionForceThirdPersonOnRemoval = true;
}

void EyeMaskVision_ResetCameraTracking()
{
    g_eyeMaskVisionForceThirdPersonOnRemoval = false;
    g_eyeMaskVisionPreviousActive = false;
    g_eyeMaskVisionPreviousViewWasFirstPerson = false;
    g_eyeMaskVisionPreviousViewCaptured = false;
}

bool EyeMaskVision_ApplyMCommonFog(
    float nearDistance,
    float farDistance,
    float density)
{
    static Il2CppClass* mcommonClass = nullptr;
    static const Il2CppMethod* instanceGetter = nullptr;
    static Il2CppField* nearField = nullptr;
    static Il2CppField* farField = nullptr;
    static Il2CppField* densityField = nullptr;
    if (mcommonClass == nullptr)
    {
        mcommonClass = FindClass(
            "ExposureUnnoticed2.Master.Common",
            "MCommon");
        instanceGetter = FindMethod(
            mcommonClass,
            "get_Instance",
            0);
        nearField = il2cpp_class_get_field_from_name(
            mcommonClass,
            "EyeMaskFogNear");
        farField = il2cpp_class_get_field_from_name(
            mcommonClass,
            "EyeMaskFogFar");
        densityField = il2cpp_class_get_field_from_name(
            mcommonClass,
            "EyeMaskFogDensity");
    }
    if (instanceGetter == nullptr ||
        nearField == nullptr ||
        farField == nullptr ||
        densityField == nullptr ||
        il2cpp_field_get_offset == nullptr)
    {
        return false;
    }

    auto* instance = Invoke(
        instanceGetter,
        nullptr,
        nullptr);
    if (instance == nullptr)
    {
        return false;
    }

    auto* base = reinterpret_cast<char*>(instance);
    *reinterpret_cast<float*>(
        base + il2cpp_field_get_offset(nearField)) =
        nearDistance;
    *reinterpret_cast<float*>(
        base + il2cpp_field_get_offset(farField)) =
        farDistance;
    *reinterpret_cast<float*>(
        base + il2cpp_field_get_offset(densityField)) =
        density;
    return true;
}

void EyeMaskVision_ApplyRenderSettings(
    float nearDistance,
    float farDistance,
    float density)
{
    static const Il2CppMethod* setFog = nullptr;
    static const Il2CppMethod* setFogMode = nullptr;
    static const Il2CppMethod* setFogStart = nullptr;
    static const Il2CppMethod* setFogEnd = nullptr;
    static const Il2CppMethod* setFogDensity = nullptr;
    if (setFog == nullptr)
    {
        auto* renderSettingsClass =
            FindClass("UnityEngine", "RenderSettings");
        setFog = FindMethod(renderSettingsClass, "set_fog", 1);
        setFogMode = FindMethod(
            renderSettingsClass,
            "set_fogMode",
            1);
        setFogStart = FindMethod(
            renderSettingsClass,
            "set_fogStartDistance",
            1);
        setFogEnd = FindMethod(
            renderSettingsClass,
            "set_fogEndDistance",
            1);
        setFogDensity = FindMethod(
            renderSettingsClass,
            "set_fogDensity",
            1);
    }
    if (setFog == nullptr ||
        setFogMode == nullptr ||
        setFogStart == nullptr ||
        setFogEnd == nullptr ||
        setFogDensity == nullptr)
    {
        return;
    }

    bool enabled = true;
    int fogMode = 3; // ExponentialSquared
    float fogStart = nearDistance;
    float fogEnd = farDistance;
    float fogDensity = density;
    void* enabledParameters[] = { &enabled };
    void* modeParameters[] = { &fogMode };
    void* startParameters[] = { &fogStart };
    void* endParameters[] = { &fogEnd };
    void* densityParameters[] = { &fogDensity };
    Invoke(setFog, nullptr, enabledParameters);
    Invoke(setFogMode, nullptr, modeParameters);
    Invoke(setFogStart, nullptr, startParameters);
    Invoke(setFogEnd, nullptr, endParameters);
    Invoke(setFogDensity, nullptr, densityParameters);
}

void EyeMaskVision_ApplyVignette(float alpha)
{
    auto* blindVignette =
        g_extendedInGameUiManager != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedInGameUiManager) + 0xA8)
            : nullptr;
    if (blindVignette == nullptr)
    {
        return;
    }

    ExtendedMode_SetComponentEnabled(blindVignette, true);
    SetWorldGameObjectActive(
        GetComponentGameObject(blindVignette),
        true);

    static const Il2CppMethod* setColor = nullptr;
    if (setColor == nullptr)
    {
        auto* graphicClass =
            FindClass("UnityEngine.UI", "Graphic");
        setColor = FindMethod(graphicClass, "set_color", 1);
    }
    if (setColor == nullptr)
    {
        return;
    }

    // The serialized asset is pure white. Avoid the boxed get_color return
    // path that was introduced after the previously tested version; pass the
    // four Color components directly to Graphic.set_color.
    float color[] = {
        1.0f,
        1.0f,
        1.0f,
        std::clamp(alpha, 0.0f, 1.0f)
    };
    void* colorParameters[] = { color };
    Invoke(setColor, blindVignette, colorParameters);
}

void EyeMaskVision_ApplyAfterOriginalUpdate()
{
    const unsigned long long now = GetTickCount64();
    static unsigned long long nextConfigPollAt = 0;
    if (now >= nextConfigPollAt)
    {
        nextConfigPollAt = now + kEyeMaskVisionConfigPollMs;
        EyeMaskVision_LoadConfig();
    }

    const bool eyeMaskActive = EyeMaskVision_IsEyeMaskActive();
    if (eyeMaskActive != g_eyeMaskVisionPreviousActive)
    {
        if (eyeMaskActive)
        {
            g_eyeMaskVisionPreviousViewWasFirstPerson =
                EyeMaskVision_IsPlayerFirstPerson();
            g_eyeMaskVisionPreviousViewCaptured = true;
            EyeMaskVision_SetPlayerFirstPerson(true);
            LogInfo(LogCategory_Action, L"Action", 
                L"Eye-mask animation completed; saved previous view=%s and switched to first-person.",
                g_eyeMaskVisionPreviousViewWasFirstPerson
                    ? L"first-person"
                    : L"third-person");
        }
        else
        {
            const bool forceThirdPerson =
                g_eyeMaskVisionForceThirdPersonOnRemoval;
            if (forceThirdPerson ||
                (g_eyeMaskVisionPreviousViewCaptured &&
                 !g_eyeMaskVisionPreviousViewWasFirstPerson))
            {
                EyeMaskVision_SetPlayerFirstPerson(false);
            }
            LogInfo(LogCategory_Action, L"Action", 
                L"Eye-mask removal completed; restoring view=%s.",
                forceThirdPerson
                    ? L"third-person (R override)"
                    : (g_eyeMaskVisionPreviousViewWasFirstPerson
                        ? L"first-person"
                        : L"third-person"));
            g_eyeMaskVisionForceThirdPersonOnRemoval = false;
            g_eyeMaskVisionPreviousViewCaptured = false;
        }
        g_eyeMaskVisionPreviousActive = eyeMaskActive;
    }

    if (!g_eyeMaskVisionSettings.enabled ||
        !eyeMaskActive)
    {
        return;
    }

    const float value = EyeMaskVision_GetValue();
    const float targetDistance =
        EyeMaskVision_ComputeTargetDistance(value);

    static float smoothedDistance = -1.0f;
    static unsigned long long lastUpdateAt = 0;
    const float deltaSeconds = lastUpdateAt == 0
        ? 0.0f
        : std::clamp(
            static_cast<float>(now - lastUpdateAt) / 1000.0f,
            0.0f,
            0.10f);
    lastUpdateAt = now;

    if (smoothedDistance <= 0.0f || deltaSeconds <= 0.0f)
    {
        smoothedDistance = targetDistance;
    }
    else
    {
        const float response = std::max(
            g_eyeMaskVisionSettings.responseSeconds,
            0.01f);
        const float blend =
            1.0f - std::exp(-deltaSeconds / response);
        smoothedDistance +=
            (targetDistance - smoothedDistance) * blend;
    }

    const float nearDistance = smoothedDistance * 0.5f;
    const float farDistance = smoothedDistance;
    const float density =
        kEyeMaskVisionDensityAtOneMeter / smoothedDistance;
    const float distanceSpan = std::max(
        g_eyeMaskVisionSettings.maxDistance -
            g_eyeMaskVisionSettings.minDistance,
        0.0001f);
    const float distanceT = std::clamp(
        (smoothedDistance -
            g_eyeMaskVisionSettings.minDistance) /
            distanceSpan,
        0.0f,
        1.0f);
    const float vignetteResponse = std::pow(
        distanceT,
        kEyeMaskVisionVignetteResponsePower);
    const float vignetteAlpha =
        g_eyeMaskVisionSettings.vignetteAtMinDistance +
        (g_eyeMaskVisionSettings.vignetteAtMaxDistance -
            g_eyeMaskVisionSettings.vignetteAtMinDistance) *
            vignetteResponse;

    const bool mcommonUpdated =
        EyeMaskVision_ApplyMCommonFog(
            nearDistance,
            farDistance,
            density);
    EyeMaskVision_ApplyRenderSettings(
        nearDistance,
        farDistance,
        density);
    EyeMaskVision_ApplyVignette(vignetteAlpha);

    static float lastLoggedValue = 100000.0f;
    static bool loggedMCommonFailure = false;
    if (std::abs(value - lastLoggedValue) >= 10.0f)
    {
        lastLoggedValue = value;
        LogInfo(LogCategory_Action, L"Action", 
            L"Eye-mask vision: value=%.2f distance=%.2fm near=%.2f far=%.2f density=%.4f vignette=%.2f mcommon=%d.",
            value,
            smoothedDistance,
            nearDistance,
            farDistance,
            density,
            vignetteAlpha,
            mcommonUpdated ? 1 : 0);
    }
    if (!mcommonUpdated && !loggedMCommonFailure)
    {
        loggedMCommonFailure = true;
        LogInfo(LogCategory_Action, L"Action", L"Eye-mask vision could not update MCommon fog fields.");
    }
}
