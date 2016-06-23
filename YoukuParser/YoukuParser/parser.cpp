#include "stdafx.h"

void parser(const tstring& html, vector<VideoList>& videolist) {
	//tcout << boolalpha << test_unicode2() << endl;

	tstring hyperlink_pattern = TEXT("<\\s*a \\s*[^>]*\\s*href\\s*=\\s*\\\"[^\"]*\\\"\\s*[^>]*\\s*>");
	tstring capturelink_pattern = TEXT("<\\s*a \\s*[^>]*\\s*href\\s*=\\s*\\\"([^\"]*)\\\"\\s*[^>]*\\s*>");
	tstring capturetitle_pattern = TEXT("title\\s*=\\s*\\\"([^\"]*)\\\"");
	tstring hasvideo_pattern = TEXT("target\\s*=\\s*\\\"\\s*video\\s*\\\"");

	tcout <<  hyperlink_pattern << endl;
	tcout << capturelink_pattern << endl;
	tcout << capturetitle_pattern << endl;
	tcout << hasvideo_pattern << endl;
	tcout << endl;
	//tcout << html << endl;
	tregex pattern(hyperlink_pattern);
	DWORD number = 0;
	for (tregex_iterator it(html.begin(), html.end(), pattern), end_it; it != end_it; ++it) {
		tregex hasvideo(hasvideo_pattern);
		tregex capturetitle(capturetitle_pattern);
		tsmatch title;
		tstring linkstring = it->str();//it->str() does not have a copy constructor like fstream, so you cannot put into regex_search(x,x,x) but can be used in regex_search(x,x)
		if (regex_search(it->str(), hasvideo) && regex_search(linkstring, title, capturetitle)) {
			tcout << TEXT("title: ") << title.str(1) << endl;
			tregex capturelink(capturelink_pattern);
			tsmatch link;
			if (regex_search(linkstring, link, capturelink)) {
				tcout << TEXT("link: ") << link.str(1) << endl;
				videolist.push_back(VideoList{ false, to_tstring(number + 1), title.str(1), link.str(1) });
				++number;
				tcout << endl << number << endl;
			}
		}
	}
	tcout << endl << number << endl;


}

tstring getVid(const tstring& url) {
	//tstring capture_vid_pattern = TEXT("[^\"]+id_([^\"]+)\\.html");
	tstring capture_vid_pattern = TEXT("youku\\.com/v_show/id_([a-zA-Z0-9=]+)");
	tregex capture_vid(capture_vid_pattern);
	tsmatch vid;
	*out << url << endl;
	if (regex_search(url, vid, capture_vid)) {
		*out << vid.str(1) << endl;
	} else {
		*out << TEXT("Not found vid!") << endl;
	}
	return vid.str(1);
}
tstring getJSONUrl(const tstring& vid) {
	tstring JSON;
	//old one does not work: JSON = TEXT("http://v.youku.com/player/getPlayList/VideoIDS/") + vid + TEXT("/Pf/4/ctype/12/ev/1");
	JSON = TEXT("http://play.youku.com/play/get.json?vid=") + vid + TEXT("&ct=12");
//http://play.youku.com/play/get.json?vid=%s&ct=10'
	*out << JSON << endl;
	return JSON;
}

void GetParameters(const tstring& vid, const tstring& encryptString, tstring& sid, tstring& token, tstring& ep) {
	tstring keyA = TEXT("becaf9be");
	string decodeEp = Decode64(encryptString);
	tstring temp = RC4(keyA, decodeEp, false);
	tstringstream ss(temp);
	tstring item;
	vector<tstring> part;
	while (getline(ss, item, TEXT('_'))) {
		part.push_back(item);
	}
	sid = part[0];
	token = part[1];

	tstring keyB = TEXT("bf7e5f01");
	tstring whole = sid + TEXT("_") + vid + TEXT("_") + token;
	string wholebyte;
	for (TCHAR c : whole) {
		wholebyte.push_back(static_cast<char>(c));
	}
	ep = UrlEncode(RC4(keyB, wholebyte, true));
}

