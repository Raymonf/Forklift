#pragma once

typedef unsigned __int64(__fastcall *getSizeT)(const char *lpFileName);

class FileSize
{
public:
	static void Install();
	static void Uninstall();
private:
	static unsigned __int64 __fastcall Hook(const char *lpFileName);
};
