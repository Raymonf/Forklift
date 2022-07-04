#pragma once

extern bool g_bHookEnabled;

typedef signed __int64(__fastcall *handleCreationT)(__int64 a1, char *assetPath, __int64 a3);

// Hook at function that gets a TAC path (or not) relative to a certain asset
class HandleCreation
{
public:
	static void Install();
	static void Uninstall();
private:
	static __int64 __fastcall Hook(__int64 a1, char *assetPath, __int64 a3);
};
