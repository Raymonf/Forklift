#pragma once

#include <Windows.h>
#include <filesystem>
#include <string>

class LedgerRecord
{
public:
	LedgerRecord(std::filesystem::directory_entry entry)
		: entry{ entry }, enabled(true)
	{}

	std::string getPath();
	std::filesystem::directory_entry getEntry();

	bool enabled;
private:
	std::filesystem::directory_entry entry;
};
