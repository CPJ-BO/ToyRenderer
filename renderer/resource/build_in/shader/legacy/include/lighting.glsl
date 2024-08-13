#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL


//计算光照效果/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DirectLightDebug {
    float shadow;
    float clusterLightCount;
    ivec3 clusters;
    vec3 clusterOffset;
};

vec3 directLight(
    vec3 albedo, vec3 F0, float roughness, float metallic,
    vec4 worldPos, vec3 N, vec3 V,
    bool diffuseOnly, bool lowShadowQuality,
    out DirectLightDebug debug)
{
    float shadow              = 0.0f;

    //平行光源计算////////////////////////////////////////////////////////////////////////////////////////////////////////

    vec3 dirColor = vec3(0.0f);
    {
        DirectionalLight dirLight = LIGHT.directionalLights[0];

        vec3 L = -normalize(dirLight.dir);    
        float NoL = dot(N, L);

        float dirShadow     = directionalShadow(worldPos.xyz, lowShadowQuality);                    

        vec3 radiance       = dirShadow * 
                              dirLight.color_intencity.a * 
                              dirLight.color_intencity.rgb * 
                              max(0, NoL);                                                                          //辐射率 = 阴影值 * 光强 * 颜色 * 余弦

        vec3 f_r;
        if(diffuseOnly)     f_r = Basic_BRDF_Diffuse(albedo.rgb, F0, roughness, metallic, N, V, L);
        else                f_r = Basic_BRDF(albedo.rgb, F0, roughness, metallic, N, V, L);

        vec3 surfaceColor   = f_r * radiance;

        dirColor += max(vec3(0.0f), surfaceColor);	
        shadow += dirShadow;
    }
    
	//点光源计算////////////////////////////////////////////////////////////////////////////////////////////////////////
	float clusterLightCount     = 0;
    ivec3 clusters              = ivec3(0);
    vec3 clusterOffset          = vec3(0.0f);
    vec3 pointColor             = vec3(0.0f);
    {
        clusters = fetchLightClusterID(worldToNdc(worldPos).xyz);
        uvec2 clusterIndex  = fetchLightClusterIndex(clusters);  //读取对应cluster的索引信息

        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        for(uint i = clusterIndex.x; i < clusterIndex.x + clusterIndex.y; i++)
        {
            PointLight pointLight = LIGHT.pointLights[LIGHT_INDEX.slot[i].index];

            float pointDistance = length(worldPos.xyz - pointLight.pos);

            vec3 L = normalize(pointLight.pos - worldPos.xyz);
            float NoL = dot(N, L);

            float pShadow       = pointShadow(pointLight, worldPos.xyz, lowShadowQuality);  
            float attenuation   = pointLightFalloff(pointDistance, pointLight.near_far_bias[1]);

            vec3 radiance = pShadow * 
                            pointLight.color_intencity.a * 
                            pointLight.color_intencity.rgb *
                            attenuation * 
                            max(0, NoL);    //辐射率 = 阴影值 * 光强 * 颜色 * 衰减 * 余弦

            vec3 f_r;
            if(diffuseOnly)     f_r = Basic_BRDF_Diffuse(albedo.rgb, F0, roughness, metallic, N, V, L);
            else                f_r = Basic_BRDF(albedo.rgb, F0, roughness, metallic, N, V, L);

            vec3 surfaceColor   = f_r * radiance;

            pointColor          += max(vec3(0.0f), surfaceColor);	
            shadow              += pShadow * attenuation;
            clusterLightCount++;
        }     
    }
    clusterLightCount   = clusterLightCount / MAX_LIGHTS_PER_CLUSTER;
    clusterOffset       = vec3(clusters) / vec3(WINDOW_WIDTH / TILE_BLOCK_PIX_SIZE, WINDOW_HEIGHT / TILE_BLOCK_PIX_SIZE, TILE_DEPTH_SIZE);


    //输出////////////////////////////////////////////////////////////////////////////////////////////////////////
    debug.shadow            = shadow;
    debug.clusterLightCount = clusterLightCount;
    debug.clusters          = clusters;
    debug.clusterOffset     = clusterOffset;

    return dirColor + pointColor;
}

