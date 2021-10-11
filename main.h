#pragma once

#include "rect.h"

int FindPageIndex(const Data* data, const char* name);

const Entity* FindEntity(const Page* page, const char* name);
const BoxLayout* FindBox(const Page* page, const char* name);

void MeasureBox(const BoxLayout* box, const Data* data, const Page* page, int* w, int* h);

void GetEntityBounds(Tigr* screen, const Data* data, const Page* page, const Entity* ent,
                     IntRect* rect, int mx, int my);

void GetBoxBounds(Tigr* screen, const Data* data, const Page* page, const BoxLayout* box,
                  IntRect* rect, int mx, int my);
