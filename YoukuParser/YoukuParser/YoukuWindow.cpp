#include "stdafx.h"



YoukuWindow::YoukuWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow, YoukuParser* p) : CWnd(hInstance, hPrevInstance, szCmdLine, iCmdShow), pYouku(p) {

	DownloadPath = tstring();
	DWORD len = GetCurrentDirectory(0, &DownloadPath[0]);
	DownloadPath.resize(len);
	len = GetCurrentDirectory(len, &DownloadPath[0]);
	DWORD errcode;
	if (len == 0) {
		errcode = GetLastError();
		hr = HRESULT_FROM_WIN32(errcode);
	}
	_com_error err(hr);
	//OK and Cancell does not show the err message, the other cases shows
	if (hr != S_OK && hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(hwnd, errMsg, TEXT("Get Current Dirctory Failed!"), MB_OK);
	}


	resolution_choice["mp4hd3"] = { TEXT("hd3"),TEXT("全高清1080P") };
	resolution_choice["hd3"] = { TEXT("hd3"),TEXT("全高清1080P") };

	resolution_choice["mp4hd2"] = { TEXT("hd2"),TEXT("超清") };
	resolution_choice["hd2"] = { TEXT("hd2"),TEXT("超清") };

	resolution_choice["mp4hd"] = { TEXT("mp4"),TEXT("高清") };
	resolution_choice["mp4"] = { TEXT("mp4"),TEXT("高清") };

	resolution_choice["flvhd"] = { TEXT("flvhd"),TEXT("标清") };
	resolution_choice["flv"] = { TEXT("flv"),TEXT("标清") };
	resolution_choice["3gphd"] = { TEXT("3gphd"),TEXT("标清") };
}
YoukuWindow::~YoukuWindow() {
	if (pEdit) {
		delete pEdit;
		pEdit = nullptr;
	}
	if (pConsole) {
		delete pConsole;
		pConsole = nullptr;
	}
	if (pPath) {
		delete pPath;
		pPath = nullptr;
	}
	if (pButton) {
		delete pButton;
		pButton = nullptr;
	}
	if (pClear) {
		delete pClear;
		pClear = nullptr;
	}
	if (pSelectAll) {
		delete pSelectAll;
		pSelectAll = nullptr;
	}
	if (pInvSelect) {
		delete pInvSelect;
		pInvSelect = nullptr;
	}
	if (pSelectFolder) {
		delete pSelectFolder;
		pSelectFolder = nullptr;
	}
	if (pOK) {
		delete pOK;
		pOK = nullptr;
	}
	if (ptitle) {
		delete ptitle;
		ptitle = nullptr;
	}
	if (presult) {
		delete presult;
		presult = nullptr;
	}
	if (pPathTitle) {
		delete pPathTitle;
		pPathTitle = nullptr;
	}
	if (pchecklist) {
		delete pchecklist;
		pchecklist = nullptr;
	}
	if (pconsole) {
		delete pconsole;
		pconsole = nullptr;
	}
	GlobalFree(hEditDS);
}
void YoukuWindow::OnInitWnd() {
	SetAppName(TEXT("Youku Parser"));
	SetWndCaption(TEXT("Youku Video Parser and Downloader"));
	SetWndClassStyle(CS_HREDRAW | CS_VREDRAW);
	SetWndStyle(WS_OVERLAPPEDWINDOW);
}
void YoukuWindow::OnCreate(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	pEdit = new InputAddress(TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_EDIT), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pPath = new CEditCtrl(TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_PATH), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	
	hEditDS = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, 256L);
	pConsole = new CEditCtrl(TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL| ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_CONSOLE), reinterpret_cast<HINSTANCE>(hEditDS));
	SendMessage(GetDlgItem(hwnd, ID_CONSOLE), EM_LIMITTEXT, 0, 0L);

	pButton = new GoButton(URL, TEXT("Go"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_BUTTON_GO), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pClear = new ClearButton(URL, TEXT("Clear"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_BUTTON_CLEAR), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pSelectAll = new CButtonCtrl(TEXT("Select All"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_SELECTALL), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pInvSelect = new CButtonCtrl(TEXT("Inverse Select"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_INVSELECT), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pSelectFolder = new CButtonCtrl(TEXT("..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_SELECT_FOLDER), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pOK = new CButtonCtrl(TEXT("Download"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_OK), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	ptitle = new CStaticCtrl(TEXT("Please enter the URL into following box:"), WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_TITLE), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pPathTitle = new CStaticCtrl(TEXT("Download Path:"), WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_PATHTITLE), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	presult = new CStaticCtrl(TEXT(""), WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_RESULT), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pchecklist = new CheckList(TEXT("Result"), WS_CHILD | WS_BORDER | LVS_REPORT , 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_LISTCHECKBOX), reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
	pconsole = new ConsoleStream(GetDlgItem(hwnd, ID_CONSOLE));
	out = pconsole;
	SetWindowText(GetDlgItem(hwnd, ID_PATH), DownloadPath.c_str());
	pEdit->CSetFocus();
}//  
void YoukuWindow::OnSize(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	CWnd::OnSize(hwnd, iMsg, wParam, lParam);
	pEdit->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2, cyClient / 16, 3 * cxClient / 4, static_cast<int>(1.5 * cyChar), true);
	pButton->CMoveWindow(7 * cxClient / 8 - 5 * cxChar, cyClient / 16, 4 * cxChar, static_cast<int>(1.5 * cyChar), true);
	pClear->CMoveWindow(7 * cxClient / 8 + 0 * cxChar, cyClient / 16, 7 * cxChar, static_cast<int>(1.5 * cyChar), true);
	ptitle->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2, cyClient / 16 - static_cast<int>(1.5 * cyChar), 45 * cxChar, static_cast<int>(1.5 * cyChar), true);
	presult->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2, cyClient / 16 + 2 * cyChar, 3 * cxClient / 4, static_cast<int>(1.5 * cyChar), true);
	pchecklist->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2, cyClient / 16 + 4 * cyChar, 3 * cxClient / 4 + 13 * cxChar, 2 * cyClient / 3, true);
	pConsole->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2, cyClient / 16 + 4 * cyChar, 3 * cxClient / 4 + 13 * cxChar, 2 * cyClient / 3, true);
	pSelectAll->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2, cyClient / 16 + 4 * cyChar + 2 * cyClient / 3 + 2 * cyChar, 10 * cxChar, static_cast<int>(1.5 * cyChar), true);
	pInvSelect->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2 + 12 * cxChar, cyClient / 16 + 4 * cyChar + 2 * cyClient / 3 + 2 * cyChar, 14 * cxChar, static_cast<int>(1.5 * cyChar), true);
	pSelectFolder->CMoveWindow(7 * cxClient / 8 - 7 * cxChar, cyClient / 16 + 4 * cyChar + 2 * cyClient / 3 + 2 * cyChar, 3 * cxChar, static_cast<int>(1.5 * cyChar),true);
	pPathTitle->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2 + 28 * cxChar, cyClient / 16 + 4 * cyChar + 2 * cyClient / 3 + 2 * cyChar, 14 * cxChar, static_cast<int>(1.5 * cyChar), true);
	pPath->CMoveWindow((cxClient / 4 - 12 * cxChar) / 2 + 42 * cxChar, cyClient / 16 + 4 * cyChar + 2 * cyClient / 3 + 2 * cyChar, 7 * cxClient / 8 - 7 * cxChar - ((cxClient / 4 - 12 * cxChar) / 2 + 42 * cxChar), static_cast<int>(1.5 * cyChar), true);
	pOK->CMoveWindow(7 * cxClient / 8 - 3 * cxChar, cyClient / 16 + 4 * cyChar + 2 * cyClient / 3 + 2 * cyChar, 10 * cxChar, static_cast<int>(1.5 * cyChar), true);
	
	//pchecklist->ChangeColumnSize(0, 3 * cxClient / 8);
	//pchecklist->ChangeColumnSize(1, 3 * cxClient / 8);
	pchecklist->AutoAdjustColumnSize(0);
	pchecklist->AutoAdjustColumnSize(1);
	pchecklist->AutoAdjustColumnSize(2);
}
void YoukuWindow::OnDestroy(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	CWnd::OnDestroy(hwnd, iMsg, wParam, lParam);
}
void YoukuWindow::OnCommand(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(wParam)) {
		case ID_EDIT: {
			break;
		}
		case ID_BUTTON_GO: {
			OnGoButtonClicked(hwnd, iMsg, wParam, lParam);
			break;
		}
		case ID_BUTTON_CLEAR: {
			OnClearButtonClicked(hwnd, iMsg, wParam, lParam);
			break;
		}
		case ID_TITLE: {
			break;
		}
		case ID_RESULT: {
			break;
		}
		case ID_SELECTALL: {
			OnSelectAllButtonClicked(hwnd, iMsg, wParam, lParam);
			break;
		}
		case ID_INVSELECT: {
			OnInvSelectButtonClicked(hwnd, iMsg, wParam, lParam);
			break;
		}
		case ID_SELECT_FOLDER: {
			OnSelectFolderButtonClicked(hwnd, iMsg, wParam, lParam);
			break;
		}
		case ID_OK: {
			OnOKButtonClicked(hwnd, iMsg, wParam, lParam);
			break;
		}
		case ID_CONSOLE: {
			OnConsoleNotification(hwnd, iMsg, wParam, lParam);
			break;
		}
		default:
			break;
	}
}
void YoukuWindow::OnConsoleNotification(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	if (HIWORD(wParam) == EN_ERRSPACE || HIWORD(wParam) == EN_MAXTEXT) {
		//MessageBox(hwnd, TEXT("Edit control out of space!"), TEXT("Warning!"),MB_OK|MB_ICONSTOP);
	}
}
void YoukuWindow::OnSetFocus(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	pEdit->CSetFocus();
}
void YoukuWindow::OnCtlColorBtn(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

}
void YoukuWindow::OnCtlColorStatic(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
}
void YoukuWindow::OnNotify(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(wParam)) {
	case ID_LISTCHECKBOX: {
		OnCheckListNotify(hwnd, iMsg, wParam, lParam);
		break;
	}
	default:
		DefWindowProc(hwnd, iMsg, wParam, lParam);
		break;
	}
}
void YoukuWindow::OnCheckListNotify(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	NMLVDISPINFO* plvdi;
	switch (reinterpret_cast<LPNMHDR>(lParam)->code) { //or switch (reinterpret_cast<NMLVDISPINFO*>(lParam)->hdr.code) it's the same thing
		case LVN_GETDISPINFO: {
			plvdi = reinterpret_cast<NMLVDISPINFO*>(lParam);
			switch (plvdi->item.iSubItem) {
				case 0:
					//plvdi->item.pszText = rgPetInfo[plvdi->item.iItem].szKind;
					break;
				case 1:
					//plvdi->item.pszText = rgPetInfo[plvdi->item.iItem].szBreed;
					break;
				case 2:
					//plvdi->item.pszText = rgPetInfo[plvdi->item.iItem].szPrice;
					break;
				default:
					break;
			}
			break;
		}
		case NM_CLICK: {
			LPNMITEMACTIVATE pnmitemativate = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
			//bool state = pchecklist->GetCheckState(pnmitemativate->iItem);
			//int item = pnmitemativate->iItem;
			//state = !state;
			//pchecklist->SetCheckState(item, true);
			//videolist[item].bChecked = state;

			//checkbox wil toggole the check state itself, no need to SetCheckState
			
			//pchecklist->SetCheckState(pnmitemativate->iItem, !pchecklist->GetCheckState(pnmitemativate->iItem));
			//videolist[pnmitemativate->iItem].bChecked = pchecklist->GetCheckState(pnmitemativate->iItem);
			videolist[pnmitemativate->iItem].bChecked = !videolist[pnmitemativate->iItem].bChecked;
			pchecklist->Update(pnmitemativate->iItem);
			break;
		}
		default:
			DefWindowProc(hwnd, iMsg, wParam, lParam);
			break;
	}
}
void InputPWD::OnCommand(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	password = TEXT("");
	TCHAR buffer[80];
	switch (LOWORD(wParam)) {
		case ID_PWD_INPUT_OK:
			//GetDlgItemText(hwnd, IDC_EDIT_PWD_INPUT, &password[0], 80);
			GetDlgItemText(hwnd, IDC_EDIT_PWD_INPUT, buffer, 80);
			//password = buffer;
		case ID_PWD_INPUT_CANCEL:
		case IDCANCEL:
			EndDialog(hwnd, wParam);
			//?? as long as hit ENTER to access case ID_PWD_INPUT_OK, after EndDialog, password will be empty. But Mouse Click OK to access ID_PWD_INPUT_OK, after EndDialog, password has the input.
			password = buffer;
	}
	
}
bool InputPWD::OnInitial(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	SetFocus(GetDlgItem(hwnd, IDC_EDIT_PWD_INPUT));
	HWND pwd_edit_hwnd = GetDlgItem(hwnd, IDC_EDIT_PWD_INPUT);
	pYoukuWindow->SetOldEditProc(reinterpret_cast<WNDPROC>(SetWindowLongPtr(GetDlgItem(hwnd, IDC_EDIT_PWD_INPUT), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(YoukuWindow::StaticSubEditProc))));
	SendMessage(pwd_edit_hwnd, WM_USER, 0, reinterpret_cast<LPARAM>(pYoukuWindow));
	return false;
}

