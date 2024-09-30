#include "MeshOptimizor.h"
#include "Core/Log/Log.h"
#include "Core/Math/Math.h"
#include <cstdint>

bool MeshOptimizor::RemapMesh(MeshRef mesh)
{
	bool ret = false;
	uint32_t indexCount = mesh->index.size();
	uint32_t vertexCount = mesh->position.size();

	//重映射，消除冗余顶点
	uint32_t newVertexCount;
	{
		//生成重映射表
		std::vector<uint32_t> remap(indexCount); // allocate temporary memory for the remap table
		newVertexCount = meshopt_generateVertexRemap(remap.data(), mesh->index.data(), indexCount, mesh->position.data(), vertexCount, sizeof(Vec3));

		if (newVertexCount < vertexCount)
		{
			//LOG_DEBUG("OptimizeMesh: reduce vertex size from %d to %d \n", vertexCount, newVertexCount);
			ret = true;
		}

		//根据重映射表重新分配内存位置
		meshopt_remapIndexBuffer(mesh->index.data(), mesh->index.data(), indexCount, remap.data());
		if(mesh->HasPosition())     
        {
            meshopt_remapVertexBuffer(mesh->position.data(), mesh->position.data(), vertexCount, sizeof(Vec3), remap.data());
            mesh->position.resize(newVertexCount);
        }
        if(mesh->HasNormal())       
        {
            meshopt_remapVertexBuffer(mesh->normal.data(), mesh->normal.data(), vertexCount, sizeof(Vec3), remap.data());
            mesh->normal.resize(newVertexCount);
        }
        if(mesh->HasTangent()) 
        {
            meshopt_remapVertexBuffer(mesh->tangent.data(), mesh->tangent.data(), vertexCount, sizeof(Vec4), remap.data());
            mesh->tangent.resize(newVertexCount);
        }     
        if(mesh->HasTexCoord()) 
        {
            meshopt_remapVertexBuffer(mesh->texCoord.data(), mesh->texCoord.data(), vertexCount, sizeof(Vec2), remap.data());
            mesh->texCoord.resize(newVertexCount);
        }    
        if(mesh->HasColor())
        {
            meshopt_remapVertexBuffer(mesh->color.data(), mesh->color.data(), vertexCount, sizeof(Vec3), remap.data());
            mesh->color.resize(newVertexCount);
        }        
        if(mesh->HasBoneIndex()) 
        {
            meshopt_remapVertexBuffer(mesh->boneIndex.data(), mesh->boneIndex.data(), vertexCount, sizeof(IVec4), remap.data());
            mesh->boneIndex.resize(newVertexCount);
        }   
        if(mesh->HasBoneWeight())   
        {
            meshopt_remapVertexBuffer(mesh->boneWeight.data(), mesh->boneWeight.data(), vertexCount, sizeof(Vec4), remap.data());
            mesh->boneWeight.resize(newVertexCount);
        }	
	}

	return ret;
}

bool MeshOptimizor::OptimizeMesh(MeshRef mesh)
{
	bool ret = RemapMesh(mesh);
	uint32_t indexCount = mesh->index.size();
	uint32_t vertexCount = mesh->position.size();

	//优化顶点缓存
	meshopt_optimizeVertexCache(mesh->index.data(), mesh->index.data(), indexCount, vertexCount);

	//过度绘制优化
	meshopt_optimizeOverdraw(mesh->index.data(), mesh->index.data(), indexCount, (float*)&mesh->position[0], vertexCount, sizeof(Vec3), 1.05f);

	return ret;
}

float MeshOptimizor::SimplifyMesh(
    MeshRef mesh,
    float threshold,
    float targetError, 
    std::vector<uint32_t>& newIndex)
{
	//网格简化，meshopt_SimplifyLockBorder保边缘

	//float			threshold = 0.2f;
	//float			targetError = 1e-2f;
	size_t			targetIndexCount = size_t(float(mesh->index.size()) * threshold);
	unsigned int	options = meshopt_SimplifyLockBorder; 
	//unsigned int	options = 0; // meshopt_SimplifyX flags, 0 is a safe default
	float			lodError = 0.f;
	uint32_t		newIndexCount;

	newIndex.resize(mesh->index.size());
	newIndexCount = meshopt_simplify(
		newIndex.data(), 
		mesh->index.data(), 
		mesh->index.size(), 
		(float*)&mesh->position[0], 
		mesh->position.size(), sizeof(Vec3),
		targetIndexCount, targetError, options, &lodError);

	newIndex.resize(newIndexCount);

	//LOG_DEBUG("SimplifyMesh: try to reduce index size from %d to %d, target error %f, result [%d, %f]\n", 
	//		(uint32_t)mesh->index.size(), (uint32_t)targetIndexCount, targetError, newIndexCount, lodError);

	return lodError;
}

