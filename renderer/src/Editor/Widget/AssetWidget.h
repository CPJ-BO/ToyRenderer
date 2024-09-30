#pragma once

#include "Core/UID/UID.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Model.h"
#include "Function/Render/RenderResource/Shader.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Resource/Asset/Asset.h"

#include <cstdint>
#include <imgui.h>
#include <memory>

class AssetWidget
{
public:
    static void DragUI(const AssetRef& inputAsset, ImVec2 ui_offset = ImVec2(0, 0))
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            // Set payload to carry the index of our item (could be anything)
            ImGui::SetDragDropPayload("ASSET_PATH", &inputAsset->GetUID(), sizeof(UID));

            // Display preview (could be anything, e.g. when dragging an image we could decide to display
            // the filename and a small preview of the image, etc.)
            std::string displayName = inputAsset->GetAssetTypeName() + ": " + inputAsset->GetUID().ToString();
            ImGui::Text("%s", displayName.c_str());

            ImGui::EndDragDropSource();
        }
    }

    template<typename TAsset, typename = std::enable_if_t<std::is_base_of<Asset, TAsset>::value, void>>
    static bool DropUI(std::shared_ptr<TAsset>& inputAsset, ImVec2 ui_offset = ImVec2(0, 0), std::string name = "")
    {
        bool update = false;

        ImGui::BeginChild("DropUI", ui_offset, false,
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoScrollWithMouse);

        if(name != "") ImGui::Button(name.c_str(), ImVec2(ImGui::GetWindowContentRegionMax().x, 25));

        ImGuiPayload payload = {};
        if (ImGui::BeginDragDropTarget())
        {
            auto payload = ImGui::AcceptDragDropPayload("ASSET_PATH");
            std::string path = std::string((char*)payload->Data);
            std::shared_ptr<TAsset> payloadAsset = EngineContext::Asset()->GetOrLoadAsset<TAsset>(path);

            inputAsset = payloadAsset;
            update = true;

            ImGui::EndDragDropTarget();
        }
        ImGui::EndChild();

        return update;
    }

    template<typename TAsset, typename = std::enable_if_t<std::is_base_of<Asset, TAsset>::value, void>>
    static bool UI(std::shared_ptr<TAsset>& inputAsset, uint64_t pushID = 0)	
    {
        ImGui::PushID(pushID);
        ImGui::Indent();
        AssetRef tempAsset = inputAsset;

        ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.4f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.4f, 0.4f));

        bool open = false;
        bool update = false;

        std::string id = inputAsset != nullptr ? inputAsset->GetAssetTypeName() + ": " + inputAsset->GetUID().ToString() : "Drag to load";

        if (inputAsset)   //资源非空
        {
            AssetType type = inputAsset->GetAssetType();

            ImGui::SetNextItemAllowOverlap();   //有这个按钮才能按

            //open = ImGui::CollapsingHeader(id.c_str(), ImGuiTreeNodeFlags_DefaultOpen);   // 默认打开
            open = ImGui::CollapsingHeader(id.c_str());

            ImGui::SameLine();
            ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - 100, 30));
            // ImGui::SameLine();                                                               // 资源置空
            // ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.5f);
            // if (ImGui::ImageButton("##3",
            //     Engine::AssetManager()->GetIcon("Delete2"),
            //     ImVec2(15.0f, 15.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
            // {
            //     if(inputAsset != nullptr) update = true;
            //     inputAsset = nullptr;
            // }
        }
        else
        {
            ImGui::SetNextItemAllowOverlap();   //有这个按钮才能按
            ImGui::CollapsingHeader(id.c_str());

            ImGui::SameLine();
            ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 30));
        }

        // ImGui::SameLine();
        // ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 32.5f);
        // update |= DropUI(inputAsset, ImVec2(ImGui::GetContentRegionAvail().x - 100, 25));

        if (open && inputAsset)
        {
            ImGui::PushID(inputAsset.get());
            {
                switch (inputAsset->GetAssetType()) {
                case ASSET_TYPE_MATERIAL:   update |= MaterialAssetUI(std::dynamic_pointer_cast<Material>(inputAsset));   break;
                case ASSET_TYPE_TEXTURE:    update |= TextureAssetUI(std::dynamic_pointer_cast<Texture>(inputAsset));      break;
                case ASSET_TYPE_SHADER:     update |= ShaderAssetUI(std::dynamic_pointer_cast<Shader>(inputAsset));         break; 
                case ASSET_TYPE_MODEL:      update |= ModelAssetUI(std::dynamic_pointer_cast<Model>(inputAsset));            break; 
                case ASSET_TYPE_ANIMATION:
                case ASSET_TYPE_SCENE:
                default:                                                                                                            break;
                }
            }
            ImGui::PopID();
        }

        ImGui::PopStyleColor(2);
        ImGui::Unindent();
        ImGui::PopID();
        return update;
    };

private:
    static bool MaterialAssetUI(std::shared_ptr<Material> material);
    static bool TextureAssetUI(std::shared_ptr<Texture> texture);
    static bool ModelAssetUI(std::shared_ptr<Model> model);
    static bool ShaderAssetUI(std::shared_ptr<Shader> shader);
};    


    