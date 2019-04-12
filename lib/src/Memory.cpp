#include "Memory.h"
#include "FreelistAlloc.h"
#include "LinkList.h"
#include <malloc.h>

// void* operator new (size_t sz, const char *pszFile, int line) {
// 	return MemAllocWithTrace(sz, pszFile, line);
// }

// void* operator new (size_t sz) {
// 	return MemAlloc(sz);
// }

// void* operator new[] (size_t sz, const char *pszFile, int line) {
// 	return MemAllocWithTrace(sz, pszFile, line);
// }

// void* operator new[] (size_t sz) {
// 	return MemAlloc(sz);
// }

// void operator delete (void *ptr) {
// 	MemFree(ptr);
// }

// void operator delete[] (void *ptr) {
// 	MemFree(ptr);
// }

// #define INTERNAL_MALLOC(size) malloc(size)
// #define INTERNAL_REALLOC(ptr, size) realloc(ptr, size);
// #define INTERNAL_FREE(ptr) free(ptr)

CFreelistAlloc g_Heap;

// #include "Threading.h"

struct MemAllocTraceInfo {
	char m_szSource[CSTR_MAX];
	dword m_nLine;
	dword m_nSize;
	LinklistNode<MemAllocTraceInfo> m_node;
};

struct MemAllocStats {
	std::mutex m_lock;
	Linklist<MemAllocTraceInfo> m_list;
};
MemAllocStats g_MemStats;

void* MemAllocWithTrace(size_t sz, const char *source, dword line) {
	dword nAlignedTraceInfo = ALIGN_SIZE(sizeof(MemAllocTraceInfo));
	byte *ptr = (byte *)g_Heap.Allocate(sz + nAlignedTraceInfo);
	if (ptr) {
		MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)ptr;
		memset(pInfo, 0, sizeof(MemAllocTraceInfo));
		strcpy(pInfo->m_szSource, source);
		pInfo->m_nSize = sz;
		pInfo->m_nLine = line;
		pInfo->m_node.m_pOwner = pInfo;

		g_MemStats.m_lock.lock();
		g_MemStats.m_list.PushBack(&pInfo->m_node);
		g_MemStats.m_lock.unlock();

		return ptr + nAlignedTraceInfo;
	}
	return nullptr;
}

void* MemReallocWithTrace(void *ptr, size_t sz, const char *source, dword line) {
	dword nAlignedTraceInfo = ALIGN_SIZE(sizeof(MemAllocTraceInfo));
	MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)((byte*)ptr - nAlignedTraceInfo);
	g_MemStats.m_lock.lock();
	g_MemStats.m_list.Remove(&pInfo->m_node);
	g_MemStats.m_lock.unlock();

	g_Heap.Free((void *)pInfo);
	byte *newPtr = (byte *)g_Heap.Allocate(sz + nAlignedTraceInfo);
	if (newPtr) {
		MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)newPtr;
		strcpy(pInfo->m_szSource, source);
		pInfo->m_nSize = sz;
		pInfo->m_nLine = line;
		pInfo->m_node.m_pOwner = pInfo;

		g_MemStats.m_lock.lock();
		g_MemStats.m_list.PushBack(&pInfo->m_node);
		g_MemStats.m_lock.unlock();

		return newPtr + nAlignedTraceInfo;
	}
	return nullptr;
}

void* MemAlloc(size_t sz) {
	return g_Heap.Allocate((dword)sz);
}

void* MemRealloc(void *ptr, size_t sz) {
	g_Heap.Free(ptr);
	return g_Heap.Allocate((dword)sz);
}

void MemFree(void *ptr) {
#ifdef TRACE_MEMORY_ALLOCATION
	dword nAlignedTraceInfo = ALIGN_SIZE(sizeof(MemAllocTraceInfo));
	MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)((byte*)ptr - nAlignedTraceInfo);
	g_MemStats.m_lock.lock();
	g_MemStats.m_list.Remove(&pInfo->m_node);
	g_MemStats.m_lock.unlock();
	g_Heap.Free((void *)pInfo);
#else
	g_Heap.Free(ptr);
#endif
}

void ReportMemleaks() {

#if COMPILER == COMPILER_MSVC
#define DbgPrint(info) OutputDebugStringA(info)
#endif

// #if TARGET_PLATFORM == PLATFORM_ANDROID
// #define DbgPrint(info) __android_log_print(ANDROID_LOG_INFO, "com.zy.renderer", "%s\n", info);
// #endif

	std::lock_guard<std::mutex> slock(g_MemStats.m_lock);

	Linklist<MemAllocTraceInfo>::_NodeType *tmp = g_MemStats.m_list.m_pRoot;
	if (tmp == nullptr) {
		DbgPrint("No memory leaks\n");
		return;
	}

	DbgPrint("Detected memory leaks:\n");
	char output[CSTR_MAX];
	while (tmp) {
		MemAllocTraceInfo *pInfo = tmp->m_pOwner;
		sprintf(output, "File: %s Line: %u Size: %u\n", pInfo->m_szSource, pInfo->m_nLine, pInfo->m_nSize);
		DbgPrint(output);
		tmp = tmp->m_pNext;
	}
}
