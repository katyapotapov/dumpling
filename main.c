#include <stdio.h>
#include <string.h>

#include "data.h"
#include "tigr.h"

static const Image* FindImage(const Data* data, const char* name) {
    for (int j = 0; j < data->imageCount; ++j) {
        if (strcmp(data->images[j].name, name) == 0) {
            return &data->images[j];
        }
    }

    return NULL;
}

static const Text* FindText(const Data* data, const char* name) {
    for (int j = 0; j < data->textCount; ++j) {
        if (strcmp(data->texts[j].name, name) == 0) {
            return &data->texts[j];
        }
    }

    return NULL;
}

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

static void MeasureBox(const BoxLayout* box, const Data* data, float pageTime,
                       int* w, int* h) {
    int ww = 0;
    int hh = 0;

    for (int i = 0; i < box->childCount; ++i) {
        if (i > 0) {
            if (!box->vert) {
                ww += box->space;
            } else {
                hh += box->space;
            }
        }

        const char* name = box->children[i];

        const Image* image = FindImage(data, name);

        if (image) {
            if (!box->vert) {
                ww += image->image->w;

                if (image->image->h > hh) {
                    hh = image->image->h;
                }
            } else {
                hh += image->image->h;

                if (image->image->w > ww) {
                    ww = image->image->w;
                }
            }

            continue;
        }

        const Text* text = FindText(data, name);

        if (text) {
            char sub[MAX_TEXT_LEN] = {0};

            GetTimedText(text, sub, pageTime);

            int tw = tigrTextWidth(tfont, text->str);
            int th = tigrTextHeight(tfont, text->str);

            if (!box->vert) {
                ww += tw;

                if (th > hh) {
                    hh = th;
                }
            } else {
                hh += th;

                if (tw > ww) {
                    ww = tw;
                }
            }

            continue;
        }
    }

    *w = ww;
    *h = hh;
}

static int HandleSpecialPos(int pos, int screenSize, int size) {
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

static void DrawBox(Tigr* screen, const Data* data, const Page* page,
                    float pageTime, const BoxLayout* box) {
    // TODO Nested boxes
    int w = 0;
    int h = 0;

    MeasureBox(box, data, pageTime, &w, &h);

    int x = HandleSpecialPos(box->x, screen->w, w);
    int y = HandleSpecialPos(box->y, screen->h, h);

    int xx = 0;
    int yy = 0;

    for (int i = 0; i < box->childCount; ++i) {
        if (i > 0) {
            if (!box->vert) {
                xx += box->space;
            } else {
                yy += box->space;
            }
        }

        const char* name = box->children[i];

        const Image* image = FindImage(data, name);

        if (image) {
            int ox = 0;
            int oy = 0;

            // TODO Allow aligning to the start instead of just center aligning
            if (!box->vert) {
                oy = h / 2 - image->image->h / 2;
            } else {
                ox = w / 2 - image->image->w / 2;
            }

            tigrBlitAlpha(screen, image->image, x + xx + ox, y + yy + oy, 0, 0,
                          image->image->w, image->image->h, 1);

            if (!box->vert) {
                xx += image->image->w;
            } else {
                yy += image->image->h;
            }

            continue;
        }

        const Text* text = FindText(data, name);

        if (text) {
            char sub[MAX_TEXT_LEN] = {0};

            GetTimedText(text, sub, pageTime);

            int ox = 0;
            int oy = 0;

            int ww = tigrTextWidth(tfont, text->str);
            int hh = tigrTextHeight(tfont, text->str);

            if (!box->vert) {
                oy = h / 2 - hh / 2;
            } else {
                ox = w / 2 - ww / 2;
            }

            tigrPrint(screen, tfont, x + xx + ox, y + yy + oy,
                      tigrRGB(255, 255, 255), sub);

            if (!box->vert) {
                xx += ww;
            } else {
                yy += hh;
            }

            continue;
        }
    }
}

static void DrawEntity(Tigr* screen, const Data* data, const Page* page,
                       float pageTime, const Entity* ent) {
    const Image* image = FindImage(data, ent->resName);

    if (image) {
        int x = HandleSpecialPos(ent->x, screen->w, image->image->w);
        int y = HandleSpecialPos(ent->y, screen->h, image->image->h);

        tigrBlitAlpha(screen, image->image, x, y, 0, 0, image->image->w,
                      image->image->h, 1);

        return;
    }

    const Text* text = FindText(data, ent->resName);

    if (text) {
        char substr[MAX_TEXT_LEN] = {0};

        GetTimedText(text, substr, pageTime);

        int tw = tigrTextWidth(tfont, text->str);
        int th = tigrTextHeight(tfont, text->str);

        int x = HandleSpecialPos(ent->x, screen->w, tw);
        int y = HandleSpecialPos(ent->y, screen->h, th);

        tigrPrint(screen, tfont, x, y, tigrRGB(255, 255, 255), substr);

        return;
    }
}

static void DrawPage(Tigr* screen, const Data* data, const Page* page,
                     float pageTime) {
    for (int i = 0; i < page->entCount; ++i) {
        DrawEntity(screen, data, page, pageTime, &page->ents[i]);
    }

    for (int i = 0; i < page->boxCount; ++i) {
        DrawBox(screen, data, page, pageTime, &page->boxes[i]);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s path/to/journal/file.txt", argv[0]);
        return 1;
    }

    static Data data;

    InitData(&data);
    LoadData(&data, argv[1]);

    int pageIndex = 0;
    float pageTime = 0;

    Tigr* screen = tigrWindow(640, 480, "Dumpling", 0);

    (void)tigrTime();

    while (!tigrClosed(screen)) {
        tigrClear(screen, tigrRGB(0x80, 0x90, 0xa0));

        pageTime += tigrTime();

        DrawPage(screen, &data, &data.pages[pageIndex], pageTime);

        if (tigrKeyDown(screen, TK_SPACE) || tigrKeyDown(screen, TK_RIGHT)) {
            pageIndex += 1;
            pageTime = 0;

            if (pageIndex >= data.pageCount) {
                pageIndex = 0;
            }
        }

        if (tigrKeyDown(screen, TK_LEFT)) {
            pageIndex -= 1;
            pageTime = 0;

            if (pageIndex < 0) {
                pageIndex = data.pageCount - 1;
            }
        }

        tigrUpdate(screen);
    }

    tigrFree(screen);

    DestroyData(&data);

    return 0;
}
