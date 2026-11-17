// Native.cpp module: logging.inc
// Included inside the shared implementation namespace by Native.cpp.

enum LogLevel
{
    LogLevel_Trace = 0,
    LogLevel_Debug,
    LogLevel_Info,
    LogLevel_Warn,
    LogLevel_Error,
};

enum LogCategory
{
    LogCategory_General = 0,
    LogCategory_Lifecycle,
    LogCategory_Hook,
    LogCategory_Input,
    LogCategory_Action,
    LogCategory_Animation,
    LogCategory_Dialogue,
    LogCategory_Inventory,
    LogCategory_Ui,
    LogCategory_Scene,
    LogCategory_Network,
    LogCategory_Persistence,
    LogCategory_Asset,
    LogCategory_Checkpoint,
};

struct LogRecord
{
    unsigned long long sequence = 0;
    FILETIME timestamp{};
    unsigned long threadId = 0;
    LogLevel level = LogLevel_Info;
    LogCategory category = LogCategory_General;
    std::wstring eventName;
    std::wstring message;
    size_t retainedBytes = 0;
};

constexpr size_t kLogQueueMaxBytes = 8u * 1024u * 1024u;
constexpr size_t kLogBatchThresholdBytes = 64u * 1024u;
constexpr size_t kLogBatchMaxRecords = 512u;
constexpr size_t kLogMessageCapacity = 4096u;
constexpr size_t kLogRecordOverheadBytes = 192u;

std::mutex g_logQueueMutex;
std::condition_variable g_logQueueCondition;
std::deque<LogRecord> g_logQueue;
std::thread g_logWriterThread;
std::atomic<bool> g_logRunning{false};
std::atomic<bool> g_logStopRequested{false};
std::atomic<bool> g_logUrgentRequested{false};
std::atomic<size_t> g_logQueuedBytes{0};
std::atomic<unsigned long long> g_logNextSequence{1};
std::atomic<unsigned long long> g_logDroppedTrace{0};
std::atomic<unsigned long long> g_logDroppedDebug{0};
std::atomic<unsigned long long> g_logDroppedInfo{0};
std::atomic<unsigned long long> g_logDroppedCritical{0};

std::mutex g_logHelperMutex;
std::unordered_set<std::string> g_logOnceKeys;
std::unordered_map<std::string, unsigned long long> g_logRateLimitTicks;
std::unordered_map<std::string, std::wstring> g_logChangedValues;

const wchar_t* Log_LevelName(LogLevel level)
{
    switch (level)
    {
    case LogLevel_Trace:
        return L"TRACE";
    case LogLevel_Debug:
        return L"DEBUG";
    case LogLevel_Info:
        return L"INFO";
    case LogLevel_Warn:
        return L"WARN";
    case LogLevel_Error:
        return L"ERROR";
    default:
        return L"INFO";
    }
}

const wchar_t* Log_CategoryName(LogCategory category)
{
    switch (category)
    {
    case LogCategory_Lifecycle:
        return L"LIFECYCLE";
    case LogCategory_Hook:
        return L"HOOK";
    case LogCategory_Input:
        return L"INPUT";
    case LogCategory_Action:
        return L"ACTION";
    case LogCategory_Animation:
        return L"ANIMATION";
    case LogCategory_Dialogue:
        return L"DIALOGUE";
    case LogCategory_Inventory:
        return L"INVENTORY";
    case LogCategory_Ui:
        return L"UI";
    case LogCategory_Scene:
        return L"SCENE";
    case LogCategory_Network:
        return L"NETWORK";
    case LogCategory_Persistence:
        return L"PERSISTENCE";
    case LogCategory_Asset:
        return L"ASSET";
    case LogCategory_Checkpoint:
        return L"CHECKPOINT";
    default:
        return L"GENERAL";
    }
}

FILETIME Log_CurrentTimestamp()
{
    FILETIME value{};
    GetSystemTimeAsFileTime(&value);
    return value;
}

std::wstring Log_GetLogDirectory()
{
    wchar_t modulePath[MAX_PATH]{};
    GetModuleFileNameW(g_instance, modulePath, MAX_PATH);
    std::wstring moduleDirectory(modulePath);
    const auto separator = moduleDirectory.find_last_of(L"\\/");
    if (separator != std::wstring::npos)
    {
        moduleDirectory.resize(separator);
    }
    const std::wstring packageDirectory =
        moduleDirectory + L"\\EscapeFromManaka";
    const std::wstring logDirectory =
        packageDirectory + L"\\Logs";
    CreateDirectoryW(packageDirectory.c_str(), nullptr);
    CreateDirectoryW(logDirectory.c_str(), nullptr);
    return logDirectory;
}

