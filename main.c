#include <stdio.h>
#include <string.h>

#include "data.h"
#include "graphic.h"
#include "tigr.h"

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

        int gw = 0;
        int gh = 0;

        MeasureGraphic(data, name, &gw, &gh);

        if (!box->vert) {
            ww += gw;

            if (gh > hh) {
                hh = gh;
            }
        } else {
            hh += gh;

            if (gw > ww) {
                ww = gw;
            }
        }
    }

    *w = ww;
    *h = hh;
}

static void DrawBox(Tigr* screen, const Data* data, float pageTime,
                    const BoxLayout* box) {
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

        int gw = 0;
        int gh = 0;

        MeasureGraphic(data, name, &gw, &gh);

        int ox = 0;
        int oy = 0;

        // TODO Allow aligning to the start instead of just center aligning
        if (!box->vert) {
            oy = h / 2 - gh / 2;
        } else {
            ox = w / 2 - gw / 2;
        }

        DrawGraphic(screen, data, pageTime, name, x + xx + ox, y + yy + oy);

        if (!box->vert) {
            xx += gw;
        } else {
            yy += gh;
        }
    }
}

static void DrawEntity(Tigr* screen, const Data* data, float pageTime,
                       const Entity* ent) {
    DrawGraphic(screen, data, pageTime, ent->resName, ent->x, ent->y);
}

static void DrawPage(Tigr* screen, const Data* data, const Page* page,
                     float pageTime) {
    for (int i = 0; i < page->entCount; ++i) {
        DrawEntity(screen, data, pageTime, &page->ents[i]);
    }

    for (int i = 0; i < page->boxCount; ++i) {
        DrawBox(screen, data, pageTime, &page->boxes[i]);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s path/to/journal/file.txt\n", argv[0]);
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