LRESULT InputAddress::OnSubEditProc(HWND hedit, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
			case VK_RETURN:
				HWND parent = GetParent(hedit);
				SendMessage(GetParent(hedit), WM_COMMAND, ID_BUTTON_GO, 0);
				return true;
		}
	}
	return CEditCtrl::OnSubEditProc(hedit, iMsg, wParam, lParam);
}

LRESULT CALLBACK YoukuWindow::StaticSubEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static YoukuWindow* current;
	if (msg == WM_USER) {
		current = reinterpret_cast<YoukuWindow*>(lParam);
		return true;
	} else {
		return current->SubEditProc(wnd, msg, wParam, lParam);
	}
}
LRESULT YoukuWindow::SubEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_KEYDOWN) {
		if (wParam == VK_RETURN) {
			SendMessage(GetParent(wnd), WM_COMMAND, ID_PWD_INPUT_OK, 0);
			return 0;
		}
	}
	return CallWindowProc(oldeditproc, wnd, msg, wParam, lParam);
}
void YoukuWindow::OnUserDefined(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	if (lParam) {
		ShowWindow(GetDlgItem(hwnd, ID_CONSOLE), SW_HIDE);
		ShowWindow(GetDlgItem(hwnd, ID_LISTCHECKBOX), SW_SHOW);
	}
}

