// Native.cpp module: equipment_stats.inc
// Applies signed equipment stats to NPC strangeness and player locomotion.

constexpr int kEquipmentStatLimit = 225;
constexpr int kEquipmentStatItemLimit = 25;
constexpr int kEquipmentStatBaseLimit = 5;
constexpr int kStrangenessBaseReduceCount = 120;
constexpr float kStrangenessBaseReducePerSecond = 0.20f;
constexpr float kEquipmentConservatismCurveRange = 225.0f;
constexpr float kEquipmentConservatismCenterMultiplier = 0.6f;
constexpr float kEquipmentMovementCurveRange = 225.0f;
constexpr float kEquipmentMovementMultiplierMin = 0.5f;
constexpr float kEquipmentMovementMultiplierMax = 1.5f;
constexpr float kEquipmentClimaxCurveRange = 225.0f;
constexpr float kEquipmentClimaxMultiplierMin = 0.5f;
constexpr float kEquipmentClimaxMultiplierMax = 2.0f;
constexpr float kEquipmentStaminaCurveRange = 225.0f;
constexpr float kEquipmentStaminaCapacityMultiplierMin = 0.5f;
constexpr float kEquipmentStaminaCapacityMultiplierMax = 2.0f;
constexpr float kEquipmentStaminaRecoveryMultiplierMin = 0.5f;
constexpr float kEquipmentStaminaRecoveryMultiplierMax = 2.0f;
constexpr float kEquipmentStaminaCooldownMultiplierMin = 0.5f;
constexpr float kEquipmentStaminaCooldownMultiplierMax = 2.0f;
constexpr float kEquipmentHandcuffCurveRange = 225.0f;
constexpr float kEquipmentHandcuffSuccessCurveRange = 450.0f;
constexpr float kEquipmentHandcuffBaseProgress2Goal = 1.0f;
constexpr float kEquipmentRewardCurveRange = 225.0f;
constexpr float kEquipmentRewardMultiplierMin = 0.5f;
constexpr float kEquipmentRewardMultiplierMax = 2.0f;
constexpr unsigned long long kExposureRewardIntervalMs = 5000ull;
constexpr long long kExposureRewardBase = 100;

std::atomic<int> g_equipmentStatsPerceptionFromEquipment{0};
std::atomic<int> g_equipmentStatsConservatismFromEquipment{0};
std::atomic<int> g_equipmentStatsNormalityFromEquipment{0};
std::atomic<int> g_equipmentStatsStrengthFromEquipment{0};
std::atomic<int> g_equipmentStatsSpeedFromEquipment{0};
std::atomic<int> g_equipmentStatsClimaxSpeedFromEquipment{0};
std::atomic<int> g_equipmentStatsStaminaCapacityFromEquipment{0};
std::atomic<int> g_equipmentStatsStaminaRecoveryFromEquipment{0};
std::atomic<int> g_equipmentStatsHandcuffProficiencyFromEquipment{0};
std::atomic<int> g_equipmentStatsRewardFromEquipment{0};
std::atomic<unsigned int> g_equipmentStatsRevision{1};
std::atomic<unsigned int> g_equipmentStatsConservatismObservedRevision{0};
std::atomic<unsigned int> g_equipmentStatsNormalityObservedRevision{0};
std::atomic<unsigned int> g_equipmentStatsMovementObservedRevision{0};
std::atomic<int> g_equipmentStatsMovementObservedMode{-1};
std::atomic<unsigned int> g_equipmentStatsHandcuffObservedRevision{0};
Il2CppObject* g_equipmentStatsHandcuffController = nullptr;
unsigned long long g_equipmentStatsHandcuffHoldStartTick = 0;
int g_equipmentStatsHandcuffAttempts = 0;
int g_equipmentStatsHandcuffObservedType = -1;
int g_equipmentStatsHandcuffObservedForcePhase = -1;
int g_equipmentStatsHandcuffObservedAttached = -1;
int g_equipmentStatsHandcuffObservedMapAttached = -1;
float g_equipmentStatsHandcuffObservedProgress2 = -1.0f;
Il2CppObject* g_equipmentStatsHandcuffMaster = nullptr;
Il2CppField* g_equipmentStatsHandcuffSpeedField = nullptr;
size_t g_equipmentStatsHandcuffSpeedOffset = 0;
float g_equipmentStatsHandcuffVanillaSpeed = 0.0f;
bool g_equipmentStatsHandcuffSpeedCaptured = false;
Il2CppObject* g_equipmentStatsHandcuffGoalController = nullptr;
float g_equipmentStatsHandcuffAppliedGoal =
    kEquipmentHandcuffBaseProgress2Goal;
bool g_equipmentStatsHandcuffGoalOwned = false;
unsigned long long g_equipmentStatsRewardWindowStart = 0;
bool g_equipmentStatsRewardExposureActive = false;
std::atomic<bool> g_equipmentStatsRaidActive{false};
std::atomic<long long> g_equipmentStatsRaidEarnings{0};

long long EquipmentStats_GetRaidEarnings()
{
    return g_equipmentStatsRaidEarnings.load(std::memory_order_relaxed);
}

void EquipmentStats_BeginRaid(const wchar_t* source)
{
    const long long discarded =
        g_equipmentStatsRaidEarnings.exchange(
            0,
            std::memory_order_relaxed);
    g_equipmentStatsRaidActive = true;
    g_equipmentStatsRewardWindowStart = 0;
    g_equipmentStatsRewardExposureActive = false;
    LogInfo(
        LogCategory_Inventory,
        L"RaidMoney",
        L"Raid earnings session started by %s; discarded stale amount=%lld.",
        source != nullptr ? source : L"unspecified",
        discarded);
}

void EquipmentStats_AddRaidEarnings(long long amount)
{
    if (amount <= 0)
    {
        return;
    }
    long long previous =
        g_equipmentStatsRaidEarnings.load(std::memory_order_relaxed);
    long long updated = previous;
    do
    {
        updated = previous > kInventoryBalanceMax - amount
            ? kInventoryBalanceMax
            : previous + amount;
    }
    while (!g_equipmentStatsRaidEarnings.compare_exchange_weak(
        previous,
        updated,
        std::memory_order_relaxed,
        std::memory_order_relaxed));
}

long long EquipmentStats_CommitRaidEarnings(const wchar_t* source)
{
    const bool wasActive = g_equipmentStatsRaidActive.exchange(false);
    const long long amount =
        g_equipmentStatsRaidEarnings.exchange(
            0,
            std::memory_order_relaxed);
    g_equipmentStatsRewardWindowStart = 0;
    g_equipmentStatsRewardExposureActive = false;
    long long balance = InventoryUi_GetBalance();
    if (amount > 0)
    {
        balance = InventoryUi_AddBalance(amount, L"raid extraction");
        InventoryUi_SaveState();
    }
    LogInfo(
        LogCategory_Inventory,
        L"RaidMoney",
        L"Raid earnings committed by %s: active=%d amount=%lld balance=%lld.",
        source != nullptr ? source : L"unspecified",
        wasActive ? 1 : 0,
        amount,
        balance);
    return amount;
}

void EquipmentStats_DiscardRaidEarnings(const wchar_t* source)
{
    const bool wasActive = g_equipmentStatsRaidActive.exchange(false);
    const long long amount =
        g_equipmentStatsRaidEarnings.exchange(
            0,
            std::memory_order_relaxed);
    g_equipmentStatsRewardWindowStart = 0;
    g_equipmentStatsRewardExposureActive = false;
    if (wasActive || amount > 0)
    {
        LogInfo(
            LogCategory_Inventory,
            L"RaidMoney",
            L"Raid earnings discarded by %s: active=%d amount=%lld.",
            source != nullptr ? source : L"unspecified",
            wasActive ? 1 : 0,
            amount);
    }
}

int EquipmentStats_ClampTotal(int value)
{
    return std::clamp(value, -kEquipmentStatLimit, kEquipmentStatLimit);
}

int EquipmentStats_GetEffectivePerception()
{
    return g_equipmentStatsPerceptionFromEquipment.load();
}

int EquipmentStats_GetEffectiveConservatism()
{
    return g_equipmentStatsConservatismFromEquipment.load();
}

int EquipmentStats_GetEffectiveNormality()
{
    return g_equipmentStatsNormalityFromEquipment.load();
}

int EquipmentStats_GetEffectiveStrength()
{
    return g_equipmentStatsStrengthFromEquipment.load();
}

int EquipmentStats_GetEffectiveSpeed()
{
    return g_equipmentStatsSpeedFromEquipment.load();
}

int EquipmentStats_GetEffectiveClimaxSpeed()
{
    return g_equipmentStatsClimaxSpeedFromEquipment.load();
}

