#ifndef DDGI_LAYOUT_GLSL
#define DDGI_LAYOUT_GLSL


layout(set = 1, binding = 0, rgba16f) uniform image2D G_BUFFER_POS_TEXTURE;	        //世界坐标纹理
layout(set = 1, binding = 1, rgba16f) uniform image2D G_BUFFER_NORMAL_TEXTURE;	    //法线纹理
layout(set = 1, binding = 2, rgba16f) uniform image2D G_BUFFER_ALBEDO_TEXTURE;	    //漫反射纹理
layout(set = 1, binding = 3, rgba16f) uniform image2D G_BUFFER_EMISSION_TEXTURE;	//自发光纹理
layout(set = 1, binding = 4, rgba16f) uniform image2D RADIANCE_TEXTURE;	            //辐射率纹理

layout(set = 1, binding = 5, rgba16f) uniform image2D IRRADIANCE_TEXTURE;	        //辐照度纹理
layout(set = 1, binding = 6, rg16f) uniform image2D DEPTH_TEXTURE;		            //深度纹理

#endif