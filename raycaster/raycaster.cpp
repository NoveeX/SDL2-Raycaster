#include "raycaster.h"

#include <cmath>
#include <algorithm>

#include "../player.h"
#include "../map.h"
#include "../rendering.h"
#include <logger.h>

#define FOV 90
#define MAX_DEPTH 8.0f
int isWall(float x, float y) {
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
		return 1;
	return mapArray[(int)y][(int)x];
}

void drawWalls()
{

	const int rayCount = rendering::_internal::width / 4;
	const float rayWidth = rendering::_internal::width / rayCount;

	for (int i = 0; i < rayCount; i++)
	{
		float rayAngle = player::angle - (FOV / 2.0f) + ((float)i / rayCount) * FOV;

		float rayDirX = cos(rayAngle * M_PI / 180.0f);
		float rayDirY = sin(rayAngle * M_PI / 180.0f);

		int mapX = (int)player::x;
		int mapY = (int)player::y;

		float deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
		float deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);

		float sideDistX, sideDistY;
		int stepX, stepY;
		int hit = 0;
		int side;

		if (rayDirX < 0) {
			stepX = -1;
			sideDistX = (player::x - mapX) * deltaDistX;
		}
		else {
			stepX = 1;
			sideDistX = (mapX + 1.0 - player::x) * deltaDistX;
		}
		if (rayDirY < 0) {
			stepY = -1;
			sideDistY = (player::y - mapY) * deltaDistY;
		}
		else {
			stepY = 1;
			sideDistY = (mapY + 1.0 - player::y) * deltaDistY;
		}
		while (!hit) {
			if (sideDistX < sideDistY) {
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			}
			else {
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}
			
			hit = isWall(mapX, mapY);
		}

		float perpWallDist = (side == 0)
			? (sideDistX - deltaDistX)
			: (sideDistY - deltaDistY);

		if (perpWallDist < 0.01f) perpWallDist = 0.01f;

		float correctedDistance = perpWallDist * cos((rayAngle - player::angle) * M_PI / 180.0f);
		int lineHeight = (int)(rendering::_internal::height / correctedDistance);
		if (lineHeight > rendering::_internal::height * 10)
			lineHeight = rendering::_internal::height * 10;

		int drawStart = rendering::_internal::height / 2 - lineHeight / 2;
		int drawEnd = rendering::_internal::height / 2 + lineHeight / 2;

		float wallX;
		if (side == 0)
			wallX = player::y + perpWallDist * rayDirY;
		else
			wallX = player::x + perpWallDist * rayDirX;
		wallX -= floor(wallX);

		int textureX = (int)(wallX * wallTexture.width);
		if ((side == 0 && rayDirX > 0) || (side == 1 && rayDirY < 0))
			textureX = wallTexture.width - textureX - 1;

		if (textureX < 0) textureX = 0;
		if (textureX >= wallTexture.width) textureX = wallTexture.width - 1;
	
		for (int y = 0; y < lineHeight; y++) {
			int textureY = (y * wallTexture.height) / lineHeight;
			int texIndex = textureY * wallTexture.width + textureX;
			if (texIndex < 0 || texIndex >= wallTexture.pixels.size()) continue;

			float shade = 1.0f - fminf(correctedDistance / MAX_DEPTH, 1.0f);
			rgba pixel = wallTexture.pixels[texIndex];
			Uint8 r, g, b;
			r = (Uint8)(pixel.r * shade);
			g = (Uint8)(pixel.g * shade);
			b = (Uint8)(pixel.b * shade);
			for (int w = 0; w < (int)rayWidth; w++) {
				rendering::drawPixel((int)(i * rayWidth) + w, drawStart + y, r, g, b, pixel.a);
			}
		}
	}
}

void drawFloorAndCeiling()
{
	int screenW = rendering::_internal::width;
	int screenH = rendering::_internal::height;

	float angleRad = player::angle * M_PI / 180.0f;


	float dirX = cos(angleRad);
	float dirY = sin(angleRad);

	float fovRad = FOV * M_PI / 180.0f;
	float fovScale = tan(fovRad / 2.0f);

	float planeX = -dirY * fovScale;
	float planeY = dirX * fovScale;
	for (int y = screenH / 2 + 1; y < screenH; ++y)
	{
		
		float rowDistance = (0.5f * screenH) / (y - screenH / 2);

		float rayDirX0 = dirX - planeX;
		float rayDirY0 = dirY - planeY;
		float rayDirX1 = dirX + planeX;
		float rayDirY1 = dirY + planeY;

		float stepX = rowDistance * (rayDirX1 - rayDirX0) / screenW;
		float stepY = rowDistance * (rayDirY1 - rayDirY0) / screenW;

		float floorX = player::x + rowDistance * rayDirX0;
		float floorY = player::y + rowDistance * rayDirY0;

		for (int x = 0; x < screenW; ++x)
		{
			
			if (rendering::_internal::screenBuffer[y * rendering::_internal::width + x] == 255)
			{
				int cellX = (int)floorX;
				int cellY = (int)floorY;

				int texX = int((floorX - cellX) * floorTexture.width) & (floorTexture.width - 1);
				int texY = int((floorY - cellY) * floorTexture.height) & (floorTexture.height - 1);

				int texIndex = texY * floorTexture.width + texX;
				rgba floorPixel = floorTexture.pixels[texIndex];
				float floorShade = 1.0f - fminf(rowDistance / MAX_DEPTH, 1.0f);
				Uint8 fr = (Uint8)(floorPixel.r * floorShade);
				Uint8 fg = (Uint8)(floorPixel.g * floorShade);
				Uint8 fb = (Uint8)(floorPixel.b * floorShade);
				rendering::drawPixel(x, y, fr, fg, fb, floorPixel.a);

				texIndex = texY * ceilingTexture.width + texX;
				rgba ceilPixel = ceilingTexture.pixels[texIndex];
				float ceilShade = 1.0f - fminf(rowDistance / MAX_DEPTH, 1.0f);
				Uint8 cr = (Uint8)(ceilPixel.r * ceilShade);
				Uint8 cg = (Uint8)(ceilPixel.g * ceilShade);
				Uint8 cb = (Uint8)(ceilPixel.b * ceilShade);
				rendering::drawPixel(x, screenH - y, cr, cg, cb, ceilPixel.a);
				
			}
			floorX += stepX;
			floorY += stepY;
		}
	}
}