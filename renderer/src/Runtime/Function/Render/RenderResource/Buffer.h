#pragma once

#include "Core/Math/Math.h"
#include "Core/Util/IndexAlloctor.h"
#include "Function/Render/RHI/RHI.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "RenderStructs.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

RHIBackendRef GlobalRHIBackend();   // TODO 避免模板类的循环引用


// 对顶点数据如何进行抽象? ////////////////////////////////////////////////////////////////////////////////////

// 如何组织顶点数据? 	顶点数据的通道数不应该固定;  单buffer内逐vertex的多通道 / 多buffer各一个通道,绑定多个stream
// 顶点数据输入?  		Vertex buffer / Vertex buffer + Storage buffer(动画) / Storage buffer(间接绘制) 
// 动画到底该怎么计算?  纹理 / Storage buffer / 计算好放到Vertex buffer(光追?) 

// 一组顶点可能在多个mesh pass里被多个不同的着色器渲染,例如阴影pass(depth only pass)和G-buffer pass
// 1. 固定mesh pass的着色器,那么需要固定顶点输入,如UE的顶点工厂里提供position only和position and normal流来做depth only着色器的输入;那么如何处理顶点动画等?  	(UE使用的方法,用顶点工厂做屏蔽)
// 2. 各个mesh pass允许override着色器,只固定输出,可以提供更灵活的绘制;更复杂的着色器管理,合批等处理?
// 为了兼容不同的顶点输入，有限的几个办法：
// 1. 每种布局写一个shader（不现实）；
// 2. 单个shader，使用变体来做区别（UE抽象了permutation数组，Unity在ShaderLab里声明和处理，都需要管理多次编译）
// 3. 固定顶点输入，通过buffer传参来告知各通道是否有效，包装一层函数来做顶点信息获取（实测Vulkan验证层会报错不影响运行，可以用dynamic input state规避报错）
// 4. 放弃顶点输入，全部走Storage buffer，甚至Index buffer也可以放弃

// 一个mesh也可能经virtual mesh的处理等,间接绘制使用完全不同的一套专用的路径?
// 存各个通道流的索引,只输入index buffer,那各个mesh pass的处理应该都可以固定下来

// 顶点数据的存储 ///////////////////////////////////////////////////////////////////////////////////////////////

// https://www.reddit.com/r/vulkan/comments/194uo6a/performance_difference_between_vertex_buffer_and/
// 在bindless，indirect，raytracing，meshshader之类新技术的视角下 Vertex buffer实在是鸡肋无比
// 缺点：renderdoc等无法调试，移动端不支持，在NV等有特定的顶点处理硬件的GPU上会有少量的性能损失
// 优点：SRV,UAV资源的访问读取太重要了；使用空的顶点输入也更有利于PSO的绘制合并做batch
// ……再进一步的把管线状态信息设置等也像indirect draw一样支持到compute里，真正意义上的GPU-Driven？

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////

// UE的处理可以参考Engine\Shaders\Private\LocalVertexBuffer.ush, FLocalVertexBuffer, FCableSceneProxy等文件
// https://www.cnblogs.com/timlly/p/15092257.html
// UE使用了一个顶点工厂Vertex Factory来对顶点输入进行抽象,每一个cpp的顶点工厂对应一个ush着色器文件(include)
// 顶点工厂规定了一个顶点输入的基本规范,此外还有permutation来辅助着色器的定义(#define宏控制哪些输入流在着色器有效)
// 由primitive scene proxy负责: 创建顶点工厂对象-初始化顶点缓冲的内容-将顶点缓冲绑定给顶点工厂-每帧内,将顶点工厂和材质等信息打包成FMeshBatch(动态路径)交给FMeshElementCollector,进行后续绘制处理
// 每个shader有一个对应的cpp对象,此外permutation也抽象成了对象,保存变长的一组define设置
// 在渲染线程生成渲染指令时,会根据当前的渲染状态(例如顶点工厂的哪些输入有效)来填充permutation,从视图的ShaderMap中动态筛选着色器,
// 再生成渲染指令(UE RHI commandlist给的接口是绑定着色器,内部做pipeline的cache和动态生成,绑定)

// 最终的选择 ///////////////////////////////////////////////////////////////////////////////////////////////

