#include "stb_image_write.h"
#include "stb_truetype.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Atlas;
class Canvas;
class Region;
class TextRenderer;

/// \brief 像素
struct Pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    /// \brief 混合两个像素的颜色
    /// \param other 另一个像素
    void blend(const Pixel& other)
    {
        unsigned int alpha = other.a;
        unsigned int invAlpha = 255 - alpha;

        r = static_cast<unsigned char>((r * invAlpha + other.r * alpha) / 255);
        g = static_cast<unsigned char>((g * invAlpha + other.g * alpha) / 255);
        b = static_cast<unsigned char>((b * invAlpha + other.b * alpha) / 255);
        a = static_cast<unsigned char>((a * invAlpha + other.a * alpha) / 255);
    }

    /// \brief lerp 线性插值
    /// \param a 起始颜色
    /// \param b 结束颜色
    /// \param t 插值因子
    /// \return 插值结果
    static struct Pixel lerp(const Pixel& a, const Pixel& b, float t)
    {
        Pixel result;
        result.r = static_cast<unsigned char>(a.r * (1 - t) + b.r * t);
        result.g = static_cast<unsigned char>(a.g * (1 - t) + b.g * t);
        result.b = static_cast<unsigned char>(a.b * (1 - t) + b.b * t);
        result.a = static_cast<unsigned char>(a.a * (1 - t) + b.a * t);
        return result;
    }
};

class Region {
public:
    Region() = default;
    Region(Canvas* canvas, int startX, int startY, int width, int height);
    bool isEmpty() const;
    Pixel& operator()(int row, int col);
    const Pixel& operator()(int row, int col) const;
    Region& operator=(const Region& other);
    void blendTo(Region& other);
    void copyTo(Region& other);
    std::pair<int, int> size() const;

private:
    Canvas* mCanvas{nullptr};
    int mStartX{0};
    int mStartY{0};
    int mWidth{0};
    int mHeight{0};
    int mStride{0};
    Pixel* mData{0};
};

class Canvas {
public:
    Canvas() = default;
    Canvas(int width, int height);
    unsigned char* data();
    Pixel* pixels();
    Pixel* pixelPtr(int x, int y);
    Region operator()(int startX, int startY, int width, int height);
    int width() const;
    int height() const;
    int stride() const;
    int strideBytes() const;
    void clear();
    void resize(int width, int height);
    void saveToPNG(const std::string& filename);

private:
    std::vector<Pixel> mPixels{};
    int mWidth{0};
    int mHeight{0};
};

struct GlyphInfo
{
    int x;
    int y;
    int width;
    int height;
    int offsetX;
    int offsetY;
};

class Atlas {
private:
    Canvas canvas;
    std::unordered_map<int, GlyphInfo> info;
    int currentRowX;
    int currentRowY;
    int currentRowHeight;

public:
    Atlas(int width, int height);
    int width() const;
    int height() const;
    GlyphInfo addGlyph(int codepoint, const unsigned char* glyph,
                       int glyphWidth, int glyphHeight, int offsetX,
                       int offsetY);
    // GlyphInfo addGlyphWithOutline(int codepoint, const unsigned char* glyph,
    //                               int glyphWidth, int glyphHeight, int
    //                               offsetX, int offsetY, const Pixel&
    //                               outlineColor);
    Region getGlyph(int codepoint, GlyphInfo* glyphInfo);
    void saveToPNG(const std::string& filename);
};

class TextRenderer {
private:
    stbtt_fontinfo font;
    std::vector<unsigned char> fontBuffer;
    Canvas canvas;
    Atlas atlas;
    int bitmapWidth;
    int bitmapHeight;

public:
    TextRenderer(int atlasWidth, int atlasHeight);

    bool loadFont(const char* fontFile);
    void createBitmap(int width, int height);
    void renderText(const std::wstring& text, int x, int y, int fontSize);
    void saveToPNG(const std::string& filename);
    void saveAtlasToPNG(const std::string& filename);
};
