#include "FreelistAlloc.h"

// #define SMALL_HEADER_SIZE ( (int)(sizeof(byte) + sizeof(byte)) )
constexpr dword MEDIUM_PAGE_SIZE = 65536 - sizeof(CFreelistAlloc::Page);
constexpr dword SMALL_HEADER_SIZE = (dword)(sizeof(byte) * 2);
constexpr dword MEDIUM_HEADER_SIZE = (dword)(sizeof(CFreelistAlloc::MediumHeapEntry) + sizeof(byte));
constexpr dword MEDIUM_SMALLEST_SIZE = ALIGN_SIZE(256) + ALIGN_SIZE(MEDIUM_HEADER_SIZE);
constexpr dword LARGE_HEADER_SIZE = (dword)(sizeof(uintptr_t) + sizeof(byte));
#define SMALL_ALIGN(bytes) ( ALIGN_SIZE((bytes) + SMALL_HEADER_SIZE) - SMALL_HEADER_SIZE )

CFreelistAlloc::CFreelistAlloc()
: m_pSmallFirstUsedPage(nullptr)
, m_pMediumFirstFreePage(nullptr)
, m_pMediumLastFreePage(nullptr)
, m_pMediumFirstUsedPage(nullptr)
, m_pSwapPage(nullptr)
, m_pLargeFirstUsedPage(nullptr)
, m_nPageSize(MEDIUM_PAGE_SIZE)
, m_nPageAllocated(0)
, m_nPageRequest(0)
, m_nSmallCurPageOffset(0)
, m_nOSAllocs(0)
{
    memset(m_ppSmallFirstFree, 0, sizeof(m_ppSmallFirstFree));
    m_pSmallCurPage = AllocatePage(m_nPageSize);
}

CFreelistAlloc::~CFreelistAlloc()
{
    if (m_pSmallCurPage != nullptr)
        FreePage(m_pSmallCurPage);
    Page *p = m_pSmallFirstUsedPage;
    while (p)
    {
        Page *pNext = p->pNext;
        FreePage(p);
        p = pNext;
    }

    p = m_pLargeFirstUsedPage;
    while (p)
    {
        Page *pNext = p->pNext;
        FreePage(p);
        p = pNext;
    }

    p = m_pMediumFirstFreePage;
    while (p)
    {
        Page *pNext = p->pNext;
        FreePage(p);
        p = pNext;
    }

    p = m_pMediumFirstUsedPage;
    while (p)
    {
        Page *pNext = p->pNext;
        FreePage(p);
        p = pNext;
    }

    if (m_pSwapPage)
        FreePageReal(m_pSwapPage);
    m_pSwapPage = nullptr;
}

void* CFreelistAlloc::Allocate(dword bytes)
{
    std::lock_guard<std::mutex> slock(m_Memlock);
    if (bytes == 0)
        return nullptr;
    if ((bytes & ~255) == 0)
        return SmallAllocate(bytes);
    else if ((bytes & ~32767) == 0)
        return MediumAllocate(bytes);
    else
        return LargeAllocate(bytes);
}

void CFreelistAlloc::Free(void *p)
{
    assert(p != nullptr);
    std::lock_guard<std::mutex> slock(m_Memlock);
    switch (((byte*)p)[-1])
    {
        case SMALL_ALLOC:
            SmallFree(p);
            break;
        case MEDIUM_ALLOC:
            MediumFree(p);
            break;
        case LARGE_ALLOC:
            LargeFree(p);
            break;
        default:
            assert(0 && "Invalid memory block");
            break;
    }
}

