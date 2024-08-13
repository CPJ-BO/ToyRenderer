#pragma once

#include "Math.h"

//AABB和视锥剔除参照Picolo
class AxisAlignedBox
{
public:
    AxisAlignedBox() {};
    AxisAlignedBox(const Vec3& center, const Vec3& halfExtent);
    ~AxisAlignedBox() {};

    void Merge(const Vec3& newPoint);
    void Update(const Vec3& center, const Vec3& halfExtent);

    inline Vec3 GetCenter() const      { return center; }
    inline Vec3 GetHalfExtent() const  { return halfExtent; }
    inline Vec3 GetMinCorner() const   { return maxCorner; }
    inline Vec3 GetMaxCorner() const   { return minCorner; }

private:
    Vec3 center = Vec3::Zero();
    Vec3 halfExtent = Vec3::Zero();
    Vec3 maxCorner = Vec3::Constant(std::numeric_limits<float>::max());
    Vec3 minCorner = Vec3::Constant(std::numeric_limits<float>::min());
};

struct Frustum
{
    Vec4 planeRight;
    Vec4 planeLeft;
    Vec4 planeTop;
    Vec4 planeBottom;
    Vec4 planeNear;
    Vec4 planeFar;
};

struct BoundingBox
{
    Vec3 maxBound = Vec3::Constant(std::numeric_limits<float>::max());
    Vec3 minBound = Vec3::Constant(std::numeric_limits<float>::min());

    BoundingBox() {}

    BoundingBox(const Vec3& point)
    {
        minBound = point;
        maxBound = point;
    }

    BoundingBox(const Vec3& minv, const Vec3 maxv)
    {
        minBound = minv;
        maxBound = maxv;
    }

    BoundingBox(const AxisAlignedBox& aabb)
    {
        minBound = aabb.GetMinCorner();
        maxBound = aabb.GetMaxCorner();
    }

    void Merge(const BoundingBox& newBox)
    {   
        
        minBound = newBox.maxBound.cwiseMin(minBound).array().floor();
        maxBound = newBox.maxBound.cwiseMax(maxBound).array().ceil();
    }

    void Merge(const Vec3& point)
    {
        minBound = point.cwiseMin(minBound).array().floor();
        maxBound = point.cwiseMax(maxBound).array().ceil();
    }
};

struct BoundingSphere
{
    Vec3   center = Vec3::Zero();
    float  radius = 0.0f;

    BoundingSphere() {};

    BoundingSphere(const Vec3& center, const float& radius) 
    {
        this->center = center;
        this->radius = radius;
    };

    BoundingSphere(const std::vector<Vec3>& points);

    BoundingSphere(const std::vector<BoundingSphere>& spheres);

    BoundingSphere(const BoundingBox& box);

    BoundingSphere(const AxisAlignedBox& box);

    BoundingSphere operator+(const BoundingSphere& other);
};

Frustum CreateFrustumFromMatrix(Mat4 mat,
    float     left,
    float     right,
    float     top,
    float     bottom,
    float     near,
    float     far);

bool FrustumIntersectBox(const Frustum& frustum, const BoundingBox& box);

BoundingBox BoundingBoxTransform(const BoundingBox& box, const Mat4& mat);

bool BoxIntersectSphere(const BoundingBox& box, const BoundingSphere& sphere);

bool BoxIntersectBox(const BoundingBox& box1, const BoundingBox& box2);

