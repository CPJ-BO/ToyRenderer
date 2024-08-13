#pragma once

#include <cstdint>
#include <memory>

class Semaphore // 信号量抽象
{
public:
    Semaphore() {};
    Semaphore(const Semaphore&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;

    virtual void Lock() = 0;
    virtual bool TryLock(uint64_t milliseconds) = 0;
    virtual void Unlock() = 0;
};

typedef std::shared_ptr<Semaphore> SemaphoreRef;