#pragma once
class ConsoleStream {
	friend ConsoleStream& operator<<(ConsoleStream& os, const tstring& text);
	friend ConsoleStream& operator<<(ConsoleStream& os, const int& number);
	friend ConsoleStream& operator<<(ConsoleStream& os, tostream& (*endline)(tostream&));
	friend ConsoleStream& operator||(ConsoleStream& os, const tstring& text);
	friend ConsoleStream& operator||(ConsoleStream& os, const int& number);
	friend ConsoleStream& operator||(ConsoleStream& os, tostream& (*endline)(tostream&));
public:
	ConsoleStream(const HWND& hwnd) : display(hwnd) {}
	~ConsoleStream();
	void clearConsole() { output.str(tstring()); }
private:
	HWND display;
	tstringstream output;
	tstring tempnumber;
	vector<tstring*> numbers;
};

