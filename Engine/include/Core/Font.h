#ifndef FDBCOREFONTF
#define FDBCOREFONTF
#include <string>
#include <vector>
#include <memory>
#include <glm/vec2.hpp>

#include "array"
#include "Texture.h"


namespace FDB::Core
{
	


    struct Ascii
    {
        Ascii();
        Ascii(char c);
        Ascii(int i);
    
        [[nodiscard]] int ToInt() const;
        [[nodiscard]] char ToChar() const;
    
        [[nodiscard]] static constexpr int ToInt(char c);
        [[nodiscard]] static constexpr char ToChar(int i);
        [[nodiscard]] static constexpr bool IsValid(char c);
    private:
        char m_data = ' ';
    };
    
    struct Glyph {
        Ascii code;
        float advance;
        glm::vec2 size;
        glm::vec2 offset;
        glm::vec2 min;
        glm::vec2 max;
    };
    
    
    
    
    struct AtlasFont
    {
        AtlasFont(FDB::Core::Texture&& image, const std::array<Glyph, 95>& glyphs, int pixelSize, float ascent, float descent);
        [[nodiscard]] Glyph GetGlyph(const Ascii& asci) const;
        [[nodiscard]] glm::ivec2 Size() const;
        [[nodiscard]] int pixSize() const;
        [[nodiscard]] float GetAscent() const;
        [[nodiscard]] float GetDescent() const;
		[[nodiscard]] FDB::Core::Texture* GetTexture();
    private:
        FDB::Core::Texture m_texture;
        int m_pixelSize = 0;
        float m_ascent;
        float m_descent;
        std::array<Glyph, 95> m_glyphs = {};
    };
    
    struct AtlasFontHelper
    {
    	[[nodiscard]] static std::unique_ptr<AtlasFont> LoadFont(const std::filesystem::path& filePath, float res = 1.0f);
    };
    
    using fontLoader_t = resourceManager_t< FDB::Core::AtlasFont, TypeWrapper<float>, FDB::Core::AtlasFontHelper::LoadFont>;

}

#endif
