#include "stdafx.h"





httpclient::~httpclient() {
	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hURL);
	InternetCloseHandle(hIntSession);
}


bool httpclient::EstablishConnection(const tstring& url, const tstring& cookie, bool getsetcookie = false, LPCTSTR referer = nullptr) {
	

	tcout << TEXT("Internet Establishing ...") << endl;
	output << TEXT("Internet Establishing ...") << endl;
	DWORD errorcode;
	URL_COMPONENTS urlstruct;
	tstring temp;
	ZeroMemory(&urlstruct, sizeof(URL_COMPONENTS));
	urlstruct.dwStructSize = sizeof(URL_COMPONENTS);
	urlstruct.dwExtraInfoLength = -1;
	urlstruct.dwHostNameLength = -1;
	urlstruct.dwPasswordLength = -1;
	urlstruct.dwSchemeLength = -1;
	urlstruct.dwUrlPathLength = -1;
	urlstruct.dwUserNameLength = -1;
	tcout << TEXT("InternetCrackUrl ...") << endl;
	output << TEXT("InternetCrackUrl ...") << endl;
	if (!InternetCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &urlstruct)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetCrackUrl failed!") << endl;
		output << TEXT("InternetCrackUrl failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetCrackUrl"));
		goto fail;
	} else {
		tcout << TEXT("InternetCrackUrl passed!") << endl;
		output << TEXT("InternetCrackUrl passed!") << endl;
	}
	PrintURLComponents(urlstruct);
	tcout << TEXT("InternetCheckConnection ...") << endl;
	output << TEXT("InternetCheckConnection ...") << endl;
	if (!InternetCheckConnection(url.c_str(), FLAG_ICC_FORCE_CONNECTION, 0)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetCheckConnection failed!") << endl;
		output << TEXT("InternetCheckConnection failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetCheckConnection"));
		goto fail;
	} else {
		tcout << TEXT("InternetCheckConnection passed!") << endl;
		output << TEXT("InternetCheckConnection passed!") << endl;
	}
	tcout << TEXT("InternetOpen ...") << endl;
	output << TEXT("InternetOpen ...") << endl;
	hIntSession = InternetOpen(TEXT("Internet Session"), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, /*INTERNET_FLAG_ASYNC*/0);
	if (hIntSession) {
		tcout << TEXT("InternetOpen passed!") << endl;
		output << TEXT("InternetOpen passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetOpen failed!") << endl;
		output << TEXT("InternetOpen failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetOpen"));
		goto fail;
	}
	
	
	tcout << TEXT("InternetSetOption receive timeout ...") << endl;
	output << TEXT("InternetSetOption receive timeout ...") << endl;
	DWORD timeout = 300;
	if (InternetSetOption(hIntSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout))) {
		tcout << TEXT("InternetSetOption receive timeout passed!") << endl;
		output << TEXT("InternetSetOption receive timeout passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetSetOption receive timeout failed!") << endl;
		output << TEXT("InternetSetOption receive timeout failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetSetOption receive timeout"));
	}
	
	
	
	temp.assign(urlstruct.lpszHostName, urlstruct.dwHostNameLength);
	tcout << TEXT("InternetConnect ...") << endl;
	output << TEXT("InternetConnect ...") << endl;
	hHttpSession = InternetConnect(hIntSession, temp.c_str(), INTERNET_DEFAULT_HTTP_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, INTERNET_FLAG_ASYNC, 0);
	if (hHttpSession) {
		tcout << TEXT("InternetConnect passed!") << endl;
		output << TEXT("InternetConnect passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetConnect failed!") << endl;
		output << TEXT("InternetConnect failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetConnect"));
		//InternetCloseHandle(hIntSession);
		goto fail;
	}
	
	//Important: if you want to send a cookie with send http request, you have to do:
	//INTERNET_FLAG_NO_COOKIES
	//and
	//InternetSetCookie
	//and
	//add cookie string pointer and length to HttpSendRequest,
	//3 conditions at same time, then the send cookie works!!!
	
	if (!cookie.empty()) {
		tstring wholeurl;
		wholeurl.assign(urlstruct.lpszHostName, urlstruct.dwHostNameLength);
		wholeurl.append(temp);
		InternetSetCookie(wholeurl.c_str(), nullptr, &cookie[0]);
	}
	// 
	tcout << TEXT("HttpOpenRequest ...") << endl;
	output << TEXT("HttpOpenRequest ...") << endl;
	temp.assign(urlstruct.lpszUrlPath, urlstruct.dwUrlPathLength);
	temp.append(urlstruct.lpszExtraInfo, urlstruct.dwExtraInfoLength);
	hHttpRequest = HttpOpenRequest(hHttpSession, TEXT("GET"), temp.c_str(), nullptr, referer, nullptr, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_COOKIES, 0);
	if (hHttpRequest) {
		tcout << TEXT("HttpOpenRequest passed!") << endl;
		output << TEXT("HttpOpenRequest passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("HttpOpenRequest failed!") << endl;
		output << TEXT("HttpOpenRequest failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("HttpOpenRequest"));
		goto fail;
	}
	
	if (!cookie.empty()) {
		tcout << TEXT("HttpAddHeader ...") << endl;
		output << TEXT("HttpAddHeader ...") << endl;
		TCHAR* buffer = new TCHAR[cookie.size() + 1];
		for (auto i = 0u; i < cookie.size(); ++i) {
			buffer[i] = cookie[i];
		}
		buffer[cookie.size()] = TEXT('\0');
		if (HttpAddRequestHeaders(hHttpRequest, buffer, static_cast<DWORD>(cookie.size()), HTTP_ADDREQ_FLAG_ADD)) {
		//if (HttpAddRequestHeaders(hHttpRequest, &cookie[0], static_cast<DWORD>(cookie.size()), HTTP_ADDREQ_FLAG_ADD)) {
			tcout << TEXT("HttpAddheader passed!") << endl;
			output << TEXT("HttpAddheader passed!") << endl;
		} else {
			errorcode = GetLastError();
			tcout << TEXT("HttpAddheader failed!") << endl;
			output << TEXT("HttpAddheader failed!") << endl;
			InternetErrorOut(output, errorcode, TEXT("HttpAddHeader"));
		}
		delete[]buffer;
	}
	
	LPCTSTR pHeader = nullptr;
	DWORD dwHeadSize = 0;
	if (!cookie.empty()) {
		//pHeader = &cookie[0];
		//dwHeadSize = static_cast<DWORD>(cookie.size());
	}
	tcout << TEXT("HttpSendRequest ...") << endl;
	output << TEXT("HttpSendRequest ...") << endl;
	int retrytime = 20;
	bool flag = false;
	while(!HttpSendRequest(hHttpRequest, pHeader, dwHeadSize, nullptr, 0)) {
		errorcode = GetLastError();
		if (errorcode == 12002) {
			if (retrytime > 0) {
				tcout << TEXT("HttpSendRequest timeout, retrying... ") << retrytime << TEXT(" times left.") << TEXT('\r');
				output || endl;
				flag = true;
				output << TEXT("HttpSendRequest timeout, retrying... ") << retrytime << TEXT(" times left.");
				--retrytime;
				continue;
			} else {
				output << endl;
				tcout << TEXT("HttpSendRequest after 20 times retrys, still timeout, retry this later.")  << endl;
				output << TEXT("HttpSendRequest after 20 times retrys, still timeout, retry this later.") << endl;
			}

		}
			tcout << TEXT("HttpSendRequest failed!") << endl;
			output << TEXT("HttpSendRequest failed!") << endl;
			InternetErrorOut(output, errorcode, TEXT("HttpSendRequest"));
			goto fail;
	}
	if (flag) {
		output << endl;
	}
	tcout << TEXT("HttpSendRequest passed!") << endl << endl;
	output << TEXT("HttpSendRequest passed!") << endl << endl;


	{
		tcout << TEXT("HttpQueryInfo: Querying for \"Http Send Request Headers\" ...") << endl << endl;
		output << TEXT("HttpQueryInfo: Querying for \"Http Send Request Headers\" ...") << endl << endl;

		DWORD dwInfoBufferLength = 0;
		//tstring InfoBuffer = TEXT("");
		TCHAR* InfoBuffer = nullptr;
		while (!HttpQueryInfo(hHttpRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS, InfoBuffer, &dwInfoBufferLength, nullptr)) {
			errorcode = GetLastError();
			if (errorcode == ERROR_INSUFFICIENT_BUFFER) {
				//InfoBuffer.resize(dwInfoBufferLength + 2);
				InfoBuffer = new TCHAR[dwInfoBufferLength + 1];
			} else if (errorcode == ERROR_HTTP_HEADER_NOT_FOUND) {
				tcout << TEXT("HttpQueryInfo: \"Http Send Request Headers\" not found!") << endl;
				output << TEXT("HttpQueryInfo: \"Http Send Request Headers\" not found!") << endl;
				break;
			} else {
				tcout << TEXT("HttpQueryInfo: Querying for \"Http Send Request Headers\" failed!") << endl;
				output << TEXT("HttpQueryInfo: Querying for \"Http Send Request Headers\" failed!") << endl;
				InternetErrorOut(output, errorcode, TEXT("HttpQueryInfo: Querying for \"Http Send Request Headers\""));
				InternetCloseHandle(hHttpRequest);
				InternetCloseHandle(hHttpSession);
				InternetCloseHandle(hIntSession);
				goto fail;
			}
		}
		errorcode = GetLastError();//here is for after ERROR_INSUFFICIENT_BUFFER, the second time HttpQueryInfo will success and errorcode will keep ERROR_INSUFFICIENT_BUFFER, so here re-get new errorcode
		if (errorcode == ERROR_SUCCESS) {//here is for after ERROR_HTTP_HEADER_NOT_FOUND, it break out of loop, but don't need to print cookie and release allocated memory.
			InfoBuffer[dwInfoBufferLength] = TEXT('\0');
			tcout << TEXT("\"Http Send Request Headers\" is : ") << endl << endl << InfoBuffer << endl;
			output << TEXT("\"Http Send Request Headers\" is : ") << endl << endl << InfoBuffer << endl;
			//setcookie = InfoBuffer;
			delete[] InfoBuffer;
		}
		tcout << TEXT("HttpQueryInfo Querying for \"Http Send Request Headers\" passed!") << endl;
		output << TEXT("HttpQueryInfo Querying for \"Http Send Request Headers\" passed!") << endl;
	}
	{
		tcout << TEXT("HttpQueryInfo: Querying for \"Http Receive Respond Headers\" ...") << endl;
		output << TEXT("HttpQueryInfo: Querying for \"Http Receive Respond Headers\" ...") << endl;

		DWORD dwInfoBufferLength = 0;
		//tstring InfoBuffer = TEXT("");
		TCHAR* InfoBuffer = nullptr;
		while (!HttpQueryInfo(hHttpRequest, HTTP_QUERY_RAW_HEADERS_CRLF, InfoBuffer, &dwInfoBufferLength, nullptr)) {
			errorcode = GetLastError();
			if (errorcode == ERROR_INSUFFICIENT_BUFFER) {
				//InfoBuffer.resize(dwInfoBufferLength + 2);
				InfoBuffer = new TCHAR[dwInfoBufferLength + 1];
			} else if (errorcode == ERROR_HTTP_HEADER_NOT_FOUND) {
				tcout << TEXT("HttpQueryInfo: \"Http Receive Respond Headers\" not found!") << endl;
				output << TEXT("HttpQueryInfo: \"Http Receive Respond Headers\" not found!") << endl;
				break;
			} else {
				tcout << TEXT("HttpQueryInfo: Querying for \"Http Receive Respond Headers\" failed!") << endl;
				output << TEXT("HttpQueryInfo: Querying for \"Http Receive Respond Headers\" failed!") << endl;
				InternetErrorOut(output, errorcode, TEXT("HttpQueryInfo: Querying for \"Http Receive Respond Headers\""));
				InternetCloseHandle(hHttpRequest);
				InternetCloseHandle(hHttpSession);
				InternetCloseHandle(hIntSession);
				goto fail;
			}
		}
		errorcode = GetLastError();//here is for after ERROR_INSUFFICIENT_BUFFER, the second time HttpQueryInfo will success and errorcode will keep ERROR_INSUFFICIENT_BUFFER, so here re-get new errorcode
		if (errorcode == ERROR_SUCCESS) {//here is for after ERROR_HTTP_HEADER_NOT_FOUND, it break out of loop, but don't need to print cookie and release allocated memory.
			InfoBuffer[dwInfoBufferLength] = TEXT('\0');
			tcout << TEXT("\"Http Receive Respond Headers\" is : ") << endl << endl << InfoBuffer << endl;
			output << TEXT("\"Http Receive Respond Headers\" is : ") << endl << endl << InfoBuffer << endl;
			//setcookie = InfoBuffer;
			delete[] InfoBuffer;
		}
		tcout << TEXT("HttpQueryInfo Querying for \"Http Receive Respond Headers\" passed!") << endl;
		output << TEXT("HttpQueryInfo Querying for \"Http Receive Respond Headers\" passed!") << endl;

	}
	{
		tcout << TEXT("HttpQueryInfo: Querying for \"Resource size\" ...") << endl;
		output << TEXT("HttpQueryInfo: Querying for \"Resource size\" ...") << endl;

		DWORD dwInfoBufferLength = 0;
		//tstring InfoBuffer = TEXT("");
		TCHAR* InfoBuffer = nullptr;
		while (!HttpQueryInfo(hHttpRequest, HTTP_QUERY_CONTENT_LENGTH, InfoBuffer, &dwInfoBufferLength, nullptr)) {
			errorcode = GetLastError();
			if (errorcode == ERROR_INSUFFICIENT_BUFFER) {
				//InfoBuffer.resize(dwInfoBufferLength + 2);
				InfoBuffer = new TCHAR[dwInfoBufferLength + 1];
			} else if (errorcode == ERROR_HTTP_HEADER_NOT_FOUND) {
				tcout << TEXT("HttpQueryInfo: \"Resource size\" not found!") << endl;
				output << TEXT("HttpQueryInfo: \"Resource size\" not found!") << endl;
				break;
			} else {
				tcout << TEXT("HttpQueryInfo: Querying for \"Resource size\" failed!") << endl;
				output << TEXT("HttpQueryInfo: Querying for \"Resource size\" failed!") << endl;
				InternetErrorOut(output, errorcode, TEXT("HttpQueryInfo: Querying for \"Resource size\""));
				InternetCloseHandle(hHttpRequest);
				InternetCloseHandle(hHttpSession);
				InternetCloseHandle(hIntSession);
				goto fail;
			}
		}
		errorcode = GetLastError();//here is for after ERROR_INSUFFICIENT_BUFFER, the second time HttpQueryInfo will success and errorcode will keep ERROR_INSUFFICIENT_BUFFER, so here re-get new errorcode
		if (errorcode == ERROR_SUCCESS) {//here is for after ERROR_HTTP_HEADER_NOT_FOUND, it break out of loop, but don't need to print cookie and release allocated memory.
			InfoBuffer[dwInfoBufferLength] = TEXT('\0');
			tcout << TEXT("\"Resource size\" is : ") << endl << endl << InfoBuffer << endl;
			output << TEXT("\"Resource size\" is : ") << endl << endl << InfoBuffer << endl;
			resource_size = stoi(InfoBuffer);
			//setcookie = InfoBuffer;
			delete[] InfoBuffer;
		}
		tcout << TEXT("HttpQueryInfo Querying for \"Http Resource size\" passed!") << endl;
		output << TEXT("HttpQueryInfo Querying for \"Http Resource size\" passed!") << endl;

	}
	if (getsetcookie) {
		tcout << TEXT("HttpQueryInfoSetCookie ...") << endl;
		output << TEXT("HttpQueryInfoSetCookie ...") << endl;
	
		DWORD dwInfoBufferLength = 0;
		//tstring InfoBuffer = TEXT("");
		TCHAR* InfoBuffer = nullptr;
		while (!HttpQueryInfo(hHttpRequest, HTTP_QUERY_SET_COOKIE, InfoBuffer, &dwInfoBufferLength, nullptr)) {
			errorcode = GetLastError();
			if (errorcode == ERROR_INSUFFICIENT_BUFFER) {
				//InfoBuffer.resize(dwInfoBufferLength + 2);
				InfoBuffer = new TCHAR[dwInfoBufferLength + 1];
			} else {
				tcout << TEXT("HttpQueryInfoSetCookie failed!") << endl;
				output << TEXT("HttpQueryInfoSetCookie failed!") << endl;
				InternetErrorOut(output, errorcode, TEXT("HttpQueryInfoSetCookie"));
				//goto fail;
			}
		}
		//InfoBuffer.push_back(TEXT('\0\0'));
		InfoBuffer[dwInfoBufferLength] = TEXT('\0');
		tcout << TEXT("HttpQueryInfoSetCookie passed!") << endl;
		output << TEXT("HttpQueryInfoSetCookie passed!") << endl;
		tcout << InfoBuffer << endl;
		output << InfoBuffer << endl;
		setcookie = InfoBuffer;
		delete[] InfoBuffer;
	}
		tcout << TEXT("Internet Establish successful!") << endl;
		output << TEXT("Internet Establish successful!") << endl;



	return true;
fail:
	tcout << TEXT("Internet Establish failed!") << endl;
	output << TEXT("Internet Establish failed!") << endl;
	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hIntSession);
	return false;
}
bool httpclient::downloadTXTfile(tstring& file) {
	char Buffer[1024];
	DWORD BufferLen = 0, errorcode;
	BOOL result;
	tcout << TEXT("InternetReadFile...") << endl;
	output << TEXT("InternetReadFile...") << endl;
	do {
		result = InternetReadFile(/*hURL*/hHttpRequest, Buffer, 1024, &BufferLen);
		if (!result) {
			tcout << TEXT("InternetReadFile failed!") << endl;
			output << TEXT("InternetReadFile failed!") << endl;
			errorcode = GetLastError();
			InternetErrorOut(output, errorcode, TEXT("InternetReadFile"));
		} else {
			#ifdef UNICODE
			wstring wBuffer;
			DWORD wBufferLen = MultiByteToWideChar(CP_UTF8, 0, Buffer, BufferLen, &wBuffer[0], 0);
			wBuffer.resize(wBufferLen);
			MultiByteToWideChar(CP_UTF8, 0, Buffer, BufferLen, &wBuffer[0], wBufferLen);
			file += wBuffer;
			#else
			file += Buffer;
			#endif
		}
	} while (result && BufferLen != 0);
	if (result) {
		tcout << TEXT("InternetReadFile finished!") << endl;
		output << TEXT("InternetReadFile finished!") << endl;
		return true;
	}
	return false;
}

