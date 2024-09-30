#ifndef MATH_GLSL
#define MATH_GLSL


float rcp(float x) { return 1 / x;}
float rsqrt(float x) { return 1 / sqrt(x);}

#define PI 		3.1415926
#define FLT_EPS 0.0000001

//采样周围九点的结构体
struct AdjacentTex3 {
    vec3 color[3][3];
};

struct AdjacentTex4 {
    vec4 color[3][3];
};

//计算线性深度
float Linear01Depth(float depth, float near, float far)
{
	return 1 / (((near - far) / (near)) * depth + far / near);
}

//带距离的深度
float LinearEyeDepth(float depth, float near, float far)            //view space深度的倒数和clip space深度有线性关系！！
{                                                                   
	return 1 / (((near - far) / (near * far)) * depth + 1 / near);
}

//重心插值
float barycentricsInterpolation(in float v0, in float v1, in float v2, vec3 barycentrics){
  return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

vec2 barycentricsInterpolation(in vec2 v0, in vec2 v1, in vec2 v2, vec3 barycentrics){
  return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

vec3 barycentricsInterpolation(in vec3 v0, in vec3 v1, in vec3 v2, vec3 barycentrics){
  return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

vec4 barycentricsInterpolation(in vec4 v0, in vec4 v1, in vec4 v2, vec3 barycentrics){
  return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}


//
vec3 clipAABB(vec3 aabb_min, vec3 aabb_max, vec3 p, vec3 q)
{
#ifdef USE_OPTIMIZATIONS
    // note: only clips towards aabb center (but fast!)
    vec3  p_clip  = 0.5 * (aabb_max + aabb_min);
    vec3  e_clip  = 0.5 * (aabb_max - aabb_min) + FLT_EPS;
    vec4  v_clip  = q - vec4(p_clip, p.w);
    vec3  v_unit  = v_clip.xyz / e_clip;
    vec3  a_unit  = abs(v_unit);
    float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));
    if (ma_unit > 1.0)
        return vec4(p_clip, p.w) + v_clip / ma_unit;
    else
        return q; // point inside aabb
#else
    vec3        r    = q - p;
    vec3        rmax = aabb_max - p.xyz;
    vec3        rmin = aabb_min - p.xyz;
    const float eps  = FLT_EPS;
    if (r.x > rmax.x + eps)
        r *= (rmax.x / r.x);
    if (r.y > rmax.y + eps)
        r *= (rmax.y / r.y);
    if (r.z > rmax.z + eps)
        r *= (rmax.z / r.z);
    if (r.x < rmin.x - eps)
        r *= (rmin.x / r.x);
    if (r.y < rmin.y - eps)
        r *= (rmin.y / r.y);
    if (r.z < rmin.z - eps)
        r *= (rmin.z / r.z);
    return p + r;
#endif
}

vec4 clipAABB(vec4 aabb_min, vec4 aabb_max, vec4 p, vec4 q)
{
    vec4        r    = q - p;
    vec4        rmax = aabb_max - p;
    vec4        rmin = aabb_min - p;
    const float eps  = FLT_EPS;
    if (r.x > rmax.x + eps)
        r *= (rmax.x / r.x);
    if (r.y > rmax.y + eps)
        r *= (rmax.y / r.y);
    if (r.z > rmax.z + eps)
        r *= (rmax.z / r.z);
    if (r.w > rmax.w + eps)
        r *= (rmax.w / r.w);   

    if (r.x < rmin.x - eps)
        r *= (rmin.x / r.x);
    if (r.y < rmin.y - eps)
        r *= (rmin.y / r.y);
    if (r.z < rmin.z - eps)
        r *= (rmin.z / r.z);
    if (r.w < rmin.w - eps)
        r *= (rmin.w / r.w);
    return p + r;
}


// https://software.intel.com/en-us/node/503873
vec3 RGBtoYCoCg(vec3 c)
{
    // Y = R/4 + G/2 + B/4
    // Co = R/2 - B/2
    // Cg = -R/4 + G/2 - B/4
    return vec3(
        c.x / 4.0 + c.y / 2.0 + c.z / 4.0,
        c.x / 2.0 - c.z / 2.0,
        -c.x / 4.0 + c.y / 2.0 - c.z / 4.0);
}

// https://software.intel.com/en-us/node/503873
vec3 YCoCgtoRGB(vec3 c)
{
    // R = Y + Co - Cg
    // G = Y + Cg
    // B = Y - Co - Cg
    // return clamp(vec3(
    //                  c.x + c.y - c.z,
    //                  c.x + c.z,
    //                  c.x - c.y - c.z),
    //              0.0,
    //              1.0);

	return vec3(c.x + c.y - c.z, c.x + c.z, c.x - c.y - c.z);
}

float RGBtoLuminance(vec3 c)
{
    return dot(c, vec3(0.2125, 0.7154, 0.0721)); 
}

AdjacentTex3 textureAdj3(sampler2D tex, vec2 uv, int mip)
{
    AdjacentTex3 adjacent;

    ivec2 texSize           = textureSize(tex, mip);
    vec2 du                 = vec2(1.0f / texSize.x, 0.0);
    vec2 dv                 = vec2(0.0, 1.0f / texSize.y);

    adjacent.color[0][0]    = texture(tex, uv - dv - du).rgb;
    adjacent.color[0][1]    = texture(tex, uv - dv).rgb;
    adjacent.color[0][2]    = texture(tex, uv - dv + du).rgb;
    adjacent.color[1][0]    = texture(tex, uv - du).rgb;
    adjacent.color[1][1]    = texture(tex, uv).rgb;
    adjacent.color[1][2]    = texture(tex, uv + du).rgb;
    adjacent.color[2][0]    = texture(tex, uv + dv - du).rgb;
    adjacent.color[2][1]    = texture(tex, uv + dv).rgb;
    adjacent.color[2][2]    = texture(tex, uv + dv + du).rgb;

    return adjacent;
}

AdjacentTex4 textureAdj4(sampler2D tex, vec2 uv, int mip)
{
    AdjacentTex4 adjacent;

    ivec2 texSize           = textureSize(tex, mip);
    vec2 du                 = vec2(1.0f / texSize.x, 0.0);
    vec2 dv                 = vec2(0.0, 1.0f / texSize.y);

    adjacent.color[0][0]    = texture(tex, uv - dv - du);
    adjacent.color[0][1]    = texture(tex, uv - dv);
    adjacent.color[0][2]    = texture(tex, uv - dv + du);
    adjacent.color[1][0]    = texture(tex, uv - du);
    adjacent.color[1][1]    = texture(tex, uv);
    adjacent.color[1][2]    = texture(tex, uv + du);
    adjacent.color[2][0]    = texture(tex, uv + dv - du);
    adjacent.color[2][1]    = texture(tex, uv + dv);
    adjacent.color[2][2]    = texture(tex, uv + dv + du);

    return adjacent;
}

vec3 maxAdj3(AdjacentTex3 adjacent)
{
    return max( adjacent.color[0][0], max(
                adjacent.color[0][1], max(
                adjacent.color[0][2], max(
                adjacent.color[1][0], max(
                adjacent.color[1][1], max(
                adjacent.color[1][2], max(
                adjacent.color[2][0], max(
                adjacent.color[2][1], 
                adjacent.color[2][2]))))))));
}

vec3 minAdj3(AdjacentTex3 adjacent)
{
    return min( adjacent.color[0][0], min(
                adjacent.color[0][1], min(
                adjacent.color[0][2], min(
                adjacent.color[1][0], min(
                adjacent.color[1][1], min(
                adjacent.color[1][2], min(
                adjacent.color[2][0], min(
                adjacent.color[2][1], 
                adjacent.color[2][2]))))))));
}

vec3 avgAdj3(AdjacentTex3 adjacent)
{
    return (    adjacent.color[0][0] + 
                adjacent.color[0][1] + 
                adjacent.color[0][2] + 
                adjacent.color[1][0] + 
                adjacent.color[1][1] + 
                adjacent.color[1][2] + 
                adjacent.color[2][0] + 
                adjacent.color[2][1] + 
                adjacent.color[2][2]) / 9.0;
}

vec4 maxAdj4(AdjacentTex4 adjacent)
{
    return max( adjacent.color[0][0], max(
                adjacent.color[0][1], max(
                adjacent.color[0][2], max(
                adjacent.color[1][0], max(
                adjacent.color[1][1], max(
                adjacent.color[1][2], max(
                adjacent.color[2][0], max(
                adjacent.color[2][1], 
                adjacent.color[2][2]))))))));
}

vec4 minAdj4(AdjacentTex4 adjacent)
{
    return min( adjacent.color[0][0], min(
                adjacent.color[0][1], min(
                adjacent.color[0][2], min(
                adjacent.color[1][0], min(
                adjacent.color[1][1], min(
                adjacent.color[1][2], min(
                adjacent.color[2][0], min(
                adjacent.color[2][1], 
                adjacent.color[2][2]))))))));
}

vec4 avgAdj4(AdjacentTex4 adjacent)
{
    return (    adjacent.color[0][0] + 
                adjacent.color[0][1] + 
                adjacent.color[0][2] + 
                adjacent.color[1][0] + 
                adjacent.color[1][1] + 
                adjacent.color[1][2] + 
                adjacent.color[2][0] + 
                adjacent.color[2][1] + 
                adjacent.color[2][2]) / 9.0;
}

vec3 clampedColor(vec3 color, AdjacentTex3 clampAdj)
{
    vec3 cmin           = minAdj3(clampAdj);
    vec3 cmax           = maxAdj3(clampAdj);
    vec3 cavg           = avgAdj3(clampAdj);

    //return  color;
    //return  clamp(color, cmin, cmax);
    return clipAABB(cmin, cmax, clamp(cavg, cmin, cmax), color);      
}

vec4 clampedColor(vec4 color, AdjacentTex4 clampAdj)
{
    vec4 cmin           = minAdj4(clampAdj);
    vec4 cmax           = maxAdj4(clampAdj);
    vec4 cavg           = avgAdj4(clampAdj);

    //return  color;
    //return  clamp(color, cmin, cmax);
    return clipAABB(cmin, cmax, clamp(cavg, cmin, cmax), color);      
}


void RGBtoYCoCg(inout AdjacentTex3 adjacent)
{
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            adjacent.color[i][j] = RGBtoYCoCg(adjacent.color[i][j]);
        }
    }
}

