#pragma once
#include "../texture.h"

int isWall(float x, float y);
void drawWalls();
void drawFloorAndCeiling();

inline pngTexture wallTexture = pngTexture();
inline pngTexture floorTexture = pngTexture();
inline pngTexture ceilingTexture = pngTexture();
