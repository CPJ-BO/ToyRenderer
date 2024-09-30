#pragma once

#include "Core/Util/IndexAlloctor.h"

#include "Function/Global/Definations.h"
#include "RenderPass.h"
#include "Function/Render/RenderResource/PipelineCache.h"
#include <array>

// mesh pass提供了对于各个需要光栅化绘制mesh的pass的抽象
// 围绕mesh pass需要提供场景图元收集，管线状态管理，合批，剔除，以及各个mesh pass的绘制代码生成的全部功能

// UE的图元收集过程 ///////////////////////////////////////////////////////////////////////////////////
// 每个FSceneRenderer创建一个FMeshElementCollector负责收集所有view的所有待绘制的meshbatch
// 每个view会有多个meshpass，
// 每个meshpass对应一个FMeshPassProcessor负责把当前pass需要绘制的meshbatch转化为meshdrawcommand
// 收集meshbatch会调用meshcomponent的动态添加函数（也有静态的，用不同的流程处理）
// 绘制命令的排序，合并等发生在创建meshdrawcommand后
// 最终会调用FMeshDrawCommand::SubmitDraw将FMeshDrawCommand转换为RHICmdList中的状态绑定和drawcall指令

// 最终设计方案 ///////////////////////////////////////////////////////////////////////////////////////
// 1. GPU-Driven
// 剔除完全GPU-Driven，提供逐物体/逐cluster/虚拟几何体的剔除功能
// 对于逐物体剔除，每个物体的每次绘制占用一个独立的RHIIndirectCommand，剔除只需要修改instanceCount为0或1 
// 对于虚拟几何体剔除，在cluster group处理完毕后回到逐cluster剔除
// 对于逐cluster剔除，属于同一管线状态的多个cluster共享一个RHIIndirectCommand，经过剔除后待绘制的cluster索引信息最终收集到全局唯一的一块缓存中
// 因此需要事先做好偏移计算，以最大可能绘制的数目做起始（TODO 浪费了很多空间）

// 2. Mesh pass
// 暂时没有单独抽象MeshElementCollector，直接在render system里完成对全部绘制的收集
// 目前仅有MeshRendererComponent一种组件提供了提交DrawBatch信息的CollectDrawBatch()函数，后续可以此为接口做扩展
// 单次绘制以DrawBatch结构体为基础，经过各个mesh pass提供的MeshPassProcessor子类完成指令解析和合批过程，并完成用于GPU端剔除和指令生成的全部数据收集
// 暂时没有CPU端剔除，后续可以做八叉树场景管理等提前削掉一部分
// MeshPassProcessor子类可重构部分处理流程函数来自定义处理，例如DrawBatch的收集条件，管线创建等过程
// 其余的功能已经尽可能封装，在扩展Mesh pass时只需要定义好processor的处理过程然后在渲染pass里靠processor录制绘制指令即可
// 剔除需要配合GPUCullingPass，该pass会收集全部剔除处理的数据并进行计算

// 支持的全部渲染类型 ///////////////////////////////////////////////////////////////////////////////////////
// 1. 无cluster，		支持动画,		实例化，				支持前向&延迟，支持动态管线状态
// 2. cluster，			仅静态网格，	merge-instance，		支持前向&延迟，支持动态管线状态
// 3. virtual mesh，	仅静态网格，	merge-instance，		支持前向&延迟，支持动态管线状态

// GPU端的数据索引顺序 ///////////////////////////////////////////////////////////////////////////////////////
// 
//					┌----------> RHIIndirectCommand
//	 IndirectMeshDrawInfo ------> [gl_instaceIndex]-----> ObjectInfo ----┬--> AnimatedTransformInfo  
//															  ∧			├--> MaterialInfo
//															  │			 ├--> VertexStream ---> pos/normal/color/texCoord/tangent ...
//			┌-------------------------------------------------┘	 		 ╘--> IndexBuffer
//		    │	       
//	MeshClusterDrawInfo -------> ClusterInfo----┬--> VertexStream
//		    ∧						           ├--> IndexBuffer
//    [gl_instaceIndex]                         ╘--> IndexOffset
//    (计算剔除后生成) 
//          │             ┌--------> RHIIndirectCommand
//   IndirectClusterDrawDatas -----> ClusterInfo
//                        ╘--------> ObjectInfo	
//
//  对于物体实例的索引依赖gl_instaceIndex
//  对于index下标的索引依赖gl_VertexIndex以及计算偏移值


