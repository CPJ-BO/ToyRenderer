#ifndef COORDINATE_GLSL
#define COORDINATE_GLSL

//坐标转换////////////////////////////////////////////////////////////////////////////

//坐标系
//world space:      x正向向前，y正向向上，z正向向右
//view space:       x正向向右，y正向向上，z负向向前
//clip space:       x正向向右，y负向向上，z正向向前
//NDC space;        左上[-1, -1]，右下[1, 1]（透视除法后）
//screen space:     左上[0, 0]，右下[1, 1]

vec2 NDCToScreen(vec2 ndc)
{
    return ndc.xy * 0.5 + 0.5;
}

vec2 ScreenToNDC(vec2 screen)
{
    return screen.xy * 2.0 - 1.0;
}

vec4 NDCToView(vec4 ndc)
{	
	vec4 view = CAMERA.invProj * ndc;       // View space position.
	view /= view.w;                         // Perspective projection.

	return view;
}

vec4 ViewToClip(vec4 view)
{
    vec4 clip = CAMERA.proj * view;

    return clip;
}

vec4 ViewToNDC(vec4 view)
{
    vec4 ndc = CAMERA.proj * view;
    ndc /= ndc.w;

    return ndc;
}  

vec2 viewToScreen(vec4 view)
{
    return NDCToScreen(ViewToNDC(view).xy);
}  

vec4 ViewToWorld(vec4 view)
{
    return CAMERA.invView * view;
}  

vec4 WorldToView(vec4 world)
{
    return CAMERA.view * world;
}  

vec4 WorldToCLip(vec4 world)
{
    return ViewToClip(WorldToView(world));
}

vec4 WorldToNDC(vec4 world)
{
    return ViewToNDC(WorldToView(world));
}  

vec2 WorldToScreen(vec4 world)
{
    return viewToScreen(WorldToView(world));
}

vec4 NDCToWorld(vec4 ndc)
{	
	return ViewToWorld(NDCToView(ndc));
}

vec4 ScreenToView(vec2 coord, float depth)
{
    vec4 ndc = vec4(ScreenToNDC(coord), depth, 1.0f);  
	return NDCToView(ndc);
}

vec4 ScreenToWorld(vec2 coord, float depth)
{
    return ViewToWorld(ScreenToView(coord, depth));
}

vec4 DepthToView(vec2 coord)
{
    return ScreenToView(coord, FetchDepth(coord));
}

vec4 DepthToWorld(vec2 coord)
{
    return ViewToWorld(DepthToView(coord));
}   

#endif