void YoukuWindow::OnGoButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	ShowWindow(GetDlgItem(hwnd, ID_CONSOLE), SW_SHOW);
	ShowWindow(GetDlgItem(hwnd, ID_LISTCHECKBOX), SW_HIDE);
	HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT);
	size_t length = SendMessage(hwndEdit, EM_LINELENGTH, 0, 0);
	URL.resize(length);
	// Before sending the message, set the first word of this buffer to the size, in TCHARs, of the buffer. For ANSI text, this is the number of bytes; 
	//for Unicode text, this is the number of characters. The size in the first word is overwritten by the copied line.
	reinterpret_cast<BYTE*>(&URL[0])[0] = static_cast<BYTE>(0x00ff & length);
	reinterpret_cast<BYTE*>(&URL[0])[1] = static_cast<BYTE>((0xff00 & length) >> 2);
	SendMessage(hwndEdit, EM_GETLINE, 0, reinterpret_cast<LPARAM>(&URL[0]));
	HWND hwndresult = GetDlgItem(hwnd, ID_RESULT);
	SetWindowText(hwndresult, &URL[0]);
	thread parse_thread(&YoukuWindow::ParseThread, this);
	parse_thread.detach();
}
void YoukuWindow::ParseThread() {
	mtx.lock();
	videolist.clear();
	pchecklist->ClearListViewItem();
	if (pYouku->parse(URL, videolist)) {
		PopulateCheckList(videolist);
		SendMessage(hwnd, WM_USER, 0, true);
	} else {
		SendMessage(hwnd, WM_USER, 0, false);
	}
	mtx.unlock();
}