float MeshOptimizor::SimplifyMeshSloppy(
    MeshRef mesh,
    float threshold,
    float targetError,
    std::vector<uint32_t>& newIndex)
{
	//网格简化，完全不保边缘

	//float			threshold = 0.2f;
	//float			targetError = 1e-2f;
	size_t			targetIndexCount = size_t(mesh->index.size() * threshold);
	float			lodError = 0.f;
	uint32_t		newIndexCount;

	newIndex.resize(mesh->index.size());
	newIndexCount = meshopt_simplifySloppy(
		newIndex.data(),
		mesh->index.data(),
		mesh->index.size(),
		(float*)&mesh->position[0], 
		mesh->position.size(), sizeof(Vec3),
		targetIndexCount, targetError, &lodError);

	newIndex.resize(newIndexCount);

	//LOG_DEBUG("SimplifyMesh: try to reduce index size from %d to %d, target error %f\n", (uint32_t)mesh->index.size(), (uint32_t)targetIndexCount, targetError);
	//LOG_DEBUG("SimplifyMesh: result index count %d, result error %f\n", newIndexCount, lodError);

	return lodError;
}

//能聚类，但是考虑了缓存问题？聚的效果在斯坦福兔子上很差
/*
uint32_t* MeshOptimizor::ClusterMesh(Vertex* vertex, uint32_t& vertexCount, uint32_t* index, uint32_t& indexCount)
{
	uint32_t* newIndices = nullptr;
	uint32_t newIndexCount = 0;

	//cluster生成
	if (true)
	{
		const size_t max_vertices = 64;		//单cluster最多使用的顶点数目
		const size_t max_triangles = 124;	//单cluster最多形成的三角形数目， *3 为索引数
		const float cone_weight = 0.0f;

		size_t max_meshlets = meshopt_buildMeshletsBound(indexCount, max_vertices, max_triangles);
		std::vector<meshopt_Meshlet> meshlets(max_meshlets);
		std::vector<unsigned int> meshlet_vertices(max_meshlets * max_vertices);
		std::vector<unsigned char> meshlet_triangles(max_meshlets * max_triangles * 3);


		size_t meshlet_count = meshopt_buildMeshlets(
			meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(),
			index, indexCount,
			(float*)&vertex[0].pos, vertexCount, sizeof(Vertex),
			max_vertices, max_triangles, cone_weight);

		//size_t meshlet_count = meshopt_buildMeshletsScan(
		//	meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(),
		//	index, indexCount,
		//	vertexCount,
		//	max_vertices, max_triangles);

		const meshopt_Meshlet& last = meshlets[meshlet_count - 1];

		meshlet_vertices.resize(last.vertex_offset + last.vertexCount);
		meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
		meshlets.resize(meshlet_count);


		//printf("meshlet_count: %d\n", meshlet_count);
		newIndexCount = meshlet_count * max_triangles * 3;
		newIndices = (uint32_t*)(malloc(newIndexCount * sizeof(uint32_t)));   //更换索引buffer存所有cluster


		int index_offset = 0;
		for (int i = 0; i < meshlet_count; i++)
		{
			const meshopt_Meshlet& mesh = meshlets[i];

			//printf("%d, %d, %d, %d\n", mesh.triangle_offset, mesh.triangle_count, mesh.vertex_offset, mesh.vertexCount);

			int triangle_offset = mesh.triangle_offset;
			int triangle_count = mesh.triangle_count;
			int vertexCount = mesh.vertexCount;
			int vertex_offset = mesh.vertex_offset;


			for (int j = 0; j < triangle_count; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					unsigned char temp_triangle = meshlet_triangles[triangle_offset + j * 3 + k];
					unsigned int temp_index = meshlet_vertices[vertex_offset + temp_triangle];

					if (index_offset < newIndexCount) newIndices[index_offset++] = temp_index;
					else goto end;
				}
			}

			if (triangle_count < max_triangles)
			{
				unsigned char temp_triangle = meshlet_triangles[triangle_offset + triangle_count * 3 - 1];
				unsigned int temp_index = meshlet_vertices[vertex_offset + temp_triangle];

				for (int j = triangle_count; j < max_triangles; j++)
				{
					for (int k = 0; k < 3; k++)
					{
						if (index_offset < newIndexCount) newIndices[index_offset++] = temp_index;
						else goto end;
					}
				}
			}

		}

		//for (int i = 0; i < meshlet_triangles.size(); i++) printf("%d ", meshlet_triangles[i]);
		//printf("\n");
		//for (int i = 0; i < meshlet_vertices.size(); i++) printf("%d ", meshlet_vertices[i]);
		//printf("/////////////////////////////////////\n");
		//for (int i = 0; i < meshlet_count; i++)
		//{
		//	for (int j = 0; j < max_triangles; j++)
		//	{
		//		for (int k = 0; k < 3; k++)
		//		{
		//			printf("%d ", newIndices[i * max_triangles * 3 + j * 3+ k]);
		//		}
		//		printf("\n");
		//	}
		//	printf("/////////////////////////////////////\n");
		//}
	}

end:

	indexCount = newIndexCount;
	return newIndices;
}
*/


