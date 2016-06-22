#pragma once

string WideToByte(unsigned int CodePage, wstring input);
string WideToByte(unsigned int CodePage, LPWSTR input);
wstring ByteToWide(unsigned int CodePage, LPSTR input);
wstring ByteToWide(unsigned int CodePage, string input);
tstring Encode64(const tstring& a);
string EncodingString(DWORD integer);
string Decode64(const tstring& a);
TCHAR ToHex(TCHAR x);
TCHAR FromHex(TCHAR x);
tstring UrlEncode(const tstring& str);
tstring UrlDecode(const tstring& str);
tstring base64_encode(const string& bytes_array);
tstring RC4(const tstring& a, const string& c, bool isToBase64);
tstring GetPvid(int len);
