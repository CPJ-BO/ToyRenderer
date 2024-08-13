#include "WindowsRunnableThread.h"
#include "Platform/HAL/PlatformProcess.h"

#include "Core/Log/Log.h"
#include <cstdint>

int WindowsRunnableThread::TranslateThreadPriority(ThreadPriorityType Priority)
{
	switch (Priority) {
    case PRIORITY_TYPE_ABOVE_NORMAL:			return THREAD_PRIORITY_ABOVE_NORMAL;
    case PRIORITY_TYPE_NORMAL:				    return THREAD_PRIORITY_NORMAL;
    case PRIORITY_TYPE_BELOW_NORMAL:			return THREAD_PRIORITY_BELOW_NORMAL;
    case PRIORITY_TYPE_HIGHEST:				    return THREAD_PRIORITY_HIGHEST;
    case PRIORITY_TYPE_TIME_CRITICAL:			return THREAD_PRIORITY_HIGHEST;
    case PRIORITY_TYPE_LOWEST:				    return THREAD_PRIORITY_LOWEST;
    case PRIORITY_TYPE_SLIGHTLY_BELOW_NORMAL:	return THREAD_PRIORITY_NORMAL;
	default:                                    return THREAD_PRIORITY_NORMAL;
	}
}

void WindowsRunnableThread::SetThreadPriority(ThreadPriorityType priority)
{
    this->priority = priority;
    ::SetThreadPriority(thread, TranslateThreadPriority(priority));
}

bool WindowsRunnableThread::SetThreadAffinity(uint64_t affinity)
{
    GROUP_AFFINITY groupAffinity = {};
	GROUP_AFFINITY previousGroupAffinity = {};
	groupAffinity.Mask = affinity ;
	//groupAffinity.Group = ;

	if (SetThreadGroupAffinity(thread, &groupAffinity, &previousGroupAffinity) == 0)
	{
		DWORD LastError = GetLastError();
		LOG_DEBUG("Runnable thread call to SetThreadAffinity failed :%u", (uint32_t)LastError);
		return  false;
	}

	affinity = affinity;
	return  previousGroupAffinity.Mask != groupAffinity.Mask || 
            previousGroupAffinity.Group != groupAffinity.Group;
};

void WindowsRunnableThread::Suspend(bool pause)
{
    if (pause == true)   SuspendThread(thread);
    else                 ResumeThread(thread);
}

bool WindowsRunnableThread::Kill(bool wait)
{
    if (runnable)        runnable->Stop();
    if (wait == true)    WaitForSingleObject(thread, INFINITE);

    CloseHandle(thread);
    thread = NULL;

    return true;
}

void WindowsRunnableThread::WaitForCompletion() 
{ 
    WaitForSingleObject(thread, INFINITE); 
}

bool WindowsRunnableThread::CreateInternal( 
    RunnableRef runnable, 
    uint32_t stackSize,
    ThreadPriorityType priority, 
    uint64_t affinityMask,
    const std::string& name) 
{
    static bool once = false;
    if (!once)
    {
        once = true;
        ::SetThreadPriority(::GetCurrentThread(), TranslateThreadPriority(PRIORITY_TYPE_NORMAL));
    }

    this->runnable = runnable;
    this->affinityMask = affinityMask;
    this->name = name;
    this->priority = priority;
    this->syncEvent = PlatformProcess::CreateSyncEvent(true);

    {
        thread = CreateThread(
            NULL, 
            stackSize, 
            _ThreadProc, 
            this, 
            STACK_SIZE_PARAM_IS_A_RESERVATION | CREATE_SUSPENDED, 
            (::DWORD*)&id);
    }

    if (thread == NULL)     runnable = nullptr;
    else
    {
        ResumeThread(thread);
        this->syncEvent->Wait(INFINITE);
        this->priority = PRIORITY_TYPE_NORMAL; 
        SetThreadPriority(priority);
    }
    syncEvent = nullptr;

    return thread != NULL;
}

uint32_t WindowsRunnableThread::Run()
{
	uint32_t exitCode = 1;
    if(runnable == nullptr) LOG_DEBUG("Runnable is null!");
	if (runnable->Init() == true)
	{
        syncEvent->Trigger();

		exitCode = runnable->Run();
		runnable->Exit();
	}
	else syncEvent->Trigger();

	return exitCode;
}