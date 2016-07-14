#include "stdafx.h"

CWnd::CWnd() {
	szAppName = TEXT("Default Application");
	szCaption = TEXT("Default");
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = RetriveWndProcPointer;
	wndclass.cbClsExtra = 0;
	//wndclass.cbWndExtra = 0;
	wndclass.cbWndExtra = sizeof(CWnd*);//send CWnd::this pointer to LPCREATESTRUCT struct
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);//LoadIcon(nullptr, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);//LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = szAppName.c_str();
	wndclass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));//need load to hInstance
	dwWndStyle = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;
	xPos = CW_USEDEFAULT;
	yPos = CW_USEDEFAULT;
	width = CW_USEDEFAULT;
	height = CW_USEDEFAULT;
	hWndParent = nullptr;
	hMenu = nullptr;
	lpParam = this;//last parameter must be this(Window::this pointer), send Core::this pointer to LPCREATESTRUCT struct
}

CWnd::CWnd(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) : hInstance(hInstance), hPrevInstance(hPrevInstance), szCmdLine(szCmdLine), iCmdShow(iCmdShow) {
	szAppName = TEXT("Default Application");
	szCaption = TEXT("Default");
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = RetriveWndProcPointer;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(CWnd*);//send CWnd::this pointer to LPCREATESTRUCT struct
										//wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);//LoadIcon(nullptr, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);//LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = szAppName.c_str();
	wndclass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));//need load to hInstance
	dwWndStyle = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;
	xPos = CW_USEDEFAULT;
	yPos = CW_USEDEFAULT;
	width = CW_USEDEFAULT;
	height = CW_USEDEFAULT;
	hWndParent = nullptr;
	hMenu = nullptr;
	lpParam = this;//last parameter must be this(CWnd::this pointer), send CWnd::this pointer to LPCREATESTRUCT struct
	cxChar = LOWORD(GetDialogBaseUnits());
	cyChar = HIWORD(GetDialogBaseUnits());
}
CWnd::~CWnd() {
		RemoveProp(hwnd, TEXT("THIS"));
	if (hfont) {
		DeleteObject(hfont);
	}
}

int WINAPI CWnd::Run() {
	OnInitWnd();
	if (!RegisterWnd()) {
		return 0;
	}
	if (!CreateWnd()) {
		return 0;
	};
	//second parameter cannot be hwnd, must be nullptr, or it will not quit properly
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}
bool CWnd::CreateWnd() {
	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, lpParam);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create Window Failed"), 0, 0);
		return false;
	}
	//SetProp(hwnd, TEXT("THIS"), this);
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	return true;
}
bool CWnd::RegisterWnd() {
	if (!RegisterClassEx(&wndclass)) {
		MessageBox(nullptr, TEXT("This program requires Windows NT!"), szAppName.c_str(), MB_ICONERROR);
		return false;
	}
	return true;
}


LRESULT CALLBACK CWnd::RetriveWndProcPointer(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	if (iMsg == WM_NCCREATE) {
		//retrieve "this" pointer which has been passed by CreateWindow() last parameter: "this" pointer is stored in the first member of CREATESTRUCT lpCreateParams
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams));//retrieve CWnd this pointer
		SetProp(hwnd, TEXT("THIS"), reinterpret_cast<CWnd*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams));
		return reinterpret_cast<CWnd*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams)->WndProc(hwnd, iMsg, wParam, lParam);
	} else {
		CWnd* curretnThis = reinterpret_cast<CWnd*>(GetProp(hwnd, TEXT("THIS")));
		//if (!curretnThis) {
		//	curretnThis = reinterpret_cast<CWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		//}
		//CWnd* curretnThis = reinterpret_cast<CWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (!curretnThis) {
			return DefWindowProc(hwnd, iMsg, wParam, lParam);
		} else {
			return curretnThis->WndProc(hwnd, iMsg, wParam, lParam);
		}
	}
}

