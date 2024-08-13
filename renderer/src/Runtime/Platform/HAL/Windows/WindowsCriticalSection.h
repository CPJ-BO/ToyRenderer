#pragma once

#include "Platform/HAL/CriticalSection.h"

#include <windows.h>

class WindowsCriticalSection : public CriticalSection
{
public:
	WindowsCriticalSection(const WindowsCriticalSection&) = delete;
	WindowsCriticalSection& operator=(const WindowsCriticalSection&) = delete;

	WindowsCriticalSection()
	{
		InitializeCriticalSection(&criticalSection);
		SetCriticalSectionSpinCount(&criticalSection,4000);
	}

	~WindowsCriticalSection()
	{
		DeleteCriticalSection(&criticalSection);
	}

	inline virtual void Lock() override
	{
		EnterCriticalSection(&criticalSection);
	}

	inline virtual bool TryLock() override
	{
		if (TryEnterCriticalSection(&criticalSection))
		{
			return true;
		}
		return false;
	}

	inline virtual void Unlock() override
	{
		LeaveCriticalSection(&criticalSection);
	}

private:
	CRITICAL_SECTION criticalSection;
};