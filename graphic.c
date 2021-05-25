#include "graphic.h"

#include <string.h>

#include "data.h"
#include "tigr.h"

static void GetTimedText(const Text* text, char* dest, float pageTime) {
    if (text->charsPerSec == 0) {
        strcpy(dest, text->str);
        return;
    }

    int maxLen = (int)strlen(text->str);

    int len = (int)(pageTime * text->charsPerSec);

    if (len > maxLen) {
        len = maxLen;
    }

    memcpy(dest, text->str, len);
}

int HandleSpecialPos(int pos, int screenSize, int size) {
    switch (pos) {
        case ENT_POS_CENTER:
            return screenSize / 2 - size / 2;

        case ENT_POS_LEFT_PAD:
            return 8;

        case ENT_POS_RIGHT_PAD:
            return screenSize - size - 8;

        default:
            return pos;
    }
}

void MeasureGraphic(const Data* data, const char* name, int* w, int* h) {
    const Image* image = FindImage(data, name);

    if (image) {
        *w = image->image->w;
        *h = image->image->h;
        return;
    }

    const Text* text = FindText(data, name);

    if (text) {
        *w = tigrTextWidth(tfont, text->str);
        *h = tigrTextHeight(tfont, text->str);
        return;
    }
}

void DrawGraphic(Tigr* screen, const Data* data, float pageTime,
                 const char* name, int x, int y) {
    const Image* image = FindImage(data, name);

    if (image) {
        x = HandleSpecialPos(x, screen->w, image->image->w);
        y = HandleSpecialPos(y, screen->h, image->image->h);

        tigrBlitAlpha(screen, image->image, x, y, 0, 0, image->image->w,
                      image->image->h, 1);

        return;
    }

    const Text* text = FindText(data, name);

    if (text) {
        char substr[MAX_TEXT_LEN] = {0};

        GetTimedText(text, substr, pageTime);

        int tw = tigrTextWidth(tfont, text->str);
        int th = tigrTextHeight(tfont, text->str);

        x = HandleSpecialPos(x, screen->w, tw);
        y = HandleSpecialPos(y, screen->h, th);

        tigrPrint(screen, tfont, x, y, tigrRGB(255, 255, 255), substr);

        return;
    }
}
