#ifndef INTERSECTION_GLSL
#define INTERSECTION_GLSL


bool LineIntersectPlane(vec3 begin, vec3 end, vec4 plane, out vec3 point)
{
	vec3 line = end - begin;

	float t = (plane.w - dot(plane.xyz, begin)) / dot(plane.xyz, line);

	bool intersect = (t >= 0.0f && t <= 1.0f);	//限定相交范围在起点终点之间
	//bool intersect = true;

	point = vec3(0, 0, 0);

	if (intersect)	point = begin + t * line;

	return intersect;
}


// UE Private/Common.ush
/*
* Clips a ray to an AABB.  Does not handle rays parallel to any of the planes.
*
* @param RayOrigin - The origin of the ray in world space.
* @param RayEnd - The end of the ray in world space.  
* @param BoxMin - The minimum extrema of the box.
* @param BoxMax - The maximum extrema of the box.
* @return - Returns the closest intersection along the ray in x, and furthest in y.  
*			If the ray did not intersect the box, then the furthest intersection <= the closest intersection.
*			The intersections will always be in the range [0,1], which corresponds to [RayOrigin, RayEnd] in worldspace.
*			To find the world space position of either intersection, simply plug it back into the ray equation:
*			WorldPos = RayOrigin + (RayEnd - RayOrigin) * Intersection;
*/
vec2 LineIntersectBox(vec3 RayOrigin, vec3 RayEnd, vec3 BoxMin, vec3 BoxMax)
{
	vec3 InvRayDir = 1.0f / (RayEnd - RayOrigin);
	
	//find the ray intersection with each of the 3 planes defined by the minimum extrema.
	vec3 FirstPlaneIntersections = (BoxMin - RayOrigin) * InvRayDir;
	//find the ray intersection with each of the 3 planes defined by the maximum extrema.
	vec3 SecondPlaneIntersections = (BoxMax - RayOrigin) * InvRayDir;
	//get the closest of these intersections along the ray
	vec3 ClosestPlaneIntersections = min(FirstPlaneIntersections, SecondPlaneIntersections);
	//get the furthest of these intersections along the ray
	vec3 FurthestPlaneIntersections = max(FirstPlaneIntersections, SecondPlaneIntersections);

	vec2 BoxIntersections;
	//find the furthest near intersection
	BoxIntersections.x = max(ClosestPlaneIntersections.x, max(ClosestPlaneIntersections.y, ClosestPlaneIntersections.z));
	//find the closest far intersection
	BoxIntersections.y = min(FurthestPlaneIntersections.x, min(FurthestPlaneIntersections.y, FurthestPlaneIntersections.z));
	//clamp the intersections to be between RayOrigin and RayEnd on the ray
	return clamp(BoxIntersections, 0.0f, 1.0f);
}


struct Frustum
{
    vec4 planes[6];       //左右上下近远，前三维为归一化平面法线，第四维为面到原点距离
};

struct BoundingBox
{
    vec3 max_bound;
    vec3 min_bound;
};

struct BoundingSphere
{
    vec4 center_radius;
};

BoundingBox BoundingBoxTransform(BoundingBox box, mat4 transform)
{
	vec3 offsets[8] = {	vec3(-1.0f, -1.0f, 1.0f),
						vec3(1.0f, -1.0f, 1.0f),
						vec3(-1.0f, 1.0f, 1.0f),
						vec3(1.0f, 1.0f, 1.0f),
						vec3(-1.0f, -1.0f, -1.0f),
						vec3(1.0f, -1.0f, -1.0f),
						vec3(-1.0f, 1.0f, -1.0f),
						vec3(1.0f, 1.0f, -1.0f)};

	vec3 center = (box.max_bound + box.min_bound) * 0.5f;
	vec3 extent = (box.max_bound - box.min_bound) * 0.5f;

	vec3 new_max_bound = vec3(-1e30);
	vec3 new_min_bound = vec3(1e30);
	for(int i = 0; i < 8; i++)
	{
		vec4 corner_before = vec4(extent * offsets[i] + center, 1.0f);
		vec4 corner = transform * corner_before;
		corner /= corner.w;

		new_max_bound = vec3(max(new_max_bound.x, corner.x), max(new_max_bound.y, corner.y), max(new_max_bound.z, corner.z));
		new_min_bound = vec3(min(new_min_bound.x, corner.x), min(new_min_bound.y, corner.y), min(new_min_bound.z, corner.z));
	}

	BoundingBox new_box;
	new_box.max_bound = new_max_bound;
	new_box.min_bound = new_min_bound;

	return new_box;
}