void Log_FallbackDebug(const wchar_t* message)
{
    if (message != nullptr)
    {
        OutputDebugStringW(message);
        OutputDebugStringW(L"\n");
    }
}

void Log_IncrementDropped(LogLevel level)
{
    switch (level)
    {
    case LogLevel_Trace:
        ++g_logDroppedTrace;
        break;
    case LogLevel_Debug:
        ++g_logDroppedDebug;
        break;
    case LogLevel_Info:
        ++g_logDroppedInfo;
        break;
    default:
        ++g_logDroppedCritical;
        break;
    }
}

bool Log_ShouldDropForWatermark(LogLevel level, size_t queuedBytes)
{
    if (level == LogLevel_Trace &&
        queuedBytes >= kLogQueueMaxBytes * 70u / 100u)
    {
        return true;
    }
    if (level == LogLevel_Debug &&
        queuedBytes >= kLogQueueMaxBytes * 85u / 100u)
    {
        return true;
    }
    if (level == LogLevel_Info &&
        queuedBytes >= kLogQueueMaxBytes * 95u / 100u)
    {
        return true;
    }
    return false;
}

bool Log_EnqueueRecord(LogRecord&& record, size_t retainedBytes)
{
    if (!g_logRunning.load(std::memory_order_acquire))
    {
        Log_FallbackDebug(record.message.c_str());
        return false;
    }
    if (Log_ShouldDropForWatermark(
            record.level,
            g_logQueuedBytes.load(std::memory_order_relaxed)))
    {
        Log_IncrementDropped(record.level);
        return false;
    }

    record.retainedBytes = retainedBytes;
    const bool urgentRecord = record.level >= LogLevel_Warn;
    bool queued = false;
    {
        std::lock_guard<std::mutex> lock(g_logQueueMutex);
        if (!g_logRunning.load(std::memory_order_relaxed))
        {
            Log_FallbackDebug(record.message.c_str());
            return false;
        }

        while (!g_logQueue.empty() &&
               g_logQueuedBytes.load(std::memory_order_relaxed) +
                       retainedBytes >
                   kLogQueueMaxBytes)
        {
            auto victim = std::find_if(
                g_logQueue.begin(),
                g_logQueue.end(),
                [](const LogRecord& candidate)
                {
                    return candidate.level <= LogLevel_Info;
                });
            if (victim == g_logQueue.end())
            {
                break;
            }
            g_logQueuedBytes.fetch_sub(
                victim->retainedBytes,
                std::memory_order_relaxed);
            Log_IncrementDropped(victim->level);
            g_logQueue.erase(victim);
        }

        if (g_logQueuedBytes.load(std::memory_order_relaxed) +
                retainedBytes >
            kLogQueueMaxBytes)
        {
            Log_IncrementDropped(record.level);
            Log_FallbackDebug(record.message.c_str());
            return false;
        }

        record.sequence =
            g_logNextSequence.fetch_add(
                1,
                std::memory_order_relaxed);
        g_logQueuedBytes.fetch_add(
            retainedBytes,
            std::memory_order_relaxed);
        g_logQueue.emplace_back(std::move(record));
        queued = true;
    }

    if (queued)
    {
        if (urgentRecord)
        {
            g_logUrgentRequested.store(
                true,
                std::memory_order_release);
        }
        g_logQueueCondition.notify_one();
    }
    return queued;
}

void Log_FormatAndEnqueue(
    LogLevel level,
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    va_list arguments)
{
    wchar_t buffer[kLogMessageCapacity]{};
    const wchar_t* safeFormat =
        format != nullptr ? format : L"<null log format>";
    const int written = _vsnwprintf_s(
        buffer,
        _countof(buffer),
        _TRUNCATE,
        safeFormat,
        arguments);
    if (written < 0)
    {
        constexpr size_t markerLength = 12;
        const size_t offset =
            _countof(buffer) - markerLength - 1;
        buffer[offset] = L'\0';
        wcscat_s(buffer, L"...truncated");
    }

    LogRecord record;
    record.timestamp = Log_CurrentTimestamp();
    record.threadId = GetCurrentThreadId();
    record.level = level;
    record.category = category;
    record.eventName =
        eventName != nullptr ? eventName : L"";
    record.message = buffer;

    const size_t retainedBytes =
        kLogRecordOverheadBytes +
        (record.eventName.size() + record.message.size()) *
            sizeof(wchar_t);
    Log_EnqueueRecord(std::move(record), retainedBytes);
}

