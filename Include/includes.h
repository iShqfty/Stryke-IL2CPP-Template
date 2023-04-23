#pragma once
#include "../Utils/Utils.h"
#include "../Utils/Hooks/Hooks.h"
#include "../Stryke/Draw/imgui.h"
#include "../Stryke/Draw/imgui_impl_win32.h"
#include "../Stryke/Draw/imgui_impl_dx11.h"
#include "../Stryke/Module/ModManager.h"
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>


typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;


inline HMODULE hmod;
inline LPVOID lpRsv;