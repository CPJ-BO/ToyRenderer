#pragma once

#include "Platform/HAL/Runnable.h"

#include <cstdint>
#include <memory>
#include <string>

enum ThreadPriorityType
{
	PRIORITY_TYPE_NORMAL = 0,
    PRIORITY_TYPE_ABOVE_NORMAL,
    PRIORITY_TYPE_BELOW_NORMAL,
    PRIORITY_TYPE_HIGHEST,
    PRIORITY_TYPE_LOWEST,
    PRIORITY_TYPE_SLIGHTLY_BELOW_NORMAL,
    PRIORITY_TYPE_TIME_CRITICAL,

    PRIORITY_TYPE_MAX_ENUM,   //
};

typedef std::shared_ptr<class RunnableThread> RunnableThreadRef;

class RunnableThread // 线程抽象
{
	static uint32_t RunnableTlsSlot;

public:
	static RunnableThreadRef Create(
        RunnableRef runnable, 
		uint32_t stackSize = 0,
		ThreadPriorityType priority = PRIORITY_TYPE_NORMAL, 
        uint64_t affinityMask = 0,
		const std::string& name = "");

    RunnableThread() {};
    virtual ~RunnableThread() {};

	virtual void SetThreadPriority(ThreadPriorityType priority) = 0;
	virtual bool SetThreadAffinity(uint64_t affinity) = 0;
	virtual void Suspend(bool pause = true) = 0;
	virtual bool Kill(bool wait = true) = 0;
	virtual void WaitForCompletion() = 0;

	const uint32_t GetThreadID() const          		{ return id; }
	const std::string& GetThreadName() const    		{ return name; }
	ThreadPriorityType GetThreadPriority() const    	{ return priority; }

protected:
	virtual bool CreateInternal( 
        RunnableRef runnable, 
		uint32_t stackSize = 0,
		ThreadPriorityType priority = PRIORITY_TYPE_NORMAL, 
        uint64_t affinityMask = 0,
		const std::string& name = "") = 0;

	std::string name;
	RunnableRef runnable;
    ThreadPriorityType priority;
	uint32_t id;
    uint64_t affinityMask;

private:

	virtual void Tick() {}
};

