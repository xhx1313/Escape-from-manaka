// Native.cpp module: scene_ui.inc
// Included inside the shared implementation namespace by Native.cpp.

void ClearSceneEventSelection()
{
    auto* eventSystemClass = FindClass(
        "UnityEngine.EventSystems",
        "EventSystem");
    if (eventSystemClass == nullptr)
    {
        return;
    }
    auto* eventSystem = Invoke(
        FindMethod(eventSystemClass, "get_current", 0),
        nullptr,
        nullptr);
    if (eventSystem == nullptr)
    {
        return;
    }
    auto* setSelected = FindMethod(
        eventSystemClass,
        "SetSelectedGameObject",
        2);
    if (setSelected != nullptr)
    {
        void* selectedParameters[] = { nullptr, nullptr };
        Invoke(setSelected, eventSystem, selectedParameters);
    }
    LogInfo(LogCategory_Scene, L"Scene", L"Scene EventSystem selection was cleared.");
}
Il2CppObject* CreateSceneGameObject(const char* name, Il2CppObject* parentTransform)
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

    auto* rect = AddSceneComponentToGameObject(
        gameObject,
        "UnityEngine",
        "RectTransform");
    auto* transform = rect != nullptr
        ? rect
        : Invoke(
            FindMethod(gameObjectClass, "get_transform", 0),
            gameObject,
            nullptr);
    if (transform != nullptr && parentTransform != nullptr)
    {
        bool keepWorldPosition = false;
        void* parentParameters[] = { parentTransform, &keepWorldPosition };
        Invoke(
            FindMethod(FindClass("UnityEngine", "Transform"), "SetParent", 2),
            transform,
            parentParameters);
    }
    return gameObject;
}

Il2CppObject* AddSceneComponentToGameObject(
    Il2CppObject* gameObject,
    const char* namespaceName,
    const char* className)
{
    if (gameObject == nullptr ||
        il2cpp_runtime_class_init == nullptr ||
        il2cpp_class_get_type == nullptr ||
        il2cpp_type_get_object == nullptr)
    {
        return nullptr;
    }

    auto* componentClass = FindClass(namespaceName, className);
    if (componentClass == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Unity component class unavailable: %S.%S", namespaceName, className);
        return nullptr;
    }
    il2cpp_runtime_class_init(componentClass);
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(componentClass));
    void* parameters[] = { managedType };
    return Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "AddComponent", 1),
        gameObject,
        parameters);
}

Il2CppObject* GetSceneComponentByClass(
    Il2CppObject* gameObject,
    const char* namespaceName,
    const char* className)
{
    if (gameObject == nullptr || il2cpp_runtime_class_init == nullptr)
    {
        return nullptr;
    }
    auto* componentClass = FindClass(namespaceName, className);
    if (componentClass == nullptr)
    {
        return nullptr;
    }
    il2cpp_runtime_class_init(componentClass);
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(componentClass));
    void* parameters[] = { managedType };
    return Invoke(
        FindMethod(FindClass("UnityEngine", "GameObject"), "GetComponent", 1),
        gameObject,
        parameters);
}

struct SceneTextureCacheEntry
{
    std::wstring path;
    Il2CppObject* texture = nullptr;
};

static std::vector<SceneTextureCacheEntry> g_sceneTextureCache;

void ClearScenePngTextureCache()
{
    if (g_sceneTextureCache.empty())
    {
        return;
    }

    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* destroy = objectClass != nullptr
        ? FindMethod(objectClass, "Destroy", 1)
        : nullptr;
    auto* inequality = objectClass != nullptr
        ? FindMethod(objectClass, "op_Inequality", 2)
        : nullptr;
    const size_t count = g_sceneTextureCache.size();
    for (const auto& entry : g_sceneTextureCache)
    {
        if (entry.texture == nullptr ||
            destroy == nullptr ||
            inequality == nullptr)
        {
            continue;
        }
        Il2CppObject* nullObject = nullptr;
        void* aliveParameters[] = { entry.texture, nullObject };
        auto* alive = Invoke(inequality, nullptr, aliveParameters);
        if (alive == nullptr ||
            il2cpp_object_unbox == nullptr ||
            !*static_cast<bool*>(il2cpp_object_unbox(alive)))
        {
            continue;
        }
        void* destroyParameters[] = { entry.texture };
        Invoke(destroy, nullptr, destroyParameters);
    }
    g_sceneTextureCache.clear();
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Released %llu scene PNG texture cache entr%s before the next scene.",
        static_cast<unsigned long long>(count),
        count == 1 ? L"y" : L"ies");
}

