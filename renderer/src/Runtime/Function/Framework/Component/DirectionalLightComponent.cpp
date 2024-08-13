
#include "DirectionalLightComponent.h"
#include "Core/Math/Math.h"
#include "Function/Framework/Component/CameraComponent.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Framework/Scene/Scene.h"
#include "Function/Global/Definations.h"
#include "TryGetComponent.h"

#include <algorithm>
#include <array>
#include <memory>

CEREAL_REGISTER_TYPE(DirectionalLightComponent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, DirectionalLightComponent)

void DirectionalLightComponent::Init()
{
	for (int i = 0; i < 4; i++) updateCnts[i] = updateFrequences[i];	//初始时先统一绘制一轮
}

void DirectionalLightComponent::Tick(float deltaTime)
{ 
	//cascade的更新计数
	for (int i = 0; i < DIRECTIONAL_SHADOW_CASCADE_LEVEL; i++)
	{	
		updateCnts[i]++;    //检测更新频率
		if (updateCnts[i] >= updateFrequences[i]) updateCnts[i] = 0;
	}

	UpdateLightInfo();
}

void DirectionalLightComponent::UpdateMatrix()
{
    std::shared_ptr<TransformComponent> transform = TryGetComponent<TransformComponent>();
    if(transform)
    {
        front = transform->GetTransform().Front();
        up = transform->GetTransform().Up();
    }
    else 
    {
    	front = Vec3::UnitX();
	    up = Vec3::UnitY();
    }
}

void DirectionalLightComponent::UpdateCascades(int index)
{
    // 获取相机
    std::shared_ptr<CameraComponent> camera;
	if(entity.lock() && entity.lock()->GetScene().lock())
	{
		camera = entity.lock()->GetScene().lock()->GetActiveCamera();
	}
	if(!camera) return;

	

	std::array<float, DIRECTIONAL_SHADOW_CASCADE_LEVEL> cascadeSplits;

	float nearClip = camera->GetNear();
	float farClip = camera->GetFar();
	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	// 计算划分，取对数划分和平均划分加权作为结果
	for (int i = 0; i < DIRECTIONAL_SHADOW_CASCADE_LEVEL; i++) 
	{
		float p = (i + 1) / (float)(DIRECTIONAL_SHADOW_CASCADE_LEVEL);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
	}

	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for (int i = 0; i < DIRECTIONAL_SHADOW_CASCADE_LEVEL; i++)
	{
		float splitDist = cascadeSplits[i];

		Vec3 frustumCorners[8] = 
		{
			Vec3(-1.0f,  1.0f, 0.0f),
			Vec3(1.0f,  1.0f, 0.0f),
			Vec3(1.0f, -1.0f, 0.0f),
			Vec3(-1.0f, -1.0f, 0.0f),
			Vec3(-1.0f,  1.0f,  1.0f),
			Vec3(1.0f,  1.0f,  1.0f),
			Vec3(1.0f, -1.0f,  1.0f),
			Vec3(-1.0f, -1.0f,  1.0f),
		};

		// Project frustum corners into world space
		// 将相机视线台体的八个顶点转到世界空间
		Mat4 invCam = (camera->GetProjectionMatrix() * camera->GetViewMatrix()).inverse();
		for (uint32_t j = 0; j < 8; j++) 
		{
			Vec4 invCorner = invCam * Vec4(frustumCorners[j].x(), frustumCorners[j].y(), frustumCorners[j].z(), 1.0f);
			frustumCorners[j] = Vec3(invCorner.x(), invCorner.y(), invCorner.z()) / invCorner.w();
		}

		// 计算台体四条斜边的世界空间方向向量，将台体的上下表面偏移一定比例来得到划分后的台体
		for (uint32_t j = 0; j < 4; j++) 
		{
			Vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
			frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
			frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
		}

		// Get frustum center
		// 寻找划分台体的中心，创建一个包围台体的包围球，那么包围这个包围球的正方体（正交投影时）就是cascade的视线范围
		Vec3 frustumCenter = Vec3::Zero();
		for (uint32_t j = 0; j < 8; j++) 
		{
			frustumCenter += frustumCorners[j];
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t j = 0; j < 8; j++) 
		{
			float distance = (frustumCorners[j] - frustumCenter).norm();
			radius = std::max(radius, distance); 
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		Vec3 maxExtents = Vec3::Constant(radius);
		Vec3 minExtents = -maxExtents;

		Vec3 lightDir = front.normalized();
		Mat4 lightViewMatrix = Math::LookAt(frustumCenter - lightDir * radius, frustumCenter, Vec3(0.0f, 1.0f, 0.0f));
		Mat4 lightOrthoMatrix = Math::Ortho(minExtents.x(), maxExtents.x(), minExtents.y(), maxExtents.y(), 0.0f, maxExtents.z() - minExtents.z());

		// 最后存储各种信息，只有当即将被更新阴影贴图时才会写入数据
		if (updateCnts[i] >= updateFrequences[i] - 1)
		{
			lightInfos[i].depth = (camera->GetNear() + splitDist * clipRange);
			lightInfos[i].view = lightViewMatrix;
			lightInfos[i].proj = lightOrthoMatrix;
			lightInfos[i].color = color;
            lightInfos[i].intencity = intencity;
            lightInfos[i].fogScattering = fogScattering;
			lightInfos[i].dir = lightDir;

			//更新裁切视锥
			lightInfos[i].frustum = CreateFrustumFromMatrix(lightOrthoMatrix * lightViewMatrix, -1.0, 1.0, -1.0, 1.0, 0.0, 1.0);
			lightInfos[i].sphere = BoundingSphere(frustumCenter, radius);
		}
		lastSplitDist = cascadeSplits[i];
	}
}

void DirectionalLightComponent::UpdateLightInfo()
{
	UpdateMatrix();

	UpdateCascades(0);  // TODO
}