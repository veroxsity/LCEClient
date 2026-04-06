// Linux64_Stubs.cpp
// Stub implementations for symbols that have no Linux equivalent yet.
// These allow the binary to link. Most are no-ops or return safe defaults.
// As real implementations are written (Phase 2-6), stubs get removed.

#include "stdafx.h"
#include "Minecraft.World/FloatBuffer.h"
#include "Minecraft.World/IntBuffer.h"
#include "Minecraft.World/ByteBuffer.h"

// ============================================================
// Category 1: C4JRender / RenderManager / PostProcesser
// Phase 3 will replace these with a real OpenGL backend.
// ============================================================

C4JRender RenderManager;

void C4JRender::Tick() {}
void C4JRender::UpdateGamma(unsigned short) {}
void C4JRender::InitialiseContext() {}
void C4JRender::StartFrame() {}
void C4JRender::Present() {}
void C4JRender::Clear(int, D3D11_RECT*) {}
void C4JRender::Set_matrixDirty() {}
bool C4JRender::IsWidescreen() { return true; }
bool C4JRender::IsHiDef() { return true; }

void C4JRender::DrawVertices(C4JRender::ePrimitiveType, int, void*, C4JRender::eVertexType, C4JRender::ePixelShaderType) {}
void C4JRender::StateSetBlendEnable(bool) {}
void C4JRender::StateSetBlendFunc(int, int) {}
void C4JRender::StateSetBlendFactor(unsigned int) {}
void C4JRender::StateSetEnableViewportClipPlanes(bool) {}
void C4JRender::StateSetForceLOD(int) {}
void C4JRender::StateSetViewport(C4JRender::eViewportType) {}
void C4JRender::TextureBind(int) {}
void C4JRender::TextureBindVertex(int) {}
void C4JRender::TextureData(int, int, void*, int, C4JRender::eTextureFormat) {}
void C4JRender::TextureDataUpdate(int, int, int, int, void*, int) {}
void C4JRender::TextureSetTextureLevels(int) {}
int  C4JRender::TextureGetTextureLevels() { return 1; }
ID3D11ShaderResourceView* C4JRender::TextureGetTexture(int) { return nullptr; }
HRESULT C4JRender::LoadTextureData(const char*, D3DXIMAGE_INFO*, int**) { return E_FAIL; }
HRESULT C4JRender::LoadTextureData(BYTE*, DWORD, D3DXIMAGE_INFO*, int**) { return E_FAIL; }