Il2CppObject* LoadScenePngTexture(const std::wstring& path)
{
    for (const auto& entry : g_sceneTextureCache)
    {
        if (entry.path == path)
        {
            return entry.texture;
        }
    }

    HANDLE file = CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Unable to open equipment image %s.", path.c_str());
        return nullptr;
    }
    LARGE_INTEGER fileSize{};
    const bool sizeRead =
        GetFileSizeEx(file, &fileSize) &&
        fileSize.QuadPart > 0 &&
        fileSize.QuadPart <= 8 * 1024 * 1024;
    std::vector<unsigned char> bytes;
    DWORD bytesRead = 0;
    if (sizeRead)
    {
        bytes.resize(static_cast<size_t>(fileSize.QuadPart));
        if (!ReadFile(
                file,
                bytes.data(),
                static_cast<DWORD>(bytes.size()),
                &bytesRead,
                nullptr) ||
            bytesRead != bytes.size())
        {
            bytes.clear();
        }
    }
    CloseHandle(file);
    if (bytes.empty())
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Unable to read equipment image %s.", path.c_str());
        return nullptr;
    }

    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* imageConversionClass = FindClass("UnityEngine", "ImageConversion");
    auto* byteClass = FindClass("System", "Byte");
    if (textureClass == nullptr ||
        imageConversionClass == nullptr ||
        byteClass == nullptr ||
        il2cpp_object_new == nullptr ||
        il2cpp_array_new == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        return nullptr;
    }
    il2cpp_runtime_class_init(textureClass);
    il2cpp_runtime_class_init(byteClass);

    auto* texture = il2cpp_object_new(textureClass);
    int width = 2;
    int height = 2;
    int textureFormat = 4;
    bool mipChain = false;
    void* constructorParameters[] = {
        &width,
        &height,
        &textureFormat,
        &mipChain,
    };
    Invoke(
        FindMethod(textureClass, ".ctor", 4),
        texture,
        constructorParameters);

    auto* data = il2cpp_array_new(byteClass, bytes.size());
    if (texture == nullptr || data == nullptr)
    {
        return nullptr;
    }
    std::memcpy(
        reinterpret_cast<char*>(data) +
            il2cpp_array_object_header_size(),
        bytes.data(),
        bytes.size());
    void* loadParameters[] = { texture, data };
    auto* loaded = Invoke(
        FindMethod(imageConversionClass, "LoadImage", 2),
        nullptr,
        loadParameters);
    if (loaded == nullptr ||
        il2cpp_object_unbox == nullptr ||
        !*static_cast<bool*>(il2cpp_object_unbox(loaded)))
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Equipment image decoding failed for %s.", path.c_str());
        return nullptr;
    }

    g_sceneTextureCache.push_back({ path, texture });
    return texture;
}

void ReleaseScenePngTexture(const std::wstring& path)
{
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* destroy = objectClass != nullptr
        ? FindMethod(objectClass, "Destroy", 1)
        : nullptr;
    for (auto iterator = g_sceneTextureCache.begin();
         iterator != g_sceneTextureCache.end();
         ++iterator)
    {
        if (iterator->path != path)
        {
            continue;
        }
        if (iterator->texture != nullptr && destroy != nullptr)
        {
            void* destroyParameters[] = { iterator->texture };
            Invoke(destroy, nullptr, destroyParameters);
        }
        g_sceneTextureCache.erase(iterator);
        return;
    }
}

void SetSceneRawImageTexture(
    Il2CppObject* rawImage,
    Il2CppObject* texture)
{
    if (rawImage == nullptr)
    {
        return;
    }
    void* parameters[] = { texture };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "RawImage"), "set_texture", 1),
        rawImage,
        parameters);
}

