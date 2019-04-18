#include "JobSystem.h"

void ThreadFunc(CWorker *pWorker) {
	while (!pWorker->Finished()) {
		if (pWorker->HasMoreJob())
			pWorker->DoWork();
		else
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(100));
	}
}

JobFactories::CreateJobCallback JobFactories::m_JobCtorCallback[JobCount];
JobFactories::DestroyJobCallback JobFactories::m_JobDtorCallback[JobCount];

CJobSystem::CJobSystem() 
: m_nWorkerCount(0), 
m_nWorkerIndex(-1)
{
	m_JobPool.Initialize(sizeof(Job));
}

CJobSystem::~CJobSystem() {
	// 阻塞主线程直到所有的工作线程全部执行完成
	for (byte i = 0; i < m_nWorkerCount; ++i) {
		m_pWorkers[i].NotifyFinished();
		m_ppThreads[i]->join();
		delete m_ppThreads[i];
	}
	MEMFREE(m_ppThreads);
	delete[] m_pWorkers;
}

void CJobSystem::Initialize() {
#if TARGET_PLATFORM == PLATFORM_WINDOWS
	SYSTEM_INFO sys_info;
	DWORD_PTR process_affinity_mask = 1;
	DWORD_PTR system_affinity_mask;
	DWORD dwAvailableProcessors = 0;
	GetSystemInfo(&sys_info);
	GetProcessAffinityMask(GetCurrentProcess(), &process_affinity_mask, &system_affinity_mask);
	for (dword i = 0; i < sys_info.dwNumberOfProcessors; ++i)
	{
		if (process_affinity_mask & ((DWORD_PTR)1 << i))
			++dwAvailableProcessors;
	}
	// 工作线程的数量是可用核心数-1
	m_nWorkerCount = dwAvailableProcessors - 1;
// #elif TARGET_PLATFORM == PLATFORM_ANDROID
//     m_nWorkerCount = android_getCpuCount() - 1;
#endif
	m_nWorkerCount = max(1, m_nWorkerCount);
	m_pWorkers = new CWorker[m_nWorkerCount];
	m_ppThreads = (std::thread**)MEMALLOC(sizeof(std::thread**) * m_nWorkerCount);
	for (byte i = 0; i < m_nWorkerCount; ++i)
		m_ppThreads[i] = new std::thread(ThreadFunc, &m_pWorkers[i]);
}

Job* CJobSystem::CreateJob(EJobType nJobType) {
	return JobFactories::m_JobCtorCallback[(dword)nJobType](m_JobPool.Allocate_mt());
}

void CJobSystem::DestroyJob(Job *pJob) {
	assert(pJob != nullptr);
	JobFactories::m_JobDtorCallback[(dword)pJob->m_Type](pJob);
	m_JobPool.Free_mt(pJob);
}

void CJobSystem::QueueJob(Job *pJob) {
	assert(pJob != nullptr);
	// 通过轮询的方式实现简单的负载均衡
	std::lock_guard<std::mutex> slock(m_WorkerLock);
	byte workerIndex = (word)(m_nWorkerIndex + 1) % m_nWorkerCount;
	m_nWorkerIndex = workerIndex;
	m_pWorkers[workerIndex].QueueJob(pJob);
}

void CJobSystem::Update_MainThread(dword nFrameId) {
	if (m_nWorkerCount == 0)
		return;
	
	// 首先决定本帧读取哪个工作线程的任务队列
	byte workerIndex = (byte)(nFrameId % m_nWorkerCount);
	CWorker &worker = m_pWorkers[workerIndex];

	Linklist<Job> lstProcessedJobs;
	{
		std::lock_guard<std::mutex> l(worker.m_ProcessedJobLock);
		lstProcessedJobs = worker.m_lstProcessedJob;
		worker.m_lstProcessedJob.Clear();
	}

	Linklist<Job>::_NodeType *pTemp = lstProcessedJobs.m_pRoot;
	while (pTemp != nullptr) {
		Job *pJob = pTemp->m_pOwner;
		pTemp = pTemp->m_pNext;
		if (!pJob->IsCanceled()) // 如果任务被取消了就不执行主线程的操作了
			pJob->DoWork_MainThread();
		pJob->OnJobFinished();
		DestroyJob(pJob);
	}
}