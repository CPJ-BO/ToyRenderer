#pragma once

#include "Platform/HAL/Runnable.h"
#include "Platform/HAL/RunnableThread.h"
#include "Platform/Thread/QueuedThreadPool.h"
#include "Platform/Thread/QueuedWork.h"

#include <iostream>
#include <vector>

class TestRun : public Runnable
{
public:
    virtual bool Init() override { return true; }   //初始化函数，返回true才会执行run和exit
	virtual uint32_t Run() override 
    { 
        while (cnt < 100000000) {
            cnt++;
            if(cnt % 100000 == 0) std::cout << cnt << std::endl;
        }

        return 0;
    };
	virtual void Stop() override {} //Kill的退出函数，可能被主动中途调用或runnablethread析构时调用
	virtual void Exit() override {} //正常执行完毕的退出函数
	virtual ~TestRun() {}

private:
    uint64_t cnt = 0;
};

class TestWork : public QueuedWork
{
public:
    TestWork(QueuedWorkPriority priority) { this->priority = priority; }

    virtual void DoThreadedWork() override 
    {
        while (cnt < 1000000) {
            cnt++;
            if(cnt % 100000 == 0) std::cout << cnt << std::endl;
        }
        std::cout << "Work finished : " << priority << std::endl;
    }

    virtual void Abandon() override {}

private:
    uint64_t cnt = 0;
};

static void TestHAL()
{
    // runnable
    std::vector<std::shared_ptr<TestRun>> runs;
    std::vector<RunnableThreadRef> threads;
    for(int i = 0; i < 5; i++)
    {
        runs.push_back(std::make_shared<TestRun>());
        threads.push_back(RunnableThread::Create(runs[i]));
    }
    for(int i = 0; i < threads.size(); i++)
    {
        //threads[i]->Suspend(true);
        //threads[i]->Kill(false);
    }
    threads[0]->WaitForCompletion();

    //queued work
    QueuedThreadPoolRef threadPool = QueuedThreadPool::Create(10);
    for(int i = 0; i < 100; i++) 
    {
        threadPool->AddQueuedWork(std::make_shared<TestWork>(WORK_PRIORITY_NORMAL));
        threadPool->AddQueuedWork(std::make_shared<TestWork>(WORK_PRIORITY_HIGHEST));
        threadPool->AddQueuedWork(std::make_shared<TestWork>(WORK_PRIORITY_LOWEST));
    }
    PlatformProcess::Sleep(2.0);
    threadPool->Destroy();
}