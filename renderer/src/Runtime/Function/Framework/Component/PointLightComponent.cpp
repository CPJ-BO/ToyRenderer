
#include "PointLightComponent.h"
#include "Core/Math/Math.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Global/EngineContext.h"
#include "TryGetComponent.h"

CEREAL_REGISTER_TYPE(PointLightComponent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, PointLightComponent)

PointLightComponent::~PointLightComponent()
{
    if(!EngineContext::Destroyed() && pointLightID != 0) EngineContext::RenderResource()->ReleasePointLightID(pointLightID);
}

void PointLightComponent::Init()
{
    Component::Init();

    pointLightID = EngineContext::RenderResource()->AllocatePointLightID();
}

void PointLightComponent::Tick(float deltaTime)
{
	// UpdateLightInfo();
}

void PointLightComponent::UpdateLightInfo()
{
    Vec3 pos = Vec3::Zero();

    std::shared_ptr<TransformComponent> transform = TryGetComponent<TransformComponent>();
    if(transform)
    {
        //更新包围信息
        pos = transform->GetTransform().GetPosition();

        box = BoundingBox(pos - Vec3::Constant(far), pos + Vec3::Constant(far));
        sphere = BoundingSphere(pos, far);
    }

	//无阴影点光源信息
	{
		info.pos = pos;
		info.color = color;
        info.intencity = intencity;
        info.fogScattering = fogScattering;
        info.near = near;
        info.far = far;
        info.bias = constantBias;
        info.enable = enable;
        info.sphere = sphere;
        info.shadowID = pointShadowID;
	}

	//带阴影点光源需要额外更新的信息
	if (pointShadowID < MAX_POINT_SHADOW_COUNT)
	{
		info.c1 = evsm[0];
		info.c2 = evsm[1];

        //learn opengl中使用的矩阵
        info.view[0] = Math::LookAt(info.pos, info.pos + Vec3::UnitX(), -Vec3::UnitY());
        info.view[1] = Math::LookAt(info.pos, info.pos - Vec3::UnitX(), -Vec3::UnitY());
        info.view[2] = Math::LookAt(info.pos, info.pos + Vec3::UnitY(), Vec3::UnitZ());
        info.view[3] = Math::LookAt(info.pos, info.pos - Vec3::UnitY(), -Vec3::UnitZ());
        info.view[4] = Math::LookAt(info.pos, info.pos + Vec3::UnitZ(), -Vec3::UnitY());
        info.view[5] = Math::LookAt(info.pos, info.pos - Vec3::UnitZ(), -Vec3::UnitY());

        info.proj = Math::Perspective(Math::ToRadians(90.0f), 1.0f, near, far);
		//info.proj[1][1] *= -1;	//Vulkan的NDC的Y坐标是向下的，通常要转一次，这下不转了？？
	}

    EngineContext::RenderResource()->SetPointLightInfo(info, pointLightID);
}


