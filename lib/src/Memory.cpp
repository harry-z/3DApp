#include "Memory.h"
#include "LinkList.h"

#include <malloc.h>
#define INTERNAL_MALLOC(size) malloc(size)
#define INTERNAL_REALLOC(ptr, size) realloc(ptr, size);
#define INTERNAL_FREE(ptr) free(ptr)



// #include "Threading.h"

struct MemAllocTraceInfo {
	char m_szSource[CSTR_MAX];
	size_t m_nLine;
	size_t m_nSize;
	LinklistNode<MemAllocTraceInfo> m_node;
};

struct MemAllocStats {
	std::mutex m_lock;
	Linklist<MemAllocTraceInfo> m_list;
};
MemAllocStats g_MemStats;

void* MemAllocWithTrace(size_t sz, const char *source, dword line) {
	size_t total = sz + sizeof(MemAllocTraceInfo);
	total = ALIGN_SIZE(total);
	byte *ptr = (byte *)INTERNAL_MALLOC(total);
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

		return ptr + sizeof(MemAllocTraceInfo);
	}
	return nullptr;
}

void* MemReallocWithTrace(void *ptr, size_t sz, const char *source, dword line) {
	MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)((byte*)ptr - sizeof(MemAllocTraceInfo));
	g_MemStats.m_lock.lock();
	g_MemStats.m_list.Remove(&pInfo->m_node);
	g_MemStats.m_lock.unlock();

	size_t total = sz + sizeof(MemAllocTraceInfo);
	total = ALIGN_SIZE(total);
	byte *newPtr = (byte *)INTERNAL_REALLOC(pInfo, total);
	if (newPtr) {
		MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)newPtr;
		strcpy(pInfo->m_szSource, source);
		pInfo->m_nSize = sz;
		pInfo->m_nLine = line;
		pInfo->m_node.m_pOwner = pInfo;

		g_MemStats.m_lock.lock();
		g_MemStats.m_list.PushBack(&pInfo->m_node);
		g_MemStats.m_lock.unlock();

		return newPtr + sizeof(MemAllocTraceInfo);
	}
	return nullptr;
}

void* MemAlloc(size_t sz) {
    return INTERNAL_MALLOC(ALIGN_SIZE(sz));
}

void* MemRealloc(void *ptr, size_t sz) {
    return INTERNAL_REALLOC(ptr, ALIGN_SIZE(sz));
}

void MemFree(void *ptr) {
#ifdef TRACE_MEMORY_ALLOCATION
	MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)((byte*)ptr - sizeof(MemAllocTraceInfo));
	g_MemStats.m_lock.lock();
	g_MemStats.m_list.Remove(&pInfo->m_node);
	g_MemStats.m_lock.unlock();
	INTERNAL_FREE(pInfo);
#else
	INTERNAL_FREE(ptr);
#endif
}

void ReportMemleaks() {

#if COMPILER == COMPILER_MSVC
#define DbgPrint(info) OutputDebugStringA(info)
#endif

// #if TARGET_PLATFORM == PLATFORM_ANDROID
// #define DbgPrint(info) __android_log_print(ANDROID_LOG_INFO, "com.zy.renderer", "%s\n", info);
// #endif

	Linklist<MemAllocTraceInfo>::_NodeType *tmp = g_MemStats.m_list.m_pRoot;
	if (tmp == nullptr) {
		DbgPrint("No memory leaks\n");
		return;
	}

	DbgPrint("Detected memory leaks:\n");
	char output[256];
	while (tmp) {
		MemAllocTraceInfo *pInfo = tmp->m_pOwner;
		sprintf(output, "File: %s Line: %u Size: %u\n", pInfo->m_szSource, pInfo->m_nLine, pInfo->m_nSize);
		DbgPrint(output);
		tmp = tmp->m_pNext;
	}
}
