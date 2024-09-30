#include "RHICommandList.h"
#include "RHI.h"
#include "RHIResource.h"

#include <cstdint>

RHICommandList::~RHICommandList() 
{ 
    info.pool->ReturnToPool(info.context); 
    info.pool = nullptr;
    info.context = nullptr;
}

void RHICommandList::BeginCommand()
{
    if(info.byPass) info.context->BeginCommand();
    else ADD_COMMAND(BeginCommand);
}

void RHICommandList::EndCommand()
{
    if(info.byPass) info.context->EndCommand();
    else ADD_COMMAND(EndCommand);
}

void RHICommandList::Execute(RHIFenceRef waitFence, RHISemaphoreRef waitSemaphore, RHISemaphoreRef signalSemaphore)
{
    if (!info.byPass) 
    {
        // LOG_DEBUG("Recording command list in delay mode.");
        for (int32_t i = 0; i < commands.size(); i++) 
        {
            commands[i]->Execute(info.context);
            delete commands[i];
        }
        commands.clear();
    }

    info.context->Execute(waitFence, waitSemaphore, signalSemaphore);
}

void RHICommandList::TextureBarrier(const RHITextureBarrier& barrier)
{
    if(info.byPass) info.context->TextureBarrier(barrier);
    else ADD_COMMAND(TextureBarrier, barrier);
}

void RHICommandList::BufferBarrier(const RHIBufferBarrier& barrier)
{
    if(info.byPass) info.context->BufferBarrier(barrier);
    else ADD_COMMAND(BufferBarrier, barrier);
}

void RHICommandList::CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset)
{
    if(info.byPass) info.context->CopyTextureToBuffer(src, srcSubresource, dst, dstOffset);
    else ADD_COMMAND(CopyTextureToBuffer, src, srcSubresource, dst, dstOffset);
}

void RHICommandList::CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    if(info.byPass) info.context->CopyBufferToTexture(src, srcOffset, dst, dstSubresource);
    else ADD_COMMAND(CopyBufferToTexture, src, srcOffset, dst, dstSubresource);
}

void RHICommandList::CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size)
{
    if(info.byPass) info.context->CopyBuffer(src, srcOffset, dst, dstOffset, size);
    else ADD_COMMAND(CopyBuffer, src, srcOffset, dst, dstOffset, size);
}

void RHICommandList::CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    if(info.byPass) info.context->CopyTexture(src, srcSubresource, dst, dstSubresource);
    else ADD_COMMAND(CopyTexture, src, srcSubresource, dst, dstSubresource);
}

void RHICommandList::GenerateMips(RHITextureRef src)
{
    if(info.byPass) info.context->GenerateMips(src);
    else ADD_COMMAND(GenerateMips, src);   
}

void RHICommandList::PushEvent(const std::string& name, Color3 color) 
{
    if(info.byPass) info.context->PushEvent(name, color);
    else ADD_COMMAND(PushEvent, name, color);
}

void RHICommandList::PopEvent() 
{
    if(info.byPass) info.context->PopEvent();
    else ADD_COMMAND(PopEvent);
}

void RHICommandList::BeginRenderPass(RHIRenderPassRef renderPass) 
{
    if(info.byPass) info.context->BeginRenderPass(renderPass);
    else ADD_COMMAND(BeginRenderPass, renderPass);
}

void RHICommandList::EndRenderPass()
{
    if(info.byPass) info.context->EndRenderPass();
    else ADD_COMMAND(EndRenderPass);
}

void RHICommandList::SetViewport(Offset2D min, Offset2D max)
{
    if(info.byPass) info.context->SetViewport(min, max);
    else ADD_COMMAND(SetViewport, min, max);
}

void RHICommandList::SetScissor(Offset2D min, Offset2D max) 
{
    if(info.byPass) info.context->SetScissor(min, max);
    else ADD_COMMAND(SetScissor, min, max);
}

void RHICommandList::SetDepthBias(float constantBias, float slopeBias, float clampBias)
{
    if(info.byPass) info.context->SetDepthBias(constantBias, slopeBias, clampBias);
    else ADD_COMMAND(SetDepthBias, constantBias, slopeBias, clampBias);
}

void RHICommandList::SetGraphicsPipeline(RHIGraphicsPipelineRef graphicsState) 
{
    if(info.byPass) info.context->SetGraphicsPipeline(graphicsState); 
    else ADD_COMMAND(SetGraphicsPipeline, graphicsState);
}

void RHICommandList::SetComputePipeline(RHIComputePipelineRef computeState) 
{
    if(info.byPass) info.context->SetComputePipeline(computeState); 
    else ADD_COMMAND(SetComputePipeline, computeState);
}	

void RHICommandList::PushConstants(void* data, uint16_t size, ShaderFrequency frequency)
{
    if(info.byPass) info.context->PushConstants(data, size, frequency);
    else ADD_COMMAND(PushConstants, data, size, frequency);
}

