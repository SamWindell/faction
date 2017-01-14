
#pragma once
#include "faction_platform.h"
#include "faction_types.h"

enum TextureIDs {
	textureID_Cobble,
	textureID_Wood,
	textureID_Count,
};

struct Game {
	GLuint textures[textureID_Count];
	float deltaT;

	Vec2 playerPos;
	Vec2 cameraPos; 
};

Game *FactionMain(int windowWidth, int windowHeight);
void GameRenderAndUpdate(Game *game, float windowWidth, float windowSize, UserInput *userInput);
