#pragma once

#include <string>
#include <filesystem>
#include <process.h>
#include <iostream>
#include <filesystem>

#include "Utilities.h"
#include "HandleCreation.h"
#include "FileSize.h"
#include "LedgerRecord.h"
#include "Ledger.h"

#ifdef SERVER_REPORT_THREAD
#	include "HasherSmall.h"
#	include "ServerReportThread.h"
#endif

#include <vector>

class Forklift {
public:
	static void initialize();
	static void destroy();

	static bool g_bHookEnabled;
};