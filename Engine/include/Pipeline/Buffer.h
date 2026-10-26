#ifndef FDBBUFFERF
#define FDBBUFFERF
#include <cstdint>
#include <vector>
#include <GLM/vec2.hpp>

namespace FDB::Pipeline
{
	template<typename T>
	struct BufferView
	{
		using value_type = T;
		value_type* data;
		glm::vec<2, std::int32_t> size;
		[[nodiscard]] operator bool() const
		{
			return data != nullptr;
		}
	};
	template<typename T>
	using buffer_view_t = BufferView<T>;

	template<typename T>
	struct Buffer
	{
		using value_type = T;
		using buffer_view_t = buffer_view_t<T>;
		Buffer() = default;
		Buffer(const glm::vec<2, std::int32_t>& size): m_size(size)
		{
			this->Resize(m_size);
		}
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) noexcept = default;
		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) noexcept = default;
		~Buffer() = default;
	public:
		buffer_view_t GetView()
		{
			return buffer_view_t{ m_data.data(),m_size };
		}
		void Clear(value_type val)
		{
			std::fill(m_data.begin(), m_data.end(), val);
		}
		void Resize(const glm::vec<2, std::int32_t>& size)
		{
			m_size = size;
			m_data.resize(size.x * size.y);
		}
		[[nodiscard]] operator bool() const
		{
			return m_data.size() != 0;
		}
		[[nodiscard]] glm::vec<2, std::int32_t> GetSize() const 
		{
			return m_size;
		}
	private:
		std::vector<value_type> m_data = {};
		glm::vec<2, std::int32_t> m_size = {};
	};
	template<typename T>
	using buffer_t = Buffer<T>;

}

#endif