bool httpclient::downloadBINfile(const tstring &link, string& file, const tstring& videoURL, CTreeListView* pTree) {
	char Buffer[1024];
	DWORD BufferLen = 0, errorcode;
	BOOL result;
	tcout << TEXT("InternetReadFile...") << endl;
	output << TEXT("InternetReadFile...") << endl;
	int percentage = 0;
	int prevpercentage = 0;
	output << TEXT("Total downloading size : ") << resource_size << endl;
	if (resource_size <= 0) {
		return false;
	}
	speedo.Reset();
	tstring* speed = pTree->GetAllRootNode()->at(videoURL)->AllSiblings.at(link)->pNodeData[4]->text;
	tstring* percentageSTR = pTree->GetAllRootNode()->at(videoURL)->AllSiblings.at(link)->pNodeData[3]->text;
	pTree->GetAllRootNode()->at(videoURL)->AllSiblings.at(link)->pNodeData[2]->pWindow->SetRange(0, 100);
	pTree->GetAllRootNode()->at(videoURL)->AllSiblings.at(link)->pNodeData[2]->pWindow->SetStep(1);
	*speed = TEXT("downloading speed estimating...");
	pTree->Invalidate(nullptr, true);
	do {
		result = InternetReadFile(/*hURL*/hHttpRequest, Buffer, 1024, &BufferLen);
		if (!result) {
			errorcode = GetLastError();
			tcout << TEXT("InternetReadFile failed!") << endl;
			output << TEXT("InternetReadFile failed!") << endl;
			InternetErrorOut(output, errorcode, TEXT("InternetReadFile"));
		} else {
			file.append(Buffer, BufferLen);
			percentage = static_cast<int>(static_cast<int>(file.size()) / static_cast<double>(resource_size) * 100);
			speedo.FeedNewSize(BufferLen);
			if (percentage > prevpercentage || speedo.NeedUpdate(*speed)) {
				pTree->Invalidate(&pTree->GetAllRootNode()->at(videoURL)->AllSiblings.at(link)->pNodeData[4]->rect, true);
				prevpercentage = percentage;
				output || endl;
				output << TEXT("Downloading progress: ") << prevpercentage << TEXT("%    ") << *speed;
				*percentageSTR = to_tstring(percentage) + TEXT("%");
				pTree->Invalidate(&pTree->GetAllRootNode()->at(videoURL)->AllSiblings.at(link)->pNodeData[3]->rect, true);
				pTree->GetAllRootNode()->at(videoURL)->AllSiblings.at(link)->pNodeData[2]->pWindow->StepIt();
			}
		}
	} while (result && BufferLen != 0);
	if (result) {
		tcout << endl << TEXT("InternetReadFile finished!") << endl;
		output << endl << TEXT("InternetReadFile finished!") << endl;
		return true;
	}
	return false;
}
bool httpclient::GetHtml(const tstring & url, tstring& html) {
	if (EstablishConnection(url, tstring(), true, nullptr)) {
		bool result = downloadTXTfile(html);
		//output << GetCookie(url) << endl;
		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hIntSession);
		if (result) {
			return true;
		}
	}
	return false;
}
bool httpclient::GetVideo(const tstring &url, string& video,  const tstring& videoURL, CTreeListView* pTree) {
	bool result = false;
	int tries = 20;
	do {
		video.clear();
		if (EstablishConnection(url, tstring(), false, nullptr)) {
			result = downloadBINfile(url, video, videoURL, pTree);
			InternetCloseHandle(hHttpRequest);
			InternetCloseHandle(hHttpSession);
			InternetCloseHandle(hIntSession);
		}
		if (result) {
			return true;
		} else {
			--tries;
		}
	} while (tries > 0);
	return false;
}

