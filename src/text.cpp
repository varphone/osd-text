#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "text.h"
#include <algorithm>

Region::Region(Canvas* canvas, int startX, int startY, int width, int height)
    : mCanvas(canvas)
    , mStartX(startX)
    , mStartY(startY)
    , mWidth(width)
    , mHeight(height)
    , mStride(canvas->width())
    , mData(canvas->pixelPtr(startX, startY))
{
}

bool Region::isEmpty() const
{
    return mWidth == 0 || mHeight == 0;
}

Pixel& Region::operator()(int row, int col)
{
    if (row < 0 || row >= mHeight || col < 0 || col >= mWidth) {
        throw std::out_of_range("Index is out of range");
    }
    return mData[row * mStride + col];
}

const Pixel& Region::operator()(int row, int col) const
{
    if (row < 0 || row >= mHeight || col < 0 || col >= mWidth) {
        throw std::out_of_range("Index is out of range");
    }
    return mData[row * mStride + col];
}

Region& Region::operator=(const Region& other)
{
    if (this != &other) {
        mCanvas = other.mCanvas;
        mStartX = other.mStartX;
        mStartY = other.mStartY;
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mStride = other.mStride;
        mData = other.mData;
    }
    return *this;
}

void Region::blendTo(Region& other)
{
    if (this != &other) {
        for (int i = 0; i < mHeight; ++i) {
            for (int j = 0; j < mWidth; ++j) {
                other(i, j).blend((*this)(i, j));
            }
        }
    }
}

void Region::copyTo(Region& other)
{
    if (this != &other) {
        for (int i = 0; i < mHeight; ++i) {
            for (int j = 0; j < mWidth; ++j) {
                other(i, j) = (*this)(i, j);
            }
        }
    }
}

std::pair<int, int> Region::size() const
{
    return {mWidth, mHeight};
}

Canvas::Canvas(int width, int height) : mWidth(width), mHeight(height)
{
    mPixels.resize(width * height);
    std::fill(mPixels.begin(), mPixels.end(), Pixel{0, 0, 0, 0});
}

unsigned char* Canvas::data()
{
    return (unsigned char*)mPixels.data();
}

Pixel* Canvas::pixels()
{
    return mPixels.data();
}

Pixel* Canvas::pixelPtr(int x, int y)
{
    return &mPixels[y * mWidth + x];
}

Region Canvas::operator()(int startX, int startY, int width, int height)
{
    return Region(this, startX, startY, width, height);
}

int Canvas::width() const
{
    return mWidth;
}

int Canvas::height() const
{
    return mHeight;
}

int Canvas::stride() const
{
    return mWidth;
}

int Canvas::strideBytes() const
{
    return mWidth * sizeof(Pixel);
}

void Canvas::clear()
{
    std::fill(mPixels.begin(), mPixels.end(), Pixel{0, 0, 0, 0});
}

void Canvas::resize(int width, int height)
{
    this->mWidth = width;
    this->mHeight = height;
    mPixels.resize(width * height);
    std::fill(mPixels.begin(), mPixels.end(), Pixel{0, 0, 0, 0});
}

void Canvas::saveToPNG(const std::string& filename)
{
    std::vector<unsigned char> bitmap(mWidth * mHeight * 4);
    for (int i = 0; i < mWidth * mHeight; ++i) {
        bitmap[i * 4] = mPixels[i].r;
        bitmap[i * 4 + 1] = mPixels[i].g;
        bitmap[i * 4 + 2] = mPixels[i].b;
        bitmap[i * 4 + 3] = mPixels[i].a;
    }
    stbi_write_png(filename.c_str(), mWidth, mHeight, 4, bitmap.data(),
                   mWidth * 4);
}

Atlas::Atlas(int width, int height)
    : canvas(width, height), currentRowX(0), currentRowY(0), currentRowHeight(0)
{
    canvas.resize(width, height);
}

int Atlas::width() const
{
    return canvas.width();
}

int Atlas::height() const
{
    return canvas.height();
}

float smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    // Evaluate polynomial
    return x * x * (3 - 2 * x);
}

GlyphInfo Atlas::addGlyph(int codepoint, const unsigned char* glyph,
                          int glyphWidth, int glyphHeight, int offsetX,
                          int offsetY)
{
    if (currentRowX + glyphWidth > canvas.width()) {
        currentRowX = 0;
        currentRowY += currentRowHeight;
        currentRowHeight = 0;
    }

    if (currentRowY + glyphHeight > canvas.height()) {
        // TODO: Handle the case where the atlas is full.
    }

    // 获取当前行的区域
    auto region = canvas(currentRowX, currentRowY, glyphWidth, glyphHeight);

    for (int i = 0; i < glyphHeight; ++i) {
        for (int j = 0; j < glyphWidth; ++j) {
            auto alpha = glyph[i * glyphWidth + j];
            if (alpha >= 160) {
                region(i, j) = {255, 255, 255, 255};
            }
            else if (alpha < 160 && alpha > 60) {
                // Calculate the distance from the edge of the glyph
                float distance = std::min(alpha - 60, 160 - alpha) / 60.0f;
                // Calculate the smoothstep value for the distance
                float smoothstepValue = smoothstep(0.75f, 1.0f, distance);
                // Interpolate between black and white based on the smoothstep
                // value
                region(i, j) =
                    Pixel::lerp(Pixel{0, 0, 0, alpha},
                                Pixel{255, 255, 255, alpha}, smoothstepValue);
            }
            else {
                region(i, j) = {255, 255, 255, alpha};
            }
        }
    }

    GlyphInfo glyphInfo = {currentRowX, currentRowY, glyphWidth,
                           glyphHeight, offsetX,     offsetY};
    info[codepoint] = glyphInfo;

    // 更新当前行的位置
    currentRowX += glyphWidth;
    // 更新当前行的高度
    currentRowHeight = std::max(currentRowHeight, glyphHeight);

    return glyphInfo;
}

