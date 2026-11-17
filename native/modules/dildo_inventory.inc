// Native.cpp module: dildo_inventory.inc
// Minimal bridge for the standalone MItem dildo. Placement uses the shipped
// choice panel for presentation, then uses the proven E-menu action path.

constexpr int kDildoItemType = 1;
constexpr int kDildoFloorType = 10000;
constexpr int kDildoWallType = 10100;
constexpr int kDildoPutFloorAction = 10028;
constexpr int kDildoPutWallAction = 10029;
constexpr int kDildoPickFloorAction = 10100;
constexpr int kDildoPickWallAction = 10106;
constexpr int kDildoSitPickAction = 10103;
constexpr unsigned long long
    kDildoInventoryPanelDiscoveryRetryMilliseconds = 5000;

struct DildoInventoryPending
{
    bool active = false;
    int itemIndex = -1;
    int definition = -1;
    int quality = 0;
    bool appraised = false;
    unsigned int wearUnits = 0;
    int stageType = -1;
    int dropType = -1;
    int originalNativeItemCount = 0;
    bool actionSeen = false;
    unsigned long long startedAt = 0;
    unsigned long long actionSeenAt = 0;
};

static DildoInventoryPending g_dildoInventoryPending;
static Il2CppObject* g_dildoInventoryPanel = nullptr;
static std::atomic<bool> g_dildoInventoryPanelActive{false};
static std::atomic<bool> g_dildoInventoryClickRequested{false};
static bool g_dildoInventoryClickConsumed = false;
static unsigned long long g_dildoInventoryPanelNextDiscoveryAt = 0;
static bool g_dildoInventoryPickupActionSeen = false;
static int g_dildoInventoryPickupType = -1;
static unsigned long long g_dildoInventoryPickupSeenAt = 0;
static bool g_dildoInventoryRewardPending = false;
static unsigned long long g_dildoInventoryRewardOpenAt = 0;

void DildoInventory_ResetPending();
void DildoInventory_RefreshPanelActive();
void DildoInventory_RemovePendingItem();

bool DildoInventory_IsPanelActive()
{
    return g_dildoInventoryPanelActive.load();
}

int DildoInventory_GetOriginalCount()
{
    auto* gameStateData = ExtendedMode_GetGameStateData();
    auto* dictionary = gameStateData != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(gameStateData) + 0x218)
        : nullptr;
    if (dictionary == nullptr ||
        il2cpp_object_get_class == nullptr ||
        il2cpp_object_unbox == nullptr)
    {
        return 0;
    }
    auto* dictionaryClass = il2cpp_object_get_class(dictionary);
    auto* containsKey = FindMethod(
        dictionaryClass,
        "ContainsKey",
        1);
    int type = kDildoItemType;
    void* parameters[] = { &type };
    auto* contains = containsKey != nullptr
        ? Invoke(containsKey, dictionary, parameters)
        : nullptr;
    if (contains == nullptr ||
        !*static_cast<bool*>(il2cpp_object_unbox(contains)))
    {
        return 0;
    }
    auto* getItem = FindMethod(
        dictionaryClass,
        "get_Item",
        1);
    auto* value = getItem != nullptr
        ? Invoke(getItem, dictionary, parameters)
        : nullptr;
    return value != nullptr
        ? *static_cast<int*>(il2cpp_object_unbox(value))
        : 0;
}

void DildoInventory_SetOriginalCount(int count)
{
    const int current = DildoInventory_GetOriginalCount();
    const int delta = std::max(0, count) - current;
    if (delta == 0)
    {
        return;
    }
    auto* gameStateClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "GameStateFacade");
    auto* changeItemNum = FindMethod(
        gameStateClass,
        "ChangeItemNum",
        2);
    if (changeItemNum == nullptr)
    {
        return;
    }
    int type = kDildoItemType;
    int amount = delta;
    void* parameters[] = { &type, &amount };
    Invoke(changeItemNum, nullptr, parameters);
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Dildo inventory adjusted original count by %d.",
        delta);
}

