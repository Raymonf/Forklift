#pragma once
#include "HashFile.h"

class ServerReportThread
{
public:
	static void push(HashFile *hashFile);
	static void work();
};