void YoukuWindow::OnOKButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	ShowWindow(GetDlgItem(hwnd, ID_CONSOLE), SW_SHOW);
	ShowWindow(GetDlgItem(hwnd, ID_LISTCHECKBOX), SW_HIDE);
	for (size_t i = 0; i < videolist.size(); ++i) {
		if (videolist[i].bChecked) {
			thread m3u8thread(&YoukuWindow::m3u8Thread, this, videolist[i].url, i);
				m3u8thread.detach();
		}
	}
}

void YoukuWindow::m3u8Thread(const tstring& videoURL, size_t index) {
	mtx.lock();
	tstring vid = getVid(videoURL);
	tstring JSONurl = getJSONUrl(vid);
	httpclient hc(*pconsole);
	tstring htmlsetcookie = htmlcookie;
	*pconsole << TEXT("htmlSetCookie = ") << htmlsetcookie << endl;
	//tstring setcookie = hc.GetSetCookie(videoURL);
	//*pconsole << TEXT("SetCookie = ") << setcookie << endl;
	//tstring cookie = TEXT("Cookie: ") /*+ htmlsetcookie + TEXT(",")*/ + setcookie + TEXT(" __ysuid=") + GetPvid(6) + TEXT("\r\n") + TEXT("Referer: ") + videoURL + TEXT("\r\n");
	//tstring cookie = TEXT("Cookie: ") + htmlsetcookie + TEXT(" __ysuid=") + GetPvid(6) + TEXT("\r\n") + TEXT("Referer: ") + videoURL + TEXT("\r\n"); ->no referer need if you put referer in GetJson()
	tstring cookie = TEXT("Cookie: ") + htmlsetcookie + TEXT(" __ysuid=") + GetPvid(6) + TEXT("\r\n");
	*pconsole << TEXT("Combined Cookie = ") << cookie << endl;
	tstring referer = videoURL;
	tstring JSON;
	*pconsole << endl << TEXT("Downloading JSON file...") << endl;
	int tries = 20;
	while (tries > 0) {
		if (hc.GetJson(JSONurl, JSON, referer, cookie)) {
			*pconsole << endl << TEXT("Finishing downloading JSON file") << endl;
			*pconsole << TEXT("JSON file content is: ") << endl;
			*pconsole << JSON << endl;
			break;
		} else {
			*pconsole << TEXT("Get JSON Failed!") << endl;
			*pconsole << TEXT("Try Get JSON Again...") << endl;
			--tries;
		}
	}
	if (tries <= 0) {
		*pconsole << TEXT("Get JSON Failed for 20 times, give up for the video!") << endl;
		mtx.unlock();
		return;
	}
	string err;
	string json;
	tstring m3u8;
#ifdef UNICODE
	json = WideToByte(CP_UTF8, JSON);
#else
	json = JSON;
#endif
	auto JSONobj = Json::parse(json, err);
	auto data = JSONobj["data"].object_items();
	if (data["stream"].is_null()) {
		if (!data["error"].is_null()) {
			if (static_cast<int>(data["error"]["code"].number_value()) == -202) {
				bool wrongpwd = true;
				while (wrongpwd) {
					InputPWD pwd(password, this);
					if (pwd.DoModal(reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE)), MAKEINTRESOURCE(IDD_PWD_INPUT_DIALOG), hwnd) == ID_PWD_INPUT_OK) {
						*pconsole << TEXT("Input password is ") << password << endl;;
					} else {
						*pconsole << TEXT("No password, the video will be skipped!") << endl;
						mtx.unlock();
						return;
					}
					JSONurl += TEXT("&pwd=") + password;
					*pconsole << endl << TEXT("New JSON URL with password is:") << endl;
					*pconsole << JSONurl << endl;
					*pconsole << endl << TEXT("Downloading JSON again file with password...") << endl;
					tries = 20;
					JSON.clear();
					while (tries > 0) {
						if (hc.GetJson(JSONurl, JSON, referer, cookie)) {
							*pconsole << endl << TEXT("Finishing downloading JSON file") << endl;
							*pconsole << TEXT("JSON file content is: ") << endl;
							*pconsole << JSON << endl;
#ifdef UNICODE
							json = WideToByte(CP_UTF8, JSON);
#else
							json = JSON;
#endif
							JSONobj = Json::parse(json, err);
							data = JSONobj["data"].object_items();
							if (data["stream"].is_null()) {
								*pconsole << TEXT("Failed: Wrong Password!") << endl;
							} else {
								wrongpwd = false;
							}
							break;
						} else {
							*pconsole << TEXT("Get JSON Failed!") << endl;
							*pconsole << TEXT("Try Get JSON Again...") << endl;
							--tries;
						}
					}
					if (tries <= 0) {
						*pconsole << TEXT("Get JSON Failed for 20 times, give up for the video!") << endl;
						mtx.unlock();
						return;
					}
				}
			} else {
				*pconsole << TEXT("Failed: ") << ByteToWide(CP_UTF8, data["error"]["note"].string_value()) << endl;
				mtx.unlock();
				return;
			}
		} else {
			*pconsole << TEXT("Failed: video not found!") << endl;
			mtx.unlock();
			return;
		}
	}
	auto security = data["security"].object_items();
	if (!security.empty()) {
		*pconsole << endl << TEXT("Extracting video resolutions from JSON file...") << endl;
		auto stream_array = data["stream"].array_items();
		for (size_t i = 0; i < stream_array.size(); ++i) {
			auto stream_type = stream_array[i]["stream_type"].string_value();
			auto height = stream_array[i]["height"].number_value();
			auto width = stream_array[i]["width"].number_value();
			if (resolutions.find(static_cast<size_t>(height * width)) == resolutions.cend()) {
				resolutions[static_cast<size_t>(height * width)] = { stream_type,{ static_cast<size_t>(width), static_cast<size_t>(height)} };
			} else {
				//print out ignore resolution and stream type
				*pconsole << TEXT("Ignore already exsiting resolution: ") << ByteToWide(CP_UTF8, stream_type) << TEXT(": ") << static_cast<int>(width) << TEXT(" x ") << static_cast<int>(height) << TEXT(" ") << resolution_choice.at(stream_type).second << endl;
			}
		}
		*pconsole << endl << TEXT("There are following resolutions in this video:") << endl;
		for (auto item : resolutions) {
			*pconsole << TEXT("Stream type: ") << ByteToWide(CP_UTF8, item.second.first) << TEXT(": ") << static_cast<int>(item.second.second.first) << TEXT(" x ") << static_cast<int>(item.second.second.second) << TEXT(" ") << resolution_choice.at(item.second.first).second <<endl;
		}
		*pconsole << endl << TEXT("The program automatically choose the highest resolution is:") << endl;
		*pconsole << TEXT("Stream type: ") << ByteToWide(CP_UTF8, (*resolutions.crbegin()).second.first) << TEXT(": ") << static_cast<int>((*resolutions.crbegin()).second.second.first) << TEXT(" x ") << static_cast<int>((*resolutions.crbegin()).second.second.second) << TEXT(" ") << resolution_choice.at((*resolutions.crbegin()).second.first).second << endl;
		*pconsole << endl << TEXT("Finishing extracting video resolutions from JSON file!") << endl;

		*pconsole << endl << TEXT("Extracting \"encrypt_string\" and \"ip\" from JSON file...") << endl;
		tstring ep, token, sid;
		tstring security_string;
		tstring ip;
		string secstr = security["encrypt_string"].string_value();
		auto oip = security["ip"].int_value();
#ifdef UNICODE
		security_string = ByteToWide(CP_UTF8, secstr);
#else
		security_string = secstr;
#endif
		ip = to_tstring(oip);
		*pconsole << TEXT("Finishing extracting \"encrypt_string\" and \"ip\" from JSON file!") << endl;
		*pconsole << endl << TEXT("Decrypting and synthething m3u8 address link...") << endl;
		GetParameters(vid, security_string, sid, token, ep);
		*pconsole << TEXT("Finishing decrypting and synthething m3u8 address link...") << endl;
		*pconsole << TEXT("M3U8 file address link is: ") << endl << endl;
		m3u8 = TEXT("http://pl.youku.com/playlist/m3u8?vid=") + vid + TEXT("&type=") + resolution_choice.at((*resolutions.crbegin()).second.first).first +TEXT("&ts=") + to_tstring(time(nullptr)) + TEXT("&keyframe=1&ep=") + ep + TEXT("&sid=") + sid + TEXT("&token=") + token + TEXT("&ctype=12&ev=1&oip=") + ip + (password.empty() ? TEXT("") : TEXT("&password=") + password);
		*pconsole << m3u8 << endl;
		*pconsole << endl << TEXT("Getting video file true downloading links container: m3u8 file...") << endl;
		tstring m3u8file;
		hc.GetM3U8(m3u8, m3u8file);
		*pconsole << endl << TEXT("Finished downloading video file true downloading links container: m3u8 file.") << endl;
		*pconsole << endl << TEXT("The content of m3u8 file is:") << endl;
		*pconsole << m3u8file << endl;

		*pconsole << endl << TEXT("Parsing m3u8 file...") << endl;


		DownloadFactors videolinks;
		videolinks.filename = videolist[index].name;


		M3U8Parser(m3u8file, videolinks);
		*pconsole << endl << TEXT("Finish parsing m3u8 file!") << endl;
		if (videolinks.links.empty()) {
			*pconsole << endl << TEXT("No video file parsed out!") << endl;
			mtx.unlock();
			return;
		}
		*pconsole << endl << TEXT("Final true video downloading links are:") << endl << endl;

		*pconsole << TEXT("video name: ") << videolinks.filename << endl << endl;

		for (auto item : videolinks.order) {
			*pconsole << TEXT("video components filename: ") << endl << item << endl;
			if (videolinks.links.find(item) != videolinks.links.cend()) {
				*pconsole << TEXT("download links:") << endl;
				for (auto link : videolinks.links.at(item)) {
					*pconsole << link << endl << endl;
				}
			} else {
				*pconsole << TEXT("error: there is no download link for ") << item << endl;
			}
		}
		tstring originalpath = DownloadPath;
		if (originalpath.back() == TEXT('\0')) {
			originalpath.pop_back();
		}
		tstring finalpath = originalpath + TEXT("\\") + videolinks.filename;
		*pconsole << TEXT("Creating downloading temp directory: ") << finalpath << endl;
		if (!CreateDirectory(finalpath.c_str(), nullptr)) {
			DWORD errorcode = GetLastError();
			*pconsole << TEXT("Create temp directory failed!") << endl;
			InternetErrorOut(*pconsole, errorcode, TEXT("CreateDirectory"));
		} else {
			string whole;
			vector<string> videofilelist;
			for (auto item : videolinks.order) {
				if (videolinks.links.find(item) != videolinks.links.cend()) {
					auto link = videolinks.links.at(item)[0];
					string videobuffer;
					if (!hc.GetVideo(link, videobuffer)) {
						mtx.unlock();
						return;
					}
					videofilelist.push_back(videobuffer);
					ofstream out;
					tstring finalname = finalpath + TEXT("\\") + item;
					*pconsole << TEXT("Final name = ") << finalname << endl;
					out.open(finalname.c_str(), ofstream::binary);
					if (!out.fail()) {
						out << videobuffer;
						out.close();
					} else {
						*pconsole << TEXT("create/open file failed!") << endl;
						out.close();
					}
					//}

				} else {
					*pconsole << TEXT("error: there is no download link for ") << item << endl;
				}
			}
			concat_flv(videofilelist, whole);
			ofstream out;
			tstring finalname = finalpath + TEXT(".flv");
			*pconsole << TEXT("Combined File name = ") << finalname << endl;
			out.open(finalname.c_str(), ofstream::binary);
			if (!out.fail()) {
				out << whole;
				out.close();
			} else {
				*pconsole << TEXT("create/open file failed!") << endl;
				out.close();
			}

		}
	}
	mtx.unlock();
}

