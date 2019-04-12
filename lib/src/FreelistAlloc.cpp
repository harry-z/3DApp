#include "FreelistAlloc.h"

// #define SMALL_HEADER_SIZE ( (int)(sizeof(byte) + sizeof(byte)) )
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
, m_nPageSize(65536 - sizeof(Page))
, m_nPageAllocated(0)
, m_nPageRequest(0)
, m_nSmallCurPageOffset(0)
, m_nOSAllocs(0)
{
    memset(m_ppSmallFirstFree, 0, sizeof(m_ppSmallFirstFree));
    m_pSmallCurPage = AllocatePage(m_nPageSize);
    assert(m_pSmallCurPage != nullptr);
}

CFreelistAlloc::~CFreelistAlloc()
{}

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
    std::lock_guard<std::mutex> slock(m_Memlock);
    if (p == nullptr)
        return;
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
        // 获取下一个Block的地址
        uintptr_t *pNextLink = (uintptr_t *)(pSmallBlock + SMALL_HEADER_SIZE);
        // 填入标记
        pSmallBlock[1] = SMALL_ALLOC;
        // 将下一个Block的地址填入m_ppSmallFirstFree对应索引位置
        m_ppSmallFirstFree[bytes/ALIGN - 1] = (void *)(*pNextLink);
        return (void *)pNextLink;
    }

    dword nBytesLeft = m_nPageSize - m_nSmallCurPageOffset;
    if (bytes >= nBytesLeft)
    {
        // CurrentPage -> FirstUsedPage -> ... -> LastUsedPage
        // 将当前页设置为第一个使用页
        m_pSmallCurPage->pNext = m_pSmallFirstUsedPage;
        m_pSmallFirstUsedPage = m_pSmallCurPage;
        // 重新分配一个当前页
        m_pSmallCurPage = AllocatePage(m_nPageSize);
        if (m_pSmallCurPage == nullptr)
            return nullptr;
        m_nSmallCurPageOffset = ALIGN_SIZE(0);
    }

    pSmallBlock = ((byte *)m_pSmallCurPage->pData) + m_nSmallCurPageOffset;
    // 保存在m_ppSmallFirstFree数组中的索引
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
    }

    // 将SmallFirstFree当前的头地址存入p的头四个字节中
    *pAddress = (uintptr_t)m_ppSmallFirstFree[idx];
    // 将SmallFirstFree的头换成当前释放的内存指针
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
        if (pPage == nullptr)
            return nullptr;

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
        // HeapEntry的Size包含了HeapEntry自身的大小
        e->nSize = m_nPageSize & ~(ALIGN - 1);
        e->nFreeBlock = 1;
        e->Prev = nullptr;
        e->Next = nullptr;
        e->PrevFree = nullptr;
        e->NextFree = nullptr;
    }

    void *pData = MediumAllocateFromPage(pPage, nSizeNeeded);

    // pPage的剩余容量不够一个最小的MediumHeap
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

        // pPage设置为MediumFirstUsed
        pPage->pPrev = nullptr;
        pPage->pNext = m_pMediumFirstUsedPage;
        if (pPage->pNext)
            pPage->pNext->pPrev = pPage;
        m_pMediumFirstUsedPage = pPage;
        return pData;
    }

    // 调整Medium的Free链表的顺序，将pPage调整到MediumFirstFreePage的位置
    // 下次再开始遍历链表时能够加快速度
    if (pPage != m_pMediumFirstFreePage)
    {
        // 如果pPage不等于FirstFreePage，以下三个指针必定不是nullptr
        assert(m_pMediumLastFreePage);
        assert(m_pMediumFirstFreePage);
        assert(pPage->pPrev);

        // 调整后顺序为
        // pPage -> ... -> LastFree -> FirstFree -> ... -> pPage->Prev
        // 将现在的FirstFree连接到LastFree后面
        m_pMediumLastFreePage->pNext = m_pMediumFirstFreePage;
        m_pMediumFirstFreePage->pPrev = m_pMediumLastFreePage;
        // 将LastFree设置为pPage的上一个Page
        m_pMediumLastFreePage = pPage->pPrev;
        pPage->pPrev->pNext = nullptr;
        pPage->pPrev = nullptr;
        // 将FirstFree设置为pPage
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

    // pBest的剩余空间除了分配nSizeNeeded之外还够一个最小的MediumHeap
    if (pBest->nSize >= nSizeNeeded + MEDIUM_SMALLEST_SIZE)
    {
        // Best是Page的FirstFree
        // Best的Next是最近一次分配的MediumHeap
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
    else // 如果Best的剩余空间不够再多一个最小的MediumHeap
    {
        // Best作为要分配的MediumHeap返回
        // Best把自己从Free链表中摘出去
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
        // 前一个HeapEntry是可以合并的
        // 因为e是正在释放的Entry，所以其PrevFree和NextFree全都是nullptr
        // 只将Entry链表关系维护好即可
        // Free链表仍然维护的是e_Prev的PrevFree和NextFree
        e_Prev->nSize += e->nSize;
        e_Prev->Next = e->Next;
        if (e->Next)
            e->Next->Prev = e_Prev;
        e = e_Prev;
    }
    else
    {
        // 前一个Entry不能合并
        // 由于e是正在释放的Entry，没有PrevFree和NextFree信息，故将其放到Free链表的头部
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
        // 下一个Entry可以被合并
        e->nSize += e_Next->nSize;
        e->Next = e_Next->Next;
        if (e_Next->Next)
            e_Next->Next->Prev = e;

        // 由于下一个Entry要被合并起来了，而且这个Entry本身是FreeBlock，所以要更新Free链表关系
        if (e_Next->PrevFree)
            e_Next->PrevFree->NextFree = e_Next->NextFree;
        else
        {
            // 如果PrevFree为空那么Next就是Page的FirstFree
            assert(e_Next == pPage->pFirstFree);
            // 将Page的FirstFree设置为Next的NextFree
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
        // 当前释放的Entry比Page最大的FreeBlock还要大
        assert(e != pPage->pFirstFree);
        pPage->nLargetFree = e->nSize;

        // 将e先从Free链表中摘掉
        if (e->PrevFree)
            e->PrevFree->NextFree = e->NextFree;
        if (e->NextFree)
            e->NextFree->PrevFree = e->PrevFree;

        // 将e的NextFree设置为当前的FirstFree
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
    if (pPage == nullptr)
        return nullptr;
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
        // 实际Page需分配的总数
        dword nTotalSize = nPageSize + sizeof(Page);
        // 多分配ALIGN-1，是为了保证pData能够正确对齐
        // 内存布局为[Page][nPageSize]
        // pData指向nPageSize的起点，需要保证pData是对齐的，即pPage + sizeof(Page)要保证对齐
        // 最坏情况需要把pData的起始向后移动ALIGN-1个字节
        pPage = (Page *)::malloc(nTotalSize + ALIGN - 1);
        if (pPage == nullptr)
        {
            assert(0 && "Malloc failure for page");
            return nullptr;
        }
        // 对齐pData
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
    if (pPage == nullptr)
        return;
    if (pPage->nDataSize == m_nPageSize && m_pSwapPage == nullptr)
        m_pSwapPage = pPage;
    else
        FreePageReal(pPage);

    --m_nPageAllocated;
}

void CFreelistAlloc::FreePageReal(Page *pPage)
{
    assert(pPage != nullptr);
    ::free(pPage);
}