BoundingSphere SphereTransform(BoundingSphere sphere, mat4 transform)
{
    BoundingSphere new_sphere;

    new_sphere.center_radius.xyz = (transform * vec4(sphere.center_radius.xyz, 1.0f)).xyz;	
    new_sphere.center_radius.w = sphere.center_radius.w * length( transform[0] );		

    return new_sphere;
}

bool PointIntersectBox(vec3 point, BoundingBox box)
{
	return max(min(point, box.max_bound), box.min_bound) == point;
}

bool FrustumIntersectBox(Frustum frustum, BoundingBox box)
{
    bool intersect = true;

    vec3 center = (box.max_bound + box.min_bound) * 0.5f;
	vec3 extent = (box.max_bound - box.min_bound) * 0.5f;

    for (int i = 0; i < 6; i++) 
    {
        vec3 abs_plane = abs(frustum.planes[i].xyz);
        float radius_project_plane = dot(abs_plane, extent);	//包围盒的投影半径，绝对值

        float signed_distance_from_plane = dot(-frustum.planes[i], vec4(center, 1.0f)); 

        intersect = intersect && (signed_distance_from_plane > -radius_project_plane);
    }

	return intersect;
}

bool FrustumIntersectSphere(Frustum frustum, BoundingSphere sphere)
{
    bool intersect = true;

    for (int i = 0; i < 6; i++) 
    {
        float signed_distance_from_plane = dot(-frustum.planes[i], vec4(sphere.center_radius.xyz, 1.0f));		
        
        intersect = intersect && (signed_distance_from_plane > -sphere.center_radius.w);
    }

	return intersect;
}

bool BoxIntersectSphere(BoundingBox box, BoundingSphere sphere)
{
    bool intersect = true;

    for(int i = 0; i < 3; i++)  //将box边界各扩展sphere半径长度即为可能的相交区域，对角处可能有一些误判
    {
        intersect = intersect && !(box.min_bound[i] > sphere.center_radius[i]  && box.min_bound[i] - sphere.center_radius[i] > sphere.center_radius[3]);
        intersect = intersect && !(box.max_bound[i] < sphere.center_radius[i]  && box.max_bound[i] - sphere.center_radius[i] < -sphere.center_radius[3]);
    }

    return intersect;
}

bool BoxIntersectBox(BoundingBox box0, BoundingBox box1)
{
    bool intersect = true;

    intersect = intersect && !(box0.min_bound[0] > box1.max_bound[0] || box0.min_bound[1] > box1.max_bound[1] || box0.min_bound[2] > box1.max_bound[2]); 
    intersect = intersect && !(box1.min_bound[0] > box0.max_bound[0] || box1.min_bound[1] > box0.max_bound[1] || box1.min_bound[2] > box0.max_bound[2]); 

    return intersect;
}

bool SphereIntersectSphere(BoundingSphere sphere0, BoundingSphere sphere1)
{
    bool intersect = length(sphere0.center_radius.xyz - sphere1.center_radius.xyz) <= sphere0.center_radius.w + sphere1.center_radius.w;
    
    return intersect;
}





//要求：view空间： x朝右，y朝上，z朝外（需要改为-z）；投影矩阵也反了，要改为-proj[1][1]
//实际：view空间： x朝右，y朝上，z朝内

