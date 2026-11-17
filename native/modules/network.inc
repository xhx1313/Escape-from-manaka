// Native.cpp module: network.inc
// Included inside the shared implementation namespace by Native.cpp.
std::wstring GetLocalIpv4Hint()
{
    char hostname[256]{};
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        return L"No LAN IPv4 address detected.";
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* results = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &results) != 0)
    {
        return L"No LAN IPv4 address detected.";
    }

    std::wstring text = L"LAN host IPv4: ";
    bool found = false;
    for (auto* item = results; item != nullptr; item = item->ai_next)
    {
        auto* address = reinterpret_cast<sockaddr_in*>(item->ai_addr);
        if (address == nullptr || address->sin_addr.s_addr == htonl(INADDR_LOOPBACK))
        {
            continue;
        }

        char buffer[INET_ADDRSTRLEN]{};
        inet_ntop(AF_INET, &address->sin_addr, buffer, sizeof(buffer));
        if (found)
        {
            text += L" / ";
        }
        text += Utf8ToWide(buffer);
        found = true;
    }
    freeaddrinfo(results);
    return found ? text + L":12121" : L"No LAN IPv4 address detected.";
}

bool SendAll(SOCKET socket, const std::string& text)
{
    size_t sent = 0;
    while (sent < text.size())
    {
        const int result = send(socket, text.data() + sent, static_cast<int>(text.size() - sent), 0);
        if (result <= 0)
        {
            return false;
        }
        sent += static_cast<size_t>(result);
    }
    return true;
}

bool ReceiveLine(SOCKET socket, std::string& line)
{
    line.clear();
    char character = 0;
    for (int index = 0; index < 4096; ++index)
    {
        const int result = recv(socket, &character, 1, 0);
        if (result <= 0)
        {
            return false;
        }
        if (character == '\n')
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            return true;
        }
        line.push_back(character);
    }
    return false;
}

void StopNetwork()
{
    const int previousMode = g_networkMode;
    g_networkStop = true;

    if (g_listenSocket != INVALID_SOCKET)
    {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
    }
    if (g_peerSocket != INVALID_SOCKET)
    {
        shutdown(g_peerSocket, SD_BOTH);
        closesocket(g_peerSocket);
        g_peerSocket = INVALID_SOCKET;
    }
    if (g_discoverySocket != INVALID_SOCKET)
    {
        closesocket(g_discoverySocket);
        g_discoverySocket = INVALID_SOCKET;
    }

    if (g_hostThread.joinable() && g_hostThread.get_id() != std::this_thread::get_id())
    {
        g_hostThread.join();
    }
    if (g_clientThread.joinable() && g_clientThread.get_id() != std::this_thread::get_id())
    {
        g_clientThread.join();
    }
    if (g_discoveryThread.joinable() && g_discoveryThread.get_id() != std::this_thread::get_id())
    {
        g_discoveryThread.join();
    }
    g_networkMode = 0;
    if (previousMode != 0)
    {
        LogInfo(
            LogCategory_Network,
            L"Stop",
            L"Network stopped previousMode=%d.",
            previousMode);
    }
}

