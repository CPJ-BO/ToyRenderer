
#include "TangentSpace.h"
#include "Core/Mesh/Mesh.h"

TangentSpace::TangentSpace() {
    m_interface.m_getNumFaces = get_num_faces;
    m_interface.m_getNumVerticesOfFace = get_num_vertices_of_face;

    m_interface.m_getNormal = get_normal;
    m_interface.m_getPosition = get_position;
    m_interface.m_getTexCoord = get_tex_coords;
    m_interface.m_setTSpaceBasic = set_tspace_basic;

    context.m_pInterface = &m_interface;
}

void TangentSpace::Generate(Mesh* mesh) 
{
    context.m_pUserData = mesh;

    genTangSpaceDefault(&this->context);
    //genTangSpace(&this->context, 180.0f);
}

int TangentSpace::get_num_faces(const SMikkTSpaceContext* context) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    return mesh->TriangleNum();
}

int TangentSpace::get_num_vertices_of_face(const SMikkTSpaceContext* context, const int iFace) 
{
    //Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);
    return 3;
}

void TangentSpace::get_position(const SMikkTSpaceContext* context, float* outpos, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = get_vertex_index(context, iFace, iVert);
    outpos[0] = mesh->position[index].x();
    outpos[1] = mesh->position[index].y();
    outpos[2] = mesh->position[index].z();
}

void TangentSpace::get_normal(const SMikkTSpaceContext* context, float* outnormal, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = get_vertex_index(context, iFace, iVert);
    outnormal[0] = mesh->normal[index].x();
    outnormal[1] = mesh->normal[index].y();
    outnormal[2] = mesh->normal[index].z();
}

void TangentSpace::get_tex_coords(const SMikkTSpaceContext* context, float* outuv, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = get_vertex_index(context, iFace, iVert);
    outuv[0] = mesh->texCoord[index].x();
    outuv[1] = mesh->texCoord[index].y();
}

void TangentSpace::set_tspace_basic(const SMikkTSpaceContext* context, const float* tangentu, const float fSign, const int iFace, const int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto index = get_vertex_index(context, iFace, iVert);
    mesh->tangent[index](0) = tangentu[0];
    mesh->tangent[index](1) = tangentu[1];
    mesh->tangent[index](2) = tangentu[2];
    mesh->tangent[index](3) = fSign;
}

int TangentSpace::get_vertex_index(const SMikkTSpaceContext* context, int iFace, int iVert) 
{
    Mesh* mesh = static_cast<Mesh*> (context->m_pUserData);

    auto face_size = get_num_vertices_of_face(context, iFace);

    auto indices_index = (iFace * face_size) + iVert;

    int index = mesh->index[indices_index];
    return index;
}