void SetSceneRawImagePreserveAspect(
    Il2CppObject* gameObject,
    Il2CppObject* texture)
{
    if (gameObject == nullptr || texture == nullptr)
    {
        return;
    }
    auto* textureClass = FindClass("UnityEngine", "Texture");
    auto* widthMethod = textureClass != nullptr
        ? FindMethod(textureClass, "get_width", 0)
        : nullptr;
    auto* heightMethod = textureClass != nullptr
        ? FindMethod(textureClass, "get_height", 0)
        : nullptr;
    if (widthMethod == nullptr || heightMethod == nullptr)
    {
        return;
    }
    auto* widthObject = Invoke(widthMethod, texture, nullptr);
    auto* heightObject = Invoke(heightMethod, texture, nullptr);
    if (widthObject == nullptr || heightObject == nullptr)
    {
        return;
    }
    const int textureWidth =
        *static_cast<int*>(il2cpp_object_unbox(widthObject));
    const int textureHeight =
        *static_cast<int*>(il2cpp_object_unbox(heightObject));
    if (textureWidth <= 0 || textureHeight <= 0)
    {
        return;
    }
    auto* fitterClass = FindClass(
        "UnityEngine.UI",
        "AspectRatioFitter");
    if (fitterClass == nullptr)
    {
        return;
    }
    auto* fitter = GetSceneComponentByClass(
        gameObject,
        "UnityEngine.UI",
        "AspectRatioFitter");
    if (fitter == nullptr)
    {
        fitter = AddSceneComponentToGameObject(
            gameObject,
            "UnityEngine.UI",
            "AspectRatioFitter");
    }
    if (fitter == nullptr)
    {
        return;
    }
    int aspectMode = 3;
    float aspectRatio =
        static_cast<float>(textureWidth) /
        static_cast<float>(textureHeight);
    void* modeParameters[] = { &aspectMode };
    void* ratioParameters[] = { &aspectRatio };
    Invoke(
        FindMethod(fitterClass, "set_aspectMode", 1),
        fitter,
        modeParameters);
    Invoke(
        FindMethod(fitterClass, "set_aspectRatio", 1),
        fitter,
        ratioParameters);
}

void SetSceneRect(
    Il2CppObject* rect,
    float minX,
    float minY,
    float maxX,
    float maxY)
{
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

void SetSceneGraphicColor(
    Il2CppObject* graphic,
    float red,
    float green,
    float blue,
    float alpha)
{
    if (graphic == nullptr)
    {
        return;
    }
    float color[] = { red, green, blue, alpha };
    void* parameters[] = { color };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Graphic"), "set_color", 1),
        graphic,
        parameters);
}

using FontFaceLoadFn = int (*)(Il2CppObject*, int32_t, int32_t);
using FontFaceLoadTwoArgFn = int (*)(Il2CppObject*, int32_t);

std::atomic_bool g_fullScFontHookActive{false};
std::atomic_int g_fullScPublicHookHits{0};
std::atomic_int g_fullScInternalHookHits{0};
std::atomic_int g_fullScPublicTwoArgHookHits{0};
std::atomic_int g_fullScInternalTwoArgHookHits{0};
std::atomic_int g_fullScNativeTwoArgHookHits{0};
std::atomic_int g_fullScNativeThreeArgHookHits{0};
FontFaceLoadFn g_originalPublicFontFaceLoad = nullptr;
FontFaceLoadFn g_originalInternalFontFaceLoad = nullptr;
FontFaceLoadTwoArgFn g_originalPublicFontFaceLoadTwoArg = nullptr;
FontFaceLoadTwoArgFn g_originalInternalFontFaceLoadTwoArg = nullptr;
FontFaceLoadTwoArgFn g_originalNativeFontFaceLoadTwoArg = nullptr;
FontFaceLoadFn g_originalNativeFontFaceLoadThreeArg = nullptr;
bool g_fullScHooksInstalled = false;

struct FullScCallPatch
{
    void* address = nullptr;
    uint8_t original[5] = {};
    bool saved = false;
};

FullScCallPatch g_fullScCalls[2];

bool ScenePatchFullScCallSites()
{
    auto* gameAssembly = reinterpret_cast<uint8_t*>(
        GetModuleHandleW(L"GameAssembly.dll"));
    if (gameAssembly == nullptr)
    {
        return false;
    }
    const uintptr_t offsets[] = { 0x1D5ABE6, 0x1D5BAA3 };
    for (size_t index = 0; index < 2; ++index)
    {
        auto& patch = g_fullScCalls[index];
        patch.address = gameAssembly + offsets[index];
        if (!patch.saved)
        {
            std::memcpy(patch.original, patch.address, 5);
            if (patch.original[0] != 0xE8)
            {
                LogInfo(LogCategory_Scene, L"Scene", 
                    L"Unexpected TMP font call opcode at RVA 0x%llX: 0x%02X.",
                    static_cast<unsigned long long>(offsets[index]),
                    patch.original[0]);
                return false;
            }
            patch.saved = true;
        }
    }

    const uint8_t patchBytes[5] = { 0xB8, 0x00, 0x00, 0x00, 0x00 };
    for (auto& patch : g_fullScCalls)
    {
        DWORD previousProtection = 0;
        if (!VirtualProtect(
                patch.address,
                5,
                PAGE_EXECUTE_READWRITE,
                &previousProtection))
        {
            return false;
        }
        std::memcpy(patch.address, patchBytes, 5);
        DWORD ignored = 0;
        VirtualProtect(
            patch.address,
            5,
            previousProtection,
            &ignored);
        FlushInstructionCache(
            GetCurrentProcess(),
            patch.address,
            5);
    }
    LogInfo(LogCategory_Scene, L"Scene", L"TMP full SC font call-site bypass installed.");
    return true;
}

