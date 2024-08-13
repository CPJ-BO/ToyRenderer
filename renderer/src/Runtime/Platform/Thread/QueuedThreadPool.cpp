#include "QueuedThreadPool.h"
#include "Platform/HAL/PlatformProcess.h"
#include "Platform/HAL/ScopeLock.h"
#include "Core/Log/Log.h"
#include "QueuedThread.h"
#include "QueuedWork.h"

#include <cassert>
#include <cstdint>
#include <memory>

QueuedThreadPoolRef QueuedThreadPool::Create(
    uint32_t numThreads, 
    uint32_t stackSize, 
    ThreadPriorityType priority, 
    const std::string& name)
{
    QueuedThreadPoolRef pool = std::make_shared<QueuedThreadPool>();
    pool->sync = PlatformProcess::CreateCriticalSection();

    for (uint32_t i = 0; i < numThreads; i++)
    {
        QueuedThreadRef thread = QueuedThread::Create(pool.get(), stackSize, priority);
        pool->idleThreads.push(thread.get());
        pool->allThreads.push_back(thread);
    }
    return pool;
}

void QueuedThreadPool::AddQueuedWork(QueuedWorkRef queuedWork)
{
    assert(queuedWork != nullptr);

    if (timeToDie)
    {
        queuedWork->Abandon();
        return;
    }

    QueuedThread* thread = nullptr;
    {
        ScopeLock lock(sync);   //加锁
        if (idleThreads.size() == 0)
        {
            works.push(queuedWork);
            return;
        }

        thread = idleThreads.front();
        idleThreads.pop();
    }

    thread->StartWork(queuedWork);
}

void QueuedThreadPool::Destroy()
{
   {
        timeToDie = true;
        ScopeLock lock(sync);   //加锁
        while (works.size() > 0)
        {   
            QueuedWorkRef work = works.top();   
            works.pop();
            work->Abandon();
        }
    }
    while (true)
    {
        {
            ScopeLock lock(sync);   //加锁
            if (allThreads.size() == idleThreads.size()) 
            {            
                while (idleThreads.size() > 0)
                {
                    QueuedThread* thread = idleThreads.front();
                    thread->KillThread();
                    idleThreads.pop();
                }
                allThreads.clear();
                break;
            }
        }        
        PlatformProcess::Sleep(0.0f); 
    }
}

QueuedWorkRef QueuedThreadPool::ReturnToPoolOrGetNextWork(QueuedThread* queuedThread)   //由QueuedThread调用
{
    assert(queuedThread != nullptr);
    QueuedWorkRef work = nullptr;
    {
        ScopeLock lock(sync);   //加锁 

        cnt++;
        if(cnt % 1000 == 0)  
        {
            LOG_DEBUG("QueuedThreadPool finished %d works", cnt);
            //cnt = 0;
        }

        if (works.size() > 0)
        {
            work = works.top();
            works.pop();
        }

        if (!work) idleThreads.push(queuedThread);
    }
    return work;
}