Il2CppObject* DildoInventory_GetDroppedController(int dropType)
{
    auto* managerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "DroppedItemManager");
    auto* getInstance = FindMethod(
        managerClass,
        "get_Instance",
        0);
    auto* getCache = FindMethod(
        managerClass,
        "GetCache",
        1);
    auto* manager = getInstance != nullptr
        ? Invoke(getInstance, nullptr, nullptr)
        : nullptr;
    int type = dropType;
    void* parameters[] = { &type };
    return manager != nullptr && getCache != nullptr
        ? Invoke(getCache, manager, parameters)
        : nullptr;
}

bool DildoInventory_ControllerIsActive(Il2CppObject* controller)
{
    auto* gameObject = GetComponentGameObject(controller);
    auto* getActive = FindMethod(
        FindClass("UnityEngine", "GameObject"),
        "get_activeInHierarchy",
        0);
    auto* value = gameObject != nullptr && getActive != nullptr
        ? Invoke(getActive, gameObject, nullptr)
        : nullptr;
    return value != nullptr &&
           il2cpp_object_unbox != nullptr &&
           *static_cast<bool*>(il2cpp_object_unbox(value));
}

bool DildoInventory_GetControllerPosition(
    Il2CppObject* controller,
    float& x,
    float& y,
    float& z)
{
    auto* transform = GetWorldTransform(controller);
    auto* getPosition = FindMethod(
        FindClass("UnityEngine", "Transform"),
        "get_position",
        0);
    auto* value = transform != nullptr && getPosition != nullptr
        ? Invoke(getPosition, transform, nullptr)
        : nullptr;
    if (value == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    const auto* position = static_cast<float*>(
        il2cpp_object_unbox(value));
    x = position[0];
    y = position[1];
    z = position[2];
    return true;
}

bool DildoInventory_DropNow(int dropType)
{
    auto* playerClassAccessor =
        g_extendedPlayerController != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedPlayerController) +
                0x40)
            : nullptr;
    auto* playerTransform =
        playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) +
                0x18)
            : nullptr;
    auto* manager =
        playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) +
                0xA0)
            : nullptr;
    if (playerTransform == nullptr || manager == nullptr)
    {
        return false;
    }
    auto* transformClass = FindClass(
        "UnityEngine",
        "Transform");
    auto* getPosition = FindMethod(
        transformClass,
        "get_position",
        0);
    auto* getRotation = FindMethod(
        transformClass,
        "get_rotation",
        0);
    auto* positionValue = getPosition != nullptr
        ? Invoke(getPosition, playerTransform, nullptr)
        : nullptr;
    auto* rotationValue = getRotation != nullptr
        ? Invoke(getRotation, playerTransform, nullptr)
        : nullptr;
    if (positionValue == nullptr ||
        rotationValue == nullptr ||
        il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    const auto* playerPosition = static_cast<float*>(
        il2cpp_object_unbox(positionValue));
    const auto* playerRotation = static_cast<float*>(
        il2cpp_object_unbox(rotationValue));
    float position[3] = {
        playerPosition[0],
        playerPosition[1],
        playerPosition[2],
    };
    float rotation[4] = {
        playerRotation[0],
        playerRotation[1],
        playerRotation[2],
        playerRotation[3],
    };

    auto* managerClass = FindClass(
        "ExposureUnnoticed2.Scripts.InGame",
        "DroppedItemManager");
    auto* transNonUseItemId = FindMethod(
        managerClass,
        "TransNonUseItemId",
        2);
    auto* drop = FindMethod(
        managerClass,
        "Drop",
        3);
    if (drop == nullptr)
    {
        return false;
    }
    int type = dropType;
    if (transNonUseItemId != nullptr)
    {
        bool isMap = false;
        void* transParameters[] = { &type, &isMap };
        auto* transformed = Invoke(
            transNonUseItemId,
            manager,
            transParameters);
        if (transformed != nullptr &&
            il2cpp_object_unbox != nullptr)
        {
            type = *static_cast<int*>(
                il2cpp_object_unbox(transformed));
        }
    }
    void* parameters[] = {
        position,
        rotation,
        &type,
    };
    auto* result = Invoke(drop, manager, parameters);
    auto* controller = DildoInventory_GetDroppedController(dropType);
    const bool active =
        DildoInventory_ControllerIsActive(controller);
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Dildo inventory invoked original DroppedItemManager.Drop type=%d result=%p active=%d.",
        dropType,
        result,
        active ? 1 : 0);
    return result != nullptr && active;
}

