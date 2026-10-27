#ifndef FDBCORERESOURCEMANAGERF
#define FDBCORERESOURCEMANAGERF


#include <filesystem>
#include <map>

struct FilePathSystem
{
	inline static void AddFilePath(const std::filesystem::path& view)
	{
		m_allPath.emplace_back(view);
	}
	inline [[nodiscard]] static std::filesystem::path GetAbsoluteFilePath(const std::filesystem::path& file)
	{
		for (auto& path : m_allPath)
		{
			const std::filesystem::path fullPath = path / file;

			if (std::filesystem::exists(fullPath))
				return fullPath;
		}
		throw std::out_of_range(
			"File not found: " + file.string());
	}
private:
	static std::vector<std::filesystem::path> m_allPath;
};
inline std::vector<std::filesystem::path> FilePathSystem::m_allPath = {};

template<typename ...T>
struct TypeWrapper {};

template<typename TextureType, typename ConstructArgs, auto FN>
struct ResourceManager;

template<typename TextureType,
	typename... ConstructArgs,
	std::unique_ptr<TextureType>(*FN)(const std::filesystem::path&, ConstructArgs...)>
struct ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>
{
public:
	using texture_ptr = std::unique_ptr<TextureType>;
	using texture_raw_ptr = TextureType*;
	[[nodiscard]] static texture_raw_ptr Load(const std::filesystem::path& path, ConstructArgs... args)
	{
		if (m_textureMap[path] != nullptr)
			return m_textureMap[path].get();
		std::filesystem::path fullPath = FilePathSystem::GetAbsoluteFilePath(path);
		texture_ptr texture = std::move(FN(path, args...));
		if (!texture)
			 throw std::out_of_range("texture null");
		m_textureMap[path] = std::move(texture);
		return m_textureMap[path].get();
	}
private:
	static std::map<std::filesystem::path, texture_ptr> m_textureMap;
};



template<typename TextureType, typename... ConstructArgs,
	std::unique_ptr<TextureType>(*FN)(const std::filesystem::path&, ConstructArgs...)>
inline std::map<std::filesystem::path, std::unique_ptr<TextureType>>
ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::m_textureMap =
std::map<std::filesystem::path, std::unique_ptr<TextureType>>{};



#endif
