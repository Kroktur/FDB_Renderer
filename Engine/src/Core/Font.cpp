#include "Core/Font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <fstream>
#include "Core/Font.h"
#include <iostream>
FDB::Core::Ascii::Ascii() = default;

FDB::Core::Ascii::Ascii(char c) : m_data(c)
{
    if (!IsValid(m_data))
        throw std::out_of_range("char not handle");
}

FDB::Core::Ascii::Ascii(int i) : m_data(ToChar(i))
{
    if (!IsValid(m_data))
        throw std::out_of_range("char not handle");
}

int FDB::Core::Ascii::ToInt() const
{
    return ToInt(m_data);
}

char FDB::Core::Ascii::ToChar() const
{
    return m_data;
}


FDB::Core::AtlasFont::AtlasFont(FDB::Core::Texture&& image,
    const std::array<Glyph, 95>& glyphs, int pixelSize, float ascent, float descent) : m_texture(std::move(image)), m_glyphs(glyphs), m_pixelSize(pixelSize), m_ascent(ascent), m_descent(descent)
{
}

FDB::Core::Glyph FDB::Core::AtlasFont::GetGlyph(const Ascii& asci) const
{
    int index = asci.ToInt() - 32;
    if (index < 0 || index >= 95)
        throw std::out_of_range("ASCII glyph not in atlas");
    return m_glyphs[index];
}

glm::ivec2 FDB::Core::AtlasFont::Size() const
{
    return m_texture.mipLevels[0].size;
}

int FDB::Core::AtlasFont::pixSize() const
{
    return m_pixelSize;
}

float FDB::Core::AtlasFont::GetAscent() const
{
    return m_ascent;
}

float FDB::Core::AtlasFont::GetDescent() const
{
    return m_descent;
}


FDB::Core::Texture* FDB::Core::AtlasFont::GetTexture()
{
    return &m_texture;
}

std::unique_ptr<FDB::Core::AtlasFont> FDB::Core::AtlasFontHelper::LoadFont(const std::filesystem::path& filePath, float res)
{
    std::vector<unsigned char> fontBuffer;
    auto size = glm::ivec2{ 512 * res,512 * res };
    auto pixelSize = 32 * res;

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("Failed to open font file: " + filePath.string());

    size_t sizeFile = file.tellg();
    fontBuffer.resize(sizeFile);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(fontBuffer.data()), static_cast<int>(sizeFile));
    auto atlas = std::vector<unsigned char>{};
    atlas.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y), 0);

    stbtt_bakedchar cdata[95];

    int res2 = stbtt_BakeFontBitmap(
        fontBuffer.data(),
        0,
        static_cast<float>(pixelSize),
        atlas.data(),
        size.x,
        size.y,
        32,
        95,
        cdata
    );

    if (res <= 0)
        throw std::runtime_error("Failed to bake font bitmap.");
    std::array<Glyph, 95> glyphs = {};

    float ascent = std::numeric_limits<float>::max();
    float descent = std::numeric_limits<float>::lowest();

    for (int i = 0; i < 95; ++i)
    {
        Glyph& g = glyphs[i];
        stbtt_bakedchar& bc = cdata[i];

        g.code = Ascii(32 + i);
        g.size = glm::vec2(bc.x1 - bc.x0, bc.y1 - bc.y0);
        g.offset = glm::vec2(bc.xoff, bc.yoff);
        g.advance = bc.xadvance;

        g.min = glm::vec2(static_cast<float>(bc.x0) / static_cast<float>(size.x), static_cast<float>(bc.y0) / static_cast<float>(size.y));
        g.max = glm::vec2(static_cast<float>(bc.x1) / static_cast<float>(size.x), static_cast<float>(bc.y1) / static_cast<float>(size.y));

        ascent = std::min(ascent, g.offset.y);
        descent = std::max(descent, (g.offset.y + g.size.y));
    }

    std::vector<glm::vec<4, std::uint8_t>> rgbaAtlas(size.x * size.y);

    for (size_t i = 0; i < size.x * size.y; ++i)
    {
        unsigned char a = atlas[i];
        rgbaAtlas[i] = glm::u8vec4(255, 255, 255, atlas[i]);
    }


    FDB::Core::Texture texture;
    texture.mipLevels.push_back({ size,0 });
    texture.data = std::move(rgbaAtlas);

    return  std::make_unique<AtlasFont>(std::move(texture), glyphs, pixelSize, ascent, descent);
}



constexpr int FDB::Core::Ascii::ToInt(char c)
{
    return static_cast<int>(c);
}

constexpr char FDB::Core::Ascii::ToChar(int i)
{
    return static_cast<char>(i);
}

constexpr bool FDB::Core::Ascii::IsValid(char c)
{
    return c > 31 && c < 127;
}