void YoukuWindow::PopulateCheckList(const vector<VideoList>& videolist) {
	LV_ITEM lvI;
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.state = 0;
	lvI.stateMask = 0;
	UINT index = 0;
	for (int i = 0; i < videolist.size(); ++i) {
		lvI.iItem = i;
		lvI.iSubItem = 0;
		lvI.pszText = const_cast<LPTSTR>(videolist[i].index.c_str());
		lvI.cchTextMax = static_cast<int>(videolist[i].name.size() + 1);
		pchecklist->AddListViewItem(lvI);
		pchecklist->SetItemText(i, 1, videolist[i].name);
		pchecklist->SetItemText(i, 2, videolist[i].url);
		pchecklist->SetCheckState(i, false);
	}
	pchecklist->AutoAdjustColumnSize(0);
	pchecklist->AutoAdjustColumnSize(1);
	pchecklist->AutoAdjustColumnSize(2);
}
void YoukuWindow::OnClearButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT);
	//If the start is 0 and the end is ¨C1, all the text in the edit control is selected. If the start is ¨C1, any current selection is deselected. 
	SendMessage(hwndEdit, EM_SETSEL, 0, -1);
	SendMessage(hwndEdit, WM_CLEAR, 0, 0);
	URL = TEXT("");
	HWND hwndresult = GetDlgItem(hwnd, ID_RESULT);
	SetWindowText(hwndresult, nullptr);
}
void YoukuWindow::OnSelectAllButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	for (int i = 0; i < videolist.size(); ++i) {
		videolist[i].bChecked = true;
		pchecklist->SetCheckState(i, true);
	}
}
void YoukuWindow::OnInvSelectButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	for (int i = 0; i < videolist.size(); ++i) {
		bool temp = videolist[i].bChecked;
		videolist[i].bChecked = !temp;
		pchecklist->SetCheckState(i, !temp);
	}
}

