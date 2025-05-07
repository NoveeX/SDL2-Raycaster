#include "raycaster.h"

#include <cmath>
#include <algorithm>

#include "../player.h"
#include "../map.h"
#include "../rendering.h"
#include <logger.h>


#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


#define FOV 90
#define MAX_DEPTH 8.0f

#define DEBUG_MINIMAP_SIZE_MODIFIER 50.0f

std::vector<_ray> rays = {};

// i stole this XD
int orientation(float p[2], float q[2], float r[2]) {
	float val = (q[1] - p[1]) * (r[0] - q[0]) -
		(q[0] - p[0]) * (r[1] - q[1]);

	if (val == 0.0f) return 0;           
	return (val > 0.0f) ? 1 : 2;           
}

bool onSegment(float p[2], float q[2], float r[2]) {
	return q[0] <= std::max(p[0], r[0]) && q[0] >= std::min(p[0], r[0]) &&
		q[1] <= std::max(p[1], r[1]) && q[1] >= std::min(p[1], r[1]);
}

int isWall(float x, float y) {
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
		return 1;
	return mapArray[(int)y][(int)x];
}

std::optional<_entity> isEntity(_ray ray)
{
	for (_entity e : entities){
		float* p1 = ray.start;
		float* q1 = ray.end;

		_ray ray2 = _ray(
			{ (e.x - e.width / 2 * cos(e.angle * M_PI / 180.0f)), (e.y - e.width / 2 * sin(e.angle * M_PI / 180.0f)) },
			{ (e.x + e.width / 2 * cos(e.angle * M_PI / 180.0f)), (e.y + e.width / 2 * sin(e.angle * M_PI / 180.0f)) }
		);

		float* p2 = ray2.start;
		float* q2 = ray2.end;

		int o1 = orientation(p1, q1, p2);
		int o2 = orientation(p1, q1, q2);
		int o3 = orientation(p2, q2, p1);
		int o4 = orientation(p2, q2, q1);

		if (o1 != o2 && o3 != o4)
			return e;

		if (o1 == 0 && onSegment(p1, p2, q1)) return e;
		if (o2 == 0 && onSegment(p1, q2, q1)) return e;
		if (o3 == 0 && onSegment(p2, p1, q2)) return e;
		if (o4 == 0 && onSegment(p2, q1, q2)) return e;
	}
	return std::nullopt;
}