void SceneRestoreFullScCallSites()
{
    for (auto& patch : g_fullScCalls)
    {
        if (!patch.saved || patch.address == nullptr)
        {
            continue;
        }
        DWORD previousProtection = 0;
        if (VirtualProtect(
                patch.address,
                5,
                PAGE_EXECUTE_READWRITE,
                &previousProtection))
        {
            std::memcpy(patch.address, patch.original, 5);
            DWORD ignored = 0;
            VirtualProtect(
                patch.address,
                5,
                previousProtection,
                &ignored);
            FlushInstructionCache(GetCurrentProcess(), patch.address, 5);
        }
    }
    LogInfo(LogCategory_Scene, L"Scene", L"TMP full SC font call-site bypass restored.");
}

int FullScPublicFontFaceLoadDetour(
    Il2CppObject* font,
    int32_t pointSize,
    int32_t faceIndex)
{
    if (g_fullScFontHookActive.load())
    {
        ++g_fullScPublicHookHits;
        return 0;
    }
    return g_originalPublicFontFaceLoad != nullptr
        ? g_originalPublicFontFaceLoad(font, pointSize, faceIndex)
        : 1;
}

int FullScInternalFontFaceLoadDetour(
    Il2CppObject* font,
    int32_t pointSize,
    int32_t faceIndex)
{
    if (g_fullScFontHookActive.load())
    {
        ++g_fullScInternalHookHits;
        return 0;
    }
    return g_originalInternalFontFaceLoad != nullptr
        ? g_originalInternalFontFaceLoad(font, pointSize, faceIndex)
        : 1;
}

int FullScPublicFontFaceLoadTwoArgDetour(
    Il2CppObject* font,
    int32_t pointSize)
{
    if (g_fullScFontHookActive.load())
    {
        ++g_fullScPublicTwoArgHookHits;
        return 0;
    }
    return g_originalPublicFontFaceLoadTwoArg != nullptr
        ? g_originalPublicFontFaceLoadTwoArg(font, pointSize)
        : 1;
}

int FullScInternalFontFaceLoadTwoArgDetour(
    Il2CppObject* font,
    int32_t pointSize)
{
    if (g_fullScFontHookActive.load())
    {
        ++g_fullScInternalTwoArgHookHits;
        return 0;
    }
    return g_originalInternalFontFaceLoadTwoArg != nullptr
        ? g_originalInternalFontFaceLoadTwoArg(font, pointSize)
        : 1;
}

int FullScNativeFontFaceLoadTwoArgDetour(
    Il2CppObject* font,
    int32_t pointSize)
{
    if (g_fullScFontHookActive.load())
    {
        ++g_fullScNativeTwoArgHookHits;
        return 0;
    }
    return g_originalNativeFontFaceLoadTwoArg != nullptr
        ? g_originalNativeFontFaceLoadTwoArg(font, pointSize)
        : 1;
}

int FullScNativeFontFaceLoadThreeArgDetour(
    Il2CppObject* font,
    int32_t pointSize,
    int32_t faceIndex)
{
    if (g_fullScFontHookActive.load())
    {
        ++g_fullScNativeThreeArgHookHits;
        return 0;
    }
    return g_originalNativeFontFaceLoadThreeArg != nullptr
        ? g_originalNativeFontFaceLoadThreeArg(font, pointSize, faceIndex)
        : 1;
}

