#pragma once
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
    
        int ToInt() const;
        char ToChar() const;
    
        static constexpr int ToInt(char c);
        static constexpr char ToChar(int i);
        static constexpr bool IsValid(char c);
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
        Glyph GetGlyph(const Ascii& asci) const;
        glm::ivec2 Size() const;
        int pixSize() const;
        float GetAscent() const;
        float GetDescent() const;
        FDB::Core::Texture* GetTexture();
    private:
        FDB::Core::Texture m_texture;
        int m_pixelSize = 0;
        float m_ascent;
        float m_descent;
        std::array<Glyph, 95> m_glyphs = {};
    };
    
    struct AtlasFontHelper
    {
    	[[nodiscard]] static std::unique_ptr<AtlasFont> LoadFont(const std::string& filePath, float res = 1.0f);
    };
    
    using fontLoader_t = resourceManager_t< FDB::Core::AtlasFont, TypeWrapper<float>, FDB::Core::AtlasFontHelper::LoadFont>;

}