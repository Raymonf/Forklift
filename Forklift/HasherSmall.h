#pragma once
#include "MinHook.h"

typedef unsigned __int64(__fastcall* tGetHashSmall)(__int64 a1);

class HasherSmall
{
public:
	static void Install();

protected:
	static unsigned __int64 __fastcall Hook(__int64 a1);
};