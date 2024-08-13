#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 color;

layout(push_constant) uniform enable_setting {
    bool enable;
}ENABLE_SETTING;

void main() 
{
    ivec2 pixPos        = HALF_SIZE_SSSR ? 
                            ivec2(fetchScreenPixPos(inUV)) / 2 :
                            ivec2(fetchScreenPixPos(inUV));
    vec4 ssrColor       = ENABLE_SETTING.enable ? imageLoad(STOCHASTIC_SSR, pixPos) : vec4(0.0f);
    vec4 screenColor    = texture(COLOR_0_SAMPLER, inUV);

    color               = vec4(screenColor.xyz + ssrColor.xyz * ssrColor.w, 1.0f);   //混合，输出
    //color               = ssrColor;
    //color               = screenColor;
}