int EquipmentStats_GetEffectiveStaminaCapacity()
{
    return g_equipmentStatsStaminaCapacityFromEquipment.load();
}

int EquipmentStats_GetEffectiveStaminaRecovery()
{
    return g_equipmentStatsStaminaRecoveryFromEquipment.load();
}

int EquipmentStats_GetEffectiveHandcuffProficiency()
{
    return g_equipmentStatsHandcuffProficiencyFromEquipment.load();
}

int EquipmentStats_GetEffectiveReward()
{
    return g_equipmentStatsRewardFromEquipment.load();
}

float EquipmentStats_GetConservatismMultiplier()
{
    const float normalizedConservatism =
        static_cast<float>(EquipmentStats_GetEffectiveConservatism()) /
        kEquipmentConservatismCurveRange;
    // Positive conservatism reduces suspicion growth; negative values raise it.
    return kEquipmentConservatismCenterMultiplier *
        std::exp2(-normalizedConservatism);
}

float EquipmentStats_GetNormalityDelayMultiplier()
{
    return 1.0f -
        static_cast<float>(EquipmentStats_GetEffectiveNormality()) / 300.0f;
}

float EquipmentStats_GetNormalityDecayMultiplier()
{
    return 1.0f +
        static_cast<float>(EquipmentStats_GetEffectiveNormality()) / 300.0f;
}

float EquipmentStats_GetMovementMultiplier(int total)
{
    const float normalized =
        static_cast<float>(total) / kEquipmentMovementCurveRange;
    const float multiplier =
        1.0f + normalized * 0.25f +
        normalized * normalized * normalized * 0.25f;
    return std::clamp(
        multiplier,
        kEquipmentMovementMultiplierMin,
        kEquipmentMovementMultiplierMax);
}

float EquipmentStats_GetClimaxGainMultiplier()
{
    const float normalized =
        static_cast<float>(EquipmentStats_GetEffectiveClimaxSpeed()) /
        kEquipmentClimaxCurveRange;
    const float multiplier = std::exp2(normalized);
    return std::clamp(
        multiplier,
        kEquipmentClimaxMultiplierMin,
        kEquipmentClimaxMultiplierMax);
}

float EquipmentStats_GetClimaxDecayMultiplier()
{
    return 1.0f / EquipmentStats_GetClimaxGainMultiplier();
}

float EquipmentStats_GetStaminaCapacityMultiplier()
{
    const float normalized =
        static_cast<float>(EquipmentStats_GetEffectiveStaminaCapacity()) /
        kEquipmentStaminaCurveRange;
    return std::clamp(
        std::exp2(normalized),
        kEquipmentStaminaCapacityMultiplierMin,
        kEquipmentStaminaCapacityMultiplierMax);
}

float EquipmentStats_GetStaminaRecoveryMultiplier()
{
    const float normalized =
        static_cast<float>(EquipmentStats_GetEffectiveStaminaRecovery()) /
        kEquipmentStaminaCurveRange;
    return std::clamp(
        std::exp2(normalized),
        kEquipmentStaminaRecoveryMultiplierMin,
        kEquipmentStaminaRecoveryMultiplierMax);
}

float EquipmentStats_GetStaminaConsumptionMultiplier()
{
    return 1.0f / EquipmentStats_GetStaminaRecoveryMultiplier();
}

float EquipmentStats_GetStaminaCooldownMultiplier()
{
    return std::clamp(
        1.0f / EquipmentStats_GetStaminaCapacityMultiplier(),
        kEquipmentStaminaCooldownMultiplierMin,
        kEquipmentStaminaCooldownMultiplierMax);
}

float EquipmentStats_GetHandcuffHoldSpeedMultiplier()
{
    return std::exp2(
        static_cast<float>(
            EquipmentStats_GetEffectiveHandcuffProficiency()) /
        kEquipmentHandcuffCurveRange);
}

float EquipmentStats_GetHandcuffSuccessGainMultiplier()
{
    return std::exp2(
        static_cast<float>(
            EquipmentStats_GetEffectiveHandcuffProficiency()) /
        kEquipmentHandcuffSuccessCurveRange);
}

float EquipmentStats_GetRewardMultiplier()
{
    const float normalized =
        static_cast<float>(EquipmentStats_GetEffectiveReward()) /
        kEquipmentRewardCurveRange;
    return std::clamp(
        std::exp2(normalized),
        kEquipmentRewardMultiplierMin,
        kEquipmentRewardMultiplierMax);
}

void EquipmentStats_LogEffectiveState(const wchar_t* source)
{
    const int perception = EquipmentStats_GetEffectivePerception();
    const int conservatism = EquipmentStats_GetEffectiveConservatism();
    const int normality = EquipmentStats_GetEffectiveNormality();
    const int strength = EquipmentStats_GetEffectiveStrength();
    const int speed = EquipmentStats_GetEffectiveSpeed();
    const int climaxSpeed = EquipmentStats_GetEffectiveClimaxSpeed();
    const int staminaCapacity = EquipmentStats_GetEffectiveStaminaCapacity();
    const int staminaRecovery =
        EquipmentStats_GetEffectiveStaminaRecovery();
    const int handcuffProficiency =
        EquipmentStats_GetEffectiveHandcuffProficiency();
    const int reward = EquipmentStats_GetEffectiveReward();
    LogInfo(
        LogCategory_Action,
        L"EquipmentStats",
        L"Equipment stats state (%s): perception=%d conservatism=%d normality=%d "
        L"strength=%d speed=%d climaxSpeed=%d staminaCapacity=%d "
        L"staminaRecovery=%d handcuffProficiency=%d "
        L"reward=%d "
        L"growth=%.6f delay=%.6f decay=%.6f "
        L"strengthMultiplier=%.6f speedMultiplier=%.6f climaxGain=%.6f "
        L"climaxDecay=%.6f staminaCapacityMultiplier=%.6f "
        L"staminaRecoveryMultiplier=%.6f staminaConsumptionMultiplier=%.6f "
        L"staminaCooldownMultiplier=%.6f handcuffHoldSpeed=%.6f "
        L"handcuffSuccessGain=%.6f rewardMultiplier=%.6f.",
        source != nullptr ? source : L"unspecified",
        perception,
        conservatism,
        normality,
        strength,
        speed,
        climaxSpeed,
        staminaCapacity,
        staminaRecovery,
        handcuffProficiency,
        reward,
        EquipmentStats_GetConservatismMultiplier(),
        EquipmentStats_GetNormalityDelayMultiplier(),
        EquipmentStats_GetNormalityDecayMultiplier(),
        EquipmentStats_GetMovementMultiplier(strength),
        EquipmentStats_GetMovementMultiplier(speed),
        EquipmentStats_GetClimaxGainMultiplier(),
        EquipmentStats_GetClimaxDecayMultiplier(),
        EquipmentStats_GetStaminaCapacityMultiplier(),
        EquipmentStats_GetStaminaRecoveryMultiplier(),
        EquipmentStats_GetStaminaConsumptionMultiplier(),
        EquipmentStats_GetStaminaCooldownMultiplier(),
        EquipmentStats_GetHandcuffHoldSpeedMultiplier(),
        EquipmentStats_GetHandcuffSuccessGainMultiplier(),
        EquipmentStats_GetRewardMultiplier());
}

void EquipmentStats_ApplyNormalityWaitCount(Il2CppObject* valueManager)
{
    if (valueManager == nullptr)
    {
        return;
    }
    const float delayCount =
        static_cast<float>(kStrangenessBaseReduceCount) *
        EquipmentStats_GetNormalityDelayMultiplier();
    const int count = std::clamp(
        static_cast<int>(std::lround(delayCount)),
        1,
        kStrangenessBaseReduceCount * 3);
    *reinterpret_cast<int*>(
        reinterpret_cast<char*>(valueManager) + 0x18) = count;
}

