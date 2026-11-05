#include "Core/Ecs.h"

FDB::Core::entity FDB::Core::Registry::CreateEntity()
{
	if (!m_eRecycled.empty())
	{
		entity e = m_eRecycled.back();
		m_eRecycled.pop_back();
		return e;
	}
	return m_nextEntity++;
}

void FDB::Core::Registry::DestroyEntity(entity e)
{
	for (auto& pool : m_containers)
	{
		if (pool)
			pool->Remove(e);
	}
	m_eRecycled.push_back(e);
}
