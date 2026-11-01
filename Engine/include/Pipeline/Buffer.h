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
		Buffer(const glm::vec<2, std::int32_t>& size): m_size(size),m_dataSize(0)
		{
			this->Resize(m_size);
		}
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept
			: m_data(other.m_data),
			m_dataSize(other.m_dataSize),
			m_size(other.m_size)
		{
			other.m_data = nullptr;
			other.m_dataSize = 0;
			other.m_size = { 0,0 };
		}
		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&& other) noexcept
		{
			if (this != &other)
			{
				delete[] m_data;

				m_data =other.m_data;
				other.m_data = nullptr;

				m_dataSize = other.m_dataSize;
				other.m_dataSize = 0;
				m_size = other.m_size;
				other.m_size = { 0,0 };
			}

			return *this;
		}
		~Buffer()
		{
			if (m_data)
				delete[] m_data;
			m_data = nullptr;
		}
	public:
		buffer_view_t GetView()
		{
			return buffer_view_t{ m_data,m_size };
		}
		void Clear(value_type val)
		{
			std::fill(m_data, m_data + m_dataSize, val);
		}
		void Resize(const glm::vec<2, std::int32_t>& size)
		{
			std::size_t oldSize = m_dataSize;
			m_size = size;
			m_dataSize = m_size.x * m_size.y;
			value_type* newPtr = new value_type[m_dataSize];
			for (std::size_t i = 0; i < std::min(oldSize, m_dataSize); ++i)
				newPtr[i] = m_data[i];

			if (m_data)
				delete[] m_data;
			m_data = std::move(newPtr);
		}
		[[nodiscard]] operator bool() const
		{
			return m_dataSize != 0;
		}
		[[nodiscard]] glm::vec<2, std::int32_t> GetSize() const 
		{
			return m_size;
		}
	private:
		value_type* m_data = nullptr;
		std::size_t m_dataSize = 0;
		glm::vec<2, std::int32_t> m_size = {};
	};


	template<typename T>
	struct Buffer<std::atomic<T>>
	{
		using value_type = T;
		
		using buffer_view_t = buffer_view_t<std::atomic<T>>;
		Buffer() = default;
		Buffer(const glm::vec<2, std::int32_t>& size) : m_size(size), m_dataSize(0)
		{
			this->Resize(m_size);
		}
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept
			: m_data(other.m_data),
			m_dataSize(other.m_dataSize),
			m_size(other.m_size)
		{
			other.m_data = nullptr;
			other.m_dataSize = 0;
			other.m_size = { 0,0 };
		}
		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&& other) noexcept
		{
			if (this != &other)
			{
				delete[] m_data;

				m_data = other.m_data;
				other.m_data = nullptr;

				m_dataSize = other.m_dataSize;
				other.m_dataSize = 0;
				m_size = other.m_size;
				other.m_size = { 0,0 };
			}

			return *this;
		}
		~Buffer()
		{
			if (m_data)
				delete[] m_data;
			m_data = nullptr;
		}
	public:
		buffer_view_t GetView()
		{
			return buffer_view_t{ m_data,m_size };
		}
		void Clear(value_type val)
		{
			std::fill(m_data, m_data + m_dataSize, val);
		}
		void Resize(const glm::vec<2, std::int32_t>& size)
		{
			std::size_t oldSize = m_dataSize;
			m_size = size;
			m_dataSize = m_size.x * m_size.y;
			
			std::atomic<value_type>* newPtr = new std::atomic<value_type>[m_dataSize];
			for (std::size_t i = 0; i < std::min(oldSize, m_dataSize); ++i)
				newPtr[i] = m_data[i].load(std::memory_order_acquire);

			if (m_data)
				delete[] m_data;
			m_data = std::move(newPtr);
		}
		[[nodiscard]] operator bool() const
		{
			return m_dataSize != 0;
		}
		[[nodiscard]] glm::vec<2, std::int32_t> GetSize() const
		{
			return m_size;
		}
	private:
		std::atomic<value_type>* m_data = nullptr;
		std::size_t m_dataSize = 0;
		glm::vec<2, std::int32_t> m_size = {};
	};
	template<typename T>
	using buffer_t = Buffer<T>;

}

#endif
