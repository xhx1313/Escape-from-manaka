// Native.cpp module: runtime_core.inc
// Included inside the shared implementation namespace by Native.cpp.
std::wstring GetModuleDirectory()
{
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(g_instance, path, MAX_PATH);
    std::wstring value(path);
    const auto position = value.find_last_of(L"\\/");
    return position == std::wstring::npos ? L"." : value.substr(0, position);
}

std::wstring GetEscapeFromManakaDirectory()
{
    const std::wstring directory =
        GetModuleDirectory() + L"\\EscapeFromManaka";
    CreateDirectoryW(directory.c_str(), nullptr);
    return directory;
}

std::wstring GetEscapeFromManakaSubdirectory(const wchar_t* name)
{
    const std::wstring directory =
        GetEscapeFromManakaDirectory() + L"\\" + name;
    CreateDirectoryW(directory.c_str(), nullptr);
    return directory;
}

void SetStatus(const std::wstring& value)
{
    {
        std::lock_guard<std::mutex> lock(g_statusMutex);
        g_status = value;
    }
    ++g_statusVersion;
    if (g_menuWindow != nullptr && IsWindow(g_menuWindow))
    {
        PostMessageW(g_menuWindow, kRefreshMenuTextMessage, 0, 0);
    }
    LogInfo(LogCategory_General, L"General", L"%s", value.c_str());
}

std::wstring GetStatus()
{
    std::lock_guard<std::mutex> lock(g_statusMutex);
    return g_status;
}