void YCoCgtoRGB(inout AdjacentTex3 adjacent)
{
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            adjacent.color[i][j] = YCoCgtoRGB(adjacent.color[i][j]);
        }
    }
}

float sign_not_zero(in float k)
{
    return (k >= 0.0) ? 1.0 : -1.0;
}

vec2 sign_not_zero(in vec2 v)
{
    return vec2(sign_not_zero(v.x), sign_not_zero(v.y));
}

// 八面体映射，输入输出范围[-1, 1]
vec2 Oct_Encode(in vec3 v) 
{
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    vec2 result = v.xy * (1.0 / l1norm);
    if (v.z < 0.0)
        result = (1.0 - abs(result.yx)) * sign_not_zero(result.xy);
    return result;
}

vec3 Oct_Decode(vec2 o)
{
    vec3 v = vec3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));

    if (v.z < 0.0)
        v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);

    return normalize(v);
}

// A simple utility to convert a float to a 2-component octohedral representation
// 向量压缩算法，实现思路和八面体映射一致
vec2 Direction_to_Octohedral(vec3 normal)
{
    vec2 p = normal.xy * (1.0f / dot(abs(normal), vec3(1.0f)));
    return normal.z > 0.0f ? p : (1.0f - abs(p.yx)) * (step(0.0f, p) * 2.0f - vec2(1.0f));
}

