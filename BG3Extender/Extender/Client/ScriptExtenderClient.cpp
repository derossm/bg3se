#include <stdafx.h>
#include <Extender/Client/ScriptExtenderClient.h>
#include <Extender/ScriptExtender.h>

#define STATIC_HOOK(name) decltype(bg3se::ecl::ScriptExtender::name) * decltype(bg3se::ecl::ScriptExtender::name)::gHook;
STATIC_HOOK(gameStateWorkerStart_)
STATIC_HOOK(gameStateChangedEvent_)
STATIC_HOOK(gameStateMachineUpdate_)

#include <Extender/Shared/ThreadedExtenderState.inl>
#include <Extender/Shared/ModuleHasher.inl>

BEGIN_SE()

void InitCrashReporting();

END_SE()

BEGIN_NS(ecl)

char const * GameStateNames[] =
{
	"Unknown",
	"Init",
	"InitMenu",
	"InitNetwork",
	"InitConnection",
	"Idle",
	"LoadMenu",
	"Menu",
	"Exit",
	"SwapLevel",
	"LoadLevel",
	"LoadModule",
	"LoadSession",
	"UnloadLevel",
	"UnloadModule",
	"UnloadSession",
	"Paused",
	"PrepareRunning",
	"Running",
	"Disconnect",
	"Join",
	"Save",
	"StartLoading",
	"StopLoading",
	"StartServer",
	"Movie",
	"Installation",
	"ModReceiving",
	"Lobby",
	"BuildStory",
	"UNKNOWN_30",
	"UNKNOWN_31",
	"AnalyticsSessionEnd"
};

ScriptExtender::ScriptExtender()
{
}

void ScriptExtender::Initialize()
{
	ResetExtensionState();

	// Wrap state change functions even if extension startup failed, otherwise
	// we won't be able to show any startup errors

	auto& lib = GetStaticSymbols();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (lib.ecl__GameStateEventManager__ExecuteGameStateChangedEvent != nullptr) {
		gameStateChangedEvent_.Wrap(lib.ecl__GameStateEventManager__ExecuteGameStateChangedEvent);
	}

	if (lib.ecl__GameStateThreaded__GameStateWorker__DoWork != nullptr) {
		gameStateWorkerStart_.Wrap(lib.ecl__GameStateThreaded__GameStateWorker__DoWork);
	}

	if (lib.ecl__GameStateMachine__Update != nullptr) {
		gameStateMachineUpdate_.Wrap(lib.ecl__GameStateMachine__Update);
	}

	DetourTransactionCommit();

	gameStateChangedEvent_.SetPostHook(&ScriptExtender::OnGameStateChanged, this);
	gameStateWorkerStart_.SetWrapper(&ScriptExtender::GameStateWorkerWrapper, this);
	gameStateMachineUpdate_.SetPostHook(&ScriptExtender::OnUpdate, this);
}

void ScriptExtender::Shutdown()
{
	DEBUG("ecl::ScriptExtender::Shutdown: Exiting");
	ResetExtensionState();
}

void ScriptExtender::PostStartup()
{
	entityHelpers_.Setup();
}

