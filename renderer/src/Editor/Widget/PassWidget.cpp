#include "PassWidget.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderPass/BloomPass.h"
#include "Function/Render/RenderPass/PostProcessingPass.h"
#include "Function/Render/RenderPass/FXAAPass.h"
#include "Function/Render/RenderPass/RenderPass.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <implot.h>
#include <imgui.h>

#include <cstdint>
#include <string>

void PassWidget::UI(std::shared_ptr<RenderPass> pass)
{
    ImGui::PushID(pass.get());
    if (ImGui::CollapsingHeader(pass->GetName().c_str(), ImGuiTreeNodeFlags_None))
    {
        switch (pass->GetType()) {                                                                                                                                         
        case DEPTH_PASS:                                                                                                                  break;
        case GPU_CULLING_PASS:          GPUCullingPassUI(std::static_pointer_cast<GPUCullingPass>(pass));                   break;
        case POINT_SHADOW_PASS:                                                                                                         break;
        case DIRECTIONAL_SHADOW_PASS:                                                                                                   break;
        case G_BUFFER_PASS:             GBufferPassUI(std::static_pointer_cast<GBufferPass>(pass));                         break;
        case DEFERRED_LIGHTING_PASS:    DeferredLightingPassUI(std::static_pointer_cast<DeferredLightingPass>(pass));       break;
        case FORWARD_PASS:                                                                                                              break;
        case TRANSPARENT_PASS:                                                                                                          break;
        case BLOOM_PASS:                BloomPassUI(std::static_pointer_cast<BloomPass>(pass));                             break;
        case FXAA_PASS:                 FXAAPassUI(std::static_pointer_cast<FXAAPass>(pass));                               break;
        case EXPOSURE_PASS:             ExposurePassUI(std::static_pointer_cast<ExposurePass>(pass));                       break;
        case POST_PROCESSING_PASS:      PostProcessingPassUI(std::static_pointer_cast<PostProcessingPass>(pass));           break;
        case EDITOR_UI_PASS:                                                                                                            break;
        case PRESENT_PASS:                                                                                                              break;
        default:                                                                                                                        break;
        }
    }
    ImGui::PopID();
}

std::string MeshPassName(MeshPassType type)
{
    switch (type) {
    case MESH_DEPTH_PASS:               return "Depth";
    case MESH_POINT_SHADOW_PASS:        return "Point Shadow";
    case MESH_DIRECTIONAL_SHADOW_PASS:  return "Dir Shadow";
    case MESH_G_BUFFER_PASS:            return "G-Buffer";
    case MESH_FORWARD_PASS:             return "Forward";
    case MESH_TRANSPARENT_PASS:         return "Transparent";
    default:                            return "";
    }
}

void PassWidget::GPUCullingPassUI(std::shared_ptr<GPUCullingPass> pass)
{
    ImGui::DragFloat("Lod error rate", &pass->lodSetting.lodErrorRate, 0.002f);
    ImGui::DragFloat("Lod error offset", &pass->lodSetting.lodErrorOffset, 0.002f);

    ImGui::Checkbox("Statistics", &pass->enableStatistics);
	if (pass->enableStatistics)
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        ImGui::SeparatorText("Meshes: ");
        if (ImGui::BeginTable("table1", 5, flags))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Mesh pass");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Processed");
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("Frustum");
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("Occlu");
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("Rendered");

            for(uint32_t i = 0; i < pass->readBackDatas.size(); i++)
            {
                std::string name = MeshPassName((MeshPassType)i);
                for(auto& readBackData : pass->readBackDatas[i])
                {        
                    auto& setting = readBackData.meshSetting;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", setting.processSize);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", setting.frustumCull);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", setting.occlusionCull);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", setting.drawSize);
                }
            }
            ImGui::EndTable();
        }

        ImGui::SeparatorText("Clusters: ");
        if (ImGui::BeginTable("table2", 5, flags))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Mesh pass");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Processed");
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("Frustum");
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("Occlu");
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("Rendered");

            for(uint32_t i = 0; i < pass->readBackDatas.size(); i++)
            {
                std::string name = MeshPassName((MeshPassType)i);
                for(auto& readBackData : pass->readBackDatas[i])
                {        
                    auto& setting = readBackData.clusterSetting;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", setting.processSize);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", setting.frustumCull);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", setting.occlusionCull);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", setting.drawSize);
                }
            }
            ImGui::EndTable();
        }

        ImGui::SeparatorText("Cluster groups: ");
        if (ImGui::BeginTable("table3", 5, flags))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Mesh pass");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Processed");
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("Frustum");
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("Occlu");
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("Rendered");

            for(uint32_t i = 0; i < pass->readBackDatas.size(); i++)
            {
                std::string name = MeshPassName((MeshPassType)i);
                for(auto& readBackData : pass->readBackDatas[i])
                {        
                    auto& setting = readBackData.clusterGroupSetting;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", setting.processSize);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", setting.frustumCull);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", setting.occlusionCull);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", setting.drawSize);
                }
            }
            ImGui::EndTable();
        }
	}
}

void PassWidget::GBufferPassUI(std::shared_ptr<GBufferPass> pass)
{

}