void YoukuWindow::OnSelectFolderButtonClicked(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		CComPtr<IFileOpenDialog> pFileOpen;
		hr = pFileOpen.CoCreateInstance(__uuidof(FileOpenDialog));
		if (SUCCEEDED(hr)) {
			DWORD dwFlags;
			hr = pFileOpen->GetOptions(&dwFlags);
			if (SUCCEEDED(hr)) {
				hr = pFileOpen->SetOptions(dwFlags | FOS_PICKFOLDERS);
				if (SUCCEEDED(hr)) {
					DownloadPath = tstring();
					DWORD len = GetCurrentDirectory(0, &DownloadPath[0]);
					DownloadPath.resize(len);
					len = GetCurrentDirectory(len, &DownloadPath[0]);
					DWORD errcode;
					if (len == 0) {
						errcode = GetLastError();
						hr = HRESULT_FROM_WIN32(errcode);
					}
					if (SUCCEEDED(hr)) {
								CComPtr<IShellItem> psiFolder;
#ifndef UNICODE
								wstring downloadpath;
								downloadpath.assign(downloadpath.cbegin(), downloadpath.cend());
								hr = SHCreateItemFromParsingName(&downloadpath[0], nullptr, IID_PPV_ARGS(&psiFolder));
#else
								hr = SHCreateItemFromParsingName(&DownloadPath[0], nullptr, IID_PPV_ARGS(&psiFolder));
#endif
								if (SUCCEEDED(hr)) {
									hr = pFileOpen->SetDefaultFolder(psiFolder);
									if (SUCCEEDED(hr)) {
										hr = pFileOpen->Show(nullptr);
										if (SUCCEEDED(hr)) {
											CComPtr<IShellItem> pItem;
											hr = pFileOpen->GetResult(&pItem);
											if (SUCCEEDED(hr)) {
												LPWSTR pszFilePath;
												hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
												if (SUCCEEDED(hr)) {
													//*pconsole << pszFilePath << endl;
													DownloadPath = tstring();//clear
#ifdef UNICODE
													DownloadPath = pszFilePath;
#else
													DownloadPath = WideToByte(CP_UTF8, pszFilePath);
#endif
													SetWindowText(GetDlgItem(hwnd, ID_PATH), DownloadPath.c_str());
													CoTaskMemFree(pszFilePath);
												}
											}
										}
									}
								}
							}
						}
						CoUninitialize();
					}
				}
	}
	_com_error err(hr);	
	//OK and Cancell does not show the err message, the other cases shows
	if (hr != S_OK && hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		LPCTSTR errMsg = err.ErrorMessage();
		*pconsole << errMsg << endl;
	}
}


	

