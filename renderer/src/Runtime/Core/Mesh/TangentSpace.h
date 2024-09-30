#pragma once

#include <mikktspace.h>

class Mesh;

class TangentSpace {

public:
    TangentSpace();
    void Generate(Mesh* mesh);

private:
    SMikkTSpaceInterface interface = {};
    SMikkTSpaceContext context = {};

    static int GetVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert);

    static int GetNumFaces(const SMikkTSpaceContext* context);
    static int GetNumVerticesOfFace(const SMikkTSpaceContext* context, int iFace);
    static void GetPosition(const SMikkTSpaceContext* context, float outpos[],
        int iFace, int iVert);

    static void GetNormal(const SMikkTSpaceContext* context, float outnormal[],
        int iFace, int iVert);

    static void GetTexcoords(const SMikkTSpaceContext* context, float outuv[],
        int iFace, int iVert);

    static void SetTspaceBasic(const SMikkTSpaceContext* context,
        const float tangentu[],
        float fSign, int iFace, int iVert);

};