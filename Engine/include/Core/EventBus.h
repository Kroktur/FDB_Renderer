#ifndef FDB_CORE_EVENTBUSF
#define FDB_CORE_EVENTBUSF

#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace FDB::Pipeline
{
	template<typename EventType>
	struct EventBus
	{
		using event_type = const EventType&;
		using internalCallBack_t = void(*)(void*, event_type);
		using id_type = std::uint32_t;
		static constexpr  id_type invalidHandle = std::numeric_limits<id_type>::max();
		struct Subscription
		{
			internalCallBack_t callBack = nullptr;
			void* instance = nullptr;
			id_type gen = 0;
		};
		struct BusHandle
		{
			id_type id = invalidHandle;
			id_type gen = invalidHandle;
			[[nodiscard]] operator bool() const
			{
				return id != invalidHandle && gen != invalidHandle;
			}
		};

		static BusHandle UnSubscribe(const BusHandle& handle)
		{
			if (m_subscriptions.size() <= handle.id)
				return {};
			Subscription& callBackContainer = m_subscriptions[handle.id];
			if (callBackContainer.gen != handle.gen)
				return {};

			++callBackContainer.gen;

			callBackContainer.instance = nullptr;
			callBackContainer.callBack = nullptr;

			m_recycledIds.push_back(handle.id);
			return {};
		}
		template<typename T, void(T::* fn)(event_type)>
		[[nodiscard]] static BusHandle SubScribe(T* obj)
		{
			if (!obj)
				return BusHandle{};
			BusHandle handle = AcquireHandle();
			Subscription& callBackContainer = m_subscriptions[handle.id];
			callBackContainer.instance = static_cast<void*>(obj);
			callBackContainer.callBack = [](void* obj_, event_type event)
				{
					(static_cast<T*>(obj_)->*fn)(event);
				};
			return handle;
		}
		template< void(*fn)(event_type)>
		[[nodiscard]] static  BusHandle SubScribe()
		{
			BusHandle handle = AcquireHandle();
			Subscription& callBackContainer = m_subscriptions[handle.id];
			callBackContainer.instance = nullptr;
			callBackContainer.callBack = [](void* obj_, event_type event)
				{
					fn(event);
				};
			return handle;
		}
		template<typename T> requires(std::is_invocable_v<T, event_type>)
			[[nodiscard]] static  BusHandle SubScribe(T* fn)
		{
			BusHandle handle = AcquireHandle();
			Subscription& callBackContainer = m_subscriptions[handle.id];
			callBackContainer.instance = static_cast<void*>(fn);
			callBackContainer.callBack = [](void* obj_, event_type event)
				{
					(*static_cast<T*>(obj_))(event);
				};
			return handle;
		}
		static void Notify(event_type event)
		{
			for (Subscription& subscription : m_subscriptions)
			{
				if (subscription.callBack)
					subscription.callBack(subscription.instance, event);
			}
		}
	private:
		[[nodiscard]] static  BusHandle AcquireHandle()
		{

			if (!m_recycledIds.empty())
			{
				id_type  id = m_recycledIds.back();
				m_recycledIds.pop_back();
				return  BusHandle{ id,m_subscriptions[id].gen };
			}
				id_type id = m_subscriptions.size();
				m_subscriptions.push_back(Subscription{});
				return  BusHandle{ id,m_subscriptions[id].gen };
			

		}
		static std::vector<Subscription> m_subscriptions;
		static std::vector<id_type> m_recycledIds;

	};
	template<typename T>
	inline std::vector<typename EventBus<T>::Subscription> EventBus<T>::m_subscriptions = {};
	template<typename T>
	inline std::vector<typename EventBus<T>::id_type> EventBus<T>::m_recycledIds = {};
}
#endif
