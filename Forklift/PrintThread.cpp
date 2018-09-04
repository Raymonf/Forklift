#pragma once
#include "stdafx.h"
#include "PrintThread.h"

std::mutex PrintThread::_mutexPrint{};

PrintThread::PrintThread() = default;

PrintThread::~PrintThread()
{
	std::lock_guard<std::mutex> guard(_mutexPrint);
	std::cout << this->str();
}