vec3 Octohedral_to_Direction(vec2 e)
{
    vec3 v = vec3(e, 1.0 - abs(e.x) - abs(e.y));
    if (v.z < 0.0)
        v.xy = (1.0 - abs(v.yx)) * (step(0.0, v.xy) * 2.0 - vec2(1.0));
    return normalize(v);
}


// https://zhuanlan.zhihu.com/p/655725271
// https://fileadmin.cs.lth.se/graphics/research/papers/2008/simdmapping/clarberg_simdmapping08_preprint.pdf

/*
// For the two equal area functions below to correctly map between eachother
// There needs to be a sign function that returns 1.0 for 0.0 and
// -1.0 for -0.0. This is due to the discontinuity in the octahedral map
// between the upper left triangle and bottom left triangle for example
// (same on right). Check image in RT Gems 16.5.4.2, light purple and dark
// purple for example.
float SignPreserveZero(float v)
{
	int i = asint(v);

	return (i < 0) ? -1.0 : 1.0;
}

// "Fast Equal-Area Mapping of the (Hemi)Sphere using SIMD"
vec3 OctSphereMap(vec2 u)
{
	u = u * 2.f - 1.f;

    // Compute radius r (branchless)
	float d = 1.f - (abs(u.x) + abs(u.y));
	float r = 1.f - abs(d);

    // Compute phi in the first quadrant (branchless, except for the
    // division-by-zero test), using sign(u) to map the result to the
    // correct quadrant below
	float phi = (r == 0.f) ? 0.f :
        (PI / 4 * ((abs(u.y) - abs(u.x)) / r + 1.f));

	float f = r * sqrt(2.f - r * r);

    // abs() around f * cos/sin(phi) is necessary because they can return
    // negative 0 due to floating precision
	float x = SignPreserveZero(u.x) * abs(f * cos(phi));
	float y = SignPreserveZero(u.y) * abs(f * sin(phi));
	float z = SignPreserveZero(d) * (1.f - r * r);

	return vec3(x, y, z);
}

vec2 InvOctSphereMap(vec3 dir)
{
	float r = sqrt(1.f - abs(dir.z));
	float phi = atan2(abs(dir.y), abs(dir.x));

	vec2 uv;
	uv.y = r * phi * M_2_PI;
	uv.x = r - uv.y;

	if (dir.z < 0.f)
	{
		uv = 1.f - uv.yx;
	}

	uv.x *= SignPreserveZero(dir.x);
	uv.y *= SignPreserveZero(dir.y);

	return uv * 0.5f + 0.5f;
}
*/




