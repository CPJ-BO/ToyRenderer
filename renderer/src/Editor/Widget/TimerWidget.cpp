#include "TimerWidget.h"

#include "Core/Util/TimeScope.h"

#include "imgui_widget_flamegraph.h"


static void ProfilerValueGetter(float* startTimestamp, float* endTimestamp, ImU8* level, const char** caption, const void* data, int idx)
{
    auto scopes = (TimeScopes*) data;

    auto firstScope = scopes->GetScopes()[0];
    auto scope = scopes->GetScopes()[idx];

    if (startTimestamp)
    {
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(scope->GetBeginTime() - firstScope->GetBeginTime());
        *startTimestamp = (float)time.count() / 1000;
    }
    if (endTimestamp)
    {
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(scope->GetEndTime() - firstScope->GetBeginTime());
        *endTimestamp = (float)time.count() / 1000;
    }
    if (level)
    {
        *level = scope->GetDepth();
    }
    if (caption)
    {
        *caption = scope->name.c_str();
    }
}

void TimerWidget::TimeScopeUI(const TimeScopes& timeScopes)
{
    ImGuiWidgetFlameGraph::PlotFlame(
        "", 
        ProfilerValueGetter, 
        &timeScopes, 
        timeScopes.Size(),
        0,
        "Main Thread Time##", 
        FLT_MAX, 
        FLT_MAX, 
        ImVec2(0, 0));
}