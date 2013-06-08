#include <string>
#include <cstdlib>
#include <iostream>
#include <pez/pez.h>

extern "C" {
#include <png/lodepng.h>
}

using namespace std;

GLuint loadTexture(string sfilename)
{
    const char* filename = sfilename.c_str();

    unsigned char* buffer;
    unsigned char* image;
    size_t buffersize, imagesize;
    LodePNG_Decoder decoder;
    LodePNG_loadFile(&buffer, &buffersize, filename);
    LodePNG_Decoder_init(&decoder);
    decoder.infoRaw.color.colorType = LCT_RGBA;
    decoder.infoRaw.color.bitDepth = 8;
    LodePNG_Decoder_decode(&decoder, &image, &imagesize, buffer, buffersize);

    pezCheck(!decoder.error,
             "error %u: %s\n", decoder.error, LodePNG_error_text(decoder.error));

    int bpp = LodePNG_InfoColor_getBpp(&decoder.infoPng.color);
    int bitDepth = decoder.infoPng.color.bitDepth;
    int colorChannels = LodePNG_InfoColor_getChannels(&decoder.infoPng.color);

    pezCheck(bpp == 32 && bitDepth == 8 && colorChannels == 4,
             "Bad format %d %d %d", bpp, bitDepth, colorChannels);

    int w = decoder.infoPng.width;
    int h = decoder.infoPng.height;

    cout << filename << ' ' << w << ' ' << h << ' '
         << buffersize << ' ' << imagesize << endl;

    pezPrintString("Loaded %s (%d x %d) bufferSize = %d, imageSize = %d\n",
                   filename, w, h, buffersize, imagesize);

    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);

    free(buffer);
    free(image);
    LodePNG_Decoder_cleanup(&decoder);

    pezCheckGL("Load texture");
    return handle;
}
