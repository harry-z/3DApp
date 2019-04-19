#pragma once

#include "PreHeader.h"

extern "C" {
	DLL_EXPORT void* MemAllocWithTrace(size_t sz, const char *source, dword line);
	DLL_EXPORT void* MemAlloc(size_t sz);
	DLL_EXPORT void MemFree(void *ptr);
	DLL_EXPORT void ReportMemleaks();
};

template <class T>
inline T* ConstructNObj(T *p, dword n) {
	for (dword i = 0; i < n; ++i)
		new ((void *)(p + i)) T();
	return p;
}

#ifdef TRACE_MEMORY_ALLOCATION
	#define NEW_TYPE(T) new (MemAllocWithTrace(sizeof(T), __FILE__, __LINE__)) T
	#define DELETE_TYPE(ptr, T) if (ptr) { (ptr)->~T(); MemFree(ptr); }
	#define NEW_TYPE_ARRAY(T, n) ConstructNObj<T>((T *)MemAllocWithTrace(sizeof(T) * n, __FILE__, __LINE__), n)
	#define DELETE_TYPE_ARRAY(ptr, T, n) if (ptr) { for (dword i=0; i<n; ++i) { (ptr)[i].~T(); } MemFree(ptr); }
	#define MEMALLOC(sz) MemAllocWithTrace(sz, __FILE__, __LINE__)
	#define MEMFREE(ptr) MemFree(ptr)
#else
	#define NEW_TYPE(T) new (MemAlloc(sizeof(T))) T
	#define DELETE_TYPE(ptr, T) if (ptr) { (ptr)->~T(); MemFree(ptr); }
	#define NEW_TYPE_ARRAY(T, n) ConstructNObj<T>(MemAlloc(sizeof(T) * n), n)
	#define DELETE_TYPE_ARRAY(ptr, T, n) if (ptr) { for (dword i=0; i<n; ++i) { (ptr)[i].~T(); } MemFree(ptr); }
	#define MEMALLOC(sz) MemAlloc(sz)
	#define MEMFREE(ptr) MemFree(ptr)
#endif