// Command buffers
void C4JRender::CBuffLockStaticCreations() {}
bool C4JRender::CBuffCall(int, bool) { return false; }
void C4JRender::CBuffClear(int) {}
int  C4JRender::CBuffSize(int) { return 0; }
void C4JRender::CBuffStart(int, bool) {}
void C4JRender::CBuffEnd() {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd() {}

// PostProcesser
#include "PostProcesser.h"
PostProcesser::PostProcesser() {}
PostProcesser::~PostProcesser() {}
void PostProcesser::Apply() const {}
void PostProcesser::ApplyFromCopied() const {}
void PostProcesser::CopyBackbuffer() {}
void PostProcesser::ResetViewport() {}
void PostProcesser::SetGamma(float) {}
void PostProcesser::SetViewport(const D3D11_VIEWPORT&) {}

// ============================================================
// Category 2: C_4JProfile / ProfileManager
// ============================================================

C_4JProfile ProfileManager;

void C_4JProfile::Initialise(unsigned int, unsigned int, unsigned short, unsigned int, unsigned int, unsigned int*, int, unsigned int*) {}
void C_4JProfile::Tick() {}
bool C_4JProfile::IsSignedIn(int) { return true; }
bool C_4JProfile::IsSignedInLive(int) { return false; }
bool C_4JProfile::IsGuest(int) { return false; }
bool C_4JProfile::IsFullVersion() { return true; }
bool C_4JProfile::IsSystemUIDisplayed() { return false; }
int  C_4JProfile::GetPrimaryPad() { return 0; }
void C_4JProfile::SetPrimaryPad(int) {}
int  C_4JProfile::GetLockedProfile() { return 0; }
void C_4JProfile::SetLockedProfile(int) {}

std::wstring C_4JProfile::GetDisplayName(int) { return L"Player"; }
const char* C_4JProfile::GetGamertag(int) { return "Player"; }
void C_4JProfile::GetXUID(int, unsigned long long*, bool) {}
bool C_4JProfile::AreXUIDSEqual(unsigned long long, unsigned long long) { return false; }
int  C_4JProfile::GetLiveConnectionStatus() { return 0; }
void C_4JProfile::QuerySigninStatus() {}
bool C_4JProfile::AllowedToPlayMultiplayer(int) { return true; }
void C_4JProfile::AllowedPlayerCreatedContent(int, bool, int*, int*) {}
void C_4JProfile::Award(int, int, bool) {}
bool C_4JProfile::CanBeAwarded(int, int) { return false; }
int  C_4JProfile::GetAwardType(int) { return 0; }
void C_4JProfile::DisplayFullVersionPurchase(bool, int, int) {}
void C_4JProfile::ForceQueuedProfileWrites(int) {}
C_4JProfile::PROFILESETTINGS* C_4JProfile::GetGameDefinedProfileData(int) { return nullptr; }
void C_4JProfile::ResetProfileProcessState() {}
void C_4JProfile::RequestSignInUI(bool, bool, bool, bool, bool, int (*)(void*, bool, int), void*, int) {}
void C_4JProfile::RequestConvertOfflineToGuestUI(int (*)(void*, bool, int), void*, int) {}
void C_4JProfile::SetCurrentGameActivity(int, int, bool) {}
void C_4JProfile::SetDebugFullOverride(bool) {}
void C_4JProfile::SetDefaultOptionsCallback(int (*)(void*, C_4JProfile::PROFILESETTINGS*, int), void*) {}
void C_4JProfile::SetRichPresenceContextValue(int, int, int) {}
void C_4JProfile::WriteToProfile(int, bool, bool) {}

// ============================================================
// Category 3: C_4JInput / InputManager
// ============================================================

C_4JInput InputManager;

void C_4JInput::Initialise(int, unsigned char, unsigned char, unsigned char) {}
void C_4JInput::Tick() {}
bool C_4JInput::IsPadConnected(int) { return false; }
bool C_4JInput::ButtonDown(int, unsigned char) { return false; }
bool C_4JInput::ButtonPressed(int, unsigned char) { return false; }
bool C_4JInput::ButtonReleased(int, unsigned char) { return false; }
float C_4JInput::GetJoypadStick_LX(int, bool) { return 0.0f; }
float C_4JInput::GetJoypadStick_LY(int, bool) { return 0.0f; }
float C_4JInput::GetJoypadStick_RX(int, bool) { return 0.0f; }
float C_4JInput::GetJoypadStick_RY(int, bool) { return 0.0f; }
float C_4JInput::GetValue(int, unsigned char, bool) { return 0.0f; }
float C_4JInput::GetIdleSeconds(int) { return 0.0f; }
unsigned char C_4JInput::GetGameJoypadMaps(unsigned char, unsigned char) { return 0; }
int  C_4JInput::GetJoypadMapVal(int) { return 0; }
void C_4JInput::SetJoypadMapVal(int, unsigned char) {}
void C_4JInput::SetJoypadStickAxisMap(int, unsigned int, unsigned int) {}
void C_4JInput::SetJoypadStickTriggerMap(int, unsigned int, unsigned int) {}
void C_4JInput::SetKeyRepeatRate(float, float) {}
void C_4JInput::SetMenuDisplayed(int, bool) {}
void C_4JInput::SetDebugSequence(const char*, int (*)(void*), void*) {}

// ============================================================
// Category 4: C4JStorage / StorageManager
// ============================================================

C4JStorage StorageManager;

void C4JStorage::Tick() {}
void C4JStorage::AllocateSaveData(unsigned int) {}
void C4JStorage::ClearDLCOffers() {}
bool C4JStorage::DoesSaveExist(bool* p) { if(p) *p = false; return false; }
void C4JStorage::GetDLCOffers(int, int (*)(void*, int, unsigned int, int), void*, unsigned int) {}
void C4JStorage::GetInstalledDLC(int, int (*)(void*, int, int), void*) {}
void C4JStorage::GetMountedDLCFileList(const char*, std::vector<std::string>&) {}
std::string C4JStorage::GetMountedPath(std::string) { return ""; }
bool C4JStorage::GetSaveData(void*, unsigned int*) { return false; }
bool C4JStorage::GetSaveDisabled() { return false; }
unsigned int C4JStorage::GetSaveSize() { return 0; }
void C4JStorage::GetSaveUniqueFilename(char*) {}
void C4JStorage::GetSaveUniqueNumber(int*) {}
void C4JStorage::MountInstalledDLC(int, unsigned int, int (*)(void*, int, unsigned int, unsigned int), void*, const char*) {}
void C4JStorage::SetSaveDeviceSelected(unsigned int, bool) {}
void C4JStorage::SetSaveDisabled(bool) {}
void C4JStorage::UnmountInstalledDLC(const char*) {}

// ============================================================
// Category 5: Iggy (Flash UI) — closed-source binary, stub to no-ops
// ============================================================

extern "C" {
void IggyDebugGetMemoryUseInfo() {}
void IggyInit() {}
void IggyLibraryCreateFromMemoryUTF16() {}
void IggyLibraryDestroy() {}
void IggyMakeEventKey() {}
void IggyPlayerCallMethodRS() {}
void IggyPlayerCreateFastName() {}
void IggyPlayerCreateFromMemory() {}
void IggyPlayerDestroy() {}
void IggyPlayerDispatchEventRS() {}
void IggyPlayerDraw() {}
void IggyPlayerGetUserdata() {}
void IggyPlayerInitializeAndTickRS() {}
void IggyPlayerProperties() {}
void IggyPlayerReadyToTick() {}
void IggyPlayerRootPath() {}
void IggyPlayerSetDisplaySize() {}
void IggyPlayerSetUserdata() {}
void IggyPlayerTickRS() {}
void IggySetAS3ExternalFunctionCallbackUTF16() {}
void IggySetCustomDrawCallback() {}
void IggySetFontCachingCalculationBuffer() {}
void IggySetTextureSubstitutionCallbacks() {}
void IggySetTraceCallbackUTF8() {}
void IggySetWarningCallback() {}
}

// ============================================================
// Category 6: UIScene / UIComponent constructors
// These are Flash/Iggy-based UI scenes. Stubbed until UI rewrite.
// ============================================================

#include "Common/UI/UI.h"

// UIScene constructors
UIScene_AnvilMenu::UIScene_AnvilMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_BeaconMenu::UIScene_BeaconMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_BrewingStandMenu::UIScene_BrewingStandMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_ConnectingProgress::UIScene_ConnectingProgress(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_ContainerMenu::UIScene_ContainerMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_ControlsMenu::UIScene_ControlsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_CraftingMenu::UIScene_CraftingMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_CreateWorldMenu::UIScene_CreateWorldMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_CreativeMenu::UIScene_CreativeMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_Credits::UIScene_Credits(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DeathMenu::UIScene_DeathMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DebugCreateSchematic::UIScene_DebugCreateSchematic(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DebugOptionsMenu::UIScene_DebugOptionsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DebugOverlay::UIScene_DebugOverlay(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DebugSetCamera::UIScene_DebugSetCamera(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DispenserMenu::UIScene_DispenserMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DLCMainMenu::UIScene_DLCMainMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_DLCOffersMenu::UIScene_DLCOffersMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_EnchantingMenu::UIScene_EnchantingMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_EndPoem::UIScene_EndPoem(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_EULA::UIScene_EULA(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_FireworksMenu::UIScene_FireworksMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_FullscreenProgress::UIScene_FullscreenProgress(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
void UIScene_FullscreenProgress::SetWasCancelled(bool) {}
UIScene_FurnaceMenu::UIScene_FurnaceMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_HelpAndOptionsMenu::UIScene_HelpAndOptionsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_HopperMenu::UIScene_HopperMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_HorseInventoryMenu::UIScene_HorseInventoryMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_HowToPlayMenu::UIScene_HowToPlayMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_HowToPlay::UIScene_HowToPlay(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_HUD::UIScene_HUD(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
void UIScene_HUD::handleGameTick() {}
UIScene_InGameHostOptionsMenu::UIScene_InGameHostOptionsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_InGameInfoMenu::UIScene_InGameInfoMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_InGamePlayerOptionsMenu::UIScene_InGamePlayerOptionsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_Intro::UIScene_Intro(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_InventoryMenu::UIScene_InventoryMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_JoinMenu::UIScene_JoinMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_Keyboard::UIScene_Keyboard(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_LanguageSelector::UIScene_LanguageSelector(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_LaunchMoreOptionsMenu::UIScene_LaunchMoreOptionsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_LeaderboardsMenu::UIScene_LeaderboardsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_LoadMenu::UIScene_LoadMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_LoadOrJoinMenu::UIScene_LoadOrJoinMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_MainMenu::UIScene_MainMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_MessageBox::UIScene_MessageBox(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_NewUpdateMessage::UIScene_NewUpdateMessage(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_PauseMenu::UIScene_PauseMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_QuadrantSignin::UIScene_QuadrantSignin(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_ReinstallMenu::UIScene_ReinstallMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SaveMessage::UIScene_SaveMessage(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SettingsAudioMenu::UIScene_SettingsAudioMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SettingsControlMenu::UIScene_SettingsControlMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SettingsGraphicsMenu::UIScene_SettingsGraphicsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
int UIScene_SettingsGraphicsMenu::DistanceToLevel(int) { return 0; }
int UIScene_SettingsGraphicsMenu::LevelToDistance(int) { return 0; }
UIScene_SettingsMenu::UIScene_SettingsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SettingsOptionsMenu::UIScene_SettingsOptionsMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SettingsUIMenu::UIScene_SettingsUIMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SignEntryMenu::UIScene_SignEntryMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_SkinSelectMenu::UIScene_SkinSelectMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_TeleportMenu::UIScene_TeleportMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_Timer::UIScene_Timer(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_TradingMenu::UIScene_TradingMenu(int a, void* b, UILayer* c) : UIScene(a, b, c) {}
UIScene_TrialExitUpsell::UIScene_TrialExitUpsell(int a, void* b, UILayer* c) : UIScene(a, b, c) {}

// UIComponent constructors
UIComponent_Chat::UIComponent_Chat(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
UIComponent_DebugUIConsole::UIComponent_DebugUIConsole(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
void UIComponent_DebugUIConsole::addText(const std::string&) {}
UIComponent_DebugUIMarketingGuide::UIComponent_DebugUIMarketingGuide(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
UIComponent_Logo::UIComponent_Logo(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
UIComponent_MenuBackground::UIComponent_MenuBackground(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
UIComponent_Panorama::UIComponent_Panorama(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
UIComponent_PressStartToPlay::UIComponent_PressStartToPlay(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
void UIComponent_PressStartToPlay::setTrialTimer(const std::wstring&) {}
void UIComponent_PressStartToPlay::showPlayerDisplayName(bool) {}
void UIComponent_PressStartToPlay::showPressStart(int, bool) {}
void UIComponent_PressStartToPlay::showSaveIcon(bool) {}
void UIComponent_PressStartToPlay::showTrialTimer(bool) {}
UIComponent_Tooltips::UIComponent_Tooltips(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
void UIComponent_Tooltips::overrideSFX(int, int, bool) {}
UIComponent_TutorialPopup::UIComponent_TutorialPopup(int a, void* b, UILayer* c) : UIComponent(a, b, c) {}
bool UIComponent_TutorialPopup::IsVisible() { return false; }
void UIComponent_TutorialPopup::RemoveInteractSceneReference(UIScene*) {}
void UIComponent_TutorialPopup::SetTutorialDescription(_TutorialPopupInfo*) {}
void UIComponent_TutorialPopup::SetVisible(bool) {}
void UIComponent_TutorialPopup::UpdateTutorialPopup() {}

// ============================================================
// Category 7: GL wrapper functions (custom buffer-type overloads)
// These are declared in stubs.h for Linux but need implementations.
// Phase 3 will provide real OpenGL implementations.
// ============================================================

void glCallLists(IntBuffer*) {}
void glFog(int, FloatBuffer*) {}
void glGetFloat(int, FloatBuffer*) {}
void glLight(int, int, FloatBuffer*) {}
void glLightModel(int, FloatBuffer*) {}
void glTexGen(int, int, FloatBuffer*) {}
void gluPerspective(float, float, float, float) {}

// ============================================================
// Category 8: Display
// ============================================================

void Display::update() {}
void Display::swapBuffers() {}

// ============================================================
// Category 9: Static constant out-of-line definitions
// C++17 inline statics should not need these, but the linker
// requires them when the address is ODR-used. No values here —
// the initializers are in the headers.
// ============================================================

// AddEntityPacket
constexpr int AddEntityPacket::ARROW;
constexpr int AddEntityPacket::BOAT;
constexpr int AddEntityPacket::EGG;
constexpr int AddEntityPacket::ENDER_CRYSTAL;
constexpr int AddEntityPacket::EYEOFENDERSIGNAL;
constexpr int AddEntityPacket::FALLING;
constexpr int AddEntityPacket::FIREWORKS;
constexpr int AddEntityPacket::FISH_HOOK;
constexpr int AddEntityPacket::ITEM;
constexpr int AddEntityPacket::ITEM_FRAME;
constexpr int AddEntityPacket::LEASH_KNOT;
constexpr int AddEntityPacket::MINECART;
constexpr int AddEntityPacket::PRIMED_TNT;
constexpr int AddEntityPacket::SNOWBALL;
constexpr int AddEntityPacket::THROWN_ENDERPEARL;
constexpr int AddEntityPacket::THROWN_EXPBOTTLE;
constexpr int AddEntityPacket::THROWN_POTION;

// AnimatePacket
constexpr int AnimatePacket::CRITICAL_HIT;
constexpr int AnimatePacket::EAT;
constexpr int AnimatePacket::MAGIC_CRITICAL_HIT;
constexpr int AnimatePacket::SWING;
constexpr int AnimatePacket::WAKE_UP;

// ClientCommandPacket
constexpr int ClientCommandPacket::PERFORM_RESPAWN;

// ContainerOpenPacket
constexpr int ContainerOpenPacket::BEACON;
constexpr int ContainerOpenPacket::BREWING_STAND;
constexpr int ContainerOpenPacket::DROPPER;
constexpr int ContainerOpenPacket::ENCHANTMENT;
constexpr int ContainerOpenPacket::FIREWORKS;
constexpr int ContainerOpenPacket::FURNACE;
constexpr int ContainerOpenPacket::HOPPER;
constexpr int ContainerOpenPacket::HORSE;
constexpr int ContainerOpenPacket::REPAIR_TABLE;
constexpr int ContainerOpenPacket::TRADER_NPC;
constexpr int ContainerOpenPacket::TRAP;
constexpr int ContainerOpenPacket::WORKBENCH;

// EntityEvent
constexpr int EntityEvent::USE_ITEM_COMPLETE;

// GameEventPacket
constexpr int GameEventPacket::SUCCESSFUL_BOW_HIT;

// SetEntityLinkPacket
constexpr int SetEntityLinkPacket::LEASH;
constexpr int SetEntityLinkPacket::RIDING;

// TileEditorOpenPacket
constexpr int TileEditorOpenPacket::SIGN;

// TileEntityDataPacket
constexpr int TileEntityDataPacket::TYPE_ADV_COMMAND;
constexpr int TileEntityDataPacket::TYPE_BEACON;
constexpr int TileEntityDataPacket::TYPE_MOB_SPAWNER;
constexpr int TileEntityDataPacket::TYPE_SKULL;

// SharedConstants
constexpr int SharedConstants::NETWORK_PROTOCOL_VERSION;

// RemoveEntitiesPacket
constexpr int RemoveEntitiesPacket::MAX_PER_PACKET;

// ChestTile
constexpr int ChestTile::TYPE_TRAP;

// QuartzBlockTile
constexpr int QuartzBlockTile::TYPE_LINES_Y;

// SkullTileEntity
constexpr int SkullTileEntity::TYPE_WITHER;

// Sapling
constexpr int Sapling::TYPE_BIRCH;
constexpr int Sapling::TYPE_DEFAULT;
constexpr int Sapling::TYPE_EVERGREEN;
constexpr int Sapling::TYPE_JUNGLE;

// TallGrass
constexpr int TallGrass::FERN;

// Tile static IDs
constexpr int Tile::bookshelf_Id;
constexpr int Tile::glass_Id;
constexpr int Tile::glowstone_Id;
constexpr int Tile::leaves_Id;
constexpr int Tile::stoneSlabHalf_Id;
constexpr int Tile::wool_Id;

// Level
constexpr int Level::maxBuildHeight;

// Item static IDs
constexpr int Item::apple_Id;
constexpr int Item::arrow_Id;
constexpr int Item::beef_cooked_Id;
constexpr int Item::beef_raw_Id;
constexpr int Item::book_Id;
constexpr int Item::boots_chain_Id;
constexpr int Item::boots_diamond_Id;
constexpr int Item::boots_iron_Id;
constexpr int Item::boots_leather_Id;
constexpr int Item::bread_Id;
constexpr int Item::bucket_lava_Id;
constexpr int Item::bucket_water_Id;
constexpr int Item::chestplate_chain_Id;
constexpr int Item::chestplate_diamond_Id;
constexpr int Item::chestplate_iron_Id;
constexpr int Item::chestplate_leather_Id;
constexpr int Item::chicken_cooked_Id;
constexpr int Item::chicken_raw_Id;
constexpr int Item::clock_Id;
constexpr int Item::coal_Id;
constexpr int Item::compass_Id;
constexpr int Item::cookie_Id;
constexpr int Item::diamond_Id;
constexpr int Item::emerald_Id;
constexpr int Item::enderPearl_Id;
constexpr int Item::expBottle_Id;
constexpr int Item::eyeOfEnder_Id;
constexpr int Item::fish_cooked_Id;
constexpr int Item::flintAndSteel_Id;
constexpr int Item::flint_Id;
constexpr int Item::goldIngot_Id;
constexpr int Item::hatchet_diamond_Id;
constexpr int Item::hatchet_iron_Id;
constexpr int Item::helmet_chain_Id;
constexpr int Item::helmet_diamond_Id;
constexpr int Item::helmet_iron_Id;
constexpr int Item::helmet_leather_Id;
constexpr int Item::hoe_diamond_Id;
constexpr int Item::hoe_iron_Id;
constexpr int Item::ironIngot_Id;
constexpr int Item::leggings_chain_Id;
constexpr int Item::leggings_diamond_Id;
constexpr int Item::leggings_iron_Id;
constexpr int Item::leggings_leather_Id;
constexpr int Item::map_Id;
constexpr int Item::melon_Id;
constexpr int Item::paper_Id;
constexpr int Item::pickAxe_diamond_Id;
constexpr int Item::pickAxe_iron_Id;
constexpr int Item::porkChop_cooked_Id;
constexpr int Item::porkChop_raw_Id;
constexpr int Item::potion_Id;
constexpr int Item::redStone_Id;
constexpr int Item::rotten_flesh_Id;
constexpr int Item::saddle_Id;
constexpr int Item::seeds_melon_Id;
constexpr int Item::seeds_pumpkin_Id;
constexpr int Item::seeds_wheat_Id;
constexpr int Item::shears_Id;
constexpr int Item::shovel_diamond_Id;
constexpr int Item::shovel_iron_Id;
constexpr int Item::skull_Id;
constexpr int Item::sword_diamond_Id;
constexpr int Item::sword_iron_Id;
constexpr int Item::wheat_Id;

// ============================================================
// Category 10: Misc symbols
// ============================================================

// SoundEngine
char SoundEngine::m_szMusicPath[512] = {};

// LeaderboardManager
LeaderboardManager* LeaderboardManager::m_instance = nullptr;

// DefineActions — input action map setup, stub for now
void DefineActions() {}

// MemSect — memory tracking
int MemSect(int) { return 0; }

// ShutdownManager — no Linux header exists, provide stub implementations
// The class is likely forward-declared somewhere but never included for Linux.
#include "Orbis/OrbisExtras/ShutdownManager.h"

// NetworkPlayerXbox — wraps IQNetPlayer for Xbox, needed by GameNetworkManager
#include "Xbox/Network/NetworkPlayerXbox.h"
NetworkPlayerXbox::NetworkPlayerXbox(IQNetPlayer* p) : m_qnetPlayer(p), m_pSocket(nullptr), m_lastChunkPacketTime(0) {}
IQNetPlayer* NetworkPlayerXbox::GetQNetPlayer() { return m_qnetPlayer; }

// IQNet_DispatchPacket
void IQNet_DispatchPacket(unsigned char, unsigned char, unsigned char*, unsigned int) {}
