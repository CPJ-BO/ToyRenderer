
#include "TangentSpace.h"
#include "Core/Mesh/Mesh.h"

TangentSpace::TangentSpace() 
{
    interface.m_getNumFaces = GetNumFaces;
    interface.m_getNumVerticesOfFace = GetNumVerticesOfFace;

    interface.m_getNormal = GetNormal;
    interface.m_getPosition = GetPosition;
    interface.m_getTexCoord = GetTexcoords;
    interface.m_setTSpaceBasic = SetTspaceBasic;

    context.m_pInterface = &interface;
}

void TangentSpace::Generate(Mesh* mesh) 
{
    context.m_pUserData = mesh;

    genTangSpaceDefault(&this->context);
    //genTangSpace(&this->context, 180.0f);
}

int TangentSpace::GetNumFaces(const SMikkTSpaceContext* context) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);
    
    return mesh->TriangleNum();
}

int TangentSpace::GetNumVerticesOfFace(const SMikkTSpaceContext* context, const int iFace) 
{
    //Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);
    
    return 3;
}

void TangentSpace::GetPosition(const SMikkTSpaceContext* context, float* outpos, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = GetVertexIndex(context, iFace, iVert);
    outpos[0] = mesh->position[index].x();
    outpos[1] = mesh->position[index].y();
    outpos[2] = mesh->position[index].z();
}

void TangentSpace::GetNormal(const SMikkTSpaceContext* context, float* outnormal, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = GetVertexIndex(context, iFace, iVert);
    outnormal[0] = mesh->normal[index].x();
    outnormal[1] = mesh->normal[index].y();
    outnormal[2] = mesh->normal[index].z();
}

void TangentSpace::GetTexcoords(const SMikkTSpaceContext* context, float* outuv, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = GetVertexIndex(context, iFace, iVert);
    outuv[0] = mesh->texCoord[index].x();
    outuv[1] = mesh->texCoord[index].y();
}

void TangentSpace::SetTspaceBasic(const SMikkTSpaceContext* context, const float* tangentu, const float fSign, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = GetVertexIndex(context, iFace, iVert);
    mesh->tangent[index](0) = tangentu[0];
    mesh->tangent[index](1) = tangentu[1];
    mesh->tangent[index](2) = tangentu[2];
    mesh->tangent[index](3) = fSign;
}

int TangentSpace::GetVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto faceSize = GetNumVerticesOfFace(context, iFace);

    auto indicesIndex = (iFace * faceSize) + iVert;

    int index = mesh->index[indicesIndex];
    return index;
}