// 每帧重新生成和收集，包含一次绘制所需要的全部上层信息
typedef struct DrawBatch
{
    uint32_t objectID;                                          // 物体唯一索引

    VertexBufferRef vertexBuffer;                               // 若不启用cluster和virtual mesh渲染，则为正常的顶点和索引缓冲
    IndexBufferRef indexBuffer;                                 // 否则为合并后的cluster组

    IndexRange clusterID = { 0, 0 };               // 若提交时begin不为0，则启用cluster渲染
    IndexRange clusterGroupID = { 0, 0 };          // 若提交时begin不为0，则启用virtual mesh渲染

    MaterialRef material;                                       // 包含了材质数据的内存块，也包含了着色器信息

} DrawBatch;

typedef struct DrawPipelineState
{
    uint32_t renderQueue;                               

    RHIShaderRef vertexShader;
    RHIShaderRef geometryShader;
    RHIShaderRef fragmentShader;

    RasterizerCullMode cullMode;           
    RasterizerFillMode fillMode;  
    CompareFunction depthCompare; 
    bool depthTest;                                    
    bool depthWrite;                     
    
    bool meshRender;
    bool clusterRender;


    friend bool operator== (const DrawPipelineState& a, const DrawPipelineState& b)
	{
		return  a.renderQueue == b.renderQueue &&
				a.cullMode == b.cullMode &&
				a.fillMode == b.fillMode &&
				a.depthTest == b.depthTest &&
				a.depthWrite == b.depthWrite &&
				a.depthCompare == b.depthCompare &&
                a.meshRender == b.meshRender && 
                a.clusterRender == b.clusterRender &&
                a.vertexShader.get() == b.vertexShader.get() &&
				a.geometryShader.get() == b.geometryShader.get() &&
				a.fragmentShader.get() == b.fragmentShader.get();
	}

    bool operator< (const DrawPipelineState& other)const
    {
        return  (renderQueue != other.renderQueue) ?                    (renderQueue < other.renderQueue) : 
                (cullMode != other.cullMode) ?                          (cullMode < other.cullMode) : 
                (fillMode != other.fillMode) ?                          (fillMode < other.fillMode) : 
                (depthTest != other.depthTest) ?                        (depthTest < other.depthTest) : 
                (depthWrite != other.depthWrite) ?                      (depthWrite < other.depthWrite) : 
                (depthCompare != other.depthCompare) ?                  (depthCompare < other.depthCompare) : 
                (meshRender != other.meshRender) ?                      (meshRender < other.meshRender) :
                (clusterRender != other.clusterRender) ?                (clusterRender < other.clusterRender) :
                (vertexShader.get() != other.vertexShader.get()) ?      (vertexShader.get() < other.vertexShader.get()) : 
                (geometryShader.get() != other.geometryShader.get()) ?  (geometryShader.get() < other.geometryShader.get()) : 
                (fragmentShader.get() != other.fragmentShader.get()) ?  (fragmentShader.get() < other.fragmentShader.get()) : false;
    }

} DrawPipelineState;

typedef struct DrawGeometryInfo
{
    uint32_t objectID;
    uint32_t vertexID;
    uint32_t indexID;
    uint32_t indexCount;
    IndexRange clusterID = { 0, 0 };            
    IndexRange clusterGroupID = { 0, 0 };

} DrawGeometryInfo;

typedef struct DrawCommand
{
    RHIGraphicsPipelineRef pipeline;    

    IndexRange meshCommandRange = { 0, 0 };
    uint32_t meshCommandOffset = 0;
    RHIBufferRef indirectMeshCommandBuffer;
    
    IndexRange clusterCommandRange = { 0, 0 };
    uint32_t clusterCommandOffset = 0;
    RHIBufferRef indirectClusterCommandBuffer;
    
} DrawCommand;