void* CFreelistAlloc::SmallAllocate(dword bytes)
{
    if (bytes < ALIGN)
        bytes = ALIGN;
    bytes = SMALL_ALIGN(bytes);

    byte *pSmallBlock = (byte *)(m_ppSmallFirstFree[bytes/ALIGN - 1]);
    if (pSmallBlock != nullptr)
    {
        uintptr_t *pNextLink = (uintptr_t *)(pSmallBlock + SMALL_HEADER_SIZE);
        pSmallBlock[1] = SMALL_ALLOC;
        m_ppSmallFirstFree[bytes/ALIGN - 1] = (void *)(*pNextLink);
        return (void *)pNextLink;
    }

    dword nBytesLeft = m_nPageSize - m_nSmallCurPageOffset;
    if (bytes >= nBytesLeft)
    {
        // CurrentPage -> FirstUsedPage -> ... -> LastUsedPage
        m_pSmallCurPage->pNext = m_pSmallFirstUsedPage;
        m_pSmallFirstUsedPage = m_pSmallCurPage;
        m_pSmallCurPage = AllocatePage(m_nPageSize);
        assert(m_pSmallCurPage != nullptr);
        m_nSmallCurPageOffset = ALIGN_SIZE(0);
    }

    pSmallBlock = ((byte *)m_pSmallCurPage->pData) + m_nSmallCurPageOffset;
    pSmallBlock[0] = (byte)(bytes/ALIGN - 1);
    pSmallBlock[1] = SMALL_ALLOC;
    m_nSmallCurPageOffset += (bytes + SMALL_HEADER_SIZE);
    return pSmallBlock + SMALL_HEADER_SIZE;
}

void CFreelistAlloc::SmallFree(void *p)
{
    ((byte*)p)[-1] = INVALID_ALLOC;

    byte *pOrigin = ((byte *)p) - SMALL_HEADER_SIZE;
    uintptr_t *pAddress = (uintptr_t *)p;
    dword idx = *pOrigin;

    if (idx >= (256/ALIGN))
    {
        assert(0 && "SmallFree: Invalid memory block");
        return;
    }

    *pAddress = (uintptr_t)m_ppSmallFirstFree[idx];
    m_ppSmallFirstFree[idx] = pOrigin;
}

void* CFreelistAlloc::MediumAllocate(dword bytes)
{
    Page *pPage;

    dword nSizeNeeded = ALIGN_SIZE(bytes) + ALIGN_SIZE(MEDIUM_HEADER_SIZE);
    for (pPage = m_pMediumFirstFreePage; pPage; pPage = pPage->pNext)
    {
        if (pPage->nLargetFree >= nSizeNeeded)
            break;
    }

    if (pPage == nullptr)
    {
        pPage = AllocatePage(m_nPageSize);
        assert(pPage != nullptr);

        pPage->pPrev = nullptr;
        pPage->pNext = m_pMediumFirstFreePage;
        if (pPage->pNext != nullptr)
            pPage->pNext->pPrev = pPage;
        else
            m_pMediumLastFreePage = pPage;
        m_pMediumFirstFreePage = pPage;

        pPage->nLargetFree = m_nPageSize;
        pPage->pFirstFree = pPage->pData;

        MediumHeapEntry *e = (MediumHeapEntry *)(pPage->pFirstFree);
        e->pPage = pPage;
        e->nSize = m_nPageSize & ~(ALIGN - 1);
        e->nFreeBlock = 1;
        e->Prev = nullptr;
        e->Next = nullptr;
        e->PrevFree = nullptr;
        e->NextFree = nullptr;
    }

    void *pData = MediumAllocateFromPage(pPage, nSizeNeeded);

    if (pPage->nLargetFree < MEDIUM_SMALLEST_SIZE)
    {
        if (pPage == m_pMediumLastFreePage)
            m_pMediumLastFreePage = pPage->pPrev;
        if (pPage == m_pMediumFirstFreePage)
            m_pMediumFirstFreePage = pPage->pNext;

        if (pPage->pPrev)
            pPage->pPrev->pNext = pPage->pNext;
        if (pPage->pNext)
            pPage->pNext->pPrev = pPage->pPrev;

        pPage->pPrev = nullptr;
        pPage->pNext = m_pMediumFirstUsedPage;
        if (pPage->pNext)
            pPage->pNext->pPrev = pPage;
        m_pMediumFirstUsedPage = pPage;
        return pData;
    }

    if (pPage != m_pMediumFirstFreePage)
    {
        assert(m_pMediumLastFreePage);
        assert(m_pMediumFirstFreePage);
        assert(pPage->pPrev);

        // 
        m_pMediumLastFreePage->pNext = m_pMediumFirstFreePage;
        m_pMediumFirstFreePage->pPrev = m_pMediumLastFreePage;

        m_pMediumLastFreePage = pPage->pPrev;
        pPage->pPrev->pNext = nullptr;
        pPage->pPrev = nullptr;

        m_pMediumFirstFreePage = pPage;
    }

    return pData;
}

