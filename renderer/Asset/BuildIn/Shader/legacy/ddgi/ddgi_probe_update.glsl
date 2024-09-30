

layout(push_constant) uniform ComputeSetting {
	int volume_light_id;
	float _padding[3];
} computeSetting;

#if defined(DEPTH_PROBE)
    #define PROBE_SIZE DDGI_DEPTH_PROBE_SIZE
    #define THREADS_X DDGI_DEPTH_PROBE_SIZE
    #define THREADS_Y DDGI_DEPTH_PROBE_SIZE
    #define THREADS_Z 1	
    #define TEXTURE_WIDTH volumeLight.setting.depth_texture_width
    #define TEXTURE_HEIGHT volumeLight.setting.depth_texture_height
    #define TEXTURE DEPTH_TEXTURE
#else
    #define PROBE_SIZE DDGI_IRRADIANCE_PROBE_SIZE
    #define THREADS_X DDGI_IRRADIANCE_PROBE_SIZE
    #define THREADS_Y DDGI_IRRADIANCE_PROBE_SIZE
    #define THREADS_Z 1	
    #define TEXTURE_WIDTH volumeLight.setting.irradiance_texture_width
    #define TEXTURE_HEIGHT volumeLight.setting.irradiance_texture_height
    #define TEXTURE IRRADIANCE_TEXTURE
#endif


#define CACHE_SIZE 64
shared vec4 shared_direction_depth[CACHE_SIZE];
#if !defined(DEPTH_PROBE) 
shared vec3 shared_radiance[CACHE_SIZE];
#endif


void loadCache(int probe_index, uint offset, uint num_rays) //每个线程负责一条光线信息的获取
{
    if (gl_LocalInvocationIndex < num_rays)
    {
        ivec2 tex_coord = ivec2(offset + uint(gl_LocalInvocationIndex), probe_index);

        shared_direction_depth[gl_LocalInvocationIndex] = imageLoad(G_BUFFER_POS_TEXTURE, tex_coord);
    #if !defined(DEPTH_PROBE) 
        shared_radiance[gl_LocalInvocationIndex] = imageLoad(RADIANCE_TEXTURE, tex_coord).rgb;    
    #endif 
    }
}

const float ENERGY_CONSERVATION = 0.95;         //守恒衰减，避免光爆掉
void probeBlend(in DDGISetting ddgi, ivec2 current_coord, uint num_rays, inout vec3 result, inout float total_weight)    //计算辐照度
{
    for (int i = 0; i < num_rays; i++)
    {
        vec4 dir_dist           = shared_direction_depth[i];
        vec3 ray_direction      = dir_dist.xyz;
        float dist              = min(dir_dist.w, ddgi.max_probe_distance); //限制最大深度为探针有效范围

        vec3 texel_direction    = Oct_Decode(fetchProbeRelativeUV(current_coord, PROBE_SIZE));

#if !defined(DEPTH_PROBE) 
        vec3 radiance       = shared_radiance[i] * ENERGY_CONSERVATION;   
        float weight        = max(0.0, dot(texel_direction, ray_direction));    //权重为探针像素法向量和辐射率方向向量的余弦,IBL里用的是采样数
        if(weight >= FLT_EPS) 
        {
            result          += vec3(radiance * weight);                         //辐射率加权
            total_weight    += weight;
        }
#else
        float weight        = pow(max(0.0, dot(texel_direction, ray_direction)), ddgi.depth_sharpness);
        if(weight >= FLT_EPS) 
        {
            result          += vec3(dist * weight, Square(dist) * weight, 0.0); //切比雪夫加权
            total_weight    += weight;
        }
#endif
    }
}

layout (local_size_x = THREADS_X, local_size_y = THREADS_Y, local_size_z = THREADS_Z) in;
void main() 
{
    VolumeLight volumeLight = LIGHT.volumeLights[computeSetting.volume_light_id];
    
	// 第二轮 计算辐照度
    ivec2 current_coord = ivec2(gl_GlobalInvocationID.xy) + (ivec2(gl_WorkGroupID.xy) * ivec2(2)) + ivec2(2);  //水平放x，y两维；垂直放z一维

    int   probe_index = fetchPorbeIndex(current_coord, TEXTURE_WIDTH, PROBE_SIZE);
    ivec3 probe_coord = fetchProbeCoord(volumeLight.setting, probe_index);


    vec3 result         = vec3(0.0f);
    float weight_sum    = 0.0f;
    uint remaining_rays = volumeLight.setting.rays_per_probe;
    uint offset         = 0;

    //probe上的各个像素共享所有的光线
    //以CACHE_SIZE为组共享内存，加载光线信息并计算，循环多次直至消耗所有光线
    while (remaining_rays > 0)  
    {
        uint num_rays = min(CACHE_SIZE, remaining_rays);

        loadCache(probe_index, offset, num_rays);
        barrier();

        probeBlend(volumeLight.setting, current_coord, num_rays, result, weight_sum);
        barrier();

        remaining_rays -= num_rays;
        offset += num_rays;
    }
    if (weight_sum > FLT_EPS) result /= weight_sum;

    //TODO 在这里做时域加权混合
    //result = mix(result, imageLoad(TEXTURE, current_coord).rgb, 0.9f);

    imageStore(TEXTURE, current_coord, vec4( result, 0.0 ));


    //vec3 texel_direction = Oct_Decode(fetchProbeRelativeUV(current_coord, PROBE_SIZE));
    //imageStore(TEXTURE, current_coord, vec4( texel_direction, 0.0 ));

}



