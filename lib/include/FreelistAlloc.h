#pragma once
#include "PreHeader.h"
#include "LinkList.h"



class CFreelistAlloc {
public:
    CFreelistAlloc();
    ~CFreelistAlloc();

    void* Allocate(dword bytes);
    void Free(void *p);

    struct Page
    {
        void *pData;
        void *pFirstFree;
        Page *pPrev;
        Page *pNext;
        dword nDataSize;
        dword nLargetFree;
    };

    struct MediumHeapEntry
    {
        Page *pPage;
        MediumHeapEntry *Prev; // 
        MediumHeapEntry *Next; // 
        MediumHeapEntry *PrevFree; // 
        MediumHeapEntry *NextFree; // 
        dword nSize; // 
        dword nFreeBlock; // 0 - Used Entry，1 - Free Entry
    };

private:
    enum {
        INVALID_ALLOC = 0xdd,
        SMALL_ALLOC = 0xaa,
        MEDIUM_ALLOC = 0xbb,
        LARGE_ALLOC = 0xcc
    };

    void* SmallAllocate(dword bytes);
    void SmallFree(void *p);
    void* MediumAllocate(dword bytes);
    void* MediumAllocateFromPage(Page *pPage, dword nSizeNeeded);
    void MediumFree(void *p);
    void* LargeAllocate(dword bytes);
    void LargeFree(void *p);

    Page* AllocatePage(dword nPageSize);
    void FreePage(Page *pPage);
    void FreePageReal(Page *pPage);

    // 
    void * m_ppSmallFirstFree[256/ALIGN];
    // 
    Page * m_pSmallCurPage;
    // 
    Page * m_pSmallFirstUsedPage;

    // 
    Page * m_pMediumFirstFreePage;
    // 
    Page * m_pMediumLastFreePage;
    // 
    Page * m_pMediumFirstUsedPage;
    // 
    Page * m_pSwapPage;

    // 
    Page * m_pLargeFirstUsedPage;

    dword m_nPageSize;
    dword m_nPageAllocated;
    dword m_nPageRequest;
    dword m_nSmallCurPageOffset;
    dword m_nOSAllocs;

    std::mutex m_Memlock;
};