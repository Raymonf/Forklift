#include "libForklift.h"
#include "MinHook.h"

bool Forklift::g_bHookEnabled = true;
std::string_view Utilities::curr_path;

void Forklift::initialize()
{
#	ifdef _DEBUG
		Utilities::setupConsole();
#	endif

	// Initialize MinHook
	if (MH_Initialize() != MH_OK)
	{
		MessageBoxA(NULL, "Failed to initialize MinHook", "libForklift", MB_OK);
		return;
	}

#	ifdef SERVER_REPORT_THREAD
#		ifndef _DEBUG
			HasherSmall::Install();
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerReportThread::work, NULL, 0, NULL);
#		endif
#	endif

	if (!std::filesystem::exists(".\\mods\\")) {
		std::filesystem::create_directories(".\\mods\\");
		Sleep(100);
	}

	Ledger::getMods(".\\mods\\");

	HandleCreation::Install();
	FileSize::Install();
}

void Forklift::destroy()
{
	g_bHookEnabled = false;

	HandleCreation::Uninstall();
	FileSize::Uninstall();

#	ifdef SERVER_REPORT_THREAD
#		ifndef _DEBUG
			HasherSmall::Uninstall();
#		endif
#	endif

	MH_Uninitialize();
}