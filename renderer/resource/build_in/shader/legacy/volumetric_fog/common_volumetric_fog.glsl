
ivec3 fetchVolumetricClusterID(vec3 ndc)
{
    vec2 coord = ndcToScreen(ndc.xy) + 0.5 * vec2(1.0 / WINDOW_WIDTH, 1.0 / WINDOW_WIDTH);	//需要加上一点偏移

    float depth         = ndc.z;   
    float linearDepth   = LinearEyeDepth(depth, CAMERA.near_far.x, CAMERA.near_far.y);

    uint clusterZ       = uint(VOLUMETRIC_FOG_SIZE_Z * log(linearDepth / CAMERA.near_far.x) / log(CAMERA.near_far.y / CAMERA.near_far.x));  //对数划分
    //uint clusterZ     = uint( (linearDepth - CAMERA.near_far.x) / ((CAMERA.near_far.y - CAMERA.near_far.x) / VOLUMETRIC_FOG_SIZE_Z));   //均匀划分
    ivec3 clusterID     = ivec3(uint(coord.x * VOLUMETRIC_FOG_SIZE_X), uint(coord.y * VOLUMETRIC_FOG_SIZE_Y), clusterZ);

    return clusterID;
}

vec4 fetchVolumetricClusterPos(vec3 clusterID)
{
	vec3 voxelSize 			= 1.0f / vec3(VOLUMETRIC_FOG_SIZE_X, VOLUMETRIC_FOG_SIZE_Y, VOLUMETRIC_FOG_SIZE_Z);
	
	vec2 screenPos 			= clusterID.xy * voxelSize.xy;	
	vec4 viewFarPos 		= screenToView(screenPos, 1.0f);
	vec4 viewNearPos 		= screenToView(screenPos, 0.0f);

	//均匀划分
	// vec4 worldNearPos 	= vec4(CAMERA.pos.xyz + normalize(CAMERA.front) * CAMERA.near_far.x, 1.0f);	//世界空间最近处
	// vec4 worldFarPos 	= screenToWorld(screenPos, 1.0f);					//世界空间最远处
	// vec4 worldPos 		= Lerp(worldNearPos, worldFarPos, (gID.z + ditter.z) * voxelSize.z);

	// 对数划分，使用线面相交计算
	// vec4 plane;
	// plane.xyz 			= vec3(0.0f, 0.0f, -1.0f);  
	// plane.w 				= CAMERA.near_far.x * pow(CAMERA.near_far.y / CAMERA.near_far.x, (gID.z + ditter.z) * voxelSize.z);	//对数划分

	// vec3 viewPos;
	// vec3 eye 			= vec3(0, 0, 0);
	// LineIntersectPlane(eye, viewFarPos.xyz, plane, viewPos);

	// 对数划分,也可以这样算
	float viewDepth 		= CAMERA.near_far.x * pow((CAMERA.near_far.y / CAMERA.near_far.x), clusterID.z * voxelSize.z);	
	vec3 viewPos 			= (viewFarPos * (viewDepth / abs(viewFarPos.z))).xyz;

	return viewToWorld(vec4(viewPos, 1.0f));	
}


