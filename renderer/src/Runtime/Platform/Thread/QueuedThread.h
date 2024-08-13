#pragma once

#include "Platform/HAL/PlatformProcess.h"
#include "QueuedWork.h"

#include <memory>

class QueuedThread;
class QueuedThreadPool;

class QueuedRunnable : public Runnable
{
public:
	virtual uint32_t Run() override;

    QueuedRunnable(QueuedThread* owner)
    : timeToDie(false)
    , queuedWork(nullptr)
    , doWorkEvent(PlatformProcess::CreateSyncEvent(false))
    , ownerThread(owner)
    {}

protected:
    bool timeToDie;
    QueuedWorkRef queuedWork;
    EventRef doWorkEvent;
    QueuedThread* ownerThread;

    friend class QueuedThread;
};
typedef std::shared_ptr<QueuedRunnable> QueuedRunnableRef;


typedef std::shared_ptr<QueuedThread> QueuedThreadRef;
class QueuedThread
{
public:
	static QueuedThreadRef Create(
        QueuedThreadPool* pool, 
        uint32_t stackSize = 0, 
        ThreadPriorityType priority = PRIORITY_TYPE_NORMAL)
	{
        QueuedThreadRef queuedThread = std::make_shared<QueuedThread>();
		queuedThread->ownerPool = pool;
        queuedThread->runnable = std::make_shared<QueuedRunnable>(queuedThread.get());
		queuedThread->thread = RunnableThread::Create(queuedThread->runnable, stackSize, priority);

        return queuedThread;
	}
 
    QueuedThread() = default;
    ~QueuedThread() {};
	
	void KillThread();
	void StartWork(QueuedWorkRef queuedWork);    //仅在未启动时调用
    QueuedWorkRef TryGetNextWork();

private:
    

    QueuedRunnableRef runnable;
	RunnableThreadRef thread;

	QueuedThreadPool* ownerPool = nullptr;
};