void EquipmentStats_RefreshFromEquipment()
{
    if (!g_inventoryStateLoaded)
    {
        return;
    }

    int perception = 0;
    int conservatism = 0;
    int normality = 0;
    int strength = 0;
    int speed = 0;
    int climaxSpeed = 0;
    int staminaCapacity = 0;
    int staminaRecovery = 0;
    int handcuffProficiency = 0;
    int reward = 0;
    int equippedCount = 0;
    for (int index = 0; index < kInventoryItemSlotCount; ++index)
    {
        const auto& item = g_inventoryItems[index];
        if (item.storage != kInventoryStorageEquipped ||
            item.definition < 0 ||
            item.definition >= kEquipmentInventoryDefinitionCount)
        {
            continue;
        }

        ++equippedCount;
        const auto& definition =
            kEquipmentInventoryDefinitions[item.definition];
        const int qualityLevel = std::clamp(
            item.quality,
            0,
            kInventoryQualityCount - 1) + 1;
        perception += std::clamp(
            definition.perceptionBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        conservatism += std::clamp(
            definition.conservatismBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        normality += std::clamp(
            definition.normalityBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        strength += std::clamp(
            definition.strengthBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        speed += std::clamp(
            definition.speedBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        climaxSpeed += std::clamp(
            definition.climaxSpeedBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        staminaCapacity += std::clamp(
            definition.staminaCapacityBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        staminaRecovery += std::clamp(
            definition.staminaRecoveryBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        handcuffProficiency += std::clamp(
            definition.handcuffProficiencyBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
        reward += std::clamp(
            definition.rewardBase * qualityLevel,
            -kEquipmentStatItemLimit,
            kEquipmentStatItemLimit);
    }

    perception = EquipmentStats_ClampTotal(perception);
    conservatism = EquipmentStats_ClampTotal(conservatism);
    normality = EquipmentStats_ClampTotal(normality);
    strength = EquipmentStats_ClampTotal(strength);
    speed = EquipmentStats_ClampTotal(speed);
    climaxSpeed = EquipmentStats_ClampTotal(climaxSpeed);
    staminaCapacity = EquipmentStats_ClampTotal(staminaCapacity);
    staminaRecovery = EquipmentStats_ClampTotal(staminaRecovery);
    handcuffProficiency = EquipmentStats_ClampTotal(handcuffProficiency);
    reward = EquipmentStats_ClampTotal(reward);
    const int previousPerception =
        g_equipmentStatsPerceptionFromEquipment.exchange(perception);
    const int previousConservatism =
        g_equipmentStatsConservatismFromEquipment.exchange(conservatism);
    const int previousNormality =
        g_equipmentStatsNormalityFromEquipment.exchange(normality);
    const int previousStrength =
        g_equipmentStatsStrengthFromEquipment.exchange(strength);
    const int previousSpeed =
        g_equipmentStatsSpeedFromEquipment.exchange(speed);
    const int previousClimaxSpeed =
        g_equipmentStatsClimaxSpeedFromEquipment.exchange(climaxSpeed);
    const int previousStaminaCapacity =
        g_equipmentStatsStaminaCapacityFromEquipment.exchange(
            staminaCapacity);
    const int previousStaminaRecovery =
        g_equipmentStatsStaminaRecoveryFromEquipment.exchange(
            staminaRecovery);
    const int previousHandcuffProficiency =
        g_equipmentStatsHandcuffProficiencyFromEquipment.exchange(
            handcuffProficiency);
    const int previousReward =
        g_equipmentStatsRewardFromEquipment.exchange(reward);
    if (previousPerception != perception ||
        previousConservatism != conservatism ||
        previousNormality != normality ||
        previousStrength != strength ||
        previousSpeed != speed ||
        previousClimaxSpeed != climaxSpeed ||
        previousStaminaCapacity != staminaCapacity ||
        previousStaminaRecovery != staminaRecovery ||
        previousHandcuffProficiency != handcuffProficiency ||
        previousReward != reward)
    {
        ++g_equipmentStatsRevision;
        LogInfo(
            LogCategory_Action,
            L"EquipmentStats",
            L"Equipped stats refreshed: equippedItems=%d perception=%d "
            L"conservatism=%d normality=%d "
            L"strength=%d speed=%d climaxSpeed=%d staminaCapacity=%d "
            L"staminaRecovery=%d handcuffProficiency=%d reward=%d.",
            equippedCount,
            perception,
            conservatism,
            normality,
            strength,
            speed,
            climaxSpeed,
            staminaCapacity,
            staminaRecovery,
            handcuffProficiency,
            reward);
        EquipmentStats_LogEffectiveState(L"equipment");
    }
}

void EquipmentStats_UpdateExposureReward()
{
    const unsigned long long now = GetTickCount64();
    const bool exposed =
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        !g_extendedSpecialCoatMode.load();

    if (exposed != g_equipmentStatsRewardExposureActive)
    {
        g_equipmentStatsRewardExposureActive = exposed;
        g_equipmentStatsRewardWindowStart = exposed ? now : 0;
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsReward",
            exposed
                ? L"Exposure reward window started; payout interval=%llu ms "
                  L"reward=%d multiplier=%.6f."
                : L"Exposure reward window paused; partial interval discarded.",
            kExposureRewardIntervalMs,
            EquipmentStats_GetEffectiveReward(),
            EquipmentStats_GetRewardMultiplier());
    }
    if (!exposed || g_equipmentStatsRewardWindowStart == 0)
    {
        return;
    }
    if (now - g_equipmentStatsRewardWindowStart <
        kExposureRewardIntervalMs)
    {
        return;
    }

    const long long amount = std::max(
        1ll,
        static_cast<long long>(std::llround(
            static_cast<double>(kExposureRewardBase) *
            EquipmentStats_GetRewardMultiplier())));
    g_equipmentStatsRewardWindowStart = now;
    EquipmentStats_AddRaidEarnings(amount);
    const long long raidEarnings =
        EquipmentStats_GetRaidEarnings();

    char notification[128]{};
    std::snprintf(
        notification,
        sizeof(notification),
        "本局获得金钱  +￥%lld",
        amount);
    GameplayStatusHud_ShowNotification(notification, 2200);
    LogInfo(
        LogCategory_Inventory,
        L"EquipmentStatsReward",
        L"Exposure reward accrued for extraction: amount=%lld reward=%d "
        L"multiplier=%.6f raidEarnings=%lld.",
        amount,
        EquipmentStats_GetEffectiveReward(),
        EquipmentStats_GetRewardMultiplier(),
        raidEarnings);
}

void SmoothFloatUpdateToHook(
    Il2CppObject* smoothFloat,
    float target,
    const Il2CppMethod* method)
{
    float adjustedTarget = target;
    if (smoothFloat != nullptr &&
        target > 0.0f &&
        g_extendedModeState.load() == ExtendedMode_Gameplay &&
        g_extendedPlayerController != nullptr &&
        GameplayInput_GetCurrentActionType() < 0)
    {
        auto* playerClassAccessor =
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(g_extendedPlayerController) + 0x40);
        auto* locomotion = playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) + 0x58)
            : nullptr;
        auto* moveSpeed = locomotion != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(locomotion) + 0x28)
            : nullptr;
        auto* playerState = playerClassAccessor != nullptr
            ? *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<char*>(playerClassAccessor) + 0x60)
            : nullptr;

        if (smoothFloat == moveSpeed && playerState != nullptr)
        {
            const char* state = reinterpret_cast<const char*>(playerState);
            const bool isDash =
                *reinterpret_cast<const bool*>(state + 0x19);
            const bool isCrouch =
                *reinterpret_cast<const bool*>(state + 0x1B);
            const bool isGamanBaibu =
                *reinterpret_cast<const bool*>(state + 0x120);
            const bool useStrength = isCrouch || isGamanBaibu;
            const int mode = isCrouch
                ? 2
                : (isGamanBaibu ? 3 : (isDash ? 1 : 0));
            const int stat = useStrength
                ? EquipmentStats_GetEffectiveStrength()
                : EquipmentStats_GetEffectiveSpeed();
            if (stat != 0)
            {
                const float multiplier =
                    EquipmentStats_GetMovementMultiplier(stat);
                adjustedTarget = target * multiplier;

                const unsigned int revision =
                    g_equipmentStatsRevision.load(
                        std::memory_order_relaxed);
                const unsigned int previousRevision =
                    g_equipmentStatsMovementObservedRevision.exchange(
                        revision);
                const int previousMode =
                    g_equipmentStatsMovementObservedMode.exchange(mode);
                if (previousRevision != revision ||
                    previousMode != mode)
                {
                    const wchar_t* modeName = mode == 0
                        ? L"walk"
                        : (mode == 1
                            ? L"dash"
                            : (mode == 2 ? L"crawl" : L"gaman"));
                    LogInfo(
                        LogCategory_Action,
                        L"EquipmentStatsMovement",
                        L"Movement speed hook active: mode=%s dash=%d "
                        L"crouch=%d gaman=%d vanillaTarget=%.6f "
                        L"scaledTarget=%.6f stat=%d multiplier=%.6f.",
                        modeName,
                        isDash ? 1 : 0,
                        isCrouch ? 1 : 0,
                        isGamanBaibu ? 1 : 0,
                        target,
                        adjustedTarget,
                        stat,
                        multiplier);
                }
            }
        }
    }

    if (g_originalSmoothFloatUpdateTo != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            float,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalSmoothFloatUpdateTo)(
                smoothFloat,
                adjustedTarget,
                method);
    }
}

void PlayerEcstasyControllerUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_originalPlayerEcstasyControllerUpdate != nullptr)
        {
            using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
            reinterpret_cast<OriginalFn>(
                g_originalPlayerEcstasyControllerUpdate)(instance, method);
        }
        return;
    }
    auto* gameStateData = ExtendedMode_GetGameStateData();
    auto* playerState = ExtendedMode_GetPlayerStateModel();
    const bool observeGameplay =
        gameStateData != nullptr &&
        playerState != nullptr;

    float before = 0.0f;
    int actionType = -1;
    if (observeGameplay)
    {
        before = *reinterpret_cast<float*>(
            reinterpret_cast<char*>(gameStateData) + 0x168);
        auto* currentAction = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerState) + 0x48);
        if (currentAction != nullptr)
        {
            actionType = *reinterpret_cast<int*>(
                reinterpret_cast<char*>(currentAction) + 0x18);
        }
    }

    if (g_originalPlayerEcstasyControllerUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalPlayerEcstasyControllerUpdate)(
                instance,
                method);
    }
    if (!observeGameplay)
    {
        return;
    }

    const float vanillaAfter = *reinterpret_cast<float*>(
        reinterpret_cast<char*>(gameStateData) + 0x168);
    float after = vanillaAfter;
    const float vanillaDelta = vanillaAfter - before;
    const int climaxSpeed = EquipmentStats_GetEffectiveClimaxSpeed();
    const bool handcuffClimaxAction =
        actionType == 10019 ||
        actionType == 10020 ||
        actionType == 10022;
    const bool forcedJump =
        (before <= 0.0001f && vanillaAfter >= 0.9999f) ||
        (before >= 0.9999f && vanillaAfter <= 0.0001f);
    const bool continuousChange =
        climaxSpeed != 0 &&
        !handcuffClimaxAction &&
        !g_npcDialogueActive.load() &&
        !forcedJump &&
        std::isfinite(before) &&
        std::isfinite(vanillaAfter) &&
        before >= -0.001f &&
        before <= 1.001f &&
        vanillaAfter >= -0.001f &&
        vanillaAfter <= 1.001f &&
        std::fabs(vanillaDelta) > 0.000001f &&
        std::fabs(vanillaDelta) <= 0.15f;
    float appliedMultiplier = 1.0f;
    if (continuousChange)
    {
        appliedMultiplier = vanillaDelta > 0.0f
            ? EquipmentStats_GetClimaxGainMultiplier()
            : EquipmentStats_GetClimaxDecayMultiplier();
        after = std::clamp(
            before + vanillaDelta * appliedMultiplier,
            0.0f,
            1.0f);
        *reinterpret_cast<float*>(
            reinterpret_cast<char*>(gameStateData) + 0x168) =
            after;
    }
}

