#include "text.h"
#include <cstdio>

int main(int argc, char** argv)
{
    TextRenderer renderer(512, 512);
    // auto fontFile = "../../fonts/VonwaonBitmap-16px.ttf";
    // auto fontFile = "../../fonts/Roboto-Regular.ttf";
    // auto fontFile = "../../fonts/wenquanyibitmapsongmedium-16.bdf";
    auto fontFile = "C:/Windows/Fonts/simfang.ttf";
    if (!renderer.loadFont(fontFile)) {
        std::cout << "Failed to load font." << std::endl;
        return 1;
    }

    auto text = L"Hello,中文,OK";
    printf("text: %04x, %04x\n", text[0], text[1]);

    renderer.createBitmap(512, 128);
    renderer.renderText(text, 8, 8, 16);
    renderer.saveToPNG("output.png");
    renderer.saveAtlasToPNG("atlas.png");
    return 0;
}

// char buffer[24<<20];
// unsigned char screen[20][79];

// int main(int arg, char **argv)
// {
//    stbtt_fontinfo font;
//    int i,j,ascent,baseline,ch=0;
//    float scale, xpos=2; // leave a little padding in case the character
//    extends left char *text = "Heljo World!"; // intentionally misspelled to
//    show 'lj' brokenness

//    fread(buffer, 1, 1000000, fopen("c:/windows/fonts/arialbd.ttf", "rb"));
//    stbtt_InitFont(&font, (const unsigned char*)buffer, 0);

//    scale = stbtt_ScaleForPixelHeight(&font, 16);
//    printf("Scale: %f\n", scale);
//    stbtt_GetFontVMetrics(&font, &ascent,0,0);
//    baseline = (int) (ascent*scale);

//    while (text[ch]) {
//       int advance,lsb,x0,y0,x1,y1;
//       float x_shift = xpos - (float) floor(xpos);
//       stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
//       stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch],
//       scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
//       stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int)
//       xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
//       // note that this stomps the old data, so where character boxes overlap
//       (e.g. 'lj') it's wrong
//       // because this API is really for baking character bitmaps into
//       textures. if you want to render
//       // a sequence of characters, you really need to render each bitmap to a
//       temp buffer, then
//       // "alpha blend" that into the working buffer
//       xpos += (advance * scale);
//       if (text[ch+1])
//          xpos += scale*stbtt_GetCodepointKernAdvance(&font,
//          text[ch],text[ch+1]);
//       ++ch;
//    }

//    for (j=0; j < 20; ++j) {
//       for (i=0; i < 78; ++i)
//          putchar(" .:ioVM@"[screen[j][i]>>5]);
//       putchar('\n');
//    }

//    return 0;
// }
