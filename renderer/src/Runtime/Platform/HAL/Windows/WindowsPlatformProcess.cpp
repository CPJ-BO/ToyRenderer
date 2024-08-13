#include "WindowsPlatformProcess.h"
#include "WindowsRunnableThread.h"
#include "WindowsSemaphore.h"
#include "WindowsEvent.h"
#include "Platform/HAL/Windows/WindowsCriticalSection.h"

#include <memory>

#undef CreateSemaphore

//线程 ////////////////////////////////////////////////////////////////////////////////////////////////////////
RunnableThreadRef WindowsPlatformProcess::CreateRunnableThread() 
{ 
    return std::make_shared<WindowsRunnableThread>(); 
}

void WindowsPlatformProcess::Sleep(float seconds)
{
	uint32_t milliseconds = (uint32_t)(seconds * 1000.0);
	if (milliseconds == 0)	::SwitchToThread();
	else					::Sleep(milliseconds);
}

//信号量 ////////////////////////////////////////////////////////////////////////////////////////////////////////
SemaphoreRef WindowsPlatformProcess::CreateSemaphore() 
{
    return std::make_shared<WindowsSemaphore>(); 
}

//事件 ////////////////////////////////////////////////////////////////////////////////////////////////////////
EventRef WindowsPlatformProcess::CreateSyncEvent(bool manualReset)
{
	EventRef event = std::make_shared<WindowsEvent>();	
	if (!event->Create(manualReset)) event = NULL;

	return event;
}

//临界区 ////////////////////////////////////////////////////////////////////////////////////////////////////////
CriticalSectionRef WindowsPlatformProcess::CreateCriticalSection()
{
	return std::make_shared<WindowsCriticalSection>();
}


