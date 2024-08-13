
#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"
#include "../include/frag.glsl"

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 outVelocity;		//速度buffer

void main() 
{
    //TODO 这个着色器还有未知bug，启动时有可能崩溃？


    Object object           = OBJECTS.slot[ID];
    MaterialInfo material   = fetchMaterial(object); 

	outVelocity = writeVelocity(POSITION, PREV_POSITION);

	//材质参数////////////////////////////////////////////////////////////////////////////////////////////////////////
                                   
    float lambertClamp  = fetchMaterialFloat(material, 0);                            //floats[0] lambertClamp
    float rampTexOffset = fetchMaterialFloat(material, 1);                           //floats[1] Ramp Tex offset
    
    vec4 albedo         = fetchDiffuse(material, TEXCOORD);  		
    vec3 normal         = fetchNormal(material, TEXCOORD, NORMAL, TANGENT);			                               


    
    vec4 lightMap       = fetchMaterialTex2D(material, 0, TEXCOORD).rgba;  			//LightMap
    lightMap = pow(lightMap, vec4(1.0/2.2));   

    //vec4 ramp = fetchMaterialTex2D(material, 1, TEXCOORD).rgba;  					    //Ramp，在下面计算阴影后采样
    //ramp = pow(ramp, vec4(1.0/2.2));      

    ivec2 rampTexSize = textureSize(TEXTURES_2D[material.texture_slots1_2d[2]], 0).xy;

    float metallic = lightMap.r;                                                    //LightMap.r        金属度
    float ao = lightMap.g;                                                          //LightMap.g        ao阴影      0为常暗，  0.5为正常， 1为常亮
    float specular = lightMap.b;                                                    //LightMap.b        高光细节
    float materialType = lightMap.a;                                                //LightMap.a        材质类型，用于ramp采样和描边颜色  0.3/0.5/0.7/1.0

    //光照计算相关向量////////////////////////////////////////////////////////////////////////////////////////////////////////

	vec3 worldPos = POSITION.xyz;
	vec3 viewPos = (CAMERA.view * POSITION).xyz;

	vec3 V = normalize(VIEW_VEC);	//视线
    vec3 N = normalize(normal);	    //法线


	//屏幕空间边缘光计算////////////////////////////////////////////////////////////////////////////////////////////////////////
    vec4 rimColor       = fetchMaterialColor(material, 0);                                                            //colors[0] Rim Color
    float rimThreshold  = fetchMaterialFloat(material, 4);                                                       //floats[4] Rim threshold
    float rimStrength   = fetchMaterialFloat(material, 3);                                                        //floats[3] Rim strength
    float rimWidth      = fetchMaterialFloat(material, 2);                                                           //floats[2] Rim width
    
    vec4 projPos =  CAMERA.proj * CAMERA.view * vec4(POSITION.xyz, 1.0);
    vec2 projCoord = projPos.xy / projPos.w;    //[-1, 1]
    projCoord = projCoord * 0.5 + 0.5;          //[0, 1]

    float depth = texture(DEPTH_SAMPLER, projCoord).r;  					                //屏幕空间深度  
    depth = Linear01Depth(depth, CAMERA.near_far.x,CAMERA.near_far.y);  

    vec4 offsetWorldPos = vec4(POSITION.xyz, 1.0) + rimWidth * vec4(normalize(NORMAL), 0.0);
    vec4 offsetProjPos =  CAMERA.proj * CAMERA.view * vec4(offsetWorldPos.xyz, 1.0);
    vec2 offsetProjCoord = offsetProjPos.xy / offsetProjPos.w;          //[-1, 1]
    offsetProjCoord = offsetProjCoord * 0.5 + 0.5;                      //[0, 1] 

    float offsetDepth = texture(DEPTH_SAMPLER, offsetProjCoord).r;  					    //法线偏移屏幕空间深度   
    offsetDepth = Linear01Depth(offsetDepth, CAMERA.near_far.x,CAMERA.near_far.y);  

    float rim = step(rimThreshold, offsetDepth - depth);    //{0, 1}

    float fresnelPower = 6;
    float fresnelClamp = 0.8;
    float fresnel = 1.0 - clamp(dot(V, N), 0.0, 1.0);
    fresnel = pow(fresnel, fresnelPower);
    fresnel = fresnel * fresnelClamp + (1 - fresnelClamp);

    rim *= fresnel;     //[0, 1] 

    vec3 outRimColor = rimColor.rgb * rimStrength * albedo.rgb;

	//平行光源计算////////////////////////////////////////////////////////////////////////////////////////////////////////
    vec3 dirColor = vec3(0.0f);
    {
        DirectionalLight dirLight = LIGHT.directionalLights[0];

        vec3 L = -normalize(dirLight.dir);    

        float dirShadow = directionalShadow(worldPos);

        //NPR光照
        float aoShadow = ao * 2;    //AO阴影,   
        float lambert = max(0.0, (dot(N,L))); 
        lambert *= dirShadow;
        float halfLambert = max(0.0, min(1.0, (lambert * 0.5 + 0.5) * aoShadow));
        halfLambert = smoothstep(0.0, lambertClamp, halfLambert);

        //ramp采样
        float rampX = halfLambert;
        float rampY = (1.0f / rampTexSize.y) * ((1 - materialType) * 4 + 0.5 + rampTexOffset);    

        vec4 ramp = fetchMaterialTex2D(material, 1, vec2(rampX, rampY)).rgba;  				
        ramp = pow(ramp, vec4(1.0/2.2));    


        vec3 surfaceColor = ramp.rgb * albedo.rgb;
        //vec3 surfaceColor = ramp.rgb;
        //vec3 surfaceColor = vec3(halfLambert);
        //vec3 surfaceColor = halfLambert * vec3(1.0);
        dirColor += max(vec3(0.0f), surfaceColor);	
    }
    






    
    vec4 outColor = vec4(1.0f);
    outColor = albedo;
    outColor = vec4(vec3(lightMap.g), 1.0f);
    outColor = vec4(dirColor, 1.0f);
    //outColor = vec4(rampCoord * rampTexSize, 0.0f, 1.0f);
    //outColor = vec4((vec3(1.0) + V) / 2, 1.0f);
    //outColor = vec4((vec3(1.0) + N) / 2, 1.0f);
    //outColor = vec4(vec3(ao), 1.0f);
    //outColor = vec4(vec3(0.5f), 1.0);

    //outColor *= depth;
    outColor = vec4(outRimColor * rim + dirColor * (1.0 - rim), 1.0);





    // float cluster = COLOR.x;
    // if(rampTexOffset > 1.0f) cluster = COLOR.y;

    // float r =  Random(vec2(cluster, cluster));
    // float g =  Random(vec2(cluster + 0.1f, cluster + 0.1f));
    // float b =  Random(vec2(cluster + 0.2f, cluster + 0.2f));
    // outColor = vec4(vec3(r, g, b), 1.0f);
    color = outColor;
}


