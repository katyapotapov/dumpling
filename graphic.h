#pragma once

typedef struct Tigr Tigr;
typedef struct Data Data;

int HandleSpecialPos(int pos, int screenSize, int size);

void MeasureGraphic(const Data* data, const char* name, int* w, int* h);
void DrawGraphic(Tigr* screen, const Data* data, float pageTime,
                 const char* name, int x, int y);