void DildoInventory_StagePickupReward(int dropType)
{
    InventoryDroppedDildoSnapshot dropped;
    if (!InventoryUi_TakeDroppedDildo(
            ExtendedMode_GetCurrentStageType(),
            dropType,
            dropped))
    {
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Dildo pickup type=%d had no matching dropped instance.",
            dropType);
        return;
    }
    const int definition = InventoryUi_FindDefinitionById(
        dropped.itemId.c_str());
    if (!InventoryUi_IsDildoDefinition(definition) ||
        !GameplayInventory_AddLootEx(
            definition,
            dropped.quality,
            dropped.appraised,
            dropped.wearUnits,
            false,
            "picked-up-dildo"))
    {
        LogInfo(LogCategory_Inventory, L"Inventory", L"Dildo pickup could not be staged in the reward supply.");
        return;
    }
    DildoInventory_SetOriginalCount(0);
    g_dildoInventoryRewardPending = true;
    g_dildoInventoryRewardOpenAt = GetTickCount64() + 500;
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Dildo pickup staged in reward supply: quality=%d wear=%u.",
        dropped.quality,
        dropped.wearUnits);
}

void DildoInventory_RefreshPanelActive()
{
    const unsigned long long now = GetTickCount64();
    Il2CppObject* panel = g_dildoInventoryPanel;
    if (!ExtendedMode_IsUnityObjectAlive(panel))
    {
        panel = nullptr;
    }
    if (panel == nullptr &&
        now >= g_dildoInventoryPanelNextDiscoveryAt)
    {
        g_dildoInventoryPanelNextDiscoveryAt =
            now + kDildoInventoryPanelDiscoveryRetryMilliseconds;
        auto* panelClass = FindClass(
            "ExposureUnnoticed2.ObjectUI.ChooseDildoPanelView",
            "ChooseDildoPanelView");
        auto* objectClass = FindClass("UnityEngine", "Object");
        auto* findAll = FindMethod(
            objectClass,
            "FindObjectsOfType",
            2);
        auto* basePanelClass = FindClass(
            "Common.Scripts.UI",
            "BasePanelView");
        auto* isActive = FindMethod(
            basePanelClass,
            "IsActivePanel",
            0);
        if (panelClass != nullptr &&
            findAll != nullptr &&
            isActive != nullptr &&
            il2cpp_runtime_class_init != nullptr)
        {
            il2cpp_runtime_class_init(panelClass);
            auto* managedType = il2cpp_type_get_object(
                il2cpp_class_get_type(panelClass));
            bool includeInactive = true;
            void* parameters[] = {
                managedType,
                &includeInactive,
            };
            auto* panels = Invoke(
                findAll,
                nullptr,
                parameters);
            if (panels != nullptr)
            {
                const uint32_t count = il2cpp_array_length(panels);
                auto* elements = reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<char*>(panels) +
                    il2cpp_array_object_header_size());
                for (uint32_t index = 0; index < count; ++index)
                {
                    auto* candidate = elements[index];
                    if (candidate == nullptr)
                    {
                        continue;
                    }
                    if (panel == nullptr)
                    {
                        panel = candidate;
                    }
                    auto* activeValue = Invoke(
                        isActive,
                        candidate,
                        nullptr);
                    if (activeValue != nullptr &&
                        il2cpp_object_unbox != nullptr &&
                        *static_cast<bool*>(
                            il2cpp_object_unbox(activeValue)))
                    {
                        panel = candidate;
                        break;
                    }
                }
            }
        }
    }

    static const Il2CppMethod* isActivePanelMethod = nullptr;
    if (isActivePanelMethod == nullptr)
    {
        isActivePanelMethod = FindMethod(
            FindClass(
                "Common.Scripts.UI",
                "BasePanelView"),
            "IsActivePanel",
            0);
    }
    auto* activeValue =
        panel != nullptr &&
        isActivePanelMethod != nullptr
        ? Invoke(
            isActivePanelMethod,
            panel,
            nullptr)
        : nullptr;
    const bool active =
        activeValue != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(
            il2cpp_object_unbox(activeValue));
    g_dildoInventoryPanel = panel;
    const bool wasActive = g_dildoInventoryPanelActive.exchange(active);
    if (active && !wasActive)
    {
        g_dildoInventoryClickConsumed = false;
        ClipCursor(nullptr);
        ExtendedMode_ForceCursor();
        ExtendedMode_SetGameplayHudRaycastsEnabled(false);
        LogInfo(LogCategory_Inventory, L"Inventory", L"Dildo inventory choice panel opened.");
    }
    else if (!active && wasActive)
    {
        g_dildoInventoryClickRequested = false;
        g_dildoInventoryClickConsumed = false;
        ExtendedMode_SetGameplayHudRaycastsEnabled(true);
        ExtendedMode_ForceGameplayCursor();
        LogInfo(LogCategory_Inventory, L"Inventory", L"Dildo inventory choice panel closed.");
    }
    if (active)
    {
        ClipCursor(nullptr);
        ExtendedMode_ForceCursor();
    }
}