void RHICommandList::BindDescriptorSet(RHIDescriptorSetRef descriptor, uint32_t set)
{
    if(info.byPass) info.context->BindDescriptorSet(descriptor, set);
    else ADD_COMMAND(BindDescriptorSet, descriptor, set);
}

void RHICommandList::BindVertexBuffer(RHIBufferRef vertexBuffer, uint32_t streamIndex, uint32_t offset)
{
    if(info.byPass) info.context->BindVertexBuffer(vertexBuffer, streamIndex, offset);
    else ADD_COMMAND(BindVertexBuffer, vertexBuffer, streamIndex, offset);
}

void RHICommandList::BindIndexBuffer(RHIBufferRef indexBuffer, uint32_t offset)
{
    if(info.byPass) info.context->BindIndexBuffer(indexBuffer, offset);
    else ADD_COMMAND(BindIndexBuffer, indexBuffer, offset);
}

void RHICommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    if(info.byPass) info.context->Dispatch(groupCountX, groupCountY, groupCountZ);
    else ADD_COMMAND(Dispatch, groupCountX, groupCountY, groupCountZ);
}

void RHICommandList::DispatchIndirect(RHIBufferRef argumentBuffer, uint32_t argumentOffset) 
{
    if(info.byPass) info.context->DispatchIndirect(argumentBuffer, argumentOffset);
    else ADD_COMMAND(DispatchIndirect, argumentBuffer, argumentOffset);
}

void RHICommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) 
{
    if(info.byPass) info.context->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
    else ADD_COMMAND(Draw, vertexCount, instanceCount, firstVertex, firstInstance);
}

void RHICommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) 
{
    if(info.byPass) info.context->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    else ADD_COMMAND(DrawIndexed, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void RHICommandList::DrawIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount) 
{
    if(info.byPass) info.context->DrawIndirect(argumentBuffer, offset, drawCount);
    else ADD_COMMAND(DrawIndirect, argumentBuffer, offset, drawCount);
}

void RHICommandList::DrawIndexedIndirect(RHIBufferRef argumentBuffer, uint32_t offset, uint32_t drawCount)
{
    if(info.byPass) info.context->DrawIndexedIndirect(argumentBuffer, offset, drawCount);
    else ADD_COMMAND(DrawIndexedIndirect, argumentBuffer, offset, drawCount);
}

void RHICommandList::ImGuiCreateFontsTexture()
{
    if(info.byPass) info.context->ImGuiCreateFontsTexture();
    else ADD_COMMAND(ImGuiCreateFontsTexture);
}

void RHICommandList::ImGuiRenderDrawData()
{
    if(info.byPass) info.context->ImGuiRenderDrawData();
    else ADD_COMMAND(ImGuiRenderDrawData);
}


void RHICommandListImmediate::Flush()
{
    // LOG_DEBUG("RHICommandListImmediate Flushed.");
    for (int32_t i = 0; i < commands.size(); i++) 
    {
        commands[i]->Execute(info.context);
        delete commands[i];
    }
    commands.clear();

    info.context->Flush();
}

void RHICommandListImmediate::TextureBarrier(const RHITextureBarrier& barrier)
{
    ADD_COMMAND_IMMEDIATE(TextureBarrier, barrier);
}

void RHICommandListImmediate::BufferBarrier(const RHIBufferBarrier& barrier)
{
    ADD_COMMAND_IMMEDIATE(BufferBarrier, barrier);
}

void RHICommandListImmediate::CopyTextureToBuffer(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHIBufferRef dst, uint64_t dstOffset)
{
    ADD_COMMAND_IMMEDIATE(CopyTextureToBuffer, src, srcSubresource, dst, dstOffset);
}

void RHICommandListImmediate::CopyBufferToTexture(RHIBufferRef src, uint64_t srcOffset, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    ADD_COMMAND_IMMEDIATE(CopyBufferToTexture, src, srcOffset, dst, dstSubresource);
}

void RHICommandListImmediate::CopyBuffer(RHIBufferRef src, uint64_t srcOffset, RHIBufferRef dst, uint64_t dstOffset, uint64_t size)
{
    ADD_COMMAND_IMMEDIATE(CopyBuffer, src, srcOffset, dst, dstOffset, size);
}

void RHICommandListImmediate::CopyTexture(RHITextureRef src, TextureSubresourceLayers srcSubresource, RHITextureRef dst, TextureSubresourceLayers dstSubresource)
{
    ADD_COMMAND_IMMEDIATE(CopyTexture, src, srcSubresource, dst, dstSubresource);
}

void RHICommandListImmediate::GenerateMips(RHITextureRef src)
{
    ADD_COMMAND_IMMEDIATE(GenerateMips, src);   
}


void RHICommandBeginCommand::Execute(RHICommandContextRef context) { context->BeginCommand(); }

void RHICommandEndCommand::Execute(RHICommandContextRef context) { context->EndCommand(); }

void RHICommandTextureBarrier::Execute(RHICommandContextRef context) { context->TextureBarrier(barrier); }

void RHICommandBufferBarrier::Execute(RHICommandContextRef context) { context->BufferBarrier(barrier); }

void RHICommandCopyTextureToBuffer::Execute(RHICommandContextRef context) { context->CopyTextureToBuffer(src, srcSubresource, dst, dstOffset); }

void RHICommandCopyBufferToTexture::Execute(RHICommandContextRef context) { context->CopyBufferToTexture(src, srcOffset, dst, dstSubresource); }

void RHICommandCopyBuffer::Execute(RHICommandContextRef context) { context->CopyBuffer(src, srcOffset, dst, dstOffset, size); }

void RHICommandCopyTexture::Execute(RHICommandContextRef context) { context->CopyTexture(src, srcSubresource, dst, dstSubresource); }

void RHICommandGenerateMips::Execute(RHICommandContextRef context) { context->GenerateMips(src); }

void RHICommandPushEvent::Execute(RHICommandContextRef context) { context->PushEvent(name, color); }

void RHICommandPopEvent::Execute(RHICommandContextRef context) { context->PopEvent(); }

void RHICommandBeginRenderPass::Execute(RHICommandContextRef context) { context->BeginRenderPass(renderPass); }

void RHICommandEndRenderPass::Execute(RHICommandContextRef context) { context->EndRenderPass(); }

void RHICommandSetViewport::Execute(RHICommandContextRef context) { context->SetViewport(min, max); }

void RHICommandSetScissor::Execute(RHICommandContextRef context) { context->SetScissor(min, max); }

void RHICommandSetDepthBias::Execute(RHICommandContextRef context) { context->SetDepthBias(constantBias, slopeBias, clampBias); }

void RHICommandSetGraphicsPipeline::Execute(RHICommandContextRef context) { context->SetGraphicsPipeline(graphicsState); }

void RHICommandSetComputePipeline::Execute(RHICommandContextRef context) { context->SetComputePipeline(computeState); }

void RHICommandPushConstants::Execute(RHICommandContextRef context) { context->PushConstants(&data[0], size, frequency); }

void RHICommandBindDescriptorSet::Execute(RHICommandContextRef context) { context->BindDescriptorSet(descriptor, set); }

void RHICommandBindVertexBuffer::Execute(RHICommandContextRef context) { context->BindVertexBuffer(vertexBuffer, streamIndex, offset); }

void RHICommandBindIndexBuffer::Execute(RHICommandContextRef context) { context->BindIndexBuffer(indexBuffer, offset); }

void RHICommandDispatch::Execute(RHICommandContextRef context) { context->Dispatch(groupCountX, groupCountY, groupCountZ); }

void RHICommandDispatchIndirect::Execute(RHICommandContextRef context) { context->DispatchIndirect(argumentBuffer, argumentOffset); }

void RHICommandDraw::Execute(RHICommandContextRef context) { context->Draw(vertexCount, instanceCount, firstVertex, firstInstance); }

void RHICommandDrawIndexed::Execute(RHICommandContextRef context) { context->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); }

