#ifndef FDBCOREMESH2DF
#define FDBCOREMESH2DF
#include <vector>

#include "Text.h"
#include "Texture.h"
#include "Ui.h"

namespace FDB::Core
{
	struct MeshData2d
	{
		std::vector<FDB::Core::VertexUi> vertices;
		std::vector<std::uint32_t> indices;
	};

	static MeshData2d square = {
		{
			FDB::Core::VertexUi{ { -0.5f, -0.5f }, { 0.f, 1.f },{ 0.f, 1.f } },
			FDB::Core::VertexUi{ { 0.5f, -0.5f}, { 1.f, 1.f },{ 1.f, 1.f } },
			FDB::Core::VertexUi{ { 0.5f, 0.5f }, { 1.f, 0.f },{ 1.f, 0.f } },
			FDB::Core::VertexUi{ { -0.5f, 0.5f }, { 0.f, 0.f },{ 0.f, 0.f } }
		},
		{
			0, 1, 2,
			2, 3, 0
		}
	};

	struct Mesh2D
	{
		MeshData2d* data = nullptr;
		FDB::Core::Texture* texture = nullptr;
		FDB::Core::bufferData GetBufferData() const;
	};
}

#endif