//https://github.com/CU-Production/VulkanRenderer?tab=readme-ov-file
//https://gist.github.com/zeux/19b1ba2ce3121e9933da18802fed09d6
//https://zeux.io/2023/01/12/approximate-projected-bounds/

//下两个算法也是这样的要求，已经在函数里改了
// 2D Polyhedral Bounds of a Clipped, Perspective-Projected 3D Sphere. Michael Mara, Morgan McGuire. 2013
bool ProjectSphereView(vec3 c, float r, float znear, float P00, float P11, out vec4 aabb)
{
    P11 = -P11; //注意，这里改了符号

    if (c.z < r + znear) return false;

    vec3 cr = c * r;
    float czr2 = c.z * c.z - r * r;

    float vx = sqrt(c.x * c.x + czr2);
    float minx = (vx * c.x - cr.z) / (vx * c.z + cr.x);
    float maxx = (vx * c.x + cr.z) / (vx * c.z - cr.x);

    float vy = sqrt(c.y * c.y + czr2);
    float miny = (vy * c.y - cr.z) / (vy * c.z + cr.y);
    float maxy = (vy * c.y + cr.z) / (vy * c.z - cr.y);

    aabb = vec4(minx * P00, miny * P11, maxx * P00, maxy * P11);
    // clip space -> uv space
    aabb = aabb.xwzy * vec4(0.5f, -0.5f, 0.5f, -0.5f) + vec4(0.5f);

    return true;
}

bool ProjectSphere(vec3 c, float r, mat4 view, float znear, float P00, float P11, out vec4 aabb)
{
    vec4 cv = (view * vec4(c, 1.0f));       //注意，这里改了view符号
	cv.z *= -1;

    return ProjectSphereView(cv.xyz, r, znear, P00, P11, aabb);
}



//以下函数还没测试，坐标系还是错的，估计也得按上面两个函数这样改//////////////////////////////////////////////////////////////////////////////////////////////////

bool ProjectBox(vec3 bmin, vec3 bmax, float znear, mat4 viewProjection, out vec4 aabb)
{
    vec4 SX = vec4(bmax.x - bmin.x, 0.0, 0.0, 0.0) * viewProjection;
    vec4 SY = vec4(0.0, bmax.y - bmin.y, 0.0, 0.0) * viewProjection;
    vec4 SZ = vec4(0.0, 0.0, bmax.z - bmin.z, 0.0) * viewProjection;

    vec4 P0 = vec4(bmin.x, bmin.y, bmin.z, 1.0) * viewProjection;
    vec4 P1 = P0 + SZ;
    vec4 P2 = P0 + SY;
    vec4 P3 = P2 + SZ;
    vec4 P4 = P0 + SX;
    vec4 P5 = P4 + SZ;
    vec4 P6 = P4 + SY;
    vec4 P7 = P6 + SZ;

    if (min(min(min(P0.w, P1.w), min(P2.w, P3.w)), min(min(P4.w, P5.w), min(P6.w, P7.w))) < znear) return false;

    aabb.xy = min(
        min(min(P0.xy / P0.w, P1.xy / P1.w), min(P2.xy / P2.w, P3.xy / P3.w)),
        min(min(P4.xy / P4.w, P5.xy / P5.w), min(P6.xy / P6.w, P7.xy / P7.w)));
    aabb.zw = max(
        max(max(P0.xy / P0.w, P1.xy / P1.w), max(P2.xy / P2.w, P3.xy / P3.w)),
        max(max(P4.xy / P4.w, P5.xy / P5.w), max(P6.xy / P6.w, P7.xy / P7.w)));

    // clip space -> uv space
    aabb = aabb.xwzy * vec4(0.5f, -0.5f, 0.5f, -0.5f) + vec4(0.5f);

    return true;
}