bool SceneInstallFullScFontHook()
{
    if (g_fullScHooksInstalled)
    {
        return true;
    }
    auto* gameAssembly = reinterpret_cast<char*>(
        GetModuleHandleW(L"GameAssembly.dll"));
    if (gameAssembly == nullptr)
    {
        return false;
    }
    const auto publicLoad = gameAssembly + 0x1FC9AE0;
    const auto internalLoad = gameAssembly + 0x1FC9B90;
    const auto publicLoadTwoArg = gameAssembly + 0x1FC99E0;
    const auto internalLoadTwoArg = gameAssembly + 0x1FC9A80;
    auto* fontEngineClass = FindClass(
        "UnityEngine.TextCore.LowLevel",
        "FontEngine");
    auto* sourceFont = FindLoadedAssetByName(
        "UnityEngine",
        "Font",
        L"NotoSerifSC-Regular");
    if (fontEngineClass != nullptr && sourceFont != nullptr)
    {
        int32_t pointSize = 90;
        int32_t faceIndex = 0;
        void* twoArgParameters[] = { sourceFont, &pointSize };
        Invoke(
            FindMethod(
                fontEngineClass,
                "LoadFontFace_With_Size_FromFont_Internal",
                2),
            nullptr,
            twoArgParameters);
        void* threeArgParameters[] = {
            sourceFont,
            &pointSize,
            &faceIndex
        };
        Invoke(
            FindMethod(
                fontEngineClass,
                "LoadFontFace_With_Size_and_FaceIndex_FromFont_Internal",
                3),
            nullptr,
            threeArgParameters);
    }
    auto* nativeLoadTwoArg = *reinterpret_cast<void**>(
        gameAssembly + 0x2EC3C70);
    auto* nativeLoadThreeArg = *reinterpret_cast<void**>(
        gameAssembly + 0x2EC3C80);
    const auto publicStatus = MH_CreateHook(
        publicLoad,
        reinterpret_cast<LPVOID>(&FullScPublicFontFaceLoadDetour),
        reinterpret_cast<LPVOID*>(&g_originalPublicFontFaceLoad));
    const auto internalStatus = MH_CreateHook(
        internalLoad,
        reinterpret_cast<LPVOID>(&FullScInternalFontFaceLoadDetour),
        reinterpret_cast<LPVOID*>(&g_originalInternalFontFaceLoad));
    const auto publicTwoArgStatus = MH_CreateHook(
        publicLoadTwoArg,
        reinterpret_cast<LPVOID>(&FullScPublicFontFaceLoadTwoArgDetour),
        reinterpret_cast<LPVOID*>(&g_originalPublicFontFaceLoadTwoArg));
    const auto internalTwoArgStatus = MH_CreateHook(
        internalLoadTwoArg,
        reinterpret_cast<LPVOID>(&FullScInternalFontFaceLoadTwoArgDetour),
        reinterpret_cast<LPVOID*>(&g_originalInternalFontFaceLoadTwoArg));
    const auto nativeTwoArgStatus = nativeLoadTwoArg != nullptr
        ? MH_CreateHook(
              nativeLoadTwoArg,
              reinterpret_cast<LPVOID>(&FullScNativeFontFaceLoadTwoArgDetour),
              reinterpret_cast<LPVOID*>(&g_originalNativeFontFaceLoadTwoArg))
        : MH_ERROR_NOT_CREATED;
    const auto nativeThreeArgStatus = nativeLoadThreeArg != nullptr
        ? MH_CreateHook(
              nativeLoadThreeArg,
              reinterpret_cast<LPVOID>(&FullScNativeFontFaceLoadThreeArgDetour),
              reinterpret_cast<LPVOID*>(&g_originalNativeFontFaceLoadThreeArg))
        : MH_ERROR_NOT_CREATED;
    const bool success =
        publicStatus == MH_OK &&
        internalStatus == MH_OK &&
        publicTwoArgStatus == MH_OK &&
        internalTwoArgStatus == MH_OK &&
        nativeTwoArgStatus == MH_OK &&
        nativeThreeArgStatus == MH_OK &&
        MH_EnableHook(publicLoad) == MH_OK &&
        MH_EnableHook(internalLoad) == MH_OK &&
        MH_EnableHook(publicLoadTwoArg) == MH_OK &&
        MH_EnableHook(internalLoadTwoArg) == MH_OK &&
        MH_EnableHook(nativeLoadTwoArg) == MH_OK &&
        MH_EnableHook(nativeLoadThreeArg) == MH_OK;
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Full SC FontEngine hooks: public=%d internal=%d public2=%d internal2=%d native2=%p/%d native3=%p/%d success=%d.",
        publicStatus,
        internalStatus,
        publicTwoArgStatus,
        internalTwoArgStatus,
        nativeLoadTwoArg,
        nativeTwoArgStatus,
        nativeLoadThreeArg,
        nativeThreeArgStatus,
        success ? 1 : 0);
    const bool callsPatched = ScenePatchFullScCallSites();
    g_fullScHooksInstalled = callsPatched;
    g_fullScFontHookActive.store(callsPatched);
    return callsPatched;
}

