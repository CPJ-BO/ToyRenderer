#pragma once

#include "Core/Serialize/Serializable.h"

#include <memory>
#include <string>

class Entity;

enum ComponentType
{
	UNDEFINED_COMPONENT = 0,

	TRANSFORM_COMPONENT,
	CAMERA_COMPONENT,
	POINT_LIGHT_COMPONENT,
	DIRECTIONAL_LIGHT_COMPONENT,
	MESH_RENDERER_COMPONENT,
	SKYBOX_COMPONENT,

	COMPONENT_TYPE_MAX_ENUM, //
};

class Component
{
public:
	Component() = default;
	virtual ~Component() {};

	virtual void Load() {};						// 文件操作接口，负责序列化时加载和存储
	virtual void Save() {};
	virtual void Init() { init = true; };		// 循环接口，执行功能逻辑，Init保证在Tick之前一定会被执行一次
	virtual void Tick(float deltaTime) = 0;

	virtual std::string GetTypeName()			{ return "Undefined"; }
	virtual ComponentType GetType()				{ return UNDEFINED_COMPONENT; }
	inline std::shared_ptr<Entity> GetEntity()	{ return entity.lock(); }

	template<typename TComponent>
    std::shared_ptr<TComponent> TryGetComponent();

	template<typename TComponent>
    std::shared_ptr<TComponent> TryGetComponentInParent(bool self = false);

private:
	bool init = false;
	std::weak_ptr<Entity> entity;	 
	friend class Entity;

private:
    BeginSerailize()
    EndSerailize
};

#define EnableComponentEditourUI() \
friend class ComponentWidget;

