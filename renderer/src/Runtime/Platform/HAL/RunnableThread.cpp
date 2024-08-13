#include "RunnableThread.h"
#include "PlatformProcess.h"

#include "Core/Log/Log.h"

RunnableThreadRef RunnableThread::Create(
    RunnableRef runnable, 
    uint32_t stackSize,
    ThreadPriorityType priority, 
    uint64_t affinityMask,
    const std::string& name)
{
    if(runnable == nullptr)
    {
        LOG_DEBUG("Runnable is empty!");
        return nullptr;
    }
	RunnableThreadRef newThread = PlatformProcess::CreateRunnableThread();
	if (newThread && !newThread->CreateInternal(runnable, stackSize, priority, affinityMask, name)) newThread = nullptr;

	return newThread;
}