vec3 directLight(
    vec3 albedo, vec3 F0, float roughness, float metallic,
    vec4 worldPos, vec3 N, vec3 V,
    bool diffuseOnly, bool lowShadowQuality)
{
    DirectLightDebug debug;

    return directLight(
            albedo, F0, roughness, metallic,
            worldPos, N, V,
            diffuseOnly, lowShadowQuality,
            debug);
}

struct IndirectLightDebug {
    vec3 indirectIrradiance;
};

vec3 indirectLight(
    vec3 albedo, vec3 F0, float roughness, float metallic,
    vec4 worldPos, vec3 N, vec3 V,
    out IndirectLightDebug debug)
{
    debug.indirectIrradiance = vec3(0.0f);


    //DDGI间接光照计算////////////////////////////////////////////////////////////////////////////////////////////////////////
    vec3 indirectColor = vec3(0.0f);            //TODO间接光照的计算会导致0.5ms左右的开销？
    vec3 indirectIrradiance = vec3(0.0f);
    for(int i = 0; i < LIGHT.lightSetting.volume_light_cnt; i++)
    {
        VolumeLight volumeLight = LIGHT.volumeLights[i];
        if(volumeLight.setting.enable &&
           PointIntersectBox(worldPos.xyz, volumeLight.setting.box))    //需要在包围盒范围内
        {
            float NoV = clamp(dot(N, V), 0.00001, 0.99999);

            vec3 kS = FresnelSchlickRoughness(NoV, F0, roughness); 
            vec3 kD = 1.0 - kS;     //在BRDF积分项里每个radiance对应的kD是不同的（kS = F 视角相关），把kD移出积分是一个近似估计
                                    //辐照度里已经预先带了1/PI，无需再乘

            indirectIrradiance  = fetchIrradiance(volumeLight.setting, worldPos.xyz, N, V, VOLUME_LIGHT_IRRADIANCE_SAMPLERS[i], VOLUME_LIGHT_DEPTH_SAMPLERS[i]);
            vec3 diffuse = indirectIrradiance * albedo.rgb;
            vec3 diffuseColor = kD * diffuse;    

            indirectColor = diffuseColor;

            break;  //暂时只用一个的，没处理重叠情况
        }
    }

    //环境光照计算，已经在SSSR里完成了////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
    vec3 ambientColor = vec3(0.0);
    {
        float NoV = clamp(dot(N, V), 0.00001, 0.99999);

        vec3 kS = FresnelSchlickRoughness(NoV, F0, roughness); 
        vec3 kD = 1.0 - kS;

        //漫反射IBL
        vec3 irradiance = texture(DIFFUSE_IBL_SAMPLER, N).rgb;
        vec3 diffuse    = irradiance * albedo.rgb;
        vec3 diffuseColor = kD * diffuse; //漫反射环境光照得到的颜色

        //镜面反射IBL
        vec2 brdf = texture(BRDF_LUT_SAMPLER, vec2(NoV, roughness)).rg;
        brdf = pow(brdf, vec2(1.0/2.2));      //gamma矫正
        vec3 reflection = PreFilteredReflection(R, roughness, SPECULAR_IBL_SAMPLER).rgb;	
	    vec3 specularColor = reflection * (kS * brdf.x + brdf.y);

        ambientColor = diffuseColor + specularColor;
    }

    //对于大场景的IBL不对，暂时先用定值
    {
        float NoV = clamp(dot(N, V), 0.00001, 0.99999);

        vec3 kS = FresnelSchlickRoughness(NoV, F0, roughness); 
        vec3 kD = 1.0 - kS;

        //漫反射IBL
        vec3 irradiance  = vec3(0.2);
        vec3 diffuse = irradiance * albedo.rgb;
        vec3 diffuseColor = kD * diffuse;    //漫反射环境光照得到的颜色

        ambientColor = diffuseColor * ao;
    }
    */


    //输出////////////////////////////////////////////////////////////////////////////////////////////////////////
    debug.indirectIrradiance = indirectIrradiance;

    return indirectColor;
}

vec3 indirectLight(
    vec3 albedo, vec3 F0, float roughness, float metallic,
    vec4 worldPos, vec3 N, vec3 V)
{
    IndirectLightDebug debug;

    return indirectLight(
            albedo, F0, roughness, metallic,
            worldPos, N, V,
            debug);
}





#endif


