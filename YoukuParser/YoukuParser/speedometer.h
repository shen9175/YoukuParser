#pragma once
class Speedometer {
public:
	Speedometer() { timer.Reset(); downloadedsize = 0; }
	void Reset() { timer.Reset(); downloadedsize = 0; }
	bool NeedUpdate(tstring& newspeed);
	void FeedNewSize(size_t length) { downloadedsize += length; }
private:
	size_t downloadedsize;
	Timer timer;
};