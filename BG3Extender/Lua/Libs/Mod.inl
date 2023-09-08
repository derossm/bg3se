﻿/// <lua_module>Mod</lua_module>
BEGIN_NS(lua::mod)

/// <summary>
/// Returns whether the module with the specified GUID is loaded.
/// This is equivalent to Osiris `NRD_IsModLoaded`, but is callable when the Osiris scripting runtime is not yet available (i.e. `ModuleLoading˙, etc events).
/// 
/// Example:
/// ```lua
/// if (Ext.IsModLoaded("5cc23efe-f451-c414-117d-b68fbc53d32d")) then
///     Ext.Print("Mod loaded")
/// end
/// ```
/// </summary>
/// <param name="modNameGuid">UUID of mod to check</param>
bool IsModLoaded(char const* modNameGuid)
{
	auto modUuid = Guid::Parse(modNameGuid);
	if (modUuid) {
		auto modManager = gExtender->GetCurrentExtensionState()->GetModManager();
		for (auto const& mod : modManager->BaseModule.LoadOrderedModules) {
			if (mod.Info.ModuleUUID == *modUuid) {
				return true;
			}
		}
	}

	return false;
}

/// <summary>
/// Returns the list of loaded module UUIDs in the order they're loaded in.
/// </summary>
/// <returns></returns>
ObjectSet<Guid> GetLoadOrder()
{
	ObjectSet<Guid> loadOrder;
	auto modManager = gExtender->GetCurrentExtensionState()->GetModManager();

	for (auto const& mod : modManager->BaseModule.LoadOrderedModules) {
		loadOrder.Add(mod.Info.ModuleUUID);
	}

	return loadOrder;
}

/// <summary>
/// Returns detailed information about the specified (loaded) module.
/// </summary>
/// <param name="modNameGuid">Mod UUID to query</param>
Module* GetMod(char const* modNameGuid)
{
	Module const * module{ nullptr };
	auto modUuid = Guid::Parse(modNameGuid);
	if (modUuid) {
		auto modManager = gExtender->GetCurrentExtensionState()->GetModManager();
		for (auto& mod : modManager->BaseModule.LoadOrderedModules) {
			if (mod.Info.ModuleUUID == *modUuid) {
				return &mod;
			}
		}
	}

	return nullptr;
}

Module* GetBaseMod()
{
	return &gExtender->GetCurrentExtensionState()->GetModManager()->BaseModule;
}

ModManager* GetModManager()
{
	return gExtender->GetCurrentExtensionState()->GetModManager();
}

void RegisterModLib()
{
	DECLARE_MODULE(Mod, Both)
	BEGIN_MODULE()
	MODULE_FUNCTION(IsModLoaded)
	MODULE_FUNCTION(GetLoadOrder)
	MODULE_FUNCTION(GetMod)
	MODULE_FUNCTION(GetBaseMod)
	MODULE_FUNCTION(GetModManager)
	END_MODULE()
}

END_NS()