void HostLoop()
{
    g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listenSocket == INVALID_SOCKET)
    {
        const int error = WSAGetLastError();
        LogError(
            LogCategory_Network,
            L"HostSocket",
            L"Network host socket creation failed WSA=%d.",
            error);
        SetStatus(L"Host failed: socket creation failed. WSA error " + std::to_wstring(error));
        return;
    }

    const BOOL reuse = TRUE;
    setsockopt(g_listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = htonl(INADDR_ANY);
    endpoint.sin_port = htons(kLobbyPort);
    if (bind(g_listenSocket, reinterpret_cast<sockaddr*>(&endpoint), sizeof(endpoint)) == SOCKET_ERROR)
    {
        const int error = WSAGetLastError();
        LogError(
            LogCategory_Network,
            L"HostBind",
            L"Network host could not bind TCP %d WSA=%d.",
            kLobbyPort,
            error);
        SetStatus(L"Host failed: TCP port 12121 is unavailable.");
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        return;
    }
    if (listen(g_listenSocket, 1) == SOCKET_ERROR)
    {
        SetStatus(L"Host failed: listen(TCP 12121) failed.");
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        return;
    }

    SetStatus(L"ROOM CREATED. Waiting for a client on TCP 12121.");
    LogInfo(
        LogCategory_Network,
        L"HostReady",
        L"Network host listening on TCP %d.",
        kLobbyPort);
    while (!g_networkStop)
    {
        g_peerSocket = accept(g_listenSocket, nullptr, nullptr);
        if (g_peerSocket == INVALID_SOCKET)
        {
            break;
        }

        DWORD timeoutMs = 15000;
        setsockopt(g_peerSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

        std::string hello;
        if (!ReceiveLine(g_peerSocket, hello) ||
            hello.rfind("MFM-LOBBY/1 HELLO", 0) != 0 ||
            !SendAll(g_peerSocket, "MFM-LOBBY/1 WELCOME HOST\n"))
        {
            SetStatus(L"Client connected, but the handshake failed.");
            closesocket(g_peerSocket);
            g_peerSocket = INVALID_SOCKET;
            continue;
        }

        SetStatus(L"CONNECTED. Peer joined the room on TCP 12121.");
        LogInfo(
            LogCategory_Network,
            L"PeerConnected",
            L"Network peer completed handshake.");
        while (!g_networkStop)
        {
            std::string line;
            if (!ReceiveLine(g_peerSocket, line))
            {
                break;
            }
            if (line == "PING")
            {
                if (!SendAll(g_peerSocket, "PONG\n"))
                {
                    break;
                }
            }
        }

        closesocket(g_peerSocket);
        g_peerSocket = INVALID_SOCKET;
        if (!g_networkStop)
        {
            SetStatus(L"Peer disconnected. Waiting on TCP 12121.");
        }
    }
}

void ClientLoop(std::wstring wideHost)
{
    const auto host = WideToUtf8(wideHost);
    SOCKET socketHandle = INVALID_SOCKET;

    sockaddr_in numericAddress{};
    numericAddress.sin_family = AF_INET;
    numericAddress.sin_port = htons(kLobbyPort);
    if (inet_pton(AF_INET, host.c_str(), &numericAddress.sin_addr) == 1)
    {
        socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socketHandle != INVALID_SOCKET &&
            connect(socketHandle, reinterpret_cast<sockaddr*>(&numericAddress), sizeof(numericAddress)) != 0)
        {
            closesocket(socketHandle);
            socketHandle = INVALID_SOCKET;
        }
    }
    else
    {
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* addresses = nullptr;
        if (getaddrinfo(host.c_str(), "12121", &hints, &addresses) != 0)
        {
            SetStatus(L"Connection failed: host name or address could not be resolved.");
            return;
        }

        for (auto* item = addresses; item != nullptr; item = item->ai_next)
        {
            socketHandle = socket(item->ai_family, item->ai_socktype, item->ai_protocol);
            if (socketHandle == INVALID_SOCKET)
            {
                continue;
            }
            if (connect(socketHandle, item->ai_addr, static_cast<int>(item->ai_addrlen)) == 0)
            {
                break;
            }
            closesocket(socketHandle);
            socketHandle = INVALID_SOCKET;
        }
        freeaddrinfo(addresses);
    }

    if (socketHandle == INVALID_SOCKET)
    {
        SetStatus(L"Connection failed: TCP 12121 is unreachable.");
        return;
    }

    g_peerSocket = socketHandle;
    DWORD timeoutMs = 10000;
    setsockopt(g_peerSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

    if (!SendAll(g_peerSocket, "MFM-LOBBY/1 HELLO\n"))
    {
        SetStatus(L"Connected, but HELLO could not be sent.");
        return;
    }

    std::string welcome;
    if (!ReceiveLine(g_peerSocket, welcome) || welcome.rfind("MFM-LOBBY/1 WELCOME", 0) != 0)
    {
        SetStatus(L"Connection failed: lobby handshake was rejected.");
        return;
    }

    SetStatus(L"CONNECTED to the host. TCP 12121 heartbeat active.");
    while (!g_networkStop)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (!SendAll(g_peerSocket, "PING\n"))
        {
            break;
        }
        std::string response;
        if (!ReceiveLine(g_peerSocket, response) || response != "PONG")
        {
            break;
        }
    }

    if (!g_networkStop)
    {
        SetStatus(L"Host connection ended.");
    }
}

