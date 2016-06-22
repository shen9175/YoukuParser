#pragma once
class YoukuParser;
class GoButton : public CButtonCtrl {
public:
	GoButton(tstring& url, const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CButtonCtrl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi), URL(url) {}
	~GoButton() {}
	void OnClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {};
private:
	tstring& URL;
};

class ClearButton : public CButtonCtrl {
public:
	ClearButton(tstring& url, const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CButtonCtrl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi), URL(url) {}
	~ClearButton() {}
	void OnClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {};
private:
	tstring& URL;
};

class CheckList : public CListViewCtrl {
public:
	CheckList(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
	void InitialListView();
	void SetCheckState(const int& i, bool fCheck);
	bool GetCheckState(const int& i);
	bool Update(const int& i);
};
class YoukuWindow;
struct PARAMS {
	HWND hwnd;
	tstring* pURL;
	vector<VideoList>* pvideolist;
	size_t index;
	YoukuParser* pYouku;
	CheckList* pchecklist;
	YoukuWindow* pYoukuWin;
};
void ParseThread(PVOID pvoid);
void m3u8Thread(PVOID pvoid);
class YoukuWindow : public CWnd{
public:
	YoukuWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow, YoukuParser* p);
	~YoukuWindow();
	void OnInitWnd();
	void OnCreate(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnNotify(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnSize(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnDestroy(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnCommand(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	//void OnPaint(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {} //do nothing but need to validate the window-->don't leave it blank or the window will never validate
	void OnCtlColorBtn(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnCtlColorStatic(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnSetFocus(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnUserDefined(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void PopulateCheckList(const vector<VideoList>& videolist);
	ConsoleStream& GetConsoleHandle() { return *pconsole; }
	const tstring& GetURL() { return URL; }
	void SetHtmlCookie(const tstring& cookie) { htmlcookie = cookie; }
	const tstring& GetHtmlCookie() { return htmlcookie; }
	const tstring& GetDownloadPath() { return DownloadPath; }
private:
	void OnGoButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnClearButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnCheckListNotify(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnSelectAllButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnInvSelectButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnOKButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnSelectFolderButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnConsoleNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	vector<VideoList> videolist;
	tstring URL;
	tstring htmlcookie;
	tstring DownloadPath;
	YoukuParser* pYouku;
	CEditCtrl* pEdit;
	CEditCtrl* pConsole;
	CEditCtrl* pPath;
	GoButton* pButton;
	ClearButton* pClear;
	CButtonCtrl* pSelectAll;
	CButtonCtrl* pInvSelect;
	CButtonCtrl* pOK;
	CButtonCtrl* pSelectFolder;
	CStaticCtrl* ptitle;
	CStaticCtrl* presult;
	CStaticCtrl* pPathTitle;
	CheckList* pchecklist;
	ConsoleStream* pconsole;
	PARAMS params;
	HGLOBAL hEditDS;
	

};

