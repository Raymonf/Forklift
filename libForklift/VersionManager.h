#pragma once
#include <string>

enum class Version {
	// Shenmue 1
	Coconut100,
	Coconut101,
	Coconut102,
	Coconut103,
	Coconut104,
	Coconut105,
	Coconut106,
	Coconut107,
	Coconut_UWP_107,

	// Shenmue 2
	Mango100,
	Mango101,
	Mango102,
	Mango103,
	Mango104,
	Mango105,
	Mango106,
	Mango107,
	Mango_UWP_107,

	Unknown
};

class VersionManager
{
public:
	static VersionManager* singleton();

	__int64 getHandleCreationAddress();
	__int64 getFileSizeAddress();
	__int64 getHasherSmallAddress();

	__int64 getInitOverrideAddress();
	__int64 getOverridesTableAddress();
	__int64 getNumOverridesAddress();

	const std::string getGameId();

	Version getVersion();


	const std::string getModsDir() 
	{
		Version ver = getVersion();
		return std::string(  ver == Version::Coconut_UWP_107 || ver == Version::Mango_UWP_107 ? 
			"C:\\Users\\Public\\Documents\\ShenmueMods\\" + getGameId() + "\\"
			:
			".\\mods\\"
			);
	}

private:
	static VersionManager* sInstance;
	VersionManager();

	Version version;
	__int64 handleCreationAddress;
	__int64 fileSizeAddress;
	__int64 hasherSmallAddress;
	
	__int64 initOverrideAddress;
	__int64 overridesTableAddress;
	__int64 numOverridesAddress;
};

