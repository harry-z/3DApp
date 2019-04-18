#pragma once

#include "Prereq.h"

#define TIMER_PAUSE(t) t->m_fScale = 0.0f
#define TIMER_START(t) t->m_fScale = 1.0f
#define TIMER_SCALE(t, r) t->m_fScale = r
#define TIMER_STOP(t) \
	t->m_fNow = t->m_fFrameInterval = t->m_fScale = 0.0f

struct Timer {
    float m_fNow;
    float m_fFrameInterval;
    float m_fScale;
    LinklistNode<Timer> m_node;
};

class CTimerManager {
public:
	CTimerManager();

    Timer* CreateTimer(bool bStart);
    void DestroyTimer(Timer *pTimer);

    void Update();

    void StartAllTimers();
    void PauseAllTimers();

    inline void SetGlobalTimeScale(float fScale) { m_fGlobalScale = fScale; }
	inline float GetGlobalTimeScale() const { return m_fGlobalScale; }

private:
    std::chrono::steady_clock::time_point m_TickCount;
    Linklist<Timer> m_lstTimer;
    CPool m_TimerPool;
    float m_fGlobalScale;
    bool m_bNeedInitialize;
};