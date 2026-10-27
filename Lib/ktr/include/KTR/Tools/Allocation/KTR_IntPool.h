#ifndef KTR_INTPOOL_F
#define KTR_INTPOOL_F


#include "KTR_Allocator.h"
#include "Containers/KTR_SparseSet.h"

namespace KTR::allocation
{
	template<typename T, ValidRebindAllocator allocatorT = default_allocator_type<T>> requires(std::is_integral_v<T>)
		struct IntPool
	{
		using value_type = T;
		using allocator_type = allocatorT;
		using sparse_type = container::IntSparseSet<value_type, allocator_type>;
		static constexpr value_type invalidValue = std::numeric_limits<value_type>::max();
	public:
		IntPool() : m_nextId(0) {}
		IntPool(const allocator_type& alloc) : m_nextId(0), m_sparse(alloc) {}
		IntPool(const IntPool&) = default;
		IntPool(IntPool&&) noexcept = default;
		~IntPool() = default;
		IntPool& operator=(const IntPool&) = default;
		IntPool& operator=(IntPool&&) noexcept = default;
	public:
		[[nodiscard]] value_type Acquire()
		{
			if (!m_sparse.Empty())
			{
				value_type id = m_sparse.Back();
				m_sparse.PopBack();
				return id;
			}
			KTR_ASSERT(m_nextId != invalidValue, "Nod ids available");
			value_type id = m_nextId++;
			return id;
		}

		template<std::size_t sizeT> requires(sizeT < static_cast<std::size_t>(invalidValue))
			[[nodiscard]] std::array<value_type, sizeT> Acquire()
		{
			std::array<value_type, sizeT> ids;
			for (std::size_t i = 0; i < sizeT; ++i)
			{
				ids[i] = this->Acquire();
			}
			return ids;
		}
		struct AcquireResult
		{
			value_type first;
			value_type last;
			[[nodiscard]] std::size_t Size() const
			{
				return static_cast<std::size_t>(last - first) + 1;
			}
			[[nodiscard]] bool Empty() const
			{
				return first == last;
			}
		};

		[[nodiscard]] AcquireResult AcquireRange(std::size_t size)
		{
			KTR_ASSERT(size < static_cast<std::size_t>(invalidValue) && (m_nextId <= invalidValue - size), "Invalid range size");
			AcquireResult result;
			result.first = m_nextId;
			m_nextId += size;
			result.last = m_nextId - 1;
			return result;
		}

		bool Recycle(value_type id)
		{
			return m_sparse.Insert(id);
		}

		void Clear()
		{
			m_nextId = 0;
			m_sparse.Clear();
		}

		[[nodiscard]] bool IsRecycled(value_type id) const
		{
			return id < m_nextId && m_sparse.Has(id);
		}

		[[nodiscard]] static constexpr std::size_t MaxSize()
		{
			return invalidValue;
		}

		[[nodiscard]] std::size_t NextId() const
		{
			return m_nextId;
		}
		[[nodiscard]] std::size_t FreeCount() const
		{
			return m_sparse.Size();
		}
		[[nodiscard]] std::size_t UseCount() const
		{
			return this->NextId() - this->FreeCount();
		}
		void Reserve(std::size_t capacity)
		{
			m_sparse.Reserve(capacity);
		}
	private:
		value_type m_nextId;
		sparse_type m_sparse;
	};

	template<typename T, std::size_t sizeT> requires(std::is_integral_v<T>&& sizeT < std::numeric_limits<T>::max())
		struct FixedIntPool
	{
		using value_type = T;
		static constexpr  std::size_t size_v = sizeT;
		static constexpr value_type invalidValue = std::numeric_limits<value_type>::max();

		using sparse_type = container::FixedIntSparseSet<value_type, size_v>;
	public:
		FixedIntPool() : m_nextId(0) {}
		FixedIntPool(const FixedIntPool&) = default;
		FixedIntPool(FixedIntPool&&) noexcept = default;
		~FixedIntPool() = default;
		FixedIntPool& operator=(const FixedIntPool&) = default;
		FixedIntPool& operator=(FixedIntPool&&) noexcept = default;
	public:
		[[nodiscard]] value_type Acquire()
		{
			if (!m_sparse.Empty())
			{
				value_type id = m_sparse.Back();
				m_sparse.Erase(id);
				return id;
			}
			KTR_ASSERT(m_nextId < size_v, "Nod ids available");
			value_type id = m_nextId++;
			return id;
		}

		template<std::size_t sizeT2> requires(sizeT2 <= static_cast<std::size_t>(size_v))
			[[nodiscard]] std::array<value_type, sizeT2> Acquire()
		{
			std::array<value_type, sizeT2> ids;
			for (std::size_t i = 0; i < sizeT2; ++i)
			{
				ids[i] = this->Acquire();
			}
			return ids;
		}
		struct AcquireResult
		{
			value_type first;
			value_type last;
			[[nodiscard]] std::size_t Size() const
			{
				return static_cast<std::size_t>(last - first) + 1;
			}
			[[nodiscard]] bool Empty() const
			{
				return last == invalidValue;
			}
		};

		[[nodiscard]] AcquireResult AcquireRange(std::size_t size)
		{
			KTR_ASSERT(size <= static_cast<std::size_t>(size_v) && (m_nextId <= size_v - size), "Invalid range size");
			AcquireResult result;
			result.first = m_nextId;
			m_nextId += size;
			result.last = size == 0 ? invalidValue : m_nextId - 1;
			return result;
		}

		bool Recycle(value_type id)
		{
			return m_sparse.Insert(id);
		}

		void Clear()
		{
			m_nextId = 0;
			m_sparse.Clear();
		}

		[[nodiscard]] bool IsRecycled(value_type id) const
		{
			return id < m_nextId && m_sparse.Has(id);
		}


		[[nodiscard]] static constexpr std::size_t MaxSize()
		{
			return size_v;
		}

		[[nodiscard]] std::size_t NextId() const
		{
			return m_nextId;
		}
		[[nodiscard]] std::size_t FreeCount() const
		{
			return m_sparse.Size();
		}
		[[nodiscard]] std::size_t UseCount() const
		{
			return this->NextId() - this->FreeCount();
		}
	private:
		value_type m_nextId;
		sparse_type m_sparse;
	};

}

#endif
