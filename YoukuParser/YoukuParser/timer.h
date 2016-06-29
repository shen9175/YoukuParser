#pragma once
class Timer {
	public:
		Timer() { mDeltaTime = -1.0; mBaseTime = 0; mPausedTime = 0; mPrevTime = 0; mCurrTime = 0; mStopTime = 0; mStopped = false; mTicks = 0; mTotalTicks = 0; }
		~Timer() {}
		double ElapsedTime() const; //return total effective time of gaming--> current time called this function substract(-) the most recent time called Reset() then substract(-) the pause time during this time interval.
		double DeltaTime() const;//return total time (time interval) from last time called Tick() function and this time called DeltaTime() function
		void Reset();//restart a total new time record. The time called Reset() will be the baseTime(the most first/earliest time in this Timer usage round)
		void Resume();//resume recording time from Pause status. If the game is in the pause status(Stop() called before), then this function will resume the pause and continue to record the time. if it is not in pause status, nothing happens.
		void Pause();//Pause the time recording.
		void Tick();//every time called Tick(), it will record the new current time and update the previous time with old current time. And calculate the updated delta time. Frames increment.
		unsigned long long TotalTicks() const{ return mTotalTicks; } //return total ticks start from last Reset() called. usually used for frame count. each frame tick one time.
	private:
		double mDeltaTime; //time interval: total time (time interval) from last time called Tick() function and this time called DeltaTime() function
		clock_t mBaseTime;//time point: the most first/earliest time in this Timer usage round
		clock_t mPausedTime;//time interval: total time between all corresponding Pause()~Resume() interval (accumalted time intervals)
		clock_t mStopTime; //time point: the most recent time point when the functio Stop() called;
		clock_t mPrevTime; //last time point when Tick() function called
		clock_t mCurrTime; //the current time point when Tick() function called
		bool mStopped; // flag indicat if the timer is in pause status
		unsigned long long mTicks;//increment by 1 when Tick() called
		unsigned long long mTotalTicks;// total Tick() calls between two Reset() called. The second Reset() will record total ticks starting from last Reset() and keep it unchanged until next Reset(). It usually used for calculating average FPS.
};


