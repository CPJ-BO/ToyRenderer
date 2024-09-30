#include "AssetManager.h"
#include "Core/UID/UID.h"
#include "Function/Global/EngineContext.h"
#include "Platform/File/FileSystem.h"
#include "Resource/Asset/Asset.h"
#include <fstream>
#include <memory>
#include <string>

void AssetManager::Init()
{
	// 初始化时扫描和加载目标路径下的全部资源，后续也可通过GetOrLoadAsset继续加载
	for(auto& path : EngineContext::File()->Traverse(EngineContext::File()->AssetPath(), true))
	{
		std::string extention = EngineContext::File()->Extension(path);
		if(	extention == "asset" || 
			extention == "binasset")
		{
			ENGINE_LOG_INFO("Pre loading asset {} from file...", path);
			LoadAsset(path, false);
		}
	}
}

void AssetManager::Tick()
{
	ENGINE_TIME_SCOPE(AssetManager::Tick);
	for(auto& asset : assets)
	{
		if(asset.second.use_count() == 1)	//每帧检查，只在manager处有引用的资源就释放掉了
		{
			ENGINE_LOG_INFO("Asset [{}] [{}] released.", asset.second->GetAssetTypeName(), asset.second->GetUID().ToString());
			assets.erase(asset.first);
		}
	}
}

void AssetManager::Save()
{
	// for (auto& iter : assets)	// 由于递归，这种写法可能多次保存同一个资源
	// {
	// 	SaveAsset(iter.second);
	// 	ENGINE_LOG_INFO("Asset [{}] [{}] saved.", iter.second->GetAssetTypeName(), iter.second->GetUID().ToString());
	// }
}

AssetRef AssetManager::LoadAsset(const std::string& filePath, bool init)
{
	AssetRef asset;

	if(filePath.empty()) 
	{
		ENGINE_LOG_WARN("Asset path cannot be null!");
		return nullptr;
	}
	
	// 读文件，反序列化
	std::string extention = EngineContext::File()->Extension(filePath);
	int format =  extention == "asset" 	 ? 2 : 
				  extention == "binasset" ? 1 : 0;
	if(format == 0)
	{
		ENGINE_LOG_WARN("Asset extention [{}] is not valid!", extention);
		return nullptr;
	}
	else if(format == 1)
	{
		std::ifstream ifs(EngineContext::File()->Absolute(filePath), std::ios::binary);
		cereal::BinaryInputArchive archive(ifs);
		archive(asset);
	}
	else 
	{
		std::ifstream ifs(EngineContext::File()->Absolute(filePath));
		cereal::JSONInputArchive archive(ifs);
		archive(asset);
	}

	// 初始化资源,存储键值索引
	if(init) 								
	{
		asset->OnLoadAsset();	//	
		assets[asset->GetUID()] = asset;
	}
	else 
	{	
		uninitializedAssets[asset->GetUID()] = asset;
	}
	UpdateFilePathAndUID(filePath, asset->GetUID());
	
	return asset;
}

AssetRef AssetManager::GetOrLoadAssetInternal(const std::string& filePath)
{
	AssetRef asset = GetAsset(filePath);
	if(asset == nullptr) 	asset = LoadAsset(filePath, true);

	if (asset == nullptr)	ENGINE_LOG_WARN("Fail to load asset \t [{}]", filePath);
	else					ENGINE_LOG_INFO("Finish loading asset \t [{}]", filePath);

	return asset;
}

AssetRef AssetManager::GetOrLoadAssetInternal(const UID& uid)
{
	AssetRef asset = GetAsset(uid);
	//if(asset == nullptr) 	asset = LoadAsset(filePath, true);	// UID找不到路径就没办法了

	if (asset == nullptr)	ENGINE_LOG_WARN("Fail to load asset \t [{}]", uid.ToString());
	else					ENGINE_LOG_INFO("Finish loading asset \t [{}]", uid.ToString());

	return asset;
}

AssetRef AssetManager::GetAsset(const std::string& filePath)
{
	UID id = FilePathToUID(filePath);
	if(id.IsEmpty())
	{
		ENGINE_LOG_WARN("Fail to find asset from cache \t [{}]", filePath);
		return nullptr;
	}
	return GetAsset(id);
}

