#include <Windows.h>
#include <string>
#include "VersionManager.h"

VersionManager* VersionManager::sInstance = nullptr;

VersionManager::VersionManager()
{
	version = getVersion();

	switch (version) {
	case Version::Coconut100:
		handleCreationAddress = 0x2900F0;
		fileSizeAddress = 0x290B40;
		hasherSmallAddress = 0x2C6380;
		break;
	case Version::Coconut101:
		handleCreationAddress = 0x28CF50;
		fileSizeAddress = 0x28D9A0;
		hasherSmallAddress = 0x2C31C0;
		break;
	case Version::Coconut102:
		handleCreationAddress = 0x28D6C0;
		fileSizeAddress = 0x28E110;
		hasherSmallAddress = 0x2C3B80;
		break;
	case Version::Coconut103:
		handleCreationAddress = 0x28E3D0;
		fileSizeAddress = 0x28EE20;
		hasherSmallAddress = 0x2C4830;
		break;
	case Version::Coconut104:
		handleCreationAddress = 0x28DA70; // CFA
		fileSizeAddress = 0x28E4C0; // GFEA
		hasherSmallAddress = 0x2C3ED0;
		break;
	case Version::Coconut105:
		handleCreationAddress = 0x28FE00;
		fileSizeAddress = 0x290850;
		hasherSmallAddress = 0x2C4D10;
		break;
	case Version::Coconut106:
		handleCreationAddress = 0x293350;
		fileSizeAddress = 0x293DA0;
		hasherSmallAddress = 0x2C81C0;
		break;
	case Version::Coconut107:
		handleCreationAddress = 0x2932F0;
		fileSizeAddress = 0x293D40;
		hasherSmallAddress = 0x2C8160;
		break;
	case Version::Coconut_UWP_107:
		handleCreationAddress = 0x2A0440;
		fileSizeAddress = 0x2A0FE0;
		hasherSmallAddress = 0x2CFAC0;

		initOverrideAddress = 0x257E90;
		overridesTableAddress = 0xEB15C0;
		numOverridesAddress = 0xEB14E4llu;
		break;

	case Version::Mango100: // Shenmue 2 v1.02 and v1.00 have the same EXEs?
	case Version::Mango102: 
		handleCreationAddress = 0x36D810;
		fileSizeAddress = 0x36E2C0;
		hasherSmallAddress = 0x3CDAE0;
		break;
	case Version::Mango101:
		handleCreationAddress = 0x36D450;
		fileSizeAddress = 0x36DF00;
		hasherSmallAddress = 0x3CD560;
		break;
	case Version::Mango103:
		handleCreationAddress = 0x36DC50; // CFA
		fileSizeAddress = 0x36E700; // GFEA
		hasherSmallAddress = 0x3CE040;
		break;
	case Version::Mango104:
		handleCreationAddress = 0x36DFB0; // CFA
		fileSizeAddress = 0x36EA60; // GFEA
		hasherSmallAddress = 0x3CE3A0;
		break;
	case Version::Mango105:
		handleCreationAddress = 0x3719D0;
		fileSizeAddress = 0x372480;
		hasherSmallAddress = 0x3D4F30;
		break;
	case Version::Mango106:
		handleCreationAddress = 0x371B00;
		fileSizeAddress = 0x3725B0;
		hasherSmallAddress = 0x3D4FC0;
		break;
	case Version::Mango107:
		handleCreationAddress = 0x371AA0;
		fileSizeAddress = 0x372550;
		hasherSmallAddress = 0x3D4F60;
		break;		
	case Version::Mango_UWP_107:
		handleCreationAddress = 0x370D70;
		fileSizeAddress = 0x371990;
		hasherSmallAddress = 0x3D0A20;
		break;
	case Version::Unknown:
	default:
		MessageBoxA(NULL, "Unable to determine the Shenmue game version. Please check for an update.", "libForklift", MB_OK);
		break;
	}
}

VersionManager* VersionManager::singleton()
{
	if (!sInstance)
		sInstance = new VersionManager();

	return sInstance;
}

__int64 VersionManager::getHandleCreationAddress()
{
	return handleCreationAddress;
}

__int64 VersionManager::getFileSizeAddress()
{
	return fileSizeAddress;
}

__int64 VersionManager::getHasherSmallAddress()
{
	return hasherSmallAddress;
}

__int64 VersionManager::getInitOverrideAddress()
{
	return initOverrideAddress;
}

__int64 VersionManager::getOverridesTableAddress()
{
	return overridesTableAddress;
}

__int64 VersionManager::getNumOverridesAddress()
{
	return numOverridesAddress;
}

const std::string VersionManager::getGameId()
{
	switch (version)
	{
	case Version::Coconut100:
	case Version::Coconut101:
	case Version::Coconut102:
	case Version::Coconut103:
	case Version::Coconut104:
	case Version::Coconut105:
	case Version::Coconut106:
	case Version::Coconut107:
	case Version::Coconut_UWP_107:
		return "sm1";
	case Version::Mango100:
	case Version::Mango101:
	case Version::Mango102:
	case Version::Mango103:
	case Version::Mango104:
	case Version::Mango105:
	case Version::Mango106:
	case Version::Mango107:
	case Version::Mango_UWP_107:
		return "sm2";
	}

	return "unknown";
}

Version VersionManager::getVersion()
{
	IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)GetModuleHandle(NULL);
	IMAGE_NT_HEADERS* pNTHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pDOSHeader + pDOSHeader->e_lfanew);
	switch (pNTHeaders->FileHeader.TimeDateStamp)
	{
	case 0x5B6C4B16: // Shenmue 1 v1.02
		return Version::Coconut100;
	case 0x5B7D321A: // Shenmue 1 v1.01
		return Version::Coconut101;
	case 0x5B7FED1B: // Shenmue 1 v1.02
		return Version::Coconut102;
	case 0x5B88872D: // Shenmue 1 v1.03
		return Version::Coconut103;
	case 0x5B91C154: // Shenmue 1 v1.04
		return Version::Coconut104;
	case 0x5BB4988B: // Shenmue 1 v1.05
		return Version::Coconut105;
	case 0x5BD1910A: // Shenmue 1 v1.06
		return Version::Coconut106;
	case 0x5BE4DE30: // Shenmue 1 v1.07
		return Version::Coconut107;

	case 0x5B7FF098: // Shenmue 2 v1.00
		return Version::Mango100;
	case 0x5B7CB4C7: // Shenmue 2 v1.01
		return Version::Mango101;
	case 0x5B8892AE: // Shenmue 2 v1.03
		return Version::Mango103;
	case 0x5B91CC33: // Shenmue 2 v1.04
		return Version::Mango104;
	case 0x5BB4971E: // Shenmue 2 v1.05
		return Version::Mango105;
	case 0x5BD19146: // Shenmue 2 v1.06
		return Version::Mango106;
	case 0x5BE4E77A: // Shenmue 2 v1.07
		return Version::Mango107;	
	
	
	case 0x5CE6F774: // UWP Shenmue v1.07
		return Version::Coconut_UWP_107;
	case 0x5CE6FE26: // UWP Shenmue 2 v1.07
		return Version::Mango_UWP_107;
	default:
		return Version::Unknown;
	}
}