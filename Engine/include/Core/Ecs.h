#ifndef FDBCOREECSF
#define FDBCOREECSF
#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>
#include <KTR/Tools/KTR_RTTI.h>

namespace FDB::Core
{
	using entity = std::uint64_t;
	static constexpr entity invalidEntity = std::numeric_limits<entity>::max();


	struct BaseContainer
	{
		virtual ~BaseContainer() = default;
		[[nodiscard]] virtual bool Has(entity e) const = 0;
		virtual void Remove(entity e) = 0;
	};

	template<typename T>
	struct Container : BaseContainer
	{
		[[nodiscard]] bool Has(entity e) const  override
		{
			if (e >= m_sparse.size())
				return false;
			const entity index = m_sparse[e];
			return index != invalidEntity && m_dense[index] == e;
		}
		void Remove(entity e) override
		{
			if (!this->Has(e))
				return;

			entity index = m_sparse[e];

			if (m_dense.size() != 1)
			{
				entity lastEntity = m_dense.back();
				T lastT = std::move(m_components.back());
				m_dense[index] = lastEntity;
				m_components[index] = std::move(lastT);
				m_sparse[lastEntity] = index;
			}

			m_dense.pop_back();
			m_components.pop_back();
			m_sparse[e] = invalidEntity;
		}
		void Set(entity e, T&& component)
		{
			if (m_sparse.size() <= e)
				m_sparse.resize(e + 1, invalidEntity);

			if (this->Has(e))
			{
				m_components[m_sparse[e]] = std::move(component);
			}
			else
			{
				m_sparse[e] = m_dense.size();
				m_dense.push_back(e);
				m_components.push_back(std::move(component));
			}
		}
		[[nodiscard]] T& operator[](entity e)
		{
			return m_components[m_sparse[e]];
		}
		[[nodiscard]] const T& operator[](entity e) const
		{
			return m_components[m_sparse[e]];
		}
		[[nodiscard]] std::vector<T>& GetComponents()
		{
			return m_components;
		}
		[[nodiscard]] const std::vector<entity>& GetEntities() const
		{
			return m_dense;
		}
	private:
		std::vector<entity> m_sparse;
		std::vector<entity> m_dense;
		std::vector<T> m_components;
	};

	struct EcsRTTI
	{
		using rtti_tag_type = void;
	};



	struct Registry
	{
		entity CreateEntity();

		void DestroyEntity(entity e);

		template<typename T>
		[[nodiscard]] bool HasComponent(entity e)
		{
			return GetContainer<T>().Has(e);
		}

		template<typename T>
		void RemoveComponent(entity e)
		{
			GetContainer<T>().Remove(e);
		}
		template<typename ...T> requires(sizeof...(T) > 1)
			void RemoveComponents(entity e)
		{
			(..., RemoveComponent<T>(e));
		}

		template<typename T>
		void AddComponent(entity e, T&& component)
		{
			GetContainer<T>().Set(e, std::move(component));
		}
		template<typename ...T> requires(sizeof...(T) > 1)
			void AddComponents(entity e, T&&... components)
		{
			(..., AddComponent<T>(e, std::move(components)));
		}

		template<typename T>
		Container<T>& GetContainer()
		{
			const std::size_t id = KTR::rtti::DedicatedCounter<EcsRTTI>::GetId<T>();
			if (m_containers.size() <= id)
				m_containers.resize(id + 1);
			if (!m_containers[id])
				m_containers[id] = std::make_unique<Container<T>>();
			return static_cast<Container<T>&>(*m_containers[id]);
		}

		template<typename ...T>
		const std::vector<entity>& GetEntitiesView()
		{
			const std::vector<entity>* smallestEntity = nullptr;

			(..., [&]()
				{
					auto& entities = GetContainer<T>().GetEntities();
					if (!smallestEntity || entities.size() < smallestEntity->size())
						smallestEntity = &entities;
				}());

			m_lastView.clear();
			m_lastView.reserve(smallestEntity->size());
			for (auto& e : *smallestEntity)
			{
				if ((... && HasComponent<T>(e)))
					m_lastView.push_back(e);
			}
			return m_lastView;
		}

	private:
		std::vector<entity> m_eRecycled;
		entity m_nextEntity = 0;
		std::vector<entity> m_lastView;
		std::vector < std::unique_ptr<BaseContainer>> m_containers;
	};


}

#endif
