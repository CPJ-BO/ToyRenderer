
#include "Platform/HAL/Event.h"

#include <windows.h>

class WindowsEvent : public Event
{
public:
	WindowsEvent()
	: event(nullptr)
	{}

	virtual ~WindowsEvent()
	{
		if (event != nullptr)   CloseHandle(event);
	}

	virtual bool Create(bool manualReset = false) override
	{
		event = CreateEvent(nullptr, manualReset, 0, nullptr);
		this->manualReset = manualReset;

		return event != nullptr;
	}

	virtual bool IsManualReset() override { return manualReset; }
	virtual void Trigger() override;
	virtual void Reset() override;
	virtual bool Wait(uint32_t WaitTime) override;

private:
	HANDLE event;

	bool manualReset;
};