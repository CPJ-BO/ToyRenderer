#include "HierarchyWidget.h"
#include "Function/Global/EngineContext.h"

#include <imgui.h>

void HierarchyWidget::UI()
{
	ImGui::Begin("Hierarchy");
    auto scene = EngineContext::World()->GetActiveScene();
    if(scene)
    {
        for (auto& entity : scene->GetEntities())
        {
            if(entity->GetFather().lock() == nullptr)   // 顶层只显示无父物体的
            {
                EntityUI(entity);
            }    
        }
    }
	ImGui::End();
}

void HierarchyWidget::EntityUI(std::shared_ptr<Entity> entity)
{
	if (ImGui::TreeNode(entity->GetName().c_str()))
	{
		if (ImGui::IsItemFocused()) {
            EngineContext::Editor()->SetSelectedEntity(entity);
		}

		//子物体
		for (auto& child : entity->GetChildren()) EntityUI(child);

		ImGui::TreePop();
	}    
}