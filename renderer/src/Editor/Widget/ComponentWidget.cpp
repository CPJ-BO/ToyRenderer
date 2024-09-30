#include "ComponentWidget.h"
#include "AssetWidget.h"
#include "Core/Math/Math.h"
#include "Function/Framework/Component/Component.h"
#include "Function/Framework/Component/MeshRendererComponent.h"
#include "Function/Render/RenderResource/Material.h"

#include <cstdint>
#include <imgui.h>

#include <memory>

void ComponentWidget::UI(std::shared_ptr<Component> component)
{
    ImGui::PushID(component.get());
    if (ImGui::CollapsingHeader(component->GetTypeName().c_str(), ImGuiTreeNodeFlags_None))
    {
        switch (component->GetType()) {
        case TRANSFORM_COMPONENT:           TransformComponentUI(std::static_pointer_cast<TransformComponent>(component));                  break;
        case CAMERA_COMPONENT:              CameraComponentUI(std::static_pointer_cast<CameraComponent>(component));                        break;
        case POINT_LIGHT_COMPONENT:         PointLightComponentUI(std::static_pointer_cast<PointLightComponent>(component));                break;
        case DIRECTIONAL_LIGHT_COMPONENT:   DirectionalLightComponentUI(std::static_pointer_cast<DirectionalLightComponent>(component));    break;   
        case MESH_RENDERER_COMPONENT:       MeshRendererComponentUI(std::static_pointer_cast<MeshRendererComponent>(component));            break;   
		case SKYBOX_COMPONENT:				SkyboxComponentUI(std::static_pointer_cast<SkyboxComponent>(component));						  break;
        default:                                                                                                                                             break;
        }
    }
    ImGui::PopID();
}

void ComponentWidget::CameraComponentUI(std::shared_ptr<CameraComponent> component)
{
    ImGui::Text("Camera position: [%f, %f, %f]", 
        component->position[0], 
        component->position[1], 
        component->position[2]);

    ImGui::Text("Camera front: [%f, %f, %f]", 
        component->front[0], 
        component->front[1], 
        component->front[2]);
	
	ImGui::DragFloat("Fov", &component->fovy);
}

void ComponentWidget::TransformComponentUI(std::shared_ptr<TransformComponent> component)
{
    Vec3 position = component->transform.GetPosition();
    Vec3 eulerAngle = component->transform.GetEulerAngle();
    Vec3 scale = component->transform.GetScale();

    ImGui::DragFloat3("Position", &position[0], 0.1f);
	ImGui::DragFloat3("Rotation", &eulerAngle[0]);
    ImGui::DragFloat3("Scale", &scale[0], 0.1f);

    component->transform.SetPosition(position);
    component->transform.SetRotation(eulerAngle);
    component->transform.SetScale(scale);
}

void ComponentWidget::DirectionalLightComponentUI(std::shared_ptr<DirectionalLightComponent> component)
{
    ImGui::Text("DirectionalLight front: [%f, %f, %f]", 
        component->front[0], 
        component->front[1], 
        component->front[2]);

	ImGui::Text("DirectionalLight up: [%f, %f, %f]", 
        component->up[0], 
        component->up[1], 
        component->up[2]);

	ImGui::DragInt4("Cascade update frequency", &component->updateFrequences[0], 0.1f, 1, 10);
	ImGui::DragFloat("Cascade split lambda", &component->cascadeSplitLambda, 0.01f);
	for (int i = 0; i < DIRECTIONAL_SHADOW_CASCADE_LEVEL; i++)
	{
		ImGui::BulletText("Cascade [%d] depth: %f", i, component->lightInfos[i].depth);
	}
	ImGui::DragFloat("Constant bias", &component->constantBias, 0.05f);
	ImGui::DragFloat("Slope bias", &component->slopeBias, 0.05f);
	ImGui::ColorEdit3("Color", &component->color[0]);
	ImGui::DragFloat("Intencity", &component->intencity, 0.05f, 0);
	ImGui::DragFloat("Fog scattering", &component->fogScattering, 0.001f, 0.0f, 1.0f);
}

void ComponentWidget::PointLightComponentUI(std::shared_ptr<PointLightComponent> component)
{
    ImGui::Checkbox("Enable", &component->enable);
	ImGui::SameLine(); ImGui::Checkbox("Cast shadow", &component->castShadow);

	ImGui::Text("Bounding box min: [%f, %f, %f]",
		component->box.minBound[0], 
        component->box.minBound[1], 
        component->box.minBound[2]);

	ImGui::Text("Bounding box max: [%f, %f, %f]",
		component->box.maxBound[0], 
        component->box.maxBound[1], 
        component->box.maxBound[2]);

	ImGui::DragFloat("Constant bias", &component->constantBias, 0.0005f);
	ImGui::DragFloat("Slope bias", &component->slopeBias, 0.05f);
	ImGui::ColorEdit3("Color", &component->color[0]);
	ImGui::DragFloat("Intencity", &component->intencity, 0.05f, 0);
	ImGui::DragFloat("Range", &component->far, 0.05f, 0.1f);
	ImGui::DragFloat2("EVSM", &component->evsm[0]);
	ImGui::DragFloat("Fog scattering", &component->fogScattering, 0.001f, 0.0f, 1.0f);
}

void ComponentWidget::MeshRendererComponentUI(std::shared_ptr<MeshRendererComponent> component)
{
    
    if (ImGui::RadioButton("Batch", component->renderMode == RENDER_MODE_DEFAULT))
		component->renderMode = RENDER_MODE_DEFAULT;
	ImGui::SameLine();
	if (ImGui::RadioButton("Cluster", component->renderMode == RENDER_MODE_CLUSTER))
		component->renderMode = RENDER_MODE_CLUSTER;
	ImGui::SameLine();
	if (ImGui::RadioButton("Virtual mesh", component->renderMode == RENDER_MODE_VIRTUAL_MESH))
		component->renderMode = RENDER_MODE_VIRTUAL_MESH;

	ImGui::Checkbox("Cast shadow", &component->castShadow);
	
	ImGui::SeparatorText("Mesh:");
	AssetWidget::UI(component->model);
	
	ImGui::SeparatorText("Materials:");
	ImGui::Text("Inspect mode:");
	ImGui::SameLine();
	if (ImGui::RadioButton("Show all", component->materilalInspectMode == 0))
		component->materilalInspectMode = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("Filter", component->materilalInspectMode == 1))
		component->materilalInspectMode = 1;

	ImGui::InputInt("Sub mesh index", &component->materilalInspectIndex);
	component->materilalInspectIndex = component->materilalInspectIndex % component->model->GetSubmeshCount();

	if (component->materilalInspectMode == 0)
	{
		for(int i = 0; i < component->model->GetSubmeshCount(); i++)
		{
			uint32_t index = i;
			uint64_t pushID = component->materials[index] != nullptr ? (uint64_t)component->materials[index].get() : index;
			ImGui::PushID(pushID);
			ImGui::SeparatorText(("[Sub mesh : " + std::to_string(index) + "]").c_str());
            AssetWidget::UI(component->materials[index]);
			ImGui::PopID();
		}
	}
	else if (component->materilalInspectMode == 1)
	{
		uint32_t index = component->materilalInspectIndex;
		ImGui::SeparatorText(("[Sub mesh : " + std::to_string(index) + "]").c_str());
		AssetWidget::UI(component->materials[index]);
	}
}

void ComponentWidget::SkyboxComponentUI(std::shared_ptr<SkyboxComponent> component)
{
	ImGui::SeparatorText("Skybox texture:");
	AssetWidget::UI(component->skyboxTexture);
}