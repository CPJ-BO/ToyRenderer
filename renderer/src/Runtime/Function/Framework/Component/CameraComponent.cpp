#include "CameraComponent.h"
#include "Core/Math/Math.h"
#include "Core/Math/Transform.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Global/EngineContext.h"
#include "TryGetComponent.h"
#include <memory>

CEREAL_REGISTER_TYPE(CameraComponent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, CameraComponent)

void CameraComponent::Init()
{
	UpdateMatrix();
}

void CameraComponent::Tick(float deltaTime)
{
	UpdateMatrix();
}

void CameraComponent::UpdateMatrix()
{
	std::shared_ptr<TransformComponent> transformComponent = TryGetComponent<TransformComponent>();
	if(transformComponent)
	{
		const Transform& transform = transformComponent->GetTransform();
		
		this->position = transform.GetPosition();
		this->front = transform.Front();
		this->up = transform.Up();
		this->right = transform.Right();
	}

	prevView = view;
	prevProj = proj;

	view = Math::LookAt(position, position + front, up);
	proj = Math::Perspective(Math::ToRadians(fovy), aspect, near, far);
	proj(1, 1) *= -1;		// Vulkan的NDC是y向下

	frustum = CreateFrustumFromMatrix(proj * view, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);

	cameraInfo.view = view;
	cameraInfo.proj = proj;
	cameraInfo.prevView = prevView;
	cameraInfo.prevProj = prevProj;
	cameraInfo.invView = GetInvViewMatrix();
	cameraInfo.invProj = GetInvProjectionMatrix();
	cameraInfo.pos = position;
	cameraInfo.front = front;
	cameraInfo.up = up;
	cameraInfo.right = right;
	cameraInfo.near = near;
	cameraInfo.far = far;
	cameraInfo.fov = Math::ToRadians(fovy);	//传弧度
	cameraInfo.aspect = aspect;
	cameraInfo.frustum = frustum;
}

void CameraComponent::UpdateCameraInfo()
{
	EngineContext::RenderResource()->SetCameraInfo(cameraInfo);
}
