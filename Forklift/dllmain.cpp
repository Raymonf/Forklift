#include "stdafx.h"
#include "MinHook.h"
#include "Utilities.h"
#include "HandleCreation.h"
#include "FileSize.h"
#include "Ledger.h"
#include <process.h>
#include <iostream>
#include "dllmain.h"

#ifndef _DEBUG
// #include "HasherSmall.h"
// #include "ServerReportThread.h"
#endif

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
#include "imgui_impl_dx9.h"
#include "imgui_impl_dx10.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

t_WindowProc OriginalDefWindowProc = nullptr;
t_WindowProc OriginalWindowProc = nullptr;
PINDICIUM_ENGINE engine = nullptr;

extern "C" __declspec(dllexport) void __v0() {}

void forkliftThread(LPVOID param)
{
#ifdef _DEBUG
	Utilities::setupConsole();
#endif

	// Initialize MinHook
	if (MH_Initialize() != MH_OK)
	{
		MessageBoxA(NULL, "Failed to initialize MinHook", "Forklift Loader", MB_OK);
		return;
	}

#ifndef _DEBUG
	// HasherSmall::Install();
	// CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerReportThread::work, NULL, 0, NULL);
#endif
	
	if (Utilities::exists(".\\mods\\")) {
		Ledger::getMods(".\\mods\\");
		HandleCreation::Install();
		FileSize::Install();
	}
	else
	{
		MessageBoxA(NULL, "To get started with modding, create a folder called mods.", "Forklift Loader", MB_OK);
	}
}

/**
 * \fn  BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
 *
 * \brief   DLL main entry point. Only Indicium engine initialization or shutdown should happen
 *          here to avoid deadlocks.
 *
 * \author  Benjamin "Nefarius" H�glinger
 * \date    16.06.2018
 *
 * \param   hInstance   The instance handle.
 * \param   dwReason    The call reason.
 * \param   parameter3  Unused.
 *
 * \return  TRUE on success, FALSE otherwise (will abort loading the library).
 */
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
		_beginthread(forkliftThread, 0, nullptr);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		HandleCreation::Uninstall();
		FileSize::Uninstall();
#ifndef _DEBUG
		// HasherSmall::Uninstall();
#endif
		MH_Uninitialize();
	}

	//
	// We don't need to get notified in thread attach- or detachments
	// 
	DisableThreadLibraryCalls(static_cast<HMODULE>(hInstance));

	INDICIUM_ENGINE_CONFIG cfg;
	INDICIUM_ENGINE_CONFIG_INIT(&cfg);

	cfg.Direct3D.HookDirect3D9 = TRUE;
	cfg.Direct3D.HookDirect3D10 = TRUE;
	cfg.Direct3D.HookDirect3D11 = TRUE;

	cfg.EvtIndiciumGameHooked = EvtIndiciumGameHooked;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:

		//
		// Bootstrap the engine. Allocates resources, establishes hooks etc.
		// 
		(void)IndiciumEngineCreate(
			static_cast<HMODULE>(hInstance),
			&cfg,
			NULL
		);

		break;
	case DLL_PROCESS_DETACH:

		//
		// Tears down the engine. Graceful shutdown, frees resources etc.
		// 
		(void)IndiciumEngineDestroy(static_cast<HMODULE>(hInstance));

		break;
	default:
		break;
	}

	return TRUE;
}

/**
 * \fn	void EvtIndiciumGameHooked( PINDICIUM_ENGINE EngineHandle, const INDICIUM_D3D_VERSION GameVersion )
 *
 * \brief	Gets called when the games' rendering pipeline has successfully been hooked and the
 * 			rendering callbacks are about to get fired. The detected version of the used
 * 			rendering objects is reported as well.
 *
 * \author	Benjamin "Nefarius" H�glinger
 * \date	16.06.2018
 *
 * \param	EngineHandle	Handle of the engine.
 * \param	GameVersion 	The detected DirectX/Direct3D version.
 */
