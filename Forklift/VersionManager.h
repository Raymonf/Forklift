#pragma once

enum class Version {
	// Shenmue 1
	Coconut100,
	Coconut101,
	Coconut102,
	Coconut103,
	Coconut104,
	Coconut105,

	// Shenmue 2
	Mango100,
	Mango101,
	Mango102,
	Mango103,
	Mango104,
	Mango105,

	Unknown
};

class VersionManager
{
public:
	static VersionManager* singleton();

	__int64 getHandleCreationAddress();
	__int64 getFileSizeAddress();
	__int64 getHasherSmallAddress();

	const char *getGameId();
private:
	static VersionManager* sInstance;
	VersionManager();

	Version version;
	__int64 handleCreationAddress;
	__int64 fileSizeAddress;
	__int64 hasherSmallAddress;

	Version getVersion();
};

