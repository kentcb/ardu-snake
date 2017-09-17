#include "timer.h"

Timer::Timer(int ms) {
	this->ms = ms;
	this->lastReading = millis();
}

boolean Timer::hasExpired() {
	int currentMs = millis();

	if (currentMs - this->lastReading >= this->ms) {
		this->lastReading = currentMs;
		return true;
	}

	return false;
}
