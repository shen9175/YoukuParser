#pragma once
class YoukuParser;
class CWnd {
public:
	CWnd();
	CWnd(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);
	~CWnd();
	virtual int WINAPI Run();
	virtual void OnInitWnd() {}
	virtual bool CreateWnd();
	virtual void OnCreate(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnNotify(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnSize(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) { cxClient = LOWORD(lParam);	cyClient = HIWORD(lParam); }
	virtual void OnDestroy(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) { PostQuitMessage(0); }
	virtual void OnCommand(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnPaint(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnSetFocus(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnCtlColorBtn(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnCtlColorStatic(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnUserDefined(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void CSetFocus() { SetFocus(hwnd); }
	virtual void CMoveWindow(int x, int y, int width, int height, bool repaint);
	virtual void CInvalidate(RECT* lpRECT, bool repaint) { InvalidateRect(hwnd, lpRECT, repaint); }
	void SetAppName(const tstring & name) { szAppName = name; wndclass.lpszClassName = szAppName.c_str(); }
	void SetWndCaption(const tstring& caption) { szCaption = caption; }
	void SetMenu(const HMENU& menu) { hMenu = menu; }
	void SetParentHandle(const HWND& parent) { hWndParent = parent; }
	void SetStartPosX(const int& x) { xPos = x; }
	void SetStartPosY(const int& y) { yPos = y; }
	void SetWidth(const int& w) { width = w; }
	void SetHeight(const int& h) { height = h; }
	void SetInstance(const HINSTANCE& hi) { hInstance = hi; wndclass.hInstance = hInstance; }
	void SetPreInstance(const HINSTANCE& phi) { hPrevInstance = phi; }
	void SetCmdLine(const PSTR& cmd) { szCmdLine = cmd; }
	void SetiCmdShow(const int& show) { iCmdShow = show; }
	void SetWndClass(const WNDCLASSEX& wc) { wndclass = wc; }
	void SetWndClassStyle(const UINT& style) { wndclass.style = style; }
	void SetWndStyle(const DWORD& style) { dwWndStyle = style; }
	void SetIcon(const HICON& icon) { wndclass.hIcon = icon; }
	void SetCursor(const HCURSOR& cursor) { wndclass.hCursor = cursor; }
	void SetBackGround(const HBRUSH& brush) { wndclass.hbrBackground = brush; }
	void SetMenuName(const tstring & menu) { wndclass.lpszMenuName = menu.c_str(); }
	void SetIconSmall(const HICON& iconsm) { wndclass.hIconSm = iconsm; }
	void SetcxChar(const int& x) { cxChar = x; }
	void SetcyChar(const int& y) { cyChar = y; }
	void Show() { ShowWindow(hwnd, SW_SHOW); }
	void Hide() { ShowWindow(hwnd, SW_HIDE); }
	bool SetFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD   fdwQuality, DWORD fdwPitchAndFamily, LPCTSTR lpszFace);
protected:



	int cxChar, cyChar, cxClient, cyClient;
	int xPos, yPos;
	int width, height;
	HWND  hwnd;
	DWORD dwExStyle;
	HWND hWndParent;
	HMENU hMenu;
	HINSTANCE    hInstance;
	HINSTANCE hPrevInstance;
	PSTR szCmdLine;
	int iCmdShow;
	WNDCLASSEX  wndclass;
	MSG msg;
	DWORD dwWndStyle;
	HRESULT hr;
	tstring szAppName;
	tstring szCaption;
	void* lpParam;
private:
	static LRESULT CALLBACK RetriveWndProcPointer(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	bool RegisterWnd();
	HFONT hfont;

};

class CControl : public CWnd{
public:
	CControl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
	virtual void OnNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) = 0;
	//virtual void OnInitWnd() = 0;
	//virtual bool CreateWnd() = 0;
	//virtual void OnCreate(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) = 0;
private:

};

class CEditCtrl : public CControl {
public:
	CEditCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
	virtual void OnNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual LRESULT OnSubEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) { return CallWindowProc(OldEditProc, wnd, msg, wParam, lParam); }
private:
	static LRESULT CALLBACK StaticEditProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	LRESULT SubEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	WNDPROC OldEditProc;
};


class CButtonCtrl : public CControl {
public:
	CButtonCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
	virtual void OnNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnPaint(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) { DefWindowProc(hwnd, iMsg, wParam, lParam); }
	virtual void OnPushed(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnUnPushed(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnDisable(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnDoubleClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnSetFocus(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void OnKillFocus(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
private:

};

class CStaticCtrl : public CControl {
public:
	CStaticCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
	virtual void OnNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	//void CSetTextColor() {SetTextColor() }
private:

};


class CCheckBoxCtrl : public CButtonCtrl {
public:
	CCheckBoxCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CButtonCtrl(caption,dwWndStyle,x,y,width,height,parent,hmenu,hi) {}

private:

};

class CListViewCtrl : public CControl {
public:
	CListViewCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
	virtual void OnNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void InitialListView(){}
	bool AddListViewColumn(const int& iCol, const LVCOLUMN& lvc);
	bool SetListViewColumn(const int& iCol, const LVCOLUMN& lvc);
	bool GetListViewColumn(const int& iCol, LPLVCOLUMN plvC);
	bool ChangeColumnSize(const int& iCol, const int& iSize);
	bool AutoAdjustColumnSize(const int& iCol);
	//bool UpdateItem
	bool AddListViewItem(const LVITEM& item);
	void SetItemText(const int& index, const int& subitem, const tstring& text);
	void ClearListViewItem() { SendMessage(hwnd, LVM_DELETEALLITEMS, 0, 0); }
private:


};

class CTreeView : public CControl {
public:
	CTreeView(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);

};

class CHeader : public CControl {
	CHeader(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
};

class CDialogBox {
public:
	CDialogBox() {}
	INT_PTR  DoModal(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hwndParent);
	virtual void OnCommand(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	virtual bool OnInitial(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) { return true; }
private:
	static LRESULT CALLBACK StaticDialogProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	bool DialogProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
};

class CImageList {
public:
	CImageList(HINSTANCE h, const int& cx,const int& cy, const UINT& flags, const int&  cInitial, const int&  cGrow);
	~CImageList();
	int Add(_In_ HBITMAP  hbmImage, _In_opt_ HBITMAP    hbmMask);
	int AddFromFile(const tstring&  hbmImage, const tstring& hbmMask);
	int AddFromResourceID(DWORD  hbmImageID, DWORD hbmMask = 0);
	void SetCurrentImage(int n) { current = n; }
	int GetCurrentImage() { return current; }
	HIMAGELIST GetHandle() { return handle; }
	bool DrawImage(HDC hdc, int x, int y, int width, int height, COLORREF rgbBk, COLORREF rgbFg, UINT style);
	bool GetImageSize(int& width, int& height);
private:
	HIMAGELIST handle;
	HINSTANCE hinstance;
	int current;
};

class CProgressBar : public CControl {
public:
	CProgressBar(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi);
	~CProgressBar();
	virtual void OnNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {}
	DWORD SetRange(unsigned __int64 low, __int64 high);
	unsigned __int64 SetStep(unsigned __int64 step);
	unsigned __int64 SetPos(unsigned __int64 pos);
	unsigned __int64 StepIt();
	void SetTotalAmount(__int64 n) { Total = n; }
	void AddAmount(__int64 n);
	bool GetUpdate() { return update; }
	int GetPercentage() { return prePercentage; }
	Speedometer* GetSpeedOmeter() { return speedo; }
private:
	//__int64 high;
	//__int64 low;
	__int64 Total;
	__int64 current;
	int prePercentage;
	bool update;
	Speedometer* speedo;
	//int counter;
};