void EvtIndiciumGameHooked(
	PINDICIUM_ENGINE EngineHandle,
	const INDICIUM_D3D_VERSION GameVersion
)
{
	IndiciumEngineLogInfo("Loading ImGui plugin");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	INDICIUM_D3D9_EVENT_CALLBACKS d3d9;
	INDICIUM_D3D9_EVENT_CALLBACKS_INIT(&d3d9);
	d3d9.EvtIndiciumD3D9PrePresent = EvtIndiciumD3D9Present;
	d3d9.EvtIndiciumD3D9PreReset = EvtIndiciumD3D9PreReset;
	d3d9.EvtIndiciumD3D9PostReset = EvtIndiciumD3D9PostReset;
	d3d9.EvtIndiciumD3D9PrePresentEx = EvtIndiciumD3D9PresentEx;
	d3d9.EvtIndiciumD3D9PreResetEx = EvtIndiciumD3D9PreResetEx;
	d3d9.EvtIndiciumD3D9PostResetEx = EvtIndiciumD3D9PostResetEx;

	INDICIUM_D3D10_EVENT_CALLBACKS d3d10;
	INDICIUM_D3D10_EVENT_CALLBACKS_INIT(&d3d10);
	d3d10.EvtIndiciumD3D10PrePresent = EvtIndiciumD3D10Present;
	d3d10.EvtIndiciumD3D10PreResizeBuffers = EvtIndiciumD3D10PreResizeBuffers;
	d3d10.EvtIndiciumD3D10PostResizeBuffers = EvtIndiciumD3D10PostResizeBuffers;

	INDICIUM_D3D11_EVENT_CALLBACKS d3d11;
	INDICIUM_D3D11_EVENT_CALLBACKS_INIT(&d3d11);
	d3d11.EvtIndiciumD3D11PrePresent = EvtIndiciumD3D11Present;
	d3d11.EvtIndiciumD3D11PreResizeBuffers = EvtIndiciumD3D11PreResizeBuffers;
	d3d11.EvtIndiciumD3D11PostResizeBuffers = EvtIndiciumD3D11PostResizeBuffers;

	switch (GameVersion)
	{
	case IndiciumDirect3DVersion9:
		IndiciumEngineSetD3D9EventCallbacks(EngineHandle, &d3d9);
		break;
	case IndiciumDirect3DVersion10:
		IndiciumEngineSetD3D10EventCallbacks(EngineHandle, &d3d10);
		break;
	case IndiciumDirect3DVersion11:
		IndiciumEngineSetD3D11EventCallbacks(EngineHandle, &d3d11);
		break;
	}
}

/**
 * \fn  void EvtIndiciumGameUnhooked()
 *
 * \brief   Gets called when all core engine hooks have been released. At this stage it is save
 *          to remove our own additional hooks and shut down the hooking sub-system as well.
 *
 * \author  Benjamin "Nefarius" H�glinger
 * \date    16.06.2018
 */
void EvtIndiciumGameUnhooked()
{
#ifdef WNDPROC_HOOK
	auto& logger = Logger::get(__func__);

	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
	{
		logger.fatal("Couldn't disable hooks, host process might crash");
		return;
	}

	IndiciumEngineLogInfo("Hooks disabled");

	if (MH_Uninitialize() != MH_OK)
	{
		logger.fatal("Couldn't shut down hook engine, host process might crash");
		return;
	}
#endif
}

#pragma region D3D9(Ex)

