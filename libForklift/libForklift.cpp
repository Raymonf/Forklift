#include "libForklift.h"

#include "VersionManager.h"
#include <string>
#include <iostream>
#include <fstream>

#include <atlbase.h>
#include <Shlobj.h>
#include <Windows.h>
#undef min
#undef max

#include "MinHook.h"

//#include <curl\curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool Forklift::g_bHookEnabled = true;

size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

/*bool compare_online_tag_to_our_tag(const char* szRepoAndOwner, const char* szCurrentTag)
{
	bool result = true;
	curl_global_init(CURL_GLOBAL_ALL);

	if (CURL* curl = curl_easy_init()) {
		struct curl_slist* headers = NULL;
		curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, std::string("https://api.github.com/repos/").append(szRepoAndOwner).append("/releases/latest").c_str());
#		ifdef _DEBUG
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#		endif
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libForklift/1.0");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

		std::string response;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
#			ifdef _DEBUG
				printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
#			endif
		} else {
			json j = json::parse(response);
			
			if (j.contains("tag_name")) {
				auto tag_name = j["tag_name"].get<std::string>();
#				ifdef _DEBUG
					printf("tag = %s\ncur = %s\n", tag_name.c_str(), szCurrentTag);
#				endif
				if (!strstr(tag_name.c_str(), szCurrentTag))
					result = false;
			}
		}
		curl_easy_cleanup(curl);
	}
#ifdef _DEBUG
	else
		printf("curl_easy_init() failed\n");
#endif

	curl_global_cleanup();
	return result;
}

static void update_thread(void) {
	if (!compare_online_tag_to_our_tag("Raymonf/Forklift", LIBFORKLIFT_TAG)) 
	{
		
		if (MessageBoxA(NULL, "Your version of Forklift is out-of-date!\n\nPlease download the latest version after clicking the OK button or press the cancel button to ignore this message.", "Forklift" " " LIBFORKLIFT_TAG, MB_OKCANCEL) == IDOK)
		{
			ShellExecuteA(NULL, "open", "https://github.com/Raymonf/Forklift/releases/latest", NULL, NULL, SW_SHOWNORMAL);
		}
	}
}*/

void Forklift::initialize()
{

#	ifdef _DEBUG
	Utilities::setupConsole();
#	endif

	// check if we're even able to do anything with this executable.
	VersionManager * versionManager = VersionManager::singleton();
	Version version = versionManager->getVersion();
	if (version == Version::Unknown)
	{
		MessageBoxA(NULL, "Failed to detect the currently running version of Shenmue I/II. Exiting.", "libForklift", MB_OK);
		exit(-1); // cya~!
		return;
	}

	// Initialize MinHook
	if (MH_Initialize() != MH_OK)
	{
		MessageBoxA(NULL, "Failed to initialize MinHook", "libForklift", MB_OK);
		exit(-1); // cya~!
		return;
	}

#	ifdef SERVER_REPORT_THREAD
#		ifndef _DEBUG
			HasherSmall::Install();
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerReportThread::work, NULL, 0, NULL);
#		endif
#	endif

	// start the update checker thread..
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&update_thread, NULL, 0, NULL);

	// make sure mods directory exists..		
	auto dir = versionManager->getModsDir();
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directories(dir);
		Sleep(100);
	}

	// initialize ledger
	Ledger::getMods();

	// install hooks
	HandleCreation::Install();
	FileSize::Install();

	// Shenmue 1 v1.07 only for now as we need to port more offsets
	//if (VersionManager::singleton()->getVersion() == Version::Coconut107)
	//	TextureOverridePatch::Install();
}

void Forklift::destroy()
{
	g_bHookEnabled = false;

	ledger.clear();

	if (TextureOverridePatch::oldOverridesTbl)
		free((void*)TextureOverridePatch::oldOverridesTbl);

	HandleCreation::Uninstall();
	FileSize::Uninstall();

#	ifdef SERVER_REPORT_THREAD
#		ifndef _DEBUG
			HasherSmall::Uninstall();
#		endif
#	endif

	MH_Uninitialize();
}