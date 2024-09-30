#version 460

layout (set = 0, binding = 0) uniform sampler2D in_px;
layout (set = 0, binding = 1) uniform sampler2D in_nx;
layout (set = 0, binding = 2) uniform sampler2D in_py;
layout (set = 0, binding = 3) uniform sampler2D in_ny;
layout (set = 0, binding = 4) uniform sampler2D in_pz;
layout (set = 0, binding = 5) uniform sampler2D in_nz;


layout(location = 0) out vec4 out_px;
layout(location = 1) out vec4 out_nx;
layout(location = 2) out vec4 out_py;
layout(location = 3) out vec4 out_ny;
layout(location = 4) out vec4 out_pz;
layout(location = 5) out vec4 out_nz;

layout (location = 0) in vec2 inUV;

layout(push_constant) uniform round_setting {
    int index;
}ROUND_SETTING;


vec4 kawase(sampler2D tex, vec2 uv, vec2 texelSize, int pixelOffset)
{
    vec4 o = vec4(0);
    o += texture(tex, uv + vec2(pixelOffset +0.5, pixelOffset +0.5) * texelSize); 
    o += texture(tex, uv + vec2(-pixelOffset -0.5, pixelOffset +0.5) * texelSize); 
    o += texture(tex, uv + vec2(-pixelOffset -0.5, -pixelOffset -0.5) * texelSize); 
    o += texture(tex, uv + vec2(pixelOffset +0.5, -pixelOffset -0.5) * texelSize); 
    return o * 0.25;
}

void main() 
{	
    vec2 size = vec2(1) / textureSize(in_px, 0);

    out_px = kawase(in_px, inUV, size, ROUND_SETTING.index);
    out_nx = kawase(in_nx, inUV, size, ROUND_SETTING.index);
    out_py = kawase(in_py, inUV, size, ROUND_SETTING.index);
    out_ny = kawase(in_ny, inUV, size, ROUND_SETTING.index);
    out_pz = kawase(in_pz, inUV, size, ROUND_SETTING.index);
    out_nz = kawase(in_nz, inUV, size, ROUND_SETTING.index);

    //out_px = texture(in_px, inUV);
    //out_nx = texture(in_nx, inUV);
    //out_py = texture(in_py, inUV);
    //out_ny = texture(in_ny, inUV);
    //out_pz = texture(in_pz, inUV);
    //out_nz = texture(in_nz, inUV);
}