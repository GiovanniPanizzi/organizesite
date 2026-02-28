#include <vector>
#include <filesystem>

class DirectoryManager {
private:
	std::vector<std::filesystem::path> directories;
public:
	DirectoryManager();
	~DirectoryManager();
	std::vector<std::filesystem::path> findFilesByExtention(const std::filesystem::path& path, const std::string& extension);
	std::vector<std::filesystem::path> findFilesByName(const std::filesystem::path& path, const std::string& name);
	bool createDirectory(const std::filesystem::path& path, const std::string& directoryName);
};
