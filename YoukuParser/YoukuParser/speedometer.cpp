#include"stdafx.h"

bool Speedometer::NeedUpdate(tstring& speedisp) {
	double elapsetime = timer.ElapsedTime();
	if (elapsetime >= 1.0f) {
		double speed = downloadedsize / elapsetime;
		downloadedsize = 0;
		timer.Reset();
		tstringstream temp;
		temp << fixed << std::setprecision(2);
		assert(speed >= 0);
		if (speed < 1024) {
			temp << speed << TEXT(" B/s");//bytes/s
		} else if (speed >= 1024 && speed < 1024 * 1024) {
			speed = speed / 1024;//KB/s
			temp << speed << TEXT(" KB/s");
		} else if (speed >= 1024 * 1024 && speed < 1024 * 1024 * 1024) {
			speed = speed / (1024 * 1024);//MB/s
			temp << speed << TEXT(" MB/s");
		} else {
			speed = speed / (1024 * 1024 * 1024); //GB/s
			temp << speed << TEXT(" GB/s");
		}
		speedisp = temp.str();
		return true;
	}
	return false;
}