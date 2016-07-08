#pragma once


class httpclient {
	public:
		httpclient(ConsoleStream& ss) : output(ss) {}
		~httpclient();
		bool EstablishConnection(const tstring& url, const tstring& cookie, bool getsetcookie, LPCTSTR referer);
		bool RetrievingHTTPHeaders(const HINTERNET& hHttp, tstring& output, const DWORD &Queryflag);
		void PrintURLComponents(const URL_COMPONENTS& urlstruct);
		bool GetHtml(const tstring& url, tstring& html);
		bool GetJson(const tstring& url, tstring& json, const tstring& referer, const tstring& cookie);
		bool GetM3U8(const tstring& url, tstring& m3u8);
		bool GetVideo(const tstring &url, string& video);
		tstring GetCookie(const tstring& url);
		tstring GetSetCookie(const tstring& url);// { return setcookie; };
		tstring GetHtmlSetCookie() { return setcookie; }
		void SetTreeListView(CTreeListView* p) { TreeListViewoutput = p; }
		void SetTreeRowInfo(TreeListNodeData* p) { row_info = p; }
	private:
		bool downloadTXTfile(tstring& file);
		bool downloadBINfile(string& file);
		HINTERNET hIntSession, hHttpSession, hHttpRequest, hURL;
		CTreeListView* TreeListViewoutput;
		TreeListNodeData* row_info;
		tstring cookie;
		tstring setcookie;
		DWORD resource_size;
		ConsoleStream& output;
		Speedometer speedo;
		unordered_map<int, tstring> internet_scheme{
			{ -2, TEXT("INTERNET_SCHEME_PARTIAL") },
			{ -1, TEXT("INTERNET_SCHEME_UNKNOWN") },
			{ 0, TEXT("INTERNET_SCHEME_DEFAULT") },
			{ 1, TEXT("INTERNET_SCHEME_FTP") },
			{ 2, TEXT("INTERNET_SCHEME_GOPHER") },
			{ 3, TEXT("INTERNET_SCHEME_HTTP") },
			{ 4, TEXT("INTERNET_SCHEME_HTTPS") },
			{ 5, TEXT("INTERNET_SCHEME_FILE") },
			{ 6, TEXT("INTERNET_SCHEME_NEWS") },
			{ 7, TEXT("INTERNET_SCHEME_MAILTO") },
			{ 8, TEXT("INTERNET_SCHEME_SOCKS") },
			{ 9, TEXT("INTERNET_SCHEME_JAVASCRIPT") },
			{ 10,TEXT("INTERNET_SCHEME_VBSCRIPT") },
			{ 11,TEXT("INTERNET_SCHEME_RES") },
			{ 12,TEXT("INTERNET_SCHEME_FIRST | INTERNET_SCHEME_FTP") },
			{ 13,TEXT("INTERNET_SCHEME_LAST | INTERNET_SCHEME_RES") }
		};
};
