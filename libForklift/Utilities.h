#pragma once

#include <Windows.h>
#include <vector>
#include <string>

namespace Utilities {
	// Stores the current "mods" directory path
	extern std::string_view curr_path;

	// Check if something exists in the filesystem
	//bool exists(std::string_view path);

	// Normalize path
	std::string normalizePath(std::string_view path);

	// Return a vector of string with all the file paths in a given directory
	std::vector<std::string> pathsInDirectory(std::string_view path);

#ifdef _DEBUG
	// Create and redirect output to a console
	void setupConsole();
#endif
}
