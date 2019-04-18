#include "JobSystem.h"
#include "Worker.h"

void CWorker::QueueJob(Job *pJob) {
	std::lock_guard<std::mutex> l(m_WaitingJobLock);
	m_lstWaitingJob.PushBack(&pJob->m_node);
}

void CWorker::DoWork() {
	Job *pJob;
	{
		// 从排队的任务中拿出第一个任务
		std::lock_guard<std::mutex> l(m_WaitingJobLock);
		pJob = m_lstWaitingJob.PopFront()->m_pOwner;
	}

	if (!pJob->IsCanceled()) { // 如果任务被取消就不执行任务内容
		if (pJob->DoWork_WorkerThread()) {
			std::lock_guard<std::mutex> l(m_ProcessedJobLock);
			m_lstProcessedJob.PushBack(&pJob->m_node);
			return;
		}
	}

	pJob->OnJobFailed();
	Global::m_pJobSystem->DestroyJob(pJob);
}