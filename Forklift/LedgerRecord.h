#pragma once

#include "stdafx.h"
#include <string>

class LedgerRecord
{
public:
	LedgerRecord(std::string_view path, __int64 size);
	std::string getPath();
	__int64 getSize();
private:
	std::string path;
	__int64 size = 0;
};