void DildoInventory_ActivateHoveredChoice()
{
    if (!g_dildoInventoryPanelActive.load() ||
        g_dildoInventoryClickConsumed ||
        g_dildoInventoryPanel == nullptr)
    {
        return;
    }
    auto* panel = g_dildoInventoryPanel;
    auto* buttonGroup = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(panel) + 0x58);
    auto* buttons = buttonGroup != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(buttonGroup) + 0x38)
        : nullptr;
    if (buttons == nullptr)
    {
        return;
    }
    const uint32_t count = il2cpp_array_length(buttons);
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(buttons) +
        il2cpp_array_object_header_size());
    int selected = -1;
    for (uint32_t index = 0; index < count; ++index)
    {
        auto* button = elements[index];
        if (button != nullptr &&
            *reinterpret_cast<unsigned char*>(
                reinterpret_cast<char*>(button) + 0x71) != 0)
        {
            selected = static_cast<int>(index);
            break;
        }
    }
    if (selected < 0 || selected > 2)
    {
        return;
    }

    auto* panelClass = FindClass(
        "ExposureUnnoticed2.ObjectUI.ChooseDildoPanelView",
        "ChooseDildoPanelView");
    const char* methodNames[] = {
        "OnClickPutFloor",
        "OnClickPutWall",
        "OnClickCancel",
    };
    auto* method = FindMethod(
        panelClass,
        methodNames[selected],
        0);
    auto* cancelMethod = FindMethod(
        panelClass,
        "OnClickCancel",
        0);
    if (method == nullptr)
    {
        return;
    }
    if (selected == 2)
    {
        Invoke(method, panel, nullptr);
        DildoInventory_ResetPending();
        g_dildoInventoryClickConsumed = true;
        LogInfo(LogCategory_Inventory, L"Inventory", L"Dildo inventory cancelled placement.");
        return;
    }

    const int dropType = selected == 0
        ? kDildoFloorType
        : kDildoWallType;
    if (!DildoInventory_DropNow(dropType))
    {
        LogInfo(LogCategory_Inventory, L"Inventory", 
            L"Dildo inventory failed to create the original dropped item type=%d.",
            dropType);
        return;
    }
    g_dildoInventoryPending.dropType = dropType;
    if (cancelMethod != nullptr)
    {
        Invoke(cancelMethod, panel, nullptr);
    }
    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Dildo inventory created type=%d and closed the choice panel.",
        dropType);
    InventoryUi_RecordDroppedDildo(
        g_dildoInventoryPending.definition,
        g_dildoInventoryPending.quality,
        g_dildoInventoryPending.appraised,
        g_dildoInventoryPending.wearUnits,
        g_dildoInventoryPending.stageType,
        dropType);
    g_dildoInventoryClickConsumed = true;
    DildoInventory_RemovePendingItem();
    DildoInventory_SetOriginalCount(
        std::max(
            0,
            g_dildoInventoryPending.originalNativeItemCount - 1));
    g_dildoInventoryPending = DildoInventoryPending{};
}