void LogEvent(
    LogLevel level,
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        level,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogTrace(
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        LogLevel_Trace,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogDebug(
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        LogLevel_Debug,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogInfo(
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        LogLevel_Info,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogWarn(
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        LogLevel_Warn,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogError(
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        LogLevel_Error,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogOnce(
    const char* key,
    LogLevel level,
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    if (key == nullptr)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(g_logHelperMutex);
        if (!g_logOnceKeys.emplace(key).second)
        {
            return;
        }
    }
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        level,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogRateLimited(
    const char* key,
    unsigned long long intervalMs,
    LogLevel level,
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* format,
    ...)
{
    if (key == nullptr)
    {
        return;
    }
    const unsigned long long now = GetTickCount64();
    {
        std::lock_guard<std::mutex> lock(g_logHelperMutex);
        auto& last = g_logRateLimitTicks[key];
        if (last != 0 && now - last < intervalMs)
        {
            return;
        }
        last = now;
    }
    va_list arguments;
    va_start(arguments, format);
    Log_FormatAndEnqueue(
        level,
        category,
        eventName,
        format,
        arguments);
    va_end(arguments);
}

void LogChanged(
    const char* key,
    LogLevel level,
    LogCategory category,
    const wchar_t* eventName,
    const wchar_t* oldValue,
    const wchar_t* newValue,
    const wchar_t* format,
    ...)
{
    if (key == nullptr)
    {
        return;
    }
    const std::wstring next =
        newValue != nullptr ? newValue : L"";
    {
        std::lock_guard<std::mutex> lock(g_logHelperMutex);
        auto iterator = g_logChangedValues.find(key);
        if (iterator != g_logChangedValues.end() &&
            iterator->second == next)
        {
            return;
        }
        g_logChangedValues[key] = next;
    }

    wchar_t buffer[kLogMessageCapacity]{};
    va_list arguments;
    va_start(arguments, format);
    _vsnwprintf_s(
        buffer,
        _countof(buffer),
        _TRUNCATE,
        format != nullptr ? format : L"",
        arguments);
    va_end(arguments);

    wchar_t changed[kLogMessageCapacity]{};
    swprintf_s(
        changed,
        L"%s old=%s new=%s",
        buffer,
        oldValue != nullptr ? oldValue : L"",
        newValue != nullptr ? newValue : L"");
    LogEvent(level, category, eventName, L"%s", changed);
}

bool Log_WriteAll(HANDLE file, const char* data, size_t size)
{
    size_t writtenTotal = 0;
    while (writtenTotal < size)
    {
        const DWORD remaining = static_cast<DWORD>(
            std::min<size_t>(
                size - writtenTotal,
                static_cast<size_t>(0x7FFFFFFFu)));
        DWORD written = 0;
        if (!WriteFile(
                file,
                data + writtenTotal,
                remaining,
                &written,
                nullptr) ||
            written == 0)
        {
            return false;
        }
        writtenTotal += written;
    }
    return true;
}

std::string Log_WideToUtf8(const std::wstring& value)
{
    if (value.empty())
    {
        return std::string();
    }
    const int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (size <= 0)
    {
        return std::string();
    }
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        size,
        nullptr,
        nullptr);
    return result;
}

std::string Log_FormatRecordUtf8(const LogRecord& record)
{
    SYSTEMTIME time{};
    FileTimeToSystemTime(&record.timestamp, &time);
    wchar_t prefix[256]{};
    swprintf_s(
        prefix,
        L"%04u-%02u-%02uT%02u:%02u:%02u.%03uZ [%s][%s]",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds,
        Log_LevelName(record.level),
        Log_CategoryName(record.category));

    std::wstring eventSuffix;
    if (!record.eventName.empty())
    {
        eventSuffix = L"[";
        eventSuffix += record.eventName;
        eventSuffix += L"]";
    }

    std::wstring normalized;
    normalized.reserve(record.message.size());
    for (wchar_t character : record.message)
    {
        if (character == L'\r')
        {
            continue;
        }
        if (character == L'\n')
        {
            normalized += L"\\n";
        }
        else
        {
            normalized.push_back(character);
        }
    }

    wchar_t metadata[128]{};
    swprintf_s(
        metadata,
        L"[seq=%08llu pid=%lu tid=%lu]",
        record.sequence,
        GetCurrentProcessId(),
        record.threadId);

    std::wstring line(prefix);
    line += eventSuffix;
    line += metadata;
    line.push_back(L' ');
    line += normalized;
    line += L"\r\n";
    return Log_WideToUtf8(line);
}

void Log_AppendDroppedSummary(
    std::string& pending,
    unsigned long long& reportedTrace,
    unsigned long long& reportedDebug,
    unsigned long long& reportedInfo,
    unsigned long long& reportedCritical)
{
    const unsigned long long trace =
        g_logDroppedTrace.load(std::memory_order_relaxed);
    const unsigned long long debug =
        g_logDroppedDebug.load(std::memory_order_relaxed);
    const unsigned long long info =
        g_logDroppedInfo.load(std::memory_order_relaxed);
    const unsigned long long critical =
        g_logDroppedCritical.load(std::memory_order_relaxed);
    if (trace == reportedTrace &&
        debug == reportedDebug &&
        info == reportedInfo &&
        critical == reportedCritical)
    {
        return;
    }

    LogRecord record;
    record.sequence =
        g_logNextSequence.fetch_add(1, std::memory_order_relaxed);
    record.timestamp = Log_CurrentTimestamp();
    record.threadId = GetCurrentThreadId();
    record.level = LogLevel_Warn;
    record.category = LogCategory_General;
    record.eventName = L"QueueOverload";
    wchar_t message[512]{};
    swprintf_s(
        message,
        L"Log queue dropped records: trace=%llu debug=%llu info=%llu critical=%llu.",
        trace - reportedTrace,
        debug - reportedDebug,
        info - reportedInfo,
        critical - reportedCritical);
    record.message = message;
    pending += Log_FormatRecordUtf8(record);
    reportedTrace = trace;
    reportedDebug = debug;
    reportedInfo = info;
    reportedCritical = critical;
}

void Log_WriterThreadMain(std::wstring logPath)
{
    HANDLE file = CreateFileW(
        logPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        g_logRunning.store(false, std::memory_order_release);
        Log_FallbackDebug(
            L"Escape logging thread could not create the native log file.");
        return;
    }

    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    if (!Log_WriteAll(
            file,
            reinterpret_cast<const char*>(bom),
            sizeof(bom)))
    {
        CloseHandle(file);
        g_logRunning.store(false, std::memory_order_release);
        Log_FallbackDebug(
            L"Escape logging thread could not write the UTF-8 BOM.");
        return;
    }

    std::string pending;
    pending.reserve(kLogBatchThresholdBytes * 2);
    unsigned long long reportedTrace = 0;
    unsigned long long reportedDebug = 0;
    unsigned long long reportedInfo = 0;
    unsigned long long reportedCritical = 0;
    auto lastFlush = std::chrono::steady_clock::now();

    while (true)
    {
        std::deque<LogRecord> batch;
        bool stopping = false;
        bool urgent = false;
        {
            std::unique_lock<std::mutex> lock(g_logQueueMutex);
            const auto ready = [&]()
            {
                return g_logStopRequested.load(std::memory_order_acquire) ||
                    g_logUrgentRequested.load(std::memory_order_acquire) ||
                    g_logQueuedBytes.load(std::memory_order_relaxed) >=
                        kLogBatchThresholdBytes;
            };
            if (g_logQueue.empty() && !ready())
            {
                g_logQueueCondition.wait(
                    lock,
                    [&]()
                    {
                        return ready() || !g_logQueue.empty();
                    });
            }
            if (!g_logQueue.empty() && !ready())
            {
                g_logQueueCondition.wait_for(
                    lock,
                    std::chrono::milliseconds(100),
                    ready);
            }

            stopping =
                g_logStopRequested.load(std::memory_order_acquire);
            urgent =
                g_logUrgentRequested.exchange(
                    false,
                    std::memory_order_acq_rel);
            size_t movedBytes = 0;
            while (!g_logQueue.empty() &&
                   batch.size() < kLogBatchMaxRecords)
            {
                auto& record = g_logQueue.front();
                movedBytes += record.retainedBytes;
                batch.emplace_back(std::move(record));
                g_logQueue.pop_front();
            }
            if (movedBytes > 0)
            {
                g_logQueuedBytes.fetch_sub(
                    movedBytes,
                    std::memory_order_relaxed);
            }
        }

        for (const auto& record : batch)
        {
            const std::string debugLine =
                Log_FormatRecordUtf8(record);
            const int debugLength = MultiByteToWideChar(
                CP_UTF8,
                0,
                debugLine.data(),
                static_cast<int>(debugLine.size()),
                nullptr,
                0);
            if (debugLength > 0)
            {
                std::wstring wide(
                    static_cast<size_t>(debugLength),
                    L'\0');
                MultiByteToWideChar(
                    CP_UTF8,
                    0,
                    debugLine.data(),
                    static_cast<int>(debugLine.size()),
                    wide.data(),
                    debugLength);
                OutputDebugStringW(wide.c_str());
            }
            pending += debugLine;
        }

        const auto now = std::chrono::steady_clock::now();
        if (urgent ||
            now - lastFlush >= std::chrono::seconds(1))
        {
            Log_AppendDroppedSummary(
                pending,
                reportedTrace,
                reportedDebug,
                reportedInfo,
                reportedCritical);
        }

        if (!pending.empty())
        {
            if (!Log_WriteAll(
                    file,
                    pending.data(),
                    pending.size()))
            {
                g_logRunning.store(false, std::memory_order_release);
                Log_FallbackDebug(
                    L"Escape logging thread stopped after a write failure.");
                break;
            }
            pending.clear();
        }

        if (urgent ||
            now - lastFlush >= std::chrono::seconds(1))
        {
            FlushFileBuffers(file);
            lastFlush = now;
        }

        if (stopping)
        {
            std::lock_guard<std::mutex> lock(g_logQueueMutex);
            if (g_logQueue.empty())
            {
                break;
            }
        }
    }

    CloseHandle(file);
}

bool Log_StartSession()
{
    if (g_logRunning.load(std::memory_order_acquire))
    {
        return true;
    }

    const std::wstring logDirectory = Log_GetLogDirectory();
    const std::wstring logPath =
        logDirectory + L"\\ManakaMultiplayer.Native.log";
    {
        std::lock_guard<std::mutex> lock(g_logQueueMutex);
        g_logQueue.clear();
        g_logQueuedBytes.store(0, std::memory_order_relaxed);
        g_logNextSequence.store(1, std::memory_order_relaxed);
        g_logStopRequested.store(false, std::memory_order_relaxed);
        g_logUrgentRequested.store(false, std::memory_order_relaxed);
    }
    g_logDroppedTrace.store(0, std::memory_order_relaxed);
    g_logDroppedDebug.store(0, std::memory_order_relaxed);
    g_logDroppedInfo.store(0, std::memory_order_relaxed);
    g_logDroppedCritical.store(0, std::memory_order_relaxed);
    g_logRunning.store(true, std::memory_order_release);
    try
    {
        g_logWriterThread =
            std::thread(Log_WriterThreadMain, logPath);
    }
    catch (...)
    {
        g_logRunning.store(false, std::memory_order_release);
        Log_FallbackDebug(
            L"Escape logging thread could not be started.");
        return false;
    }

    LogInfo(
        LogCategory_Lifecycle,
        L"SessionStart",
        L"Native log session started: mod=%s pid=%lu path=%s.",
        MANAKA_MOD_VERSION_STRING,
        GetCurrentProcessId(),
        logPath.c_str());
    return true;
}

void Log_FlushAndStop()
{
    if (!g_logWriterThread.joinable())
    {
        g_logRunning.store(false, std::memory_order_release);
        return;
    }

    LogInfo(
        LogCategory_Lifecycle,
        L"SessionStop",
        L"Native log session is stopping; draining queued records.");
    {
        std::lock_guard<std::mutex> lock(g_logQueueMutex);
        g_logStopRequested.store(true, std::memory_order_release);
    }
    g_logQueueCondition.notify_all();
    g_logWriterThread.join();
    g_logWriterThread = std::thread();
    g_logRunning.store(false, std::memory_order_release);
}
