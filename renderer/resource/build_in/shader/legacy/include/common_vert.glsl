#ifndef COMMON_VERT_GLSL
#define COMMON_VERT_GLSL


#define FRAMES_IN_FLIGHT 3

//object////////////////////////////////////////////////////////////////////////////

int fetchFrameIndex(Object object)
{
    return object.frame_index;
}

int fetchPrevFrameIndex(Object object)
{
    int index = object.frame_index - 1;
    index = index < 0 ? index + FRAMES_IN_FLIGHT : index;

    return index;
}

int fetchNextFrameIndex(Object object)
{
    int index = object.frame_index + 1;
    index = index >= FRAMES_IN_FLIGHT ? index - FRAMES_IN_FLIGHT : index;

    return index;
}

vec4 fetchPrevLocalPos(Object object, vec3 pos, ivec4 bone_ids, vec4 bone_weights){

    vec4 localPos = vec4(0.0f);
    vec4 basePos = vec4(pos, 1.0f);

    if(object.animationID > 0)
    {
        for(int i = 0 ; i < 4 ; i++)
        {
            if(bone_ids[i] == -1) 
                continue;
            if(bone_ids[i] >= 200) 
            {
                localPos = basePos;
                break;
            }    
            localPos += (ANIMATIONS.slot[object.animationID * FRAMES_IN_FLIGHT + fetchPrevFrameIndex(object)].transform[bone_ids[i]] * basePos) * bone_weights[i];         

        }
    }
    else
    {
        localPos = basePos;
    }

    return localPos;
}

vec4 fetchLocalPos(Object object, vec3 pos, ivec4 bone_ids, vec4 bone_weights){

    vec4 localPos = vec4(0.0f);
    vec4 basePos = vec4(pos, 1.0f);

    if(object.animationID > 0)
    {
        for(int i = 0 ; i < 4 ; i++)
        {
            if(bone_ids[i] == -1) 
                continue;
            if(bone_ids[i] >= 200) 
            {
                localPos = basePos;
                break;
            }    
            localPos += (ANIMATIONS.slot[object.animationID * FRAMES_IN_FLIGHT + fetchFrameIndex(object)].transform[bone_ids[i]] * basePos) * bone_weights[i];         

        }
    }
    else
    {
        localPos = basePos;
    }

    return localPos;
}

vec4 fetchLocalNormal(Object object, vec3 normal, ivec4 bone_ids, vec4 bone_weights){

    vec4 localNormal = vec4(0.0f);
    vec4 baseNormal = vec4(normal, 1.0f);

    if(object.animationID > 0)
    {
        for(int i = 0 ; i < 4 ; i++)
        {
            if(bone_ids[i] == -1) 
                continue;
            if(bone_ids[i] >= 200) 
            {
                localNormal = baseNormal;
                break;
            } 
            localNormal += (ANIMATIONS.slot[object.animationID * FRAMES_IN_FLIGHT + fetchFrameIndex(object)].transform[bone_ids[i]] * baseNormal) * bone_weights[i];        

        }
    }
    else
    {
        localNormal = baseNormal;
    }

    return localNormal;
}

vec4 fetchLocalTangent(Object object, vec3 tangent, ivec4 bone_ids, vec4 bone_weights){

    vec4 localTangent = vec4(0.0f);
    vec4 baseTangent = vec4(tangent, 1.0f);

    if(object.animationID > 0)
    {
        for(int i = 0 ; i < 4 ; i++)
        {
            if(bone_ids[i] == -1) 
                continue;
            if(bone_ids[i] >= 200) 
            {
                localTangent = baseTangent;
                break;
            } 
            localTangent += (ANIMATIONS.slot[object.animationID * FRAMES_IN_FLIGHT + fetchFrameIndex(object)].transform[bone_ids[i]] * baseTangent) * bone_weights[i];        

        }
    }
    else
    {
        localTangent = baseTangent;
    }

    return localTangent;
}

#endif















