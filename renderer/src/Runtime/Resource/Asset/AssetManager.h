#pragma once

#include "Core/UID/UID.h"
#include "Resource/Asset/Asset.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

// 什么样的对象应该被抽象成资源？ /////////////////////////////////////////////////////////////
// 需要被缓存以免重复创建的对象，需要序列化反序列化进行存储的对象
// 资源应该是cpp类对象，只是持有对物理文件的引用，而不是具体的物理文件例如.jpg .fbx等本身
// （但是unity对于rendertexture,material等无物理文件引用的类对象仍然有.rendertexutre,.mat和.meta两种文件，前者包含序列化信息，后者包含UUID，为什么要拆开？）
// 资源未必一定需要存储到物理文件中（也就是对应一个具体的文件路径），应该需要支持纯运行时的抽象，但是也需要唯一标识来索引（在这样的需求下，UUID是否就是唯一可选的？）
// 但是当真正需要存储到物理文件时（例如材质索引了着色器，需要在序列化材质的同时序列化着色器），新加的这些文件多起来之后也感觉不太合适？
// （例如unity统一使用.meta后缀存储资源信息，内含UUID，像着色器这种没有很多有效信息的.meta文件也还是和.shader一一对应的，乐）
// 使用UUID标识的优点在于和文件路径解耦，把资源文件到处扔仍然保证可用，只需要保证资源索引的.jpg等文件的路径正确就行；缺点在于需要额外的搜索代价

// 资源应该怎样管理？ /////////////////////////////////////////////////////////////
// 如何将文件路径内的资源与运行时资源做映射，以保证缓存读？ UUID和文件路径两种方法，用UUID
// 资源之间的引用关系？运行时智能指针，存储时UUID
// 资源和物理文件的引用关系？暂时不管理了（TODO）

// 最终的解决方案 /////////////////////////////////////////////////////////////
// AssetManager作为全局单例，负责管理整个系统的全部Asset
// 所有运行时Asset在创建时就会被分配唯一UUID标识
// 所有文件存储的Asset将会在引擎初始化时由AssetManager扫描指定文件根目录下的全部文件，并进行创建（但并不会初始化，仅当资源被实际请求时完成所有资源的初始化成为可用的资源）	// 此处也可以做成随后再找的
// 只有使用AssetManager处理Asset时（序列反序列化），会将Asset并入到AssetManager缓存中进行管理，也即纯运行时构建的Asset不受管理
// Asset文件存储的索引通过UUID完成，运行时索引使用智能指针，序列化时通过指针去查UUID来存，反序列化时通过UUID向AssetManager请求运行时资源
// 此外AssetManager也维护全部物理文件路径与资源的索引关系，方便通过路径查找
// 序列化和反序列化资源时，需要递归的处理全部依赖资源；这种递归处理也会为还没并入AssetManager缓存中的资源创建一个默认文件，并加入管理
// TODO 资源的请求最终需要支持异步

class AssetManager
{
public:
	AssetManager() = default;
	~AssetManager() {};

	void Init();

	void Tick();

	void Save();

	UID FilePathToUID(const std::string& filePath);				// 检索特定文件目录是否对应资源，返回UID
	std::string UIDToFilePath(const UID& uid);	

	template<typename Type>
	std::shared_ptr<Type> GetOrLoadAsset(const std::string& filePath)			
	{
		AssetRef asset = GetOrLoadAssetInternal(filePath);
		return std::dynamic_pointer_cast<Type>(asset);
	}

	template<typename Type>
	std::shared_ptr<Type> GetOrLoadAsset(const UID& uid)			
	{
		AssetRef asset = GetOrLoadAssetInternal(uid);
		return std::dynamic_pointer_cast<Type>(asset);
	}

	AssetRef GetAsset(const std::string& filePath);	
	AssetRef GetAsset(const UID& uid);						

	void SaveAsset(AssetRef asset, const std::string& filePath = "");	// 保存资源到指定路径；会覆盖已有资源
	void DeleteAsset(AssetRef asset);									// 删除指定资源的物理文件
	void DeleteAsset(const std::string& filePath);

	const std::unordered_map<UID, AssetRef>& GetAssets() { return assets; }

private:
	std::unordered_map<UID, AssetRef> assets;					// 管理的全部资产，键为资产UID			
	std::unordered_map<UID, AssetRef> uninitializedAssets;	
	std::unordered_map<std::string, UID> pathToUID;				// 文件路径到UID的索引，本身也是主键，有一一对应关系
	std::unordered_map<UID, std::string> uidToPath;

	void UpdateFilePathAndUID(const std::string& filePath, const UID& uid);	

	AssetRef GetOrLoadAssetInternal(const std::string& filePath);
	AssetRef GetOrLoadAssetInternal(const UID& uid);

	AssetRef LoadAsset(const std::string& path, bool init = false);	
};

