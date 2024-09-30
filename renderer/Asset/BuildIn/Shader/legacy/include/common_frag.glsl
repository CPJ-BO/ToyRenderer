#ifndef COMMON_FRAG_GLSL
#define COMMON_FRAG_GLSL


void alphaClip(MaterialInfo material, float alpha)
{
    if(material.alpha_clip > alpha) discard; 
}

vec4 writeVelocity(vec4 pos, vec4 prevPos)
{
    vec4 clipPos            = CAMERA.proj * CAMERA.view * pos;
    clipPos                 /= clipPos.w;
    vec2 coordPos           = clipPos.xy * 0.5 + vec2(0.5);

    vec4 prevClipPos        = CAMERA.prev_proj * CAMERA.prev_view * prevPos;
    prevClipPos             /= prevClipPos.w;
    vec2 prevCoordPos       = prevClipPos.xy * 0.5 + vec2(0.5);

    vec4 velocity             = vec4(vec2(coordPos.xy - prevCoordPos.xy), 0.0f, 0.0f);
    //velocity             = vec4(vec2(coordPos.xy - prevCoordPos.xy), coordPos.xy);

    //vec4 velocity             = vec4(vec3(clipPos.xyz - prevClipPos.xyz), 0.0f);
    //velocity.xy          -= (CAMERA.prev_jitter.xy - CAMERA.jitter.xy);
    //velocity.z          = 0.0f;

    //速度坐标方向：x正方向 y负方向

    //velocity = vec4(coordPos, 0.0f, 0.0f);

    return velocity;
}

#endif