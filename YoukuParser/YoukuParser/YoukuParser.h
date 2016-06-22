#pragma once

class YoukuParser {
public:
	YoukuParser(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);
	~YoukuParser();
	int WINAPI run() { return pwin->Run(); }
	bool parse(const tstring& URL, vector<VideoList>& videolist);
	tstring getCookie() { return cookie; }
private:
	YoukuWindow* pwin;
	tstring cookie;
};
