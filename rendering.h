#pragma once

#include "SDL.h"
#include "texture.h"
namespace rendering
{
	namespace _internal {
		inline SDL_Window* window;
		inline SDL_Renderer* renderer;

		inline const char* title;
		inline static int width = 1400;
		inline static int height = 1000;

		inline bool running;
		inline uint32_t* screenBuffer = new uint32_t[width * height];
		inline SDL_PixelFormat* format;

	}

	inline float deltaTime;

	void initialize(const char* title);
	void postInit();
	void render();
	void cleanup();

	void drawPixel(int x, int y, int r, int g, int b, int a);

	void drawRect(int x, int y, int w, int h, int r, int g, int b, int a);
	void onEvent(SDL_Event event);
}