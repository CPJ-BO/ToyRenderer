
#include "TransformComponent.h"
#include "TryGetComponent.h"

CEREAL_REGISTER_TYPE(TransformComponent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, TransformComponent)

void TransformComponent::Init()
{
	UpdateMatrix();
}

void TransformComponent::Tick(float deltaTime)
{
    UpdateMatrix();
}

void TransformComponent::UpdateMatrix()
{
    std::shared_ptr<TransformComponent> fatherTransform = TryGetComponentInParent<TransformComponent>();
    if(fatherTransform)
    {
        // TODO 
    }

    //这里的矩阵计算顺序需要和ImGuizmo库一致，不然会有跳变
    model = transform.GetMatrix();
}


