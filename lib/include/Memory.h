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
		new ((void *)(pbytes + size * i)) T();
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

template <class ObjType, class ... ArgsType>
inline ObjType* NewObject(ArgsType &&... Args)
{
	return NEW_TYPE(ObjType)(std::forward<ArgsType>(Args)...);
}

template <class ObjType>
inline void DeleteObject(ObjType *pObj)
{
	DELETE_TYPE(pObj, ObjType);
}

template <class ObjType>
inline ObjType* NewObjectArray(dword nCount)
{
	ObjType *pMem = (ObjType *)MEMALLOC(sizeof(ObjType) * nCount);
	ConstructItems(pMem, nCount);
	return pMem;
}

template <class ObjType>
inline void DeleteObjectArray(ObjType *pObjs, dword nCount)
{
	DestructItems(pObjs, nCount);
	MEMFREE(pObjs);
}

template <class T>
inline typename TEnableIf<!TIsZeroContructType<T>::Value>::Type ConstructItems(T *pData, dword nCount)
{
	while (nCount)
	{
		new (pData) T;
		++pData;
		--nCount;
	}
}

template <class T>
inline typename TEnableIf<TIsZeroContructType<T>::Value>::Type ConstructItems(T *pData, dword nCount)
{
	memset(pData, 0, sizeof(T) * nCount);
}

template <class T>
inline typename TEnableIf<!TIsTriviallyDestructible<T>::Value>::Type DestructItems(T *pData, dword nCount)
{
	while (nCount)
	{
		pData->~T();
		++pData;
		--nCount;
	}
}

template <class T>
inline typename TEnableIf<TIsTriviallyDestructible<T>::Value>::Type DestructItems(T *pData, dword nCount)
{}

class CFrameAllocator
{
public:
	CFrameAllocator();
	~CFrameAllocator();

	byte* Allocate(dword nSize);
	void Reset();

private:
	static const dword SectionSize = 512 * 1024;
	struct Section
	{
		Section *m_pNext = nullptr;
		byte *m_pMemory = nullptr;
		dword m_nCurrentSize = 0;
		Section() {
			m_pMemory = (byte *)malloc(SectionSize);
		}
		~Section() {
			free(m_pMemory);
		}
		bool CanAllocate(dword nSize) const {
			return SectionSize - m_nCurrentSize > nSize;
		}
		byte* Allocate(dword nSize) {
			m_nCurrentSize += nSize;
			return m_pMemory + m_nCurrentSize;
		}
		void Reset() { m_nCurrentSize = 0; }
	};
	Section *m_pFirstSection;
	Section *m_pCurrentSection;
};
extern CFrameAllocator *g_pFrameAllocator;