typedef struct MeshPassIndirectBuffers
{
    Buffer<IndirectMeshDrawDatas> meshDrawDataBuffer;           
    Buffer<IndirectMeshDrawCommands> meshDrawCommandBuffer 
        = Buffer<IndirectMeshDrawCommands>(RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_INDIRECT_BUFFER);

    Buffer<IndirectClusterDrawDatas> clusterDrawDataBuffer;
    Buffer<IndirectClusterDrawCommands> clusterDrawCommandBuffer 
        = Buffer<IndirectClusterDrawCommands>(RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_INDIRECT_BUFFER);;

    Buffer<IndirectClusterGroupDrawDatas> clusterGroupDrawDataBuffer;

} MeshPassIndirectBuffers;

// 各个meshpass对应子类，收集各meshpass需要的绘制信息，生成绘制指令 
class MeshPassProcessor
{
public:
    void Init();
    void Process(const std::vector<DrawBatch>& batches);
    void Draw(RHICommandListRef command);

    std::shared_ptr<MeshPassIndirectBuffers> GetIndirectBuffers();

    static const uint32_t& GetGlobalClusterOffset()                                                 { return globalClusterOffset; }
    static const uint32_t& AddGlobalClusterOffset(uint32_t size)                                    { globalClusterOffset += size; return globalClusterOffset; }
    static void ResetGlobalClusterOffset()                                                          { globalClusterOffset = 0; }

protected:    
    virtual void OnCollectBatch(const DrawBatch& batch);                                        // 由子类重载，负责条件判断和实际添加batch进processor，
    virtual void OnBuildDrawInfo(const DrawBatch& batch);                                       // 由子类重载，负责生成绘制信息（包括管线状态信息和几何信息）进processor，
    virtual RHIGraphicsPipelineRef OnCreatePipeline(const DrawPipelineState& pipelineState);    // 由子类重载，负责根据管线状态创建管线
    virtual void OnBuildDrawCommands(                                                           // 由子类重载，负责填充间接绘制相关的缓冲和生成绘制指令
                    uint32_t pipelineIndex,     
                    RHIGraphicsPipelineRef pipeline, 
                    const std::vector<DrawGeometryInfo>& geometries);
    
    void AddBatch(const DrawBatch& batch)                                                           { batches.push_back(batch); }
    void AddDrawInfo(const DrawPipelineState& pipelineState, const DrawGeometryInfo& geometryInfo)  { drawGeometries[pipelineState].push_back(geometryInfo); }
    void AddDrawCommand(const DrawCommand& drawCommand)                                             { drawCommands.push_back(drawCommand); }

    // 需要提交给GPU缓冲的信息 ///////////////////////////////////////////////////
    std::vector<RHIIndirectCommand> meshDrawCommand;                            
    std::vector<IndirectMeshDrawInfo> meshDrawInfo;
    uint32_t clusterCount[MAX_PER_PASS_PIPELINE_STATE_COUNT];   // 属于各个不同管线状态的cluster信息的数目，也就是该pass的该管线状态下最大可能绘制的cluster数目
    std::vector<RHIIndirectCommand> clusterDrawCommand;
    std::vector<IndirectClusterDrawInfo> clusterDrawInfo;
    std::vector<IndirectClusterGroupDrawInfo> clusterGroupDrawInfo;

    // GPU缓冲 ///////////////////////////////////////////////////
    std::array<std::shared_ptr<MeshPassIndirectBuffers>, FRAMES_IN_FLIGHT> indirectBuffers;     // 每帧都完全重构的buffer，因此需要每帧一份   
    
private:
    // CPU端的数据处理结果 ///////////////////////////////////////////////////
    std::vector<DrawBatch> batches;
    std::map<DrawPipelineState, std::vector<DrawGeometryInfo>> drawGeometries;
    std::vector<DrawCommand> drawCommands;

    // cluster数据的全局偏移，最后所有的pass都需要收集cluster绘制信息到一个全局buffer，需要记录偏移
    static uint32_t globalClusterOffset;
};
typedef std::shared_ptr<MeshPassProcessor> MeshPassProcessorRef;

class MeshPass : public RenderPass
{
public:
    virtual void Init() override                                        { meshPassProcessor->Init(); }
    virtual std::vector<MeshPassProcessorRef> GetMeshPassProcessors()   { return { meshPassProcessor }; }

protected:
    MeshPassProcessorRef meshPassProcessor = std::make_shared<MeshPassProcessor>();
};

