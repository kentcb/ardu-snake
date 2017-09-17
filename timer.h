#ifndef TIMER_H
#define TIMER_H

#include "core/core.h"

class Timer {
	public:
		Timer(int ms);
		boolean hasExpired();

	private:
		int lastReading;
		int ms;
};

#endif
