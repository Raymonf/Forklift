#pragma once


class TextureOverridePatch {
public:
	TextureOverridePatch() = default;
	~TextureOverridePatch() = default;

	struct texOverride_t {
		char original_path[MAX_PATH];
		char remap_path[MAX_PATH];

		texOverride_t()
		{
			memset(original_path, 0x00, MAX_PATH);
			memset(remap_path, 0x00, MAX_PATH);
		}
	};

	static std::vector<texOverride_t> mappings;
	static DWORD_PTR oldOverridesTbl;
	static int oldNumOverrides;
	static bool bHasBackedUp;

	static bool g_bMappingsInitialized;

	static void Initialize();
	static void init_override_thread();
	static void hook(__int64 a1, int a2, const char* a3, __int64 a4);
	static int AddMappings(std::istream& f);

};