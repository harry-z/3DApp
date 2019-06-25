#pragma once

#include "PreHeader.h"

extern "C" {
	DLL_EXPORT void* MemAllocWithTrace(size_t sz, const char *source, dword line);
	DLL_EXPORT void* MemAlloc(size_t sz);
	DLL_EXPORT void MemFree(void *ptr);
	DLL_EXPORT void ReportMemleaks();
};

template <class T>
inline T* ConstructNObj(void *p, dword size, dword n) {
	byte *pbytes = (byte *)p;
	for (dword i = 0; i < n; ++i)
		new ((void *)(pbytes + size)) T();
	return (T *)pbytes;
}

#ifdef TRACE_MEMORY_ALLOCATION
	#define NEW_TYPE(T) new (MemAllocWithTrace(sizeof(T), __FILE__, __LINE__)) T
	#define DELETE_TYPE(ptr, T) if (ptr) { (ptr)->~T(); MemFree(ptr); }
	#define NEW_TYPE_ARRAY(T, n) ConstructNObj<T>(MemAllocWithTrace(ALIGN_SIZE(sizeof(T)) * n, __FILE__, __LINE__), ALIGN_SIZE(sizeof(T)), n)
	#define DELETE_TYPE_ARRAY(ptr, T, n) if (ptr) { for (dword i=0; i<n; ++i) { (ptr)[i].~T(); } MemFree(ptr); }
	#define MEMALLOC(sz) MemAllocWithTrace(sz, __FILE__, __LINE__)
	#define MEMFREE(ptr) MemFree(ptr)
#else
	#define NEW_TYPE(T) new (MemAlloc(sizeof(T))) T
	#define DELETE_TYPE(ptr, T) if (ptr) { (ptr)->~T(); MemFree(ptr); }
	#define NEW_TYPE_ARRAY(T, n) ConstructNObj<T>(MemAlloc(ALIGN_SIZE(sizeof(T)) * n), ALIGN_SIZE(sizeof(T)), n)
	#define DELETE_TYPE_ARRAY(ptr, T, n) if (ptr) { for (dword i=0; i<n; ++i) { (ptr)[i].~T(); } MemFree(ptr); }
	#define MEMALLOC(sz) MemAlloc(sz)
	#define MEMFREE(ptr) MemFree(ptr)
#endif
