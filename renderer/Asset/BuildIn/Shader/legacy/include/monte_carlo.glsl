#ifndef MONTE_CARLO_GLSL
#define MONTE_CARLO_GLSL


// 全部来自UE Private/MonteCarlo.ush

// 切线空间转换/////////////////////////////////////////////////////////////////////

// [ Duff et al. 2017, "Building an Orthonormal Basis, Revisited" ]
// Discontinuity at TangentZ.z == 0
mat3 GetTangentBasis( vec3 TangentZ )
{
	const float Sign = TangentZ.z >= 0 ? 1 : -1;
	const float a = -rcp( Sign + TangentZ.z );
	const float b = TangentZ.x * TangentZ.y * a;
	
	vec3 TangentX = { 1 + Sign * a * Pow2( TangentZ.x ), Sign * b, -Sign * TangentZ.x };
	vec3 TangentY = { b,  Sign + a * Pow2( TangentZ.y ), -TangentZ.y };

	return mat3( TangentX, TangentY, TangentZ );
}

// [Frisvad 2012, "Building an Orthonormal Basis from a 3D Unit Vector Without Normalization"]
// Discontinuity at TangentZ.z < -0.9999999f
mat3 GetTangentBasisFrisvad(vec3 TangentZ)
{
	vec3 TangentX;
	vec3 TangentY;

	if (TangentZ.z < -0.9999999f)
	{
		TangentX = vec3(0, -1, 0);
		TangentY = vec3(-1, 0, 0);
	}
	else
	{
		float A = 1.0f / (1.0f + TangentZ.z);
		float B = -TangentZ.x * TangentZ.y * A;
		TangentX = vec3(1.0f - TangentZ.x * TangentZ.x * A, B, -TangentZ.x);
		TangentY = vec3(B, 1.0f - TangentZ.y * TangentZ.y * A, -TangentZ.y);
	}

	return mat3( TangentX, TangentY, TangentZ );
}

vec3 TangentToWorld( vec3 Vec, vec3 TangentZ )
{
	return GetTangentBasis(TangentZ) * Vec;
}

vec3 WorldToTangent( vec3 Vec, vec3 TangentZ )
{
	return GetTangentBasis(TangentZ) * Vec;
}

// 低差异序列/////////////////////////////////////////////////////////////////////


float RadicalInverse(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10f;
}
 
vec2 Hammersley(uint i, uint N) 
{
    return vec2(float(i) / float(N), RadicalInverse(i));
}

// vec2 Hammersley( uint Index, uint NumSamples, uvec2 Random )
// {
// 	float E1 = fract( float(Index) / NumSamples + float( Random.x & 0xffff ) / (1<<16) );
// 	float E2 = float( bitfieldReverse(Index) ^ Random.y ) * 2.3283064365386963e-10;
// 	return vec2( E1, E2 );
// }

// vec2 Hammersley16( uint Index, uint NumSamples, uvec2 Random )
// {
// 	float E1 = fract( float(Index) / NumSamples + float( Random.x ) * (1.0 / 65536.0) );
// 	float E2 = float( ( bitfieldReverse(Index) >> 16 ) ^ Random.y ) * (1.0 / 65536.0);
// 	return vec2( E1, E2 );
// }

// 球面映射/////////////////////////////////////////////////////////////////////

// Based on: [Clarberg 2008, "Fast Equal-Area Mapping of the (Hemi)Sphere using SIMD"]
// Fixed sign bit for UV.y == 0 and removed branch before division by using a small epsilon
// https://fileadmin.cs.lth.se/graphics/research/papers/2008/simdmapping/clarberg_simdmapping08_preprint.pdf
vec3 EquiAreaSphericalMapping(vec2 UV)
{
	UV = 2 * UV - 1;
	float D = 1 - (abs(UV.x) + abs(UV.y));
	float R = 1 - abs(D);
	// Branch to avoid dividing by 0.
	// Only happens with (0.5, 0.5), usually occurs in odd number resolutions which use the very central texel
	float Phi = R == 0 ? 0 : (PI / 4) * ((abs(UV.y) - abs(UV.x)) / R + 1);
	float F = R * sqrt(2 - R * R);
	return vec3(
		F * sign(UV.x) * abs(cos(Phi)),
		F * sign(UV.y) * abs(sin(Phi)),
		sign(D) * (1 - R * R)
	);
}

