#include <Windows.h>

#include "MinHook.h"
#include <nlohmann/json.hpp>

#include "VersionManager.h"
#include "TextureOverridePatch.h"

#include <shendk/files/container/tad.h>
#include <shendk/utils/hash_db.h>

using json = nlohmann::json;

typedef void(__fastcall* fn_t)(__int64, int, const char*, __int64);
fn_t fn_orig;

std::vector<TextureOverridePatch::texOverride_t> TextureOverridePatch::mappings;
DWORD_PTR TextureOverridePatch::oldOverridesTbl = 0;
int TextureOverridePatch::oldNumOverrides = 0;
bool TextureOverridePatch::bHasBackedUp = false;

bool TextureOverridePatch::g_bMappingsInitialized = false;

void TextureOverridePatch::Install()
{
	auto addr = VersionManager::singleton()->getInitOverrideAddress();
	MH_STATUS status = MH_CreateHook(reinterpret_cast<void**>((DWORD_PTR)GetModuleHandle(NULL) + addr), hook, reinterpret_cast<void**>(&fn_orig));
	if (status == MH_OK)
	{
		status = MH_EnableHook(reinterpret_cast<void**>((DWORD_PTR)GetModuleHandle(NULL) + addr));

		if (status == MH_OK)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&init_override_thread, NULL, 0, NULL);
		}
	}
}

int TextureOverridePatch::AddMappings(std::istream& f)
{
	int res = 0;
	json data = json::parse(f);
	std::string src, dst;
	texOverride_t tmp;
	for (const auto& items : data["Mappings"]) {
		src = items["Source"].get<std::string>();
		dst = items["Destination"].get<std::string>();

		sprintf_s(tmp.original_path, "%s", src.c_str());
		for (int i = src.size(); i < MAX_PATH; ++i)
			tmp.original_path[i] = 0x00;

		sprintf_s(tmp.remap_path, "%s", dst.c_str());
		for (int i = dst.size(); i < MAX_PATH; ++i)
			tmp.remap_path[i] = 0x00;

		mappings.push_back(tmp);
		mappings.shrink_to_fit();
		res++;
	}
	return res;
}

void TextureOverridePatch::hook(__int64 a1, int a2, const char* a3, __int64 a4)
{
	// @todo: will need changing for UWP maybe
	while (!g_bMappingsInitialized) {}

	fn_orig(a1, a2, a3, a4);

	auto base = GetModuleHandle(NULL);
	auto overridesTableOffset = VersionManager::singleton()->getOverridesTableAddress();
	auto numOverridesOffset = VersionManager::singleton()->getNumOverridesAddress();

	// back up the old table address as this has to be freed manually anyways, 
	// so we can back it up and restore it if the user wants to disable overrides for whatever reason.
	if (!bHasBackedUp) {
		oldOverridesTbl = *(DWORD_PTR*)((DWORD_PTR)(base) + overridesTableOffset);
		oldNumOverrides = *(int*)((DWORD_PTR)base + numOverridesOffset);
		bHasBackedUp = true;
	}

	// allocate new space which can hold the new mappings
	DWORD_PTR mapping_tbl = *(DWORD_PTR*)((DWORD_PTR)(base) + overridesTableOffset);
	size_t size = (MAX_PATH * 2) * mappings.size();
	void* mem = malloc(size);
	if (mem)
	{
		// update the pointer in memory to point to our new space
		memset(mem, 0x00, size);
		*(DWORD_PTR*)((DWORD_PTR)(base) + overridesTableOffset) = (DWORD_PTR)mem;

		// write all of the mappings into the new space
		int index = 0;
		for (auto& mapping : mappings)
		{
			memcpy(((char*)mem + ((MAX_PATH * 2) * index)), &mapping, sizeof texOverride_t);
			++index;
		}

		// finally update the total count of mappings!
		*(int*)((DWORD_PTR)base + numOverridesOffset) = index;
	}
}

void TextureOverridePatch::init_override_thread()
{
	shendk::HashDB& db = shendk::HashDB::getInstance();
	db.initialize("filename_database.json");

	for (const auto& file : std::filesystem::recursive_directory_iterator(".\\archives\\")) {
		std::string str = file.path().string();

		bool bNotValid = (!strstr(str.c_str(), "audio_") && !strstr(str.c_str(), "disk_") && !strstr(str.c_str(), "shaders_"));
		bool bIsTAD = strstr(file.path().extension().string().c_str(), ".tad");
		if (bNotValid && bIsTAD) {
			shendk::TAD tad(str);

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

	g_bMappingsInitialized = true;
}