bool ProjectBoxView(vec3 c, vec3 r, float znear, float P00, float P11, out vec4 aabb)
{
    if (c.z - r.z < znear) return false;

    // when we're computing the extremum of projection along an axis, the maximum
    // is reached by front face for positive and by back face for negative values
    float rminz = 1 / (c.z - r.z);
    float rmaxz = 1 / (c.z + r.z);
    float minx = (c.x - r.x) * (c.x - r.x >= 0 ? rmaxz : rminz);
    float maxx = (c.x + r.x) * (c.x + r.x >= 0 ? rminz : rmaxz);
    float miny = (c.y - r.y) * (c.y - r.y >= 0 ? rmaxz : rminz);
    float maxy = (c.y + r.y) * (c.y + r.y >= 0 ? rminz : rmaxz);

    aabb = vec4(minx * P00, miny * P11, maxx * P00, maxy * P11);
    // clip space -> uv space
    aabb = aabb.xwzy * vec4(0.5f, -0.5f, 0.5f, -0.5f) + vec4(0.5f);

    return true;
}

bool ProjectBoxApprox(vec3 min, vec3 max, mat4 view, float znear, float P00, float P11, out vec4 aabb)
{
    vec4 c = vec4((min + max) * 0.5, 1.0) * view;
    vec3 s = (max - min) * 0.5;
    vec3 r = s * mat3(abs(view[0].xyz), abs(view[1].xyz), abs(view[2].xyz));
    
    return ProjectBoxView(c.xyz, r, znear, P00, P11, aabb);
}

bool ProjectSphereApprox(vec3 c, float r, mat4 view, float znear, float P00, float P11, out vec4 aabb)
{
    vec4 cv = vec4(c, 1.0) * view;

    return ProjectBoxView(cv.xyz, vec3(r), znear, P00, P11, aabb);
}








//输入：世界空间包围球
//由于是Hiz生成后再做的遮蔽剔除，无需使用上帧重投影
bool OcclusionCull( 
	BoundingSphere sphere,
	vec3 camera_pos, mat4 view, mat4 proj, float z_near,
	sampler2D Hiz) 	//Hiz的第0级没做复制，还在原深度缓冲里
{
	vec3 center = sphere.center_radius.xyz;
	float radius = sphere.center_radius.w;

	vec4 aabb;
	bool visible = true;

	if (ProjectSphere(center, radius, view, z_near, proj[0][0], proj[1][1], aabb)) 
	{

		ivec2 depth_pyramid_size 	= textureSize(Hiz, 0);	//计算Hiz对应尺寸的层级
		float width 				= (aabb.z - aabb.x) * depth_pyramid_size.x;
		float height 				= (aabb.w - aabb.y) * depth_pyramid_size.y;
		float level 				= max(floor(log2(max(width, height))), 1.0f);

		// Sampler is set up to do max reduction, so this computes the minimum depth of a 2x2 texel quad
		vec2 uv = (aabb.xy + aabb.zw) * 0.5;
    	uv.y = 1 - uv.y;
		
		float depth;
		depth = textureLod(Hiz, uv, level).r;
		depth = max(depth, textureLod(Hiz, vec2(aabb.x, aabb.y), level).r);
		depth = max(depth, textureLod(Hiz, vec2(aabb.x, aabb.w), level).r);
		depth = max(depth, textureLod(Hiz, vec2(aabb.z, aabb.y), level).r);
		depth = max(depth, textureLod(Hiz, vec2(aabb.z, aabb.w), level).r);
			
		vec3 dir 				= normalize(camera_pos - center);	//投影世界空间球面离相机最近的点到裁剪空间，比较深度
		float distance 			= length(camera_pos - center);		//距离小于包围盒半径的也直接当可见
		vec4 sceen_space_near 	= proj * view * vec4(center + dir * radius, 1.0);
		float depth_sphere 		= sceen_space_near.z / sceen_space_near.w;

		visible = (distance <= radius || depth_sphere <= depth);
	}

	return visible;
}

#endif