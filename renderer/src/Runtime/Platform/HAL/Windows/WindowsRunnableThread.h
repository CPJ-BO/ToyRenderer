#pragma once

#include "Platform/HAL/RunnableThread.h"
#include "Platform/HAL/Event.h"

#include <cstdint>
#include <windows.h>

class WindowsRunnableThread : public RunnableThread
{
public:
	~WindowsRunnableThread() { if(thread) Kill(true); }

	virtual void SetThreadPriority(ThreadPriorityType priority)  override final;
    virtual bool SetThreadAffinity(uint64_t affinity) override final;
	virtual void Suspend(bool pause = true) override final;
	virtual bool Kill(bool wait = false) override final;
	virtual void WaitForCompletion() override final;

private:
    HANDLE thread = 0;
    EventRef syncEvent;

    static ::DWORD _ThreadProc(LPVOID pThis)
	{
		auto* thisThread = (WindowsRunnableThread*)pThis;
		//FThreadManager::Get().AddThread(ThisThread->GetThreadID(), ThisThread);
		return thisThread->Run();
	}

	uint32_t Run();

    static int TranslateThreadPriority(ThreadPriorityType priority);

    virtual bool CreateInternal( 
    RunnableRef runnable, 
    uint32_t stackSize = 0,
    ThreadPriorityType priority = PRIORITY_TYPE_NORMAL, 
    uint64_t affinityMask = 0,
    const std::string& name = "") override final;
};