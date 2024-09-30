#pragma once

#include "Core/Serialize/Serializable.h"
#include "Core/UID/UID.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum AssetType
{
	ASSET_TYPE_UNKNOWN = 0,
	ASSET_TYPE_MODEL,
	ASSET_TYPE_TEXTURE,
	ASSET_TYPE_SHADER,
	ASSET_TYPE_MATERIAL,	
	ASSET_TYPE_ANIMATION,
	ASSET_TYPE_SCENE,

    ASSET_TYPE_MAX_ENUM,    //
};

//Asset的初始化由cereal库的反序列化实现，初始化参数后使用OnLoadAsset()创建相应的资源内容
class Asset 
{
public:
    Asset() = default;
	virtual ~Asset() = default;

	virtual std::string GetAssetTypeName() 	    { return "Unknown"; }				// 类型字符串
	virtual AssetType GetAssetType() 			{ return ASSET_TYPE_UNKNOWN; }		// 类型枚举
    virtual void OnLoadAsset() {};													// 反序列化后，实际申请资源时调用以初始化对象
	virtual void OnSaveAsset() {};													// 序列化前，完成资源绑定等保存前的准备工作
    
    inline const UID& GetUID()                  { return uid; }						// 全局唯一标识符，主键

protected:
    UID uid = {};
	friend class AssetManager;

private:
    BeginSerailize()
    SerailizeEntry(uid)
    EndSerailize
};
typedef std::shared_ptr<Asset> AssetRef;

class AssetBinder
{
protected:
	std::unordered_map<std::string, UID> assetMap;
	std::unordered_map<std::string, std::vector<UID>> assetArrayMap;

private:
	BeginSerailize()
	SerailizeEntry(assetMap)
	SerailizeEntry(assetArrayMap)
	EndSerailize
};

#define EnableAssetEditourUI() \
friend class AssetWidget;

#define BeginLoadAssetBind() {

#define LoadAssetBind(className, bind)	 												\
do {																					\
	auto iter = assetMap.find(#bind);													\
    if(iter != assetMap.end() && !iter->second.IsEmpty())								\
	{																					\
		bind = EngineContext::Asset()->GetOrLoadAsset<className>(iter->second);			\
	}																					\
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
			if(!iter->second[i].IsEmpty())					\
			{												\
				bind[i] = EngineContext::Asset()->GetOrLoadAsset<className>(iter->second[i]);	\
			}												\
		}													\
	}														\
} while(0);

#define EndLoadAssetBind }

#define BeginSaveAssetBind() 							\
{ 	assetMap.clear();									\
	assetArrayMap.clear();

#define SaveAssetBind(bind) 							\
	if(bind) 											\
	{													\
		EngineContext::Asset()->SaveAsset(bind);		\
		assetMap.emplace(#bind, bind->GetUID());		\
	}													\
	else	 assetMap.emplace(#bind, UID::Empty());

#define SaveAssetArrayBind(bind) 						\
do {													\
	std::vector<UID> uids;								\
	for(uint32_t i = 0; i < bind.size(); i++) 			\
	{													\
	 	if(bind[i]) 									\
		{												\
			EngineContext::Asset()->SaveAsset(bind[i]);	\
			uids.push_back(bind[i]->GetUID());			\
		}												\
		else		uids.push_back(UID::Empty());		\
	}													\
	assetArrayMap.emplace(#bind, uids);					\
} while(0);

#define EndSaveAssetBind }