void DildoInventory_RemovePendingItem()
{
    if (!InventoryUi_IsItemSlot(g_dildoInventoryPending.itemIndex))
    {
        return;
    }
    auto& item =
        g_inventoryItems[g_dildoInventoryPending.itemIndex];
    if (item.definition != g_dildoInventoryPending.definition ||
        item.storage < 0)
    {
        return;
    }
    item.storage = -1;
    item.quickSlot = -1;
    item.element = SceneUiElement{};
    item.content = SceneUiElement{};
    item.icon = SceneUiElement{};
    item.label = SceneUiElement{};
    item.appraisalBadgeCircle = SceneUiElement{};
    item.appraisalBadgeText = SceneUiElement{};
    InventoryUi_PruneQuickSlotBindings();
    InventoryUi_RefreshEquipment();
    InventoryUi_UpdateQuickSlots();
    GameplayInventory_UpdateQuickSlots();
    InventoryUi_SaveState();
    LogInfo(LogCategory_Inventory, L"Inventory", L"Dildo inventory removed the placed item and saved the backpack state.");
}

void DildoInventory_ResetPending()
{
    if (g_dildoInventoryPending.active)
    {
        DildoInventory_SetOriginalCount(
            g_dildoInventoryPending.originalNativeItemCount);
    }
    g_dildoInventoryPending = DildoInventoryPending{};
}

bool DildoInventory_TryUseBoundItem(int itemIndex)
{
    if (!InventoryUi_IsItemSlot(itemIndex) ||
        g_dildoInventoryPending.active ||
        g_gameplayInventoryOpen.load() ||
        g_extendedOriginalInteractionMenuActive.load() ||
        g_extendedOriginalHandcuffsPanelActive.load() ||
        CheckpointSystem_IsTaskUiOpen() ||
        NpcDialogue_IsActive())
    {
        return false;
    }
    const auto& item = g_inventoryItems[itemIndex];
    if (std::strcmp(
            kEquipmentInventoryDefinitions[item.definition].id,
            "217-adult-011-mitemdildo") != 0 ||
        item.storage < 0)
    {
        return false;
    }
    g_dildoInventoryPending = DildoInventoryPending{};
    g_dildoInventoryPending.active = true;
    g_dildoInventoryPending.itemIndex = itemIndex;
    g_dildoInventoryPending.definition = item.definition;
    g_dildoInventoryPending.quality = item.quality;
    g_dildoInventoryPending.appraised = item.appraised;
    g_dildoInventoryPending.wearUnits = item.wearUnits;
    g_dildoInventoryPending.stageType =
        ExtendedMode_GetCurrentStageType();
    g_dildoInventoryPending.originalNativeItemCount =
        DildoInventory_GetOriginalCount();
    g_dildoInventoryPending.startedAt = GetTickCount64();
    DildoInventory_SetOriginalCount(
        std::max(1, g_dildoInventoryPending.originalNativeItemCount));
    g_dildoInventoryPanelNextDiscoveryAt = 0;

    LogInfo(LogCategory_Inventory, L"Inventory", 
        L"Dildo inventory queued JustRingDildo for the shipped PlayerAction path.");
    return true;
}

