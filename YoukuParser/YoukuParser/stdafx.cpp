// stdafx.cpp : source file that includes just the standard includes
// InitDisplay.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

CRITICAL_SECTION cs;
ConsoleStream* out;
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#ifdef UNICODE
	wostream tcout{ wcout.rdbuf() };
	tstring to_tstring(const wstring& s) {
		return s;
	}

	tstring to_tstring(int value) { return to_wstring(value); }
	tstring to_tstring(long value) { return to_wstring(value); }
	tstring to_tstring(long long value) { return to_wstring(value); }
	tstring to_tstring(unsigned value) { return to_wstring(value); }
	tstring to_tstring(unsigned long value) { return to_wstring(value); }
	tstring to_tstring(unsigned long long value) { return to_wstring(value); }
	tstring to_tstring(float value) { return to_wstring(value); }
	tstring to_tstring(double value) { return to_wstring(value); }
	tstring to_tstring(long double value) { return to_wstring(value); }
	

#else

	ostream tcout{ cout.rdbuf() };
	tstring to_tstring(const wstring& s) {
		string str;
		str.assign(s.begin(), s.end());
		return str;
	}
	tstring to_tstring(int value) { return to_string(value); }
	tstring to_tstring(long value) { return to_string(value); }
	tstring to_tstring(long long value) { return to_string(value); }
	tstring to_tstring(unsigned value) { return to_string(value); }
	tstring to_tstring(unsigned long value) { return to_string(value); }
	tstring to_tstring(unsigned long long value) { return to_string(value); }
	tstring to_tstring(float value) { return to_string(value); }
	tstring to_tstring(double value) { return to_string(value); }
	tstring to_tstring(long double value) { return to_string(value); }
#endif


