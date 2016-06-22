#include"stdafx.h"
string WideToByte(unsigned int CodePage, wstring input) {
	string ret = "";
	DWORD wLen = WideCharToMultiByte(CodePage, 0, &input[0], static_cast<int>(input.length()), &ret[0], 0, nullptr, nullptr);
	ret.resize(wLen);
	WideCharToMultiByte(CodePage, 0, &input[0], static_cast<int>(input.length()), &ret[0], wLen, nullptr, nullptr);
	return ret;
}
string WideToByte(unsigned int CodePage, LPWSTR input) {
	string ret = "";
	DWORD wLen = WideCharToMultiByte(CodePage, 0, input, static_cast<int>(wcslen(input)), &ret[0], 0, nullptr, nullptr);
	ret.resize(wLen);
	WideCharToMultiByte(CodePage, 0, input, static_cast<int>(wcslen(input)), &ret[0], wLen, nullptr, nullptr);
	return ret;
}
wstring ByteToWide(unsigned int CodePage, string input) {
	wstring ret = L"";
	DWORD wLen = MultiByteToWideChar(CodePage, 0, &input[0], static_cast<int>(input.length()), &ret[0], 0);
	ret.resize(wLen);
	MultiByteToWideChar(CodePage, 0, &input[0], static_cast<int>(input.length()), &ret[0], wLen);
	return ret;
}
wstring ByteToWide(unsigned int CodePage, LPSTR input) {
	wstring ret = L"";
	DWORD wLen = MultiByteToWideChar(CodePage, 0, input, static_cast<int>(strlen(input)), &ret[0], 0);
	ret.resize(wLen);
	MultiByteToWideChar(CodePage, 0, input, static_cast<int>(strlen(input)), &ret[0], wLen);
	return ret;
}
tstring Encode64(const tstring& a) {
	if (a.empty()) {
		return a;
	}
	tstring b;
	int c, d;
	tstring h = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
	for (d = static_cast<int>(a.length()), c = 0, b = TEXT(""); d > c;) {
		int e = 255 & a[c++];
		if (c == d) {
			b += h[e >> 2];
			b += h[(3 & e) << 4];
			b += TEXT("==");
			break;
		}
		int f = a[c++];
		if (c == d) {
			b += h[e >> 2];
			b += h[(3 & e) << 4 | (240 & f) >> 4];
			b += h[(15 & f) << 2];
			b += TEXT("=");
			break;
		}
		int g = a[c++];
		b += h[e >> 2];
		b += h[(3 & e) << 4 | (240 & f) >> 4];
		b += h[(15 & f) << 2 | (192 & g) >> 6];
		b += h[63 & g];
	}
	return b;
}
//byte[]
string EncodingString(DWORD integer) {
	string ret;
	for (int i = 0; i < 4; ++i) {
		char byte = ((0xff << ((3 - i) * 8)) & integer) >> ((3 - i) * 8);
		if (byte != 0) {
			ret.push_back(byte);
		}
	}
	return ret;
}
//byte[] generate an array of bytes
string Decode64(const tstring& input) {
	if (input.empty()) {
		return string();
	}
	string a = WideToByte(CP_UTF8, input);
	int f, g;
	//string h;
	string l;
	int i[] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1,
		-1, -1
	};
	for (g = static_cast<int>(a.length()), f = 0/*, h = ""*/; g > f;) {
		int b;
		do {
			b = i[255 & a[f++]];
		} while (g > f && -1 == b);
		if (-1 == b) {
			break;
		}
		int c;
		do {
			c = i[255 & a[f++]];
		} while (g > f && -1 == c);
		if (-1 == c) {
			break;
		}
		char byte0 = b << 2 | (48 & c) >> 4;
		l.push_back(byte0);
		//DWORD byte0 = b << 2 | (48 & c) >> 4;
		//string Bytes0 = EncodingString(byte0);
		//h += Bytes0;
		//l.push_back(Bytes0[0]);
		int d;
		do {
			d = 255 & a[f++];
			if (61 == d) {
				return l;
			}
			d = i[d];
		} while (g > f && -1 == d);
		if (-1 == d) {
			break;
		}
		char byte1 = (15 & c) << 4 | (60 & d) >> 2;
		l.push_back(byte1);
		//DWORD byte1 = (15 & c) << 4 | (60 & d) >> 2;
		//string Bytes1 = EncodingString(byte1);
		//h += Bytes1;
		//l.push_back(Bytes1[0]);
		int e;
		do {
			e = 255 & a[f++];
			if (61 == e) {
				return l;
			}
			e = i[e];
		} while (g > f && -1 == e);
		char byte2 = (3 & d) << 6 | e;
		l.push_back(byte2);
		//DWORD byte2 = (3 & d) << 6 | e;
		//string Bytes2 = EncodingString(byte2);
		//h += Bytes2;
		//l.push_back(Bytes2[0]);
	}
	return l;
}

