#pragma once

#include "Core/Serialize/Serializable.h"
#include "Component.h"






// 剔除系统是对各个meshpass做处理 单独处理？   如何封装材质  封装单个绘制任务
// 

// 每个FSceneRenderer创建一个FMeshElementCollector负责收集所有view的所有待绘制的meshbatch
// 每个view会有多个meshpass，
// 每个meshpass对应一个FMeshPassProcessor负责把当前pass需要绘制的meshbatch转化为meshdrawcommand
// 收集meshbatch会调用meshcomponent的动态添加函数（也有静态的，用不同的流程处理）
// 绘制命令的排序，合并等发生在创建meshdrawcommand后
// 最终会调用FMeshDrawCommand::SubmitDraw将FMeshDrawCommand转换为RHICmdList中的状态绑定和drawcall指令

// MeshRendererComponent仅提供渲染接口的声明,由子类实现
class MeshRendererComponent : public Component
{
public:
	MeshRendererComponent() = default;
	virtual ~MeshRendererComponent() {};

	virtual void Init() override;

	virtual void Tick(float deltaTime) override;

	virtual ComponentType GetType() override	    { return MESH_RENDERER_COMPONENT; }
    
private:
    BeginSerailize()
    SerailizeBaseClass(Component)
    EndSerailize
};