std::wstring Utf8ToWide(const std::string& value)
{
    if (value.empty())
    {
        return std::wstring();
    }

    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::string WideToUtf8(const std::wstring& value)
{
    if (value.empty())
    {
        return std::string();
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
    return result;
}

BOOL CALLBACK FindGameWindowCallback(HWND window, LPARAM parameter)
{
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (processId != GetCurrentProcessId() || !IsWindowVisible(window) || GetWindow(window, GW_OWNER) != nullptr)
    {
        return TRUE;
    }

    RECT rectangle{};
    if (!GetClientRect(window, &rectangle))
    {
        return TRUE;
    }

    const auto width = rectangle.right - rectangle.left;
    const auto height = rectangle.bottom - rectangle.top;
    const auto area = static_cast<long long>(width) * height;
    auto* best = reinterpret_cast<std::pair<HWND, long long>*>(parameter);
    if (area > best->second)
    {
        best->first = window;
        best->second = area;
    }
    return TRUE;
}

HWND FindGameWindow()
{
    std::pair<HWND, long long> best{nullptr, 0};
    EnumWindows(FindGameWindowCallback, reinterpret_cast<LPARAM>(&best));
    return best.first;
}

template <typename T>
bool ResolveExport(HMODULE module, const char* name, T& output)
{
    output = reinterpret_cast<T>(GetProcAddress(module, name));
    if (output == nullptr)
    {
        LogInfo(LogCategory_General, L"General", L"Missing GameAssembly export: %S", name);
        return false;
    }
    return true;
}

bool ResolveIl2CppApi()
{
    const auto gameAssembly = GetModuleHandleW(L"GameAssembly.dll");
    if (gameAssembly == nullptr)
    {
        return false;
    }

    if (il2cpp_domain_get == nullptr)
    {
        const bool resolved =
            ResolveExport(gameAssembly, "il2cpp_domain_get", il2cpp_domain_get) &&
            ResolveExport(gameAssembly, "il2cpp_thread_attach", il2cpp_thread_attach) &&
            ResolveExport(gameAssembly, "il2cpp_domain_get_assemblies", il2cpp_domain_get_assemblies) &&
            ResolveExport(gameAssembly, "il2cpp_assembly_get_image", il2cpp_assembly_get_image) &&
            ResolveExport(gameAssembly, "il2cpp_class_from_name", il2cpp_class_from_name) &&
            ResolveExport(gameAssembly, "il2cpp_class_get_field_from_name", il2cpp_class_get_field_from_name) &&
            ResolveExport(gameAssembly, "il2cpp_class_get_fields", il2cpp_class_get_fields) &&
            ResolveExport(gameAssembly, "il2cpp_field_get_name", il2cpp_field_get_name) &&
            ResolveExport(gameAssembly, "il2cpp_field_static_get_value", il2cpp_field_static_get_value) &&
            ResolveExport(gameAssembly, "il2cpp_field_get_value", il2cpp_field_get_value) &&
            ResolveExport(gameAssembly, "il2cpp_field_get_offset", il2cpp_field_get_offset) &&
            ResolveExport(gameAssembly, "il2cpp_class_get_methods", il2cpp_class_get_methods) &&
            ResolveExport(gameAssembly, "il2cpp_method_get_name", il2cpp_method_get_name) &&
            ResolveExport(gameAssembly, "il2cpp_method_get_param_count", il2cpp_method_get_param_count) &&
            ResolveExport(gameAssembly, "il2cpp_method_is_generic", il2cpp_method_is_generic) &&
            ResolveExport(gameAssembly, "il2cpp_runtime_invoke", il2cpp_runtime_invoke) &&
            ResolveExport(gameAssembly, "il2cpp_string_new", il2cpp_string_new) &&
            ResolveExport(gameAssembly, "il2cpp_string_chars", il2cpp_string_chars) &&
            ResolveExport(gameAssembly, "il2cpp_string_length", il2cpp_string_length) &&
            ResolveExport(gameAssembly, "il2cpp_object_unbox", il2cpp_object_unbox) &&
            ResolveExport(gameAssembly, "il2cpp_object_new", il2cpp_object_new) &&
            ResolveExport(gameAssembly, "il2cpp_array_length", il2cpp_array_length) &&
            ResolveExport(gameAssembly, "il2cpp_array_object_header_size", il2cpp_array_object_header_size) &&
            ResolveExport(gameAssembly, "il2cpp_array_new", il2cpp_array_new) &&
            ResolveExport(gameAssembly, "il2cpp_class_get_type", il2cpp_class_get_type) &&
            ResolveExport(gameAssembly, "il2cpp_type_get_object", il2cpp_type_get_object) &&
            ResolveExport(gameAssembly, "il2cpp_runtime_class_init", il2cpp_runtime_class_init) &&
            ResolveExport(gameAssembly, "il2cpp_object_get_class", il2cpp_object_get_class) &&
            ResolveExport(gameAssembly, "il2cpp_gc_wbarrier_set_field", il2cpp_gc_wbarrier_set_field) &&
            ResolveExport(gameAssembly, "il2cpp_gchandle_new", il2cpp_gchandle_new) &&
            ResolveExport(gameAssembly, "il2cpp_gchandle_get_target", il2cpp_gchandle_get_target) &&
            ResolveExport(gameAssembly, "il2cpp_gchandle_free", il2cpp_gchandle_free);

        if (!resolved)
        {
            return false;
        }
    }

    g_domain = il2cpp_domain_get();
    if (g_domain == nullptr)
    {
        return false;
    }

    il2cpp_thread_attach(g_domain);

    size_t assemblyCount = 0;
    const auto assemblies = il2cpp_domain_get_assemblies(g_domain, &assemblyCount);
    if (assemblies == nullptr || assemblyCount == 0)
    {
        return false;
    }

    for (size_t index = 0; index < assemblyCount && g_titleClass == nullptr; ++index)
    {
        const auto image = il2cpp_assembly_get_image(assemblies[index]);
        if (image == nullptr)
        {
            continue;
        }

        g_titleClass = il2cpp_class_from_name(image, "ExposureUnnoticed2.Scripts.UI", "TitleSceneView");
    }

    if (g_titleClass == nullptr)
    {
        return false;
    }

    g_titleInstanceField = il2cpp_class_get_field_from_name(g_titleClass, "_Instance_k__BackingField");
    if (g_titleInstanceField == nullptr)
    {
        g_titleInstanceField = il2cpp_class_get_field_from_name(g_titleClass, "Instance");
    }

    if (g_titleInstanceField == nullptr)
    {
        void* iterator = nullptr;
        while (const auto field = il2cpp_class_get_fields(g_titleClass, &iterator))
        {
            const auto* fieldName = il2cpp_field_get_name(field);
            if (fieldName != nullptr)
            {
                LogInfo(LogCategory_General, L"General", L"TitleSceneView field: %S", fieldName);
                if (std::strstr(fieldName, "Instance") != nullptr)
                {
                    g_titleInstanceField = const_cast<Il2CppField*>(field);
                    break;
                }
            }
        }
    }

    if (g_titleInstanceField == nullptr)
    {
        LogInfo(LogCategory_General, L"General", L"TitleSceneView was found, but no static Instance field could be resolved.");
        return false;
    }

    g_titleApiReady = true;
    LogInfo(LogCategory_General, L"General", L"Native IL2CPP title-scene detection initialized.");
    return true;
}

Il2CppClass* FindClass(const char* namespaceName, const char* className)
{
    if (g_domain == nullptr)
    {
        return nullptr;
    }

    size_t assemblyCount = 0;
    const auto assemblies = il2cpp_domain_get_assemblies(g_domain, &assemblyCount);
    if (assemblies == nullptr)
    {
        return nullptr;
    }

    for (size_t index = 0; index < assemblyCount; ++index)
    {
        const auto image = il2cpp_assembly_get_image(assemblies[index]);
        if (image == nullptr)
        {
            continue;
        }
        auto* found = il2cpp_class_from_name(image, namespaceName, className);
        if (found != nullptr)
        {
            return found;
        }
    }
    return nullptr;
}

const Il2CppMethod* FindMethod(Il2CppClass* klass, const char* name, int parameterCount)
{
    if (klass == nullptr)
    {
        return nullptr;
    }

    void* iterator = nullptr;
    while (const auto method = il2cpp_class_get_methods(klass, &iterator))
    {
        const auto* methodName = il2cpp_method_get_name(method);
        if (methodName != nullptr &&
            std::strcmp(methodName, name) == 0 &&
            il2cpp_method_get_param_count(method) == parameterCount &&
            !il2cpp_method_is_generic(method))
        {
            return method;
        }
    }
    return nullptr;
}

const Il2CppMethod* FindMethodOverload(
    Il2CppClass* klass,
    const char* name,
    int parameterCount,
    int occurrence)
{
    if (klass == nullptr || occurrence < 0)
    {
        return nullptr;
    }

    int found = 0;
    void* iterator = nullptr;
    while (const auto method = il2cpp_class_get_methods(klass, &iterator))
    {
        const auto* methodName = il2cpp_method_get_name(method);
        if (methodName != nullptr &&
            std::strcmp(methodName, name) == 0 &&
            il2cpp_method_get_param_count(method) == parameterCount &&
            !il2cpp_method_is_generic(method))
        {
            if (found == occurrence)
            {
                return method;
            }
            ++found;
        }
    }
    return nullptr;
}

std::wstring ReadIl2CppString(Il2CppString* value);

Il2CppObject* Invoke(const Il2CppMethod* method, Il2CppObject* instance, void** parameters)
{
    if (method == nullptr || il2cpp_runtime_invoke == nullptr)
    {
        return nullptr;
    }

    Il2CppObject* exception = nullptr;
    const auto result = il2cpp_runtime_invoke(method, instance, parameters, &exception);
    if (exception != nullptr)
    {
        LogInfo(LogCategory_General, L"General", L"IL2CPP runtime invoke raised an exception at %p.", exception);
        auto* exceptionClass = FindClass("System", "Exception");
        const auto getMessage = FindMethod(exceptionClass, "get_Message", 0);
        if (getMessage != nullptr)
        {
            Il2CppObject* nestedException = nullptr;
            auto* message = il2cpp_runtime_invoke(
                getMessage,
                exception,
                nullptr,
                &nestedException);
            if (nestedException == nullptr && message != nullptr)
            {
                LogInfo(LogCategory_General, L"General", 
                    L"IL2CPP exception message: %s",
                    ReadIl2CppString(static_cast<Il2CppString*>(message)).c_str());
            }
        }
        const auto getStackTrace = FindMethod(exceptionClass, "get_StackTrace", 0);
        if (getStackTrace != nullptr)
        {
            Il2CppObject* nestedException = nullptr;
            auto* stackTrace = il2cpp_runtime_invoke(
                getStackTrace,
                exception,
                nullptr,
                &nestedException);
            if (nestedException == nullptr && stackTrace != nullptr)
            {
                LogInfo(LogCategory_General, L"General", 
                    L"IL2CPP exception stack: %s",
                    ReadIl2CppString(static_cast<Il2CppString*>(stackTrace)).c_str());
            }
        }
        return nullptr;
    }
    return result;
}

std::wstring ReadIl2CppString(Il2CppString* value)
{
    if (value == nullptr || il2cpp_string_chars == nullptr || il2cpp_string_length == nullptr)
    {
        return std::wstring();
    }

    const auto* characters = il2cpp_string_chars(value);
    const auto length = il2cpp_string_length(value);
    return std::wstring(reinterpret_cast<const wchar_t*>(characters), static_cast<size_t>(length));
}

std::wstring GetObjectName(Il2CppObject* object)
{
    if (object == nullptr)
    {
        return std::wstring();
    }

    auto* objectClass = FindClass("UnityEngine", "Object");
    const auto getName = FindMethod(objectClass, "get_name", 0);
    auto* value = Invoke(getName, object, nullptr);
    return ReadIl2CppString(static_cast<Il2CppString*>(value));
}

void LogObjectReference(const char* label, Il2CppObject* object)
{
    if (object == nullptr)
    {
        LogInfo(LogCategory_General, L"General", L"%S: unavailable", label);
        return;
    }
    LogInfo(LogCategory_General, L"General", L"%S: %s", label, GetObjectName(object).c_str());
}

void LogObjectField(
    const char* label,
    Il2CppClass* klass,
    Il2CppObject* instance,
    const char* fieldName)
{
    if (klass == nullptr || instance == nullptr || il2cpp_field_get_value == nullptr)
    {
        return;
    }
    auto* field = il2cpp_class_get_field_from_name(klass, fieldName);
    if (field == nullptr)
    {
        LogInfo(LogCategory_General, L"General", L"%S field unavailable: %S", label, fieldName);
        return;
    }
    void* value = nullptr;
    il2cpp_field_get_value(field, instance, &value);
    LogObjectReference(label, static_cast<Il2CppObject*>(value));
}

void LogMethodRva(
    const char* label,
    const char* namespaceName,
    const char* className,
    const char* methodName,
    int parameterCount)
{
    auto* klass = FindClass(namespaceName, className);
    auto* method = FindMethod(klass, methodName, parameterCount);
    auto* pointer = MethodPointer(method);
    auto* module = GetModuleHandleW(L"GameAssembly.dll");
    const auto rva = pointer != nullptr && module != nullptr
        ? reinterpret_cast<uintptr_t>(pointer) -
            reinterpret_cast<uintptr_t>(module)
        : 0;
    LogInfo(LogCategory_General, L"General", 
        L"Lifecycle method %S.%S::%S/%d pointer=%p rva=0x%llx",
        namespaceName,
        className,
        methodName,
        parameterCount,
        pointer,
        static_cast<unsigned long long>(rva));
}

void LogLoadedAssetsByClass(
    const char* namespaceName,
    const char* className)
{
    auto* assetClass = FindClass(namespaceName, className);
    auto* resourcesClass = FindClass("UnityEngine", "Resources");
    if (assetClass == nullptr ||
        resourcesClass == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        LogInfo(LogCategory_General, L"General", 
            L"Loaded asset probe missing class %S.%S.",
            namespaceName,
            className);
        return;
    }

    il2cpp_runtime_class_init(assetClass);
    auto* assetType = il2cpp_type_get_object(
        il2cpp_class_get_type(assetClass));
    void* parameters[] = { assetType };
    auto* assets = Invoke(
        FindMethod(resourcesClass, "FindObjectsOfTypeAll", 1),
        nullptr,
        parameters);
    if (assets == nullptr)
    {
        LogInfo(LogCategory_General, L"General", 
            L"Loaded asset probe returned no inventory for %S.%S.",
            namespaceName,
            className);
        return;
    }

    const auto count = il2cpp_array_length(assets);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(assets) + il2cpp_array_object_header_size());
    int logged = 0;
    for (uint32_t index = 0; index < count && logged < 120; ++index)
    {
        auto* asset = elements[index];
        if (asset == nullptr)
        {
            continue;
        }
        LogInfo(LogCategory_General, L"General", 
            L"Loaded asset[%u] %S.%S: %s",
            index,
            namespaceName,
            className,
            GetObjectName(asset).c_str());
        ++logged;
    }
    LogInfo(LogCategory_General, L"General", 
        L"Loaded asset probe %S.%S count=%u logged=%d.",
        namespaceName,
        className,
        count,
        logged);
}

Il2CppObject* FindLoadedAssetByName(
    const char* namespaceName,
    const char* className,
    const wchar_t* targetName)
{
    auto* assetClass = FindClass(namespaceName, className);
    auto* resourcesClass = FindClass("UnityEngine", "Resources");
    if (assetClass == nullptr ||
        resourcesClass == nullptr ||
        targetName == nullptr ||
        il2cpp_runtime_class_init == nullptr)
    {
        return nullptr;
    }

    il2cpp_runtime_class_init(assetClass);
    auto* assetType = il2cpp_type_get_object(
        il2cpp_class_get_type(assetClass));
    void* parameters[] = { assetType };
    auto* assets = Invoke(
        FindMethod(resourcesClass, "FindObjectsOfTypeAll", 1),
        nullptr,
        parameters);
    if (assets == nullptr)
    {
        return nullptr;
    }

    const auto count = il2cpp_array_length(assets);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(assets) + il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* asset = elements[index];
        if (asset != nullptr && GetObjectName(asset) == targetName)
        {
            return asset;
        }
    }
    return nullptr;
}

