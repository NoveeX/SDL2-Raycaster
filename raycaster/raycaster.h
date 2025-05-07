#pragma once
#include "../texture.h"
#include "../entity.h"
#include <optional>

struct _ray {
	float start[2];
	float end[2];

	enum {
		wallRay,
		entityRay
	} type;
};

int isWall(float x, float y);
std::optional<_entity> isEntity(_ray ray);
void raycast();
void drawFloorAndCeiling();
void drawDebugMiniMap();

inline pngTexture wallTexture = pngTexture();
inline pngTexture entityTexture = pngTexture();
inline pngTexture floorTexture = pngTexture();
inline pngTexture ceilingTexture = pngTexture();
