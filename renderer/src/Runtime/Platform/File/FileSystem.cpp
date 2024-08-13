
#include "FileSystem.h"
#include "Function/Global/EngineContext.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <string>

void FileSystem::Init(const std::string& basePath)
{
    std::string abs = std::filesystem::absolute(".").generic_string();
    size_t index = abs.find("renderer");
    if(index == std::string::npos) 
    {
        ENGINE_LOG_FATAL("Can't init file system base path!");
    }
    abs = "/" + abs.erase(0, index);
    ENGINE_LOG_INFO("Current exe path is: {}", abs);

    std::string workingPath = "";
    int count = std::count(abs.begin(), abs.end(), '/');  
    for(int i = 0; i < count - 1; i++) workingPath.append("../");
    ENGINE_LOG_INFO("Current file system working path is: {}", ("/" + basePath));

    root = std::filesystem::path(workingPath);
}

std::string FileSystem::Absolute(const std::string& path)
{
	std::filesystem::path header = root;
	return header.append(path).generic_string();
}

std::string FileSystem::Extension(const std::string& path)
{
    std::filesystem::path relativePath = this->root;
    relativePath.append(path);

	std::string extension = relativePath.extension().generic_string();
	if (!extension.empty() && extension[0] == '.')
	{
		extension.erase(0, 1);
	}
	return extension;
}

std::string FileSystem::Basename(const std::string& path)
{
    std::filesystem::path relativePath = this->root;
    relativePath.append(path);

	return relativePath.stem().generic_string();
}

std::string FileSystem::Filename(const std::string& path)
{
    std::filesystem::path relativePath = this->root;
    relativePath.append(path);

	return relativePath.filename().generic_string();
}


std::string FileSystem::ModifiedTime(const std::string& path)
{
    std::string relativePath = this->root.generic_string();
    relativePath.append(path);

	if (IsFile(path))
	{
		return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(relativePath).time_since_epoch()).count());
	}

	if (IsDir(path))
	{
		long long last_write_time = 0;
		std::vector<std::string> files = Traverse(path, true);
		for (const std::string& file : files)
		{
			if (IsFile(file))
			{
				last_write_time = std::max(last_write_time, std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(file).time_since_epoch()).count());
			}
		}
		return std::to_string(last_write_time);
	}
	return "";
}

std::vector<std::string> FileSystem::Traverse(const std::string& path, bool is_recursive, FileOrder file_order, bool is_reverse)
{
    std::string relativePath = this->root.generic_string();
    relativePath.append(path);

	std::vector<std::string> filenames;
	if (!std::filesystem::exists(relativePath))
	{
		return filenames;
	}

	if (is_recursive)
	{
		for (const auto& file : std::filesystem::recursive_directory_iterator(relativePath))
		{
			filenames.push_back(std::filesystem::relative(file.path(), root).generic_string());
		}
	}
	else
	{
		for (const auto& file : std::filesystem::directory_iterator(relativePath))
		{
			filenames.push_back(std::filesystem::relative(file.path(), root).generic_string());
		}
	}

	std::sort(filenames.begin(), filenames.end(),
		[file_order, is_reverse](const std::string& lhs, const std::string& rhs)
		{
			bool result = false;
			switch (file_order)
			{
			case FILE_NAME:
				result = lhs < rhs;
				break;
			case FILE_TIME:
				result = std::filesystem::last_write_time(lhs) < std::filesystem::last_write_time(rhs);
				break;
			case FILE_SIZE:
				result = std::filesystem::file_size(lhs) < std::filesystem::file_size(rhs);
				break;
			default:
				break;
			}
			return is_reverse ? !result : result;
		});

	return filenames;
}

bool FileSystem::Exists(const std::string& path)
{
    std::string relativePath = this->root.generic_string();
    relativePath.append(path);

	return std::filesystem::exists(relativePath);
}

bool FileSystem::IsFile(const std::string& path)
{
    std::string relativePath = this->root.generic_string();
    relativePath.append(path);

	return std::filesystem::is_regular_file(relativePath);
}

bool FileSystem::IsDir(const std::string& path)
{
    std::string relativePath = this->root.generic_string();
    relativePath.append(path);

	return std::filesystem::is_directory(relativePath);
}

bool FileSystem::IsEmptyDir(const std::string& path)
{
    std::string relativePath = this->root.generic_string();
    relativePath.append(path);

	return relativePath.empty();
}

bool FileSystem::CreateFile(const std::string& filename, std::ios_base::openmode mode)
{
	if (Exists(filename))
	{
		return false;
	}

    std::string name = this->root.generic_string();
    name.append(filename);

	std::ofstream ofs(name, mode);
	ofs.close();
	return true;
}

bool FileSystem::CreateDir(const std::string& path, bool is_recursive)
{
	if (Exists(path))
	{
		return false;
	}

    std::filesystem::path relativePath = this->root;
    relativePath.append(path);

	if (is_recursive)
	{
		return std::filesystem::create_directories(relativePath);
	}
	return std::filesystem::create_directory(relativePath);
}

bool FileSystem::RemoveFile(const std::string& filename)
{
	if (!Exists(filename))
	{
		return false;
	}

    std::string name = this->root.generic_string();
    name.append(filename);

	return std::filesystem::remove(name);
}

bool FileSystem::RemoveDir(const std::string& path, bool is_recursive)
{
	if (!Exists(path))
	{
		return false;
	}

    std::filesystem::path relativePath = this->root;
    relativePath.append(path);

	if (is_recursive)
	{
		return std::filesystem::remove_all(relativePath) > 0;
	}
	return std::filesystem::remove(relativePath);
}

void FileSystem::CopyFile(const std::string& from, const std::string& to)
{
    std::string relativeFrom = this->root.generic_string();
    relativeFrom.append(from);
    std::string relativeTo = this->root.generic_string();
    relativeTo.append(from);

	std::filesystem::copy(relativeFrom, relativeTo, std::filesystem::copy_options::overwrite_existing);
}

void FileSystem::RenameFile(const std::string& dir, const std::string& old_name, const std::string& new_name)
{
	if (old_name.compare(new_name))
	{
		try
		{
            std::string relativePath = this->root.generic_string();
            relativePath.append(dir);

			std::filesystem::rename(relativePath + old_name, relativePath + new_name);
		}
		catch (const std::filesystem::filesystem_error& e)
		{
            ENGINE_LOG_WARN("Rename file error!");
		}
	}
}

bool FileSystem::LoadBinary(const std::string& filename, std::vector<uint8_t>& data)
{
    std::string name = this->root.generic_string();
    name.append(filename);

	std::ifstream file(name, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
        ENGINE_LOG_WARN("Failed to load binary file %s!", filename.c_str());
		return false;
	}

	size_t file_size = (size_t)file.tellg();
	data.resize(file_size);

	file.seekg(0);
	file.read((char*)data.data(), file_size);
	file.close();

	return true;
}

bool FileSystem::WriteString(const std::string& filename, const std::string& str)
{
    std::string name = this->root.generic_string();
    name.append(filename);

	std::ofstream file(name);
	if (!file.is_open())
	{
        ENGINE_LOG_WARN("Failed to write string file %s!", filename.c_str());
		return false;
	}

	file << str;
	file.close();

	return true;
}

bool FileSystem::LoadString(const std::string& filename, std::string& str)
{
    std::string name = this->root.generic_string();
    name.append(filename);

	std::ifstream file(name);
	if (!file.is_open())
	{
        ENGINE_LOG_WARN("Failed to load string file %s!", filename.c_str());
		return false;
	}

	file >> str;
	file.close();

	return true;
}