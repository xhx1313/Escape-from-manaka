// Native.cpp module: moaning_audio.inc
// Included inside the shared implementation namespace by Native.cpp.

constexpr int kMoaningAudioCategoryCount = 6;
constexpr int kMoaningAudioFilesPerPage = 8;
constexpr const char* kMoaningAudioCategoryNames[kMoaningAudioCategoryCount] = {
    "gasping",
    "low",
    "medium",
    "high",
    "peak",
    "orgasm",
};
constexpr const char* kMoaningAudioCategoryLabels[kMoaningAudioCategoryCount] = {
    "GASPING",
    "LOW",
    "MEDIUM",
    "HIGH",
    "PEAK",
    "ORGASM",
};

struct MoaningAudioFile
{
    std::wstring path;
    std::string name;
    Il2CppObject* clip = nullptr;
    float length = 0.0f;
    bool loadFailed = false;
};

struct MoaningAudioSelection
{
    int category = -1;
    int file = -1;
};

std::vector<MoaningAudioFile> g_moaningAudioFiles[kMoaningAudioCategoryCount];
std::vector<MoaningAudioSelection> g_moaningAudioFlat;
bool g_moaningAudioInitialized = false;
Il2CppObject* g_moaningAudioSourceObject = nullptr;
Il2CppObject* g_moaningAudioSource = nullptr;
int g_moaningAudioCurrentCategory = -1;
int g_moaningAudioCurrentFile = -1;
int g_moaningAudioDebugPage = 0;
std::chrono::steady_clock::time_point g_moaningAudioStartedAt{};
float g_moaningAudioCurrentLength = 0.0f;
bool g_moaningAudioFinished = false;
std::wstring g_moaningAudioStatus = L"Audio pack not scanned yet.";
std::mt19937 g_moaningAudioRandom(
    static_cast<uint32_t>(
        std::chrono::high_resolution_clock::now()
            .time_since_epoch()
            .count()));

uint16_t MoaningAudio_ReadU16(
    const std::vector<unsigned char>& bytes,
    size_t offset)
{
    if (offset + 2 > bytes.size())
    {
        return 0;
    }
    return static_cast<uint16_t>(
        static_cast<uint16_t>(bytes[offset]) |
        (static_cast<uint16_t>(bytes[offset + 1]) << 8));
}

uint32_t MoaningAudio_ReadU32(
    const std::vector<unsigned char>& bytes,
    size_t offset)
{
    if (offset + 4 > bytes.size())
    {
        return 0;
    }
    return static_cast<uint32_t>(bytes[offset]) |
           (static_cast<uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<uint32_t>(bytes[offset + 3]) << 24);
}

bool MoaningAudio_ReadFileBytes(
    const std::wstring& path,
    std::vector<unsigned char>& bytes)
{
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
        LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio could not open %s.", path.c_str());
        return false;
    }

    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) ||
        size.QuadPart <= 0 ||
        size.QuadPart > 16 * 1024 * 1024)
    {
        CloseHandle(file);
        LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio rejected file size for %s.", path.c_str());
        return false;
    }

    bytes.resize(static_cast<size_t>(size.QuadPart));
    DWORD read = 0;
    const bool success =
        ReadFile(
            file,
            bytes.data(),
            static_cast<DWORD>(bytes.size()),
            &read,
            nullptr) &&
        read == bytes.size();
    CloseHandle(file);
    if (!success)
    {
        bytes.clear();
        LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio could not read %s.", path.c_str());
    }
    return success;
}

