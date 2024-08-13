#pragma once

#include <memory>

enum QueuedWorkPriority
{
	WORK_PRIORITY_BLOCK = 0,
	WORK_PRIORITY_HIGHEST,
	WORK_PRIORITY_HIGH,
	WORK_PRIORITY_NORMAL,
	WORK_PRIORITY_LOW,
	WORK_PRIORITY_LOWEST,
	
    WORK_PRIORITY_MAX_ENUM, //
};

typedef std::shared_ptr<class QueuedWork> QueuedWorkRef;
class QueuedWork
{
public:
    virtual void DoThreadedWork() 
    {
        //SetPromise(Promise, Function);
        //delete this;
    }

    virtual void Abandon()
    {
        // not supported
    }

    bool operator<(const QueuedWork& other)
    {
        return this->priority < other.priority;
    }

    struct Compare {
        bool operator()(const QueuedWorkRef& left, const QueuedWorkRef& right) const {
            return left->priority < right->priority;	// 高到低
        }
    };

protected:
    // TUniqueFunction<ResultType()> Function; // 被执行的函数列表.
    // TPromise<ResultType> Promise; // 用于同步的对象

    QueuedWorkPriority priority = WORK_PRIORITY_NORMAL;
};