void HostDiscoveryLoop()
{
    g_discoverySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_discoverySocket == INVALID_SOCKET)
    {
        return;
    }

    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = htonl(INADDR_ANY);
    endpoint.sin_port = htons(kLobbyPort);
    if (bind(g_discoverySocket, reinterpret_cast<sockaddr*>(&endpoint), sizeof(endpoint)) == SOCKET_ERROR)
    {
        closesocket(g_discoverySocket);
        g_discoverySocket = INVALID_SOCKET;
        LogInfo(LogCategory_Network, L"Network", L"UDP discovery could not bind port 12121.");
        return;
    }

    while (!g_networkStop)
    {
        char buffer[128]{};
        sockaddr_in sender{};
        int senderLength = sizeof(sender);
        const int received = recvfrom(
            g_discoverySocket,
            buffer,
            sizeof(buffer) - 1,
            0,
            reinterpret_cast<sockaddr*>(&sender),
            &senderLength);
        if (received <= 0)
        {
            break;
        }
        if (std::strncmp(buffer, "MFM-DISCOVER", 12) == 0)
        {
            const char response[] = "MFM-HOST";
            sendto(
                g_discoverySocket,
                response,
                static_cast<int>(sizeof(response) - 1),
                0,
                reinterpret_cast<sockaddr*>(&sender),
                senderLength);
        }
    }

    if (g_discoverySocket != INVALID_SOCKET)
    {
        closesocket(g_discoverySocket);
        g_discoverySocket = INVALID_SOCKET;
    }
}

void DiscoveryClientLoop()
{
    SOCKET socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketHandle == INVALID_SOCKET)
    {
        SetStatus(L"Host discovery could not create a UDP socket.");
        g_networkMode = 0;
        return;
    }

    BOOL broadcast = TRUE;
    setsockopt(socketHandle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&broadcast), sizeof(broadcast));
    DWORD timeoutMs = 3000;
    setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    target.sin_port = htons(kLobbyPort);
    const char request[] = "MFM-DISCOVER";
    sendto(
        socketHandle,
        request,
        static_cast<int>(sizeof(request) - 1),
        0,
        reinterpret_cast<sockaddr*>(&target),
        sizeof(target));

    char response[128]{};
    sockaddr_in hostAddress{};
    int hostAddressLength = sizeof(hostAddress);
    const int received = recvfrom(
        socketHandle,
        response,
        sizeof(response) - 1,
        0,
        reinterpret_cast<sockaddr*>(&hostAddress),
        &hostAddressLength);
    closesocket(socketHandle);

    if (received <= 0 || std::strncmp(response, "MFM-HOST", 8) != 0)
    {
        SetStatus(L"No LAN host responded on UDP 12121.");
        g_networkMode = 0;
        return;
    }

    char addressText[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &hostAddress.sin_addr, addressText, sizeof(addressText));
    const auto host = Utf8ToWide(addressText);
    LogInfo(LogCategory_Network, L"Network", L"LAN host discovered at %s.", host.c_str());
    g_networkMode = 3;
    ClientLoop(host);
}

void StartClientDiscovery()
{
    LogInfo(
        LogCategory_Network,
        L"DiscoveryStart",
        L"LAN host discovery started on UDP %d.",
        kLobbyPort);
    StopNetwork();
    g_networkStop = false;
    g_networkMode = 2;
    g_discoveryThread = std::thread(DiscoveryClientLoop);
}

void StartHost()
{
    LogInfo(
        LogCategory_Network,
        L"HostStart",
        L"Network host start requested on TCP/UDP %d.",
        kLobbyPort);
    StopNetwork();
    g_networkStop = false;
    g_networkMode = 1;
    if (g_hostThread.joinable())
    {
        g_hostThread = std::thread();
    }
    g_hostThread = std::thread(HostLoop);
    g_discoveryThread = std::thread(HostDiscoveryLoop);
}

void StartClient(const std::wstring& host)
{
    LogInfo(
        LogCategory_Network,
        L"ClientStart",
        L"Network client start requested for %s.",
        host.c_str());
    StopNetwork();
    g_networkStop = false;
    if (g_clientThread.joinable())
    {
        g_clientThread = std::thread();
    }
    g_clientThread = std::thread(ClientLoop, host);
}

