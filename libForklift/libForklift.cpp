#define _CRT_SECURE_NO_WARNINGS

#include "libForklift.h"

#include <string>
#include <iostream>
#include <fstream>

#include <Windows.h>
#undef min
#undef max

#include "MinHook.h"

#include <curl\curl.h>

#include <nlohmann/json.hpp>

#include <shendk/files/container/tad.h>
#include <shendk/utils/hash_db.h>

using json = nlohmann::json;

bool Forklift::g_bHookEnabled = true;
std::string_view Utilities::curr_path;

size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

bool compare_online_tag_to_our_tag(const char* szRepoAndOwner, const char* szCurrentTag)
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
	if (!compare_online_tag_to_our_tag("Raymonf/Forklift", LIBFORKLIFT_TAG)) {
		int res = MessageBoxA(NULL, "Your version of Forklift is out-of-date!\n\nPlease download the latest version after clicking the OK button or press the cancel button to ignore this message.", "Forklift" " " LIBFORKLIFT_TAG, MB_OKCANCEL);

		if (res == IDOK) {
			ShellExecuteA(NULL, "open", "https://github.com/Raymonf/Forklift/releases/latest", NULL, NULL, SW_SHOWNORMAL);
			exit(-1);		// cya~!
		}
	}
}

struct texOverride_t {
	char original_path[MAX_PATH];
	char remap_path[MAX_PATH];

	texOverride_t()
	{
		memset(original_path, 0x00, MAX_PATH);
		memset(remap_path, 0x00, MAX_PATH);
	}
};

std::vector<texOverride_t> mappings;
DWORD_PTR oldOverridesTbl = 0;
int oldNumOverrides = 0;
bool bHasBackedUp = false;

typedef void(__fastcall* fn_t)(__int64, int, const char*, __int64);
fn_t fn_orig;

int AddMappings(std::istream& f)
{
	int res = 0;
	json data = json::parse(f);
	for (auto& items : data["Mappings"]) {
		texOverride_t tmp;
		sprintf(tmp.original_path, "%s", items["Source"].get<std::string>().c_str());
		sprintf(tmp.remap_path, "%s", items["Destination"].get<std::string>().c_str());
		mappings.push_back(tmp);
		mappings.shrink_to_fit();
		res++;
	}
	return res;
}

static bool g_bMappingsInitialized = false;

void hook(__int64 a1, int a2, const char* a3, __int64 a4)
{
	while (!g_bMappingsInitialized) {}

	fn_orig(a1, a2, a3, a4);

	auto base = GetModuleHandle(NULL);

	// iterate through every json, collecting all of the mapping overrides and
	// inserting them into our own vector for them.
	for (auto& file : std::filesystem::recursive_directory_iterator(".\\mods\\jsons\\")) {
		if (strstr(file.path().extension().string().c_str(), ".json")) {
			std::ifstream f(file.path().string());
			
			if (f.good()) {
				auto ret = AddMappings(f);
#ifdef _DEBUG
				printf("Added %d custom mappings from mods dir (%s)\n", ret, file.path().string().c_str());
#endif
				f.close();
			}
		}
	}

	// back up the old table address as this has to be freed manually anyways, 
	// so we can back it up and restore it if the user wants to disable overrides for whatever reason.
	if (!bHasBackedUp) {
		oldOverridesTbl = *(DWORD_PTR*)((DWORD_PTR)(base) + 0xEB15C0);
		oldNumOverrides = *(int*)((DWORD_PTR)base + 0xEB14E4llu);
		bHasBackedUp = true;
	}
	
	// allocate new space which can hold the new mappings
	DWORD_PTR mapping_tbl = *(DWORD_PTR*)((DWORD_PTR)(base) + 0xEB15C0);
	size_t size = (MAX_PATH * 2) * mappings.size();
	void* mem = malloc(size);
	if (mem) 
	{
		// update the pointer in memory to point to our new space
		memset(mem, 0x00, size);
		*(DWORD_PTR*)((DWORD_PTR)(base) + 0xEB15C0) = (DWORD_PTR)mem;

		// write all of the mappings into the new space
		int index = 0;
		for (auto& mapping : mappings)
		{
			memcpy(((char*)mem + ((MAX_PATH * 2) * index)), &mapping, sizeof texOverride_t);
			++index;
		}

		// finally update the total count of mappings!
		*(int*)((DWORD_PTR)base + 0xEB14E4llu) = index;
	}
}

void init_override_thread()
{
	shendk::HashDB& db = shendk::HashDB::getInstance();
	db.initialize("filename_database.json");

	for (const auto& file : std::filesystem::recursive_directory_iterator(".\\archives\\")) {
		if ((!strstr(file.path().string().c_str(), "audio_") && !strstr(file.path().string().c_str(), "disk_") && !strstr(file.path().string().c_str(), "shaders_"))
			&& strstr(file.path().extension().string().c_str(), ".tad")) {
			shendk::TAD tad(file.path().string());

			std::filesystem::path tacpath = file.path();
			tacpath.replace_extension(".tac");

			auto data = tad.readAsset(tacpath.string(), "/tex/assets/textureoverride/sdtextureoverride.json.8f5015c9");
			if (data.size()) {
				imstream f(&data[0], data.size());

				if (f.good()) {
					auto ret = AddMappings(f);
#ifdef _DEBUG
					printf("Added %d mappings from TAD/TAC (%s)\n", ret, tacpath.string().c_str());
#endif
				}
			}
		}
	}
	g_bMappingsInitialized = true;
}

void init_override_patch()
{
	MH_STATUS status = MH_CreateHook(reinterpret_cast<void**>((DWORD_PTR)GetModuleHandle(NULL) + 0x257E90), hook, reinterpret_cast<void**>(&fn_orig));
	if (status == MH_OK)
		status = MH_EnableHook(reinterpret_cast<void**>((DWORD_PTR)GetModuleHandle(NULL) + 0x257E90));

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&init_override_thread, NULL, 0, NULL);
}

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

	// start the update checker thread..
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&update_thread, NULL, 0, NULL);

	// make sure mods directory exists..		
	if (!std::filesystem::exists(LIBFORKLIFT_MODS_DIR)) {
		std::filesystem::create_directories(LIBFORKLIFT_MODS_DIR);
		Sleep(100);
	}

	// initialize ledger
	Ledger::getMods(LIBFORKLIFT_MODS_DIR);

	// install hooks
	HandleCreation::Install();
	FileSize::Install();

	init_override_patch();
}

void Forklift::destroy()
{
	g_bHookEnabled = false;

	ledger.clear();

	HandleCreation::Uninstall();
	FileSize::Uninstall();

#	ifdef SERVER_REPORT_THREAD
#		ifndef _DEBUG
			HasherSmall::Uninstall();
#		endif
#	endif

	MH_Uninitialize();
}