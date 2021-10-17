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
    if (pos == ENT_POS_HIDDEN) {
        return -size;
    } else if (pos > ENT_POS_RIGHT_PAD - WINDOW_WIDTH && pos < ENT_POS_RIGHT_PAD + WINDOW_WIDTH) {
        int rightPad = screenSize - size - 8;
        return rightPad + pos - ENT_POS_RIGHT_PAD;    
    } else if (pos > ENT_POS_LEFT_PAD - WINDOW_WIDTH && pos < ENT_POS_LEFT_PAD + WINDOW_WIDTH) {
        int leftPad = 8;
        return leftPad + pos - ENT_POS_LEFT_PAD;    
    } else if (pos > ENT_POS_CENTER - WINDOW_WIDTH && pos < ENT_POS_CENTER + WINDOW_WIDTH) {
        int center = screenSize / 2 - size / 2;
        return center + pos - ENT_POS_CENTER;    
    } else {
        return pos;
    }
}

void MeasureGraphic(const Data* data, const char* name, int* w, int* h) {
    const Image* image = FindImage(data, name);

    if (image) {
        *w = image->image->w;
        *h = image->image->h;

        if (image->frameCount) {
            *w /= image->frameCount;
        }
        return;
    }

    const Text* text = FindText(data, name);

    if (text) {
        *w = tigrTextWidth(tfont, text->str);
        *h = tigrTextHeight(tfont, text->str);
        return;
    }
}

void DrawGraphic(Tigr* screen, const Data* data, float pageTime, const char* name, int x, int y,
                 int mx, int my) {
    const Image* image = FindImage(data, name);

    if (image) {
        int sx = 0;
        int sy = 0;
        int sw = image->image->w;
        int sh = image->image->h;

        if (image->frameCount) {
            sw /= image->frameCount;
            sx = ((int)(pageTime / image->frameTime) % image->frameCount) * sw;
        }

        x = HandleSpecialPos(x, screen->w, sw) + mx;
        y = HandleSpecialPos(y, screen->h, sh) + my;

        tigrBlitAlpha(screen, image->image, x, y, sx, sy, sw, sh, 1);

        return;
    }

    const Text* text = FindText(data, name);

    if (text) {
        char substr[MAX_TEXT_LEN] = {0};

        GetTimedText(text, substr, pageTime);

        int tw = tigrTextWidth(tfont, text->str);
        int th = tigrTextHeight(tfont, text->str);

        x = HandleSpecialPos(x, screen->w, tw) + mx;
        y = HandleSpecialPos(y, screen->h, th) + my;

        tigrPrint(screen, tfont, x, y, tigrRGB(255, 255, 255), substr);

        return;
    }
}
