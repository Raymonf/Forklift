#pragma once

#define LIBFORKLIFT_TAG							"v1.1.1"
#define LIBFORKLIFT_MODS_DIR					".\\mods\\"
#define LIBFORKLIFT_UWP_FALLBACK_MODS_DIR		"C:\\Users\\Public\\Documents\\"

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

#include "TextureOverridePatch.h"

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