LRESULT CWnd::WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg) {
		case WM_CREATE: {
			OnCreate(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_SETFOCUS: {
			OnSetFocus(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_NOTIFY: {
			OnNotify(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_COMMAND: {
			OnCommand(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_SIZE: {
			OnSize(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_CTLCOLORBTN: {
			OnCtlColorBtn(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_CTLCOLORSTATIC: {
			OnCtlColorStatic(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_USER: {
			OnUserDefined(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		case WM_PAINT: {
			OnPaint(hwnd, iMsg, wParam, lParam);
			return 0;
		}
		
		case WM_DESTROY: {
			OnDestroy(hwnd, iMsg, wParam, lParam);
			return 0;
		}
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
void CWnd::OnPaint(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	//do nothing but need to validate the window
	DefWindowProc(hwnd, iMsg, wParam, lParam);
	//PAINTSTRUCT ps;
	//HDC hdc = BeginPaint(hwnd, &ps);
	//EndPaint(hwnd, &ps);
}

void CWnd::CMoveWindow(int x, int y, int width, int height, bool repaint) {
	SetStartPosX(x);
	SetStartPosY(y);
	SetWidth(width);
	SetHeight(height);
	MoveWindow(hwnd, x, y, width, height, repaint);
}


bool CWnd::SetFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD   fdwQuality, DWORD fdwPitchAndFamily, LPCTSTR lpszFace) {
	hfont = CreateFont(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
	if (hfont == 0 || hwnd == 0) {
		return false;
	}
	SendMessage(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(hfont), true);
	return true;
}
CControl::CControl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) {
	SetWndCaption(caption);
	SetWndStyle(dwWndStyle);
	SetStartPosX(x);
	SetStartPosY(y);
	SetWidth(width);
	SetHeight(height);
	SetParentHandle(parent);
	SetMenu(hmenu);
	SetInstance(hi);
	lpParam = 0;
}
CEditCtrl::CEditCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CControl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	SetAppName(TEXT("edit")); 
	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, 0);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create Edit Control Failed"), 0, 0);
	}
	//extract the offical defined Edit Control window procedure into OldEditProc with substitution of your own customized StaticEditProc
	OldEditProc =reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CEditCtrl::StaticEditProc)));
	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

LRESULT CALLBACK CEditCtrl::StaticEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	CEditCtrl* current = reinterpret_cast<CEditCtrl*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
	assert(current);
	return current->SubEditProc(wnd, msg, wParam, lParam);
}
LRESULT CEditCtrl::SubEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	//the derived class can override this OnSubEdit function to make customization
	//using default official Edit Control window procedure to handle everything
	return OnSubEditProc(wnd, msg, wParam, lParam);
	
}


CStaticCtrl::CStaticCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CControl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	SetAppName(TEXT("static"));
	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, 0);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create Static Control Failed"), 0, 0);
	}
}

void CButtonCtrl::OnNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (HIWORD(wParam)) {
	case BN_CLICKED: {
		OnClicked(hwnd, iMsg, wParam, lParam);
		break;
	}
	case BN_PAINT: {
		OnPaint(hwnd, iMsg, wParam, lParam);
		break;
	}
	case BN_PUSHED: {
		OnPushed(hwnd, iMsg, wParam, lParam);
		break;
	}
	case BN_UNPUSHED: {
		OnUnPushed(hwnd, iMsg, wParam, lParam);
		break;
	}
	case BN_DISABLE: {
		OnDisable(hwnd, iMsg, wParam, lParam);
		break;
	}
	case BN_DBLCLK: {
		OnDoubleClicked(hwnd, iMsg, wParam, lParam);
		break;
	}
	case BN_SETFOCUS: {
		OnSetFocus(hwnd, iMsg, wParam, lParam);
		break;
	}
	case BN_KILLFOCUS: {
		OnKillFocus(hwnd, iMsg, wParam, lParam);
		break;
	}
	default:
		break;
	}
}
CButtonCtrl::CButtonCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CControl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	SetAppName(TEXT("button"));
	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, 0);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create Button Control Failed"), 0, 0);
	}
}
CTreeView::CTreeView(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CControl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	SetAppName(WC_TREEVIEW);
	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, 0);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create TreeView Control Failed"), 0, 0);
	}
}