bool httpclient::GetJson(const tstring& url, tstring& json, const tstring& referer, const tstring& cookie) {
	if (EstablishConnection(url, cookie, false, &referer[0])) {
		bool result = downloadTXTfile(json);
		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hIntSession);
		if (result) {
			return true;
		}
	}
	return false;
}

bool httpclient::GetM3U8(const tstring& url, tstring& m3u8) {
	if (EstablishConnection(url, tstring(), false, nullptr)) {
		bool result = downloadTXTfile(m3u8);
		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hIntSession);
		if (result) {
			return true;
		}
	}
	return false;
}
bool httpclient::RetrievingHTTPHeaders(const HINTERNET& hHttp, tstring& output, const DWORD& Queryflag) {

	DWORD dwSize = 0;
	DWORD errorcode;
	LPTSTR buffer = nullptr;
retry:
	if (!HttpQueryInfo(hHttp, Queryflag, buffer, &dwSize, NULL)) {
		errorcode = GetLastError();
		if (errorcode == ERROR_HTTP_HEADER_NOT_FOUND) {
			return true;
		} else {
			if (errorcode == ERROR_INSUFFICIENT_BUFFER) {
				//output.resize(dwSize);
				buffer = new TCHAR[dwSize + 1];
				goto retry;
			} else {
				InternetErrorOut(nullptr, errorcode, TEXT("HttpQueryInfo"));
				return false;
			}
		}
	}
	buffer[dwSize] = TEXT('\0');
	output = buffer;
	delete[]buffer;
	return true;
}

