#include "PlatformProcess.h"
#include "Windows/WindowsPlatformProcess.h" // TODO

RunnableThreadRef PlatformProcess::CreateRunnableThread() { return WindowsPlatformProcess::CreateRunnableThread(); }

void PlatformProcess::Sleep(float seconds) { WindowsPlatformProcess::Sleep(seconds); }

SemaphoreRef PlatformProcess::CreateSemaphore() {return WindowsPlatformProcess::CreateSemaphore(); }

EventRef PlatformProcess::CreateSyncEvent(bool manualReset) { return WindowsPlatformProcess::CreateSyncEvent(manualReset); }	

CriticalSectionRef PlatformProcess::CreateCriticalSection() { return WindowsPlatformProcess::CreateCriticalSection(); }