CHeader::CHeader(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CControl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	SetAppName(WC_HEADER);
	INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
	icex.dwICC = ICC_LISTVIEW_CLASSES;	//Load list-view and header control classes.
	InitCommonControlsEx(&icex);
	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, 0);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create Header Control Failed"), 0, 0);
	}
}

CListViewCtrl::CListViewCtrl(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CControl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	SetAppName(TEXT("SysListView32"));
	INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, 0);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create ListView Control Failed"), 0, 0);
	}
}




bool CListViewCtrl::AddListViewColumn(const int& iCol, const LVCOLUMN& lvc) {
	if (ListView_InsertColumn(hwnd, iCol, &lvc) == -1) {
		tstring message = TEXT("Add ListView Column ") + to_tstring(iCol) + TEXT(" Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return false;
	}
		
	return true;
}

bool CListViewCtrl::SetListViewColumn(const int& iCol, const LVCOLUMN& lvc) {
	if (!ListView_SetColumn(hwnd, iCol, &lvc)) {
		tstring message = TEXT("Set ListView Column ") + to_tstring(iCol) + TEXT(" Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return false;
	}
	return true;
}

bool CListViewCtrl::GetListViewColumn(const int& iCol, LPLVCOLUMN plvC) {
	if (!ListView_GetColumn(hwnd, iCol, plvC)) {
		tstring message = TEXT("Get ListView Column ") + to_tstring(iCol) + TEXT(" Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return false;
	}
	return true;
}

bool CListViewCtrl::AddListViewItem(const LVITEM& item) {
	if (ListView_InsertItem(hwnd, &item) == -1) {
		tstring message = TEXT("Add ListView Item Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return false;
	}
	return true;
}

void CListViewCtrl::SetItemText(const int& index, const int& subitem,  const tstring& text) {
	ListView_SetItemText(hwnd, index, subitem, const_cast<LPTSTR>(&text[0]));
}

bool CListViewCtrl::ChangeColumnSize(const int& iCol, const int& iSize) {
	if (!ListView_SetColumnWidth(hwnd, iCol, iSize)) {
		tstring message = TEXT("Set ListView Column Width") + to_tstring(iCol) + TEXT(" Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return false;
	}
	return true;
}
bool CListViewCtrl::AutoAdjustColumnSize(const int& iCol) {
	if (!ListView_SetColumnWidth(hwnd, iCol, LVSCW_AUTOSIZE_USEHEADER)) {
		tstring message = TEXT("Auto Adjust ListView Column Width") + to_tstring(iCol) + TEXT(" Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return false;
	}
	return true;
}

INT_PTR  CDialogBox::DoModal(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hwndParent) {
	return DialogBoxParam(hInstance, lpTemplate, hwndParent, static_cast<DLGPROC>(StaticDialogProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CALLBACK CDialogBox::StaticDialogProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	static CDialogBox* current = nullptr;
	if (iMsg == WM_INITDIALOG) {
		current = reinterpret_cast<CDialogBox*>(lParam);
		return current->OnInitial(hwnd, iMsg, wParam, lParam);
	} else {
		return current->DialogProc(hwnd, iMsg, wParam, lParam);
	}
}
bool CDialogBox::DialogProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg) {
		case WM_COMMAND: {
			OnCommand(hwnd, iMsg, wParam, lParam);
			return true;
		}
	}
	return false;
}

CImageList::CImageList(HINSTANCE h, const int& cx, const int& cy, const UINT& flags, const int&  cInitial, const int&  cGrow) : current(0), hinstance(h) {
	if (!(handle = ImageList_Create(cx, cy, flags, cInitial, cGrow))) {
		tstring message = TEXT("ImageList creation Failed!");
		MessageBox(0, message.c_str(), 0, 0);
	}
}
int CImageList::Add(HBITMAP  hbmImage,  HBITMAP    hbmMask) {
	return ImageList_Add(handle, hbmImage, hbmMask);
}
int CImageList::AddFromFile(const tstring&  hbmImage, const tstring& hbmMask = TEXT("")) {
	int ret;
	HBITMAP image = LoadBitmap(hinstance, hbmImage.c_str());
	if (!hbmMask.empty()) {
		HBITMAP mask = LoadBitmap(hinstance, hbmMask.c_str());
		ret = ImageList_Add(handle, image, mask);
		DeleteObject(mask);
	} else {
		ret = ImageList_Add(handle, image, nullptr);
	}
	DeleteObject(image);
	return ret;
}
int CImageList::AddFromResourceID(DWORD  hbmImageID, DWORD hbmMask) {
	int ret;
	HBITMAP image = LoadBitmap(hinstance, MAKEINTRESOURCE(hbmImageID));
	if (!image) {
		tstring message = TEXT("Load Bitmap by ID ") + to_tstring(hbmImageID) + TEXT(" Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return -1;
	}
	if (hbmMask) {
		HBITMAP mask = LoadBitmap(hinstance,MAKEINTRESOURCE(hbmMask));
		if (!mask) {
			tstring message = TEXT("Load Bitmap Mask by ID ") + to_tstring(hbmImageID) + TEXT(" Failed");
			MessageBox(0, message.c_str(), 0, 0);
			return -1;
		}
		ret = ImageList_Add(handle, image, mask);
		if (ret == -1) {
			tstring message = TEXT("ImageList Add Image + Mask Failed");
			MessageBox(0, message.c_str(), 0, 0);
			DeleteObject(mask);
			DeleteObject(image);
			return ret;
		}
		DeleteObject(mask);
	} else {
		ret = ImageList_Add(handle, image, nullptr);
		if (ret == -1) {
			tstring message = TEXT("ImageList Add Image only Failed");
			MessageBox(0, message.c_str(), 0, 0);
			DeleteObject(image);
			return ret;
		}
	}
	DeleteObject(image);
	return ret;
}
bool CImageList::DrawImage(HDC hdc, int x, int y, int width, int height, COLORREF rgbBk, COLORREF rgbFg, UINT style) {
	if (ImageList_DrawEx(handle, current, hdc, x, y, width, height, rgbBk, rgbFg, style)) {
		return true;
	} else {
		return false;
	}
}

bool CImageList::GetImageSize(int& width, int& height) {
	if (ImageList_GetIconSize(handle, &width, &height)) {
		return true;
	} else {
		tstring message = TEXT("Get Image size Failed");
		MessageBox(0, message.c_str(), 0, 0);
		return false;
	}
}
CImageList::~CImageList() {
	if (!ImageList_Destroy(handle)) {
		tstring message = TEXT("ImageList Destroy Failed!");
		MessageBox(0, message.c_str(), 0, 0);
	}
}

CProgressBar::CProgressBar(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CControl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	SetAppName(PROGRESS_CLASS);
	INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
	icex.dwICC = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icex);

	DWORD errorcode;
	hwnd = CreateWindow(szAppName.c_str(), szCaption.c_str(), dwWndStyle, xPos, yPos, width, height, hWndParent, hMenu, hInstance, 0);
	errorcode = GetLastError();
	if (hwnd == nullptr) {
		MessageBox(0, TEXT("Create Progress Bar Control Failed"), 0, 0);
	}
}

DWORD CProgressBar::SetRange(unsigned __int64 low, __int64 high) {
	return static_cast<DWORD>(SendMessage(hwnd, PBM_SETRANGE32, low, high));
}
unsigned __int64 CProgressBar::SetStep(unsigned __int64 step) {
	return SendMessage(hwnd, PBM_SETSTEP, step, 0);
}
unsigned __int64 CProgressBar::StepIt() {
	return SendMessage(hwnd, PBM_STEPIT, 0, 0);
}
unsigned __int64 CProgressBar::SetPos(unsigned __int64 pos) {
	return SendMessage(hwnd, PBM_SETPOS, pos, 0);
}