void httpclient::PrintURLComponents(const URL_COMPONENTS& urlstruct) {
	tstring temp;
	//use std::(w)string assign to capture the partial char* string with corresponding length
	tcout << TEXT("Struct Size: ") << urlstruct.dwStructSize << endl;
	tcout << TEXT("Scheme name: ") << temp.assign(urlstruct.lpszScheme, urlstruct.dwSchemeLength) << endl;
	tcout << TEXT("Scheme Length: ") << urlstruct.dwSchemeLength << endl;
	tcout << TEXT("internet scheme: ") << internet_scheme[urlstruct.nScheme] << endl;
	tcout << TEXT("host name: ") << temp.assign(urlstruct.lpszHostName, urlstruct.dwHostNameLength) << endl;
	tcout << TEXT("host name length: ") << urlstruct.dwHostNameLength << endl;
	tcout << TEXT("UserName: ") << temp.assign(urlstruct.lpszUserName, urlstruct.dwUserNameLength) << endl;
	tcout << TEXT("UserName length: ") << urlstruct.dwUserNameLength << endl;
	tcout << TEXT("Password: ") << temp.assign(urlstruct.lpszPassword, urlstruct.dwPasswordLength) << endl;
	tcout << TEXT("Password length: ") << urlstruct.dwPasswordLength << endl;
	tcout << TEXT("Url Path: ") << temp.assign(urlstruct.lpszUrlPath, urlstruct.dwUrlPathLength) << endl;
	tcout << TEXT("Url Path length: ") << urlstruct.dwUrlPathLength << endl;
	tcout << TEXT("Extra Info: ") << temp.assign(urlstruct.lpszExtraInfo, urlstruct.dwExtraInfoLength) << endl;
	tcout << TEXT("Extra Info length: ") << urlstruct.dwExtraInfoLength << endl;

	output << TEXT("Struct Size: ") << urlstruct.dwStructSize << endl;
	output << TEXT("Scheme name: ") << temp.assign(urlstruct.lpszScheme, urlstruct.dwSchemeLength) << endl;
	output << TEXT("Scheme Length: ") << urlstruct.dwSchemeLength << endl;
	output << TEXT("internet scheme: ") << internet_scheme[urlstruct.nScheme] << endl;
	output << TEXT("host name: ") << temp.assign(urlstruct.lpszHostName, urlstruct.dwHostNameLength) << endl;
	output << TEXT("host name length: ") << urlstruct.dwHostNameLength << endl;
	output << TEXT("UserName: ") << temp.assign(urlstruct.lpszUserName, urlstruct.dwUserNameLength) << endl;
	output << TEXT("UserName length: ") << urlstruct.dwUserNameLength << endl;
	output << TEXT("Password: ") << temp.assign(urlstruct.lpszPassword, urlstruct.dwPasswordLength) << endl;
	output << TEXT("Password length: ") << urlstruct.dwPasswordLength << endl;
	output << TEXT("Url Path: ") << temp.assign(urlstruct.lpszUrlPath, urlstruct.dwUrlPathLength) << endl;
	output << TEXT("Url Path length: ") << urlstruct.dwUrlPathLength << endl;
	output << TEXT("Extra Info: ") << temp.assign(urlstruct.lpszExtraInfo, urlstruct.dwExtraInfoLength) << endl;
	output << TEXT("Extra Info length: ") << urlstruct.dwExtraInfoLength << endl;
}
tstring httpclient::GetCookie(const tstring& url) {
	tstring cookie = TEXT("");
	DWORD errorcode;
	tcout << TEXT("InternetCookie...") << endl;
	output << TEXT("InternetGetCookie...") << endl;
	DWORD cookie_size = 0;
	TCHAR * pbuffer = nullptr;
	//has to be nullptr, not pbuffer for the 3rd parameter for retrieving the size. NO INSUFFICIENT_BUFFER errorcode	
	if (!InternetGetCookie(url.c_str(), nullptr, nullptr, &cookie_size)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetGetCookie failed!") << endl;
		output << TEXT("InternetGetCookie failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetGetCookie"));
		return tstring();
	}
	pbuffer = new TCHAR[cookie_size + 1];
	//cookie.resize(cookie_size + 1);
	if (!InternetGetCookie(url.c_str(), nullptr, /*&cookie[0]*/pbuffer, &cookie_size)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetGetCookie failed!") << endl;
		output << TEXT("InternetGetCookie failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetGetCookie"));
		delete[] pbuffer;
		return tstring();
	}
	//cookie.push_back(TEXT('\0'));
	//cookie[cookie_size] = TEXT('\0');
	pbuffer[cookie_size] = TEXT('\0');
	cookie = pbuffer;
	delete[] pbuffer;
	return cookie;
}

