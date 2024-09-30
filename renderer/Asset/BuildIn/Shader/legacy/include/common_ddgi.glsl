#ifndef COMMON_DDGI_GLSL
#define COMMON_DDGI_GLSL


struct DDGIPayload
{
    float dist;
    vec4 normal;
    vec4 albedo;
    vec4 emission;
};


#define M_PI 3.14159265359
#define MAX_HIT_DIST 10000.0    //最大命中距离

// 相对坐标：probe小纹理内的坐标
// 绝对坐标：大纹理整体的坐标

// 一维下标到三维下标索引
ivec3 fetchProbeCoord(in DDGISetting ddgi, int probe_index)
{
    ivec3 i_pos;

    i_pos.x = probe_index % ddgi.probe_counts.x;
    i_pos.y = (probe_index % (ddgi.probe_counts.x * ddgi.probe_counts.y)) / ddgi.probe_counts.x;
    i_pos.z = probe_index / (ddgi.probe_counts.x * ddgi.probe_counts.y);

    return i_pos;
}

// 三维下标索引到世界空间坐标
vec3 fetchProbeWorldPos(in DDGISetting ddgi, ivec3 probe_coord)
{
    return ddgi.grid_step * vec3(probe_coord) + ddgi.grid_start_position;
}

// 一维下标标索到世界空间坐标
vec3 fetchProbeWorldPos(in DDGISetting ddgi, int probe_index)
{
    ivec3 probe_coord = fetchProbeCoord(ddgi, probe_index);

    return fetchProbeWorldPos(ddgi, probe_coord);
}

// 世界空间到三维下标索引，下取整
ivec3 fetchBaseProbeCoord(in DDGISetting ddgi, vec3 worldPos) 
{
    return clamp(ivec3((worldPos - ddgi.grid_start_position) / ddgi.grid_step), ivec3(0, 0, 0), ivec3(ddgi.probe_counts) - ivec3(1, 1, 1));
}

// 三维下标到一维下标索引
int fetchPorbeIndex(in DDGISetting ddgi, ivec3 probe_coords) 
{
    return int(probe_coords.x + probe_coords.y * ddgi.probe_counts.x + probe_coords.z * ddgi.probe_counts.x * ddgi.probe_counts.y);
}

// 绝对像素坐标到一维下标索引
int fetchPorbeIndex(vec2 tex_coord, int full_texture_width, int probe_side_length)
{
    int probe_with_border_side = probe_side_length + 2;
    int probes_per_side        = (full_texture_width - 2) / probe_with_border_side;                                 //每行探针数目
    return int(tex_coord.x / probe_with_border_side) + probes_per_side * int(tex_coord.y / probe_with_border_side); //一维偏移
}

// 绝对/相对像素坐标到相对UV坐标
vec2 fetchProbeRelativeUV(ivec2 tex_coord, int probe_side_length)
{ 
    int probe_with_border_side = probe_side_length + 2;

    vec2 oct_frag_coord = ivec2((tex_coord.x - 2) % probe_with_border_side, (tex_coord.y - 2) % probe_with_border_side);    //绝对/相对像素坐标到相对像素坐标
                
    return (vec2(oct_frag_coord) + vec2(0.5f)) * (2.0f / float(probe_side_length)) - vec2(1.0f, 1.0f);                      //相对像素坐标到相对UV坐标，加了半像素到像素中心(必须加)
                                                                                                                            //= normalized_oct_coord
}

// 根据法线及探针三维下标索引获取绝对纹理坐标
vec2 fetchProbeAbsTexCoord(vec3 dir, int probe_index, int full_texture_width, int full_texture_height, int probe_side_length) 
{
    vec2 normalized_oct_coord = Oct_Encode(normalize(dir));
    vec2 normalized_oct_coord_zero_one = (normalized_oct_coord + vec2(1.0f)) * 0.5f;    // 八面体映射的相对像素坐标

    float probe_with_border_side = probe_side_length + 2.0f;                    // 单个probe纹理的像素尺寸 + 2像素padding
                                                                                // 单个probe左上1像素padding，整个纹理左上1像素padding

    vec2 oct_coord_normalized_to_texture_dimensions = 
        vec2(normalized_oct_coord_zero_one * probe_side_length) / 
        vec2(full_texture_width, full_texture_height);                          // 相对像素坐标到绝对UV坐标

    int probes_per_row = (full_texture_width - 2) / int(probe_with_border_side);

    vec2 probe_top_left_position = vec2(	mod(probe_index, probes_per_row) * probe_with_border_side,
                                        	(probe_index / probes_per_row) * probe_with_border_side) +
                                   vec2(2.0f, 2.0f);                                    // 小纹理左上在整个纹理的像素坐标，包括了padding
                                                                                        // 小纹理按一维索引行优先排列

    vec2 normalized_probe_top_left_position = vec2(probe_top_left_position) / vec2(full_texture_width, full_texture_height);  //

    return vec2(normalized_probe_top_left_position + oct_coord_normalized_to_texture_dimensions);   //左上UV偏移 + 小纹理内UV偏移
}