bool EquipmentStats_EnsureHandcuffMasterSpeed()
{
    static Il2CppClass* mcommonClass = nullptr;
    static const Il2CppMethod* instanceGetter = nullptr;
    static Il2CppField* speedField = nullptr;
    static bool resolutionAttempted = false;
    if (!resolutionAttempted)
    {
        resolutionAttempted = true;
        mcommonClass = FindClass(
            "ExposureUnnoticed2.Master.Common",
            "MCommon");
        instanceGetter = FindMethod(
            mcommonClass,
            "get_Instance",
            0);
        speedField = il2cpp_class_get_field_from_name(
            mcommonClass,
            "HandcuffsSpeed");
    }
    if (instanceGetter == nullptr ||
        speedField == nullptr ||
        il2cpp_field_get_offset == nullptr)
    {
        LogOnce(
            "equipment-stats-handcuff-master-missing",
            LogLevel_Warn,
            LogCategory_Action,
            L"EquipmentStatsHandcuff",
            L"Handcuff speed field is unavailable; proficiency speed "
            L"cannot be applied.");
        return false;
    }

    auto* master = Invoke(instanceGetter, nullptr, nullptr);
    if (master == nullptr)
    {
        return false;
    }
    g_equipmentStatsHandcuffMaster = master;
    g_equipmentStatsHandcuffSpeedField = speedField;
    g_equipmentStatsHandcuffSpeedOffset =
        il2cpp_field_get_offset(speedField);
    if (!g_equipmentStatsHandcuffSpeedCaptured)
    {
        const float current =
            *reinterpret_cast<const float*>(
                reinterpret_cast<const char*>(master) +
                g_equipmentStatsHandcuffSpeedOffset);
        if (std::isfinite(current) && current > 0.0f)
        {
            g_equipmentStatsHandcuffVanillaSpeed = current;
            g_equipmentStatsHandcuffSpeedCaptured = true;
        }
    }
    return g_equipmentStatsHandcuffSpeedCaptured;
}

void EquipmentStats_RestoreHandcuffMasterSpeed()
{
    if (g_equipmentStatsHandcuffMaster == nullptr ||
        g_equipmentStatsHandcuffSpeedField == nullptr ||
        !g_equipmentStatsHandcuffSpeedCaptured)
    {
        return;
    }
    *reinterpret_cast<float*>(
        reinterpret_cast<char*>(g_equipmentStatsHandcuffMaster) +
        g_equipmentStatsHandcuffSpeedOffset) =
        g_equipmentStatsHandcuffVanillaSpeed;
}

void EquipmentStats_ForgetHandcuffProgressGoal()
{
    g_equipmentStatsHandcuffGoalController = nullptr;
    g_equipmentStatsHandcuffAppliedGoal =
        kEquipmentHandcuffBaseProgress2Goal;
    g_equipmentStatsHandcuffGoalOwned = false;
}

void EquipmentStats_RestoreHandcuffProgressGoal(
    Il2CppObject* handcuffController)
{
    if (!g_equipmentStatsHandcuffGoalOwned)
    {
        return;
    }
    if (handcuffController != nullptr &&
        handcuffController == g_equipmentStatsHandcuffGoalController)
    {
        *reinterpret_cast<float*>(
            reinterpret_cast<char*>(handcuffController) + 0x28) =
            kEquipmentHandcuffBaseProgress2Goal;
    }
    EquipmentStats_ForgetHandcuffProgressGoal();
}

void EquipmentStats_ApplyHandcuffProgressGoal(
    Il2CppObject* handcuffController,
    float goal)
{
    if (handcuffController == nullptr)
    {
        return;
    }
    if (!g_equipmentStatsHandcuffGoalOwned ||
        g_equipmentStatsHandcuffGoalController != handcuffController)
    {
        g_equipmentStatsHandcuffGoalController = handcuffController;
        g_equipmentStatsHandcuffGoalOwned = true;
    }
    *reinterpret_cast<float*>(
        reinterpret_cast<char*>(handcuffController) + 0x28) = goal;
    g_equipmentStatsHandcuffAppliedGoal = goal;
}