void EvtIndiciumD3D9Present(
	LPDIRECT3DDEVICE9   pDevice,
	const RECT* pSourceRect,
	const RECT* pDestRect,
	HWND                hDestWindowOverride,
	const RGNDATA* pDirtyRegion
)
{
	static auto initialized = false;
	static bool show_overlay = true;
	static std::once_flag init;

	//
	// This section is only called once to initialize ImGui
	// 
	std::call_once(init, [&](LPDIRECT3DDEVICE9 pd3dDevice)
		{
			D3DDEVICE_CREATION_PARAMETERS params;

			const auto hr = pd3dDevice->GetCreationParameters(&params);
			if (FAILED(hr))
			{
				IndiciumEngineLogError("Couldn't get creation parameters from device");
				return;
			}

			ImGui_ImplWin32_Init(params.hFocusWindow);
			ImGui_ImplDX9_Init(pd3dDevice);

			IndiciumEngineLogInfo("ImGui (DX9) initialized");

			HookWindowProc(params.hFocusWindow);

			initialized = true;

		}, pDevice);

	if (!initialized)
		return;

	TOGGLE_STATE(VK_F12, show_overlay);
	if (!show_overlay)
		return;

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	RenderScene();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void EvtIndiciumD3D9PreReset(
	LPDIRECT3DDEVICE9       pDevice,
	D3DPRESENT_PARAMETERS* pPresentationParameters
)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void EvtIndiciumD3D9PostReset(
	LPDIRECT3DDEVICE9       pDevice,
	D3DPRESENT_PARAMETERS* pPresentationParameters
)
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

void EvtIndiciumD3D9PresentEx(
	LPDIRECT3DDEVICE9EX     pDevice,
	const RECT* pSourceRect,
	const RECT* pDestRect,
	HWND                    hDestWindowOverride,
	const RGNDATA* pDirtyRegion,
	DWORD                   dwFlags
)
{
	static auto initialized = false;
	static bool show_overlay = true;
	static std::once_flag init;

	//
	// This section is only called once to initialize ImGui
	// 
	std::call_once(init, [&](LPDIRECT3DDEVICE9EX pd3dDevice)
		{
			D3DDEVICE_CREATION_PARAMETERS params;

			const auto hr = pd3dDevice->GetCreationParameters(&params);
			if (FAILED(hr))
			{
				IndiciumEngineLogError("Couldn't get creation parameters from device");
				return;
			}

			ImGui_ImplWin32_Init(params.hFocusWindow);
			ImGui_ImplDX9_Init(pd3dDevice);

			IndiciumEngineLogInfo("ImGui (DX9Ex) initialized");

			HookWindowProc(params.hFocusWindow);

			initialized = true;

		}, pDevice);

	if (!initialized)
		return;

	TOGGLE_STATE(VK_F12, show_overlay);
	if (!show_overlay)
		return;

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	RenderScene();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void EvtIndiciumD3D9PreResetEx(
	LPDIRECT3DDEVICE9EX     pDevice,
	D3DPRESENT_PARAMETERS* pPresentationParameters,
	D3DDISPLAYMODEEX* pFullscreenDisplayMode
)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void EvtIndiciumD3D9PostResetEx(
	LPDIRECT3DDEVICE9EX     pDevice,
	D3DPRESENT_PARAMETERS* pPresentationParameters,
	D3DDISPLAYMODEEX* pFullscreenDisplayMode
)
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

#pragma endregion

#pragma region D3D10

void EvtIndiciumD3D10Present(
	IDXGISwapChain* pSwapChain,
	UINT            SyncInterval,
	UINT            Flags
)
{
	static auto initialized = false;
	static bool show_overlay = true;
	static std::once_flag init;

	//
	// This section is only called once to initialize ImGui
	// 
	std::call_once(init, [&](IDXGISwapChain* pChain)
		{
			IndiciumEngineLogInfo("Grabbing device and context pointers");

			ID3D10Device* pDevice;
			if (FAILED(D3D10_DEVICE_FROM_SWAPCHAIN(pChain, &pDevice)))
			{
				IndiciumEngineLogError("Couldn't get device from swapchain");
				return;
			}

			DXGI_SWAP_CHAIN_DESC sd;
			pChain->GetDesc(&sd);

			IndiciumEngineLogInfo("Initializing ImGui");

			ImGui_ImplWin32_Init(sd.OutputWindow);
			ImGui_ImplDX10_Init(pDevice);

			IndiciumEngineLogInfo("ImGui (DX10) initialized");

			HookWindowProc(sd.OutputWindow);

			initialized = true;

		}, pSwapChain);

	if (!initialized)
		return;

	TOGGLE_STATE(VK_F12, show_overlay);
	if (!show_overlay)
		return;


	// Start the Dear ImGui frame
	ImGui_ImplDX10_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	RenderScene();

	ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());
}

void EvtIndiciumD3D10PreResizeBuffers(
	IDXGISwapChain* pSwapChain,
	UINT            BufferCount,
	UINT            Width,
	UINT            Height,
	DXGI_FORMAT     NewFormat,
	UINT            SwapChainFlags
)
{
	ImGui_ImplDX10_InvalidateDeviceObjects();
}

void EvtIndiciumD3D10PostResizeBuffers(
	IDXGISwapChain* pSwapChain,
	UINT            BufferCount,
	UINT            Width,
	UINT            Height,
	DXGI_FORMAT     NewFormat,
	UINT            SwapChainFlags
)
{
	ImGui_ImplDX10_CreateDeviceObjects();
}

#pragma endregion

#pragma region D3D11

// TODO: lazy global, improve
static ID3D11RenderTargetView* g_d3d11_mainRenderTargetView = nullptr;

void EvtIndiciumD3D11Present(
	IDXGISwapChain* pSwapChain,
	UINT						SyncInterval,
	UINT						Flags,
	PINDICIUM_EVT_PRE_EXTENSION Extension
)
{
	static auto initialized = false;
	static bool show_overlay = true;
	static std::once_flag init;

	static ID3D11DeviceContext* pContext;

	//
	// This section is only called once to initialize ImGui
	// 
	std::call_once(init, [&](IDXGISwapChain* pChain)
		{
			IndiciumEngineLogInfo("Grabbing device and context pointers");

			ID3D11Device* pDevice;
			if (FAILED(D3D11_DEVICE_IMMEDIATE_CONTEXT_FROM_SWAPCHAIN(pChain, &pDevice, &pContext)))
			{
				IndiciumEngineLogError("Couldn't get device and context from swapchain");
				return;
			}

			ID3D11Texture2D* pBackBuffer;
			pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_d3d11_mainRenderTargetView);
			pBackBuffer->Release();

			DXGI_SWAP_CHAIN_DESC sd;
			pChain->GetDesc(&sd);

			IndiciumEngineLogInfo("Initializing ImGui");

			ImGui_ImplWin32_Init(sd.OutputWindow);
			ImGui_ImplDX11_Init(pDevice, pContext);

			IndiciumEngineLogInfo("ImGui (DX11) initialized");

			HookWindowProc(sd.OutputWindow);

			initialized = true;

		}, pSwapChain);

	if (!initialized)
		return;

	TOGGLE_STATE(VK_F12, show_overlay);
	if (!show_overlay)
		return;

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	pContext->OMSetRenderTargets(1, &g_d3d11_mainRenderTargetView, NULL);

	RenderScene();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

//
// Called prior to the original invocation of ResizeBuffers.
// 
void EvtIndiciumD3D11PreResizeBuffers(
	IDXGISwapChain* pSwapChain,
	UINT						BufferCount,
	UINT						Width,
	UINT						Height,
	DXGI_FORMAT					NewFormat,
	UINT						SwapChainFlags,
	PINDICIUM_EVT_PRE_EXTENSION Extension
)
{
	if (g_d3d11_mainRenderTargetView)
	{
		g_d3d11_mainRenderTargetView->Release();
		g_d3d11_mainRenderTargetView = nullptr;
	}
}

//
// Called after the original invocation of ResizeBuffers.
// 
void EvtIndiciumD3D11PostResizeBuffers(
	IDXGISwapChain* pSwapChain,
	UINT							BufferCount,
	UINT							Width,
	UINT							Height,
	DXGI_FORMAT						NewFormat,
	UINT							SwapChainFlags,
	PINDICIUM_EVT_POST_EXTENSION	Extension
)
{
	ID3D11Texture2D* pBackBuffer;
	ID3D11DeviceContext* pContext;
	ID3D11Device* pDevice;
	D3D11_DEVICE_IMMEDIATE_CONTEXT_FROM_SWAPCHAIN(pSwapChain, &pDevice, &pContext);

	pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_d3d11_mainRenderTargetView);
	pBackBuffer->Release();
}

