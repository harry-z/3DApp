#include "JobSystem.h"
#include "Worker.h"

void CWorker::QueueJob(Job *pJob) {
	std::lock_guard<std::mutex> l(m_WaitingJobLock);
	m_lstWaitingJob.PushBack(&pJob->m_node);
}

void CWorker::DoWork() {
	Job *pJob;
	{
		std::lock_guard<std::mutex> l(m_WaitingJobLock);
		pJob = m_lstWaitingJob.PopFront()->m_pOwner;
	}

	if (!pJob->IsCanceled()) {
		if (pJob->DoWork_WorkerThread()) {
			std::lock_guard<std::mutex> l(m_ProcessedJobLock);
			m_lstProcessedJob.PushBack(&pJob->m_node);
			return;
		}
	}

	pJob->OnJobFailed();
	Global::m_pJobSystem->DestroyJob(pJob);
}