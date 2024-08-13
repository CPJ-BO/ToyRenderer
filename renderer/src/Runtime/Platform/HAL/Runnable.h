#pragma once

#include <cstdint>
#include <memory>

class Runnable
{
public:
	virtual bool Init() { return true; }
	virtual uint32_t Run() = 0;
	virtual void Stop() {}
	virtual void Exit() {}
	virtual ~Runnable() {}
};

typedef std::shared_ptr<Runnable> RunnableRef;