#ifndef SCREEN_GLSL
#define SCREEN_GLSL


vec2 ScreenPixToUV(ivec2 pixel, ivec2 totalPixels)  //左上角[0, 0]，像素中心UV实测需要偏移0.5
{
    return (pixel + vec2(0.5f)) / vec2(totalPixels);
}

#ifdef COMMON_GLSL

vec2 ScreenPixToUV(ivec2 pixel)  //左上角[0, 0]，像素中心UV实测需要偏移0.5
{
    return ScreenPixToUV(pixel, ivec2(WINDOW_WIDTH, WINDOW_HEIGHT));
}

vec2 FetchScreenPixPos(vec2 coord)
{
	return vec2(coord.x * WINDOW_WIDTH, coord.y * WINDOW_HEIGHT);
}

vec2 FetchScreenPixPos(vec3 ndc)
{
    return FetchScreenPixPos(NDCToScreen(ndc.xy));
}

vec2 FetchHalfScreenPixPos(vec2 coord)
{
	return vec2(coord.x * HALF_WINDOW_WIDTH, coord.y * HALF_WINDOW_HEIGHT);
}

vec2 FetchHalfScreenPixPos(vec3 ndc)
{
    return FetchHalfScreenPixPos(NDCToScreen(ndc.xy));
}

#endif

#endif