#include "BoundingBox.h"
#include "Core/Math/Math.h"
#include <iostream>



AxisAlignedBox::AxisAlignedBox(const Vec3& center, const Vec3& halfExtent) { Update(center, halfExtent); }

void AxisAlignedBox::Merge(const Vec3& newPoint)
{
    minCorner = newPoint.cwiseMin(minCorner);
    maxCorner = newPoint.cwiseMax(maxCorner);

    center = 0.5f * (minCorner + maxCorner);
    halfExtent = center - minCorner;
}

void AxisAlignedBox::Update(const Vec3& center, const Vec3& halfExtent)
{
    this->center = center;
    this->halfExtent = halfExtent;
    this->minCorner = center - halfExtent;
    this->maxCorner = center + halfExtent;
}

BoundingSphere::BoundingSphere(const std::vector<Vec3>& points)
{
    if (points.size() == 1)
    {
        center = points[0];
        radius = 0;
    }
    else if (points.size() == 2)
    {
        center = (points[0] + points[1]).array() / Vec3::Constant(2.0f).array();
        radius = (points[1] - center).norm();
    }
    else
    {
        uint32_t minIndex[3] = {};
        uint32_t maxIndex[3] = {};
        for (uint32_t i = 0; i < points.size(); i++)
        {
            for (int k = 0; k < 3; k++)
            {
                if (points[i](k) < points[minIndex[k]](k)) minIndex[k] = i;
                if (points[i](k) > points[maxIndex[k]](k)) maxIndex[k] = i;
            }
        }

        float max_len = 0;
        uint32_t max_axis = 0;
        for (uint32_t k = 0; k < 3; k++)
        {
            Vec3 pmin = points[minIndex[k]];
            Vec3 pmax = points[maxIndex[k]];
            float tlen = pow((pmax - pmin).norm(), 2);  
            if (tlen > max_len) max_len = tlen, max_axis = k;
        }
        Vec3 pmin = points[minIndex[max_axis]];
        Vec3 pmax = points[maxIndex[max_axis]];


        center = (pmin + pmax) * 0.5f;
        radius = float(0.5 * sqrt(max_len));
        max_len = radius * radius;

        for (uint32_t i = 0; i < points.size(); i++) 
        {
            float len = pow((points[i] - center).norm(), 2);
            if (len > max_len) 
            {
                len = sqrt(len);
                float t = 0.5 - 0.5 * (radius / len);
                center = center + (points[i] - center) * t;
                radius = (radius + len) * 0.5;
                max_len = radius * radius;
            }
        }

        for (uint32_t i = 0; i < points.size(); i++) 
        {
            float len = (points[i] - center).norm();
            assert(len - 1e-1 <= radius);   //?
        }
    }
}

BoundingSphere::BoundingSphere(const std::vector<BoundingSphere>& spheres)
{
    uint32_t minIndex[3] = {};
    uint32_t maxIndex[3] = {};
    for (uint32_t i = 0; i < spheres.size(); i++) 
    {
        for (uint32_t k = 0; k < 3; k++) 
        {
            if (spheres[i].center(k) - spheres[i].radius < spheres[minIndex[k]].center(k) - spheres[minIndex[k]].radius)
                minIndex[k] = i;
            if (spheres[i].center(k) + spheres[i].radius < spheres[maxIndex[k]].center(k) + spheres[maxIndex[k]].radius)
                maxIndex[k]  = i;
        }
    }

    float max_len = 0;
    uint32_t max_axis = 0;
    for (uint32_t k = 0; k < 3; k++) 
    {
        BoundingSphere spmin = spheres[minIndex[k]];
        BoundingSphere spmax = spheres[maxIndex[k]];
        float tlen = (spmax.center - spmin.center).norm() + spmax.radius + spmin.radius;
        if (tlen > max_len) max_len = tlen, max_axis = k;
    }

    BoundingSphere sphere = spheres[minIndex[max_axis]];
    sphere = sphere + spheres[maxIndex[max_axis]];
    for (uint32_t i = 0; i < spheres.size(); i++)
    {
        sphere = sphere + spheres[i];
    }

    for (uint32_t i = 0; i < spheres.size(); i++)
    {
        float t1 = pow(sphere.radius - spheres[i].radius, 2);
        float t2 = pow((sphere.center - spheres[i].center).norm(), 2);
        assert(t1 + 1e-1 >= t2);
    }

    this->center = sphere.center;
    this->radius = sphere.radius;
}

BoundingSphere::BoundingSphere(const BoundingBox& box)
{
    center = (box.maxBound + box.minBound).array() / Vec3::Constant(2.0f).array(); //简单的外接球
    radius = (box.maxBound - center).norm();
}

