#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class TimeScope
{
public:
    TimeScope() { Clear(); }
    ~TimeScope() {}

    std::string name;

    std::shared_ptr<TimeScope> parent;
    std::vector<std::shared_ptr<TimeScope>> children;

    void Clear();
    void Begin();
    void End();
    float GetMicroSeconds();
    float GetMilliSeconds();
    float GetSeconds();

private:
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    std::chrono::nanoseconds duration;
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

private:
    std::shared_ptr<TimeScope> current;
    std::vector<std::shared_ptr<TimeScope>> roots;
    uint32_t depth = 0;
};