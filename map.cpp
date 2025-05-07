#include "map.h"

void generateMap()
{
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			mapArray[y][x] = 0;

			if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1)
			{
				mapArray[y][x] = 1;
			}
		}
	}
}
