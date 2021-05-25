#pragma once

#include <stdbool.h>

#define MAX_NAME_LEN 64

#define MAX_IMAGES 128

#define MAX_TEXTS 1024
#define MAX_TEXT_LEN 256

#define MAX_BOXES 32
#define MAX_BOX_CHILDREN 8

#define MAX_ENTS 32

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

typedef struct Page {
    char name[MAX_NAME_LEN];

    int entCount;
    Entity ents[MAX_ENTS];

    int boxCount;
    BoxLayout boxes[MAX_BOXES];
} Page;

typedef struct Data {
    int imageCount;
    Image images[MAX_IMAGES];

    int textCount;
    Text texts[MAX_TEXTS];

    int pageCount;
    Page pages[MAX_PAGES];
} Data;

void InitData(Data* data);
void LoadData(Data* data, const char* filename);

// TODO Load from an encrypted file given password

const Image* FindImage(const Data* data, const char* name);
const Text* FindText(const Data* data, const char* name);

void DestroyData(Data* data);

