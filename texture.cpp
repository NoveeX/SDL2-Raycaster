#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb/stb_image.h"
#include <logger.h>

void pngTexture::load(const char* path)
{
	int channels;

	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	if (!data) {
		logger.error("Could not load image by path!\n%s", stbi_failure_reason());
		return;
	}

	for (int i = 0; i < width * height; i++) {
		rgba pd;
		pd.r = data[i * channels + 0]; // Red
		pd.g = data[i * channels + 1]; // Green
		pd.b = data[i * channels + 2]; // Blue
		pixels.push_back(pd);
	}

	stbi_image_free(data);
}