void EquipmentStats_PrepareHandcuffUnlock(
    Il2CppObject* playerController)
{
    if (playerController == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }
    auto* playerClassAccessor =
        *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerController) + 0x40);
    auto* handcuffController = playerClassAccessor != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0xF0)
        : nullptr;
    auto* gameStateData = ExtendedMode_GetGameStateData();
    if (handcuffController == nullptr || gameStateData == nullptr)
    {
        EquipmentStats_RestoreHandcuffMasterSpeed();
        EquipmentStats_ForgetHandcuffProgressGoal();
        g_equipmentStatsHandcuffController = nullptr;
        g_equipmentStatsHandcuffHoldStartTick = 0;
        g_equipmentStatsHandcuffAttempts = 0;
        g_equipmentStatsHandcuffObservedProgress2 = -1.0f;
        return;
    }

    // These persisted handcuff flags are fields of GameState.GameStateData.
    // PlayerStateModel uses different fields at the same offsets.
    const auto* rawGameState =
        reinterpret_cast<const char*>(gameStateData);
    const int attachedType =
        *reinterpret_cast<const int*>(rawGameState + 0x1AC);
    const bool attached =
        *reinterpret_cast<const bool*>(rawGameState + 0x1A8);
    const bool mapAttached =
        *reinterpret_cast<const bool*>(rawGameState + 0x1A9);
    const bool ordinaryHandcuff =
        (attached || mapAttached) &&
        attachedType == 3;
    const int forcePhase = handcuffController != nullptr
        ? *reinterpret_cast<const int*>(
            reinterpret_cast<const char*>(handcuffController) + 0x30)
        : -1;

    const bool attachmentStateChanged =
        g_equipmentStatsHandcuffObservedType != attachedType ||
        g_equipmentStatsHandcuffObservedAttached != (attached ? 1 : 0) ||
        g_equipmentStatsHandcuffObservedMapAttached !=
            (mapAttached ? 1 : 0);
    const bool forcePhaseChanged =
        g_equipmentStatsHandcuffObservedForcePhase != forcePhase;
    if (attachmentStateChanged || forcePhaseChanged)
    {
        g_equipmentStatsHandcuffObservedType = attachedType;
        g_equipmentStatsHandcuffObservedAttached = attached ? 1 : 0;
        g_equipmentStatsHandcuffObservedMapAttached =
            mapAttached ? 1 : 0;
        g_equipmentStatsHandcuffObservedForcePhase = forcePhase;
        g_equipmentStatsHandcuffHoldStartTick = 0;
        if (attachmentStateChanged)
        {
            g_equipmentStatsHandcuffAttempts = 0;
            g_equipmentStatsHandcuffObservedRevision = 0;
            g_equipmentStatsHandcuffObservedProgress2 = -1.0f;
        }
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsHandcuff",
            L"Handcuff controller observed: gameState=%p controller=%p type=%d "
            L"attached=%d mapAttached=%d forcePhase=%d ordinary=%d.",
            gameStateData,
            handcuffController,
            attachedType,
            attached ? 1 : 0,
            mapAttached ? 1 : 0,
            forcePhase,
            ordinaryHandcuff ? 1 : 0);
    }

    if (!EquipmentStats_EnsureHandcuffMasterSpeed())
    {
        return;
    }
    const int handcuffProficiency =
        EquipmentStats_GetEffectiveHandcuffProficiency();
    const float speedMultiplier = ordinaryHandcuff
        ? EquipmentStats_GetHandcuffHoldSpeedMultiplier()
        : 1.0f;
    *reinterpret_cast<float*>(
        reinterpret_cast<char*>(g_equipmentStatsHandcuffMaster) +
        g_equipmentStatsHandcuffSpeedOffset) =
        g_equipmentStatsHandcuffVanillaSpeed * speedMultiplier;

    if (!ordinaryHandcuff || handcuffController == nullptr)
    {
        if (!attached && !mapAttached)
        {
            EquipmentStats_RestoreHandcuffProgressGoal(handcuffController);
        }
        else
        {
            // Key and timer handcuffs own their original 0.01 target.
            EquipmentStats_ForgetHandcuffProgressGoal();
        }
        if (g_equipmentStatsHandcuffController != nullptr)
        {
            g_equipmentStatsHandcuffController = nullptr;
            g_equipmentStatsHandcuffHoldStartTick = 0;
            g_equipmentStatsHandcuffAttempts = 0;
            g_equipmentStatsHandcuffObservedProgress2 = -1.0f;
        }
        return;
    }
    g_equipmentStatsHandcuffController = handcuffController;
    const float successMultiplier =
        EquipmentStats_GetHandcuffSuccessGainMultiplier();
    const float goal = 1.0f / successMultiplier;
    EquipmentStats_ApplyHandcuffProgressGoal(handcuffController, goal);

    const unsigned int revision =
        g_equipmentStatsRevision.load(std::memory_order_relaxed);
    if (g_equipmentStatsHandcuffObservedRevision.exchange(revision) !=
        revision)
    {
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsHandcuff",
            L"Handcuff proficiency active: controller=%p stat=%d "
            L"vanillaSpeed=%.6f effectiveSpeed=%.6f speedMultiplier=%.6f "
            L"successMultiplier=%.6f effectiveGoal=%.6f revision=%u.",
            handcuffController,
            handcuffProficiency,
            g_equipmentStatsHandcuffVanillaSpeed,
            g_equipmentStatsHandcuffVanillaSpeed * speedMultiplier,
            speedMultiplier,
            successMultiplier,
            g_equipmentStatsHandcuffAppliedGoal,
            revision);
    }
}

void EquipmentStats_ObserveHandcuffUnlock(
    Il2CppObject* playerController)
{
    if (playerController == nullptr ||
        g_equipmentStatsHandcuffController == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }
    auto* controller = g_equipmentStatsHandcuffController;
    auto* rawController = reinterpret_cast<char*>(controller);
    const float progress =
        *reinterpret_cast<const float*>(rawController + 0x18);
    const float progress2 =
        *reinterpret_cast<const float*>(rawController + 0x1C);
    const unsigned long long now = GetTickCount64();
    if (progress > 0.0001f &&
        g_equipmentStatsHandcuffHoldStartTick == 0)
    {
        g_equipmentStatsHandcuffHoldStartTick = now;
    }
    if (progress <= 0.0001f)
    {
        g_equipmentStatsHandcuffHoldStartTick = 0;
    }

    if (g_equipmentStatsHandcuffObservedProgress2 < 0.0f ||
        std::fabs(
            progress2 -
            g_equipmentStatsHandcuffObservedProgress2) > 0.000001f)
    {
        const float previous =
            g_equipmentStatsHandcuffObservedProgress2;
        g_equipmentStatsHandcuffObservedProgress2 = progress2;
        if (previous < 0.0f || progress2 + 0.000001f < previous)
        {
            return;
        }
        ++g_equipmentStatsHandcuffAttempts;
        const unsigned long long holdStart =
            g_equipmentStatsHandcuffHoldStartTick;
        const unsigned long long holdMilliseconds =
            holdStart != 0 && now >= holdStart
                ? now - holdStart
                : 0;
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsHandcuff",
            L"Handcuff unlock attempt completed: attempt=%d "
            L"holdMilliseconds=%llu progress2=%.6f->%.6f.",
            g_equipmentStatsHandcuffAttempts,
            holdMilliseconds,
            previous,
            progress2);
        if (progress2 >= 0.9999f)
        {
            LogInfo(
                LogCategory_Action,
                L"EquipmentStatsHandcuff",
                L"Handcuff unlock succeeded: attempts=%d.",
                g_equipmentStatsHandcuffAttempts);
        }
    }
}

