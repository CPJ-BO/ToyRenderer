#pragma once

#include "Component.h"
#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"
#include "Core/Serialize/Serializable.h"
#include "Function/Global/Definations.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include <cstdint>

class PointLightComponent : public Component
{
public:
	PointLightComponent() = default;
	~PointLightComponent();

	virtual void Init() override;
	virtual void Tick(float deltaTime) override;

    void SetScale(float scale)                      { this->far = scale; }
    void SetColor(Vec3 color)                       { this->color = color; }
    void SetIntencity(float intencity)              { this->intencity = intencity; }
    void SetCastShadow(bool castShadow)             { this->castShadow = castShadow; }
    void SetEnable(bool enable)                     { this->enable = enable; }

    virtual std::string GetTypeName() override		{ return "Point Light Component"; }
	virtual ComponentType GetType() override	    { return POINT_LIGHT_COMPONENT; }

	inline BoundingBox GetBoundingBox()	const		{ return box; }
	inline BoundingSphere GetBoundingSphere() const	{ return sphere; }
    float GetConstantBias()						    { return constantBias; }
	float GetSlopeBias()						    { return slopeBias; }

	inline bool CastShadow() const				    { return castShadow; }
	inline bool Enable() const					    { return enable; }

    inline uint32_t GetPointLightID()               { return pointLightID; }

private:
    uint32_t pointLightID = 0;
    uint32_t pointShadowID = MAX_POINT_SHADOW_COUNT;    // 由RenderLightManager负责分配点光源阴影,有效值为[0, MAX_POINT_SHADOW_COUNT - 1]

private:
    float near = 0.1f;          //光源设置
    float far = 25.0f;
    Vec3 color = Vec3::Ones();
    float intencity = 50.0f;
    float evsm[2] = {10, 15};
    float fogScattering = 0.02f;
	bool castShadow = true;	        //此光源是否投射阴影
	bool enable = true;		        //此光源是否启用
	float constantBias = 0.005f;    //固定偏移
    float slopeBias = 0.0;

	BoundingBox box;                //包围盒包围球
	BoundingSphere sphere;

    PointLightInfo info;            //向GPU提交的光源信息

    void UpdateLightInfo();

private:
    BeginSerailize()
    SerailizeBaseClass(Component)
    SerailizeEntry(near)
    SerailizeEntry(far)
    SerailizeEntry(color)
    SerailizeEntry(intencity)
    SerailizeEntry(evsm)
    SerailizeEntry(fogScattering)
    SerailizeEntry(castShadow)
    SerailizeEntry(enable)
    SerailizeEntry(constantBias)
    EndSerailize

    EnableComponentEditourUI()
    friend class RenderLightManager;
};