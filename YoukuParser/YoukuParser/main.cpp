#include "stdafx.h"
#include "targetver.h"
#pragma execution_character_set( "utf-8" )
/*
void SetLucidaFont() {
	HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_FONT_INFOEX info;
	memset(&info, 0, sizeof(CONSOLE_FONT_INFOEX));
	info.cbSize = sizeof(CONSOLE_FONT_INFOEX);              // prevents err=87 below
	if (GetCurrentConsoleFontEx(StdOut, FALSE, &info)) {
		info.FontFamily = FF_DONTCARE;
		info.dwFontSize.X = 0;  // leave X as zero
		info.dwFontSize.Y = 14;
		info.FontWeight = 400;
		wcscpy_s(info.FaceName, L"Lucida Console");
		if (SetCurrentConsoleFontEx(StdOut, FALSE, &info)) {
		}
	}
}
//http://i.youku.com/i/UMTYxNDI3NDAzNg==/videos

int main() {
	
	//SetConsoleOutputCP(65001);
	//locale loc("chs");
	//wcout.imbue(loc);
	//std::wcout << L"中国" << endl;
	//cout.imbue(loc);
	//std::cout << "中国" << endl;

	SetLucidaFont();
	//SetConsoleOutputCP(936);



	tcout.imbue(locale("CHS"));

	//wchar_t* pwName = L"测试中";

	tcout << TEXT("浪迹团队") << endl;

	
	//printf("Testing unicode -- English -- Ελληνικά -- Español -- Русский. aäbcdefghijklmnoöpqrsßtuüvwxyz\n");
	//_setmode(_fileno(stdout), _O_U8TEXT);
	//const std::locale utf8_locale = std::locale(std::locale(),
	//new std::codecvt_utf8<wchar_t>());
	httpclient hc;
	tstring html;
	hc.GetHtml(TEXT("http://i.youku.com/i/UMzMxOTI1MzI0NA==/videos"), html);
	//tcout << html << endl;
	vector<pair<tstring, tstring>> videolist;
	parser(html, videolist);
	return 0;
}
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	YoukuParser* p = new YoukuParser(hInstance, hPrevInstance, szCmdLine, iCmdShow);
	int ret = p->run();
	delete p;
	p = nullptr;
	return ret;
}
