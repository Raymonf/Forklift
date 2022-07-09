#include <Windows.h>
#include "FileSize.h"
#include "VersionManager.h"
#include "MinHook.h"
#include "Ledger.h"
#include <string>
#include <iostream>

__int64 getSizeAddr = reinterpret_cast<__int64>(GetModuleHandle(NULL)) + VersionManager::singleton()->getFileSizeAddress();
getSizeT getSize = reinterpret_cast<getSizeT>(getSizeAddr);

void FileSize::Install()
{
	MH_CreateHook(reinterpret_cast<LPVOID>(getSizeAddr), &Hook, reinterpret_cast<LPVOID*>(&getSize));
	MH_EnableHook(reinterpret_cast<LPVOID>(getSizeAddr));
}

void FileSize::Uninstall()
{	
	MH_DisableHook(reinterpret_cast<LPVOID>(getSizeAddr));
}

unsigned __int64 __fastcall FileSize::Hook(const char *lpFileName)
{
	if (Ledger::isModPath(lpFileName)) {
		auto entry = Ledger::getRecordFromPath(lpFileName).getEntry();
		return entry.file_size();
	}

	return getSize(lpFileName);
}