#pragma endregion

#pragma region WNDPROC Hooking

void HookWindowProc(HWND hWnd)
{
#ifdef WNDPROC_HOOK

	auto& logger = Logger::get(__func__);

	MH_STATUS ret;

	if ((ret = MH_CreateHook(
		&DefWindowProcW,
		&DetourDefWindowProc,
		reinterpret_cast<LPVOID*>(&OriginalDefWindowProc))
		) != MH_OK)
	{
		IndiciumEngineLogError("Couldn't create hook for DefWindowProcW: %lu", static_cast<ULONG>(ret));
		return;
	}

	if (ret == MH_OK && MH_EnableHook(&DefWindowProcW) != MH_OK)
	{
		IndiciumEngineLogError("Couldn't enable DefWindowProcW hook");
	}

	if ((ret = MH_CreateHook(
		&DefWindowProcA,
		&DetourDefWindowProc,
		reinterpret_cast<LPVOID*>(&OriginalDefWindowProc))
		) != MH_OK)
	{
		IndiciumEngineLogError("Couldn't create hook for DefWindowProcA: %lu", static_cast<ULONG>(ret));
		return;
	}

	if (ret == MH_OK && MH_EnableHook(&DefWindowProcA) != MH_OK)
	{
		IndiciumEngineLogError("Couldn't enable DefWindowProcW hook");
	}

	auto lptrWndProc = reinterpret_cast<t_WindowProc>(GetWindowLongPtr(hWnd, GWLP_WNDPROC));

	if (MH_CreateHook(lptrWndProc, &DetourWindowProc, reinterpret_cast<LPVOID*>(&OriginalWindowProc)) != MH_OK)
	{
		logger.warning("Couldn't create hook for GWLP_WNDPROC");
		return;
	}

	if (MH_EnableHook(lptrWndProc) != MH_OK)
	{
		IndiciumEngineLogError("Couldn't enable GWLP_WNDPROC hook");
	}
#endif
}