void RHICommandDrawIndirect::Execute(RHICommandContextRef context) { context->DrawIndirect(argumentBuffer, offset, drawCount); }

void RHICommandDrawIndexedIndirect::Execute(RHICommandContextRef context) { context->DrawIndexedIndirect(argumentBuffer, offset, drawCount); }

void RHICommandImGuiCreateFontsTexture::Execute(RHICommandContextRef context) { context->ImGuiCreateFontsTexture(); }

void RHICommandImGuiRenderDrawData::Execute(RHICommandContextRef context) { context->ImGuiRenderDrawData(); }

void RHICommandImmediateTextureBarrier::Execute(RHICommandContextImmediateRef context) { context->TextureBarrier(barrier); }

void RHICommandImmediateBufferBarrier::Execute(RHICommandContextImmediateRef context) { context->BufferBarrier(barrier); }

void RHICommandImmediateCopyTextureToBuffer::Execute(RHICommandContextImmediateRef context) { context->CopyTextureToBuffer(src, srcSubresource, dst, dstOffset); }

void RHICommandImmediateCopyBufferToTexture::Execute(RHICommandContextImmediateRef context) { context->CopyBufferToTexture(src, srcOffset, dst, dstSubresource); }

void RHICommandImmediateCopyBuffer::Execute(RHICommandContextImmediateRef context) { context->CopyBuffer(src, srcOffset, dst, dstOffset, size); }

void RHICommandImmediateCopyTexture::Execute(RHICommandContextImmediateRef context) { context->CopyTexture(src, srcSubresource, dst, dstSubresource); }

void RHICommandImmediateGenerateMips::Execute(RHICommandContextImmediateRef context) { context->GenerateMips(src); }