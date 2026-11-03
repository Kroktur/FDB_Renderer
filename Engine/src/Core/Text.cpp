#include "Core/Text.h"



void FDB::Core::Text::SetText(const std::string& text)
{
	if (text == m_message)
		return;
	m_message =  text;
	m_dirty = true;
}

void FDB::Core::Text::SetAlign(const Align& align)
{
	if (align == m_align)
		return;
	m_align = align;
	m_dirty = true;
}

std::string FDB::Core::Text::GetText() const
{
	return  m_message;
}

FDB::Core::Text::Align FDB::Core::Text::GetAlign() const
{
	return m_align;
}

FDB::Core::bufferData FDB::Core::Text::GetBufferData()
{
	if (m_dirty)
	{
		Upload();
		m_dirty = false;
	}
	return { m_vertices.data(), m_indices.data(), m_indices.size() };
}

void FDB::Core::Text::SetTexture(FDB::Core::Texture* texture)
{
	m_textTexture = texture;
}

FDB::Core::Texture* FDB::Core::Text::GetTexture() const
{
	return m_textTexture;
}

void FDB::Core::Text::SetFont(AtlasFont* font)
{
	if (m_font == font)
		return;
	m_font = font;
	m_dirty = true;
}

FDB::Core::AtlasFont* FDB::Core::Text::GetFont() const
{
	return m_font;
}

float FDB::Core::Text::Offset(const Align& align, float totalAdvance, float currentAdvance)
{
	switch (align)
	{
	case  Align::Left:
		return 0.0f;
	case Align::Center:
		return (totalAdvance - currentAdvance) / 2.0f;
	case Align::Right:
		return (totalAdvance - currentAdvance);
	}
	throw std::out_of_range("case not handle");
}

void FDB::Core::Text::Upload()
{

	m_vertices.clear();
	m_indices.clear();
	// create the temp vector for vertices and index
	if (m_message.empty())
	{
		return;
	}

	// create the vector of screen ( to support \n )
	std::vector<std::string> splitMessage;
	// and push a first string in it
	splitMessage.emplace_back();
	// for all letter 
	for (auto& m : m_message)
	{
		// if it's not a \n just add the char to the last string
		if (m != '\n')
			splitMessage.back().push_back(m);
		// else add a new string to the vector
		else
			splitMessage.emplace_back();
	}
	// calculate here the advance total of the text base on pixel size
	float totalAdvance = 0.0f;
	std::vector<float> advances;
	for (auto& str : splitMessage)
	{
		// create the local one 
		advances.emplace_back(0.0f);
		for (auto& c : str)
		{
			advances.back() += m_font->GetGlyph(c).advance;
		}
		// and compare it to the global one to store only the longest 
		totalAdvance = std::max(totalAdvance, advances.back());
	}

	// to match with the ui wee need a square of 1 by 1 that start with -0.5 and end with 0.5

	// this is the scale for the x to go from pixel to NDC
	float scaleX = 1.0f / totalAdvance;
	// this is the scale for the y to go from pixel to NDC
	float scaleY = 1.0f / (static_cast<float>(std::abs(m_font->GetAscent()) + m_font->GetDescent()));
	// divide this by the amount of line of the text
	scaleY /= static_cast<float>(splitMessage.size());

	// this is the index to fill the indexBuffer
	uint32_t indexOffset = 0;
	// the number of line 
	float lineNumber = static_cast<float>(splitMessage.size());
	// the scale for the line 
	float lineOffset = 1.0f / lineNumber;


	// now the tricky part the offest.y of a glyph in pixel return a negative value so we need to add
	// this to the bottom of our quad to get the top y 
	// so with that info we now that we need to calculate the bottom of the first line to start the text 
	float startY = 0.5f - (lineNumber - 1) * lineOffset;

	for (size_t row = 0; row < static_cast<size_t>(lineNumber); ++row)
	{
		// the x start to the left of the square 
		float startX = -0.5f;
		for (size_t col = 0; col < splitMessage[row].size(); ++col)
		{
			// get the glyph
			auto glyph = m_font->GetGlyph(splitMessage[row][col]);

			// calculate the start on x
			float offset = Text::Offset(m_align, totalAdvance, advances[row]);
			float minX = startX + glyph.offset.x * scaleX + offset * scaleX;
			// same for y remind that y is negative and descent is how much letters go below the baseline
			// we subtract it to keep line consistency while making sure the text stays inside the quad
			float minY = startY + glyph.offset.y * scaleY - m_font->GetDescent() * scaleY;
			// calculate the end x base on the size 
			float maxX = minX + glyph.size.x * scaleX;
			// same for y
			float maxY = minY + glyph.size.y * scaleY;


			// add the vertices 
			m_vertices.push_back({ {minX, minY,0,0},     {glyph.min.x, glyph.min.y} ,{minX + 0.5f, minY + 0.5f} });
			m_vertices.push_back({ {maxX, minY,0,0},     {glyph.max.x, glyph.min.y},{maxX + 0.5f, minY + 0.5f} });
			m_vertices.push_back({ {maxX, maxY,0,0}, {glyph.max.x, glyph.max.y} ,{maxX + 0.5f, maxY + 0.5f} });
			m_vertices.push_back({ {minX, maxY,0,0}, {glyph.min.x, glyph.max.y} ,{minX + 0.5f, maxY + 0.5f} });

			// add the index 
			m_indices.push_back(indexOffset + 0);
			m_indices.push_back(indexOffset + 2);
			m_indices.push_back(indexOffset + 1);

			m_indices.push_back(indexOffset + 2);
			m_indices.push_back(indexOffset + 0);
			m_indices.push_back(indexOffset + 3);
			// increment by 4 to keep coherence between indices
			indexOffset += 4;
			// advance the text to keep it clear with no big space 
			startX += glyph.advance * scaleX;
		}
		// add the offset to go on the next line 
		startY += lineOffset;
	}
	// create the buffer and set the size for draw 
}
