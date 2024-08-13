#pragma once

#include "Component.h"
#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"
#include "Core/Serialize/Serializable.h"
#include "Function/Render/RenderResource/RenderStructs.h"

class PointLightComponent : public Component
{
public:
	PointLightComponent() = default;
	~PointLightComponent() {};

	virtual void Init() override;

	virtual void Tick(float deltaTime) override;

	virtual ComponentType GetType() override	    { return POINT_LIGHT_COMPONENT; }

	inline BoundingBox GetBoundingBox()	const		{ return box; }
	inline BoundingSphere GetBoundingSphere() const	{ return sphere; }

	inline bool CastShadow() const				    { return castShadow; }
	inline bool Enable() const					    { return enable; }

	void UpdateLightInfo();

private:
    float near = 0.1f;          //光源设置
    float far = 25.0f;
    Vec3 color = Vec3::Ones();
    float intencity = 0.7;
    float c1 = 10;
    float c2 = 15;
    float fogScattering = 0.02f;
	bool castShadow = true;	    //此光源是否投射阴影
	bool enable = true;		    //此光源是否启用
	float constantBias = 0.0f;  //固定偏移

	BoundingBox box;            //包围盒包围球
	BoundingSphere sphere;

    PointLightInfo info;        //向GPU提交的光源信息

private:
    BeginSerailize()
    SerailizeBaseClass(Component)
    SerailizeEntry(near)
    SerailizeEntry(far)
    SerailizeEntry(color)
    SerailizeEntry(intencity)
    SerailizeEntry(c1)
    SerailizeEntry(c2)
    SerailizeEntry(fogScattering)
    SerailizeEntry(castShadow)
    SerailizeEntry(enable)
    SerailizeEntry(constantBias)
    EndSerailize
};