bool SceneLoadFullScFontFace(int32_t pointSize)
{
    auto* fontEngineClass = FindClass(
        "UnityEngine.TextCore.LowLevel",
        "FontEngine");
    if (fontEngineClass == nullptr)
    {
        return false;
    }
    const std::wstring fontPath =
        GetEscapeFromManakaSubdirectory(L"Fonts") +
        L"\\NotoSerifSC-Regular.ttf";
    auto* pathValue = il2cpp_string_new(WideToUtf8(fontPath).c_str());
    int32_t faceIndex = 0;
    void* parameters[] = { pathValue, &pointSize, &faceIndex };
    const auto wasActive = g_fullScFontHookActive.exchange(false);
    auto* result = Invoke(
        FindMethod(
            fontEngineClass,
            "LoadFontFace_With_Size_And_FaceIndex_Internal",
            3),
        nullptr,
        parameters);
    g_fullScFontHookActive.store(wasActive);
    const bool loaded = result != nullptr &&
        *static_cast<int*>(il2cpp_object_unbox(result)) == 0;
    uint32_t shiCodepoint = 0x5E02;
    void* glyphParameters[] = { &shiCodepoint };
    auto* glyphValue = Invoke(
        FindMethod(fontEngineClass, "GetGlyphIndex", 1),
        nullptr,
        glyphParameters);
    const uint32_t glyphIndex = glyphValue != nullptr
        ? *static_cast<uint32_t*>(il2cpp_object_unbox(glyphValue))
        : 0;
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Full SC font face loaded=%d U+5E02 glyph=%u publicHits=%d internalHits=%d public2Hits=%d internal2Hits=%d native2Hits=%d native3Hits=%d.",
        loaded ? 1 : 0,
        glyphIndex,
        g_fullScPublicHookHits.load(),
        g_fullScInternalHookHits.load(),
        g_fullScPublicTwoArgHookHits.load(),
        g_fullScInternalTwoArgHookHits.load(),
        g_fullScNativeTwoArgHookHits.load(),
        g_fullScNativeThreeArgHookHits.load());
    if (!loaded)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"TextCore could not load the full SC font face.");
    }
    return loaded;
}

bool SceneTmpFontAddCharacters(
    Il2CppObject* fontAsset,
    Il2CppString* text,
    bool logFailure)
{
    if (fontAsset == nullptr || text == nullptr)
    {
        return false;
    }

    auto* fontAssetClass = FindClass("TMPro", "TMP_FontAsset");
    auto* populationValue = Invoke(
        FindMethod(fontAssetClass, "get_atlasPopulationMode", 0),
        fontAsset,
        nullptr);
    if (populationValue == nullptr ||
        *static_cast<int*>(il2cpp_object_unbox(populationValue)) == 0)
    {
        return false;
    }
    auto* addCharacters = FindMethod(
        fontAssetClass,
        "TryAddCharacters",
        3);
    if (addCharacters == nullptr)
    {
        if (logFailure)
        {
            LogInfo(LogCategory_Scene, L"Scene", L"TMP_FontAsset.TryAddCharacters(string, out string, bool) is unavailable.");
        }
        return false;
    }

    Il2CppString* missingCharacters = nullptr;
    bool includeFontFeatures = false;
    void* parameters[] = {
        text,
        &missingCharacters,
        &includeFontFeatures
    };
    if (g_tmpFontUsesFullScHook && fontAsset == g_tmpFontAsset)
    {
        SceneLoadFullScFontFace(90);
        SceneInstallFullScFontHook();
    }
    auto* result = Invoke(addCharacters, fontAsset, parameters);
    const bool added = result != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
    static Il2CppObject* fontAssetWithLoggedFailure = nullptr;
    if (added)
    {
        fontAssetWithLoggedFailure = nullptr;
    }
    if (!added && logFailure)
    {
        if (fontAssetWithLoggedFailure != fontAsset)
        {
            fontAssetWithLoggedFailure = fontAsset;
            LogInfo(LogCategory_Scene, L"Scene", 
                L"TMP font could not add every character; missing: %s",
                ReadIl2CppString(missingCharacters).c_str());
        }
    }
    return added;
}

bool SceneTmpFontHasCodepoint(
    Il2CppObject* fontAsset,
    int codepoint)
{
    if (fontAsset == nullptr)
    {
        return false;
    }

    auto* hasCharacter = FindMethod(
        FindClass("TMPro", "TMP_FontAsset"),
        "HasCharacter",
        1);
    if (hasCharacter == nullptr)
    {
        return false;
    }

    void* parameters[] = { &codepoint };
    auto* result = Invoke(hasCharacter, fontAsset, parameters);
    return result != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(result));
}

