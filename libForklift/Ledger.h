#pragma once

#include "LedgerRecord.h"
#include <string>
#include <vector>

class Ledger
{
public:
	static void getMods(std::string_view path);
	static bool isModPath(std::string_view path);
	static LedgerRecord getRecordFromPath(std::string_view path);
};

extern std::vector<LedgerRecord> ledger;