Il2CppObject* MoaningAudio_CreateClip(
    MoaningAudioFile& entry)
{
    if (entry.clip != nullptr)
    {
        return entry.clip;
    }
    if (entry.loadFailed)
    {
        return nullptr;
    }

    std::vector<unsigned char> bytes;
    if (!MoaningAudio_ReadFileBytes(entry.path, bytes) ||
        bytes.size() < 44 ||
        std::memcmp(bytes.data(), "RIFF", 4) != 0 ||
        std::memcmp(bytes.data() + 8, "WAVE", 4) != 0)
    {
        entry.loadFailed = true;
        return nullptr;
    }

    uint16_t audioFormat = 0;
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint16_t blockAlign = 0;
    uint16_t bitsPerSample = 0;
    size_t dataOffset = 0;
    uint32_t dataSize = 0;

    size_t offset = 12;
    while (offset + 8 <= bytes.size())
    {
        const uint32_t chunkSize = MoaningAudio_ReadU32(bytes, offset + 4);
        const size_t chunkData = offset + 8;
        if (chunkData > bytes.size() ||
            chunkSize > bytes.size() - chunkData)
        {
            break;
        }

        if (std::memcmp(bytes.data() + offset, "fmt ", 4) == 0 &&
            chunkSize >= 16)
        {
            audioFormat = MoaningAudio_ReadU16(bytes, chunkData);
            channels = MoaningAudio_ReadU16(bytes, chunkData + 2);
            sampleRate = MoaningAudio_ReadU32(bytes, chunkData + 4);
            blockAlign = MoaningAudio_ReadU16(bytes, chunkData + 12);
            bitsPerSample = MoaningAudio_ReadU16(bytes, chunkData + 14);
            if (audioFormat == 0xFFFE && chunkSize >= 40)
            {
                audioFormat = MoaningAudio_ReadU16(bytes, chunkData + 24);
            }
        }
        else if (std::memcmp(bytes.data() + offset, "data", 4) == 0)
        {
            dataOffset = chunkData;
            dataSize = chunkSize;
        }

        size_t next = chunkData + chunkSize;
        if ((chunkSize & 1u) != 0)
        {
            ++next;
        }
        if (next <= offset)
        {
            break;
        }
        offset = next;
    }

    const bool supportedEncoding =
        (audioFormat == 1 &&
         (bitsPerSample == 16 ||
          bitsPerSample == 24 ||
          bitsPerSample == 32)) ||
        (audioFormat == 3 && bitsPerSample == 32);
    if (!supportedEncoding ||
        channels < 1 ||
        channels > 2 ||
        sampleRate < 8000 ||
        sampleRate > 192000 ||
        blockAlign == 0 ||
        dataOffset == 0 ||
        dataSize == 0)
    {
        entry.loadFailed = true;
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Moaning audio unsupported WAV %s: format=%u channels=%u "
            L"rate=%u bits=%u.",
            entry.path.c_str(),
            audioFormat,
            channels,
            sampleRate,
            bitsPerSample);
        return nullptr;
    }

    const size_t bytesPerSample = bitsPerSample / 8;
    const size_t expectedBlockAlign =
        static_cast<size_t>(channels) * bytesPerSample;
    if (blockAlign != expectedBlockAlign)
    {
        entry.loadFailed = true;
        LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio has an invalid block alignment: %s.", entry.path.c_str());
        return nullptr;
    }

    const size_t frameCount = dataSize / blockAlign;
    const size_t sampleCount = frameCount * channels;
    if (frameCount == 0 ||
        frameCount > static_cast<size_t>(INT32_MAX) ||
        sampleCount == 0)
    {
        entry.loadFailed = true;
        return nullptr;
    }

    auto* singleClass = FindClass("System", "Single");
    if (singleClass == nullptr ||
        il2cpp_array_new == nullptr ||
        il2cpp_array_object_header_size == nullptr)
    {
        entry.loadFailed = true;
        return nullptr;
    }
    il2cpp_runtime_class_init(singleClass);
    auto* samples = il2cpp_array_new(singleClass, sampleCount);
    if (samples == nullptr)
    {
        entry.loadFailed = true;
        return nullptr;
    }
    auto* output = reinterpret_cast<float*>(
        reinterpret_cast<char*>(samples) +
        il2cpp_array_object_header_size());

    for (size_t index = 0; index < sampleCount; ++index)
    {
        const size_t sampleOffset = dataOffset + index * bytesPerSample;
        if (audioFormat == 3)
        {
            std::memcpy(output + index, bytes.data() + sampleOffset, 4);
        }
        else if (bitsPerSample == 16)
        {
            const int16_t value = static_cast<int16_t>(
                MoaningAudio_ReadU16(bytes, sampleOffset));
            output[index] = static_cast<float>(value) / 32768.0f;
        }
        else if (bitsPerSample == 24)
        {
            int32_t value =
                static_cast<int32_t>(bytes[sampleOffset]) |
                (static_cast<int32_t>(bytes[sampleOffset + 1]) << 8) |
                (static_cast<int32_t>(bytes[sampleOffset + 2]) << 16);
            if ((value & 0x00800000) != 0)
            {
                value |= static_cast<int32_t>(0xFF000000);
            }
            output[index] =
                static_cast<float>(value) / 8388608.0f;
        }
        else
        {
            const int32_t value = static_cast<int32_t>(
                MoaningAudio_ReadU32(bytes, sampleOffset));
            output[index] =
                static_cast<float>(
                    static_cast<double>(value) / 2147483648.0);
        }
    }

    auto* audioClipClass = FindClass("UnityEngine", "AudioClip");
    if (audioClipClass == nullptr || il2cpp_string_new == nullptr)
    {
        entry.loadFailed = true;
        return nullptr;
    }
    il2cpp_runtime_class_init(audioClipClass);
    auto* clipName = il2cpp_string_new(entry.name.c_str());
    int lengthSamples = static_cast<int>(frameCount);
    int clipChannels = static_cast<int>(channels);
    int frequency = static_cast<int>(sampleRate);
    bool stream = false;
    void* createParameters[] = {
        clipName,
        &lengthSamples,
        &clipChannels,
        &frequency,
        &stream,
    };
    auto* clip = Invoke(
        FindMethod(audioClipClass, "Create", 5),
        nullptr,
        createParameters);
    int offsetSamples = 0;
    void* setDataParameters[] = { samples, &offsetSamples };
    auto* setDataResult = clip != nullptr
        ? Invoke(
            FindMethod(audioClipClass, "SetData", 2),
            clip,
            setDataParameters)
        : nullptr;
    if (clip == nullptr ||
        setDataResult == nullptr ||
        !*static_cast<bool*>(il2cpp_object_unbox(setDataResult)))
    {
        entry.loadFailed = true;
        LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio could not create an AudioClip for %s.", entry.path.c_str());
        return nullptr;
    }

    auto* lengthObject = Invoke(
        FindMethod(audioClipClass, "get_length", 0),
        clip,
        nullptr);
    entry.length = lengthObject != nullptr
        ? *static_cast<float*>(il2cpp_object_unbox(lengthObject))
        : static_cast<float>(frameCount) / static_cast<float>(sampleRate);
    entry.clip = clip;
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Moaning audio loaded %s: %.2f seconds, %u Hz, %u channel(s).",
        entry.path.c_str(),
        entry.length,
        sampleRate,
        channels);
    return entry.clip;
}

