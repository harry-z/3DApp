#pragma once

#include "Job.h"


class CWorker {
public:
	friend class CJobSystem;

	CWorker() { m_Flag = false; }
	~CWorker() {}

	void QueueJob(Job *pJob);

	bool HasMoreJob() { 
		std::lock_guard<std::mutex> l(m_WaitingJobLock);
		return !m_lstWaitingJob.Empty();
	}
	void DoWork();

	void NotifyFinished() { m_Flag = true; }
	bool Finished() { return m_Flag; }

private:
	Linklist<Job> m_lstWaitingJob;
	Linklist<Job> m_lstProcessedJob;

	std::mutex m_WaitingJobLock;
	std::mutex m_ProcessedJobLock;

	std::atomic_bool m_Flag;
};