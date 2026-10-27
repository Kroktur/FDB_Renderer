#ifndef KTR_RTTI_F
#define KTR_RTTI_F

#include <atomic>
#include <string_view>
#include "KTR/Tools/Hash/KTR_Hasher.h"
#include "KTR/Tools/META/KTR_MetaTools.h"
#include "KTR/KTR_congif_compiler.h"
namespace KTR
{
	template<typename T>
	struct TypeInfo;
}

#define KTR_REGISTER_RTTI(T)						                            \
namespace KTR										                            \
{													                            \
	template<>										                            \
	struct TypeInfo<T>								                            \
	{												                            \
		using hash_type =  KTR::hash::default_hasher_type<std::string_view>;	    \
		using hash_result_type = hash_type::result_type;                        \
		[[nodiscard]] static constexpr std::string_view Name()                  \
		{											                            \
			return #T;								                            \
		}											                            \
		[[nodiscard]] static constexpr hash_result_type Id()                    \
		{											                            \
			 static hash_result_type hash = hash_type::Hash(TypeInfo::Name());  \
			 return hash;                                                       \
		}																		\
	};												                            \
}

template<typename T>
concept RTTIType  = requires()
{
	{ KTR::TypeInfo<T>::Name() };
	{ KTR::TypeInfo<T>::Id() };
};

KTR_DEFINE_HAS_MEMBER_TYPE(has_rtti_tag_type, rtti_tag_type)

namespace KTR
{
	namespace rtti
	{
		#ifdef KTR_64B 
		using atomic_type = std::atomic_uint64_t;
		using value_type = std::uint64_t;
		#elifdef KTR_32B
		using atomic_type = std::atomic_uint32_t;
		using value_type = std::uint32_t;
		#endif

		struct Counter
		{
			template<typename T>
			[[nodiscard]] static value_type GetId();

		private:
			static atomic_type m_id;
		};

		template<typename TagT> requires (has_rtti_tag_type_v<TagT>)
		struct DedicatedCounter
		{
			using tag_type = TagT;
			template<typename T>
			[[nodiscard]] static value_type GetId();

		private:
			static atomic_type m_id;
		};

		
		template <typename T>
		[[nodiscard]] value_type Counter::GetId()
		{
			static value_type id = m_id++;
			return id;
		}

		template <typename TagT>requires (has_rtti_tag_type_v<TagT>)
		template <typename T>
		[[nodiscard]] value_type DedicatedCounter<TagT>::GetId()
		{

			static value_type id = m_id++;
			return id;
		}

		struct Hasher
		{
			template<RTTIType T>
			[[nodiscard]] static value_type GetId();
		};

		template <RTTIType T>
		[[nodiscard]] value_type Hasher::GetId()
		{
			return TypeInfo<T>::Id();
		}

		template<typename T>
		struct Instance
		{
			Instance();
			[[nodiscard]] value_type GetInstance() const;
			[[nodiscard]] static value_type GetInstanceCount();

		private:
			value_type m_instanceID;
			static atomic_type m_lastInstance;
		};

		template <typename T>
		Instance<T>::Instance(): m_instanceID(m_lastInstance++)
		{}

		template <typename T>
		value_type Instance<T>::GetInstance() const
		{
			return m_instanceID;
		}

		template <typename T>
		value_type Instance<T>::GetInstanceCount()
		{
			return m_lastInstance;
		}

		template<typename T>
		atomic_type Instance<T>::m_lastInstance = 0;
	}

	inline rtti::atomic_type rtti::Counter::m_id = 0;
	template<typename TagT> requires (has_rtti_tag_type_v<TagT>)
	inline rtti::atomic_type rtti::DedicatedCounter<TagT>::m_id = 0;
}




#endif
