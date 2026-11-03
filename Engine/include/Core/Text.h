#ifndef FDBCORETEXTF
#define FDBCORETEXTF
#include "Core/Font.h"
#include <vector>

#include "Font.h"
#include "Ui.h"


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
	public:
		enum class Align
		{
			Center,
			Left,
			Right
		};
		Text() = default;
		Text(const Text&) = default;
		Text(Text&&) noexcept = default;
		Text& operator=(const Text&) = default;
		Text& operator=(Text&&) noexcept = default;
		~Text() = default;
	public:

		void SetText(const std::string& text);
		void SetAlign(const Align& align);
		[[nodiscard]] std::string GetText() const;
		[[nodiscard]] Align GetAlign() const;
		void SetTexture(FDB::Core::Texture* texture);
		[[nodiscard]] FDB::Core::Texture* GetTexture() const;
		void SetFont(AtlasFont* font);
		[[nodiscard]] AtlasFont* GetFont() const;
		[[nodiscard]] bufferData GetBufferData();
	private:
		[[nodiscard]] static float Offset(const Align& align, float totalAdvance, float currentAdvance);
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

#endif
