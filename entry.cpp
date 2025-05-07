#include "Windows.h"
#include "SDL.h"
#include "logger.h"
#include "rendering.h"
#include "string.h"
#include "raycaster/raycaster.h"
#include "player.h"
#include "ext/unistd.h"
#include "SDL.h"
#include "map.h"
void rendering::postInit() {
	SDL_SetRelativeMouseMode(SDL_TRUE);

	char cwd[1024];
	_getcwd(cwd, sizeof(cwd));

	logger.log("Current working directory: %s", cwd);

	std::string assetsImgPath = std::string(cwd) + "\\Assets\\Img\\";

	wallTexture.load((assetsImgPath + "wall.png").c_str());
	floorTexture.load((assetsImgPath + "floor.png").c_str());
	ceilingTexture.load((assetsImgPath + "ceiling.png").c_str());
	entityTexture.load("empty.png");

	rendering::_internal::format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	generateMap();
}

void rendering::render()
{
	raycast();
	drawFloorAndCeiling();
	//drawDebugMiniMap();
	player::playerInput();
}

void rendering::onEvent(SDL_Event event)
{
	if (event.type == SDL_MOUSEMOTION) {
		int dx = event.motion.xrel;
		player::angle += dx * player::mouseSensitivity;

		if (player::angle < 0) player::angle += 360;
		if (player::angle >= 360) player::angle -= 360;
	}
}

void rendering::cleanup()
{
	SDL_FreeFormat(rendering::_internal::format);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	logger.initialize(true);
	rendering::initialize("Raycaster");

	return 0;
}