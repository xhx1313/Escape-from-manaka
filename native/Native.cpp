#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <MinHook.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cwchar>
#include <deque>
#include <fstream>
#include <functional>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <nlohmann/json.hpp>
#include "mod_version.inc"

namespace
{
constexpr int kLobbyPort = 12121;
constexpr UINT_PTR kOverlayPollTimer = 1;
constexpr UINT_PTR kMenuPollTimer = 2;
constexpr UINT kRefreshMenuTextMessage = WM_APP + 10;
constexpr UINT kTestUiOpenMessage = WM_APP + 50;
constexpr UINT kTestUiCloseMessage = WM_APP + 51;
constexpr UINT kTestUiTabMessage = WM_APP + 52;
constexpr UINT kTestUiLobbyActionMessage = WM_APP + 53;
constexpr UINT kTestUiPlayerProbeMessage = WM_APP + 54;
constexpr UINT kTestEfmActionMessage = WM_APP + 55;
constexpr UINT kTestEfmSystemMenuMessage = WM_APP + 56;
constexpr UINT kTestEfmMoveMessage = WM_APP + 57;
constexpr UINT kTestEfmExitMessage = WM_APP + 58;
constexpr int kOverlayButtonId = 0x4D41;
constexpr int kHostButtonId = 100;
constexpr int kJoinButtonId = 101;
constexpr int kStopButtonId = 102;
constexpr int kCloseButtonId = 103;
constexpr int kAddressEditId = 200;
constexpr int kStatusLabelId = 300;
constexpr int kInfoLabelId = 301;

HINSTANCE g_instance = nullptr;
HWND g_messageWindow = nullptr;
HWND g_gameWindow = nullptr;
HWND g_overlayWindow = nullptr;
HWND g_menuWindow = nullptr;
HWND g_addressEdit = nullptr;
HWND g_statusLabel = nullptr;
HWND g_infoLabel = nullptr;
HWND g_connectedOverlay = nullptr;
HFONT g_uiFont = nullptr;

std::atomic<bool> g_stopRequested{false};
std::atomic<bool> g_runtimeShutdownRequested{false};
std::atomic<bool> g_runtimeThreadStopped{false};
std::atomic<bool> g_titlePresent{false};
std::atomic<bool> g_titleApiReady{false};
std::atomic<bool> g_titleProbeComplete{false};
std::atomic<bool> g_fallbackVisible{false};
std::atomic<bool> g_networkStop{false};
std::atomic<unsigned long long> g_statusVersion{1};
std::atomic<int> g_activeUiTab{0};
std::atomic<int> g_pendingUiCommand{0};
std::atomic<int> g_playerSceneProbeFrames{0};

std::mutex g_statusMutex;
std::wstring g_status = L"Idle";
std::thread g_hostThread;
std::thread g_clientThread;
std::thread g_discoveryThread;
SOCKET g_listenSocket = INVALID_SOCKET;
SOCKET g_peerSocket = INVALID_SOCKET;
SOCKET g_discoverySocket = INVALID_SOCKET;

using Il2CppDomain = void;
using Il2CppAssembly = void;
using Il2CppImage = void;
using Il2CppClass = void;
using Il2CppField = void;
using Il2CppMethod = void;
using Il2CppObject = void;
using Il2CppString = void;

using DomainGetFn = Il2CppDomain* (*)();
using ThreadAttachFn = void* (*)(Il2CppDomain*);
using DomainGetAssembliesFn = const Il2CppAssembly** (*)(const Il2CppDomain*, size_t*);
using AssemblyGetImageFn = Il2CppImage* (*)(const Il2CppAssembly*);
using ClassFromNameFn = Il2CppClass* (*)(const Il2CppImage*, const char*, const char*);
using ClassGetFieldFromNameFn = Il2CppField* (*)(Il2CppClass*, const char*);
using ClassGetFieldsFn = Il2CppField* (*)(Il2CppClass*, void**);
using FieldGetNameFn = const char* (*)(Il2CppField*);
using FieldStaticGetValueFn = void (*)(Il2CppField*, void*);
using FieldStaticSetValueFn = void (*)(Il2CppField*, void*);
using FieldGetValueFn = void (*)(Il2CppField*, Il2CppObject*, void*);
using FieldGetOffsetFn = size_t (*)(Il2CppField*);
using ClassGetMethodsFn = const Il2CppMethod* (*)(Il2CppClass*, void**);
using MethodGetNameFn = const char* (*)(const Il2CppMethod*);
using MethodGetParamCountFn = uint8_t (*)(const Il2CppMethod*);
using MethodIsGenericFn = bool (*)(const Il2CppMethod*);
using RuntimeInvokeFn = Il2CppObject* (*)(const Il2CppMethod*, Il2CppObject*, void**, Il2CppObject**);
using StringNewFn = Il2CppString* (*)(const char*);
using StringCharsFn = const uint16_t* (*)(Il2CppString*);
using StringLengthFn = int32_t (*)(Il2CppString*);
using ObjectUnboxFn = void* (*)(Il2CppObject*);
using ArrayLengthFn = uint32_t (*)(Il2CppObject*);
using ArrayObjectHeaderSizeFn = int32_t (*)();
using ArrayNewFn = Il2CppObject* (*)(Il2CppClass*, size_t);
using ClassGetTypeFn = void* (*)(Il2CppClass*);
using TypeGetObjectFn = Il2CppObject* (*)(void*);
using RuntimeClassInitFn = void (*)(Il2CppClass*);
using ObjectNewFn = Il2CppObject* (*)(Il2CppClass*);
using ObjectGetClassFn = Il2CppClass* (*)(Il2CppObject*);
using GcWbarrierSetFieldFn = void (*)(Il2CppObject*, void**, Il2CppObject*);
using GcHandleNewFn = uint32_t (*)(Il2CppObject*, bool);
using GcHandleGetTargetFn = Il2CppObject* (*)(uint32_t);
using GcHandleFreeFn = void (*)(uint32_t);
using PlayerTitChangeFn = void (*)(Il2CppObject*, Il2CppObject*, const void*);
using Il2CppThrowFn = void (*)();

DomainGetFn il2cpp_domain_get = nullptr;
ThreadAttachFn il2cpp_thread_attach = nullptr;
DomainGetAssembliesFn il2cpp_domain_get_assemblies = nullptr;
AssemblyGetImageFn il2cpp_assembly_get_image = nullptr;
ClassFromNameFn il2cpp_class_from_name = nullptr;
ClassGetFieldFromNameFn il2cpp_class_get_field_from_name = nullptr;
ClassGetFieldsFn il2cpp_class_get_fields = nullptr;
FieldGetNameFn il2cpp_field_get_name = nullptr;
FieldStaticGetValueFn il2cpp_field_static_get_value = nullptr;
FieldStaticSetValueFn il2cpp_field_static_set_value = nullptr;
FieldGetValueFn il2cpp_field_get_value = nullptr;
FieldGetOffsetFn il2cpp_field_get_offset = nullptr;
ClassGetMethodsFn il2cpp_class_get_methods = nullptr;
MethodGetNameFn il2cpp_method_get_name = nullptr;
MethodGetParamCountFn il2cpp_method_get_param_count = nullptr;
MethodIsGenericFn il2cpp_method_is_generic = nullptr;
RuntimeInvokeFn il2cpp_runtime_invoke = nullptr;
StringNewFn il2cpp_string_new = nullptr;
StringCharsFn il2cpp_string_chars = nullptr;
StringLengthFn il2cpp_string_length = nullptr;
ObjectUnboxFn il2cpp_object_unbox = nullptr;
ArrayLengthFn il2cpp_array_length = nullptr;
ArrayObjectHeaderSizeFn il2cpp_array_object_header_size = nullptr;
ArrayNewFn il2cpp_array_new = nullptr;
ClassGetTypeFn il2cpp_class_get_type = nullptr;
TypeGetObjectFn il2cpp_type_get_object = nullptr;
RuntimeClassInitFn il2cpp_runtime_class_init = nullptr;
ObjectNewFn il2cpp_object_new = nullptr;
ObjectGetClassFn il2cpp_object_get_class = nullptr;
GcWbarrierSetFieldFn il2cpp_gc_wbarrier_set_field = nullptr;
GcHandleNewFn il2cpp_gchandle_new = nullptr;
GcHandleGetTargetFn il2cpp_gchandle_get_target = nullptr;
GcHandleFreeFn il2cpp_gchandle_free = nullptr;

Il2CppDomain* g_domain = nullptr;
Il2CppClass* g_titleClass = nullptr;
Il2CppField* g_titleInstanceField = nullptr;
Il2CppObject* g_titleInstance = nullptr;
Il2CppObject* g_nativeButton = nullptr;
Il2CppObject* g_nativeButtonComponent = nullptr;
Il2CppObject* g_nativeButtonRect = nullptr;
Il2CppObject* g_hostButtonComponent = nullptr;
Il2CppObject* g_joinButtonComponent = nullptr;
Il2CppObject* g_continueButtonComponent = nullptr;
Il2CppObject* g_startButtonComponent = nullptr;
Il2CppObject* g_uiRoot = nullptr;
Il2CppObject* g_uiBackdrop = nullptr;
Il2CppObject* g_equipmentPage = nullptr;
Il2CppObject* g_lobbyPage = nullptr;
Il2CppObject* g_equipmentTabButton = nullptr;
Il2CppObject* g_lobbyTabButton = nullptr;
Il2CppObject* g_closeUiButton = nullptr;
Il2CppObject* g_fleaMarketButton = nullptr;
Il2CppObject* g_startGameButton = nullptr;
Il2CppObject* g_lobbyStatusLabel = nullptr;
Il2CppObject* g_roomListLabel = nullptr;
Il2CppObject* g_titleCanvasComponent = nullptr;
Il2CppObject* g_multiplayerCanvasObject = nullptr;
Il2CppObject* g_multiplayerScene = nullptr;
Il2CppObject* g_titleScene = nullptr;
Il2CppObject* g_tmpFontAsset = nullptr;
Il2CppObject* g_fullScUnityFont = nullptr;
bool g_tmpFontUsesFullScHook = false;
Il2CppObject* g_previewStageComponent = nullptr;
Il2CppObject* g_previewCameraComponent = nullptr;
Il2CppObject* g_previewRenderTexture = nullptr;
Il2CppObject* g_previewModel = nullptr;
Il2CppObject* g_previewAnimator = nullptr;
Il2CppObject* g_previewAssetBundle = nullptr;
Il2CppObject* g_efmLoadoutPanelTransform = nullptr;
Il2CppObject* g_efmPreparingScene = nullptr;
Il2CppObject* g_efmUiPlayerGameObject = nullptr;
Il2CppObject* g_efmUiPlayerController = nullptr;
Il2CppObject* g_efmPreviewController = nullptr;
Il2CppObject* g_efmPreviewIdleClip = nullptr;
Il2CppObject* g_efmDebugActionPanel = nullptr;
uint32_t g_efmSavedPlayerCustomDataHandle = 0;
uint32_t g_efmPreviewGameStateDataHandle = 0;
uint32_t g_efmPreviewCostumeDictHandle = 0;
uint32_t g_efmPreviewCosplayDictHandle = 0;
uint32_t g_efmPreviewPlayerClassAccessorHandle = 0;
uint32_t g_efmPreviewBodyAdjusterHandle = 0;
uint32_t g_efmPreviewTitControllerHandle = 0;
std::atomic<bool> g_efmSkipPlayerTitChange{false};
void* g_originalPlayerTitChange = nullptr;
void* g_originalIl2CppThrowNull = nullptr;
std::atomic<bool> g_efmTraceBodyNull{false};
std::atomic<int> g_efmPreparingState{0};
std::atomic<int> g_efmPreparingFrames{0};
std::atomic<bool> g_efmInitializeInvoked{false};
std::atomic<bool> g_efmPoseResetInvoked{false};
std::atomic<bool> g_efmToggleActionListRequested{false};
std::atomic<bool> g_efmActionListOpen{false};
std::atomic<bool> g_efmAudioSilenced{false};
bool g_efmModelDragging = false;
float g_efmModelYaw = 0.0f;
POINT g_efmModelDragPoint{};
int g_efmSelectedAction = 0;
int g_efmActionPage = 0;
std::atomic<int> g_efmPendingAction{-1};
std::vector<Il2CppObject*> g_efmActionButtons;
std::vector<Il2CppObject*> g_efmActionLabels;
Il2CppObject* g_efmActionPageLabel = nullptr;
Il2CppObject* g_efmPrevActionPageButton = nullptr;
Il2CppObject* g_efmNextActionPageButton = nullptr;
std::vector<Il2CppObject*> g_uiButtons;
std::vector<Il2CppObject*> g_roomRowLabels;
std::vector<Il2CppObject*> g_titleMenuButtons;

// Escape from Manaka gameplay integration. This state is deliberately
// independent from the legacy title-scene preview state above.
Il2CppObject* g_extendedSavePanel = nullptr;
Il2CppObject* g_extendedInGameManager = nullptr;
Il2CppObject* g_extendedInGameReferencer = nullptr;
Il2CppObject* g_extendedPlayerController = nullptr;
Il2CppObject* g_extendedPlayerGameObject = nullptr;
Il2CppObject* g_extendedAvatarReferencer = nullptr;
Il2CppObject* g_extendedAvatarRoot = nullptr;
Il2CppObject* g_extendedCamera = nullptr;
Il2CppObject* g_extendedInGameUiManager = nullptr;
Il2CppObject* g_extendedCanvas = nullptr;
Il2CppObject* g_extendedActionPanel = nullptr;
Il2CppObject* g_extendedSelectedActionLabel = nullptr;
Il2CppObject* g_extendedPageLabel = nullptr;
Il2CppObject* g_extendedPrevPageButton = nullptr;
Il2CppObject* g_extendedNextPageButton = nullptr;
std::vector<Il2CppObject*> g_extendedActionButtons;
std::vector<Il2CppObject*> g_extendedActionLabels;
std::atomic<int> g_extendedModeState{0};
constexpr int kExtendedModeActiveStateValue = 4;
std::atomic<int> g_extendedStateFrames{0};
std::atomic<bool> g_extendedSaveSelectionInvoked{false};
std::atomic<bool> g_extendedTakeoverComplete{false};
std::atomic<bool> g_extendedSuppressPlayerUpdate{false};
std::atomic<bool> g_extendedToggleActionListRequested{false};
std::atomic<bool> g_extendedToggleEquipmentPreviewRequested{false};
std::atomic<bool> g_extendedToggleEquipmentCatalogBatchRequested{false};
std::atomic<bool> g_extendedActionListOpen{false};
std::atomic<bool> g_extendedExitRequested{false};
std::atomic<bool> g_extendedAwaitingReturnConfirmation{false};
std::atomic<bool> g_gameplaySpecialCoatPhysicalDown{false};
std::atomic<bool> g_extendedOpenSystemMenuRequested{false};
std::atomic<bool> g_extendedEscapeDismissRequested{false};
std::atomic<bool> g_extendedEscapeDismissConsumed{false};
std::atomic<unsigned long long> g_extendedEscapeDismissBlockedUntil{0};
std::atomic<bool> g_extendedToggleGameplayInventoryRequested{false};
std::atomic<bool> g_gameplayInventoryOpen{false};
std::atomic<bool> g_gameplayWorldMapOpen{false};
std::atomic<bool> g_gameplayWorldMapToggleRequested{false};
std::atomic<bool> g_checkpointSearchRequested{false};
std::atomic<bool> g_checkpointInteractActionDown{false};
std::atomic<bool> g_checkpointInteractionKeyConsumed{false};
std::atomic<bool> g_checkpointAbandonActionDown{false};
std::atomic<bool> g_checkpointAbandonTaskRequested{false};
std::atomic_bool g_checkpointFailureUiActive{false};
std::atomic<bool> g_checkpointMapEditorToggleRequested{false};
std::atomic<bool> g_checkpointMapEditorEditRequested{false};
std::atomic<int> g_checkpointMapEditorActionRequested{-1};
extern std::string g_checkpointPendingTransferSpawnId;
std::atomic<bool> g_extendedOriginalSystemMenuVisible{false};
std::atomic<bool> g_extendedOriginalSystemMenuOpen{false};
std::atomic<bool> g_extendedOriginalTutorialActive{false};
std::atomic<bool> g_modOriginalTutorialSuppressed{false};
std::atomic<bool> g_extendedForwardKeyDown{false};
std::atomic<bool> g_extendedBackwardKeyDown{false};
std::atomic<bool> g_extendedLeftKeyDown{false};
std::atomic<bool> g_extendedRightKeyDown{false};
std::atomic<bool> g_extendedForwardKeyPulse{false};
std::atomic<bool> g_extendedBackwardKeyPulse{false};
std::atomic<bool> g_extendedLeftKeyPulse{false};
std::atomic<bool> g_extendedRightKeyPulse{false};
std::atomic<int> g_extendedPendingAction{-1};
int g_extendedActionPage = 0;
int g_extendedSelectedAction = 0;
int g_extendedReframeCountdown = 0;
unsigned int g_extendedLegDiagnosticFrame = 0;
bool g_extendedModelDragging = false;
bool g_extendedModelLeftWasDown = false;
float g_extendedModelYaw = 0.0f;
POINT g_extendedModelDragPoint{};
std::atomic<bool> g_extendedManualPointerDown{false};
std::atomic<bool> g_extendedManualPointerConsumed{false};
std::atomic<bool> g_extendedManualPointerInventoryEligible{false};
std::atomic<unsigned int> g_extendedManualPointerPressSerial{0};
std::atomic<unsigned int> g_extendedManualPointerReleaseSerial{0};
std::atomic<unsigned int> g_extendedManualRightPressSerial{0};
std::atomic<LONG> g_extendedManualPointerPressX{0};
std::atomic<LONG> g_extendedManualPointerPressY{0};
std::atomic<LONG> g_extendedManualPointerCurrentX{0};
std::atomic<LONG> g_extendedManualPointerCurrentY{0};
std::atomic<bool> g_extendedManualPointerPositionValid{false};
std::atomic<bool> g_nativeHooksInstalled{false};
std::atomic<bool> g_nativeButtonInjected{false};
Il2CppObject* g_nativeButtonTitleInstance = nullptr;
std::atomic<bool> g_buttonRectReady{false};
std::atomic<bool> g_lobbyUiOpen{false};
std::atomic<int> g_networkMode{0};
void* g_originalTitleUpdate = nullptr;
void* g_originalButtonOnClick = nullptr;
void* g_originalApplicationQuit = nullptr;
void* g_originalInputManagerUpdate = nullptr;
void* g_originalInputManagerUpdateProcess = nullptr;
void* g_originalInputManagerIsDown = nullptr;
void* g_originalInputManagerIsDownFrame = nullptr;
void* g_originalInputManagerIsUpFrame = nullptr;
void* g_originalInputManagerGetInputAction = nullptr;
void* g_originalInputResolverResolve = nullptr;
void* g_originalUnityInputGetKey = nullptr;
void* g_originalUnityInputGetKeyDown = nullptr;
void* g_originalUnityInputGetKeyUp = nullptr;
void* g_originalCursorSetLockState = nullptr;
void* g_originalCursorSetVisible = nullptr;
void* g_originalGetAsyncKeyState = nullptr;
void* g_originalGetKeyState = nullptr;
void* g_originalSetCursorPos = nullptr;
void* g_originalClipCursor = nullptr;
void* g_originalInventoryPanelOpen = nullptr;
void* g_originalPlayerUpdate = nullptr;
void* g_originalPlayerOnFoundStrangerEvent = nullptr;
void* g_originalNpcManagerUpdate = nullptr;
void* g_originalPlayerActionUpdate = nullptr;
void* g_originalPlayerActionChangeAction = nullptr;
void* g_originalPlayerActionOnFrameEnd = nullptr;
void* g_originalPlayerAnimationManagerUpdate = nullptr;
void* g_originalPlayerAnimationTransAction = nullptr;
void* g_originalPlayerAnimationSetMoveSpeed = nullptr;
void* g_originalPlayerAnimationSetGamanBaibu = nullptr;
void* g_originalPlayerAnimationSetEcstasy = nullptr;
void* g_originalPlayerAdultGoodsControllerUpdate = nullptr;
void* g_originalPlayerAnimationCommonStartAnim = nullptr;
void* g_originalPlayerAnimationCommonOnEndAnim = nullptr;
void* g_originalPlayerAnimationChangeVibrator = nullptr;
void* g_originalPlayerAnimationEndEcstasy = nullptr;
void* g_originalPlayerClothesActionUpdate = nullptr;
void* g_originalPlayerClothesGetExposedBodyParts = nullptr;
void* g_originalPlayerHoldInHandHold = nullptr;
void* g_originalPlayerAnimationDropClothes = nullptr;
void* g_originalPlayerAnimationEndPickingCoat = nullptr;
void* g_originalEventMethodManagerUpdate = nullptr;
void* g_originalInGameManagerUpdate = nullptr;
const Il2CppMethod* g_playerControllerUpdateMethod = nullptr;
bool g_extendedPlayerUpdateSeenThisFrame = false;
void* g_originalBaseCameraLateUpdate = nullptr;
void* g_originalTpCameraLateUpdate = nullptr;
void* g_originalInventoryCameraLateUpdate = nullptr;
void* g_originalFpCameraLateUpdate = nullptr;
void* g_originalSaveManagerSave = nullptr;
const Il2CppMethod* g_saveManagerSaveMethod = nullptr;
void* g_originalSystemMenuOpen = nullptr;
void* g_originalSystemMenuResume = nullptr;
void* g_originalSystemMenuSaveAndGoToTitle = nullptr;
void* g_originalTutorialPanelOpen = nullptr;
void* g_originalMissionFailedLoad = nullptr;
void* g_originalMissionFailedGoToTitle = nullptr;
void* g_originalMissionFailedOpen = nullptr;
RECT g_buttonScreenRect{};

void CheckpointSystem_ProcessMapEditorAction(int action);
void CheckpointSystem_CloseMapEditorPanel(bool cancelDraft);
void CheckpointSystem_RequestMapEditorToggle();
bool CheckpointSystem_IsMapEditorModeActive();
bool CheckpointSystem_IsMapEditorUiOpen();
bool CheckpointSystem_IsMapEditorTextInputFocused();
bool CheckpointSystem_ShouldBlockOriginalEyeMaskHotkey();
bool CheckpointSystem_ProcessMapEditorPhysicalKey(
    int virtualKey,
    bool down,
    const wchar_t* source);

void OpenMenu();
void StartHost();
void StartClientDiscovery();
void* MethodPointer(const Il2CppMethod* method);
void LogSkeletonTransforms(Il2CppObject* transform, int depth, int& remaining);
void ApplyEscapeFromManakaStandingPose(Il2CppObject* modelTransform);
void ExtendedMode_CaptureAnimatorState();
void ExtendedMode_ExitToTitle();
void ExtendedMode_SetCustomUiBehindSystemMenu(bool behind);
bool TitleHooks_TryAutoConfirmReturnToTitle();
void GameplayInventory_TickMainThread();
void GameplayInventory_Close();
void GameplayInventory_Reset();
void GameplayWorldMap_SetOpen(bool open);
void ItemContextMenu_Tick();
void ItemContextMenu_Reset();
bool ItemContextMenu_IsOpen();
bool ItemContextMenu_ShouldBlockPointer();
int CountComponentsInChildren(
    Il2CppObject* gameObject,
    const char* namespaceName,
    const char* className);
Il2CppObject* FindChildGameObjectByName(
    Il2CppObject* transform,
    const wchar_t* targetName,
    int depth = 0);
bool EquipmentPreviewUi_HandleButtonClick(Il2CppObject* instance);
bool EquipmentPreviewUi_HandleManualPointer(float cursorX, float cursorY);
bool EquipmentPreviewUi_IsOpen();
void EquipmentPreviewUi_TickDildoAutoCapture();
void EquipmentPreviewUi_TickMainThread();
void EquipmentPreviewUi_Toggle();
void EquipmentPreviewUi_Close();
void EquipmentPreviewUi_Reset();
void ModGuideUi_Tick();
void ModGuideUi_Reset();
void ModGuideUi_RequestClose();
void ModGuideUi_RequestOpenFromTutorial();
bool ModGuideUi_IsOpen();
bool ModGuideUi_ProcessPhysicalKey(
    int virtualKey,
    bool down,
    const wchar_t* source);
bool ModGuideUi_ProcessMouseMessage(
    UINT message,
    LONG screenX,
    LONG screenY);
void EquipmentPreviewUi_ApplyOutfitToGameplayAvatar(
    const char* const* itemIds,
    int itemCount);
SHORT GameplayInput_ReadRawAsyncKeyState(int virtualKey);
void InteractionFilter_ApplySceneWhitelist();
bool ExtendedMode_IsUnityObjectAlive(Il2CppObject* object);
void CheckpointSystem_OnGameplayReady(int stageType);
void CheckpointSystem_Tick();
void DevMapCapture_Tick();
void CheckpointSystem_ClearSceneObjects();
void CheckpointSystem_Reset();
bool CheckpointSystem_TryGetSelectedSpawn(
    float& x,
    float& y,
    float& z);
bool CheckpointSystem_IsTaskUiOpen();
bool CheckpointSystem_IsRewardInventoryOpen();
bool CheckpointSystem_IsCoatToggleLocked();
void CheckpointSystem_ApplyRequiredTaskPose(
    Il2CppObject* inputData);
bool CheckpointSystem_IsFailureUiActive();
void CheckpointSystem_OnGameFailureUiOpened();
bool CheckpointSystem_ShouldSuppressFailurePanelOpen();
void CheckpointSystem_RequestFailureReturn(
    const wchar_t* source);
bool ExtendedMode_RequestFailureReturnToWarehouse();
void CheckpointSystem_RequestRewardInventoryClose();
Il2CppObject* CheckpointSystem_GetRingSprite();
void Runtime_RequestShutdown(const wchar_t* source);
bool CheckpointSystem_HasAcceptedTask();
void CheckpointCompass_Reset();
void CheckpointCompass_Update();
void CheckpointCompass_Hide();
bool DildoInventory_TryUseBoundItem(int itemIndex);
void DildoInventory_Tick();
void DildoInventory_ResetPending();
bool DildoInventory_IsPanelActive();
void DildoInventory_ActivateHoveredChoice();
void EyeMaskVision_ApplyAfterOriginalUpdate();
bool EyeMaskVision_IsEyeMaskActive();
void EyeMaskVision_ForceThirdPersonAfterRemoval();
void EyeMaskVision_ResetCameraTracking();
bool EyeMaskVision_RequestAdjustment(
    int direction,
    const wchar_t* source);
bool EyeMaskVision_ShouldSuppressUnityArrowKey(int keyCode);
void GameplayActionPursuit_RequestFromEMenu(int actionType);
void GameplayActionPursuit_Tick();
void GameplayActionPursuit_EnforceHoldsAfterNpcUpdate();
void GameplayActionPursuit_Reset();
void GameplayActionPursuit_ProbeDependencies();

// --- Feature modules (included in dependency order) ---
#include "modules/logging.inc"
#include "modules/runtime_core.inc"
#include "modules/unity_probe.inc"
#include "modules/title_menu_ui.inc"
#include "modules/scene_ui.inc"
#include "modules/moaning_audio.inc"
#include "modules/character_preview.inc"
#include "modules/inventory_ui.inc"
#include "modules/player_actions.inc"
#include "modules/character_animation.inc"
#include "modules/escape_mode_ui.inc"
#include "modules/extended_mode.inc"
#include "modules/exhibition_ui.inc"
#include "modules/dev_map_capture.inc"
#include "modules/npc_dialogue.inc"
#include "modules/gameplay_status_hud.inc"
#include "modules/gameplay_action_pursuit.inc"
#include "modules/gameplay_inventory.inc"
#include "modules/dildo_inventory.inc"
#include "modules/item_context_menu.inc"
#include "modules/checkpoint_system.inc"
#include "modules/eye_mask_vision.inc"
#include "modules/gameplay_minimap.inc"
#include "modules/mod_guide_ui.inc"
#include "modules/checkpoint_map_editor.inc"
#include "modules/checkpoint_compass.inc"
#include "modules/update_ui.inc"
#include "modules/equipment_preview_ui.inc"
#include "modules/interaction_filter.inc"
#include "modules/title_hooks.inc"
#include "modules/network.inc"
#include "modules/windows_ui.inc"

}

