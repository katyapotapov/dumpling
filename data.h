#pragma once

#include <stdbool.h>

#include "cute_sound.h"

#define MAX_NAME_LEN 64

#define MAX_IMAGES 128

#define MAX_TEXTS 1024
#define MAX_TEXT_LEN 256

#define MAX_SOUNDS 32

#define MAX_BOXES 32
#define MAX_BOX_CHILDREN 8

#define MAX_ENTS 32

#define MAX_SOUND_PLAYERS 4

#define MAX_PAGES 1024

#define ENT_POS_CENTER -1000
#define ENT_POS_LEFT_PAD -1001
#define ENT_POS_RIGHT_PAD -1002

typedef struct Tigr Tigr;

typedef struct Image {
    char name[MAX_NAME_LEN];

    Tigr* image;
} Image;

typedef struct Text {
    char name[MAX_NAME_LEN];

    float charsPerSec;
    char str[MAX_TEXT_LEN];
} Text;

typedef struct Sound {
    char name[MAX_NAME_LEN];

    // TODO Put cs_loaded_sound_t here instead so we can adjust the params
    // on the cs_play_sound_def_t on a per-player basis
    cs_loaded_sound_t loaded;
    cs_play_sound_def_t sound;
} Sound;

typedef struct Entity {
    char name[MAX_NAME_LEN];

    int x;
    int y;

    char resName[MAX_NAME_LEN];
} Entity;

typedef struct BoxLayout {
    char name[MAX_NAME_LEN];

    bool vert;

    int x;
    int y;

    int space;

    int childCount;
    char children[MAX_BOX_CHILDREN][MAX_NAME_LEN];
} BoxLayout;

typedef struct SoundPlayer {
    char name[MAX_NAME_LEN];

    char soundName[MAX_NAME_LEN];
} SoundPlayer;

typedef struct Page {
    char name[MAX_NAME_LEN];

    int entCount;
    Entity ents[MAX_ENTS];

    int boxCount;
    BoxLayout boxes[MAX_BOXES];

    int soundPlayerCount;
    SoundPlayer soundPlayers[MAX_SOUND_PLAYERS];
} Page;

typedef struct Data {
    int imageCount;
    Image images[MAX_IMAGES];

    int textCount;
    Text texts[MAX_TEXTS];

    int soundCount;
    Sound sounds[MAX_SOUNDS];

    int pageCount;
    Page pages[MAX_PAGES];
} Data;

void InitData(Data* data);
void LoadData(Data* data, const char* filename);

// TODO Load from an encrypted file given password

const Image* FindImage(const Data* data, const char* name);
const Text* FindText(const Data* data, const char* name);
const Sound* FindSound(const Data* data, const char* name);

void DestroyData(Data* data);