TCHAR ToHex(TCHAR x) {
	return  x > 9 ? x + 55 : x + 48;
}

TCHAR FromHex(TCHAR x) {
	TCHAR y;
	if (x >= TEXT('A') && x <= TEXT('Z')) y = x - TEXT('A') + 10;
	else if (x >= TEXT('a') && x <= TEXT('z')) y = x - TEXT('a') + 10;
	else if (x >= TEXT('0') && x <= TEXT('9')) y = x - TEXT('0');
	else assert(0);
	return y;
}

tstring UrlEncode(const tstring& str) {
	tstring strTemp = TEXT("");
	size_t length = str.length();
	for (size_t i = 0; i < length; i++) {
		if (isalnum((TCHAR)str[i]) ||
			(str[i] == TEXT('-')) ||
			(str[i] == TEXT('_')) ||
			(str[i] == TEXT('.')) ||
			(str[i] == TEXT('~')))
			strTemp += str[i];
		else if (str[i] == ' ')
			strTemp += TEXT("+");
		else {
			strTemp += TEXT('%');
			strTemp += ToHex((TCHAR)str[i] >> 4);
			strTemp += ToHex((TCHAR)str[i] % 16);
		}
	}
	return strTemp;
}

tstring UrlDecode(const tstring& str) {
	tstring strTemp = TEXT("");
	size_t length = str.length();
	for (size_t i = 0; i < length; i++) {
		if (str[i] == TEXT('+')) strTemp += TEXT(' ');
		else if (str[i] == TEXT('%')) {
			assert(i + 2 < length);
			TCHAR high = FromHex((TCHAR)str[++i]);
			TCHAR low = FromHex((TCHAR)str[++i]);
			strTemp += high * 16 + low;
		} else strTemp += str[i];
	}
	return strTemp;
}
tstring base64_encode(const string& bytes_array) {
	tstring base64_chars = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
	tstring ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	unsigned int in_len = static_cast<unsigned int>(bytes_array.length());
	char const* bytes_to_encode = bytes_array.c_str();
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += TEXT('=');

	}

	return ret;

}
tstring RC4(const tstring& a, const string& c, bool isToBase64) {
	int f = 0, h = 0, q;
	int b[256];
	for (int i = 0; i < 256; ++i) {
		b[i] = i;
	}
	while (h < 256) {
		f = (f + b[h] + a[h % a.length()]) % 256;
		swap(b[h], b[f]);
		++h;
	}
	f = 0; h = 0; q = 0;
	tstring result = TEXT("");
	string bytesR;
	while (q < static_cast<int>(c.length())) {
		h = (h + 1) % 256;
		f = (f + b[h]) % 256;
		swap(b[h], b[f]);
		char byte = c[q] ^ b[(b[h] + b[f]) % 256];
		bytesR.push_back(byte);
		result.push_back(byte);
		++q;
	}
	if (isToBase64) {
		result = base64_encode(bytesR);
	}
	return result;
}

tstring GetPvid(int len) {
	vector<tstring> randchar = { TEXT("0"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"),
		TEXT("a"), TEXT("b"), TEXT("c"), TEXT("d"), TEXT("e"), TEXT("f"), TEXT("g"), TEXT("h"), TEXT("i"), TEXT("j"), TEXT("k"), TEXT("l"), TEXT("m"), TEXT("n"), TEXT("o"), TEXT("p"), TEXT("q"), TEXT("r"), TEXT("s"), TEXT("t"), TEXT("u"), TEXT("v"), TEXT("w"), TEXT("x"), TEXT("y"), TEXT("z"),
		TEXT("A"), TEXT("B"), TEXT("C"), TEXT("D"), TEXT("E"), TEXT("F"), TEXT("G"), TEXT("H"), TEXT("I"), TEXT("J"), TEXT("K"), TEXT("L"), TEXT("M"), TEXT("N"), TEXT("O"), TEXT("P"), TEXT("Q"), TEXT("R"), TEXT("S"), TEXT("T"), TEXT("U"), TEXT("V"), TEXT("W"), TEXT("X"), TEXT("Y"), TEXT("Z")
	};
	int i;
	tstring r = TEXT("");
	time_t ts = time(0);
	default_random_engine random(static_cast<unsigned int>(time(0)));
	for (i = 0; i < len; i++) {
		long long a = static_cast<long long>(random() * pow(10, 6));
		int index = a % static_cast<int>(randchar.size());
		r += randchar[index];
	}
	return to_tstring(ts) + r;
}