#include "stdafx.h"
#include "Ledger.h"
#include "Utilities.h"
#include "LedgerRecord.h"
#include <algorithm>
#include <vector>
#include <filesystem>

std::vector<LedgerRecord> ledger;

std::vector<LedgerRecord> recordsInDirectory(std::string_view path)
{
	std::vector<LedgerRecord> records;
	for (auto &p : std::filesystem::recursive_directory_iterator(path))
	{
		if (!p.is_directory())
		{
			std::string filePath = p.path().string();
			// We only want the path relative to the path given.
			filePath.erase(0, path.length());
			records.push_back(LedgerRecord(filePath, p.file_size()));
		}
	}
	return records;
}

void Ledger::getMods(std::string_view path)
{
	ledger = recordsInDirectory(path);
}

bool Ledger::isModPath(std::string_view path)
{
	std::string normalized = Utilities::normalizePath(path);
	auto it = std::find_if(ledger.begin(), ledger.end(), [&normalized](LedgerRecord &record) { return Utilities::normalizePath(record.getPath()) == normalized; });
	if (it != ledger.end())
		return true;
	return false;
}

LedgerRecord Ledger::getRecordFromPath(std::string_view path)
{
	std::string normalized = Utilities::normalizePath(path);
	auto it = std::find_if(ledger.begin(), ledger.end(), [&normalized](LedgerRecord &record) { return Utilities::normalizePath(record.getPath()) == normalized; });
	if (it != ledger.end())
		return *it;
}