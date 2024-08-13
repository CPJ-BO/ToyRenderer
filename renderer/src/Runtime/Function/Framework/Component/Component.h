#pragma once

#include "Core/Serialize/Serializable.h"

#include <memory>

class Entity;

enum ComponentType
{
	UNDEFINED_COMPONENT = 0,

	ANIMATOR_COMPONENT,
	CAMERA_COMPONENT,
	DIRECTIONAL_LIGHT_COMPONENT,
	MESH_COMPONENT,
	MESH_RENDERER_COMPONENT,
	POINT_LIGHT_COMPONENT,
	SKYBOX_COMPONENT,
	TRANSFORM_COMPONENT,
	VOLUME_LIGHT_COMPONENT,

	COMPONENT_TYPE_MAX_ENUM, //
};

class Component
{
public:
	Component() = default;
	virtual ~Component() {};

	virtual void Init() = 0;
	virtual void Tick(float deltaTime) = 0;
	virtual void Save() {};

	virtual ComponentType GetType()				{ return UNDEFINED_COMPONENT; }
	inline std::weak_ptr<Entity> GetEntity()	{ return entity; }

	template<typename TComponent>
    std::shared_ptr<TComponent> TryGetComponent();

	template<typename TComponent>
    std::shared_ptr<TComponent> TryGetComponentInParent(bool self = false);

protected:
	std::weak_ptr<Entity> entity;	 
	friend class Entity;

private:
    BeginSerailize()
    EndSerailize
};


