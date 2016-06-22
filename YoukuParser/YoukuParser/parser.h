#pragma once

void parser(const tstring& html, vector<VideoList>& videolist);
tstring getVid(const tstring& url);
tstring getJSONUrl(const tstring& vid);
void GetParameters(const tstring& vid, const tstring& encryptString, tstring& sid, tstring& token, tstring& ep);
bool M3U8Parser(const tstring& m3u8, DownloadFactors& videolinks);