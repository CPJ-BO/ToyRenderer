#pragma once

#include "Component.h"
#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"
#include "Core/Serialize/Serializable.h"
#include "Function/Global/Definations.h"
#include "Function/Render/RenderResource/RenderStructs.h"

#include <array>
#include <cstdint>

class DirectionalLightComponent : public Component
{
public:
	DirectionalLightComponent() = default;
	~DirectionalLightComponent() {};

	virtual void Init() override;

	virtual void Tick(float deltaTime) override;

	inline Frustum GetFrustum(int index)		{ return lightInfos[index].frustum; }

	virtual ComponentType GetType() override	{ return DIRECTIONAL_LIGHT_COMPONENT; }

private:
	Vec3 color = Vec3::Ones();
    float intencity = 0.7f;
	float cascadeSplitLambda = 0.8f;	//在对数划分和均匀划分间的加权权值
	std::array<uint32_t, DIRECTIONAL_SHADOW_CASCADE_LEVEL> updateFrequences = { 0 };	//设置每级cascade更新频率
	std::array<uint32_t, DIRECTIONAL_SHADOW_CASCADE_LEVEL> updateCnts = { 0 };
	float constantBias = 1.0f;          //bias，分为固定偏移和斜率偏移两个
	float slopeBias = 5.0f;
	float fogScattering = 0.02f;

	std::array<DirectionalLightInfo, DIRECTIONAL_SHADOW_CASCADE_LEVEL> lightInfos;          //向GPU提交的光源信息

	Vec3 front = Vec3::UnitX();
	Vec3 up = Vec3::UnitY();

	void UpdateMatrix();

	void UpdateCascades(int index);

    void UpdateLightInfo();

private:
    BeginSerailize()
    SerailizeBaseClass(Component)
    SerailizeEntry(color)
    SerailizeEntry(intencity)
    SerailizeEntry(cascadeSplitLambda)
    SerailizeEntry(updateFrequences)
    SerailizeEntry(updateCnts)
    SerailizeEntry(constantBias)
    SerailizeEntry(slopeBias)
    SerailizeEntry(fogScattering)
    EndSerailize
};