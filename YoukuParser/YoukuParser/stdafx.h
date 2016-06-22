// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <Shobjidl.h>
#include <atlbase.h>
#include <comdef.h>
#include <Wininet.h>
#include <stdio.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <thread>
#include <mutex>

#include <VersionHelpers.h>
#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<vector>
#include<codecvt>
#include<locale>
#include<unordered_map>
#include<unordered_set>
#include<regex>
#include<random>
#include<ctime>
//json
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <limits>
#include <map>
#include <memory>
#include <initializer_list>


template<class Interface>
inline void SafeRelease(
	Interface **ppInterfaceToRelease
	) {
	if (*ppInterfaceToRelease != nullptr) {
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = nullptr;
	}
}


// TODO: reference additional headers your program requires here


using namespace std;


#ifdef UNICODE
	typedef wstring tstring;
	typedef wstringstream tstringstream;
	typedef wostream tostream;
	typedef wregex tregex;
	typedef wsmatch tsmatch;
	typedef wsregex_iterator tregex_iterator;
	typedef wcregex_iterator tcregex_iterator;
	extern wostream tcout;
	tstring to_tstring(const wstring& s);
	
#else
	typedef string tstring;
	typedef stringstream tstringstream;
	typedef ostream tostream;
	typedef regex tregex;
	typedef smatch tsmatch;
	typedef sregex_iterator tregex_iterator;
	typedef cregex_iterator tcregex_iterator;
	extern ostream tcout;
	tstring to_tstring(const wstring& s);
	
#endif

	tstring to_tstring(int value);
	tstring to_tstring(long value);
	tstring to_tstring(long long value);
	tstring to_tstring(unsigned value);
	tstring to_tstring(unsigned long value);
	tstring to_tstring(unsigned long long value);
	tstring to_tstring(float value);
	tstring to_tstring(double value);
	tstring to_tstring(long double value);

	//COM Coding Practices
	//https://msdn.microsoft.com/en-us/library/windows/desktop/ff485839(v=vs.85).aspx
struct VideoList {
	bool bChecked;
	tstring index;
	tstring name;
	tstring url;
};
struct DownloadFactors {
	tstring filename; //final readable video filename;->same with VideoList->name
	vector<tstring> order;//list of partial video file names->has the order
	unordered_map<tstring, vector<tstring>> links; //key is the real unreadable partial file name, values are multiple possible download links->one same file can be saved on different servers
};

#define ID_EDIT 1
#define ID_TITLE 2
#define ID_BUTTON_GO 3
#define ID_RESULT 4
#define ID_BUTTON_CLEAR 5
#define ID_LISTCHECKBOX 6
#define ID_CONSOLE 7
#define ID_PATH 8
#define ID_SELECTALL 9
#define ID_INVSELECT 10
#define ID_OK 11
#define ID_PATHTITLE 12
#define ID_SELECT_FOLDER 13

#include "json11.hpp"
#include "utilities.h"
#include "flv_joiner.h"
using namespace json11;

#include "consolestream.h"
#include "ErrorHandle.h"
#include "http.h"
#include "parser.h"
#include "window.h"
#include "YoukuWindow.h"
#include "YoukuParser.h"
extern CRITICAL_SECTION cs;
extern ConsoleStream* out;


