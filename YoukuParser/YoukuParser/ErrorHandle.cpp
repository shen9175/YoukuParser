#include "stdafx.h"


// Function for displaying Internet Error information in a message box
BOOL WINAPI InternetErrorOut(HWND hWnd, DWORD dwError, const tstring& szFailingFunctionName) {
	tstring  szMsgBoxBuffer, szFormatBuffer, szConnectiveText = TEXT("\nAdditional Information: "), szExtErrMsg, szCombinedErrMsg;
	DWORD  errorcode, dwInetError, dwBaseLength, dwExtLength = 0;

	szFormatBuffer.resize(INET_ERR_OUT_FORMAT_BUFFER_SIZE);
	dwBaseLength = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle(TEXT("wininet.dll")), dwError, 0,	&szFormatBuffer[0],	INET_ERR_OUT_FORMAT_BUFFER_SIZE, NULL); 
	errorcode = GetLastError();
	if (!dwBaseLength) {
		szMsgBoxBuffer += TEXT("Call to FormatMessage( ) failed...");
		addLastErrorToMsg(szMsgBoxBuffer, errorcode);
		goto InetErrorOutError;
	}

	szMsgBoxBuffer += szFailingFunctionName + TEXT(" error; code: ") + to_tstring(dwError) + TEXT("\nDescription: ") + szFormatBuffer + TEXT("\n");
	dwBaseLength = static_cast<DWORD>(szMsgBoxBuffer.size());
	// Adjust base-length value to count the number of bytes:
	dwBaseLength *= sizeof(TCHAR);

	if (dwError == ERROR_INTERNET_EXTENDED_ERROR) {
		InternetGetLastResponseInfo(&dwInetError, nullptr, &dwExtLength);
		// Adjust the extended-length value to a byte count 
		// that includes the terminating null:
		++dwExtLength *= sizeof(TCHAR);
		szExtErrMsg.resize(dwExtLength);

		if (!InternetGetLastResponseInfo(&dwInetError, &szExtErrMsg[0], &dwExtLength)) {
			errorcode = GetLastError();
			szMsgBoxBuffer += TEXT("\nCall to InternetGetLastResponseInfo( ) failed--");
			addLastErrorToMsg(szMsgBoxBuffer, errorcode);
			goto InetErrorOutError;
		}
	}

	if (dwExtLength == 0) {
		szCombinedErrMsg += szMsgBoxBuffer;
	} else {
		szCombinedErrMsg += szMsgBoxBuffer + szConnectiveText + szExtErrMsg;
	}

	if (szCombinedErrMsg.empty()) {
		szMsgBoxBuffer += TEXT("\nFailure: Could not assemble final message.");
		goto InetErrorOutError;
	}
	if (hWnd == nullptr) {
		tcout << TEXT("Internet Error Message: ") << szCombinedErrMsg << endl;
	} else {
		MessageBox(hWnd, szCombinedErrMsg.c_str(), TEXT("Internet Error Message"), MB_OK | MB_ICONERROR);
	}


	return(TRUE);


InetErrorOutError:
	if (hWnd == nullptr) {
		tcout << szFailingFunctionName << TEXT(" error code: ") << dwError << TEXT("\n")<< TEXT("InternetErrorOut( ) Failed: ") << szMsgBoxBuffer << endl;
	} else {
		szMsgBoxBuffer = szFailingFunctionName + TEXT(" error code: ") + to_tstring(dwError) + TEXT("\n") + szMsgBoxBuffer;
		MessageBox(hWnd, szMsgBoxBuffer.c_str(), TEXT("InternetErrorOut( ) Failed"), MB_OK | MB_ICONERROR);
	}
	return(FALSE);
}


void WINAPI addLastErrorToMsg(tstring& szMsgBuffer, DWORD dwError) {
	tstring szNumberBuffer(to_tstring(dwError));
	szMsgBuffer += TEXT("\r\n   System Error number: ");
	szMsgBuffer += szNumberBuffer;
	szMsgBuffer += TEXT(".\r\n");
}


BOOL WINAPI InternetErrorOut(ConsoleStream& output, DWORD dwError, const tstring& szFailingFunctionName) {
	tstring  szMsgBoxBuffer, szFormatBuffer, szConnectiveText = TEXT("\r\nAdditional Information: "), szExtErrMsg, szCombinedErrMsg;
	DWORD  errorcode, dwInetError, dwBaseLength, dwExtLength = 0;

	szFormatBuffer.resize(INET_ERR_OUT_FORMAT_BUFFER_SIZE);
	dwBaseLength = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle(TEXT("wininet.dll")), dwError, 0, &szFormatBuffer[0], INET_ERR_OUT_FORMAT_BUFFER_SIZE, NULL);
	errorcode = GetLastError();
	if (!dwBaseLength) {
		szMsgBoxBuffer += TEXT("Call to FormatMessage( ) failed...");
		addLastErrorToMsg(szMsgBoxBuffer, errorcode);
		goto InetErrorOutError;
	}

	szMsgBoxBuffer += szFailingFunctionName + TEXT(" error; code: ") + to_tstring(dwError) + TEXT("\r\nDescription: ") + szFormatBuffer + TEXT("\r\n");
	dwBaseLength = static_cast<DWORD>(szMsgBoxBuffer.size());
	// Adjust base-length value to count the number of bytes:
	dwBaseLength *= sizeof(TCHAR);

	if (dwError == ERROR_INTERNET_EXTENDED_ERROR) {
		InternetGetLastResponseInfo(&dwInetError, nullptr, &dwExtLength);
		// Adjust the extended-length value to a byte count 
		// that includes the terminating null:
		++dwExtLength *= sizeof(TCHAR);
		szExtErrMsg.resize(dwExtLength);

		if (!InternetGetLastResponseInfo(&dwInetError, &szExtErrMsg[0], &dwExtLength)) {
			errorcode = GetLastError();
			szMsgBoxBuffer += TEXT("\r\nCall to InternetGetLastResponseInfo( ) failed--");
			addLastErrorToMsg(szMsgBoxBuffer, errorcode);
			goto InetErrorOutError;
		}
	}

	if (dwExtLength == 0) {
		szCombinedErrMsg += szMsgBoxBuffer;
	} else {
		szCombinedErrMsg += szMsgBoxBuffer + szConnectiveText + szExtErrMsg;
	}

	if (szCombinedErrMsg.empty()) {
		szMsgBoxBuffer += TEXT("\r\nFailure: Could not assemble final message.");
		goto InetErrorOutError;
	}
	
		output << TEXT("Internet Error Message: ") << szCombinedErrMsg << endl;
	


	return(TRUE);


InetErrorOutError:
		output << szFailingFunctionName << TEXT(" error code: ") << dwError << TEXT("\r\n") << TEXT("InternetErrorOut( ) Failed: ") << szMsgBoxBuffer << endl;

	return(FALSE);
}