// 获取辐照度
vec3 fetchIrradiance(
    in DDGISetting ddgi, 
    vec3 worldPos, vec3 normal, vec3 viewVec, 
    sampler2D irradiance_texture, 
    sampler2D depth_texture)
{
    ivec3 base_grid_coord = fetchBaseProbeCoord(ddgi, worldPos);            //下取整的探针索引
    vec3 base_probePos = fetchProbeWorldPos(ddgi, base_grid_coord);    //探针坐标
    
    vec3  sum_irradiance = vec3(0.0f);
    float sum_weight = 0.0f;

    vec3 alpha = clamp((worldPos - base_probePos) / ddgi.grid_step, vec3(0.0f), vec3(1.0f));   //距下取整坐标的三维重心坐标

    for (int i = 0; i < 8; ++i) 
    {
        //对包围的八个探针中的辐照度进行加权
        //权重包括三部分：
        //1. 三线性插值系数，着色点距离探针越远，权重越低
        //2. 方向系数，着色点到探针的表面法线越大，权重越低
        //3. 切比雪夫系数，着色点到探针之间有遮挡物的概率越大，权重越低

        float weight = 1.0;

        ivec3  offset           = ivec3(i, i >> 1, i >> 2) & ivec3(1);
        ivec3  probe_grid_coord = clamp(base_grid_coord + offset, ivec3(0), ddgi.probe_counts - ivec3(1));  //循环测试的探针索引
        int probeIdx            = fetchPorbeIndex(ddgi, probe_grid_coord);
        vec3 probePos           = fetchProbeWorldPos(ddgi, probe_grid_coord);

        //2. 方向系数
        {
            vec3 direction_to_probe = normalize(probePos - worldPos);
            //weight *= max(0.0001, dot(direction_to_probe, normal));    //
            weight *= Square(max(0.0001, (dot(direction_to_probe, normal) + 1.0) * 0.5)) + 0.2;  
        }

        //3. 切比雪夫系数
        if (ddgi.visibility_test)
        {
            vec3 bias           = (normal + 3.0 * viewVec) * ddgi.normal_bias;
            vec3 probeToPoint   = (worldPos - probePos) + bias;
            vec3 dir            = normalize(-probeToPoint);
            float dist          = length(probeToPoint);
            vec2 texCoord       = fetchProbeAbsTexCoord(-dir, probeIdx, ddgi.depth_texture_width, ddgi.depth_texture_height, DDGI_DEPTH_PROBE_SIZE);


            vec2 temp       = texture(depth_texture, texCoord).rg;  //采样距离和距离平方
            float mean      = temp.x;
            float variance  = abs(Square(temp.x) - temp.y);

            float chebyshev = variance / (variance + Square(max(dist - mean, 0.0)));
            chebyshev       = max(Pow3(chebyshev), 0.0);  //以切比雪夫系数三次方作为权重

            weight *= (dist <= mean) ? 1.0 : chebyshev;
        }

        //避免计算精度问题
        {
            weight = max(0.000001, weight); 

            const float crush_threshold = 0.2f;
            if (weight < crush_threshold)
                weight *= weight * weight * (1.0f / Square(crush_threshold)); 
        }


        //1. 三线性插值系数
        {
            vec3 trilinear = mix(1.0 - alpha, alpha, offset);
            weight *= trilinear.x * trilinear.y * trilinear.z;  
        }

        //采样，累计光照      
        {
            vec3 irradiance_dir     = normal;
            vec2 texCoord           = fetchProbeAbsTexCoord(normalize(irradiance_dir), probeIdx, ddgi.irradiance_texture_width, ddgi.irradiance_texture_height, DDGI_IRRADIANCE_PROBE_SIZE);
            vec3 probe_irradiance   = texture(irradiance_texture, texCoord).rgb;
        
            sum_irradiance += weight * probe_irradiance;
            sum_weight += weight;
        }
    }

    vec3 net_irradiance = sum_irradiance / sum_weight;
    net_irradiance *= ddgi.energy_preservation;

    return 0.5f * M_PI * net_irradiance;   
}

#endif