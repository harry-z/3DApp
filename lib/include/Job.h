#pragma once

#include "Prereq.h"


#define TERRAIN_JOB		0
#define TEXTURE_LOADING_JOB				1
#define MATERIAL_LOADING_JOB			2
#define MESH_LOADING_JOB		3
#define JOB_TYPE_COUNT 4

enum class EJobType : byte {
	JobType_Texture = 0,
	JobType_Material,
	JobType_Mesh,
	JobType_Num
};
constexpr dword JobCount = (dword)EJobType::JobType_Num;

struct Job {
	
	LinklistNode<Job> m_node;
	byte *m_pData;
	EJobType m_Type;
	std::atomic_bool m_Canceled;

	Job() : m_pData(nullptr) { 
		m_node.m_pOwner = this; 
		m_Canceled = false;
	}
	virtual ~Job() {}

	virtual void OnJobFailed() = 0;
	virtual void OnJobFinished() = 0;
	virtual bool DoWork_WorkerThread() = 0;
	virtual bool DoWork_MainThread() = 0;

	inline void Cancel() { m_Canceled = true; }
	inline bool IsCanceled() { return m_Canceled; }
};

// typedef Job* (*CreateJobCallback)(void*);
// typedef void (*DestroyJobCallback)(Job*);

struct JobFactories {
	using CreateJobCallback = std::function<Job*(void*)>;
	using DestroyJobCallback = std::function<void(Job*)>;
	static CreateJobCallback m_JobCtorCallback[JobCount];
	static DestroyJobCallback m_JobDtorCallback[JobCount];
	static void Add(EJobType type, CreateJobCallback ctor, DestroyJobCallback dtor) {
		m_JobCtorCallback[(dword)type] = ctor;
		m_JobDtorCallback[(dword)type] = dtor;
	}
};


#define REGISTER_JOB_CTOR_DTOR_DECL(type, typeenum, ctor, dtor) \
	struct type##_FACTORY { \
		type##_FACTORY() { JobFactories::Add(typeenum, ctor, dtor); } \
	};

#define REGISTER_JOB_CTOR_DTOR_IMPL(type) \
	type##_FACTORY factory;