extern "C" __declspec(dllexport) unsigned int ManakaMultiplayerVersion()
{
    return MANAKA_MOD_VERSION_PACKED;
}

extern "C" __declspec(dllexport) const wchar_t* ManakaMultiplayerVersionString()
{
    return MANAKA_MOD_VERSION_STRING;
}

extern "C" __declspec(dllexport) int ManakaTestStartHost()
{
    WSADATA winsockData{};
    const int startup = WSAStartup(MAKEWORD(2, 2), &winsockData);
    if (startup != 0)
    {
        SetStatus(L"WSAStartup failed with code " + std::to_wstring(startup));
        return 0;
    }
    StartHost();
    return 1;
}

extern "C" __declspec(dllexport) int ManakaTestJoin(const wchar_t* host)
{
    if (host == nullptr || host[0] == L'\0')
    {
        return 0;
    }
    WSADATA winsockData{};
    const int startup = WSAStartup(MAKEWORD(2, 2), &winsockData);
    if (startup != 0)
    {
        SetStatus(L"WSAStartup failed with code " + std::to_wstring(startup));
        return 0;
    }
    StartClient(host);
    return 1;
}

extern "C" __declspec(dllexport) int ManakaTestStatus(wchar_t* buffer, int capacity)
{
    if (buffer == nullptr || capacity <= 0)
    {
        return 0;
    }

    const auto status = GetStatus();
    const int count = static_cast<int>(status.copy(buffer, static_cast<size_t>(capacity - 1)));
    buffer[count] = L'\0';
    return count;
}

extern "C" __declspec(dllexport) void ManakaTestStop()
{
    StopNetwork();
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_instance = instance;
        DisableThreadLibraryCalls(instance);

        wchar_t disabled[8]{};
        if (GetEnvironmentVariableW(L"MANAKA_MP_DISABLE_OVERLAY", disabled, 8) > 0 && disabled[0] == L'1')
        {
            return TRUE;
        }

        const auto thread = CreateThread(nullptr, 0, [](LPVOID) -> DWORD
        {
            RuntimeThread();
            return 0;
        }, nullptr, 0, nullptr);
        if (thread != nullptr)
        {
            CloseHandle(thread);
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        // DLL_PROCESS_DETACH runs under the loader lock. Only lock-free atomic
        // stores are safe here; Winsock, CRT, logging and thread cleanup can
        // deadlock the host while it is tearing down.
        g_runtimeShutdownRequested = true;
        g_stopRequested = true;
        g_networkStop = true;
    }
    return TRUE;
}
