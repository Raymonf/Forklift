#include "LedgerRecord.h"
#include "Utilities.h"
#include <filesystem>
#include <iostream>

LedgerRecord::LedgerRecord(std::string_view path, __int64 size)
	:path{path}, size{size}
{
}

std::string LedgerRecord::getPath()
{
	return path;
}

__int64 LedgerRecord::getSize()
{
	return size;
}
