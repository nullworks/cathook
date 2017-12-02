/*
 * averager.hpp
 *
 *  Created on: May 26, 2017
 *      Author: nullifiedcat
 */

#ifndef AVERAGER_HPP_
#define AVERAGER_HPP_

#include <cstddef>
#include <vector>
#include <initializer_list>
#include <string>
#include <memory>

/*
 * Averager class.
 * 	Can return average of last N pushed values, older values are discarded.
 */

template<typename T>
class Averager {
public:
	/*
	 * Initialize Averager with a given size
	 */
	inline Averager(size_t _size) {
		resize(_size);
	}
	/*
	 * Resize the internal array of Averager
	 * 	Changes value_count_ and value_index_ so they don't become Out-Of-Bounds
	 */
	inline void resize(size_t _size) {
		size_ = _size;
		values_.resize(_size);
		value_count_ = std::min(_size, value_count_);
		value_index_ = std::min(_size - 1, value_index_);
	}
	/*
	 * Resets Averager state
	 */
	inline void reset(const T& def = 0.0f) {
		value_count_ = 0;
		value_index_ = 0;
		value_ = def;
	}
	/*
	 * Pushes a new value and recalculates the sum, discards oldest value
	 */
	inline void push(T value) {
		if (value_count_ > value_index_) value_ -= values_[value_index_];
		values_[value_index_++] = value;
		value_ += value;
		if (value_index_ > value_count_) {
			value_count_ = value_index_;
		}
		if (value_index_ >= size_) {
			value_index_ = 0;
		}
	}
	/*
	 * Pushes all values in initializer list
	 */
	inline void push(std::initializer_list<T> list) {
		for (const auto& f : list) {
			push(f);
		}
	}
	/*
	 * Returns the average value, returns 0 if no values were pushed
	 */
	inline T average() const {
		if (value_count_)
			return value_ / value_count_;
		else return T{};
	}
	/*
	 * Size of internal buffer
	 */
	inline size_t size() const {
		return size_;
	}
	/*
	 * Amount of values stored in the buffer
	 */
	inline size_t value_count() const {
		return value_count_;
	}
private:
	std::vector<T> values_;
	size_t size_ { 0 };
	size_t value_count_ { 0 };
	size_t value_index_ { 0 };
	T value_ {};
};


#endif /* AVERAGER_HPP_ */
