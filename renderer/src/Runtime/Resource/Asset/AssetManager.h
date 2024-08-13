#pragma once

#include "Resource/Asset/Asset.h"

#include <memory>
#include <string>
#include <unordered_map>

class AssetManager
{
public:
	AssetManager() = default;
	~AssetManager() {};

	void Init();

	void Save();

	// AssetType GetAssetType(const std::string& path);

	template<typename Type>
	std::shared_ptr<Type> GetOrLoadAsset(const std::string& path)			
	{
		std::shared_ptr<Asset> asset = GetOrLoadAssetInternal(path);
		return std::dynamic_pointer_cast<Type>(asset);
	}

	std::shared_ptr<Asset> GetAsset(const std::string& path);					

	const std::unordered_map<std::string, std::shared_ptr<Asset>>& GetAssets() { return assets; }

	void SaveAsset(std::shared_ptr<Asset> asset, const std::string& filePath = "");

	std::shared_ptr<Asset> LoadAsset(const std::string& path);

private:
	std::unordered_map<std::string, std::shared_ptr<Asset>> assets;						

	std::shared_ptr<Asset> GetOrLoadAssetInternal(const std::string& path);
};