//tstring url_pattern = TEXT("<\\s*div\\s+class\\s*=\\s*\\\"\\s*v-link\\s*\\\"\\s*>\\s*<\\s*a\\s+title\\s*=\\s*\\\".*\\\"\\s*target\\s*=\\s*\\\"\\s*video\\s*\\\"\\s*href\\s*=\\s*\\\"http://v.youku.com/v_show/id.*\\\"\\s*>\\s*<\\s*/\\s*a\\s*>\\s*<\\s*/\\s*div\\s*>");
//tstring url_pattern = TEXT("<\\s*div\\s+class\\s*=\\s*\\\"\\s*v-link\\s*\\\"\\s*>\\s*<\\s*a\\s+title\\s*=\\s*\\\".*\\\"\\s*target\\s*=\\s*\\\"\\s*video\\s*\\\"");
//tstring hyperlink_pattern = TEXT("\\S*<\\S*a \\S*href\\S*=\\S*\\\"\\S*_^(?:(?:https?|ftp)://)(?:\\S+(?::\\S*)?@)?(?:(?!10(?:\\.\\d{1,3}){3})(?!127(?:\\.\\d{1,3}){3})(?!169\\.254(?:\\.\\d{1,3}){2})(?!192\\.168(?:\\.\\d{1,3}){2})(?!172\\.(?:1[6-9]|2\\d|3[0-1])(?:\\.\\d{1,3}){2})(?:[1-9]\\d?|1\\d\\d|2[01]\\d|22[0-3])(?:\\.(?:1?\\d{1,2}|2[0-4]\\d|25[0-5])){2}(?:\\.(?:[1-9]\\d?|1\\d\\d|2[0-4]\\d|25[0-4]))|(?:(?:[a-z\\x{00a1}-\\x{ffff}0-9]+-?)*[a-z\\x{00a1}-\\x{ffff}0-9]+)(?:\\.(?:[a-z\\x{00a1}-\\x{ffff}0-9]+-?)*[a-z\\x{00a1}-\\x{ffff}0-9]+)*(?:\\.(?:[a-z\\x{00a1}-\\x{ffff}]{2,})))(?::\\d{2,5})?(?:/[^\\s]*)?$_iuS\\S*\" \\S*target\\S*=\\S*\"\\S*video\\S*\"\\S* title\\S*=\\S*");


bool M3U8Parser(const tstring& m3u8, DownloadFactors& videolinks) {
	//  regex:  ^((?!my string).)*$ not contain a string pattern http://stackoverflow.com/questions/717644/regular-expression-that-doesnt-contain-certain-string
	//															 http://stackoverflow.com/questions/406230/regular-expression-to-match-line-that-doesnt-contain-a-word
	// regex: AND operator: (?=expr)   (?=.*word1)(?=.*word2)(?=.*word3) http://stackoverflow.com/questions/469913/regular-expressions-is-there-an-and-operator
	// not contain: 03008001005756AC0E8896000032C87E2BE91A-97F6-1E3B-A634-DDEEE48EB109.flv.ts?ts_start=0.0&ts_end=6.26&ts_seg_no=0&ts_keyframe=1 AND has pattern http://[^\?]+.flv -> http:// + any character not ? + .flv
	tstring videolink_pattern = TEXT("(http://[^\\?]+\\.flv)\\.ts\\?[^#]+");//non-greedy matching: http://www.regular-expressions.info/repeat.html
	tregex videolink(videolink_pattern);
	tsmatch vlinks;
	unordered_set<tstring> removedupfilename;
	unordered_set<tstring> removeduplink;
	for (tregex_iterator it(m3u8.cbegin(), m3u8.cend(), videolink), end_it; it != end_it; ++it) {
		tstring truevideolink = it->str(1);
		tstring filename_pattern = TEXT("[^/^\\.]+\\.flv");
		tregex filenamefilter(filename_pattern);
		tsmatch filename;
		if (regex_search(truevideolink, filename, filenamefilter)) {
			if (removedupfilename.find(filename.str()) == removedupfilename.cend() && filename.str() != TEXT("03008001005756AC0E8896000032C87E2BE91A-97F6-1E3B-A634-DDEEE48EB109.flv")) { //no duplicate yet and not the last youku ad one
				removedupfilename.insert(filename.str());
				videolinks.order.push_back(filename.str());
			}	
		}
		if (removeduplink.find(truevideolink) == removeduplink.cend()) {
			removeduplink.insert(truevideolink);
			videolinks.links[filename.str()].push_back(truevideolink);
		}
	}
	return true;
}