vec3 ToneMap(vec3 x)
{
    return x / (x + vec3(1.0f)); // Reinhard tonemap
}

vec3 InverseToneMap(vec3 x)
{
    return x / max((vec3(1.0f) - x), vec3(FLT_EPS));
}

float Saturate(float x)
{
	return clamp(x, 0.0f, 1.0f);
}

vec2 Saturate(vec2 x)
{
	return clamp(x, 0.0f, 1.0f);
}

vec3 Saturate(vec3 x)
{
	return clamp(x, 0.0f, 1.0f);
}

vec4 Saturate(vec4 x)
{
	return clamp(x, 0.0f, 1.0f);
}

float Lerp(float a, float b, float c)
{
    return (1 - c) * a + c * b;
}

vec2 Lerp(vec2 a, vec2 b, float c)
{
    return (1 - c) * a + c * b;
}

vec3 Lerp(vec3 a, vec3 b, float c)
{
    return (1 - c) * a + c * b;
}

vec4 Lerp(vec4 a, vec4 b, float c)
{
    return (1 - c) * a + c * b;
}

float Square( float x )
{
	return x*x;
}

vec2 Square( vec2 x )
{
	return x*x;
}

vec3 Square( vec3 x )
{
	return x*x;
}

vec4 Square( vec4 x )
{
	return x*x;
}

float Pow2( float x )
{
	return x*x;
}

vec2 Pow2( vec2 x )
{
	return x*x;
}

vec3 Pow2( vec3 x )
{
	return x*x;
}

vec4 Pow2( vec4 x )
{
	return x*x;
}

float Pow3( float x )
{
	return x*x*x;
}

vec2 Pow3( vec2 x )
{
	return x*x*x;
}


vec3 Pow3( vec3 x )
{
	return x*x*x;
}

vec4 Pow3( vec4 x )
{
	return x*x*x;
}

float Pow4( float x )
{
	float xx = x*x;
	return xx * xx;
}

vec2 Pow4( vec2 x )
{
	vec2 xx = x*x;
	return xx * xx;
}

vec3 Pow4( vec3 x )
{
	vec3 xx = x*x;
	return xx * xx;
}

vec4 Pow4( vec4 x )
{
	vec4 xx = x*x;
	return xx * xx;
}

float Pow5( float x )
{
	float xx = x*x;
	return xx * xx * x;
}

vec2 Pow5( vec2 x )
{
	vec2 xx = x*x;
	return xx * xx * x;
}

vec3 Pow5( vec3 x )
{
	vec3 xx = x*x;
	return xx * xx * x;
}

vec4 Pow5( vec4 x )
{
	vec4 xx = x*x;
	return xx * xx * x;
}

#endif