void MoaningAudio_Initialize()
{
    if (g_moaningAudioInitialized)
    {
        return;
    }
    g_moaningAudioInitialized = true;

    const std::wstring packRoot =
        GetEscapeFromManakaSubdirectory(L"Audio") +
        L"\\Moaning3\\ryuzen-nikomi-2";
    g_moaningAudioFlat.clear();
    int totalFiles = 0;
    for (int category = 0;
         category < kMoaningAudioCategoryCount;
         ++category)
    {
        auto& files = g_moaningAudioFiles[category];
        files.clear();
        const std::wstring categoryRoot =
            packRoot + L"\\" +
            Utf8ToWide(kMoaningAudioCategoryNames[category]);
        const std::wstring pattern = categoryRoot + L"\\*.wav";
        WIN32_FIND_DATAW data{};
        HANDLE find = FindFirstFileW(pattern.c_str(), &data);
        if (find == INVALID_HANDLE_VALUE)
        {
            LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio category unavailable: %s.", categoryRoot.c_str());
            continue;
        }
        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                continue;
            }
            MoaningAudioFile entry{};
            entry.path = categoryRoot + L"\\" + data.cFileName;
            entry.name = WideToUtf8(data.cFileName);
            files.push_back(entry);
        }
        while (FindNextFileW(find, &data));
        FindClose(find);
        std::sort(
            files.begin(),
            files.end(),
            [](const MoaningAudioFile& left, const MoaningAudioFile& right)
            {
                return left.name < right.name;
            });
        totalFiles += static_cast<int>(files.size());
        for (int file = 0;
             file < static_cast<int>(files.size());
             ++file)
        {
            g_moaningAudioFlat.push_back({ category, file });
        }
        LogInfo(LogCategory_Asset, L"Asset", 
            L"Moaning audio category %S: %u file(s).",
            kMoaningAudioCategoryNames[category],
            static_cast<unsigned>(files.size()));
    }

    if (totalFiles > 0)
    {
        wchar_t status[256]{};
        _snwprintf_s(
            status,
            _TRUNCATE,
            L"Audio pack ready: %d files\n%s",
            totalFiles,
            packRoot.c_str());
        g_moaningAudioStatus = status;
    }
    else
    {
        g_moaningAudioStatus =
            L"Audio pack not found.\nRun the current EscapeFromManaka installer.";
    }
}

