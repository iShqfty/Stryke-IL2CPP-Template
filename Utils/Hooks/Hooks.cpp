#pragma once

#include "Hooks.h"

//inline void(__fastcall* ExampleHook)(CExample* __this);
//inline void __stdcall HookExampleHook(CExample* __this) {
//	hook.example = __this;
//	return ExampleHook(__this);
//}

//Example Hook Template Above (For Getting Player)

void Hooks::AllHooks(bool enable) {
	if (enable) {
		//For Each Hook Template Defined Above, Create and Hook below.

		//MH_CreateHook(this->EXAMPLE_EXAMPLEHOOK_ADDR, &HookExampleHook, (LPVOID*)&ExampleHook);
		//MH_EnableHook(this->EXAMPLE_EXAMPLEHOOK_ADDR);

		return;
	}
	//If !Enable, Disable All Created Hooks.

	MH_DisableHook(MH_ALL_HOOKS);
}