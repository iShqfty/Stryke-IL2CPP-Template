#include "../Include/includes.h"

#include "../Utils/Hooks/Hooks.h"

extern DWORD WINAPI RenderThread(LPVOID lpReserved);


DWORD WINAPI main(LPVOID lpReserved) {
	MH_Initialize();
	RenderThread(lpReserved);
	//Additional Init Calls Below
	hook.AllHooks(true);
	mmgr.initMods();
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, main, hMod, 0, nullptr);
		hmod = hMod;
		lpRsv = lpReserved;
		break;
	case DLL_PROCESS_DETACH:
		mmgr.deInitMods();
		hook.AllHooks(false);
		kiero::shutdown();
		break;
	}
	return TRUE;
}