void SetSceneText(
    Il2CppObject* textComponent,
    const char* text,
    float fontSize,
    int alignment,
    float red,
    float green,
    float blue,
    float alpha)
{
    if (textComponent == nullptr)
    {
        return;
    }
    // The shipped Unity build has a partially stripped TMP dynamic-font path.
    // Use its complete SC Unity Font directly so UGUI can request every glyph.
    auto* textClass = FindClass("UnityEngine.UI", "Text");
    if (textClass == nullptr)
    {
        return;
    }
    auto* textValue = il2cpp_string_new(text);
    float color[] = { red, green, blue, alpha };
    void* textParameters[] = { textValue };
    void* fontParameters[] = { g_fullScUnityFont };
    int integerFontSize = static_cast<int>(std::round(fontSize));
    if (integerFontSize < 1)
    {
        integerFontSize = 1;
    }
    void* sizeParameters[] = { &integerFontSize };
    int textAnchor = 4;
    switch (alignment)
    {
    case 257: textAnchor = 0; break;
    case 258: textAnchor = 1; break;
    case 260: textAnchor = 2; break;
    case 513: textAnchor = 3; break;
    case 514: textAnchor = 4; break;
    case 516: textAnchor = 5; break;
    case 1025: textAnchor = 6; break;
    case 1026: textAnchor = 7; break;
    case 1028: textAnchor = 8; break;
    default: break;
    }
    void* alignmentParameters[] = { &textAnchor };
    void* colorParameters[] = { color };
    bool raycastTarget = false;
    void* raycastParameters[] = { &raycastTarget };
    int horizontalWrap = 0;
    int verticalOverflow = 1;
    bool richText = true;
    void* horizontalParameters[] = { &horizontalWrap };
    void* verticalParameters[] = { &verticalOverflow };
    void* richTextParameters[] = { &richText };
    if (g_fullScUnityFont != nullptr)
    {
        Invoke(FindMethod(textClass, "set_font", 1), textComponent, fontParameters);
    }
    Invoke(FindMethod(textClass, "set_text", 1), textComponent, textParameters);
    Invoke(FindMethod(textClass, "set_fontSize", 1), textComponent, sizeParameters);
    Invoke(FindMethod(textClass, "set_alignment", 1), textComponent, alignmentParameters);
    Invoke(FindMethod(textClass, "set_horizontalOverflow", 1), textComponent, horizontalParameters);
    Invoke(FindMethod(textClass, "set_verticalOverflow", 1), textComponent, verticalParameters);
    Invoke(FindMethod(textClass, "set_supportRichText", 1), textComponent, richTextParameters);
    Invoke(FindMethod(FindClass("UnityEngine.UI", "Graphic"), "set_color", 1), textComponent, colorParameters);
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Graphic"), "set_raycastTarget", 1),
        textComponent,
        raycastParameters);
}

Il2CppObject* FindFirstTmpTextComponent(Il2CppObject* gameObject)
{
    auto* tmpTextClass = FindClass("TMPro", "TMP_Text");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (gameObject == nullptr || tmpTextClass == nullptr || gameObjectClass == nullptr)
    {
        return nullptr;
    }
    if (il2cpp_runtime_class_init != nullptr)
    {
        il2cpp_runtime_class_init(tmpTextClass);
    }
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(tmpTextClass));
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
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(components) + il2cpp_array_object_header_size());
    return elements[0];
}

SceneUiElement CreateScenePanel(
    const char* name,
    Il2CppObject* parentTransform,
    float minX,
    float minY,
    float maxX,
    float maxY,
    float red,
    float green,
    float blue,
    float alpha)
{
    SceneUiElement element;
    element.gameObject = CreateSceneGameObject(name, parentTransform);
    if (element.gameObject == nullptr)
    {
        return element;
    }
    element.rect = GetSceneComponentByClass(
        element.gameObject,
        "UnityEngine",
        "RectTransform");
    element.image = AddSceneComponentToGameObject(
        element.gameObject,
        "UnityEngine.UI",
        "Image");
    SetSceneRect(element.rect, minX, minY, maxX, maxY);
    SetSceneGraphicColor(element.image, red, green, blue, alpha);
    bool raycastTarget = false;
    void* raycastParameters[] = { &raycastTarget };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Graphic"), "set_raycastTarget", 1),
        element.image,
        raycastParameters);
    return element;
}

