#include "PresentPass.h"
#include "Function/Global/EngineContext.h"

void PresentPass::Init()
{

}   

void PresentPass::Build(RDGBuilder& builder) 
{
    RHITextureRef swapchainTexture = EngineContext::Render()->GetSwapchain()->GetTexture(EngineContext::CurrentFrameIndex());
    
    RDGTextureHandle outColor = builder.GetTexture("Final Color");

    RDGTextureHandle present = builder.CreateTexture("Present")
        .Import(swapchainTexture, RESOURCE_STATE_PRESENT)
        .Finish();

    RDGPresentPassHandle pass = builder.CreatePresentPass(GetName())
        .PresentTexture(present)
        .Texture(outColor)
        .Finish();
}