void PlayerHandcuffsControllerUpdateHook(
    Il2CppObject* instance,
    Il2CppObject* inputData,
    const Il2CppMethod* method)
{
    if (g_originalPlayerHandcuffsControllerUpdate == nullptr)
    {
        return;
    }
    using OriginalFn = void (*)(
        Il2CppObject*,
        Il2CppObject*,
        const Il2CppMethod*);
    auto callOriginal = [&]()
    {
        reinterpret_cast<OriginalFn>(
            g_originalPlayerHandcuffsControllerUpdate)(
                instance,
                inputData,
                method);
    };

    if (instance == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        callOriginal();
        return;
    }

    auto* rawController = reinterpret_cast<char*>(instance);
    auto* playerClassAccessor =
        *reinterpret_cast<Il2CppObject**>(rawController + 0x10);
    auto* playerState = playerClassAccessor != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(playerClassAccessor) + 0x60)
        : nullptr;
    if (playerState == nullptr)
    {
        callOriginal();
        return;
    }

    const auto* rawState =
        reinterpret_cast<const char*>(playerState);
    const int attachedHandcuffType =
        *reinterpret_cast<const int*>(rawState + 0x1AC);
    const int forceUnlockPhase =
        *reinterpret_cast<const int*>(rawController + 0x30);
    const bool ordinaryHandcuff = attachedHandcuffType == 3;
    const float progressBefore =
        *reinterpret_cast<const float*>(rawController + 0x18);
    const float progress2Before =
        *reinterpret_cast<const float*>(rawController + 0x1C);

    if (g_equipmentStatsHandcuffController != instance)
    {
        g_equipmentStatsHandcuffController = instance;
        g_equipmentStatsHandcuffHoldStartTick = 0;
        g_equipmentStatsHandcuffAttempts = 0;
        g_equipmentStatsHandcuffObservedRevision = 0;
        g_equipmentStatsHandcuffObservedType = -1;
        g_equipmentStatsHandcuffObservedForcePhase = -1;
    }

    const bool forcePhaseBecameActive =
        g_equipmentStatsHandcuffObservedForcePhase <= 0 &&
        forceUnlockPhase > 0;
    if (g_equipmentStatsHandcuffObservedType != attachedHandcuffType ||
        forcePhaseBecameActive)
    {
        g_equipmentStatsHandcuffObservedType = attachedHandcuffType;
        g_equipmentStatsHandcuffObservedForcePhase = forceUnlockPhase;
        g_equipmentStatsHandcuffHoldStartTick = 0;
        g_equipmentStatsHandcuffAttempts = 0;
        g_equipmentStatsHandcuffObservedRevision = 0;
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsHandcuff",
            L"Handcuff unlock controller observed: controller=%p type=%d "
            L"forcePhase=%d handcuffAttached=%d mapHandcuffAttached=%d.",
            instance,
            attachedHandcuffType,
            forceUnlockPhase,
            *reinterpret_cast<const bool*>(rawState + 0x1A8) ? 1 : 0,
            *reinterpret_cast<const bool*>(rawState + 0x1A9) ? 1 : 0);
    }

    if (!ordinaryHandcuff)
    {
        callOriginal();
        return;
    }

    const int handcuffProficiency =
        EquipmentStats_GetEffectiveHandcuffProficiency();
    const float holdSpeedMultiplier =
        EquipmentStats_GetHandcuffHoldSpeedMultiplier();
    const float successGainMultiplier =
        EquipmentStats_GetHandcuffSuccessGainMultiplier();
    const float baseProgress2Goal =
        *reinterpret_cast<const float*>(rawController + 0x28);
    const bool adjustGoal =
        std::fabs(baseProgress2Goal - 1.0f) <= 0.001f;

    Il2CppObject* mcommon = nullptr;
    Il2CppField* handcuffsSpeedField = nullptr;
    size_t handcuffsSpeedOffset = 0;
    float originalHandcuffsSpeed = 0.0f;
    float appliedProgress2Goal = baseProgress2Goal;
    if (handcuffProficiency != 0)
    {
        static Il2CppClass* mcommonClass = nullptr;
        static const Il2CppMethod* instanceGetter = nullptr;
        static Il2CppField* speedField = nullptr;
        static bool resolutionAttempted = false;
        if (!resolutionAttempted)
        {
            resolutionAttempted = true;
            mcommonClass = FindClass(
                "ExposureUnnoticed2.Master.Common",
                "MCommon");
            instanceGetter = FindMethod(
                mcommonClass,
                "get_Instance",
                0);
            speedField = il2cpp_class_get_field_from_name(
                mcommonClass,
                "HandcuffsSpeed");
        }
        if (instanceGetter != nullptr &&
            speedField != nullptr &&
            il2cpp_field_get_offset != nullptr)
        {
            mcommon = Invoke(instanceGetter, nullptr, nullptr);
            handcuffsSpeedField = speedField;
            if (mcommon != nullptr)
            {
                handcuffsSpeedOffset =
                    il2cpp_field_get_offset(handcuffsSpeedField);
                originalHandcuffsSpeed =
                    *reinterpret_cast<const float*>(
                        reinterpret_cast<const char*>(mcommon) +
                        handcuffsSpeedOffset);
                *reinterpret_cast<float*>(
                    reinterpret_cast<char*>(mcommon) +
                    handcuffsSpeedOffset) =
                    originalHandcuffsSpeed * holdSpeedMultiplier;
            }
        }
        else
        {
            LogOnce(
                "equipment-stats-handcuff-mcommon-missing",
                LogLevel_Warn,
                LogCategory_Action,
                L"EquipmentStatsHandcuff",
                L"Handcuff proficiency hook could not resolve "
                L"MCommon.HandcuffsSpeed; the original unlock path will run "
                L"unchanged.");
        }
        if (adjustGoal)
        {
            appliedProgress2Goal = 1.0f / successGainMultiplier;
            *reinterpret_cast<float*>(rawController + 0x28) =
                appliedProgress2Goal;
        }
    }

    const unsigned int revision =
        g_equipmentStatsRevision.load(std::memory_order_relaxed);
    if (handcuffProficiency != 0 &&
        g_equipmentStatsHandcuffObservedRevision.exchange(revision) !=
            revision)
    {
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsHandcuff",
            L"Handcuff proficiency hook active: controller=%p stat=%d "
            L"holdSpeedMultiplier=%.6f successGainMultiplier=%.6f "
            L"vanillaSpeed=%.6f effectiveSpeed=%.6f baseGoal=%.6f "
            L"effectiveGoal=%.6f revision=%u.",
            instance,
            handcuffProficiency,
            holdSpeedMultiplier,
            successGainMultiplier,
            originalHandcuffsSpeed,
            originalHandcuffsSpeed * holdSpeedMultiplier,
            baseProgress2Goal,
            appliedProgress2Goal,
            revision);
    }

    callOriginal();

    if (mcommon != nullptr && handcuffsSpeedField != nullptr)
    {
        *reinterpret_cast<float*>(
            reinterpret_cast<char*>(mcommon) +
            handcuffsSpeedOffset) =
            originalHandcuffsSpeed;
    }
    if (adjustGoal)
    {
        *reinterpret_cast<float*>(rawController + 0x28) =
            baseProgress2Goal;
    }

    const float progressAfter =
        *reinterpret_cast<const float*>(rawController + 0x18);
    const float progress2After =
        *reinterpret_cast<const float*>(rawController + 0x1C);
    const unsigned long long now = GetTickCount64();
    if (progressBefore <= 0.0001f &&
        progressAfter > progressBefore)
    {
        g_equipmentStatsHandcuffHoldStartTick = now;
    }

    const bool attemptCompleted =
        progress2After > progress2Before + 0.000001f;
    if (attemptCompleted)
    {
        ++g_equipmentStatsHandcuffAttempts;
        const unsigned long long holdStart =
            g_equipmentStatsHandcuffHoldStartTick;
        const unsigned long long holdMilliseconds =
            holdStart != 0 && now >= holdStart
                ? now - holdStart
                : 0;
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsHandcuff",
            L"Handcuff unlock attempt completed: attempt=%d "
            L"holdMilliseconds=%llu progress2=%.6f->%.6f "
            L"effectiveGoal=%.6f stat=%d.",
            g_equipmentStatsHandcuffAttempts,
            holdMilliseconds,
            progress2Before,
            progress2After,
            appliedProgress2Goal,
            handcuffProficiency);
        g_equipmentStatsHandcuffHoldStartTick = 0;
        if (progress2After >= 0.9999f)
        {
            LogInfo(
                LogCategory_Action,
                L"EquipmentStatsHandcuff",
                L"Handcuff unlock succeeded: attempts=%d stat=%d "
                L"holdSpeedMultiplier=%.6f successGainMultiplier=%.6f.",
                g_equipmentStatsHandcuffAttempts,
                handcuffProficiency,
                holdSpeedMultiplier,
                successGainMultiplier);
        }
    }
    else if (progressAfter <= 0.0001f)
    {
        g_equipmentStatsHandcuffHoldStartTick = 0;
    }
}

struct StaminaManagerFrameContext
{
    Il2CppObject* manager = nullptr;
    float recoveryBaseline = 0.0f;
    float postUpdateMaxCurrent = 0.0f;
    bool maxUpdated = false;
    bool active = false;
};

thread_local StaminaManagerFrameContext g_staminaFrameContext;
Il2CppObject* g_equipmentStatsStaminaManager = nullptr;
unsigned int g_equipmentStatsStaminaObservedRevision = 0;
unsigned long long g_equipmentStatsStaminaRecoveryLogTick = 0;
unsigned int g_equipmentStatsStaminaRecoveryLogRevision = 0;
bool g_equipmentStatsStaminaRecoveryStarted = false;

void EquipmentStats_ResetSceneRuntimeBindings()
{
    EquipmentStats_RestoreHandcuffMasterSpeed();
    g_equipmentStatsRewardWindowStart = 0;
    g_equipmentStatsRewardExposureActive = false;
    g_equipmentStatsHandcuffMaster = nullptr;
    g_equipmentStatsHandcuffSpeedField = nullptr;
    g_equipmentStatsHandcuffSpeedOffset = 0;
    g_equipmentStatsHandcuffVanillaSpeed = 0.0f;
    g_equipmentStatsHandcuffSpeedCaptured = false;
    g_equipmentStatsStaminaManager = nullptr;
    g_equipmentStatsStaminaObservedRevision = 0;
    g_equipmentStatsStaminaRecoveryStarted = false;
    g_equipmentStatsStaminaRecoveryLogTick = 0;
    g_equipmentStatsStaminaRecoveryLogRevision = 0;
    g_equipmentStatsHandcuffController = nullptr;
    g_equipmentStatsHandcuffHoldStartTick = 0;
    g_equipmentStatsHandcuffAttempts = 0;
    g_equipmentStatsHandcuffObservedRevision = 0;
    g_equipmentStatsHandcuffObservedType = -1;
    g_equipmentStatsHandcuffObservedForcePhase = -1;
    g_equipmentStatsHandcuffObservedAttached = -1;
    g_equipmentStatsHandcuffObservedMapAttached = -1;
    g_equipmentStatsHandcuffObservedProgress2 = -1.0f;
    g_equipmentStatsHandcuffGoalController = nullptr;
    g_equipmentStatsHandcuffAppliedGoal =
        kEquipmentHandcuffBaseProgress2Goal;
    g_equipmentStatsHandcuffGoalOwned = false;
    g_staminaFrameContext = StaminaManagerFrameContext{};
}

bool EquipmentStats_ReadStaminaSnapshot(float& current, float& max)
{
    auto* manager = g_equipmentStatsStaminaManager;
    if (manager == nullptr)
    {
        return false;
    }
    const auto* raw = reinterpret_cast<const char*>(manager);
    current = *reinterpret_cast<const float*>(raw + 0x10);
    max = *reinterpret_cast<const float*>(raw + 0x14);
    return std::isfinite(current) &&
        std::isfinite(max) &&
        max > 0.0f;
}