void wallcast(int i, int rayCount, float rayWidth) {
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

	_ray ray = { {player::x, player::y}, {rayDirX * perpWallDist + player::x, rayDirY * perpWallDist + player::y}, _ray::wallRay };
	rays.push_back(ray);

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
void entitycast(int i, int rayCount, float rayWidth) {
	float rayAngle = player::angle - (FOV / 2.0f) + ((float)i / rayCount) * FOV;

	float rayDirX = cos(rayAngle * M_PI / 180.0f);
	float rayDirY = sin(rayAngle * M_PI / 180.0f);

	std::optional<_entity> hit = std::nullopt;
	float distance = 0;
	bool isWallInFront = false;
	while (hit == std::nullopt && distance < MAX_DEPTH) {
		distance += 0.01f;
		if (isWall(rayDirX * distance + player::x, rayDirY * distance + player::y)) {
			isWallInFront = true;
			break;
		}
		hit = isEntity({ {player::x, player::y}, {rayDirX * distance + player::x, rayDirY * distance + player::y} });
	}
	if (distance > MAX_DEPTH || isWallInFront) return;

	_ray ray = { {player::x, player::y}, {rayDirX * distance + player::x, rayDirY * distance + player::y}, _ray::entityRay };
	rays.push_back(ray);

	if (distance < 0.01f) distance = 0.01f;

	float correctedDistance = distance * cos((rayAngle - player::angle) * M_PI / 180.0f);
	int lineHeight = (int)(rendering::_internal::height / correctedDistance);
	if (lineHeight > rendering::_internal::height * 10)
		lineHeight = rendering::_internal::height * 10;

	int drawStart = rendering::_internal::height / 2 - lineHeight / 2;
	int drawEnd = rendering::_internal::height / 2 + lineHeight / 2;

	float ex = hit->x;
	float ey = hit->y;
	float angleRad = hit->angle * M_PI / 180.0f;
	float dx = cos(angleRad);
	float dy = sin(angleRad);

	float hitX = player::x + rayDirX * distance;
	float hitY = player::y + rayDirY * distance;
	float hx = hitX - ex;
	float hy = hitY - ey;

	float projection = hx * dx + hy * dy;

	float textureOrientation = 0.5f + (projection / hit->width);
	textureOrientation = fmod(textureOrientation, 1.0f);
	if (textureOrientation < 0) textureOrientation += 1.0f;

	textureOrientation -= floor(textureOrientation);

	int textureX = (int)(textureOrientation * entityTexture.width);

	for (int y = 0; y < lineHeight; y++) {
		int textureY = (y * entityTexture.height) / lineHeight;
		int texIndex = textureY * entityTexture.width + textureX;
		if (texIndex < 0 || texIndex >= entityTexture.pixels.size()) continue;

		float shade = 1.0f - fminf(correctedDistance / MAX_DEPTH, 1.0f);
		rgba pixel = entityTexture.pixels[texIndex];
		Uint8 r, g, b;
		r = (Uint8)(pixel.r * shade);
		g = (Uint8)(pixel.g * shade);
		b = (Uint8)(pixel.b * shade);
		for (int w = 0; w < (int)rayWidth; w++) {
			rendering::drawPixel((int)(i * rayWidth) + w, drawStart + y, r, g, b, pixel.a);
		}
	}
}
void raycast()
{

	const int rayCount = rendering::_internal::width / 2;
	const float rayWidth = rendering::_internal::width / rayCount;

	for (int i = 0; i < rayCount; i++)
	{
		wallcast(i, rayCount, rayWidth);
		entitycast(i, rayCount, rayWidth);
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

void drawDebugMiniMap()
{
	rendering::drawRect(0, 0, MAP_WIDTH * DEBUG_MINIMAP_SIZE_MODIFIER, MAP_HEIGHT * DEBUG_MINIMAP_SIZE_MODIFIER, 21, 21, 21, 255);

	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			if (isWall(x, y))
			{
				rendering::drawRect(x * DEBUG_MINIMAP_SIZE_MODIFIER, y * DEBUG_MINIMAP_SIZE_MODIFIER, DEBUG_MINIMAP_SIZE_MODIFIER, DEBUG_MINIMAP_SIZE_MODIFIER, 255, 255, 255, 255);
			}
		}
	}

	rendering::drawRect(player::x * DEBUG_MINIMAP_SIZE_MODIFIER - DEBUG_MINIMAP_SIZE_MODIFIER / 4, player::y * DEBUG_MINIMAP_SIZE_MODIFIER - DEBUG_MINIMAP_SIZE_MODIFIER / 4, DEBUG_MINIMAP_SIZE_MODIFIER / 2, DEBUG_MINIMAP_SIZE_MODIFIER / 2, 0, 0, 244, 255);

	for (const _ray& ray : rays)
	{
		rgba rayColor = {};
		if (ray.type == _ray::wallRay)
		{
			rayColor = { 255, 0, 0, 255 };
		}
		else if (ray.type == _ray::entityRay)
		{
			rayColor = { 0, 0, 255, 255 };
		}
		rendering::drawLine(ray.start[0] * DEBUG_MINIMAP_SIZE_MODIFIER, ray.start[1] * DEBUG_MINIMAP_SIZE_MODIFIER, ray.end[0] * DEBUG_MINIMAP_SIZE_MODIFIER, ray.end[1] * DEBUG_MINIMAP_SIZE_MODIFIER, rayColor.r, rayColor.g, rayColor.b, rayColor.a);
	}

	for (_entity e : entities){
		rendering::drawLine((e.x - e.width / 2 * cos(e.angle * M_PI / 180.0f)) * DEBUG_MINIMAP_SIZE_MODIFIER, (e.y - e.width / 2 * sin(e.angle * M_PI / 180.0f)) * DEBUG_MINIMAP_SIZE_MODIFIER, (e.x + e.width / 2 * cos(e.angle * M_PI / 180.0f)) * DEBUG_MINIMAP_SIZE_MODIFIER, (e.y + e.width / 2 * sin(e.angle * M_PI / 180.0f)) * DEBUG_MINIMAP_SIZE_MODIFIER, 0, 0, 255, 100);
		rendering::drawPixel(e.x * DEBUG_MINIMAP_SIZE_MODIFIER, e.y * DEBUG_MINIMAP_SIZE_MODIFIER, 0, 255, 255, 255);
	}

	rays.clear();
}
