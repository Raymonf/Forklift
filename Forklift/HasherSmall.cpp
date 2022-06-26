#include "stdafx.h"
#include <iostream>
#include "MinHook.h"
#include "HasherSmall.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include "PrintThread.h"
#include "HashFile.h"
// #include "ServerReportThread.h"
#include "VersionManager.h"

tGetHashSmall originalHasherSmall = NULL;
std::vector<std::string> noted;
std::vector<HashFile> hashes;

void HasherSmall::Install()
{
	__int64 addr = (__int64)GetModuleHandle(NULL) + VersionManager::singleton()->getHasherSmallAddress();

	MH_CreateHook((LPVOID)addr, &Hook, reinterpret_cast<LPVOID*>(&originalHasherSmall));
	MH_EnableHook((LPVOID)addr);
}

void HasherSmall::Uninstall()
{
	__int64 addr = (__int64)GetModuleHandle(NULL) + VersionManager::singleton()->getHasherSmallAddress();

	MH_DisableHook(reinterpret_cast<LPVOID>(addr));
}

unsigned __int64 __fastcall HasherSmall::Hook(__int64 a1)
{
	const char *path = (const char *)a1;

	auto res = originalHasherSmall((__int64)path);

	auto hash = _byteswap_ulong((unsigned long)res);

	if (std::find(noted.begin(), noted.end(), path) == noted.end())
	{
		noted.push_back(path);

		try
		{
			std::string p(path);

			if (p.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-/") == std::string::npos)
			{
				auto size = p.size();
				char *buffer = new char[size + 1];
				memcpy(buffer, p.c_str(), size + 1);

				HashFile* hashFile = new HashFile();
				hashFile->path = buffer;
				hashFile->hash = hash;
				// ServerReportThread::push(hashFile);
			}
		}
		catch (std::exception &ex)
		{
			//PrintThread{} << "[HasherSmall] Error: " << ex.what() << std::endl;
		}
	}

	return res;
}