SceneUiElement CreateSceneRawImageElement(
    const char* name,
    Il2CppObject* parentTransform,
    const std::wstring& texturePath,
    float minX,
    float minY,
    float maxX,
    float maxY,
    float red,
    float green,
    float blue,
    float alpha)
{
    SceneUiElement element;
    element.gameObject = CreateSceneGameObject(name, parentTransform);
    if (element.gameObject == nullptr)
    {
        return element;
    }
    element.rect = GetSceneComponentByClass(
        element.gameObject,
        "UnityEngine",
        "RectTransform");
    element.rawImage = AddSceneComponentToGameObject(
        element.gameObject,
        "UnityEngine.UI",
        "RawImage");
    SetSceneRect(element.rect, minX, minY, maxX, maxY);
    element.texture = texturePath.empty()
        ? nullptr
        : LoadScenePngTexture(texturePath);
    SetSceneRawImageTexture(element.rawImage, element.texture);
    SetSceneGraphicColor(element.rawImage, red, green, blue, alpha);
    bool raycastTarget = false;
    void* raycastParameters[] = { &raycastTarget };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Graphic"), "set_raycastTarget", 1),
        element.rawImage,
        raycastParameters);
    return element;
}

SceneUiElement CreateSceneTextElement(
    const char* name,
    Il2CppObject* parentTransform,
    const char* text,
    float minX,
    float minY,
    float maxX,
    float maxY,
    float fontSize,
    int alignment,
    float red,
    float green,
    float blue,
    float alpha)
{
    SceneUiElement element;
    element.gameObject = CreateSceneGameObject(name, parentTransform);
    element.rect = GetSceneComponentByClass(
        element.gameObject,
        "UnityEngine",
        "RectTransform");
    element.text = AddSceneComponentToGameObject(
        element.gameObject,
        "UnityEngine.UI",
        "Text");
    SetSceneRect(element.rect, minX, minY, maxX, maxY);
    SetSceneText(
        element.text,
        text,
        fontSize,
        alignment,
        red,
        green,
        blue,
        alpha);
    return element;
}

SceneUiElement CreateSceneButton(
    const char* name,
    Il2CppObject* parentTransform,
    const char* label,
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
    SceneUiElement element;
    element.gameObject = CreateSceneGameObject(name, parentTransform);
    element.rect = GetSceneComponentByClass(
        element.gameObject,
        "UnityEngine",
        "RectTransform");
    element.image = AddSceneComponentToGameObject(
        element.gameObject,
        "UnityEngine.UI",
        "Image");
    element.button = AddSceneComponentToGameObject(
        element.gameObject,
        "UnityEngine.UI",
        "Button");
    SetSceneRect(element.rect, minX, minY, maxX, maxY);
    SetSceneGraphicColor(element.image, red, green, blue, alpha);
    void* targetParameters[] = { element.image };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Selectable"), "set_targetGraphic", 1),
        element.button,
        targetParameters);
    int transition = 0;
    void* transitionParameters[] = { &transition };
    Invoke(
        FindMethod(FindClass("UnityEngine.UI", "Selectable"), "set_transition", 1),
        element.button,
        transitionParameters);
    SetButtonInteractable(element.button, interactable);

    auto labelElement = CreateSceneTextElement(
        "Label",
        element.rect,
        label,
        0.04f,
        0.08f,
        0.96f,
        0.92f,
        fontSize,
        alignment,
        0.94f,
        0.96f,
        0.98f,
        1.0f);
    element.text = labelElement.text;
    if (interactable)
    {
        g_uiButtons.push_back(element.button);
    }
    return element;
}

void LogSceneRect(const char* label, Il2CppObject* rect)
{
    if (rect == nullptr || il2cpp_object_unbox == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Rect %S unavailable", label);
        return;
    }
    auto* value = Invoke(
        FindMethod(FindClass("UnityEngine", "RectTransform"), "get_rect", 0),
        rect,
        nullptr);
    if (value == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Rect %S read failed", label);
        return;
    }
    const auto* values = static_cast<float*>(il2cpp_object_unbox(value));
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Rect %S: x=%.1f y=%.1f width=%.1f height=%.1f",
        label,
        values[0],
        values[1],
        values[2],
        values[3]);
}

void BringSceneObjectToFront(Il2CppObject* gameObject)
{
    if (gameObject == nullptr)
    {
        return;
    }
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* transformClass = FindClass("UnityEngine", "Transform");
    if (gameObjectClass == nullptr || transformClass == nullptr)
    {
        return;
    }
    auto* transform = Invoke(
        FindMethod(gameObjectClass, "get_transform", 0),
        gameObject,
        nullptr);
    auto* setAsLastSibling = FindMethod(
        transformClass,
        "SetAsLastSibling",
        0);
    if (transform != nullptr && setAsLastSibling != nullptr)
    {
        Invoke(setAsLastSibling, transform, nullptr);
    }
}

