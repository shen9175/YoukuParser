#include "stdafx.h"

YoukuParser::YoukuParser(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow){
	pwin = new YoukuWindow(hInstance, hPrevInstance, szCmdLine, iCmdShow, this);
}
YoukuParser::~YoukuParser() {
	if (pwin) {
		delete pwin;
		pwin = nullptr;
	}
}

bool YoukuParser::parse(const tstring& URL, vector<VideoList>& videolist) {
	httpclient hc(pwin->GetConsoleHandle());
	tstring html;
	if (hc.GetHtml(URL, html)) {
		parser(html, videolist);
		cookie = hc.GetHtmlSetCookie();
		pwin->SetHtmlCookie(cookie);
		return true;
	} else {
		return false;
	}
}