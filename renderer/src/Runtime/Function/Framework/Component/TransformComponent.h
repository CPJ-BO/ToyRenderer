#pragma once

#include "Component.h"
#include "Core/Math/Math.h"
#include "Core/Math/Transform.h"

class TransformComponent : public Component
{
public:
	TransformComponent() = default;
	~TransformComponent() {};
	
	virtual void Init() override;
	virtual void Tick(float deltaTime) override;

	inline void SetPosition(Vec3 position) 				{ transform.SetPosition(position); }
	inline void SetScale(Vec3 scale) 					{ transform.SetScale(scale); }
	inline void SetRotation(Quaternion rotation) 		{ transform.SetRotation(rotation); }
	inline void SetRotation(Vec3 angle) 				{ transform.SetRotation(angle); }
    Vec3 Translate(Vec3 translation)        			{ return transform.Translate(translation); }	// TODO 相对操作
    Vec3 Scale(Vec3 scale)                  			{ return transform.Scale(scale); }
    Vec3 Rotate(Vec3 angle)                 			{ return transform.Rotate(angle); }

	const Transform& GetTransform()	const				{ return transform; }

	Mat4 GetModelMat()									{ return model; }
	Mat4 GetModelMatInv()								{ return model.inverse(); }

	virtual std::string GetTypeName() override			{ return "Transform Component"; }
	virtual ComponentType GetType()	override final		{ return TRANSFORM_COMPONENT; }

private:
    Transform transform;

	Mat4 model;

	void UpdateMatrix();

private:
    BeginSerailize()
    SerailizeBaseClass(Component)
    SerailizeEntry(transform)
    EndSerailize

	EnableComponentEditourUI()
};