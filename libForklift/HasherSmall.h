#pragma once

typedef unsigned __int64(__fastcall* tGetHashSmall)(__int64 a1);

class HasherSmall
{
public:
	static void Install();
	static void Uninstall();

private:
	static unsigned __int64 __fastcall Hook(__int64 a1);
};