void CheckList::InitialListView() {
	LV_COLUMN lvC;
	lvC.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	lvC.fmt = LVCFMT_LEFT;
	lvC.cx = 300;
	lvC.pszText = TEXT("No.");
	lvC.iSubItem = 0;
	AddListViewColumn(0, lvC);
	lvC.pszText = TEXT("Name");
	lvC.iSubItem = 1;
	AddListViewColumn(1, lvC);
	lvC.pszText = TEXT("URL");
	lvC.iSubItem = 2;
	AddListViewColumn(2, lvC);
}

CheckList::CheckList(const tstring& caption, const DWORD& dwWndStyle, const int& x, const int& y, const int& width, const int& height, const HWND& parent, const HMENU& hmenu, const HINSTANCE& hi) : CListViewCtrl(caption, dwWndStyle, x, y, width, height, parent, hmenu, hi) {
	InitialListView();
	ListView_SetExtendedListViewStyle(hwnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
}

void CheckList::SetCheckState(const int& i, bool fCheck) {
	ListView_SetCheckState(hwnd, i, fCheck);
}
bool CheckList::GetCheckState(const int& i) {
	if (ListView_GetCheckState(hwnd, i)) {
		return true;
	} else {
		return false;
	}
}
bool CheckList::Update(const int& i) {
	if (ListView_Update(hwnd, i)) {
		return true;
	} else {
		return false;
	}
}

