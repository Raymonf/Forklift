#pragma once

#include <ostream>
#include <sstream>
#include <iostream>
#include <mutex>

class PrintThread : public std::ostringstream
{
public:
	PrintThread();

	~PrintThread();

private:
	static std::mutex _mutexPrint;
};