bool MoaningAudio_EnsureSource()
{
    if (g_moaningAudioSource != nullptr)
    {
        return true;
    }

    g_moaningAudioSourceObject =
        CreateSceneGameObject("EFM_Moaning3_Audio", nullptr);
    if (g_moaningAudioSourceObject == nullptr)
    {
        return false;
    }
    g_moaningAudioSource = AddSceneComponentToGameObject(
        g_moaningAudioSourceObject,
        "UnityEngine",
        "AudioSource");
    if (g_moaningAudioSource == nullptr)
    {
        g_moaningAudioSourceObject = nullptr;
        return false;
    }

    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    bool playOnAwake = false;
    bool loop = false;
    bool muted = false;
    float volume = 1.0f;
    void* playOnAwakeParameters[] = { &playOnAwake };
    void* loopParameters[] = { &loop };
    void* muteParameters[] = { &muted };
    void* volumeParameters[] = { &volume };
    Invoke(
        FindMethod(audioSourceClass, "set_playOnAwake", 1),
        g_moaningAudioSource,
        playOnAwakeParameters);
    Invoke(
        FindMethod(audioSourceClass, "set_loop", 1),
        g_moaningAudioSource,
        loopParameters);
    Invoke(
        FindMethod(audioSourceClass, "set_mute", 1),
        g_moaningAudioSource,
        muteParameters);
    Invoke(
        FindMethod(audioSourceClass, "set_volume", 1),
        g_moaningAudioSource,
        volumeParameters);
    LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio player created.");
    return true;
}

bool MoaningAudio_IsManagedSource(Il2CppObject* source)
{
    if (source == nullptr)
    {
        return false;
    }
    if (source == g_moaningAudioSource)
    {
        return true;
    }
    return g_moaningAudioSourceObject != nullptr &&
           GetComponentGameObject(source) == g_moaningAudioSourceObject;
}

void MoaningAudio_Stop()
{
    if (g_moaningAudioSource != nullptr)
    {
        Invoke(
            FindMethod(
                FindClass("UnityEngine", "AudioSource"),
                "Stop",
                0),
            g_moaningAudioSource,
            nullptr);
    }
    g_moaningAudioCurrentCategory = -1;
    g_moaningAudioCurrentFile = -1;
    g_moaningAudioCurrentLength = 0.0f;
    g_moaningAudioFinished = false;
    g_moaningAudioStartedAt = {};
    g_moaningAudioStatus = L"Playback stopped.";
}

