#include "stdafx.h"

ConsoleStream::~ConsoleStream() {
	for (auto item : numbers) {
		if (item) {
			delete item;
		}
	}
}
 ConsoleStream& operator<<(ConsoleStream& os, const tstring& text) {
	 //tstring output = os.output.str();
	 os.output << text;
	 size_t ndx = GetWindowTextLength(os.display);
	 SetFocus(os.display);
	 SendMessage(os.display, EM_SETSEL, static_cast<WPARAM>(ndx), static_cast<LPARAM>(ndx));//put the cursor in the end
	 SendMessage(os.display, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(&text[0]));

	 /*
	 SetWindowText(os.display, os.output.str().c_str());
	 SendMessage(os.display, EM_SETSEL, 0, -1);
	 SendMessage(os.display, EM_SETSEL, -1, -1);
	 SendMessage(os.display, EM_SCROLLCARET, 0, 0);
	 */
	 InvalidateRect(os.display, nullptr, true);
	 return os;
}
 ConsoleStream& operator<<(ConsoleStream& os, const int& number) {
	 os.output << number;
	 tstring* temp = new tstring(to_tstring(number));
	 os.numbers.push_back(temp);
	 size_t ndx = GetWindowTextLength(os.display);
	 SetFocus(os.display);
	 SendMessage(os.display, EM_SETSEL, static_cast<WPARAM>(ndx), static_cast<LPARAM>(ndx));//put the cursor in the end
	 SendMessage(os.display, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(&(*temp)[0]));
	 /*
	 SetWindowText(os.display, os.output.str().c_str());
	 SendMessage(os.display, EM_SETSEL, 0, -1);
	 SendMessage(os.display, EM_SETSEL, -1, -1);
	 SendMessage(os.display, EM_SCROLLCARET, 0, 0);
	 */
	 InvalidateRect(os.display, nullptr, true);
	 return os;
 }
// for << endl-->endl is a function pointer of type totream&(*pf)(tostream&), pf is the function pointer variable identifier
 //see this: http://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator

 ConsoleStream& operator<<(ConsoleStream& os, tostream& (*endline)(tostream&)) {
	 os.output << '\r' <<endline; //need \r\n to get a new line in edit control. endl only have \n
	 size_t ndx = GetWindowTextLength(os.display);
	 SetFocus(os.display);
	 SendMessage(os.display, EM_SETSEL, static_cast<WPARAM>(ndx), static_cast<LPARAM>(ndx));//put the cursor in the end
	 SendMessage(os.display, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(TEXT("\r\n")));
	 /*
	 SetWindowText(os.display, os.output.str().c_str());
	 SendMessage(os.display, EM_SETSEL, 0, -1);
	 SendMessage(os.display, EM_SETSEL, -1, -1);
	 SendMessage(os.display, EM_SCROLLCARET, 0, 0);
	 */
	 InvalidateRect(os.display, nullptr, true);
	 return os;
 }

 ConsoleStream& operator||(ConsoleStream& os, const tstring& text) {
	 int start = static_cast<int>(SendMessage(os.display, EM_LINEINDEX, -1, 0));
	 int length = static_cast<int>(SendMessage(os.display, EM_LINELENGTH, start, 0));
	 int end = start + length;
	 SendMessage(os.display, EM_SETSEL, end, end);
	 //int idx = GetWindowTextLength(os.display);
	// SendMessage(os.display, EM_SETSEL, static_cast<WPARAM>(idx), static_cast<LPARAM>(idx));
	 SendMessage(os.display, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(&text[0]));
	 InvalidateRect(os.display, nullptr, true);
	 return os;
 }
 ConsoleStream& operator||(ConsoleStream& os, const int& number) {
	 os.tempnumber = to_tstring(number);
	 int start = static_cast<int>(SendMessage(os.display, EM_LINEINDEX, -1, 0));
	 int length = static_cast<int>(SendMessage(os.display, EM_LINELENGTH, start, 0));
	 int end = start + length;
	 SendMessage(os.display, EM_SETSEL, end, end);
	// int idx = GetWindowTextLength(os.display);
	// SendMessage(os.display, EM_SETSEL, static_cast<WPARAM>(idx), static_cast<LPARAM>(idx));
	 SendMessage(os.display, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(&os.tempnumber[0]));
	 InvalidateRect(os.display, nullptr, true);
	 return os;
 }
 // for << endl-->endl is a function pointer of type totream&(*pf)(tostream&), pf is the function pointer variable identifier
 //see this: http://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator

 ConsoleStream& operator||(ConsoleStream& os, tostream& (*endline)(tostream&)) {
	 /*
	 tstring output = os.output.str();
	 size_t pos = output.find_last_of(TEXT("\r\n"));
	 if ( pos != tstring::npos) {
		 output = output.substr(0, pos + 1);
	 }
	 //os.output.str(output);
	 os.output.str(tstring());
	 os.output << output;
	 SetWindowText(os.display, os.output.str().c_str());
	 SendMessage(os.display, EM_SETSEL, 0, -1);
	 SendMessage(os.display, EM_SETSEL, -1, -1);
	 SendMessage(os.display, EM_SCROLLCARET, 0, 0);
	 InvalidateRect(os.display, nullptr, true);
	 return os;
	 */
	 size_t ndx = GetWindowTextLength(os.display);
	 SetFocus(os.display);
	 SendMessage(os.display, EM_SETSEL, static_cast<WPARAM>(ndx), static_cast<LPARAM>(ndx));//put the cursor in the end
	 int start = static_cast<int>(SendMessage(os.display, EM_LINEINDEX, -1, 0));
	 int length = static_cast<int>(SendMessage(os.display, EM_LINELENGTH, start, 0));
	 int end = start + length;
	 SendMessage(os.display, EM_SETSEL, start, end);//select the last line
	 SendMessage(os.display, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(TEXT("")));//replace with empty string
	 InvalidateRect(os.display, nullptr, true);
	 return os;
 }

 //another approach to append text to edit control: http://stackoverflow.com/questions/15446969/add-a-new-line-using-setwindowtext-function