BoundingSphere::BoundingSphere(const AxisAlignedBox& box)
{
    center = box.GetCenter();                   //简单的外接球
    radius = box.GetHalfExtent().norm();
}

BoundingSphere BoundingSphere::operator+(const BoundingSphere& other) 
{
    Vec3 t = other.center - center;

    float tlen2 = pow(t.norm(), 2);
    if (pow(radius - other.radius, 2) >= tlen2) 
    {
        return radius < other.radius ? other : *this;
    }

    BoundingSphere sphere;
    float tlen = sqrt(tlen2);
    sphere.radius = (tlen + radius + other.radius) * 0.5;
    sphere.center = center + t * ((sphere.radius - radius) / tlen);

    return sphere;
}

Frustum CreateFrustumFromMatrix(Mat4 mat,
    float     x_left,
    float     x_right,
    float     y_top,
    float     y_bottom,
    float     z_near,
    float     z_far)
{
    Frustum frustum;

    // the following is in the vulkan space
    // note that the Y axis is flipped in Vulkan

    // calculate the tiled frustum
    // [Fast Extraction of Viewing Frustum Planes from the WorldView - Projection
    // Matrix](http://gamedevsphere.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf)

    // picolo 这里有bug，glm::vector.length() 返回的是向量的维数，glm::length(glm::vector)才是向量长度

    Mat4 matColumn = mat.transpose();

    frustum.planeRight = matColumn.col(0) - (matColumn.col(3) * x_right);
    frustum.planeRight *= (1.0f / Vec3(frustum.planeRight.x(), frustum.planeRight.y(), frustum.planeRight.z()).norm());

    frustum.planeLeft = (matColumn.col(3) * x_left) - matColumn.col(0);
    frustum.planeLeft *= (1.0f / Vec3(frustum.planeLeft.x(), frustum.planeLeft.y(), frustum.planeLeft.z()).norm());

    frustum.planeTop = (matColumn.col(3) * y_top) - matColumn.col(1);
    frustum.planeTop *= (1.0f / Vec3(frustum.planeTop.x(), frustum.planeTop.y(), frustum.planeTop.z()).norm());

    frustum.planeBottom = matColumn.col(1) - (matColumn.col(3) * y_bottom);
    frustum.planeBottom *= (1.0f / Vec3(frustum.planeBottom.x(), frustum.planeBottom.y(), frustum.planeBottom.z()).norm());

    frustum.planeNear = (matColumn.col(3) * z_near) - matColumn.col(2);
    frustum.planeNear *= (1.0f / Vec3(frustum.planeNear.x(), frustum.planeNear.y(), frustum.planeNear.z()).norm());

    frustum.planeFar = matColumn.col(2) - (matColumn.col(3) * z_far);
    frustum.planeFar *= (1.0f / Vec3(frustum.planeFar.x(), frustum.planeFar.y(), frustum.planeFar.z()).norm());

    return frustum;
}

bool FrustumIntersectBox(const Frustum& frustum, const BoundingBox& box)
{
    // Center of the box.
    Vec4 boxCenter((box.maxBound.x() + box.minBound.x()) * 0.5f,
        (box.maxBound.y() + box.minBound.y()) * 0.5f,
        (box.maxBound.z() + box.minBound.z()) * 0.5f,
        1.0f);

    // Distance from the center to each side.
    // half extent //more exactly
    Vec3 boxExtents((box.maxBound.x() - box.minBound.x()) * 0.5f,
        (box.maxBound.y() - box.minBound.y()) * 0.5f,
        (box.maxBound.z() - box.minBound.z()) * 0.5f);

    // planeRight
    {
        float signedDistance = frustum.planeRight.dot(boxCenter);
        float radiusProject = Vec3(fabs(frustum.planeRight.x()), fabs(frustum.planeRight.y()), fabs(frustum.planeRight.z())).dot(boxExtents);

        bool intersectingOrInside = signedDistance < radiusProject;
        if (!intersectingOrInside)
        {
            return false;
        }
    }

    // planeLeft
    {
        float signedDistance = frustum.planeLeft.dot(boxCenter);
        float radiusProject = Vec3(fabs(frustum.planeLeft.x()), fabs(frustum.planeLeft.y()), fabs(frustum.planeLeft.z())).dot(boxExtents);

        bool intersectingOrInside = signedDistance < radiusProject;
        if (!intersectingOrInside)
        {
            return false;
        }
    }

    // planeTop
    {
        float signedDistance = frustum.planeTop.dot(boxCenter);
        float radiusProject = Vec3(fabs(frustum.planeTop.x()), fabs(frustum.planeTop.y()), fabs(frustum.planeTop.z())).dot(boxExtents);

        bool intersectingOrInside = signedDistance < radiusProject;
        if (!intersectingOrInside)
        {
            return false;
        }
    }

    // planeBottom
    {
        float signedDistance = frustum.planeBottom.dot(boxCenter);
        float radiusProject = Vec3(fabs(frustum.planeBottom.x()), fabs(frustum.planeBottom.y()), fabs(frustum.planeBottom.z())).dot(boxExtents);

        bool intersectingOrInside = signedDistance < radiusProject;
        if (!intersectingOrInside)
        {
            return false;
        }
    }

    // planeNear
    {
        float signedDistance = frustum.planeNear.dot(boxCenter);
        float radiusProject = Vec3(fabs(frustum.planeNear.x()), fabs(frustum.planeNear.y()), fabs(frustum.planeNear.z())).dot(boxExtents);

        bool intersectingOrInside = signedDistance < radiusProject;
        if (!intersectingOrInside)
        {
            return false;
        }
    }

    // planeFar
    {
        float signedDistance = frustum.planeFar.dot(boxCenter);
        float radiusProject = Vec3(fabs(frustum.planeFar.x()), fabs(frustum.planeFar.y()), fabs(frustum.planeFar.z())).dot(boxExtents);

        bool intersectingOrInside = signedDistance < radiusProject;
        if (!intersectingOrInside)
        {
            return false;
        }
    }

    return true;
}

