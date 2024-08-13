#pragma once

#include "QueuedThread.h"
#include "QueuedWork.h"
#include "Platform/HAL/RunnableThread.h"
#include "Platform/HAL/CriticalSection.h"

#include <cstdint>
#include <memory>
#include <queue>
#include <string>
#include <vector>


typedef std::shared_ptr<class QueuedThreadPool> QueuedThreadPoolRef;
class QueuedThreadPool
{
public: 
	QueuedThreadPool() = default;
	~QueuedThreadPool() { Destroy(); };

	static QueuedThreadPoolRef Create(
        uint32_t numThreads, 
        uint32_t stackSize = (32 * 1024), 
        ThreadPriorityType priority = PRIORITY_TYPE_NORMAL, 
        const std::string& name = "");

	void Destroy();
	void AddQueuedWork(QueuedWorkRef queuedWork);
	int32_t GetNumThreads() const { return allThreads.size(); };

private:
	std::priority_queue<QueuedWorkRef, std::vector<QueuedWorkRef>, QueuedWork::Compare> works = {};      // 所有暂未执行的任务
	std::queue<QueuedThread*> idleThreads = {};       	// 暂未运行的线程
	std::vector<QueuedThreadRef> allThreads = {};       // 所有分配的线程

	CriticalSectionRef sync;							// 临界区，锁线程资源
	bool timeToDie;
	uint32_t cnt;

	QueuedWorkRef ReturnToPoolOrGetNextWork(QueuedThread* queuedThread);
	friend class QueuedThread;
};