LRESULT WINAPI DetourDefWindowProc(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	static std::once_flag flag;
	std::call_once(flag, []() { IndiciumEngineLogInfo("++ DetourDefWindowProc called"); });

	ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);

	return OriginalDefWindowProc(hWnd, Msg, wParam, lParam);
}

LRESULT WINAPI DetourWindowProc(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	static std::once_flag flag;
	std::call_once(flag, []() { IndiciumEngineLogInfo("++ DetourWindowProc called"); });

	ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);

	return OriginalWindowProc(hWnd, Msg, wParam, lParam);
}

#pragma endregion

#pragma region Main content rendering

void RenderScene()
{
	static std::once_flag flag;
	std::call_once(flag, []() { IndiciumEngineLogInfo("++ RenderScene called"); });

	ImGui::ShowMetricsWindow();

	{
		ImGui::SetNextWindowPos(ImVec2(1400, 60));
		ImGui::Begin(
			"Some plots =)",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize
		);

		static bool animate = true;
		ImGui::Checkbox("Animate", &animate);

		static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
		ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

		// Create a dummy array of contiguous float values to plot
		// Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
		static float values[90] = { 0 };
		static int values_offset = 0;
		static float refresh_time = 0.0f;
		if (!animate || refresh_time == 0.0f)
			refresh_time = ImGui::GetTime();
		while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
		{
			static float phase = 0.0f;
			values[values_offset] = cosf(phase);
			values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
			phase += 0.10f * values_offset;
			refresh_time += 1.0f / 60.0f;
		}
		ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, "avg 0.0", -1.0f, 1.0f, ImVec2(0, 80));
		ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));

		// Use functions to generate output
		// FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
		struct Funcs
		{
			static float Sin(void*, int i) { return sinf(i * 0.1f); }
			static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
		};
		static int func_type = 0, display_count = 70;
		ImGui::Separator();
		ImGui::PushItemWidth(100); ImGui::Combo("func", &func_type, "Sin\0Saw\0"); ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::SliderInt("Sample count", &display_count, 1, 400);
		float(*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
		ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
		ImGui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
		ImGui::Separator();

		// Animate a simple progress bar
		static float progress = 0.0f, progress_dir = 1.0f;
		if (animate)
		{
			progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
			if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
			if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
		}

		// Typically we would use ImVec2(-1.0f,0.0f) to use all available width, or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
		ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Progress Bar");

		float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
		char buf[32];
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
		sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
		ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);

		ImGui::End();
	}

	ImGui::Render();
}

#pragma endregion

#pragma region ImGui-specific (taken from their examples unmodified)

bool ImGui_ImplWin32_UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
		return false;

	ImGuiMouseCursor imgui_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		::SetCursor(NULL);
	}
	else
	{
		// Hardware cursor type
		LPTSTR win32_cursor = IDC_ARROW;
		switch (imgui_cursor)
		{
		case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
		case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
		case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
		case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
		case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
		case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
		case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
		}
		::SetCursor(::LoadCursor(NULL, win32_cursor));
	}
	return true;
}

#pragma endregion




