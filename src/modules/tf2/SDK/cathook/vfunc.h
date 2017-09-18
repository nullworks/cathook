/*
 * vfunc.h
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#ifndef VFUNC_H_
#define VFUNC_H_

template<typename F>
inline F vfunc(void* thisptr, uintptr_t idx, uintptr_t offset = 0) {
	void** vmt = *reinterpret_cast<void***>(uintptr_t(thisptr) + offset);
	return reinterpret_cast<F>((vmt)[idx]);
}

#endif /* VFUNC_H_ */
