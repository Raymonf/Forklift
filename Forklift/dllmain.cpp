#include "dllmain.h"

#ifdef FORKLIFT_MANAGER
HINSTANCE g_hInstance;
#endif

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
#	ifdef FORKLIFT_MANAGER
		g_hInstance = hInstance;
#	endif

	DisableThreadLibraryCalls(static_cast<HMODULE>(hInstance));
	if (dwReason == DLL_PROCESS_ATTACH) {
		Forklift::initialize();

#		ifdef FORKLIFT_MANAGER
			// Initialize Indicium-Supra
			INDICIUM_ENGINE_CONFIG cfg;
			INDICIUM_ENGINE_CONFIG_INIT(&cfg);
			cfg.Direct3D.HookDirect3D11 = TRUE;
			cfg.Direct3D.HookDirect3D10 = FALSE;
			cfg.Direct3D.HookDirect3D9 = FALSE;
			cfg.EvtIndiciumGameHooked = EvtIndiciumGameHooked;

			(void)IndiciumEngineCreate(static_cast<HMODULE>(g_hInstance), &cfg, NULL);
#		endif
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		Forklift::destroy();

#		ifdef FORKLIFT_MANAGER
			(void)IndiciumEngineDestroy(static_cast<HMODULE>(hInstance));
#		endif
	}
	return TRUE;
}

#ifdef FORKLIFT_MANAGER
t_WindowProc OriginalDefWindowProc = nullptr;
t_WindowProc OriginalWindowProc = nullptr;
PINDICIUM_ENGINE engine = nullptr;

// Renders the UI for Forklift Manager
void RenderUI(void)
{
	auto ResetLedger = []() {
		ledger.clear();
		Ledger::getMods(LIBFORKLIFT_MODS_DIR);
	};

	ImGui::SetNextWindowSizeConstraints(ImVec2(375, 475), ImVec2(500, 500));
	ImGui::Begin(APP_STRING);
	if (ImGui::Checkbox("Enabled", &g_bHookEnabled))
		if (g_bHookEnabled)
			ResetLedger();
	ImGui::Separator();

#ifdef _DEBUG
	if (ImGui::Button("Refresh Ledger"))
		ResetLedger();
	ImGui::SameLine();

	if (ImGui::Button("Clear Ledger"))
		ledger.clear();
	ImGui::SameLine();

	// Specifies the first # of entries to set to false in the test subroutine
	#define TEST_SET_FIRST_NUM 10
	if (ImGui::Button(std::string("Toggle first " + std::to_string(TEST_SET_FIRST_NUM)).c_str())) {
		int counter = 0;
		for (LedgerRecord& tmp : ledger) {
			tmp.enabled = !tmp.enabled;
			if (++counter >= TEST_SET_FIRST_NUM)
				break;
		}
	}
	ImGui::Separator();
#endif

	if (ledger.size()) {
		ImGui::Text("\nFiles:");
		ImGui::BeginChild("Mods");
		for (LedgerRecord& record : ledger) {
			ImGui::Checkbox(record.getPath().c_str(), &record.enabled);
		}
		ImGui::EndChild();
		ImGui::Separator();
	}
	ImGui::End();
}

/**
 * \fn	void EvtIndiciumGameHooked( PINDICIUM_ENGINE EngineHandle, const INDICIUM_D3D_VERSION GameVersion )
 *
 * \brief	Gets called when the games' rendering pipeline has successfully been hooked and the
 * 			rendering callbacks are about to get fired. The detected version of the used
 * 			rendering objects is reported as well.
 *
 * \author	Benjamin "Nefarius" Höglinger
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

	/*INDICIUM_D3D9_EVENT_CALLBACKS d3d9;
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
	d3d10.EvtIndiciumD3D10PostResizeBuffers = EvtIndiciumD3D10PostResizeBuffers;*/

	INDICIUM_D3D11_EVENT_CALLBACKS d3d11;
	INDICIUM_D3D11_EVENT_CALLBACKS_INIT(&d3d11);
	d3d11.EvtIndiciumD3D11PrePresent = EvtIndiciumD3D11Present;
	d3d11.EvtIndiciumD3D11PreResizeBuffers = EvtIndiciumD3D11PreResizeBuffers;
	d3d11.EvtIndiciumD3D11PostResizeBuffers = EvtIndiciumD3D11PostResizeBuffers;

	switch (GameVersion)
	{
	/*case IndiciumDirect3DVersion9:
		IndiciumEngineSetD3D9EventCallbacks(EngineHandle, &d3d9);
		break;
	case IndiciumDirect3DVersion10:
		IndiciumEngineSetD3D10EventCallbacks(EngineHandle, &d3d10);
		break;*/
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
 * \author  Benjamin "Nefarius" Höglinger
 * \date    16.06.2018
 */
void EvtIndiciumGameUnhooked()
{
#ifdef WNDPROC_HOOK
	//auto& logger = Logger::get(__func__);

	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
	{
		//logger.fatal("Couldn't disable hooks, host process might crash");
		return;
	}

	IndiciumEngineLogInfo("Hooks disabled");

	if (MH_Uninitialize() != MH_OK)
	{
		//logger.fatal("Couldn't shut down hook engine, host process might crash");
		return;
	}
#endif
}

/*

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

*/

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

			// MessageBoxA(sd.OutputWindow, "Welcome to " APP_STRING "!\n\nPress F11 to toggle Forklift during gameplay.\nPress F12 to toggle the overlay window during gameplay.", APP_STRING, MB_OK);

			HookWindowProc(sd.OutputWindow);

			initialized = true;

		}, pSwapChain);

	if (!initialized)
		return;

	TOGGLE_STATE(VK_F12, show_overlay);
	TOGGLE_STATE(VK_F11, g_bHookEnabled);
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

	//auto& logger = Logger::get(__func__);

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
		//logger.warning("Couldn't create hook for GWLP_WNDPROC");
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

	return ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);

	//return OriginalWindowProc(hWnd, Msg, wParam, lParam);
}

#pragma endregion

#pragma region Main content rendering

void RenderScene()
{
	static std::once_flag flag;
	std::call_once(flag, []() { IndiciumEngineLogInfo("++ RenderScene called"); });

	RenderUI();

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

#endif