
#include "gl_lite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "faction_platform.h"
#include "faction.h"
#include "imgui/imgui.h"

#include <math.h>

Game *FactionMain(int windowPixelWidth, int windowPixelHeight) {
	Game *gameState = new Game;
	memset(gameState, 0, sizeof(Game));

	glGenTextures(textureID_Count, gameState->textures);

	{
		int width, height, comp = 0;
		unsigned char* img = 0;
		glBindTexture(GL_TEXTURE_2D, gameState->textures[textureID_Cobble]);
		img = stbi_load("../resources/cobble.png", &width, &height, &comp, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(img);		
	}

	{
		int width, height, comp = 0;
		unsigned char* img = 0;
		glBindTexture(GL_TEXTURE_2D, gameState->textures[textureID_Wood]);
		img = stbi_load("../resources/wood.png", &width, &height, &comp, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(img);
	}


	glOrtho(0.0, windowPixelWidth, windowPixelHeight, 0.0, -1, 1);

	return gameState;
}

static void DrawTexturedRectangle(Game *gameState, int textureID, float x, float y, float width, float height) {
	glBindTexture(GL_TEXTURE_2D, gameState->textures[textureID]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   
	glBegin(GL_QUADS);
	
	// upper left
	glTexCoord2f(0, 1);
	glVertex3f(x, y + height, 0.0);
	// upper right
	glTexCoord2f(1, 1);
	glVertex3f(x + width, y + height, 0.0);    
	// lower right
	glTexCoord2f(1, 0);
	glVertex3f(x + width, y, 0.0);    
	// lower left
	glTexCoord2f(0, 0);
	glVertex3f(x, y, 0.0);    
	
	glEnd();	
}

static bool IsKeyDown(UserInput *userInput, int keyID) {
	return userInput->keysDown[userInput->keyMap[keyID]];
}

void GameRenderAndUpdate(Game *game, float windowWidth, float windowHeight, UserInput *userInput) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLint)windowWidth, (GLint)windowHeight);

	if (IsKeyDown(userInput, keyCode_W)) {
		game->cameraPos.y -= 60 * game->deltaT;
	}
	if (IsKeyDown(userInput, keyCode_S)) {
		game->cameraPos.y += 60 * game->deltaT;
	}

	if (IsKeyDown(userInput, keyCode_A)) {
		game->cameraPos.x -= 60 * game->deltaT;
	}
	if (IsKeyDown(userInput, keyCode_D)) {
		game->cameraPos.x += 60 * game->deltaT;
	}

	#define WORLD_WIDTH 30
	#define WORLD_HEIGHT 14
	static int world[WORLD_HEIGHT][WORLD_WIDTH] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
	};

	float tileSize = 100;
	int cameraTileWidth = 6;
	int cameraTileHeight = 6;

	int tileNumX = Max((int)(game->cameraPos.x / tileSize), 0);
	int tileNumY = Max((int)(game->cameraPos.y / tileSize), 0);
	float offsetX = fmodf(game->cameraPos.x, tileSize);
	float offsetY = fmodf(game->cameraPos.y, tileSize);
	for (int y = 0; y < cameraTileHeight; ++y) {
		for (int x = 0; x < cameraTileWidth; ++x) {
			DrawTexturedRectangle(game, world[y + tileNumY][x + tileNumX], x * tileSize + offsetX, 
			                      y * tileSize + offsetY, tileSize, tileSize);			
		}
	}

	// DrawTexturedRectangle(game, textureID_Wood, game->playerPos.x, game->playerPos.y, tileSize, tileSize);
	// DrawTexturedRectangle(game, textureID_Wood, game->playerPos.x, game->playerPos.y + tileSize, tileSize, tileSize);
}