void* CFreelistAlloc::MediumAllocateFromPage(Page *pPage, dword nSizeNeeded)
{
    MediumHeapEntry *pBest = (MediumHeapEntry *)pPage->pFirstFree;
    assert(pBest != nullptr);
    assert(pBest->nSize == pPage->nLargetFree);
    assert(pBest->nSize >= nSizeNeeded);

    MediumHeapEntry *pNw = nullptr;

    if (pBest->nSize >= nSizeNeeded + MEDIUM_SMALLEST_SIZE)
    {
        // 
        // 
        pNw = (MediumHeapEntry *)(((byte *)pBest) + pBest->nSize - nSizeNeeded);
        pNw->pPage = pPage;
        pNw->Prev = pBest;
        pNw->Next = pBest->Next;
        pNw->PrevFree = pNw->NextFree = nullptr;
        pNw->nSize = nSizeNeeded;
        pNw->nFreeBlock = 0;
        if (pBest->Next != nullptr)
            pBest->Next->Prev = pNw;
        pBest->Next = pNw;
        pBest->nSize -= nSizeNeeded;

        pPage->nLargetFree = pBest->nSize;
    }
    else // 
    {
        // 
        // 
        if (pBest->PrevFree != nullptr)
            pBest->PrevFree->NextFree = pBest->NextFree;
        else
            pPage->pFirstFree = pBest->NextFree;
        if (pBest->NextFree != nullptr)
            pBest->NextFree->PrevFree = pBest->PrevFree;

        pBest->PrevFree = pBest->NextFree = nullptr;
        pBest->nFreeBlock = 0;
        pNw = pBest;

        pPage->nLargetFree = 0;
    }

    byte *pRet = (byte *)pNw + ALIGN_SIZE(MEDIUM_HEADER_SIZE);
    pRet[-1] = MEDIUM_ALLOC;
    return (void *)pRet;
}

void CFreelistAlloc::MediumFree(void *p)
{
    ((byte *)p)[-1] = INVALID_ALLOC;

    MediumHeapEntry *e = (MediumHeapEntry *)((byte *)p - ALIGN_SIZE(MEDIUM_HEADER_SIZE));
    Page *pPage = e->pPage;
    bool bIsInFreelist = pPage->nLargetFree >= MEDIUM_SMALLEST_SIZE;

    assert(e->nSize);
    assert(e->nFreeBlock == 0);

    MediumHeapEntry *e_Prev = e->Prev;

    if (e_Prev && e_Prev->nFreeBlock == 1)
    {
        // 
        // 
        // 
        // 
        e_Prev->nSize += e->nSize;
        e_Prev->Next = e->Next;
        if (e->Next)
            e->Next->Prev = e_Prev;
        e = e_Prev;
    }
    else
    {
        // 
        // 
        e->PrevFree = nullptr;
        e->NextFree = (MediumHeapEntry *)pPage->pFirstFree;
        if (e->NextFree)
        {
            assert(e->NextFree->PrevFree == nullptr);
            e->NextFree->PrevFree = e;
        }

        pPage->pFirstFree = e;
        pPage->nLargetFree = e->nSize;
        e->nFreeBlock = 1;
    }

    MediumHeapEntry *e_Next = e->Next;

    if (e_Next && e_Next->nFreeBlock == 1)
    {
        // 
        e->nSize += e_Next->nSize;
        e->Next = e_Next->Next;
        if (e_Next->Next)
            e_Next->Next->Prev = e;

        // 
        if (e_Next->PrevFree)
            e_Next->PrevFree->NextFree = e_Next->NextFree;
        else
        {
            // 
            assert(e_Next == pPage->pFirstFree);
            // 
            pPage->pFirstFree = e_Next->NextFree;
        }

        if (e_Next->NextFree)
            e_Next->NextFree->PrevFree = e_Next->PrevFree;
    }

    if (pPage->pFirstFree)
        pPage->nLargetFree = ((MediumHeapEntry *)pPage->pFirstFree)->nSize;
    else
        pPage->nLargetFree = 0;

    if (e->nSize > pPage->nLargetFree)
    {
        // 
        assert(e != pPage->pFirstFree);
        pPage->nLargetFree = e->nSize;

        // 
        if (e->PrevFree)
            e->PrevFree->NextFree = e->NextFree;
        if (e->NextFree)
            e->NextFree->PrevFree = e->PrevFree;

        // 
        e->NextFree = (MediumHeapEntry *)pPage->pFirstFree;
        e->PrevFree = nullptr;
        if (e->NextFree)
            e->NextFree->PrevFree = e;
        pPage->pFirstFree = e;
    }

    if (!bIsInFreelist)
    {
        if (pPage->pPrev)
            pPage->pPrev->pNext = pPage->pNext;
        if (pPage->pNext)
            pPage->pNext->pPrev = pPage->pPrev;
        if (pPage == m_pMediumFirstUsedPage)
            m_pMediumFirstUsedPage = pPage->pNext;

        pPage->pNext = nullptr;
        pPage->pPrev = m_pMediumLastFreePage;
        if (m_pMediumLastFreePage)
            m_pMediumLastFreePage->pNext = pPage;
        m_pMediumLastFreePage = pPage;
        if (m_pMediumFirstFreePage == nullptr)
            m_pMediumFirstFreePage = pPage;
    }
}