// 全走bindless的storage buffer吧，放弃vertex input，反正写GPU-Driven和光追就完全用不上
// 此处VertexBuffer从功能上和UE的Vertex Factory应该大致相当？封装顶点输入流；设计的接口也支持每帧更新？
// 不过由于Vertex Factory有顶点输入，实际还和HLSL代码相关；此处的VertexBuffer用全bindless就不用管了

class VertexBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    void SetPosition(const std::vector<Vec3>& position);
    void SetNormal(const std::vector<Vec3>& normal);
    void SetTangent(const std::vector<Vec4>& tangent);
    void SetTexCoord(const std::vector<Vec2>& texCoord);
    void SetColor(const std::vector<Vec3>& color);
    void SetBoneIndex(const std::vector<IVec4>& boneIndex);
    void SetBoneWeight(const std::vector<Vec4>& boneWeight);

    RHIBufferRef positionBuffer;
    RHIBufferRef normalBuffer;
    RHIBufferRef tangentBuffer;
    RHIBufferRef texCoordBuffer;
    RHIBufferRef colorBuffer;
    RHIBufferRef boneIndexBuffer;
    RHIBufferRef boneWeightBuffer;

    uint32_t vertexID = 0;
    VertexInfo vertexInfo = {
        .positionID = 0,
        .normalID = 0,
        .texCoordID = 0,
        .colorID = 0,
        .boneIndexID = 0,
        .boneWeightID = 0
    };

    inline uint32_t VertexNum() { return vertexNum; }

private:
    void SetBufferData(void* data, uint32_t size, RHIBufferRef& buffer, uint32_t& id, uint32_t slot);

    // RHIBufferRef stagingBuffer;

    uint32_t vertexNum = 0;
};
typedef std::shared_ptr<VertexBuffer> VertexBufferRef;

class IndexBuffer
{
public:
    IndexBuffer() = default;
    ~IndexBuffer();

    void SetIndex(const std::vector<uint32_t>& index);

    RHIBufferRef buffer;

    uint32_t indexID = 0;

    inline uint32_t IndexNum() { return indexNum; }
    
private:
    uint32_t indexNum = 0;
};
typedef std::shared_ptr<IndexBuffer> IndexBufferRef;

template<typename Type>
class Buffer
{
public:
    Buffer(ResourceType type = RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_UNIFORM_BUFFER)
    {
        buffer = GlobalRHIBackend()->CreateBuffer({
            .size = sizeof(Type),
            .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
            .type = type,      
            .creationFlag = BUFFER_CREATION_PERSISTENT_MAP});
    }

    void SetData(const Type& data)
    {
        memcpy(buffer->Map(), &data, sizeof(Type));
    }

    void SetData(const void* data, uint32_t size, uint32_t offset = 0)
    {
        memcpy((uint8_t*)buffer->Map() + offset, data, size);
    }

    void GetData(Type* data)
    {
        memcpy(data, buffer->Map(), sizeof(Type));
    }

    void GetData(void* data, uint32_t size, uint32_t offset = 0)
    {
        memcpy(data, (uint8_t*)buffer->Map() + offset, size);
    }

    RHIBufferRef buffer;
};

template<typename Type, size_t arraySize>
class ArrayBuffer
{
public:
    ArrayBuffer(ResourceType type = RESOURCE_TYPE_RW_BUFFER | RESOURCE_TYPE_UNIFORM_BUFFER | RESOURCE_TYPE_INDIRECT_BUFFER)
    : idAlloctor(arraySize)
    {
        buffer = GlobalRHIBackend()->CreateBuffer({
            .size = arraySize * sizeof(Type),
            .memoryUsage = MEMORY_USAGE_CPU_TO_GPU,
            .type = type,      
            .creationFlag = BUFFER_CREATION_PERSISTENT_MAP});
    }

    void SetData(const Type& data, uint32_t index)
    {
        memcpy((Type*)buffer->Map() + index, &data, sizeof(Type));
    }

    void SetData(const std::vector<Type>& data, uint32_t index)
    {
        memcpy((Type*)buffer->Map() + index, data.data(), data.size() * sizeof(Type));
    }

    uint32_t Allocate()                             { return idAlloctor.Allocate(); }
    IndexRange Allocate(uint32_t size)              { return idAlloctor.Allocate(size); }
    void Release(uint32_t index)                    { idAlloctor.Release(index); }
    void Release(IndexRange range)                  { idAlloctor.Release(range); }

    RHIBufferRef buffer;

private:
    IndexAlloctor idAlloctor;
};

