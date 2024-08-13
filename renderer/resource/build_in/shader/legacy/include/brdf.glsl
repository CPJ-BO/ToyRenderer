#ifndef BRDF_GLSL
#define BRDF_GLSL


#include "math.glsl"
#include "monte_carlo.glsl"


// https://zhuanlan.zhihu.com/p/145410416

//辐射度量学
//辐射能量				Q	radiant energy  		辐射的电磁能，焦耳
//辐射通量/辐射功率		 Φ	 radiant flux			单位时间的辐射能量，瓦特
//辐射强度				I    radiant intensity		单位立体角的辐射通量，坎德拉				立体角：所对应球面面积除以半径的平方；球面立体角共4 PI
//辐照度				E	irradiance				单位面积的辐射通量，瓦特/平方米				遵从Lambert's Law，余弦
//辐射率				L	radiance				单位垂直面积单位立体角的辐射通量，尼特		（定义时考虑了方向无关性，积分辐照度时加上余弦）






struct BxDFContext
{
	float NoV;
	float NoL;
	float VoL;
	float NoH;
	float VoH;
};

void Init( inout BxDFContext Context, vec3 N, vec3 V, vec3 L )
{
	Context.NoL = dot(N, L);
	Context.NoV = dot(N, V);
	Context.VoL = dot(V, L);
	float InvLenH = rsqrt( 2 + 2 * Context.VoL );
	//Context.NoH = saturate( ( Context.NoL + Context.NoV ) * InvLenH );
	//Context.VoH = saturate( InvLenH + InvLenH * Context.VoL );
    Context.NoH = clamp(( Context.NoL + Context.NoV ) * InvLenH, 0, 1);
	Context.VoH = clamp(InvLenH + InvLenH * Context.VoL, 0, 1);
}