BoundingBox BoundingBoxTransform(const BoundingBox& box, const Mat4& mat)
{
    Vec3 boxOffset[8] = { Vec3(-1.0f, -1.0f, 1.0f),
                                      Vec3(1.0f, -1.0f, 1.0f),
                                      Vec3(1.0f, 1.0f, 1.0f),
                                      Vec3(-1.0f, 1.0f, 1.0f),
                                      Vec3(-1.0f, -1.0f, -1.0f),
                                      Vec3(1.0f, -1.0f, -1.0f),
                                      Vec3(1.0f, 1.0f, -1.0f),
                                      Vec3(-1.0f, 1.0f, -1.0f) };

    // Load center and extentsphere.
    // Center of the box.
    Vec3 center((box.maxBound.x() + box.minBound.x()) * 0.5f,
        (box.maxBound.y() + box.minBound.y()) * 0.5f,
        (box.maxBound.z() + box.minBound.z()) * 0.5f);

    // Distance from the center to each side.
    // half extent //more exactly
    Vec3 extents((box.maxBound.x() - box.minBound.x()) * 0.5f,
        (box.maxBound.y() - box.minBound.y()) * 0.5f,
        (box.maxBound.z() - box.minBound.z()) * 0.5f);

    Vec3 min;
    Vec3 max;

    // Compute and transform the corners and find new min/max boundsphere.
    for (size_t i = 0; i < 8; ++i)
    {
        Vec3 cornerBefore = extents.array() * boxOffset[i].array() + center.array();
        Vec4 cornerWithW = mat * Vec4(cornerBefore.x(), cornerBefore.y(), cornerBefore.z(), 1.0);
        Vec3 corner = Vec3(cornerWithW.x() / cornerWithW.w(),
            cornerWithW.y() / cornerWithW.w(),
            cornerWithW.z() / cornerWithW.w());

        if (0 == i)
        {
            min = corner;
            max = corner;
        }
        else
        {
            min = min.cwiseMin(corner);
            max = max.cwiseMax(corner);
        }
    }

    BoundingBox out;
    out.maxBound = max;
    out.minBound = min;

    return out;
}

bool BoxIntersectSphere(const BoundingBox& box, const BoundingSphere& sphere)
{
    for (int i = 0; i < 3; ++i)
    {
        if (sphere.center[i] < box.minBound[i])
        {
            if ((box.minBound[i] - sphere.center[i]) > sphere.radius)
            {
                return false;
            }
        }
        else if (sphere.center[i] > box.maxBound[i])
        {
            if ((sphere.center[i] - box.maxBound[i]) > sphere.radius)
            {
                return false;
            }
        }
    }

    return true;
}

bool BoxIntersectBox(const BoundingBox& box1, const BoundingBox& box2)
{
    if (box1.maxBound(0) < box2.minBound(0) || box1.maxBound(1) < box2.minBound(1) || box1.maxBound(2) < box2.minBound(2) ||
        box2.maxBound(0) < box1.minBound(0) || box2.maxBound(1) < box1.minBound(1) || box2.maxBound(2) < box1.minBound(2))
        return false;

    return true;
}
