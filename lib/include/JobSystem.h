#pragma once

#include "Worker.h"


class CJobSystem {
public:
	CJobSystem();
	~CJobSystem();

	void Initialize();

	Job* CreateJob(EJobType nJobType);
	void DestroyJob(Job *pJob);
	void QueueJob(Job *pJob);

	void Update_MainThread(dword nFrameId);

private:
	byte m_nWorkerCount;
	char m_nWorkerIndex;
	std::mutex m_WorkerLock;

	std::thread **m_ppThreads;
	CWorker *m_pWorkers;
	CPool m_JobPool;
};