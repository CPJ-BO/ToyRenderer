#include "EditorUIPass.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHICommandList.h"

#include "Function/Render/RHI/RHIStructs.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <string>

void EditorUIPass::Init()
{
    InitImGuiStyle();
}   

void EditorUIPass::Build(RDGBuilder& builder) 
{
    RDGTextureHandle outColor = builder.GetTexture("Final Color");
    RDGTextureHandle depth = builder.GetTexture("Depth");

    RDGRenderPassHandle pass = builder.CreateRenderPass(GetName())
        .Color(0, outColor, ATTACHMENT_LOAD_OP_LOAD, ATTACHMENT_STORE_OP_STORE, {0.0f, 0.0f, 0.0f, 0.0f})
        .DepthStencil(depth, ATTACHMENT_LOAD_OP_LOAD, ATTACHMENT_STORE_OP_STORE, 1.0f, 0)   // 为什么必须加深度才有效？
        .Execute([&](RDGPassContext context) {
            
            Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();

            RHICommandListRef command = context.command;                                            
            command->SetViewport({0, 0}, {windowExtent.width, windowExtent.height});
            command->SetScissor({0, 0}, {windowExtent.width, windowExtent.height}); 

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            // IMGUIZMO_NAMESPACE::BeginFrame();

			EngineContext::Editor()->UI();

            ImGui::Render();
            command->ImGuiRenderDrawData();
        })
        .Finish();
}

void EditorUIPass::InitImGuiStyle()
{   
    // 字体加载
    ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF(EngineContext::File()->Absolute(EngineContext::File()->FontPath() + "fa-regular-400.ttf").c_str(), 20);	//这两好像有问题 用不了
	//io.Fonts->AddFontFromFileTTF(EngineContext::File()->Absolute(EngineContext::File()->FontPath() + "fa-solid-900.ttf").c_str(), 20);
	io.Fonts->AddFontFromFileTTF(EngineContext::File()->Absolute(EngineContext::File()->FontPath() + "notosans-bold.ttf").c_str(), 20);
	io.Fonts->AddFontFromFileTTF(EngineContext::File()->Absolute(EngineContext::File()->FontPath() + "notosans-regular.ttf").c_str(), 20);
	io.Fonts->AddFontFromFileTTF(EngineContext::File()->Absolute(EngineContext::File()->FontPath() + "sourcecodepro-regular.ttf").c_str(), 20);
	{
        RHIQueueRef queue           = EngineContext::RHI()->GetQueue({.type = QUEUE_TYPE_GRAPHICS, .index = 0});
        RHICommandPoolRef pool      = EngineContext::RHI()->CreateCommandPool({ queue });  
        RHICommandListRef command   = pool->CreateCommandList(true);
        command->BeginCommand();
        command->ImGuiCreateFontsTexture();
        command->EndCommand();
        command->Execute();
        queue->WaitIdle();

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

    // 风格设置
    ImGuiStyle& style = ImGui::GetStyle();
    style.TabRounding = 0;
    style.IndentSpacing = 7;
    style.Colors[ImGuiCol_WindowBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
    style.Colors[ImGuiCol_TitleBgActive]  = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
}