Il2CppObject* FindFirstComponentOfClass(Il2CppClass* klass)
{
    if (klass == nullptr ||
        il2cpp_runtime_class_init == nullptr ||
        il2cpp_class_get_type == nullptr ||
        il2cpp_type_get_object == nullptr)
    {
        return nullptr;
    }

    il2cpp_runtime_class_init(klass);
    auto* objectClass = FindClass("UnityEngine", "Object");
    const auto findAll = FindMethod(objectClass, "FindObjectsOfType", 2);
    if (findAll == nullptr)
    {
        return nullptr;
    }

    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(klass));
    bool includeInactive = true;
    void* parameters[] = { managedType, &includeInactive };
    auto* objects = Invoke(findAll, nullptr, parameters);
    if (objects == nullptr || il2cpp_array_length(objects) == 0)
    {
        return nullptr;
    }

    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(objects) + il2cpp_array_object_header_size());
    return elements[0];
}

Il2CppObject* FindGameObjectByNameToken(const wchar_t* token)
{
    if (token == nullptr || il2cpp_runtime_class_init == nullptr)
    {
        return nullptr;
    }
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* managedType = il2cpp_type_get_object(il2cpp_class_get_type(gameObjectClass));
    bool includeInactive = true;
    void* parameters[] = { managedType, &includeInactive };
    auto* objects = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (objects == nullptr)
    {
        return nullptr;
    }
    const auto count = il2cpp_array_length(objects);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(objects) + il2cpp_array_object_header_size());
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* gameObject = elements[index];
        if (gameObject == nullptr)
        {
            continue;
        }
        const auto name = GetObjectName(gameObject);
        if (name.find(token) != std::wstring::npos)
        {
            return gameObject;
        }
    }
    return nullptr;
}