void MoaningAudio_RefreshDebugPanel()
{
    MoaningAudio_Initialize();

    const auto applyLabels = [](std::vector<Il2CppObject*>& labels)
    {
        for (int slot = 0;
             slot < kMoaningAudioFilesPerPage;
             ++slot)
        {
            if (slot >= static_cast<int>(labels.size()) ||
                labels[slot] == nullptr)
            {
                continue;
            }
            const int flatIndex =
                g_moaningAudioDebugPage *
                    kMoaningAudioFilesPerPage +
                slot;
            char label[192]{};
            if (flatIndex >= 0 &&
                flatIndex < static_cast<int>(g_moaningAudioFlat.size()))
            {
                const auto& selection = g_moaningAudioFlat[flatIndex];
                const auto& file =
                    g_moaningAudioFiles[selection.category][selection.file];
                std::snprintf(
                    label,
                    sizeof(label),
                    "%s · %s\nCLICK TO PLAY",
                    kMoaningAudioCategoryLabels[selection.category],
                    file.name.c_str());
            }
            SetSceneText(
                labels[slot],
                label,
                12.0f,
                514,
                0.92f,
                0.94f,
                0.96f,
                1.0f);
        }
    };
    applyLabels(g_efmActionLabels);
    applyLabels(g_extendedActionLabels);

    std::wstring status = g_moaningAudioStatus;
    if (g_moaningAudioCurrentCategory >= 0 &&
        g_moaningAudioCurrentCategory < kMoaningAudioCategoryCount &&
        g_moaningAudioCurrentFile >= 0 &&
        g_moaningAudioCurrentFile <
            static_cast<int>(
                g_moaningAudioFiles[g_moaningAudioCurrentCategory].size()))
    {
        const auto& file =
            g_moaningAudioFiles[g_moaningAudioCurrentCategory]
                               [g_moaningAudioCurrentFile];
        const float rawElapsed = g_moaningAudioStartedAt
                .time_since_epoch()
                .count() != 0
            ? std::chrono::duration<float>(
                std::chrono::steady_clock::now() -
                g_moaningAudioStartedAt).count()
            : 0.0f;
        const float elapsed = std::min(
            rawElapsed,
            g_moaningAudioCurrentLength);
        wchar_t playing[512]{};
        _snwprintf_s(
            playing,
            _TRUNCATE,
            L"%S / %S  %.1fs / %.1fs\n%s",
            kMoaningAudioCategoryLabels[g_moaningAudioCurrentCategory],
            file.name.c_str(),
            elapsed,
            g_moaningAudioCurrentLength,
            !g_moaningAudioFinished &&
                elapsed < g_moaningAudioCurrentLength
                ? L"PLAYING"
                : L"FINISHED  ·  ~ CLOSE");
        status = playing;
    }
    const int pageCount = std::max<int>(
        1,
        (static_cast<int>(g_moaningAudioFlat.size()) +
         kMoaningAudioFilesPerPage - 1) /
            kMoaningAudioFilesPerPage);
    if (g_moaningAudioDebugPage < 0)
    {
        g_moaningAudioDebugPage = 0;
    }
    else if (g_moaningAudioDebugPage >= pageCount)
    {
        g_moaningAudioDebugPage = pageCount - 1;
    }
    char statusText[640]{};
    std::snprintf(
        statusText,
        sizeof(statusText),
        "PAGE %d / %d   ·   %u FILES\n%s",
        g_moaningAudioDebugPage + 1,
        pageCount,
        static_cast<unsigned>(g_moaningAudioFlat.size()),
        WideToUtf8(status).c_str());
    if (g_efmActionPageLabel != nullptr)
    {
        SetSceneText(
            g_efmActionPageLabel,
            statusText,
            14.0f,
            514,
            0.90f,
            0.92f,
            0.95f,
            1.0f);
    }
    if (g_extendedPageLabel != nullptr)
    {
        SetSceneText(
            g_extendedPageLabel,
            statusText,
            13.0f,
            514,
            0.90f,
            0.92f,
            0.95f,
            1.0f);
    }
    if (g_extendedSelectedActionLabel != nullptr)
    {
        SetSceneText(
            g_extendedSelectedActionLabel,
            "音频试听  /  MOANING3 AUDIO TEST",
            15.0f,
            257,
            0.92f,
            0.94f,
            0.96f,
            1.0f);
    }
}

