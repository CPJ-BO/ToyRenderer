#include "EditorSystem.h"
#include "Function/Global/EngineContext.h"
#include "Platform/Input/InputSystem.h"
#include "Widget/HierarchyWidget.h"
#include "Widget/InspectorWidget.h"

void EditorSystem::Init()
{

}

void EditorSystem::UI()
{
    if(EngineContext::Input()->OnKeyPress(KEY_TYPE_Q))  show = !show;
    if(show)
    {
        HierarchyWidget::UI();
        InspectorWidget::UI();
    }
}