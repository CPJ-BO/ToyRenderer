#pragma once

#include "Core/Mesh/Mesh.h"

//其他可用的几何处理库包括CGAL，libigl等，之后再试吧
#include "meshoptimizer.h"

#include <vector>

class MeshOptimizor
{
public:

	//去除冗余顶点，并重新分配index索引
	static bool RemapMesh(MeshRef mesh);	

	//优化网格，包括顶点去重，缓存优化等
	static bool OptimizeMesh(MeshRef mesh);	

	//减面，输出优化后的index索引，顶点缓冲不变
	static float SimplifyMesh(								
		MeshRef mesh,
		float threshold,
		float targetError, 
		std::vector<uint32_t>& newIndex);	

	static float SimplifyMeshSloppy(
		MeshRef mesh,
		float threshold,
		float targetError,
		std::vector<uint32_t>& newIndex);
};