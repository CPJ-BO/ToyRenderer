#include "TimeScope.h"
#include "Core/Log/log.h"
#include "Platform/HAL/PlatformProcess.h"

#include <memory>

void TimeScope::Clear()
{
    begin = std::chrono::steady_clock::now(); 
    end = std::chrono::steady_clock::now(); 
    duration = std::chrono::microseconds::zero();
}

void TimeScope::Begin() 
{ 
    begin = std::chrono::steady_clock::now(); 
}

void TimeScope::End()
{
    end = std::chrono::steady_clock::now(); 
    auto diff = end - begin;
    duration = std::chrono::duration_cast<std::chrono::microseconds>(diff);
}

void TimeScope::EndAfterMilliSeconds(float milliSecondes)
{
    End();
    if(GetMilliSeconds() < milliSecondes)
    {
        PlatformProcess::Sleep((milliSecondes - GetMilliSeconds()) / 1000.0f);
    }
    End();
}

float TimeScope::GetMicroSeconds()
{
    return (float)duration.count();
}

float TimeScope::GetMilliSeconds()
{
    return (float)duration.count() / 1000;
}

float TimeScope::GetSeconds()
{
    return (float)duration.count() / 1000000;
}

void TimeScopes::PushScope(std::string name)
{
    std::shared_ptr<TimeScope> newScope = std::make_shared<TimeScope>();
    newScope->name = name;
    newScope->depth = depth;
    newScope->Begin();

    scopes.push_back(newScope); 
    runningScopes.push(newScope);

    depth++;
}

void TimeScopes::PopScope()
{
    if(runningScopes.empty()) 
    {
        LOG_FATAL("Time scope popping is not valid!");
        return;
    }

    auto scope = runningScopes.top();
    runningScopes.pop();
    scope->End();

    depth--;
}

void TimeScopes::Clear() 
{ 
    scopes.clear();
    runningScopes = {};
    depth = 0; 
};

const std::vector<std::shared_ptr<TimeScope>>& TimeScopes::GetScopes()
{
    if(depth != 0) LOG_FATAL("Time scope recording is not complete!");
    return scopes;
}