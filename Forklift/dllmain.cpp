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

BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		_beginthread(forkliftThread, 0, nullptr);
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		HandleCreation::Uninstall();
		FileSize::Uninstall();
#ifndef _DEBUG
		// HasherSmall::Uninstall();
#endif
		MH_Uninitialize();
	}

	return TRUE;
}

extern "C" __declspec(dllexport) void __v0() {}
