#include "player.h"
#include <SDL.h>
#include "rendering.h"
#include "raycaster/raycaster.h"

void player::playerInput()
{
    float moveSpeed = 1.5f * rendering::deltaTime;
    float dx = cos(player::angle * M_PI / 180.0f);
    float dy = sin(player::angle * M_PI / 180.0f);

    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    float nextX = player::x;
    float nextY = player::y;

    if (keystate[SDL_SCANCODE_W]) {
        nextX += dx * moveSpeed;
        nextY += dy * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_S]) {
        nextX -= dx * moveSpeed;
        nextY -= dy * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_A]) {
        nextX += dy * moveSpeed;
        nextY -= dx * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_D]) {
        nextX -= dy * moveSpeed;
        nextY += dx * moveSpeed;
    }
    if (!isWall(nextX, player::y)) {
        player::x = nextX;
    }
    if (!isWall(player::x, nextY)) {
        player::y = nextY;
    }
}
