#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cute_sound.h"
#include "tigr.h"

static Image ParseImage(FILE* f) {
    Image image = {0};

    char filename[256];
    int scale = 1;

    fscanf(f, "%s %d %s", image.name, &scale, filename);

    image.image = tigrLoadImage(filename);

    if (!image.image) {
        tigrError(NULL, "Failed to load image %s", filename);
    }

    if (scale < 1) {
        scale = 1;
    }

    if (scale > 1) {
        int sw = image.image->w * scale;
        int sh = image.image->h * scale;

        Tigr* newImg = tigrBitmap(sw, sh);

        // Nearest neighbor scaling
        for (int y = 0; y < sh; ++y) {
            for (int x = 0; x < sw; ++x) {
                newImg->pix[y * sw + x] =
                    image.image
                        ->pix[(y / scale) * image.image->w + (x / scale)];
            }
        }

        tigrFree(image.image);

        image.image = newImg;
    }

    return image;
}

static Text ParseText(FILE* f, bool multi) {
    Text text = {0};

    fscanf(f, "%s %f ", text.name, &text.charsPerSec);

    if (!multi) {
        fgets(text.str, MAX_TEXT_LEN, f);
    } else {
        char* s = text.str;
        char buf[MAX_TEXT_LEN] = {0};

        while (!feof(f)) {
            fgets(buf, MAX_TEXT_LEN, f);

            if (strcmp(buf, "endtext\n") == 0) {
                break;
            }

            int len = (int)strlen(buf);

            if ((s - text.str) + len >= MAX_TEXT_LEN) {
                break;
            }

            memcpy(s, buf, len);
            s += len;
        }
    }

    // Remove newline from end
    char* newline = strrchr(text.str, '\n');

    if (newline) {
        *newline = '\0';
    }

    return text;
}

static void ParseSound(Sound* sound, FILE* f) {
    char filename[256];

    fscanf(f, "%s %s", sound->name, filename);

    sound->loaded = cs_load_wav(filename);

    if (!sound->loaded.channels[0]) {
        tigrError(NULL, "Failed to load sound: %s", cs_error_reason);
    }

    sound->sound = cs_make_def(&sound->loaded);
}

static Page ParsePage(FILE* f) {
    Page page = {0};

    fscanf(f, "%s", page.name);

    return page;
}

static int ConvertPos(const char* s) {
    if (s[0] == 'c') {
        return ENT_POS_CENTER;
    }

    if (s[0] == 'l') {
        return ENT_POS_LEFT_PAD;
    }

    if (s[0] == 'r') {
        return ENT_POS_RIGHT_PAD;
    }

    if (s[0] == 'h') {
        return ENT_POS_HIDDEN;
    }

    return atoi(s);
}

static Entity ParseEntity(FILE* f) {
    Entity ent = {0};

    char px[32];
    char py[32];

    fscanf(f, "%s %s %s %s", ent.name, px, py, ent.resName);

    ent.x = ConvertPos(px);
    ent.y = ConvertPos(py);

    return ent;
}

static BoxLayout ParseBox(FILE* f, bool vert) {
    BoxLayout box = {0};

    box.vert = vert;

    char px[32];
    char py[32];

    fscanf(f, "%s %s %s %d %d", box.name, px, py, &box.space, &box.childCount);

    box.x = ConvertPos(px);
    box.y = ConvertPos(py);

    for (int i = 0; i < box.childCount; ++i) {
        fscanf(f, "%s", box.children[i]);
    }

    return box;
}

static SoundPlayer ParseSoundPlayer(FILE* f) {
    SoundPlayer sp = {0};

    fscanf(f, "%s %s", sp.name, sp.soundName);

    return sp;
}

static Question ParseQuestion(FILE* f) {
    Question q = {0};

    char ch;

    fscanf(f, "%s %c %s %s", q.name, &ch, q.correctAnswerPageName,
           q.incorrectAnswerPageName);

    if (ch < 'A' || ch > 'Z') {
        tigrError(NULL,
                  "Question %s correct answer key must be a letter (A to Z)",
                  q.name);
    }

    q.correctAnswerKey = ch;

    return q;
}

