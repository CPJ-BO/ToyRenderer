#include "Transform.h"
#include "Core/Math/Math.h"

Transform::Transform(Mat4 matrix)
{
    // 从矩阵分解
    position = matrix.block<3,1>(0,3);
    scale(0) = matrix.block<1,1>(0,0)(0,0);
    scale(1) = matrix.block<1,1>(1,1)(0,0);
    scale(2) = matrix.block<1,1>(2,2)(0,0);
    rotation = Quaternion(matrix.block<3,3>(0,0));     // 三维旋转矩阵直接转四元数
    rotation.normalize();

}

Mat4 Transform::GetMatrix() const
{
    // Eigen::Affine3f model = Eigen::Affine3f::Identity();
    // model.scale(scale);
    // model.rotate(rotation);
    // model.translate(position);
    // return model.matrix();
 

    Mat4 transform = Mat4::Identity();
    transform.block<3,3>(0,0) = rotation.toRotationMatrix() * scale.asDiagonal();
    transform.block<3,1>(0,3) = position;
    return transform;
}

Mat4 Transform::GetInverseMatrix() const
{
    // Mat4 invTransform = Mat4::Identity();
    // invTransform.block<3,3>(0,0) = (rotation.toRotationMatrix() * scale.asDiagonal()).inverse();
    // invTransform.block<3,1>(0,3) = -(invTransform.block<3,3>(0,0) * position);
    // return invTransform;

    return GetMatrix().inverse();
}

void Transform::UpdateVector()
{
    front = (rotation.toRotationMatrix() * Vec3::UnitX()).normalized();
    up = (rotation.toRotationMatrix() * Vec3::UnitY()).normalized();
    right = (rotation.toRotationMatrix() * Vec3::UnitZ()).normalized();
    // std::cout << front.transpose() << " | " << up.transpose() << " | " << right.transpose() << std::endl;
}