float EquipmentStats_ReadStaminaBaseMax(Il2CppObject* manager)
{
    if (manager == nullptr)
    {
        return -1.0f;
    }
    const auto* raw = reinterpret_cast<const char*>(manager);
    auto* record = *reinterpret_cast<Il2CppObject* const*>(raw + 0x20);
    if (record == nullptr)
    {
        return -1.0f;
    }
    const int reinforce = *reinterpret_cast<const int*>(raw + 0x28);
    const int difficulty = *reinterpret_cast<const int*>(raw + 0x30);
    float maxStamina =
        *reinterpret_cast<const float*>(
            reinterpret_cast<const char*>(record) + 0x20);
    maxStamina *= 1.0f + static_cast<float>(reinforce) * 0.01f;
    if (difficulty != 2)
    {
        maxStamina += 0.5f;
    }
    return std::max(0.0f, maxStamina);
}

bool EquipmentStats_ApplyStaminaCapacity(
    Il2CppObject* manager,
    float baseMax,
    float missing,
    const wchar_t* reason)
{
    if (manager == nullptr ||
        !std::isfinite(baseMax) ||
        baseMax < 0.0f ||
        !std::isfinite(missing))
    {
        return false;
    }
    auto* raw = reinterpret_cast<char*>(manager);
    const float oldCurrent = *reinterpret_cast<float*>(raw + 0x10);
    const float oldMax = *reinterpret_cast<float*>(raw + 0x14);
    const float multiplier = EquipmentStats_GetStaminaCapacityMultiplier();
    const float newMax = baseMax * multiplier;
    const float newCurrent = std::clamp(
        newMax - std::max(0.0f, missing),
        0.0f,
        newMax);
    *reinterpret_cast<float*>(raw + 0x14) = newMax;
    *reinterpret_cast<float*>(raw + 0x10) = newCurrent;
    g_equipmentStatsStaminaManager = manager;

    const unsigned int revision =
        g_equipmentStatsRevision.load(std::memory_order_relaxed);
    const bool changed =
        std::fabs(oldMax - newMax) > 0.00001f ||
        std::fabs(oldCurrent - newCurrent) > 0.00001f ||
        g_equipmentStatsStaminaObservedRevision != revision;
    g_equipmentStatsStaminaObservedRevision = revision;
    if (changed)
    {
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsStamina",
            L"Stamina max applied (%s): baseMax=%.4f multiplier=%.6f "
            L"oldMax=%.4f newMax=%.4f oldCurrent=%.4f newCurrent=%.4f "
            L"missing=%.4f capacity=%d recovery=%d revision=%u.",
            reason != nullptr ? reason : L"unspecified",
            baseMax,
            multiplier,
            oldMax,
            newMax,
            oldCurrent,
            newCurrent,
            missing,
            EquipmentStats_GetEffectiveStaminaCapacity(),
            EquipmentStats_GetEffectiveStaminaRecovery(),
            revision);
    }
    return true;
}

bool EquipmentStats_EnsureStaminaCapacity(
    Il2CppObject* manager,
    const wchar_t* reason)
{
    if (manager == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return false;
    }
    const unsigned int revision =
        g_equipmentStatsRevision.load(std::memory_order_relaxed);
    if (manager == g_equipmentStatsStaminaManager &&
        revision == g_equipmentStatsStaminaObservedRevision)
    {
        return true;
    }
    if (manager != g_equipmentStatsStaminaManager)
    {
        g_equipmentStatsStaminaRecoveryStarted = false;
    }
    const float baseMax = EquipmentStats_ReadStaminaBaseMax(manager);
    if (baseMax < 0.0f)
    {
        LogRateLimited(
            "equipment-stats-stamina-record-missing",
            5000,
            LogLevel_Warn,
            LogCategory_Action,
            L"EquipmentStatsStamina",
            L"Unable to apply stamina stats because the skill record is "
            L"unavailable: manager=%p.",
            manager);
        return false;
    }
    const auto* raw = reinterpret_cast<const char*>(manager);
    const float current = *reinterpret_cast<const float*>(raw + 0x10);
    const float max = *reinterpret_cast<const float*>(raw + 0x14);
    return EquipmentStats_ApplyStaminaCapacity(
        manager,
        baseMax,
        std::max(0.0f, max - current),
        reason);
}

void StaminaManagerUpdateMaxHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_originalStaminaManagerUpdateMax != nullptr)
        {
            using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
            reinterpret_cast<OriginalFn>(
                g_originalStaminaManagerUpdateMax)(instance, method);
        }
        return;
    }
    if (instance == nullptr)
    {
        return;
    }
    const auto* beforeRaw = reinterpret_cast<const char*>(instance);
    const float beforeCurrent =
        *reinterpret_cast<const float*>(beforeRaw + 0x10);
    const float beforeMax =
        *reinterpret_cast<const float*>(beforeRaw + 0x14);
    const float missing = std::max(0.0f, beforeMax - beforeCurrent);

    if (g_originalStaminaManagerUpdateMax != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalStaminaManagerUpdateMax)(
            instance,
            method);
    }

    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        return;
    }
    const auto* vanillaRaw = reinterpret_cast<const char*>(instance);
    const float vanillaMax = *reinterpret_cast<const float*>(vanillaRaw + 0x14);
    if (!EquipmentStats_ApplyStaminaCapacity(
            instance,
            vanillaMax,
            missing,
            L"UpdateMax"))
    {
        return;
    }
    if (g_staminaFrameContext.active &&
        g_staminaFrameContext.manager == instance)
    {
        g_staminaFrameContext.maxUpdated = true;
        g_staminaFrameContext.postUpdateMaxCurrent =
            *reinterpret_cast<const float*>(vanillaRaw + 0x10);
    }
}

void StaminaManagerConsumePerSecondHook(
    Il2CppObject* instance,
    float stamina,
    const Il2CppMethod* method)
{
    if (instance == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_originalStaminaManagerConsumePerSecond != nullptr)
        {
            using OriginalFn = void (*)(
                Il2CppObject*,
                float,
                const Il2CppMethod*);
            reinterpret_cast<OriginalFn>(
                g_originalStaminaManagerConsumePerSecond)(
                    instance,
                    stamina,
                    method);
        }
        return;
    }
    EquipmentStats_EnsureStaminaCapacity(instance, L"ConsumePerSecond");

    auto* raw = reinterpret_cast<char*>(instance);
    const float beforeCurrent = *reinterpret_cast<float*>(raw + 0x10);
    const float consumptionMultiplier =
        EquipmentStats_GetStaminaConsumptionMultiplier();
    const float cooldownMultiplier =
        EquipmentStats_GetStaminaCooldownMultiplier();
    const float scaledStamina = stamina * consumptionMultiplier;

    if (g_originalStaminaManagerConsumePerSecond != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            float,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalStaminaManagerConsumePerSecond)(
                instance,
                scaledStamina,
                method);
    }
    auto* record = *reinterpret_cast<Il2CppObject**>(raw + 0x20);
    float vanillaCooldown = *reinterpret_cast<float*>(raw + 0x18);
    float scaledCooldown = vanillaCooldown;
    if (record != nullptr &&
        std::fabs(cooldownMultiplier - 1.0f) > 0.000001f)
    {
        scaledCooldown = vanillaCooldown * cooldownMultiplier;
        *reinterpret_cast<float*>(raw + 0x18) = scaledCooldown;
    }

    const unsigned long long now = GetTickCount64();
    const unsigned int revision =
        g_equipmentStatsRevision.load(std::memory_order_relaxed);
    const int staminaRecovery =
        EquipmentStats_GetEffectiveStaminaRecovery();
    const int staminaCapacity =
        EquipmentStats_GetEffectiveStaminaCapacity();
    const bool diagnostic = staminaRecovery != 0 || staminaCapacity != 0;
    if (diagnostic &&
        (revision != g_equipmentStatsStaminaRecoveryLogRevision ||
         g_equipmentStatsStaminaRecoveryLogTick == 0 ||
         now - g_equipmentStatsStaminaRecoveryLogTick >= 1000))
    {
        g_equipmentStatsStaminaRecoveryLogRevision = revision;
        g_equipmentStatsStaminaRecoveryLogTick = now;
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsStamina",
            L"Stamina consumption sample: currentBefore=%.4f "
            L"currentAfter=%.4f max=%.4f vanillaInput=%.6f scaledInput=%.6f "
            L"vanillaCooldown=%.4f scaledCooldown=%.4f "
            L"capacity=%d recovery=%d consumptionMultiplier=%.6f "
            L"cooldownMultiplier=%.6f isZero=%d revision=%u.",
            beforeCurrent,
            *reinterpret_cast<float*>(raw + 0x10),
            *reinterpret_cast<float*>(raw + 0x14),
            stamina,
            scaledStamina,
            vanillaCooldown,
            scaledCooldown,
            staminaCapacity,
            staminaRecovery,
            consumptionMultiplier,
            cooldownMultiplier,
            *reinterpret_cast<bool*>(raw + 0x2C) ? 1 : 0,
            revision);
    }
}