void* CFreelistAlloc::LargeAllocate(dword bytes)
{
    dword nAlignedHeaderSize = ALIGN_SIZE(LARGE_HEADER_SIZE);
    Page *pPage = AllocatePage(bytes + nAlignedHeaderSize);
    assert(pPage != nullptr);
    byte *pData = ((byte *)pPage->pData) + nAlignedHeaderSize;
    pData[-1] = LARGE_ALLOC;
    ((uintptr_t *)pPage->pData)[0] = (uintptr_t)pPage;

    pPage->pPrev = nullptr;
    pPage->pNext = m_pLargeFirstUsedPage;
    if (pPage->pNext)
        pPage->pNext->pPrev = pPage;
    m_pLargeFirstUsedPage = pPage;
    return (void *)pData;
}

void CFreelistAlloc::LargeFree(void *p)
{
    ((byte *)p)[-1] = INVALID_ALLOC;

    Page *pPage = (Page *)(*((uintptr_t *)((byte *)p - ALIGN_SIZE(LARGE_HEADER_SIZE))));
    if (pPage->pPrev)
        pPage->pPrev->pNext = pPage->pNext;
    if (pPage->pNext)
        pPage->pNext->pPrev = pPage->pPrev;
    if (pPage == m_pLargeFirstUsedPage)
        m_pLargeFirstUsedPage = pPage->pNext;
    pPage->pNext = pPage->pPrev = nullptr;
    FreePage(pPage);
}

CFreelistAlloc::Page* CFreelistAlloc::AllocatePage(dword nPageSize)
{
    Page *pPage;
    ++m_nPageRequest;

    if (m_pSwapPage != nullptr && m_pSwapPage->nDataSize == nPageSize)
    {
        pPage = m_pSwapPage;
        m_pSwapPage = nullptr;
    }
    else
    {
        // 
        dword nTotalSize = nPageSize + sizeof(Page);
        // 
        // 
        // 
        // 
        pPage = (Page *)::malloc(nTotalSize + ALIGN - 1);
        if (pPage == nullptr)
        {
            assert(0 && "Malloc failure for page");
            return nullptr;
        }

        pPage->pData = (void *)ALIGN_SIZE( (uintptr_t)(((byte *)pPage) + sizeof(Page)) );
        pPage->nDataSize = nTotalSize - sizeof(Page);
        pPage->pFirstFree = nullptr;
        pPage->nLargetFree = 0;
        ++m_nOSAllocs;
    }

    pPage->pNext = pPage->pPrev = nullptr;
    ++m_nPageAllocated;
    return pPage;
}

void CFreelistAlloc::FreePage(Page *pPage)
{
    if (pPage->nDataSize == m_nPageSize && m_pSwapPage == nullptr)
        m_pSwapPage = pPage;
    else
        FreePageReal(pPage);

    --m_nPageAllocated;
}

void CFreelistAlloc::FreePageReal(Page *pPage)
{
    ::free(pPage);
}