void MoaningAudio_ChangePage(int delta)
{
    MoaningAudio_Initialize();
    g_moaningAudioDebugPage += delta;
    MoaningAudio_RefreshDebugPanel();
}

bool MoaningAudio_PlayCategory(
    int category,
    bool refreshDebugPanel = true)
{
    MoaningAudio_Initialize();
    if (category < 0 || category >= kMoaningAudioCategoryCount)
    {
        return false;
    }
    auto& files = g_moaningAudioFiles[category];
    if (files.empty())
    {
        g_moaningAudioStatus =
            L"Category has no audio files.\nCheck the installed audio pack.";
        if (refreshDebugPanel)
        {
            MoaningAudio_RefreshDebugPanel();
        }
        return false;
    }

    std::uniform_int_distribution<size_t> distribution(
        0,
        files.size() - 1);
    const size_t index = distribution(g_moaningAudioRandom);
    auto* clip = MoaningAudio_CreateClip(files[index]);
    if (clip == nullptr || !MoaningAudio_EnsureSource())
    {
        g_moaningAudioStatus =
            L"AudioClip creation failed.\nSee EscapeFromManaka\\Logs\\ManakaMultiplayer.Native.log.";
        if (refreshDebugPanel)
        {
            MoaningAudio_RefreshDebugPanel();
        }
        return false;
    }

    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    Invoke(
        FindMethod(audioSourceClass, "Stop", 0),
        g_moaningAudioSource,
        nullptr);
    float volume = 1.0f;
    bool muted = false;
    void* clipParameters[] = { clip };
    void* muteParameters[] = { &muted };
    void* volumeParameters[] = { &volume };
    Invoke(
        FindMethod(audioSourceClass, "set_clip", 1),
        g_moaningAudioSource,
        clipParameters);
    Invoke(
        FindMethod(audioSourceClass, "set_mute", 1),
        g_moaningAudioSource,
        muteParameters);
    Invoke(
        FindMethod(audioSourceClass, "set_volume", 1),
        g_moaningAudioSource,
        volumeParameters);
    Invoke(
        FindMethod(audioSourceClass, "Play", 0),
        g_moaningAudioSource,
        nullptr);

    g_moaningAudioCurrentCategory = category;
    g_moaningAudioCurrentFile = static_cast<int>(index);
    g_moaningAudioCurrentLength = files[index].length;
    g_moaningAudioFinished = false;
    g_moaningAudioStartedAt = std::chrono::steady_clock::now();
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Moaning audio preview %S/%S.",
        kMoaningAudioCategoryNames[category],
        files[index].name.c_str());
    if (refreshDebugPanel)
    {
        MoaningAudio_RefreshDebugPanel();
    }
    return true;
}

bool MoaningAudio_PlayFlatIndex(int flatIndex)
{
    MoaningAudio_Initialize();
    if (flatIndex < 0 ||
        flatIndex >= static_cast<int>(g_moaningAudioFlat.size()))
    {
        return false;
    }
    const auto selection = g_moaningAudioFlat[flatIndex];
    auto& file =
        g_moaningAudioFiles[selection.category][selection.file];
    auto* clip = MoaningAudio_CreateClip(file);
    if (clip == nullptr || !MoaningAudio_EnsureSource())
    {
        g_moaningAudioStatus =
            L"AudioClip creation failed.\nSee EscapeFromManaka\\Logs\\ManakaMultiplayer.Native.log.";
        MoaningAudio_RefreshDebugPanel();
        return false;
    }

    auto* audioSourceClass = FindClass("UnityEngine", "AudioSource");
    Invoke(
        FindMethod(audioSourceClass, "Stop", 0),
        g_moaningAudioSource,
        nullptr);
    float volume = 1.0f;
    bool muted = false;
    void* clipParameters[] = { clip };
    void* muteParameters[] = { &muted };
    void* volumeParameters[] = { &volume };
    Invoke(
        FindMethod(audioSourceClass, "set_clip", 1),
        g_moaningAudioSource,
        clipParameters);
    Invoke(
        FindMethod(audioSourceClass, "set_mute", 1),
        g_moaningAudioSource,
        muteParameters);
    Invoke(
        FindMethod(audioSourceClass, "set_volume", 1),
        g_moaningAudioSource,
        volumeParameters);
    Invoke(
        FindMethod(audioSourceClass, "Play", 0),
        g_moaningAudioSource,
        nullptr);

    g_moaningAudioCurrentCategory = selection.category;
    g_moaningAudioCurrentFile = selection.file;
    g_moaningAudioCurrentLength = file.length;
    g_moaningAudioFinished = false;
    g_moaningAudioStartedAt = std::chrono::steady_clock::now();
    LogInfo(LogCategory_Asset, L"Asset", 
        L"Moaning audio preview %S/%S.",
        kMoaningAudioCategoryNames[selection.category],
        file.name.c_str());
    MoaningAudio_RefreshDebugPanel();
    return true;
}