tstring httpclient::GetSetCookie(const tstring& url) {
	tstring setcookie;
	tcout << TEXT("GetSetCookie of ") << url << endl;
	output << TEXT("GetSetCookie of ") << url << endl;
	tcout << TEXT("Internet GetSetCookie ...") << endl;
	output << TEXT("Internet GetSetCookie ...") << endl;
	DWORD errorcode;
	URL_COMPONENTS urlstruct;
	tstring temp;
	ZeroMemory(&urlstruct, sizeof(URL_COMPONENTS));
	urlstruct.dwStructSize = sizeof(URL_COMPONENTS);
	urlstruct.dwExtraInfoLength = -1;
	urlstruct.dwHostNameLength = -1;
	urlstruct.dwPasswordLength = -1;
	urlstruct.dwSchemeLength = -1;
	urlstruct.dwUrlPathLength = -1;
	urlstruct.dwUserNameLength = -1;
	tcout << TEXT("InternetCrackUrl ...") << endl;
	output << TEXT("InternetCrackUrl ...") << endl;
	if (!InternetCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &urlstruct)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetCrackUrl failed!") << endl;
		output << TEXT("InternetCrackUrl failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetCrackUrl"));
		goto fail;
	} else {
		tcout << TEXT("InternetCrackUrl passed!") << endl;
		output << TEXT("InternetCrackUrl passed!") << endl;
	}
	//PrintURLComponents(urlstruct);
	tcout << TEXT("InternetCheckConnection ...") << endl;
	output << TEXT("InternetCheckConnection ...") << endl;
	if (!InternetCheckConnection(url.c_str(), FLAG_ICC_FORCE_CONNECTION, 0)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetCheckConnection failed!") << endl;
		output << TEXT("InternetCheckConnection failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetCheckConnection"));
		goto fail;
	} else {
		tcout << TEXT("InternetCheckConnection passed!") << endl;
		output << TEXT("InternetCheckConnection passed!") << endl;
	}
	tcout << TEXT("InternetOpen ...") << endl;
	output << TEXT("InternetOpen ...") << endl;
	hIntSession = InternetOpen(TEXT("Internet Session"), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
	if (hIntSession) {
		tcout << TEXT("InternetOpen passed!") << endl;
		output << TEXT("InternetOpen passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetOpen failed!") << endl;
		output << TEXT("InternetOpen failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetOpen"));
		goto fail;
	}

	tcout << TEXT("InternetSetOption receive timeout ...") << endl;
	output << TEXT("InternetSetOption receive timeout ...") << endl;
	DWORD timeout = 300;
	if (InternetSetOption(hIntSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout))) {
		tcout << TEXT("InternetSetOption receive timeout passed!") << endl;
		output << TEXT("InternetSetOption receive timeout passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetSetOption receive timeout failed!") << endl;
		output << TEXT("InternetSetOption receive timeout failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetSetOption receive timeout"));
	}


	temp.assign(urlstruct.lpszHostName, urlstruct.dwHostNameLength);
	tcout << TEXT("InternetConnect ...") << endl;
	output << TEXT("InternetConnect ...") << endl;
	hHttpSession = InternetConnect(hIntSession, temp.c_str(), INTERNET_DEFAULT_HTTP_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, INTERNET_FLAG_ASYNC, 0);
	if (hHttpSession) {
		tcout << TEXT("InternetConnect passed!") << endl;
		output << TEXT("InternetConnect passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetConnect failed!") << endl;
		output << TEXT("InternetConnect failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("InternetConnect"));
		InternetCloseHandle(hIntSession);
		goto fail;
	}


	tcout << TEXT("HttpOpenRequest ...") << endl;
	output << TEXT("HttpOpenRequest ...") << endl;
	temp.assign(urlstruct.lpszUrlPath, urlstruct.dwUrlPathLength);
	temp.append(urlstruct.lpszExtraInfo, urlstruct.dwExtraInfoLength);
	hHttpRequest = HttpOpenRequest(hHttpSession, TEXT("GET"), temp.c_str(), nullptr, nullptr, nullptr, INTERNET_FLAG_RELOAD, 0);
	if (hHttpRequest) {
		tcout << TEXT("HttpOpenRequest passed!") << endl;
		output << TEXT("HttpOpenRequest passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("HttpOpenRequest failed!") << endl;
		output << TEXT("HttpOpenRequest failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("HttpOpenRequest"));
		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hIntSession);
		goto fail;
	}

	tcout << TEXT("HttpSendRequest ...") << endl;
	output << TEXT("HttpSendRequest ...") << endl;
	int retrytime = 20;
	while (!HttpSendRequest(hHttpRequest, nullptr, 0, nullptr, 0)) {
		errorcode = GetLastError();
		if (errorcode == 12002) {
			if (retrytime > 0) {
				tcout << TEXT("HttpSendRequest timeout, retrying... ") << retrytime << TEXT(" times left.") << endl;
				output << TEXT("HttpSendRequest timeout, retrying... ") << retrytime << TEXT(" times left.") << endl;
				--retrytime;
				continue;
			} else {
				tcout << TEXT("HttpSendRequest after 20 times retrys, still timeout, retry this later.") << endl;
				output << TEXT("HttpSendRequest after 20 times retrys, still timeout, retry this later.") << endl;
			}

		}
		tcout << TEXT("HttpSendRequest failed!") << endl;
		output << TEXT("HttpSendRequest failed!") << endl;
		InternetErrorOut(output, errorcode, TEXT("HttpSendRequest"));
		goto fail;
	}
	tcout << TEXT("HttpSendRequest passed!") << endl;
	output << TEXT("HttpSendRequest passed!") << endl;

	{
		tcout << TEXT("HttpQueryInfoSetCookie ...") << endl;
		output << TEXT("HttpQueryInfoSetCookie ...") << endl;

		DWORD dwInfoBufferLength = 0;
		//tstring InfoBuffer = TEXT("");
		TCHAR* InfoBuffer = nullptr;
		while (!HttpQueryInfo(hHttpRequest, HTTP_QUERY_SET_COOKIE, InfoBuffer, &dwInfoBufferLength, nullptr)) {
			errorcode = GetLastError();
			if (errorcode == ERROR_INSUFFICIENT_BUFFER) {
				//InfoBuffer.resize(dwInfoBufferLength + 2);
				InfoBuffer = new TCHAR[dwInfoBufferLength + 1];
			} else {
				tcout << TEXT("HttpQueryInfoSetCookie failed!") << endl;
				output << TEXT("HttpQueryInfoSetCookie failed!") << endl;
				InternetErrorOut(output, errorcode, TEXT("HttpQueryInfoSetCookie"));
				InternetCloseHandle(hHttpRequest);
				InternetCloseHandle(hHttpSession);
				InternetCloseHandle(hIntSession);
				goto fail;
			}
		}
		InfoBuffer[dwInfoBufferLength] = TEXT('\0');
		tcout << TEXT("HttpQueryInfoSetCookie passed!") << endl;
		output << TEXT("HttpQueryInfoSetCookie passed!") << endl;
		tcout << InfoBuffer << endl;
		output << InfoBuffer << endl;
		setcookie = InfoBuffer;
		delete[] InfoBuffer;
	}
	tcout << TEXT("Internet GetSetCookie successful!") << endl;
	output << TEXT("Internet GetSetCookie successful!") << endl;


/*
	{
		tcout << TEXT("HttpQueryInfoCookie ...") << endl;
		output << TEXT("HttpQueryInfoCookie ...") << endl;

		DWORD dwInfoBufferLength = 0;
		//tstring InfoBuffer = TEXT("");
		TCHAR* InfoBuffer = nullptr;
		while (!HttpQueryInfo(hHttpRequest, HTTP_QUERY_COOKIE, InfoBuffer, &dwInfoBufferLength, nullptr)) {
			errorcode = GetLastError();
			if (errorcode == ERROR_INSUFFICIENT_BUFFER) {
				//InfoBuffer.resize(dwInfoBufferLength + 2);
				InfoBuffer = new TCHAR[dwInfoBufferLength + 1];
			} else {
				tcout << TEXT("HttpQueryInfoCookie failed!") << endl;
				output << TEXT("HttpQueryInfoCookie failed!") << endl;
				InternetErrorOut(output, errorcode, TEXT("HttpQueryInfoCookie"));
				InternetCloseHandle(hHttpRequest);
				InternetCloseHandle(hHttpSession);
				InternetCloseHandle(hIntSession);
				goto fail;
			}
		}
		InfoBuffer[dwInfoBufferLength] = TEXT('\0');
		tcout << TEXT("HttpQueryInfoCookie passed!") << endl;
		output << TEXT("HttpQueryInfoCookie passed!") << endl;
		tcout << InfoBuffer << endl;
		output << InfoBuffer << endl;
		//setcookie = InfoBuffer;
		delete[] InfoBuffer;
	}
	tcout << TEXT("Internet GetCookie successful!") << endl;
	output << TEXT("Internet GetCookie successful!") << endl;
*/

	return setcookie;
fail:
	tcout << TEXT("Internet GetSetCookie failed!") << endl;
	output << TEXT("Internet GetSetCookie failed!") << endl;
	return setcookie;
}

/*bool RetrievingHTTPHeaders(HINTERNET hHttp, tstring& output, DWORD Queryflag);


bool EstablishConnection(tstring url, HINTERNET & handle) {
	HINTERNET hIntSession, hHttpSession, hHttpRequest, hURL;
	DWORD errorcode;
	URL_COMPONENTS urlstruct;
	ZeroMemory(&urlstruct, sizeof(URL_COMPONENTS));
	urlstruct.dwStructSize = sizeof(URL_COMPONENTS);
	urlstruct.dwExtraInfoLength = -1;
	urlstruct.dwHostNameLength = -1;
	urlstruct.dwPasswordLength = -1;
	urlstruct.dwSchemeLength = -1;
	urlstruct.dwUrlPathLength = -1;
	urlstruct.dwUserNameLength = -1;
	if (!InternetCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &urlstruct)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetCrackUrl failed!") << endl;
		InternetErrorOut(nullptr, errorcode, TEXT("InternetCrackUrl"));
		return false;
	} else {
		tcout << TEXT("InternetCrackUrl passed!") << endl;
	}
	//use std::(w)string assign to capture the partial char* string with corresponding length
	tstring temp;
	tcout << TEXT("Struct Size: ") << urlstruct.dwStructSize << endl;
	tcout << TEXT("Scheme name: ") << temp.assign(urlstruct.lpszScheme, urlstruct.dwSchemeLength) << endl;
	tcout << TEXT("Scheme Length: ") << urlstruct.dwSchemeLength << endl;
	tcout << TEXT("internet scheme: ") << internet_scheme[urlstruct.nScheme] << endl;
	tcout << TEXT("host name: ") << temp.assign(urlstruct.lpszHostName, urlstruct.dwHostNameLength)<< endl;
	tcout << TEXT("host name length: ") << urlstruct.dwHostNameLength << endl;
	tcout << TEXT("UserName: ") << temp.assign(urlstruct.lpszUserName, urlstruct.dwUserNameLength) << endl;
	tcout << TEXT("UserName length: ") << urlstruct.dwUserNameLength << endl;
	tcout << TEXT("Password: ") << temp.assign(urlstruct.lpszPassword, urlstruct.dwPasswordLength) << endl;
	tcout << TEXT("Password length: ") << urlstruct.dwPasswordLength << endl;
	tcout << TEXT("Url Path: ") << temp.assign(urlstruct.lpszUrlPath, urlstruct.dwUrlPathLength) << endl;
	tcout << TEXT("Url Path length: ") << urlstruct.dwUrlPathLength << endl;
	tcout << TEXT("Extra Info: ") << temp.assign(urlstruct.lpszExtraInfo, urlstruct.dwExtraInfoLength) << endl;
	tcout << TEXT("Extra Info length: ") << urlstruct.dwExtraInfoLength << endl;
	//urlstruct.lpszHostName = TEXT("45.112.212.40");
	//urlstruct.dwHostNameLength = 13 * sizeof(TCHAR);
	if (!InternetCheckConnection(url.c_str(), FLAG_ICC_FORCE_CONNECTION, 0)) {
		errorcode = GetLastError();
		tcout << TEXT("InternetCheckConnection failed!") << endl;
		InternetErrorOut(nullptr, errorcode, TEXT("InternetCheckConnection"));
		return false;
	} else {
		tcout << TEXT("InternetCheckConnection passed!") << endl;
	}
	hIntSession = InternetOpen(TEXT("test"), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0 );
	if (hIntSession) {
		tcout << TEXT("InternetOpen passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetOpen failed!") << endl;
		InternetErrorOut(nullptr, errorcode, TEXT("InternetOpen"));
		return false;
	}

	temp.assign(urlstruct.lpszHostName, urlstruct.dwHostNameLength);
	hHttpSession = InternetConnect(hIntSession, temp.c_str(), INTERNET_DEFAULT_HTTP_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0 , 0);
	if (hHttpSession) {
		tcout << TEXT("InternetConnect passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetConnect failed!") << endl;
		InternetErrorOut(nullptr, errorcode, TEXT("InternetConnect"));
		InternetCloseHandle(hIntSession);
		return false;
	}

	temp.assign(urlstruct.lpszUrlPath, urlstruct.dwUrlPathLength);
	hHttpRequest = HttpOpenRequest(hHttpSession, TEXT("GET"), TEXT(""), nullptr, nullptr, nullptr, INTERNET_FLAG_RELOAD, NULL);
	if (hHttpRequest) {
		tcout << TEXT("HttpOpenRequest passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("HttpOpenRequest failed!") << endl;
		InternetErrorOut(nullptr, errorcode, TEXT("HttpOpenRequest"));
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hIntSession);
		return false;
	}
	tstring httpheader;
	if (RetrievingHTTPHeaders(hHttpRequest, httpheader, HTTP_QUERY_ACCEPT_ENCODING)) {
		tcout << TEXT("Http Header:") << httpheader << endl;
	} else {
		errorcode = GetLastError();
		InternetErrorOut(nullptr, errorcode, TEXT("RetrievingHTTPHeaders"));
	}
	hURL = InternetOpenUrl(hIntSession, url.c_str(), NULL, 0, 0, 0);
	if (hURL) {
		tcout << TEXT("InternetOpenUrl passed!") << endl;
	} else {
		errorcode = GetLastError();
		tcout << TEXT("InternetOpenUrl failed!") << endl;
		InternetErrorOut(nullptr, errorcode, TEXT("InternetOpenUrl"));
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hIntSession);
		return false;
	}

	BOOL result = HttpSendRequest(hHttpRequest, url.c_str(), (DWORD)url.length(), nullptr, 0);
	errorcode = GetLastError();
	if (result) {
		tcout << TEXT("HttpSendRequest passed!") << endl;
	} else {
		InternetErrorOut(nullptr, errorcode, TEXT("HttpSendRequest"));
		tcout << TEXT("HttpSendRequest failed!") << endl;
		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hURL);
		InternetCloseHandle(hIntSession);
		return false;
	}
	if (RetrievingHTTPHeaders(hHttpRequest, httpheader, HTTP_QUERY_CONTENT_LENGTH)) {
		tcout << TEXT("Http Header:") << httpheader << endl;
	} else {
		errorcode = GetLastError();
		InternetErrorOut(nullptr, errorcode, TEXT("RetrievingHTTPHeaders"));
	}
	TCHAR Buffer[1024] = TEXT("\0");
	DWORD BufferLen = 0, BytesWritten = 0;
	//BOOL result;
	//DWORD errorcode;
	HANDLE FileHandle;
	FileHandle = CreateFile(TEXT("temp.txt"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == NULL) { tcout << TEXT("FileHandle == NULL") << endl; }
	tstring html;
	do {
		result = InternetReadFile(hURL, Buffer, 1024, &BufferLen);
		if (!result) {
			errorcode = GetLastError();
			InternetErrorOut(nullptr, errorcode, TEXT("InternetReadFile"));
		}
		html += Buffer;
		WriteFile(FileHandle, Buffer, BufferLen, &BytesWritten, NULL);

	} while (result && BufferLen != 0);
	tcout << endl;
	CloseHandle(FileHandle);
	ShellExecute(0, TEXT("open"), TEXT("temp.txt"), NULL, NULL, 1);
	tcout << html << endl;

	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	//InternetCloseHandle(hURL);
	InternetCloseHandle(hIntSession);
	//handle = hHttpRequest;
	handle = hURL;
	return true;
}

*/