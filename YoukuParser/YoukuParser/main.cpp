#include "stdafx.h"
#include "targetver.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	YoukuParser* p = new YoukuParser(hInstance, hPrevInstance, szCmdLine, iCmdShow);
	int ret = p->run();
	delete p;
	p = nullptr;
	return ret;
}
