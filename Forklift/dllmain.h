#pragma once

// BEGIN FORKLIFT CONFIGURATION
// ====================================================================================================

// Should be defined by Visual Studio for us..
// #define FORKLIFT_MANAGER

// Does not work yet.
// #define SERVER_REPORT_THREAD

#ifdef FORKLIFT_MANAGER
#	define APP_TITLE		"Forklift Manager"
#else
#	define APP_TITLE		"Forklift"
#endif
#define APP_VERSION			"1.0"
#define APP_STRING			APP_TITLE " " APP_VERSION

// END FORKLIFT CONFIGURATION
// ====================================================================================================

#ifdef SERVER_REPORT_THREAD
#	include "HasherSmall.h"
#	include "ServerReportThread.h"
#endif

#include "libForklift.h"

#include "MinHook.h"

#ifdef FORKLIFT_MANAGER
// Incidium-Supra
// We want to hook WndProc so we get the user input events
#define WNDPROC_HOOK

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "imgui.h"

#include <Indicium/Engine/IndiciumCore.h>
#include <Indicium/Engine/IndiciumDirect3D9.h>
#include <Indicium/Engine/IndiciumDirect3D10.h>
#include <Indicium/Engine/IndiciumDirect3D11.h>

typedef LRESULT(WINAPI* t_WindowProc)(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
	);

LRESULT WINAPI DetourDefWindowProc(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

LRESULT WINAPI DetourWindowProc(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

void HookWindowProc(HWND hWnd);
void RenderScene();

bool ImGui_ImplWin32_UpdateMouseCursor();
IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

EVT_INDICIUM_GAME_HOOKED EvtIndiciumGameHooked;
EVT_INDICIUM_GAME_UNHOOKED EvtIndiciumGameUnhooked;

EVT_INDICIUM_D3D9_PRESENT EvtIndiciumD3D9Present;
EVT_INDICIUM_D3D9_RESET EvtIndiciumD3D9PreReset;
EVT_INDICIUM_D3D9_RESET EvtIndiciumD3D9PostReset;
EVT_INDICIUM_D3D9_PRESENT_EX EvtIndiciumD3D9PresentEx;
EVT_INDICIUM_D3D9_RESET_EX EvtIndiciumD3D9PreResetEx;
EVT_INDICIUM_D3D9_RESET_EX EvtIndiciumD3D9PostResetEx;

EVT_INDICIUM_D3D10_PRESENT EvtIndiciumD3D10Present;
EVT_INDICIUM_D3D10_RESIZE_BUFFERS EvtIndiciumD3D10PreResizeBuffers;
EVT_INDICIUM_D3D10_RESIZE_BUFFERS EvtIndiciumD3D10PostResizeBuffers;

EVT_INDICIUM_D3D11_PRE_PRESENT EvtIndiciumD3D11Present;
EVT_INDICIUM_D3D11_PRE_RESIZE_BUFFERS EvtIndiciumD3D11PreResizeBuffers;
EVT_INDICIUM_D3D11_POST_RESIZE_BUFFERS EvtIndiciumD3D11PostResizeBuffers;


/**
 * \fn	TOGGLE_STATE(int key, bool& toggle)
 *
 * \brief	Overly complicated key toggle helper
 *
 * \author	Benjamin "Nefarius" Höglinger-Stelzer
 * \date	7/09/2018
 *
 * \param 		  	key   	The key.
 * \param [in,out]	toggle	The value to toggle.
 */
VOID
FORCEINLINE
TOGGLE_STATE(int key, bool& toggle)
{
	static auto pressedPast = false, pressedNow = false;

	if (GetAsyncKeyState(key) & 0x8000)
	{
		pressedNow = true;
	}
	else
	{
		pressedPast = false;
		pressedNow = false;
	}

	if (!pressedPast && pressedNow)
	{
		toggle = !toggle;

		pressedPast = true;

		Sleep(1000);
	}
}


// 
// Detours
// 
#include <detours.h>

// 
// STL
// 
#include <mutex>

// 
// ImGui includes
// 
#include <imgui.h>
//#include "imgui_impl_dx9.h"
//#include "imgui_impl_dx10.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#endif

/* Dummy export to help when used via IAT injection */
extern "C" __declspec(dllexport) void __v0() {}