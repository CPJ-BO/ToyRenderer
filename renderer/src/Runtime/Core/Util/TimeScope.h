#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>

class TimeScope
{
public:
    TimeScope() { Clear(); }
    ~TimeScope() {}

    std::string name;

    void Clear();
    void Begin();
    void End();
    void EndAfterMilliSeconds(float milliSecondes);
    float GetMicroSeconds();
    float GetMilliSeconds();
    float GetSeconds();

    std::chrono::steady_clock::time_point GetBeginTime()    { return begin; }
    std::chrono::steady_clock::time_point GetEndTime()      { return end; }
    uint32_t GetDepth()                                     { return depth; }

private:
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    std::chrono::microseconds duration;
    uint32_t depth = 0;
    friend class TimeScopes;
} ;

class TimeScopes
{
public:
    TimeScopes() = default;
    ~TimeScopes() {}

    void PushScope(std::string name);
    void PopScope();
    void Clear();
    const std::vector<std::shared_ptr<TimeScope>>& GetScopes();
    inline uint32_t Size() const { return scopes.size(); }

private:
    std::vector<std::shared_ptr<TimeScope>> scopes;
    std::stack<std::shared_ptr<TimeScope>> runningScopes;
    uint32_t depth = 0;
};

class TimeScopeHelper
{
public:
    TimeScopeHelper(std::string name, TimeScopes* scopes)
    : scopes(scopes)
    {
        scopes->PushScope(name);
    }

    ~TimeScopeHelper()
    {
        scopes->PopScope();
    }

private:
    TimeScopes* scopes;
};