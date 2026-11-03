#pragma once
#include "Core/Font.h"
#include <vector>




struct VertexUi {
	glm::vec4 position;
	glm::vec2 uv;
	glm::vec2 uv2;
};

namespace FDB::Core
{



	struct bufferData
	{
		VertexUi* vertices;
		uint32_t* indices;
		std::size_t indicesCount;
	};


	struct Text
	{
		enum class Align
		{
			Center,
			Left,
			Right
		};
		Text() = default;

		void SetText(const std::string& text);
		void SetAlign(const Align& align);
		std::string GetText() const;
		Align GetAlign() const;
		bufferData GetBufferData();
		void SetTexture(FDB::Core::Texture* texture);
		FDB::Core::Texture* GetTexture() const;
		void SetFont(AtlasFont* font);
		AtlasFont* GetFont() const;
	private:
		static float Offset(const Align& align, float totalAdvance, float currentAdvance);
		std::vector<VertexUi> m_vertices;
		std::vector<uint32_t> m_indices;
		void Upload();
		std::string m_message{ " " };
		bool m_dirty = false;
		Align m_align = Align::Center;
		AtlasFont* m_font = nullptr;
		FDB::Core::Texture* m_textTexture = nullptr;
	};
}

struct TMP2DSHADERS
{
	
};