Region Atlas::getGlyph(int codepoint, GlyphInfo* glyphInfo)
{
    if (info.count(codepoint)) {
        *glyphInfo = info[codepoint];
        return canvas(glyphInfo->x, glyphInfo->y, glyphInfo->width,
                      glyphInfo->height);
    }
    return Region{};
}

void Atlas::saveToPNG(const std::string& filename)
{
    stbi_write_png(filename.c_str(), canvas.width(), canvas.height(), 4,
                   canvas.data(), canvas.strideBytes());
}

TextRenderer::TextRenderer(int atlasWidth, int atlasHeight)
    : font{}, canvas(), atlas(atlasWidth, atlasHeight)
{
}

bool TextRenderer::loadFont(const char* fontFile)
{
    std::ifstream file(fontFile, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    fontBuffer.resize(size);
    if (!file.read((char*)fontBuffer.data(), size)) {
        return false;
    }

    if (!stbtt_InitFont(&font, fontBuffer.data(), 0)) {
        return false;
    }

    return true;
}

void TextRenderer::createBitmap(int width, int height)
{
    canvas.resize(width, height);
}

void TextRenderer::renderText(const std::wstring& text, int x, int y,
                              int fontSize)
{
    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    ascent *= scale;
    descent *= scale;
    wchar_t prevChar = 0;
    for (auto c : text) {
        int advance, leftSideBearing, kern;
        stbtt_GetCodepointHMetrics(&font, c, &advance, &leftSideBearing);
        advance *= scale;
        leftSideBearing *= scale;

        GlyphInfo glyphInfo;
        Region glyph = atlas.getGlyph(c, &glyphInfo);

        if (glyph.isEmpty()) {
            // auto glyph_ptr = stbtt_GetCodepointBitmap(
            //     &font, 0, scale, c, &glyphInfo.width, &glyphInfo.height,
            //     &glyphInfo.offsetX, &glyphInfo.offsetY);
            // glyphInfo =
            //     atlas.addGlyph(c, glyph_ptr, glyphInfo.width,
            //     glyphInfo.height,
            //                    glyphInfo.offsetX, glyphInfo.offsetY);
            // // 释放由 stbtt_GetCodepointBitmap 分配的内存
            // stbtt_FreeBitmap((unsigned char*)glyph_ptr, 0);

            auto glyph_ptr = stbtt_GetCodepointSDF(
                &font, scale, c, 5, 255, 255.0 / 5.0, &glyphInfo.width,
                &glyphInfo.height, &glyphInfo.offsetX, &glyphInfo.offsetY);
            glyphInfo =
                atlas.addGlyph(c, glyph_ptr, glyphInfo.width, glyphInfo.height,
                               glyphInfo.offsetX, glyphInfo.offsetY);
            printf("GlyphInfo A: %dx%d @ %d,%d\n", glyphInfo.width,
                   glyphInfo.height, glyphInfo.offsetX, glyphInfo.offsetY);
            // 释放由 stbtt_FreeSDF 分配的内存
            stbtt_FreeSDF(glyph_ptr, 0);
        }

        glyph = atlas.getGlyph(c, &glyphInfo);
        printf("GlyphInfo B: %dx%d @ %d,%d\n", glyphInfo.width,
               glyphInfo.height, glyphInfo.offsetX, glyphInfo.offsetY);
        auto dstRegion = canvas(x + leftSideBearing + glyphInfo.offsetX,
                                y + ascent + glyphInfo.offsetY, glyphInfo.width,
                                glyphInfo.height);
        glyph.blendTo(dstRegion);

        // 更新 x 坐标，移动到下一个字符的位置
        x += advance + leftSideBearing;

        if (c != text.back()) {
            x += scale * stbtt_GetCodepointKernAdvance(&font, c,
                                                       text[text.find(c) + 1]);
        }

        prevChar = c;
    }
}

void TextRenderer::saveToPNG(const std::string& filename)
{
    stbi_write_png(filename.c_str(), canvas.width(), canvas.height(), 4,
                   canvas.data(), canvas.strideBytes());
}

void TextRenderer::saveAtlasToPNG(const std::string& filename)
{
    atlas.saveToPNG(filename);
}
