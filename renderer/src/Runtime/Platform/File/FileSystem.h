#pragma once

#include <string>
#include <vector>
#include <filesystem>

//抄的 https://github.com/Code-Guy/Bamboo/tree/master

enum FileOrder
{
	FILE_NAME = 0, 
	FILE_TIME,
	FILE_SIZE
};

class FileSystem
{
public:
	FileSystem() = default;
	~FileSystem() {}

	std::string AssetPath() 		{ return "Asset/"; }				// 写死的固定路径
    std::string TempAssetPath() 	{ return "Asset/Temp/"; }
	std::string BuildInAssetPath()	{ return "Asset/BuildIn/"; }
	std::string ShaderPath()		{ return "Asset/BuildIn/Shader/"; }
	std::string FontPath()			{ return "Asset/BuildIn/Font/"; }
	std::string ScenePath()			{ return "Asset/BuildIn/Scene/"; }
	std::string TexturePath()		{ return "Asset/BuildIn/Texture/"; }
	std::string ModelPath()			{ return "Asset/BuildIn/Model/"; }
	std::string MaterialPath()		{ return "Asset/BuildIn/Material/"; }

	void Init(const std::string& basePath);

    std::string Absolute(const std::string& path);
	std::string Extension(const std::string& path);
	std::string ReplaceExtension(const std::string& path, const std::string& extention);
	std::string ReplaceFileName(const std::string& path, const std::string& fileName);
	std::string RemoveFilename(const std::string& path);
	std::string Basename(const std::string& path);
	std::string Filename(const std::string& path);
	std::string ModifiedTime(const std::string& path);
	std::vector<std::string> Traverse(const std::string& path = "", bool isRecursive = false, FileOrder fileOrder = FILE_NAME, bool isReverse = false);

	bool Exists(const std::string& path);
	bool IsFile(const std::string& path);
	bool IsDir(const std::string& path);
	bool IsEmptyDir(const std::string& path);

	bool CreateFile(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out);
	bool CreateDir(const std::string& path, bool isRecursive = false);
	bool RemoveFile(const std::string& filename);
	bool RemoveDir(const std::string& path, bool isRecursive = false);
	void CopyFile(const std::string& from, const std::string& to);
	void RenameFile(const std::string& dir, const std::string& oldName, const std::string& newName);

	bool LoadBinary(const std::string& filename, std::vector<uint8_t>& data);
	bool WriteString(const std::string& filename, const std::string& str);
	bool LoadString(const std::string& filename, std::string& str);

private:
	std::filesystem::path root;
};