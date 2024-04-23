#include <Windows.h>
#include "Utilities.h"
#include <iostream>
#include <string>
#include <filesystem>

namespace Utilities {

	bool exists(std::string_view path)
	{
		return std::filesystem::exists(path);
	}
	
	std::string normalizePath(std::string_view path)
	{
		std::string normalized("");

		for (size_t i = 0; i < path.length(); i++)
		{
			auto c = path[i];

			if ((i == 0 || i == 1) && c == '.')
				continue;

			if (c == '/' || c == '\\' || c == ' ')
				continue;

			normalized += tolower(c);
		}

		return normalized;
	}

	std::vector<std::string> pathsInDirectory(std::string_view path)
	{
		std::vector<std::string> paths;
		for (auto &p : std::filesystem::recursive_directory_iterator(path))
		{
			if (!p.is_directory())
			{
				std::string filePath = p.path().string();
				// We only want the path relative to the path given.
				filePath.erase(0, path.length());
				paths.push_back(filePath);
			}
		}
		return paths;
	}

	void setupConsole()
	{
#		ifdef _DEBUG
			// Create Forklift console
			AllocConsole();
			SetConsoleTitleA("Forklift");

			// Redirect standard streams to Forklift's console
			FILE *pNewStdout = nullptr;
			FILE *pNewStderr = nullptr;
			FILE *pNewStdin = nullptr;
			::freopen_s(&pNewStdout, "CONOUT$", "w", stdout);
			::freopen_s(&pNewStderr, "CONOUT$", "w", stderr);
			::freopen_s(&pNewStdin, "CONIN$", "r", stdin);
	#	endif
	}
}
