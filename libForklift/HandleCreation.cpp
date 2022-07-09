#include <Windows.h>
#include "HandleCreation.h"
#include "VersionManager.h"
#include "MinHook.h"
#include "Utilities.h"
#include "Ledger.h"
#include <iostream>
#include <string>

bool g_bHookEnabled = true;

__int64 handleCreationAddr = reinterpret_cast<__int64>(GetModuleHandle(NULL)) + VersionManager::singleton()->getHandleCreationAddress();
handleCreationT handleCreation = nullptr;

void HandleCreation::Install()
{
	MH_CreateHook(reinterpret_cast<LPVOID>(handleCreationAddr), &Hook, reinterpret_cast<LPVOID*>(&handleCreation));
	MH_EnableHook(reinterpret_cast<LPVOID>(handleCreationAddr));
}

void HandleCreation::Uninstall()
{
	MH_DisableHook(reinterpret_cast<LPVOID>(handleCreationAddr));
}

__int64 HandleCreation::Hook(__int64 a1, char *assetPath, __int64 a3)
{
	std::string path = assetPath;
	if (g_bHookEnabled) 
	{
		if (Ledger::isModPath(path))
		{
			path = ".\\mods\\" + path;
#ifdef _DEBUG
			std::cout << "[ * ] " << path << std::endl;
#endif
		}
	}
	auto ret = handleCreation(a1, const_cast<char *>(path.c_str()), a3);
	return ret;
}