void MoaningAudio_PlayRandom()
{
    MoaningAudio_Initialize();
    if (g_moaningAudioFlat.empty())
    {
        g_moaningAudioStatus = L"No WAV files were found.";
        MoaningAudio_RefreshDebugPanel();
        return;
    }
    std::uniform_int_distribution<size_t> distribution(
        0,
        g_moaningAudioFlat.size() - 1);
    const size_t index = distribution(g_moaningAudioRandom);
    g_moaningAudioDebugPage =
        static_cast<int>(index) / kMoaningAudioFilesPerPage;
    MoaningAudio_PlayFlatIndex(static_cast<int>(index));
}

void MoaningAudio_PlayDebugSlot(int slot)
{
    const int flatIndex =
        g_moaningAudioDebugPage *
            kMoaningAudioFilesPerPage +
        slot;
    MoaningAudio_PlayFlatIndex(flatIndex);
}

void MoaningAudio_TickDebugPanel()
{
    if (!g_efmActionListOpen &&
        !g_extendedActionListOpen.load())
    {
        return;
    }
    if (!g_moaningAudioFinished &&
        g_moaningAudioCurrentCategory >= 0 &&
        g_moaningAudioStartedAt.time_since_epoch().count() != 0)
    {
        const float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() -
            g_moaningAudioStartedAt).count();
        if (elapsed >= g_moaningAudioCurrentLength)
        {
            g_moaningAudioFinished = true;
            if (g_moaningAudioSource != nullptr)
            {
                Invoke(
                    FindMethod(
                        FindClass("UnityEngine", "AudioSource"),
                        "Stop",
                        0),
                    g_moaningAudioSource,
                    nullptr);
            }
            LogInfo(LogCategory_Asset, L"Asset", L"Moaning audio preview finished.");
        }
    }
    static unsigned int tick = 0;
    ++tick;
    if ((tick % 15u) == 0)
    {
        MoaningAudio_RefreshDebugPanel();
    }
}

void MoaningAudio_Release()
{
    MoaningAudio_Stop();
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* destroy = objectClass != nullptr
        ? FindMethod(objectClass, "Destroy", 1)
        : nullptr;
    if (destroy != nullptr)
    {
        for (auto& files : g_moaningAudioFiles)
        {
            for (auto& entry : files)
            {
                if (entry.clip == nullptr)
                {
                    continue;
                }
                void* parameters[] = { entry.clip };
                Invoke(destroy, nullptr, parameters);
                entry.clip = nullptr;
                entry.length = 0.0f;
                entry.loadFailed = false;
            }
        }
        if (g_moaningAudioSourceObject != nullptr)
        {
            void* parameters[] = { g_moaningAudioSourceObject };
            Invoke(destroy, nullptr, parameters);
        }
    }
    g_moaningAudioSourceObject = nullptr;
    g_moaningAudioSource = nullptr;
    g_moaningAudioInitialized = false;
    g_moaningAudioStatus = L"Audio pack released.";
}