/*
static void merge(std::vector<meshopt_Meshlet>& meshlets, std::vector<uint32_t>& index, std::vector<meshopt_Bounds>& meshletBounds, std::vector<uint8_t>vertices, int stride)
{
	std::vector<uint32_t>newIndex;
	std::vector<meshopt_Meshlet>new_meshlets;
	std::vector<meshopt_Bounds>new_bound;
	std::vector<bool>is_merged(meshlets.size(), false);
	auto offset = 0;
	for (int cnt = 0; cnt + 1 < meshlets.size(); cnt = cnt + 2)
	{
		meshopt_Meshlet mt1, mt2;
		{
			//.....
				//find mt1,mt2
		}
		for (int i = 0; i < mt1.triangle_count * 3; i++)
		{
			newIndex.emplace_back(index[mt1.triangle_offset + i]);
		}
		for (int i = 0; i < mt2.triangle_count * 3; i++)
		{
			newIndex.emplace_back(index[mt2.triangle_offset + i]);
		}
		meshopt_Meshlet tmp;
		tmp.triangle_count = mt1.triangle_count + mt2.triangle_count;
		tmp.triangle_offset = offset;
		new_meshlets.emplace_back(tmp);
		offset = newIndex.size();
		//
		auto p = meshopt_computeClusterBounds(&newIndex[tmp.triangle_offset], tmp.triangle_count * 3, (float*)vertices.data(), vertices.size() / stride, stride);
		new_bound.emplace_back(p);
		//
	}
	//odd size
	if (meshlets.size() % 2 != 0)
	{
		meshopt_Meshlet last;
		for (int id = 0; id < is_merged.size(); id++)
		{
			if (is_merged[id] == false)
			{
				last = meshlets[id];
			}
		}
		for (int i = 0; i < last.triangle_count * 3; i++)
		{
			newIndex.emplace_back(index[last.triangle_offset + i]);
		}
		meshopt_Meshlet tmp;
		tmp.triangle_count = last.triangle_count;
		tmp.triangle_offset = offset;
		new_meshlets.emplace_back(tmp);
		offset = newIndex.size();
	}
	meshlets.assign(new_meshlets.begin(), new_meshlets.end());
	index.assign(newIndex.begin(), newIndex.end());
	//simplify meshlets
	for (int i = 0; i < meshlets.size(); i++)
	{
		auto& mt = meshlets[i];
		float targetError = 0.9;
		float lodError = 0.f;
		auto n = meshopt_simplify(&index[mt.triangle_offset], &index[mt.triangle_offset], mt.triangle_count * 3, (float*)vertices.data(), vertices.size() / stride, stride,
			6, targetError, 1, &lodError);
		mt.triangle_count = n / 3;
	}
	//recalculate the bounds
	for (int i = 0; i < meshlets.size(); i++)
	{
		auto tmp = meshlets[i];
		auto p = meshopt_computeClusterBounds(&newIndex[tmp.triangle_offset], tmp.triangle_count * 3, (float*)vertices.data(), vertices.size() / stride, stride);
		new_bound.emplace_back(p);
	}
	meshletBounds.assign(new_bound.begin(), new_bound.end());
}
*/