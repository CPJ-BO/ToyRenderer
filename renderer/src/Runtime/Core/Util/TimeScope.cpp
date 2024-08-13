#include "TimeScope.h"
#include "Core/Log/log.h"

#include <memory>

void TimeScope::Clear()
{
    begin = std::chrono::steady_clock::now(); 
    end = std::chrono::steady_clock::now(); 
    duration = std::chrono::nanoseconds::zero();
}

void TimeScope::Begin() 
{ 
    begin = std::chrono::steady_clock::now(); 
}

void TimeScope::End()
{
    end = std::chrono::steady_clock::now(); 
    duration = end - begin;
}

float TimeScope::GetMicroSeconds()
{
    return duration.count() / 1000.0f;
}

float TimeScope::GetMilliSeconds()
{
    return duration.count() / 1000000.0f;
}

float TimeScope::GetSeconds()
{
    return duration.count() / 1000000000.0f;
}

void TimeScopes::PushScope(std::string name)
{
    std::shared_ptr<TimeScope> newScope = std::make_shared<TimeScope>();
    newScope->name = name;
    newScope->Begin();

    if(depth == 0)
    {
        roots.push_back(newScope);
        current = newScope;     
    }
    else 
    {
        newScope->parent = current;
        current->children.push_back(newScope);
        current = newScope;
    }

    depth++;
}

void TimeScopes::PopScope()
{
    if(depth <= 0) 
    {
        LOG_FATAL("Time scope popping is not valid!");
        return;
    }

    current->End();
    current = current->parent;
    depth--;
}

void TimeScopes::Clear() 
{ 
    roots.clear();
    current = nullptr; 
    depth = 0; 
};

const std::vector<std::shared_ptr<TimeScope>>& TimeScopes::GetScopes()
{
    if(depth != 0) LOG_FATAL("Time scope recording is not complete!");
    return roots;
}