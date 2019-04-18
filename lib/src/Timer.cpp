#include "Timer.h"


CTimerManager::CTimerManager() 
: m_bNeedInitialize(true),
m_fGlobalScale(1.0f) {
    m_TimerPool.Initialize(sizeof(Timer));
}

Timer* CTimerManager::CreateTimer(bool bStart) {
    Timer *pTimer = (Timer*)(m_TimerPool.Allocate_mt());
    pTimer->m_fNow = pTimer->m_fFrameInterval = pTimer->m_fScale = 0.0f;
    pTimer->m_node.m_pOwner = pTimer;
    m_lstTimer.PushBack(&pTimer->m_node);
    if (bStart)
        TIMER_START(pTimer);
    return pTimer;
}

void CTimerManager::DestroyTimer(Timer *pTimer) {
    m_lstTimer.Remove(&pTimer->m_node);
    m_TimerPool.Free_mt(pTimer);
}

void CTimerManager::Update() {
    using namespace std::chrono;
    steady_clock::time_point now = steady_clock::now();
    if (m_bNeedInitialize) {
        m_TickCount = now;
        m_bNeedInitialize = false;
    }
    duration<float> interval = duration_cast<duration<float>>(steady_clock::now() - m_TickCount);
    float fIntervalInMilliseconds = (float)interval.count() * m_fGlobalScale * 1000.0f;
    m_TickCount = now;
    Linklist<Timer>::_NodeType *pTemp = m_lstTimer.m_pRoot;
    while (pTemp != nullptr) {
        Timer *pTimer = pTemp->m_pOwner;
        pTimer->m_fFrameInterval = fIntervalInMilliseconds * pTimer->m_fScale;
        pTimer->m_fNow += pTimer->m_fFrameInterval;
        pTemp = pTemp->m_pNext;
    }
}

void CTimerManager::StartAllTimers() {
    Linklist<Timer>::_NodeType *pTemp = m_lstTimer.m_pRoot;
    while (pTemp != nullptr) {
        TIMER_START(pTemp->m_pOwner);
        pTemp = pTemp->m_pNext;
    }
}

void CTimerManager::PauseAllTimers() {
    Linklist<Timer>::_NodeType *pTemp = m_lstTimer.m_pRoot;
    while (pTemp != nullptr) {
        TIMER_PAUSE(pTemp->m_pOwner);
        pTemp = pTemp->m_pNext;
    }
}