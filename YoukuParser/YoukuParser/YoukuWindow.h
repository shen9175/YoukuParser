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
class InputAddress : public CEditCtrl {
public:
	InputAddress(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CEditCtrl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {}
	LRESULT OnSubEditProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
};
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
	//const tstring& GetURL() { return URL; }
	void SetHtmlCookie(const tstring& cookie) { htmlcookie = cookie; }
	void SetOldEditProc(WNDPROC p) { oldeditproc = p; };
	//const tstring& GetHtmlCookie() { return htmlcookie; }
	//const tstring& GetDownloadPath() { return DownloadPath; }
	static LRESULT CALLBACK StaticSubEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	
	LRESULT SubEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OnGoButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnClearButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnCheckListNotify(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnSelectAllButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnInvSelectButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnOKButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnSelectFolderButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void OnConsoleNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void ParseThread();
	void m3u8Thread(const tstring& videoURL, size_t index);
	map< size_t, pair<string, pair<size_t, size_t>>> resolutions;
	unordered_map<string, pair<tstring,tstring>> resolution_choice;
	vector<VideoList> videolist;
	tstring URL;
	tstring htmlcookie;
	tstring DownloadPath;
	tstring password;
	YoukuParser* pYouku;
	InputAddress* pEdit;
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
	CTreeListView* pTreeListView;
	vector<vector<TreeListNodeData*>> download_info;
	HGLOBAL hEditDS;
	mutex mtx;
	WNDPROC oldeditproc;
};
class DownloadInfo {
	DownloadInfo();
	~DownloadInfo();
private:
	vector<vector<TreeListNodeData*>> download_info;
};

class InputPWD : public CDialogBox {
public:
	InputPWD(tstring& pwd, YoukuWindow* p) : password(pwd), pYoukuWindow(p) {}
	void OnCommand(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	bool OnInitial(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
private:
	tstring& password;
	YoukuWindow* pYoukuWindow;
};
