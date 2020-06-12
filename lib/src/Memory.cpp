#include "Memory.h"
#include "FreelistAlloc.h"
#include "LinkList.h"

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
	void AddStatInfo(LinklistNode<MemAllocTraceInfo> *pStatInfo) {
		std::lock_guard<std::mutex> slock(m_lock);
		m_list.PushBack(pStatInfo);
	}
	void RemoveStatInfo(LinklistNode<MemAllocTraceInfo> *pStatInfo) {
		std::lock_guard<std::mutex> slock(m_lock);
		m_list.Remove(pStatInfo);
	}
};

class CMemoryWrapper {
public:
	CFreelistAlloc m_Heap;
#ifdef TRACE_MEMORY_ALLOCATION
	MemAllocStats m_MemStats;
#endif
	~CMemoryWrapper() {
#ifdef TRACE_MEMORY_ALLOCATION
		ReportMemleaks();
#endif
	}
} g_MemoryWrapper;

void* MemAllocWithTrace(size_t sz, const char *source, dword line) {
	dword nAlignedTraceInfo = ALIGN_SIZE(sizeof(MemAllocTraceInfo));
	byte *ptr = (byte *)g_MemoryWrapper.m_Heap.Allocate(sz + nAlignedTraceInfo);
	if (ptr) {
		MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)ptr;
		memset(pInfo, 0, sizeof(MemAllocTraceInfo));
		strcpy(pInfo->m_szSource, source);
		pInfo->m_nSize = sz;
		pInfo->m_nLine = line;
		pInfo->m_node.m_pOwner = pInfo;

		g_MemoryWrapper.m_MemStats.AddStatInfo(&pInfo->m_node);

		return ptr + nAlignedTraceInfo;
	}
	return nullptr;
}

// void* MemReallocWithTrace(void *ptr, size_t sz, const char *source, dword line) {
// 	dword nAlignedTraceInfo = ALIGN_SIZE(sizeof(MemAllocTraceInfo));
// 	MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)((byte*)ptr - nAlignedTraceInfo);
// 	g_MemoryWrapper.m_MemStats.RemoveStatInfo(&pInfo->m_node);
// 	g_MemoryWrapper.m_Heap.Free((void *)pInfo);
// 	byte *newPtr = (byte *)g_MemoryWrapper.m_Heap.Allocate(sz + nAlignedTraceInfo);
// 	if (newPtr) {
// 		MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)newPtr;
// 		strcpy(pInfo->m_szSource, source);
// 		pInfo->m_nSize = sz;
// 		pInfo->m_nLine = line;
// 		pInfo->m_node.m_pOwner = pInfo;

// 		g_MemoryWrapper.m_MemStats.AddStatInfo(&pInfo->m_node);

// 		return newPtr + nAlignedTraceInfo;
// 	}
// 	return nullptr;
// }

void* MemAlloc(size_t sz) {
	return g_MemoryWrapper.m_Heap.Allocate((dword)sz);
}

// void* MemRealloc(void *ptr, size_t sz) {
// 	g_MemoryWrapper.m_Heap.Free(ptr);
// 	return g_MemoryWrapper.m_Heap.Allocate((dword)sz);
// }

void MemFree(void *ptr) {
#ifdef TRACE_MEMORY_ALLOCATION
	dword nAlignedTraceInfo = ALIGN_SIZE(sizeof(MemAllocTraceInfo));
	MemAllocTraceInfo *pInfo = (MemAllocTraceInfo *)((byte*)ptr - nAlignedTraceInfo);
	g_MemoryWrapper.m_MemStats.RemoveStatInfo(&pInfo->m_node);
	g_MemoryWrapper.m_Heap.Free((void *)pInfo);
#else
	g_MemoryWrapper.m_Heap.Free(ptr);
#endif
}

void ReportMemleaks() {

#if COMPILER == COMPILER_MSVC
#define DbgPrint(info) OutputDebugStringA(info)
#endif

// #if TARGET_PLATFORM == PLATFORM_ANDROID
// #define DbgPrint(info) __android_log_print(ANDROID_LOG_INFO, "com.zy.renderer", "%s\n", info);
// #endif

	std::lock_guard<std::mutex> slock(g_MemoryWrapper.m_MemStats.m_lock);

	Linklist<MemAllocTraceInfo>::_NodeType *tmp = g_MemoryWrapper.m_MemStats.m_list.m_pRoot;
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

CFrameAllocator *g_pFrameAllocator = nullptr;
CFrameAllocator::CFrameAllocator()
{
	m_pFirstSection = NewObject<Section>();
	m_pCurrentSection = m_pFirstSection;
	g_pFrameAllocator = this;
}

CFrameAllocator::~CFrameAllocator()
{
	Section *pSection = m_pFirstSection;
	while (pSection != nullptr)
	{
		Section *pTemp = pSection->m_pNext;
		DeleteObject<Section>(pSection);
		pSection = pTemp;
	}
	m_pFirstSection = m_pCurrentSection = nullptr;
	g_pFrameAllocator = nullptr;
}

byte* CFrameAllocator::Allocate(dword nSize)
{
	dword nAlignedSize = ALIGN_SIZE(nSize);
	assert(nAlignedSize < SectionSize);
	if (!m_pCurrentSection->CanAllocate(nAlignedSize))
	{
		Section *pNewSection = NewObject<Section>();
		m_pCurrentSection->m_pNext = pNewSection;
		m_pCurrentSection = pNewSection;
	}
	return m_pCurrentSection->Allocate(nAlignedSize);
}

void CFrameAllocator::Reset()
{
	Section *pSection = m_pFirstSection;
	while (pSection != nullptr)
	{
		pSection->Reset();
		pSection = pSection->m_pNext;
	}
	m_pCurrentSection = m_pFirstSection;
}