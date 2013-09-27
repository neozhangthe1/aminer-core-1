#pragma once
#include "MemGC.h"

// XXX should be replaced with a correct version.
inline void * InterlockedExchangePointer(void * volatile * target, void* value) {
    void * volatile original = *target;
    *target = value;
    return original;
}

template<typename T>
inline T* xchg_ptr(T* volatile * target, T* value)
{
	return reinterpret_cast<T*>(InterlockedExchangePointer(reinterpret_cast<void *volatile *>(target), value));
}

template<typename T>
inline void assign_ptr(T* volatile * target, T* value)
{
	MemGC::instance().addGarbage(new DeleterImp<T>(xchg_ptr(target, value)));
}
