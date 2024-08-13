#pragma once

#include <memory>

class CriticalSection	// 临界区抽象
{
public:
    CriticalSection() {};
	CriticalSection(const CriticalSection&) = delete;
	CriticalSection& operator=(const CriticalSection&) = delete;

	inline virtual void Lock() = 0;
	inline virtual bool TryLock() = 0;
	inline virtual void Unlock() = 0;
};
typedef std::shared_ptr<CriticalSection> CriticalSectionRef;