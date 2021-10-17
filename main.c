#include <stdio.h>
#include <string.h>

#ifdef __linux__

#include <SDL2/SDL.h>
#define CUTE_SOUND_FORCE_SDL

#endif

#define CUTE_SOUND_IMPLEMENTATION

#include "cute_sound.h"
#include "data.h"
#include "graphic.h"
#include "main.h"
#include "rect.h"
#include "tigr.h"
#include "util.h"

int FindPageIndex(const Data* data, const char* name) {
    for (int i = 0; i < data->pageCount; ++i) {
        if (strcmp(data->pages[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

const Entity* FindEntity(const Page* page, const char* name) {
    for (int i = 0; i < page->entCount; ++i) {
        if (strcmp(page->ents[i].name, name) == 0) {
            return &page->ents[i];
        }
    }

    return NULL;
}

const BoxLayout* FindBox(const Page* page, const char* name) {
    for (int i = 0; i < page->boxCount; ++i) {
        if (strcmp(page->boxes[i].name, name) == 0) {
            return &page->boxes[i];
        }
    }

    return NULL;
}

void MeasureBox(const BoxLayout* box, const Data* data, const Page* page, int* w, int* h) {
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

        const BoxLayout* childBox = FindBox(page, name);

        int gw = 0;
        int gh = 0;

        if (childBox) {
            MeasureBox(childBox, data, page, &gw, &gh);
        } else {
            MeasureGraphic(data, name, &gw, &gh);
        }

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

static void DrawBox(Tigr* screen, const Data* data, const Page* page, float pageTime,
                    const BoxLayout* box, int parentX, int parentY) {
    int w = 0;
    int h = 0;

    MeasureBox(box, data, page, &w, &h);

    int x = 0;
    int y = 0;

    if (parentX == 0 && parentY == 0) {
        x = HandleSpecialPos(box->x, screen->w, w);
        y = HandleSpecialPos(box->y, screen->h, h);
    } else {
        x = parentX;
        y = parentY;
    }

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

        const BoxLayout* childBox = FindBox(page, name);

        int gw = 0;
        int gh = 0;

        if (childBox) {
            MeasureBox(childBox, data, page, &gw, &gh);
        } else {
            MeasureGraphic(data, name, &gw, &gh);
        }

        int ox = 0;
        int oy = 0;

        // TODO Allow aligning to the start instead of just center aligning
        if (!box->vert) {
            oy = h / 2 - gh / 2;
        } else {
            ox = w / 2 - gw / 2;
        }

        if (childBox) {
            DrawBox(screen, data, page, pageTime, childBox, x + xx + ox, y + yy + oy);
        } else {
            DrawGraphic(screen, data, pageTime, name, x + xx + ox, y + yy + oy, 0, 0);
        }

        if (!box->vert) {
            xx += gw;
        } else {
            yy += gh;
        }
    }
}

static void DrawEntity(Tigr* screen, const Data* data, float pageTime, const Entity* ent, int mx,
                       int my) {
    DrawGraphic(screen, data, pageTime, ent->resName, ent->x, ent->y, mx, my);
}

void GetEntityBounds(Tigr* screen, const Data* data, const Page* page, const Entity* ent,
                     IntRect* rect, int mx, int my) {
    MeasureGraphic(data, ent->resName, &rect->w, &rect->h);

    int mmx = 0;
    int mmy = 0;

    if (page->hasMover && strcmp(ent->name, page->mover.objectName) == 0) {
        mmx = mx;
        mmy = my;
    }

    rect->x = HandleSpecialPos(ent->x, screen->w, rect->w) + mmx;
    rect->y = HandleSpecialPos(ent->y, screen->h, rect->h) + mmy;
}

void GetBoxBounds(Tigr* screen, const Data* data, const Page* page, const BoxLayout* box,
                  IntRect* rect, int mx, int my) {
    // TODO Make this work with nested boxes
    MeasureBox(box, data, page, &rect->w, &rect->h);

    // TODO Throw an error if the box position is hidden (most likely a
    // nested box)

    rect->x = HandleSpecialPos(box->x, screen->w, rect->w);
    rect->y = HandleSpecialPos(box->y, screen->h, rect->h);
}

static void GetObjectBounds(Tigr* screen, const Data* data, const Page* page, const char* name,
                            IntRect* rect, int mx, int my) {
    const Entity* ent = FindEntity(page, name);

    if (ent) {
        GetEntityBounds(screen, data, page, ent, rect, mx, my);
        return;
    }

    const BoxLayout* box = FindBox(page, name);

    if (box) {
        GetBoxBounds(screen, data, page, box, rect, mx, my);
        return;
    }
}

static void DrawPage(Tigr* screen, const Data* data, const Page* page, float pageTime, int mx,
                     int my) {
    for (int i = 0; i < page->entCount; ++i) {
        // Apply the mover offset if the name of the mover matches the entity
        // name
        int mmx = 0;
        int mmy = 0;

        if (page->hasMover && strcmp(page->ents[i].name, page->mover.objectName) == 0) {
            mmx = mx;
            mmy = my;
        }

        DrawEntity(screen, data, pageTime, &page->ents[i], mmx, mmy);
    }

    for (int i = 0; i < page->boxCount; ++i) {
        DrawBox(screen, data, page, pageTime, &page->boxes[i], 0, 0);
    }
}

static void HandlePageInput(Tigr* screen, const Data* data, const Page* page, int* newPageIndex,
                            int* mx, int* my, float dt) {
    if (page->hasQuestion) {
        if (tigrKeyDown(screen, page->question.correctAnswerKey)) {
            *newPageIndex = FindPageIndex(data, page->question.correctAnswerPageName);
        } else {
            for (int ch = 'A'; ch <= 'Z'; ++ch) {
                if (tigrKeyDown(screen, ch)) {
                    *newPageIndex = FindPageIndex(data, page->question.incorrectAnswerPageName);
                    break;
                }
            }
        }
    } else if (page->hasMover) {
        const int speed = (int)(page->mover.speed * dt);

        if (tigrKeyHeld(screen, TK_LEFT)) {
            *mx -= speed;
        }

        if (tigrKeyHeld(screen, TK_RIGHT)) {
            *mx += speed;
        }

        if (tigrKeyHeld(screen, TK_UP)) {
            *my -= speed;
        }

        if (tigrKeyHeld(screen, TK_DOWN)) {
            *my += speed;
        }
    } else if (tigrKeyDown(screen, TK_SPACE) || tigrKeyDown(screen, TK_RIGHT)) {
        if (page->hasCustomNextPage) {
            *newPageIndex = FindPageIndex(data, page->customNextPageName);
        } else {
            *newPageIndex += 1;

            if (*newPageIndex >= data->pageCount) {
                *newPageIndex = 0;
            }
        }
    }

    if (!page->hasMover && tigrKeyDown(screen, TK_LEFT)) {
        *newPageIndex -= 1;

        if (*newPageIndex < 0) {
            *newPageIndex = data->pageCount - 1;
        }
    }

    int mouseX = 0;
    int mouseY = 0;
    int mb = 0;

    tigrMouse(screen, &mouseX, &mouseY, &mb);

    for (int i = 0; i < page->clickCount; ++i) {
        const Clickable* click = &page->clicks[i];

        IntRect rect;

        GetObjectBounds(screen, data, page, click->objectName, &rect, *mx, *my);

        if (IntRectContainsPoint(rect, mouseX, mouseY)) {
            if (mb) {
                *newPageIndex = FindPageIndex(data, click->clickPageName);
            }

            tigrRect(screen, rect.x, rect.y, rect.w, rect.h, tigrRGBA(255, 255, 255, 100));
        }
    }

    for (int i = 0; i < page->touchCount; ++i) {
        const Touchable* touch = &page->touches[i];

        IntRect a = {0};
        IntRect b = {0};

        GetObjectBounds(screen, data, page, touch->toucherObjectName, &a, *mx, *my);
        GetObjectBounds(screen, data, page, touch->touchableObjectName, &b, *mx, *my);

        if (!IntRectCollidesIntRect(a, b)) {
            continue;
        }

        *newPageIndex = FindPageIndex(data, touch->touchPageName);
    }
}

static void PlayPageSounds(cs_context_t* ctx, const Data* data, const Page* page) {
    if (cs_get_playing(ctx)) {
        cs_stop_all_sounds(ctx);
    }

    for (int i = 0; i < page->soundPlayerCount; ++i) {
        const Sound* sound = FindSound(data, page->soundPlayers[i].soundName);

        if (!sound) {
            continue;
        }

        cs_play_sound(ctx, sound->sound);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s path/to/journal/file.txt\n", argv[0]);
        return 1;
    }

    static Data data;

    int pageIndex = 0;
    float pageTime = 0;
    int mx = 0;
    int my = 0;

    Tigr* screen = tigrWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dumpling", 0);

#ifdef __linux__
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        tigrError(screen, "Failed to init SDL: %s", SDL_GetError());
    }
#endif

    cs_context_t* ctx = cs_make_context(screen->handle, 44100, 0, MAX_SOUND_PLAYERS, NULL);

    if (!ctx) {
        tigrError(screen, "Failed to init audio: %s", cs_error_reason);
    }

    (void)tigrTime();

    int prevPageIndex = -1;
    long long lastDataWriteTime = 0;

    while (!tigrClosed(screen)) {
        long long dataWriteTime = 0;

        // Hot-reload the data file
        if (GetLastWriteTime(argv[1], &dataWriteTime) && dataWriteTime != lastDataWriteTime) {
            if (lastDataWriteTime != 0) {
                DestroyData(&data);
            }

            InitData(&data);
            LoadData(&data, argv[1]);

            lastDataWriteTime = dataWriteTime;

            if (data.pageCount == 0) {
                tigrError(screen, "No pages in the journal.");
            }

            pageTime = 0;
            mx = 0;
            my = 0;

            if (pageIndex >= data.pageCount) {
                pageIndex = 0;
            }
        }

        const Page* page = &data.pages[pageIndex];

        tigrClear(screen, tigrRGB(0x80, 0x90, 0xa0));

        const float dt = tigrTime();

        pageTime += dt;

        DrawPage(screen, &data, page, pageTime, mx, my);

        int newPageIndex = pageIndex;

        HandlePageInput(screen, &data, page, &newPageIndex, &mx, &my, dt);

        if (newPageIndex < 0) {
            tigrError(screen,
                      "Invalid target page from page %s (check your question, "
                      "npage, or click)",
                      page->name);
        }

        pageIndex = newPageIndex;

        if (pageIndex != prevPageIndex) {
            pageTime = 0;
            mx = 0;
            my = 0;
            PlayPageSounds(ctx, &data, &data.pages[pageIndex]);
        }

        prevPageIndex = pageIndex;

        tigrUpdate(screen);
        cs_mix(ctx);
    }

    cs_shutdown_context(ctx);

#ifdef __linux__
    SDL_Quit();
#endif

    tigrFree(screen);

    DestroyData(&data);

    return 0;
}