void ScriptExtender::OnGameStateChanged(void * self, GameState fromState, GameState toState)
{
	if (self != *GetStaticSymbols().ecl__gGameStateEventManager) {
		gExtender->GetServer().OnGameStateChanged(self, (esv::GameState)fromState, (esv::GameState)toState);
		return;
	}

	if (gExtender->GetConfig().SendCrashReports) {
		// We need to initialize the crash reporter after the game engine has started,
		// otherwise the game will overwrite the top level exception filter
		InitCrashReporting();
	}

	// Check to make sure that startup is done even if the extender was loaded when the game was already in GameState::Init
	if (toState != GameState::Unknown
		&& toState != GameState::StartLoading
		&& toState != GameState::InitMenu
		&& !gExtender->GetLibraryManager().CriticalInitializationFailed()) {
		gExtender->PostStartup();
	}

#if defined(DEBUG_SERVER_CLIENT)
	DEBUG("ecl::ScriptExtender::OnGameStateChanged(): %s -> %s", 
		GameStateNames[(unsigned)fromState], ClientGameStateNames[(unsigned)toState]);
#endif

	if (fromState != GameState::Unknown) {
		AddThread(GetCurrentThreadId());
	}

	switch (fromState) {
	case GameState::LoadModule:
		INFO("ecl::ScriptExtender::OnGameStateChanged(): Loaded module");
		LoadExtensionState();
		break;

	case GameState::LoadSession:
		if (extensionState_) {
			extensionState_->OnGameSessionLoaded();
		}
		break;

	case GameState::InitConnection:
		//networkManager_.ExtendNetworkingClient();
		break;
	}

	switch (toState) {
	case GameState::InitNetwork:
	case GameState::Disconnect:
		//networkManager_.ClientReset();
		break;

	case GameState::UnloadModule:
		hasher_.ClearCaches();
		break;

	case GameState::UnloadSession:
		INFO("ecl::ScriptExtender::OnGameStateChanged(): Unloading session");
		ResetExtensionState();
		break;

	case GameState::LoadModule:
		if (gExtender->GetConfig().DisableModValidation) {
			auto globals = GetStaticSymbols().GlobalSwitches;
			if (globals && *globals) {
				(*globals)->EnableHashing = false;
				INFO("Disabled mod validation");
			} else {
				ERR("Could not disable mod validation - GlobalSwitches not available!");
			}
		}
		break;

	case GameState::LoadSession:
		INFO("ecl::ScriptExtender::OnClientGameStateChanged(): Loading game session");
		LoadExtensionState();
		//networkManager_.ExtendNetworkingClient();
		if (extensionState_) {
			extensionState_->OnGameSessionLoading();
		}
		break;
	}

	LuaClientPin lua(ExtensionState::Get());
	if (lua) {
		lua->OnGameStateChanged(fromState, toState);
	}
}

void ScriptExtender::GameStateWorkerWrapper(void (*wrapped)(void*), void* self)
{
	AddThread(GetCurrentThreadId());
	wrapped(self);
	RemoveThread(GetCurrentThreadId());
}

void ScriptExtender::OnUpdate(void* self, GameTime* time)
{
	RunPendingTasks();
}

bool ScriptExtender::IsInClientThread() const
{
	return IsInThread();
}

void ScriptExtender::ResetLuaState()
{
	auto server = GetEoCServer();
	if (server && server->GameServer && false /* networking not available yet! */) {
		// Reset clients via a network message if the server is running
		/*auto& networkMgr = gExtender->GetNetworkManager();
		auto msg = networkMgr.GetFreeServerMessage(ReservedUserId);
		if (msg != nullptr) {
			auto resetMsg = msg->GetMessage().mutable_s2c_reset_lua();
			resetMsg->set_bootstrap_scripts(true);
			networkMgr.ServerBroadcast(msg, ReservedUserId);
		} else {
			OsiErrorS("Could not get free message!");
		}*/

	} else if (extensionState_ && extensionState_->GetLua()) {
		bool serverThread = !IsInClientThread();

		// Do a direct (local) reset if server is not available (main menu, etc.)
		auto ext = extensionState_.get();

		ext->AddPostResetCallback([ext]() {
			ext->OnModuleResume();
			auto state = GetStaticSymbols().GetClientState();
			if (state && (state == GameState::Paused || state == GameState::Running)) {
				ext->OnGameSessionLoading();
				ext->OnGameSessionLoaded();
				ext->OnResetCompleted();
			}
		});
		ext->LuaReset(true);
	}
}

void ScriptExtender::ResetExtensionState()
{
	extensionState_ = std::make_unique<ExtensionState>();
	extensionState_->Reset();
	extensionLoaded_ = false;
}

void ScriptExtender::LoadExtensionState()
{
	if (extensionLoaded_) return;

	PostStartup();

	if (!extensionState_) {
		ResetExtensionState();
	}

	//extensionState_->LoadConfigs();

	if (!gExtender->GetLibraryManager().CriticalInitializationFailed()) {
		//networkManager_.ExtendNetworkingClient();
		DEBUG("ecl::ScriptExtender::LoadExtensionStateClient(): Re-initializing module state.");
		extensionState_->LuaReset(true);
	}

	extensionLoaded_ = true;
}

END_NS()
