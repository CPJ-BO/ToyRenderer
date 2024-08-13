#include "AssetManager.h"
#include "Function/Global/EngineContext.h"
#include "Resource/Asset/Asset.h"
#include <fstream>
#include <memory>


void AssetManager::Init()
{

}

void AssetManager::Save()
{
	for (auto& iter : assets)
	{
		SaveAsset(iter.second);
	}
}

// AssetType AssetManager::GetAssetType(const std::string& path)
// {
// 	std::string extension = EngineContext::File()->Extension(path.c_str());

//     for(int i = 0; i < assetExtentions.size(); i++)
//     {
//         if(assetExtentions[i].compare(extension) == 0) return (AssetType)i;
//     }

// 	return ASSET_TYPE_UNKNOWN;
// }

std::shared_ptr<Asset> AssetManager::GetAsset(const std::string& path)
{
	if (!EngineContext::File()->Exists(path.c_str()))   return nullptr;
	if (assets.find(path) != assets.end())              return assets[path];

	return nullptr;
}

void AssetManager::SaveAsset(std::shared_ptr<Asset> asset, const std::string& filePath)
{
	if(!filePath.empty())
	{
		auto iter = assets.find(asset->filePath);
		if(iter != assets.end()) assets.erase(iter);
		assets.emplace(filePath, asset);

		asset->filePath = filePath;
	}

	if(asset->filePath.empty()) 
	{
		ENGINE_LOG_WARN("Asset path cannot be null!");
		return;
	}
	asset->OnSaveAsset();

	// AssetType type = asset->GetType();
	// const std::string& asset_ext = assetExtentions[type];
	{
		std::ofstream ofs(EngineContext::File()->Absolute(asset->filePath));
		cereal::JSONOutputArchive archive(ofs);
		archive(asset);

		//std::ofstream ofs(filename, std::ios::binary);
		//cereal::BinaryOutputArchive archive(ofs);
		//archive(cereal::make_nvp(asset_ext.c_str(), asset));
	}
	ENGINE_LOG_INFO("Finish saving asset \t [{}]", asset->filePath);
}

std::shared_ptr<Asset> AssetManager::LoadAsset(const std::string& path)
{
	if(path.empty()) 
	{
		ENGINE_LOG_WARN("Asset path cannot be null!");
		return nullptr;
	}

	std::shared_ptr<Asset> asset;

	// AssetType type = GetAssetType(path);
	// const std::string& asset_ext = assetExtentions[type];
	{
		std::ifstream ifs(EngineContext::File()->Absolute(path));
		cereal::JSONInputArchive archive(ifs);
		archive(asset);

		//std::ifstream ifs(filename, std::ios::binary);
		//cereal::BinaryInputArchive archive(ifs);
		//archive(asset);
	}

	asset->filePath = path;
	asset->OnLoadAsset();

	assets[path] = asset;
	return asset;
}

std::shared_ptr<Asset> AssetManager::GetOrLoadAssetInternal(const std::string& path)
{
	ENGINE_LOG_INFO("Start to load asset \t [{}]", path);
	std::shared_ptr<Asset> asset = GetAsset(path);
	if(asset == nullptr) asset = LoadAsset(path);

	if (asset == nullptr)	ENGINE_LOG_WARN("Fail to load asset \t [{}]", path);
	else					ENGINE_LOG_INFO("Finish loading asset \t [{}]", path);

	return asset;
}