// Based on: [Clarberg 2008, "Fast Equal-Area Mapping of the (Hemi)Sphere using SIMD"]
// Removed branch before division by using a small epsilon
// https://fileadmin.cs.lth.se/graphics/research/papers/2008/simdmapping/clarberg_simdmapping08_preprint.pdf
vec2 InverseEquiAreaSphericalMapping(vec3 Direction)
{
	// Most use cases of this func generate Direction by diffing two positions and thus unnormalized
	Direction = normalize(Direction);
	
	vec3 AbsDir = abs(Direction);
	float R = sqrt(1 - AbsDir.z);
	float Epsilon = 5.42101086243e-20; // 2^-64 (this avoids 0/0 without changing the rest of the mapping)
	float x = min(AbsDir.x, AbsDir.y) / (max(AbsDir.x, AbsDir.y) + Epsilon);

	// Coefficients for 6th degree minimax approximation of atan(x)*2/pi, x=[0,1].
	const float t1 = 0.406758566246788489601959989e-5f;
	const float t2 = 0.636226545274016134946890922156f;
	const float t3 = 0.61572017898280213493197203466e-2f;
	const float t4 = -0.247333733281268944196501420480f;
	const float t5 = 0.881770664775316294736387951347e-1f;
	const float t6 = 0.419038818029165735901852432784e-1f;
	const float t7 = -0.251390972343483509333252996350e-1f;

	// Polynomial approximation of atan(x)*2/pi
	float Phi = t6 + t7 * x;
	Phi = t5 + Phi * x;
	Phi = t4 + Phi * x;
	Phi = t3 + Phi * x;
	Phi = t2 + Phi * x;
	Phi = t1 + Phi * x;

	Phi = (AbsDir.x < AbsDir.y) ? 1 - Phi : Phi;
	vec2 UV = vec2(R - Phi * R, Phi * R);
	UV = (Direction.z < 0) ? 1 - UV.yx : UV;
	UV = vec2(uvec2(UV) ^ (uvec2(Direction.xy) & 0x80000000u));
	return UV * 0.5 + 0.5;
}


// 采样函数/////////////////////////////////////////////////////////////////////

vec2 UniformSampleDisk( vec2 E )
{
	float Theta = 2 * PI * E.x;
	float Radius = sqrt( E.y );
	return Radius * vec2( cos( Theta ), sin( Theta ) );
}

// Returns a point on the unit circle and a radius in z
vec3 ConcentricDiskSamplingHelper(vec2 E)
{
	// Rescale input from [0,1) to (-1,1). This ensures the output radius is in [0,1)
	vec2 p = 2 * E - 0.99999994;
	vec2 a = abs(p);
	float Lo = min(a.x, a.y);
	float Hi = max(a.x, a.y);
	float Epsilon = 5.42101086243e-20; // 2^-64 (this avoids 0/0 without changing the rest of the mapping)
	float Phi = (PI / 4) * (Lo / (Hi + Epsilon) + 2 * float(a.y >= a.x));
	float Radius = Hi;
	// copy sign bits from p
	const uint SignMask = 0x80000000;
	vec2 Disk = vec2((uvec2(vec2(cos(Phi), sin(Phi))) & ~SignMask) | (uvec2(p) & SignMask));
	// return point on the circle as well as the radius
	return vec3(Disk, Radius);
}

vec2 UniformSampleDiskConcentric( vec2 E )
{
	vec3 Result = ConcentricDiskSamplingHelper(E);
	return Result.xy * Result.z; // uniform sampling
}

// based on the approximate equal area transform from
// http://marc-b-reynolds.github.io/math/2017/01/08/SquareDisc.html
vec2 UniformSampleDiskConcentricApprox( vec2 E )
{
	vec2 sf = E * sqrt(2.0) - sqrt(0.5);	// map 0..1 to -sqrt(0.5)..sqrt(0.5)
	vec2 sq = sf*sf;
	float root = sqrt(2.0*max(sq.x, sq.y) - min(sq.x, sq.y));
	if (sq.x > sq.y)
	{
		sf.x = sf.x > 0 ? root : -root;
	}
	else
	{
		sf.y = sf.y > 0 ? root : -root;
	}
	return sf;
}

// PDF = 1 / (4 * PI)
vec4 UniformSampleSphere( vec2 E )
{
	float Phi = 2 * PI * E.x;
	float CosTheta = 1 - 2 * E.y;
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );

	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;

	float PDF = 1.0 / (4 * PI);

	return vec4( H, PDF );
}

// PDF = 1 / (2 * PI)
vec4 UniformSampleHemisphere( vec2 E )
{
	float Phi = 2 * PI * E.x;
	float CosTheta = E.y;
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );

	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;

	float PDF = 1.0 / (2 * PI);

	return vec4( H, PDF );
}



// 重要性采样/////////////////////////////////////////////////////////////////////
// E.x 圆周角
// E.y 仰角，[90, 0] 对应 [0, 1]
// PDF = D * NoH / (4 * VoH)
// 根据样本生成一个采样用的半程向量
vec4 ImportanceSampleGGX( vec2 E, float a2 )
{
	float Phi = 2 * PI * E.x;
	float CosTheta = sqrt( (1 - E.y) / ( 1 + (a2 - 1) * E.y ) );
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );

	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;
	
	float d = ( CosTheta * a2 - CosTheta ) * CosTheta + 1;
	float D = a2 / ( PI*d*d );
	float PDF = D * CosTheta;	// 用法线分布来对环境光照进行采样时的使用的概率密度函数，按法线分布函数的定义，D * CosTheta半球积分就是1

	return vec4( H, PDF );
}


#endif