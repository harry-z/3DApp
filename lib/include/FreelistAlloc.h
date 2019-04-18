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
        MediumHeapEntry *Prev; // 前一个Entry
        MediumHeapEntry *Next; // 下一个Entry
        MediumHeapEntry *PrevFree; // 前一个Free Entry，当Entry为已分配时为nullptr
        MediumHeapEntry *NextFree; // 后一个Free Entry，当Entry为已分配时为nullptr
        dword nSize; // 整个Entry的大小
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

    // SmallAllocate的Free链表
    void * m_ppSmallFirstFree[256/ALIGN];
    // SmallAllocate当前正在使用的页
    Page * m_pSmallCurPage;
    // SmallAllocate第一个已经被占满的页
    Page * m_pSmallFirstUsedPage;

    // MediumAllocate第一个未使用的页
    Page * m_pMediumFirstFreePage;
    // MediumAllocate最后一个未使用页
    Page * m_pMediumLastFreePage;
    // MediumAllocate第一个使用页
    Page * m_pMediumFirstUsedPage;
    // MediumAllocate临时交换页
    Page * m_pSwapPage;

    // LargeAllocate第一个使用页
    Page * m_pLargeFirstUsedPage;

    dword m_nPageSize;
    dword m_nPageAllocated;
    dword m_nPageRequest;
    dword m_nSmallCurPageOffset;
    dword m_nOSAllocs;

    std::mutex m_Memlock;
};