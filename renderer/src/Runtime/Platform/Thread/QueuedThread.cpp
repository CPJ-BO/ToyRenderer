#include "QueuedThread.h"
#include "QueuedThreadPool.h"
#include "Core/Log/Log.h"

#include <cassert>

uint32_t QueuedRunnable::Run() 
{
	while (!timeToDie)
	{
		doWorkEvent->Wait();
		if(timeToDie) break;

		QueuedWorkRef localQueuedWork = queuedWork;
		queuedWork = nullptr;

		assert(localQueuedWork != nullptr);		
		while (localQueuedWork)
		{
			localQueuedWork->DoThreadedWork();
			localQueuedWork = ownerThread->TryGetNextWork();
		}
	}
	return 0;
}

void QueuedThread::KillThread()
{
	runnable->timeToDie = true;
	runnable->doWorkEvent->Trigger();

	thread->WaitForCompletion();
}

void QueuedThread::StartWork(QueuedWorkRef queuedWork)    //仅在未启动时调用
{
	if(runnable->queuedWork != nullptr) LOG_DEBUG("QueuedThread is already working!");
	runnable->queuedWork = queuedWork;
	runnable->doWorkEvent->Trigger();
}

QueuedWorkRef QueuedThread::TryGetNextWork()
{
	return ownerPool->ReturnToPoolOrGetNextWork(this);
}