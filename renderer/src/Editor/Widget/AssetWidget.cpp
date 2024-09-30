#include "AssetWidget.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Texture.h"

#include <cstdint>
#include <imgui.h>

#include <memory>

bool AssetWidget::MaterialAssetUI(std::shared_ptr<Material> material)
{
    bool update = false;

    uint32_t pushID = 0;
    {
        ImGui::SeparatorText("Diffuse");
        update |= AssetWidget::UI(material->textureDiffuse, pushID++);
        ImGui::SeparatorText("Normal");
        update |= AssetWidget::UI(material->textureNormal, pushID++);
        ImGui::SeparatorText("AO/Roughness/Metallic");
        update |= AssetWidget::UI(material->textureArm, pushID++);
        ImGui::SeparatorText("Specular");
        update |= AssetWidget::UI(material->textureSpecular, pushID++);
    }

    ImGui::SeparatorText("");
    if(ImGui::CollapsingHeader("Additional texture"))
    {
        ImGui::Indent();
        if(ImGui::CollapsingHeader("Texture 2D"))
            for(auto& texture : material->texture2D)    update |= AssetWidget::UI(texture, pushID++);
        if(ImGui::CollapsingHeader("Texture Cube"))
            for(auto& texture : material->textureCube)  update |= AssetWidget::UI(texture, pushID++);
        if(ImGui::CollapsingHeader("Texture 3D"))
            for(auto& texture : material->texture3D)    update |= AssetWidget::UI(texture, pushID++);   
        ImGui::Unindent();
    }

    ImGui::SeparatorText("");
    {
        update |= ImGui::DragFloat("Roughness", &material->roughness, 0.001f, 0.0f, 1.0f);
        update |= ImGui::DragFloat("Metallic", &material->metallic, 0.001f, 0.0f, 1.0f);
        update |= ImGui::DragFloat("Alpha clip", &material->alphaClip, 0.001f, 0.0f, 1.0f);
        update |= ImGui::ColorEdit4("Base color", &material->diffuse[0]);
        update |= ImGui::ColorEdit4("Emission", &material->emission[0]);
    }

    ImGui::SeparatorText("");
    if(ImGui::CollapsingHeader("Pipeline states"))
    {
        int renderQueue = material->renderQueue;
        ImGui::InputInt("Render queue", &renderQueue);
        material->renderQueue = renderQueue;

        const char* cullModes[] = { "CULL_MODE_NONE", 
                                    "CULL_MODE_FRONT", 
                                    "CULL_MODE_BACK" };
        int cullMode = material->cullMode;
        update |= ImGui::Combo("Cull mode", &cullMode, cullModes, IM_ARRAYSIZE(cullModes));
        material->cullMode = (RasterizerCullMode)cullMode;

        const char* fillModes[] = { "FILL_MODE_POINT", 
                                    "FILL_MODE_WIREFRAME", 
                                    "FILL_MODE_SOLID" };
        int fillMode = material->fillMode;
        update |= ImGui::Combo("Fill mode", &fillMode, fillModes, IM_ARRAYSIZE(fillModes));
        material->fillMode = (RasterizerFillMode)fillMode;

        ImGui::Checkbox("Depth test", &material->depthTest);
        ImGui::SameLine();
        ImGui::Checkbox("Depth write", &material->depthWrite);

        const char* depthCompares[] = { "COMPARE_FUNCTION_LESS", 
                                        "COMPARE_FUNCTION_LESS_EQUAL", 
                                        "COMPARE_FUNCTION_GREATER",
                                        "COMPARE_FUNCTION_GREATER_EQUAL", 
                                        "COMPARE_FUNCTION_EQUAL", 
                                        "COMPARE_FUNCTION_NOT_EQUAL", 
                                        "COMPARE_FUNCTION_NEVER", 
                                        "COMPARE_FUNCTION_ALWAYS"};
        int depthCompare = material->depthCompare;
        update |= ImGui::Combo("Depth compare", &depthCompare, depthCompares, IM_ARRAYSIZE(depthCompares));
        material->depthCompare = (CompareFunction)depthCompare;

        ImGui::SeparatorText("Pass masks");
        ImGui::CheckboxFlags("Forward", &material->renderPassMask, PASS_MASK_FORWARD_PASS);
        ImGui::SameLine();
        ImGui::CheckboxFlags("Deferred", &material->renderPassMask, PASS_MASK_DEFERRED_PASS);
        ImGui::SameLine();
        ImGui::CheckboxFlags("Transparent", &material->renderPassMask, PASS_MASK_TRANSPARENT_PASS);

        ImGui::SeparatorText("Vertex shader");
        update |= AssetWidget::UI(material->vertexShader, pushID++);
        ImGui::SeparatorText("Geometry shader");
        update |= AssetWidget::UI(material->geometryShader, pushID++);
        ImGui::SeparatorText("Fragment shader");
        update |= AssetWidget::UI(material->fragmentShader, pushID++);
    }

    if(update) material->Update();
    return update;
}

