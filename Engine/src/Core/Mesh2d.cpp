#include "Core/Mesh2d.h"

FDB::Core::bufferData FDB::Core::Mesh2D::GetBufferData() const
{
	return { data->vertices.data(), data->indices.data(), data->indices.size() };
}