//IBL/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 PreFilteredReflection(vec3 R, float roughness, samplerCube prefilteredMap)
{
	const float MAX_REFLECTION_LOD = 8.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
	vec3 b = textureLod(prefilteredMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

//Learn OpenGL//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// Physically based shading model
// parameterized with the below options
// [ Karis 2013, "Real Shading in Unreal Engine 4" slide 11 ]

// E = Random sample for BRDF.
// N = Normal of the macro surface.
// H = Normal of the micro surface.
// V = View vector going from surface's position towards the view's origin.
// L = Light ray direction

// D = Microfacet NDF
// G = Shadowing and masking
// F = Fresnel

// Vis = G / (4*NoL*NoV)
// f = Microfacet specular BRDF = D*G*F / (4*NoL*NoV) = D*Vis*F

// a 粗糙度平方 a2 粗糙度四次方

//漫反射项////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



vec3 Diffuse_Lambert( vec3 DiffuseColor )
{
	return DiffuseColor * (1 / PI);
}

// [Burley 2012, "Physically-Based Shading at Disney"]
vec3 Diffuse_Burley( vec3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH )
{
	float FD90 = 0.5 + 2 * VoH * VoH * Roughness;
	float FdV = 1 + (FD90 - 1) * pow(1 - NoV, 5);
	float FdL = 1 + (FD90 - 1) * pow(1 - NoL, 5);
	return DiffuseColor * ( (1 / PI) * FdV * FdL );
}

// [Gotanda 2012, "Beyond a Simple Physically Based Blinn-Phong Model in Real-Time"]
vec3 Diffuse_OrenNayar( vec3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH )
{
	float a = Roughness * Roughness;
	float s = a;// / ( 1.29 + 0.5 * a );
	float s2 = s * s;
	float VoL = 2 * VoH * VoH - 1;		// double angle identity
	float Cosri = VoL - NoV * NoL;
	float C1 = 1 - 0.5 * s2 / (s2 + 0.33);
	float C2 = 0.45 * s2 / (s2 + 0.09) * Cosri * ( Cosri >= 0 ? rcp( max( NoL, NoV ) ) : 1 );
	return DiffuseColor / PI * ( C1 + C2 ) * ( 1 + Roughness * 0.5 );
}

// [Gotanda 2014, "Designing Reflectance Models for New Consoles"]
vec3 Diffuse_Gotanda( vec3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH )
{
	float a = Roughness * Roughness;
	float a2 = a * a;
	float F0 = 0.04;
	float VoL = 2 * VoH * VoH - 1;		// double angle identity
	float Cosri = VoL - NoV * NoL;

	float a2_13 = a2 + 1.36053;
	float Fr = ( 1 - ( 0.542026*a2 + 0.303573*a ) / a2_13 ) * ( 1 - pow( 1 - NoV, 5 - 4*a2 ) / a2_13 ) * ( ( -0.733996*a2*a + 1.50912*a2 - 1.16402*a ) * pow( 1 - NoV, 1 + rcp(39*a2*a2+1) ) + 1 );
	float Lm = ( max( 1 - 2*a, 0 ) * ( 1 - pow( 1 - NoL, 5) ) + min( 2*a, 1 ) ) * ( 1 - 0.5*a + 0.5*a * NoL );
	float Vd = ( a2 / ( (a2 + 0.09) * (1.31072 + 0.995584 * NoV) ) ) * ( 1 - pow( 1 - NoL, ( 1 - 0.3726732 * NoV * NoV ) / ( 0.188566 + 0.38841 * NoV ) ) );
	float Bp = Cosri < 0 ? 1.4 * NoV * Cosri : Cosri / max( NoL, 1e-8 );
	float Lr = (21.0 / 20.0) * (1 - F0) * ( Fr * Lm + Vd + Bp );
	return DiffuseColor / PI * Lr;
}

//法线分布项NDF////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define POW_CLAMP 0.000001f
float ClampedPow(float X,float Y)
{
	return pow(max(abs(X),POW_CLAMP),Y);
}

float PhongShadingPow(float X, float Y)
{
	// The following clamping is done to prevent NaN being the result of the specular power computation.
	// Clamping has a minor performance cost.

	// In HLSL pow(a, b) is implemented as exp2(log2(a) * b).

	// For a=0 this becomes exp2(-inf * 0) = exp2(NaN) = NaN.

	// As seen in #TTP 160394 "QA Regression: PS3: Some maps have black pixelated artifacting."
	// this can cause severe image artifacts (problem was caused by specular power of 0, lightshafts propagated this to other pixels).
	// The problem appeared on PlayStation 3 but can also happen on similar PC NVidia hardware.

	// In order to avoid platform differences and rarely occuring image atrifacts we clamp the base.

	// Note: Clamping the exponent seemed to fix the issue mentioned TTP but we decided to fix the root and accept the
	// minor performance cost.

	return ClampedPow(X, Y);
}

// [Blinn 1977, "Models of light reflection for computer synthesized pictures"]
float D_Blinn( float a2, float NoH )
{
	float n = 2 / a2 - 2;
	return (n+2) / (2*PI) * PhongShadingPow( NoH, n );		// 1 mad, 1 exp, 1 mul, 1 log
}

// [Beckmann 1963, "The scattering of electromagnetic waves from rough surfaces"]
float D_Beckmann( float a2, float NoH )
{
	float NoH2 = NoH * NoH;
	return exp( (NoH2 - 1) / (a2 * NoH2) ) / ( PI * a2 * NoH2 * NoH2 );
}

// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX( float a2, float NoH )
{
	float d = ( NoH * a2 - NoH ) * NoH + 1;	// 2 mad
	return a2 / ( PI*d*d );					// 4 mul, 1 rcp
}

// Anisotropic GGX
// [Burley 2012, "Physically-Based Shading at Disney"]
float D_GGXaniso( float ax, float ay, float NoH, float XoH, float YoH )
{
	float d = XoH*XoH / (ax*ax) + YoH*YoH / (ay*ay) + NoH*NoH;
	return 1.0f / ( PI * ax*ay * d*d );
}

float D_InvBlinn( float a2, float NoH )
{
	float A = 4;
	float Cos2h = NoH * NoH;
	float Sin2h = 1 - Cos2h;
	//return rcp( PI * (1 + A*m2) ) * ( 1 + A * ClampedPow( Sin2h, 1 / m2 - 1 ) );
	return rcp( PI * (1 + A*a2) ) * ( 1 + A * exp( -Cos2h / a2 ) );
}

float D_InvBeckmann( float a2, float NoH )
{
	float A = 4;
	float Cos2h = NoH * NoH;
	float Sin2h = 1 - Cos2h;
	float Sin4h = Sin2h * Sin2h;
	return rcp( PI * (1 + A*a2) * Sin4h ) * ( Sin4h + A * exp( -Cos2h / (a2 * Sin2h) ) );
}

float D_InvGGX( float a2, float NoH )
{
	float A = 4;
	float d = ( NoH - a2 * NoH ) * NoH + a2;
	return rcp( PI * (1 + A*a2) ) * ( 1 + 4 * a2*a2 / ( d*d ) );
}

//菲涅尔项////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec3 F_None( vec3 SpecularColor )
{
	return SpecularColor;
}

// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
vec3 F_Schlick( vec3 SpecularColor, float VoH )
{
	float Fc = pow(1 - VoH, 5);					// 1 sub, 3 mul
	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad
	
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return clamp(( 50.0 * SpecularColor.g ), 0, 1) * Fc + (1 - Fc) * SpecularColor;
}

vec3 F_Schlick(vec3 F0, vec3 F90, float VoH)
{
	float Fc = pow(1 - VoH, 5);
	return F90 * Fc + (1 - Fc) * F0;
}

vec3 F_AdobeF82(vec3 F0, vec3 F82, float VoH)
{
	// [Kutz et al. 2021, "Novel aspects of the Adobe Standard Material" ]
	// See Section 2.3 (note the formulas in the paper do not match the code, the code is the correct version)
	// The constants below are derived by just constant folding the terms dependent on CosThetaMax=1/7
	const float Fc = pow(1 - VoH, 5);
	const float K = 49.0 / 46656.0;
	vec3 b = (K - K * F82) * (7776.0 + 9031.0 * F0);
	return clamp((F0 + Fc * ((1 - F0) - b * (VoH - VoH * VoH))), 0, 1);
}

vec3 F_Fresnel( vec3 SpecularColor, float VoH )
{
	vec3 SpecularColorSqrt = sqrt( clamp(SpecularColor, vec3(0, 0, 0), vec3(0.99, 0.99, 0.99) ) );
	vec3 n = ( 1 + SpecularColorSqrt ) / ( 1 - SpecularColorSqrt );
	vec3 g = sqrt( n*n + VoH*VoH - 1 );
	return 0.5 * Square( (g - VoH) / (g + VoH) ) * ( 1 + Square( ((g+VoH)*VoH - 1) / ((g-VoH)*VoH + 1) ) );
}

//几何函数项G，Vis合并了几何函数和归一化系数(4*NoL*NoV)////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Vis_Implicit()
{
	return 0.25;
}

// [Neumann et al. 1999, "Compact metallic reflectance models"]
float Vis_Neumann( float NoV, float NoL )
{
	return 1 / ( 4 * max( NoL, NoV ) );
}

// [Kelemen 2001, "A microfacet based coupled specular-matte brdf model with importance sampling"]
float Vis_Kelemen( float VoH )
{
	// constant to prevent NaN
	return rcp( 4 * VoH * VoH + 1e-5);
}

// Tuned to match behavior of Vis_Smith
// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
float Vis_Schlick( float a2, float NoV, float NoL )
{
	float k = sqrt(a2) * 0.5;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25 / ( Vis_SchlickV * Vis_SchlickL );
}

// Smith term for GGX
// [Smith 1967, "Geometrical shadowing of a random rough surface"]
float Vis_Smith( float a2, float NoV, float NoL )
{
	float Vis_SmithV = NoV + sqrt( NoV * (NoV - NoV * a2) + a2 );
	float Vis_SmithL = NoL + sqrt( NoL * (NoL - NoL * a2) + a2 );
	return rcp( Vis_SmithV * Vis_SmithL );
}

// Appoximation of joint Smith term for GGX
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJointApprox( float a2, float NoV, float NoL )
{
	float a = sqrt(a2);
	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
	return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
}

// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJoint(float a2, float NoV, float NoL) 
{
	float Vis_SmithV = NoL * sqrt(NoV * (NoV - NoV * a2) + a2);
	float Vis_SmithL = NoV * sqrt(NoL * (NoL - NoL * a2) + a2);
	return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
}












//////////////////////////////////////////////////////////////////////////////////////////////////////

vec3 Basic_BRDF(vec3 albedo, vec3 F0, float roughness, float metallic, vec3 N, vec3 V, vec3 L)
{
	BxDFContext context;
    Init(context, N, V, L);

	float a2 			= Pow4(roughness) ;                                          

	float D             = D_GGX(a2, context.NoH);        
    float Vis           = Vis_Smith(a2, context.NoV, context.NoL ); 
    vec3 F              = F_Schlick(F0, context.VoH );      

	//vec3 f_diffuse    = Diffuse_Lambert(albedo);                                                                  	//BRDF反射函数，漫反射项
	vec3 f_diffuse      = Diffuse_Burley(albedo, roughness, context.NoV, context.NoL, context.VoH );
	vec3 f_specular     = D * Vis * F;                                                                                  //BRDF反射函数，镜面反射项

	vec3 k_specular     = F;                                                                                            //镜面反射率，也就是菲涅尔项
	vec3 k_diffuse      = (vec3(1.0f) - k_specular);                                                                    //漫反射率

	vec3 f_r            = (1.0 - metallic) * k_diffuse * f_diffuse +                                                    //BRDF反射函数             
							f_specular;																					//(1.0 - metallic)是漫反射颜色和albedo之间的系数差

	return f_r;
}

vec3 Basic_BRDF_Diffuse(vec3 albedo, vec3 F0, float roughness, float metallic, vec3 N, vec3 V, vec3 L)
{
	BxDFContext context;
    Init(context, N, V, L);

    vec3 F              = F_Schlick(F0, context.VoH );    

	//vec3 f_diffuse    = Diffuse_Lambert(albedo);     
	vec3 f_diffuse      = Diffuse_Burley(albedo, roughness, context.NoV, context.NoL, context.VoH ); 
	vec3 k_diffuse      = (vec3(1.0f) - F);                                                                 

	vec3 f_r            = (1.0 - metallic) * k_diffuse * f_diffuse;

	return f_r;
}

vec3 Basic_BRDF_Specular(vec3 F0, float roughness, vec3 N, vec3 V, vec3 L)
{
	BxDFContext context;
    Init(context, N, V, L);

	float a2 			= Pow4(roughness) ;                                          

	float D             = D_GGX(a2, context.NoH);        
    float Vis           = Vis_Smith(a2, context.NoV, context.NoL ); 
    vec3 F              = F_Schlick(F0, context.VoH );      

	vec3 f_specular     = D * Vis * F;              

	return f_specular;
}

//Learn OpenGL
// {
// 	 vec3 F0 = vec3(0.04); 
//     F0 = mix(F0, albedo, metallic);

//     // reflectance equation
//     vec3 Lo = vec3(0.0);
//     for(int i = 0; i < 4; ++i) 
//     {
//         // calculate per-light radiance
//         vec3 L = normalize(lightPositions[i] - WorldPos);
//         vec3 H = normalize(V + L);
//         float distance = length(lightPositions[i] - WorldPos);
//         float attenuation = 1.0 / (distance * distance);
//         vec3 radiance = lightColors[i] * attenuation;

//         // Cook-Torrance BRDF
//         float NDF = DistributionGGX(N, H, roughness);   
//         float G   = GeometrySmith(N, V, L, roughness);      
//         vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
//         vec3 numerator    = NDF * G * F; 
//         float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
//         vec3 specular = numerator / denominator;
        
//         // kS is equal to Fresnel
//         vec3 kS = F;
//         // for energy conservation, the diffuse and specular light can't
//         // be above 1.0 (unless the surface emits light); to preserve this
//         // relationship the diffuse component (kD) should equal 1.0 - kS.
//         vec3 kD = vec3(1.0) - kS;
//         // multiply kD by the inverse metalness such that only non-metals 
//         // have diffuse lighting, or a linear blend if partly metal (pure metals
//         // have no diffuse light).
//         kD *= 1.0 - metallic;	  

//         // scale light by NdotL
//         float NdotL = max(dot(N, L), 0.0);        

//         // add to outgoing radiance Lo
//         Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
//     }   
// }




#endif