#pragma once

#include "../Semaphore.h"
#include "Core/Log/Log.h"

#include <cstdint>
#include <windows.h>

class WindowsSemaphore : public Semaphore
{
public:

	WindowsSemaphore()
	: semaphore(CreateSemaphore(nullptr, 0, 1, nullptr))
	{}

	~WindowsSemaphore() { CloseHandle(semaphore); }

	virtual void Lock() override final
	{
		DWORD res = WaitForSingleObject(semaphore, INFINITE);
		if(res != WAIT_OBJECT_0) LOG_DEBUG("Lock semaphore failed: %u (%u)", (uint32_t)res, (uint32_t)GetLastError());
	}

	virtual bool TryLock(uint64_t milliseconds) override final
	{
		DWORD res = WaitForSingleObject(semaphore, (DWORD)milliseconds);
		return res == WAIT_OBJECT_0;
		if(!(res == WAIT_OBJECT_0 || res == WAIT_TIMEOUT)) LOG_DEBUG(TEXT("Try lock semaphore failed: %d (%u)"), (uint32_t)res, (uint32_t)GetLastError());
	}

	virtual void Unlock() override final
	{
		bool res = ReleaseSemaphore(semaphore, 1, nullptr);
		if(!res) LOG_DEBUG(TEXT("Unlock semaphore failed: %u"), (uint32_t)GetLastError());
	}

private:
	HANDLE semaphore;
};