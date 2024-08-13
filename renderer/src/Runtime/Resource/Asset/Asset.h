#pragma once

#include "Core/Serialize/Serializable.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum AssetType
{
	ASSET_TYPE_UNKNOWN = 0,
	ASSET_TYPE_MODEL,
	ASSET_TYPE_TEXTURE,
	ASSET_TYPE_MATERIAL,	
	ASSET_TYPE_ANIMATION,
	ASSET_TYPE_SCENE,

    ASSET_TYPE_MAX_ENUM,    //
};

// const std::vector<std::string> assetExtentions = {
// 	"unknown",
// 	"model",
// 	"texr",
// 	"mtl",
// 	"anim",
// 	"scene",
// 	"max"
// };

//Asset的初始化由cereal库的反序列化实现，初始化参数后使用OnLoadAsset()创建相应的资源内容
class Asset	: public std::enable_shared_from_this<Asset>
{
public:
    Asset() = default;
	~Asset() { std::cout << "Asset [" << this << "] destructed :" << filePath << std::endl; }

	virtual AssetType GetType() 					{ return ASSET_TYPE_UNKNOWN; }

	const std::string& GetFilePath() 				{ return filePath; }

protected:
	std::string filePath;

	virtual void OnLoadAsset() = 0;
	virtual void OnSaveAsset() {};

	friend class AssetManager;

private:
    BeginSerailize()
    SerailizeEntry(filePath)
    EndSerailize
};

class AssetBinder
{
protected:
	std::unordered_map<std::string, std::string> assetMap;
	std::unordered_map<std::string, std::vector<std::string>> assetArrayMap;

private:
	BeginSerailize()
	SerailizeEntry(assetMap)
	SerailizeEntry(assetArrayMap)
	EndSerailize
};

#define BeginLoadAssetBind() {

#define LoadAssetBind(className, bind)	 										\
do {																			\
	auto iter = assetMap.find(#bind);											\
    if(iter != assetMap.end() && !iter->second.empty())							\
	{																			\
		bind = EngineContext::Asset()->GetOrLoadAsset<className>(iter->second);	\
	}																			\
} while(0);

// 对于vector类型的asset索引，load时要先分配大小；array类型就不需要了
#define ResizeAssetArray(bind)								\
do {														\
	auto iter = assetArrayMap.find(#bind);					\
    if(iter != assetArrayMap.end())							\
	{														\
		bind.resize(iter->second.size());					\
	}														\
} while(0);

#define LoadAssetArrayBind(className, bind)	 				\
do {														\
	auto iter = assetArrayMap.find(#bind);					\
    if(iter != assetArrayMap.end())							\
	{														\
		for(uint32_t i = 0; i < iter->second.size(); i++)	\
		{													\
			if(!iter->second[i].empty())					\
			{												\
				bind[i] = EngineContext::Asset()->GetOrLoadAsset<className>(iter->second[i]);	\
			}												\
		}													\
	}														\
} while(0);

#define EndLoadAssetBind }

#define BeginSaveAssetBind() 	\
{ 	assetMap.clear();			\
	assetArrayMap.clear();

#define SaveAssetBind(bind) 	\
	if(bind) assetMap.emplace(#bind, bind->GetFilePath());	\
	else	 assetMap.emplace(#bind, "");

#define SaveAssetArrayBind(bind) 				\
do {											\
	std::vector<std::string> paths;				\
	for(uint32_t i = 0; i < bind.size(); i++) 	\
	{											\
	 	if(bind[i]) paths.push_back(bind[i]->GetFilePath());	\
		else		paths.push_back("");		\
	}											\
	assetArrayMap.emplace(#bind, paths);		\
} while(0);

#define EndSaveAssetBind }



