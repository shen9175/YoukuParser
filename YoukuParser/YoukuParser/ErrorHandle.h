#include "stdafx.h"
#define  INET_ERR_OUT_FORMAT_BUFFER_SIZE       256
// Forward declaration of helper function
void WINAPI addLastErrorToMsg(tstring& szMsgBuffer, DWORD dwError);
BOOL WINAPI InternetErrorOut(HWND hWnd, DWORD dwError, const tstring& szFailingFunctionName);
BOOL WINAPI InternetErrorOut(ConsoleStream& output, DWORD dwError, const tstring& szFailingFunctionName);