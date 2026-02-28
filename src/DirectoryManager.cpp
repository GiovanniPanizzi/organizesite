#include "../include/DirectoryManager.hpp"
#include <iostream>

DirectoryManager::DirectoryManager() {
}

DirectoryManager::~DirectoryManager() {
}

std::vector<std::filesystem::path> DirectoryManager::findFilesByExtention(const std::filesystem::path& path, const std::string& extension) {
	std::vector<std::filesystem::path> foundFiles;
	if(std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
		for(const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if(entry.is_regular_file() && entry.path().extension() == extension) {
				foundFiles.push_back(entry.path());
			}
		}
	}
	return foundFiles;
}

std::vector<std::filesystem::path> DirectoryManager::findFilesByName(const std::filesystem::path& path, const std::string& name) {
	std::vector<std::filesystem::path> foundFiles;
	if(std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
		for(const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if(entry.is_regular_file() && entry.path().filename() == name) {
				foundFiles.push_back(entry.path());
			}
		}
	}
	return foundFiles;
}

// If the directory already exists it return false, otherwise it creates the directory and return true
bool DirectoryManager::createDirectory(const std::filesystem::path& path, const std::string& directoryName) {
	std::filesystem::path newDir = path / directoryName;
	if(!std::filesystem::exists(newDir)) {
		std::filesystem::create_directory(newDir);
		std::cout << "Directory created: " << newDir << std::endl;
		return true;
	}
	else{
		std::cout << "Directory already exists: " << newDir << std::endl;
		return false;
	}
}