static Clickable ParseClickable(FILE* f) {
    Clickable c = {0};

    fscanf(f, "%s %s %s", c.name, c.objectName, c.clickPageName);

    return c;
}

void InitData(Data* data) { memset(data, 0, sizeof(*data)); }

void LoadData(Data* data, const char* filename) {
    FILE* f = fopen(filename, "r");

    Page* curPage = NULL;

    while (!feof(f)) {
        char cmd[64];

        fscanf(f, "%s", cmd);

        if (strcmp(cmd, "image") == 0) {
            data->images[data->imageCount++] = ParseImage(f);
        } else if (strcmp(cmd, "text") == 0) {
            data->texts[data->textCount++] = ParseText(f, false);
        } else if (strcmp(cmd, "mtext") == 0) {
            data->texts[data->textCount++] = ParseText(f, true);
        } else if (strcmp(cmd, "sound") == 0) {
            ParseSound(&data->sounds[data->soundCount++], f);
        } else if (strcmp(cmd, "page") == 0) {
            curPage = &data->pages[data->pageCount];
            data->pages[data->pageCount++] = ParsePage(f);
        } else if (strcmp(cmd, "ent") == 0) {
            curPage->ents[curPage->entCount++] = ParseEntity(f);
        } else if (strcmp(cmd, "hbox") == 0) {
            curPage->boxes[curPage->boxCount++] = ParseBox(f, false);
        } else if (strcmp(cmd, "vbox") == 0) {
            curPage->boxes[curPage->boxCount++] = ParseBox(f, true);
        } else if (strcmp(cmd, "splayer") == 0) {
            curPage->soundPlayers[curPage->soundPlayerCount++] =
                ParseSoundPlayer(f);
        } else if (strcmp(cmd, "question") == 0) {
            if (curPage->hasQuestion) {
                tigrError(NULL,
                          "Attempted to add multiple questions to page %s",
                          curPage->name);
            }

            if (curPage->hasCustomNextPage) {
                tigrError(NULL,
                          "Cannot add question to page %s because it has a "
                          "custom next page",
                          curPage->name);
            }

            curPage->hasQuestion = true;
            curPage->question = ParseQuestion(f);
        } else if (strcmp(cmd, "npage") == 0) {
            if (curPage->hasQuestion) {
                tigrError(
                    NULL,
                    "Cannot set next page on page %s because it has a question",
                    curPage->name);
            }

            if (curPage->clickCount > 0) {
                tigrError(NULL,
                          "Cannot set next page on page %s because it has a "
                          "clickable",
                          curPage->name);
            }

            curPage->hasCustomNextPage = true;
            fscanf(f, "%s", curPage->customNextPageName);
        } else if (strcmp(cmd, "click") == 0) {
            curPage->clicks[curPage->clickCount++] = ParseClickable(f);
        }
    }

    fclose(f);
}

const Image* FindImage(const Data* data, const char* name) {
    for (int j = 0; j < data->imageCount; ++j) {
        if (strcmp(data->images[j].name, name) == 0) {
            return &data->images[j];
        }
    }

    return NULL;
}

const Text* FindText(const Data* data, const char* name) {
    for (int j = 0; j < data->textCount; ++j) {
        if (strcmp(data->texts[j].name, name) == 0) {
            return &data->texts[j];
        }
    }

    return NULL;
}

const Sound* FindSound(const Data* data, const char* name) {
    for (int j = 0; j < data->soundCount; ++j) {
        if (strcmp(data->sounds[j].name, name) == 0) {
            return &data->sounds[j];
        }
    }

    return NULL;
}

void DestroyData(Data* data) {
    for (int i = 0; i < data->soundCount; ++i) {
        cs_free_sound(&data->sounds[i].loaded);
    }

    for (int i = 0; i < data->imageCount; ++i) {
        tigrFree(data->images[i].image);
    }
}