void PassWidget::DeferredLightingPassUI(std::shared_ptr<DeferredLightingPass> pass)
{
	if (ImGui::RadioButton("Color", pass->setting.mode == 0))
		pass->setting.mode = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("Normal", pass->setting.mode == 1))
		pass->setting.mode = 1;
	ImGui::SameLine();
	if (ImGui::RadioButton("Roughness/Metallic", pass->setting.mode == 2))
		pass->setting.mode = 2;

    //////////////////////////////////
	if (ImGui::RadioButton("World pos", pass->setting.mode == 3))
		pass->setting.mode = 3;
	ImGui::SameLine();
	if (ImGui::RadioButton("Shadow", pass->setting.mode == 4))
		pass->setting.mode = 4;
	ImGui::SameLine();
	if (ImGui::RadioButton("AO", pass->setting.mode == 5))
		pass->setting.mode = 5;
	ImGui::SameLine();
	if (ImGui::RadioButton("Shadow cascade", pass->setting.mode == 6))
		pass->setting.mode = 6;

    //////////////////////////////////
	if (ImGui::RadioButton("Cluster light count", pass->setting.mode == 7))
		pass->setting.mode = 7;
    ImGui::SameLine();
	if (ImGui::RadioButton("Cluster XY", pass->setting.mode == 8))
		pass->setting.mode = 8;
	ImGui::SameLine();
	if (ImGui::RadioButton("Cluster Z", pass->setting.mode == 9))
		pass->setting.mode = 9;

    //////////////////////////////////
	if (ImGui::RadioButton("Indirect light", pass->setting.mode == 10))
		pass->setting.mode = 10;
	ImGui::SameLine();
	if (ImGui::RadioButton("Indirect irradiance", pass->setting.mode == 11))
		pass->setting.mode = 11;
	ImGui::SameLine();
	if (ImGui::RadioButton("Direct light", pass->setting.mode == 12))
		pass->setting.mode = 12;
}

void PassWidget::BloomPassUI(std::shared_ptr<BloomPass> pass)
{
	ImGui::DragFloat("Blur stride", &pass->setting.stride, 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat("Mip bias", &pass->setting.bias, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("Accumulate intencity", &pass->setting.accumulateIntencity, 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat("Combine intencity", &pass->setting.combineIntencity, 0.1f, 0.0f, 1.0f);
}

void PassWidget::FXAAPassUI(std::shared_ptr<FXAAPass> pass)
{
    bool enable = pass->enable > 0 ? true : false;
    ImGui::Checkbox("Enable", &enable);
    pass->enable = enable;
}

void PassWidget::ExposurePassUI(std::shared_ptr<ExposurePass> pass)
{
    ImGui::DragFloat("Min luminance", &pass->minLuminance, 0.01f);
	ImGui::DragFloat("Max luminance", &pass->maxLuminance, 0.1f);
	ImGui::DragFloat("Adjust speed", &pass->adjustSpeed, 0.1f);
    ImGui::Checkbox("Statistics", &pass->enableStatistics);
	if (pass->enableStatistics)
    {
        ImGui::Text("Avarage luminance: %f", pass->readBackData.luminance);   
        ImGui::Text("Adapted avarage luminance: %f", pass->readBackData.adaptedLuminance);   
        
        float luminanceMax = 0.0f;
        float luminance = log2(pass->readBackData.luminance);
        float adaptedLuminance = log2(pass->readBackData.adaptedLuminance);

        std::array<float, 256> xs;
        std::array<float, 256> counts;
        auto extent = EngineContext::Render()->GetWindowsExtent();
        uint32_t pixelCount = extent.width * extent.height;
        for(uint32_t i = 0; i < 256; i++)
        {
            xs[i] = pass->readBackData.setting.minLog2Luminance + (pass->readBackData.setting.luminanceRange / 256) * i;
            counts[i] = (float)pass->readBackData.readBackHistogramBuffer[i] / pixelCount * 100; 

            luminanceMax = std::max(luminanceMax, counts[i]);
        }
        if (ImPlot::BeginPlot("")) 
        {
            ImPlot::SetupAxes("log2 luminance","count(%)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::PlotBars("##0", xs.data(), counts.data(), 256, (pass->readBackData.setting.luminanceRange / 256));
            ImPlot::SetNextFillStyle(ImVec4(1,1,0,1));
            ImPlot::PlotBars("##1", &adaptedLuminance, &luminanceMax, 1, (pass->readBackData.setting.luminanceRange / 512));
            ImPlot::SetNextFillStyle(ImVec4(1,1,1,1));
            ImPlot::PlotBars("##2", &luminance, &luminanceMax, 1, (pass->readBackData.setting.luminanceRange / 512));  
            ImPlot::EndPlot();
        } 
    }
}

void PassWidget::PostProcessingPassUI(std::shared_ptr<PostProcessingPass> pass)
{
    if (ImGui::RadioButton("CryEngine", pass->setting.mode == 0))
		pass->setting.mode = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("Uncharted", pass->setting.mode == 1))
		pass->setting.mode = 1;
	ImGui::SameLine();
	if (ImGui::RadioButton("ACES", pass->setting.mode == 2))
		pass->setting.mode = 2;
	ImGui::SameLine();
	if (ImGui::RadioButton("None", pass->setting.mode == 3))
		pass->setting.mode = 3;

    ImGui::DragFloat("Saturation", &pass->setting.saturation, 0.1f);
	ImGui::DragFloat("Contrast", &pass->setting.contrast, 0.1f);
}