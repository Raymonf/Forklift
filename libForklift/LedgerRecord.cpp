#include "LedgerRecord.h"
#include "Utilities.h"
#include "VersionManager.h"
#include <filesystem>
#include <iostream>

std::string LedgerRecord::getPath()
{
	std::string filePath = entry.path().string();

	// We only want the path relative to the path given.
	filePath.erase(0, VersionManager::singleton()->getModsDir().length());

	return filePath;
}

std::filesystem::directory_entry LedgerRecord::getEntry()
{
	entry.refresh();
	return entry;
}