#pragma once
#include <vector>

struct rgba {
	int r, g, b, a;
};

class pngTexture {
public:

	void load(const char* path);

	int width;
	int height;

	std::vector<rgba> pixels;
};