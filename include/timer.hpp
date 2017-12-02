/*
 * timer.hpp
 *
 *  Created on: Jul 29, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <chrono>

class Timer {
public:
	typedef std::chrono::system_clock clock;

	inline Timer() {};

	inline bool check(unsigned ms) const {
		return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - last).count() >= ms;
	}
	inline bool test_and_set(unsigned ms) {
		if (check(ms)) {
			update();
			return true;
		}
		return false;
	}
	inline void update() {
		last = clock::now();
	}

public:
	std::chrono::time_point<clock> last {};
};


