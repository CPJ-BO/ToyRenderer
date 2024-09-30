#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout(set = 1, binding = 0) uniform sampler2D CURRENT;
layout(set = 1, binding = 1) uniform sampler2D HISTORY;

layout(push_constant) uniform TAASetting {
    float enable;
    float sharpen;
    float reprojection_only;
    float show_velocity;
    float velocity_factor;
    float blend_factor;
    
} setting;

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 IN_TEXCOORD;

vec3 find_closest_fragment_3x3(vec2 uv, vec2 texSize, sampler2D depth)
{
    vec2 dd = abs(texSize.xy);
    vec2 du = vec2(dd.x, 0.0);
    vec2 dv = vec2(0.0, dd.y);

    vec3 dtl = vec3(-1, -1, texture(depth, uv - dv - du).x);
    vec3 dtc = vec3(0, -1, texture(depth, uv - dv).x);
    vec3 dtr = vec3(1, -1, texture(depth, uv - dv + du).x);

    vec3 dml = vec3(-1, 0, texture(depth, uv - du).x);
    vec3 dmc = vec3(0, 0, texture(depth, uv).x);
    vec3 dmr = vec3(1, 0, texture(depth, uv + du).x);

    vec3 dbl = vec3(-1, 1, texture(depth, uv + dv - du).x);
    vec3 dbc = vec3(0, 1, texture(depth, uv + dv).x);
    vec3 dbr = vec3(1, 1, texture(depth, uv + dv + du).x);

    vec3 dmin = dtl;
    if (dmin.z > dtc.z) dmin = dtc;
    if (dmin.z > dtr.z) dmin = dtr;

    if (dmin.z > dml.z) dmin = dml;
    if (dmin.z > dmc.z) dmin = dmc;
    if (dmin.z > dmr.z) dmin = dmr;

    if (dmin.z > dbl.z) dmin = dbl;
    if (dmin.z > dbc.z) dmin = dbc;
    if (dmin.z > dbr.z) dmin = dbr;

    return vec3(uv + dd.xy * dmin.xy, dmin.z);
}



void main() 
{	
    vec2 currCoord          = IN_TEXCOORD;

    //vec3 colseCoord         = find_closest_fragment_3x3(currCoord, textureSize(DEPTH_SAMPLER, 0), DEPTH_SAMPLER);

    vec2 velocity           = texture(VELOCITY_SAMPLER, currCoord.xy).xy;
    vec2 prevCoord          = currCoord - velocity;


    //重投影
    if(setting.reprojection_only > 0.0f)
    {
        vec3 worldPos       = depthToWorld(IN_TEXCOORD).xyz;                                    //本帧的世界空间坐标（假定物体不动，也就是前一帧的）
        vec4 prevClipPos    = CAMERA.prev_proj * CAMERA.prev_view * vec4(worldPos, 1.0f);       //前帧的裁剪空间坐标
        prevCoord           = (prevClipPos.xy / prevClipPos.w) * 0.5 + vec2(0.5);               //前帧的UV

        // float prevDepth     = Linear01Depth(prevClipPos.z / prevClipPos.w, CAMERA.near_far[0], CAMERA.near_far[1]);
        // float currDepth     = Linear01Depth(fetchDepth(currCoord), CAMERA.near_far[0], CAMERA.near_far[1]);
        // float deltaDepth    = abs(prevDepth - currDepth);
    }


    AdjacentTex3 currentAdj  = textureAdj3(CURRENT, currCoord, 0);
    vec3 currColor          = currentAdj.color[1][1];
    RGBtoYCoCg(currentAdj);

    vec3 historyColor       = RGBtoYCoCg(texture(HISTORY, prevCoord).rgb);
    historyColor            = YCoCgtoRGB(clampedColor(historyColor, currentAdj));


    //锐化
    if (setting.enable > 0.0f &&
        setting.sharpen > 0.0f)
    {
        vec3 sum = vec3(0.0);

        sum += -1.0 *   currentAdj.color[0][1];
        sum += -1.0 *   currentAdj.color[1][0];
        sum += 5.0 *    currentAdj.color[1][1];
        sum += -1.0 *   currentAdj.color[1][2];
        sum += -1.0 *   currentAdj.color[2][1];

        currColor = max(YCoCgtoRGB(sum), vec3(0.0f));
    }

    //色调映射
    historyColor        = ToneMap(max(historyColor, 0.0f));
    currColor           = ToneMap(max(currColor, 0.0f));

    vec3 finalColor     = Lerp(historyColor, currColor, setting.blend_factor);
    finalColor          = InverseToneMap(finalColor);
    currColor           = InverseToneMap(currColor);
    historyColor        = InverseToneMap(historyColor);

    if(setting.enable > 0.0f)
    {
        color.rgb       = finalColor;
        color.a         = 1.0f;
    }
    else
    {
        color.rgb       = currColor;
        color.a         = 1.0f;
    }

    if(setting.show_velocity > 0.0f)
    {
        color.rg       = velocity * setting.velocity_factor;
        color.ba       = vec2(0.0f, 1.0f);
    }
}