bool AssetWidget::TextureAssetUI(std::shared_ptr<Texture> texture)
{
    bool update = false;

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("table1", 2, flags))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Type");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", texture->textureType);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Format");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", texture->format);

        ImGui::EndTable();
    }

    ImGui::SeparatorText("Paths");
    ImGui:ImGui::Indent();
    for (std::string& path : texture->paths)
    {
        
        ImGui::Text("%s", path.c_str());

        // ImGui_Text::InputText(std::string("##").append(std::to_string((uint64_t)(this) + (uint64_t)&path)).c_str(), &path, 0);
        // if (ImGui::IsItemDeactivatedAfterEdit()) {}
    }
    ImGui::Unindent();

    return update;
}

bool AssetWidget::ModelAssetUI(std::shared_ptr<Model> model)
{
    bool update = false;

    ImGui::Text("Total Vertex number: %lld", model->totalVertex);
    ImGui::Text("Total Index number: %lld", model->totalIndex);
    ImGui::Text("Total Mesh cluster number: %d", model->totalClusterCnt);
    ImGui::Text("Total Mesh cluster max mip: %d", model->totalClusterMaxMip);

    ImGui::Indent();
    ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.4f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.4f, 0.4f));

    if (ImGui::CollapsingHeader("Bones", ImGuiTreeNodeFlags_None))
    {
        for(auto& submesh : model->submeshes)
        {
            ImGui::SeparatorText(submesh.mesh->name.c_str());
            for (auto& boneInfo : submesh.mesh->bone)
            {
                static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                if (ImGui::BeginTable("table2", 2, flags))
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("ID: %d", boneInfo.index);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", boneInfo.name.c_str());

                    ImGui::EndTable();
                }
            }
        }
    }

    for (int i = 0; i < model->submeshes.size(); i++)
    {
        auto& mesh = model->submeshes[i].mesh;

        std::string title = "Sub mesh " + std::to_string(i) + " " + mesh->name;
        if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_None))
        {
            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
            if (ImGui::BeginTable("table1", 2, flags))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Vertex number");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", (uint32_t)mesh->position.size());

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Index number");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", (uint32_t)mesh->index.size());

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Triangle number");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", mesh->TriangleNum());

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Has tangent");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", mesh->tangent.size() > 0 ? "True" : "False");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Has color");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", mesh->color.size() > 0 ? "True" : "False");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Has texCoord");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", mesh->texCoord.size() > 0 ? "True" : "False");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Has bone");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", mesh->bone.size() > 0 ? "True" : "False");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("AABB max");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("[%f, %f, %f]",
                    mesh->aabb.GetMaxCorner().x(),
                    mesh->aabb.GetMaxCorner().y(),
                    mesh->aabb.GetMaxCorner().z());

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("AABB min");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("[%f, %f, %f]",
                    mesh->aabb.GetMinCorner().x(),
                    mesh->aabb.GetMinCorner().y(),
                    mesh->aabb.GetMinCorner().z());

                ImGui::EndTable();
            }
        }
    }
    ImGui::PopStyleColor(2);
    ImGui::Unindent();

    return update;
}

bool AssetWidget::ShaderAssetUI(std::shared_ptr<Shader> shader)
{
    bool update = false;

    ImGui::Text("%s", shader->path.c_str());

    return update;
}