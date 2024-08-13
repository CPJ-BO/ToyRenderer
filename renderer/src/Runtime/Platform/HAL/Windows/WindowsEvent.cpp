#include "WindowsEvent.h"

void WindowsEvent::Trigger()
{
	SetEvent(event);
}

void WindowsEvent::Reset()
{
	ResetEvent(event);
}

bool WindowsEvent::Wait(uint32_t WaitTime)
{
	return (WaitForSingleObject(event, WaitTime) == WAIT_OBJECT_0);
}