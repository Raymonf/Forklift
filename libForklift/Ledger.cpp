#include <Windows.h>
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
	for (auto &p : std::filesystem::recursive_directory_iterator(path))	{
		if (!p.is_directory())
			records.push_back(LedgerRecord(p));
	}
	records.shrink_to_fit();
	return records;
}

void Ledger::getMods(std::string path)
{
	Utilities::curr_path = path;
	ledger = recordsInDirectory(path);
}

bool Ledger::isModPath(std::string_view path)
{
	auto normalized = Utilities::normalizePath(path);
	auto it = std::find_if(ledger.begin(), ledger.end(), [&normalized](LedgerRecord &record) { return record.enabled && (Utilities::normalizePath(record.getPath()) == normalized); });
	if (it != ledger.end())
		return true;
	return false;
}

LedgerRecord Ledger::getRecordFromPath(std::string_view path)
{
	auto normalized = Utilities::normalizePath(path);
	auto it = std::find_if(ledger.begin(), ledger.end(), [&normalized](LedgerRecord& record) { return record.enabled && (Utilities::normalizePath(record.getPath()) == normalized); });
	if (it != ledger.end())
		return *it;
}