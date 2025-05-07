#include "rendering.h"
#include "logger.h"
#include "texture.h"

#include <algorithm>
#include <cmath>

#ifdef max
#undef max
#endif

void rendering::initialize(const char* title)
{

	_internal::title = title;

	using namespace _internal;

	int result = SDL_Init(SDL_INIT_EVERYTHING);
	if (result < 0) {
		logger.log("SDL could not be initialized! SDL_Error: %s", SDL_GetError());
		return;
	}

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
	if (window == NULL) {
		logger.log("Window could not be created! SDL_Error: %s", SDL_GetError());
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (window == NULL) {
		logger.log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
		return;
	}

	SDL_Texture* frameTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	
	postInit();

	Uint32 lastTime = SDL_GetTicks();

	running = true;
	while (running)
	{
		Uint32 currentTime = SDL_GetTicks();
		deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;

			onEvent(event);
		}
		drawRect(0, 0, width, height, 0, 0, 0, 255);

		render();

		SDL_UpdateTexture(frameTexture, nullptr, screenBuffer, rendering::_internal::width * sizeof(uint32_t));
		SDL_RenderCopy(renderer, frameTexture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	}

	rendering::cleanup();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

}

void rendering::drawPixel(int x, int y, int r, int g, int b, int a)
{
	if (x >= 0 && x < rendering::_internal::width && y >= 0 && y < rendering::_internal::height)
	{
		uint32_t color = SDL_MapRGBA(rendering::_internal::format, r, g, b, a);
		_internal::screenBuffer[y * rendering::_internal::width + x] = color;

	}
}

void rendering::drawRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			drawPixel(x + i, y + j, r, g, b, a);
		}
	}
}


void rendering::drawLine(float x1, float y1, float x2, float y2, int r, int g, int b, int a) {
	float dx = x2 - x1;
	float dy = y2 - y1;

	float steps = std::max(std::abs(dx), std::abs(dy));

	if (steps == 0) {
		rendering::drawPixel(static_cast<int>(x1), static_cast<int>(y1), r, g, b, a);
		return;
	}

	float xInc = dx / steps;
	float yInc = dy / steps;

	float x = x1;
	float y = y1;

	for (int i = 0; i <= steps; ++i) {
		rendering::drawPixel(static_cast<int>(x), static_cast<int>(y), r, g, b, a);
		x += xInc;
		y += yInc;
	}
}