void DildoInventory_Tick()
{
    DildoInventory_RefreshPanelActive();
    if (g_dildoInventoryRewardPending &&
        GetTickCount64() >= g_dildoInventoryRewardOpenAt &&
        !g_gameplayInventoryOpen.load() &&
        !CheckpointSystem_IsTaskUiOpen())
    {
        g_dildoInventoryRewardPending = false;
        if (!g_extendedSpecialCoatMode.load())
        {
            ExtendedMode_SetSpecialCoatMode(true);
        }
        GameplayInventory_OpenRewardMode();
        LogInfo(LogCategory_Inventory, L"Inventory", L"Dildo pickup reward inventory opened.");
    }
    if (g_dildoInventoryPanelActive.load() &&
        g_dildoInventoryClickRequested.exchange(false))
    {
        DildoInventory_ActivateHoveredChoice();
    }
    const int currentAction = GameplayInput_GetCurrentActionType();
    if (currentAction == kDildoPickFloorAction ||
        currentAction == kDildoPickWallAction ||
        currentAction == kDildoSitPickAction)
    {
        g_dildoInventoryPickupActionSeen = true;
        g_dildoInventoryPickupType =
            currentAction == kDildoPickWallAction
                ? kDildoWallType
                : kDildoFloorType;
        g_dildoInventoryPickupSeenAt = GetTickCount64();
    }
    else if (g_dildoInventoryPickupActionSeen &&
             GetTickCount64() -
                 g_dildoInventoryPickupSeenAt > 350)
    {
        const int pickupType =
            g_dildoInventoryPickupType;
        g_dildoInventoryPickupActionSeen = false;
        g_dildoInventoryPickupType = -1;
        DildoInventory_StagePickupReward(pickupType);
    }
    if (!g_dildoInventoryPending.active ||
        g_dildoInventoryPending.dropType < 0)
    {
        return;
    }
    const int action = GameplayInput_GetCurrentActionType();
    if (action == kDildoPutFloorAction ||
        action == kDildoPutWallAction)
    {
        if (!g_dildoInventoryPending.actionSeen)
        {
            g_dildoInventoryPending.actionSeen = true;
            g_dildoInventoryPending.actionSeenAt = GetTickCount64();
        }
    }
    if (g_dildoInventoryPending.actionSeen)
    {
        auto* controller = DildoInventory_GetDroppedController(
            g_dildoInventoryPending.dropType);
        if (DildoInventory_ControllerIsActive(controller))
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            DildoInventory_GetControllerPosition(
                controller,
                x,
                y,
                z);
            DildoInventory_RemovePendingItem();
            DildoInventory_SetOriginalCount(
                std::max(
                    0,
                    g_dildoInventoryPending.originalNativeItemCount - 1));
            LogInfo(LogCategory_Inventory, L"Inventory", 
                L"Dildo inventory placement completed at %.2f, %.2f, %.2f.",
                x,
                y,
                z);
            g_dildoInventoryPending = DildoInventoryPending{};
            return;
        }
    }
    const unsigned long long now = GetTickCount64();
    if ((g_dildoInventoryPending.actionSeen &&
         now - g_dildoInventoryPending.actionSeenAt > 15000) ||
        (!g_dildoInventoryPending.actionSeen &&
         now - g_dildoInventoryPending.startedAt > 60000))
    {
        LogInfo(LogCategory_Inventory, L"Inventory", L"Dildo inventory placement timed out.");
        DildoInventory_ResetPending();
    }
}