AssetRef AssetManager::GetAsset(const UID& uid)
{
	if (assets.find(uid) != assets.end()) return assets[uid];

	auto iter = uninitializedAssets.find(uid);
	if(iter != uninitializedAssets.end())
	{
		AssetRef asset = iter->second;
		asset->OnLoadAsset();
		uninitializedAssets.erase(iter);
		assets[uid] = asset;

		return asset;
	}

	ENGINE_LOG_WARN("Fail to find asset from cache \t [{}]", uid.ToString());
	return nullptr;
}

void AssetManager::SaveAsset(AssetRef asset, const std::string& filePath)
{
	// 处理文件路径
	std::string extention = EngineContext::File()->Extension(filePath);
	int format =  extention == "asset" 	 ? 2 : 
				  extention == "binasset" ? 1 : 0;
	if(!filePath.empty() && format == 0)
	{
		ENGINE_LOG_WARN("Asset extention [{}] is not valid!", extention);
		return;
	}

	std::string oldPath = UIDToFilePath(asset->GetUID());
	std::string path = 	!filePath.empty() ? filePath :																		// 显式提供的新路径
						!oldPath.empty() ? oldPath :																		// 读取时候的旧路径
						EngineContext::File()->TempAssetPath().append(asset->GetUID().ToString() + "." + "asset");	// 默认保存路径

	asset->OnSaveAsset();	// 

	UpdateFilePathAndUID(path, asset->GetUID());
	
	if(!path.compare(oldPath))	// 删除旧文件
	{
		EngineContext::File()->RemoveFile(oldPath);
	}

	// 序列化
	if(EngineContext::File()->Extension(path) == "binasset")
	{
		std::ofstream ofs(EngineContext::File()->Absolute(path), std::ios::binary);
		cereal::BinaryOutputArchive archive(ofs);
		archive(asset);
	}
	else 
	{
		std::ofstream ofs(EngineContext::File()->Absolute(path));
		cereal::JSONOutputArchive archive(ofs);
		archive(asset);
	}

	assets[asset->GetUID()] = asset;
}

void AssetManager::UpdateFilePathAndUID(const std::string& filePath, const UID& uid)
{
	if(filePath.empty())
	{
		auto uidIter = uidToPath.find(uid);
		if (uidIter != uidToPath.end()) 
		{
			pathToUID.erase(uidIter->second);
			uidToPath.erase(uid);
		}	
	}
	else 
	{
		// 路径被其他资源使用，覆盖
		auto pathIter = pathToUID.find(filePath);
		if (pathIter != pathToUID.end() && pathIter->second != uid) 
		{
			uidToPath.erase(pathIter->second);
			pathToUID.erase(pathIter);
		}

		// 更换路径
		auto uidIter = uidToPath.find(uid);
		if (uidIter != uidToPath.end() && uidIter->second != filePath) 
		{
			pathToUID.erase(filePath);
		}

		// 更新索引
		pathToUID[filePath] = uid;
		uidToPath[uid] = filePath;
	}
}

UID AssetManager::FilePathToUID(const std::string& filePath)
{
	auto iter = pathToUID.find(filePath);
	if(iter == pathToUID.end()) 
	{
		return UID::Empty();
	}
	return iter->second;
}

std::string AssetManager::UIDToFilePath(const UID& uid)
{
	auto iter = uidToPath.find(uid);
	if(iter == uidToPath.end()) 
	{
		return "";
	}
	return iter->second;
}

void AssetManager::DeleteAsset(AssetRef asset)
{
	std::string oldPath = UIDToFilePath(asset->GetUID());
	if(oldPath.empty())
	{
		ENGINE_LOG_WARN("Asset doesn't have file path! \t [{}]", asset->GetUID().ToString());
		return;
	}

	DeleteAsset(oldPath);
}

void AssetManager::DeleteAsset(const std::string& filePath)
{
	std::string extention = EngineContext::File()->Extension(filePath);
	int format =  extention == "asset" 	 ? 2 : 
				  extention == "binasset" ? 1 : 0;
	if(filePath.empty() || format == 0)
	{
		ENGINE_LOG_WARN("File extention [{}] is not valid!", extention);
		return;
	}

	EngineContext::File()->RemoveFile(filePath);

	UID uid = FilePathToUID(filePath);
	if(!uid.IsEmpty())
	{
		UpdateFilePathAndUID("", uid);
	}
}