void StaminaManagerOnUpdateHook(
    Il2CppObject* instance,
    const Il2CppMethod* method)
{
    if (instance == nullptr ||
        g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_originalStaminaManagerOnUpdate != nullptr)
        {
            using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
            reinterpret_cast<OriginalFn>(
                g_originalStaminaManagerOnUpdate)(instance, method);
        }
        return;
    }
    EquipmentStats_EnsureStaminaCapacity(instance, L"OnUpdate");

    auto* raw = reinterpret_cast<char*>(instance);
    const float preparedCurrent = *reinterpret_cast<float*>(raw + 0x10);
    g_staminaFrameContext = StaminaManagerFrameContext{};
    g_staminaFrameContext.active = true;
    g_staminaFrameContext.manager = instance;
    g_staminaFrameContext.recoveryBaseline = preparedCurrent;

    if (g_originalStaminaManagerOnUpdate != nullptr)
    {
        using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(g_originalStaminaManagerOnUpdate)(
            instance,
            method);
    }

    const bool observe =
        g_staminaFrameContext.active &&
        g_staminaFrameContext.manager == instance &&
        g_extendedModeState.load() == ExtendedMode_Gameplay;
    const float baseline = g_staminaFrameContext.maxUpdated
        ? g_staminaFrameContext.postUpdateMaxCurrent
        : g_staminaFrameContext.recoveryBaseline;
    g_staminaFrameContext = StaminaManagerFrameContext{};
    if (!observe)
    {
        return;
    }

    const float current = *reinterpret_cast<float*>(raw + 0x10);
    const float max = *reinterpret_cast<float*>(raw + 0x14);
    const float vanillaDelta = current - baseline;
    const int staminaRecovery =
        EquipmentStats_GetEffectiveStaminaRecovery();
    const float recoveryMultiplier =
        EquipmentStats_GetStaminaRecoveryMultiplier();
    if (staminaRecovery != 0 &&
        vanillaDelta > 0.000001f &&
        std::isfinite(vanillaDelta) &&
        std::isfinite(current) &&
        std::isfinite(max))
    {
        const float adjusted = std::clamp(
            baseline + vanillaDelta * recoveryMultiplier,
            0.0f,
            max);
        *reinterpret_cast<float*>(raw + 0x10) = adjusted;
        if (!g_equipmentStatsStaminaRecoveryStarted)
        {
            g_equipmentStatsStaminaRecoveryStarted = true;
            LogInfo(
                LogCategory_Action,
                L"EquipmentStatsStamina",
                L"Stamina recovery started: baseline=%.4f "
                L"vanillaAfter=%.4f after=%.4f vanillaDelta=%.6f "
                L"recovery=%d recoveryMultiplier=%.6f max=%.4f "
                L"cooldown=%.4f revision=%u.",
                baseline,
                current,
                adjusted,
                vanillaDelta,
                staminaRecovery,
                recoveryMultiplier,
                max,
                *reinterpret_cast<float*>(raw + 0x18),
                g_equipmentStatsRevision.load(
                    std::memory_order_relaxed));
        }
    }
    else if (current >= max - 0.00001f ||
             *reinterpret_cast<float*>(raw + 0x18) > 0.0f)
    {
        g_equipmentStatsStaminaRecoveryStarted = false;
    }
}

void StrangenessValueManagerSetValueHook(
    Il2CppObject* valueManager,
    float value,
    const Il2CppMethod* method)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_originalStrangenessValueSet != nullptr)
        {
            using OriginalFn = void (*)(
                Il2CppObject*,
                float,
                const Il2CppMethod*);
            reinterpret_cast<OriginalFn>(
                g_originalStrangenessValueSet)(
                    valueManager,
                    value,
                    method);
        }
        return;
    }
    if (g_originalStrangenessValueSet != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            float,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalStrangenessValueSet)(
                valueManager,
                value,
                method);
    }
    EquipmentStats_ApplyNormalityWaitCount(valueManager);
}

void StrangenessValueManagerUpdateHook(
    Il2CppObject* valueManager,
    bool isSkipReduceCount,
    const Il2CppMethod* method)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_originalStrangenessValueUpdate != nullptr)
        {
            using OriginalFn = void (*)(
                Il2CppObject*,
                bool,
                const Il2CppMethod*);
            reinterpret_cast<OriginalFn>(
                g_originalStrangenessValueUpdate)(
                    valueManager,
                    isSkipReduceCount,
                    method);
        }
        return;
    }
    if (valueManager == nullptr)
    {
        return;
    }

    const float reducePerSecond =
        kStrangenessBaseReducePerSecond *
        EquipmentStats_GetNormalityDecayMultiplier();
    *reinterpret_cast<float*>(
        reinterpret_cast<char*>(valueManager) + 0x10) =
        reducePerSecond;

    if (g_originalStrangenessValueUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            bool,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalStrangenessValueUpdate)(
                valueManager,
                isSkipReduceCount,
                method);
    }

    const unsigned int revision =
        g_equipmentStatsRevision.load(std::memory_order_relaxed);
    if (g_equipmentStatsNormalityObservedRevision.exchange(revision) !=
        revision)
    {
        const auto* raw =
            reinterpret_cast<const char*>(valueManager);
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsNormality",
            L"Normality hook active: manager=%p value=%.6f count=%d "
            L"reducePerSecond=%.6f stat=%d delay=%.6f decay=%.6f.",
            valueManager,
            *reinterpret_cast<const float*>(raw + 0x14),
            *reinterpret_cast<const int*>(raw + 0x18),
            reducePerSecond,
            EquipmentStats_GetEffectiveNormality(),
            EquipmentStats_GetNormalityDelayMultiplier(),
            EquipmentStats_GetNormalityDecayMultiplier());
    }
}

void NpcStrangenessManagerUpdateHook(
    Il2CppObject* manager,
    const Il2CppMethod* method)
{
    if (g_extendedModeState.load() != ExtendedMode_Gameplay)
    {
        if (g_originalNpcStrangenessManagerUpdate != nullptr)
        {
            using OriginalFn = void (*)(Il2CppObject*, const Il2CppMethod*);
            reinterpret_cast<OriginalFn>(
                g_originalNpcStrangenessManagerUpdate)(manager, method);
        }
        return;
    }
    if (manager == nullptr)
    {
        return;
    }

    auto* valueManager = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(manager) + 0x10);
    const float before = valueManager != nullptr
        ? *reinterpret_cast<float*>(
            reinterpret_cast<char*>(valueManager) + 0x14)
        : 0.0f;

    if (g_originalNpcStrangenessManagerUpdate != nullptr)
    {
        using OriginalFn = void (*)(
            Il2CppObject*,
            const Il2CppMethod*);
        reinterpret_cast<OriginalFn>(
            g_originalNpcStrangenessManagerUpdate)(
                manager,
                method);
    }

    valueManager = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(manager) + 0x10);
    if (valueManager == nullptr)
    {
        return;
    }
    const float after = *reinterpret_cast<float*>(
        reinterpret_cast<char*>(valueManager) + 0x14);
    if (!(after > before))
    {
        return;
    }

    auto* nca = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(manager) + 0x28);
    auto* stateModel = nca != nullptr
        ? *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(nca) + 0x40)
        : nullptr;
    const bool alreadyFound =
        stateModel != nullptr &&
        *reinterpret_cast<bool*>(
            reinterpret_cast<char*>(stateModel) + 0x1D);
    if (alreadyFound)
    {
        return;
    }

    const float multiplier =
        EquipmentStats_GetConservatismMultiplier();
    const float modified = std::clamp(
        before + (after - before) * multiplier,
        0.0f,
        1.0f);
    *reinterpret_cast<float*>(
        reinterpret_cast<char*>(valueManager) + 0x14) =
        modified;
    EquipmentStats_ApplyNormalityWaitCount(valueManager);

    const unsigned int revision =
        g_equipmentStatsRevision.load(std::memory_order_relaxed);
    if (g_equipmentStatsConservatismObservedRevision.exchange(revision) !=
        revision)
    {
        LogInfo(
            LogCategory_Action,
            L"EquipmentStatsConservatism",
            L"Conservatism hook active: npcManager=%p before=%.6f vanilla=%.6f "
            L"value=%.6f delta=%.6f stat=%d multiplier=%.6f.",
            manager,
            before,
            after,
            modified,
            modified - before,
            EquipmentStats_GetEffectiveConservatism(),
            multiplier);
    }
}
