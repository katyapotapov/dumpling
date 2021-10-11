#pragma once

#include <stdbool.h>

typedef struct IntRect {
    int x;
    int y;
    int w;
    int h;
} IntRect;

inline static bool IntRectContainsPoint(IntRect rect, int x, int y) {
    return x >= rect.x && y >= rect.y && x <= rect.x + rect.w && y <= rect.y + rect.h;
}

inline static bool IntRectCollidesIntRect(IntRect a, IntRect b) {
    return a.x + a.w >= b.x && a.y + a.h >= b.y && b.x + b.w >= a.x && b.y + b.h >= a.y;
}
