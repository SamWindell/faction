
#include "gl_lite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "faction_platform.h"
#include "faction.h"

static int GRID_COUNT_X = 40;
static int GRID_COUNT_Y = 30;

void FactionMain() {
	GLuint texture;
	glGenTextures(1, &texture);
	int width, height, comp = 0;
	unsigned char* img = 0;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	img = stbi_load("../resources/terrain.png", &width, &height, &comp, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(img);

	glOrtho(0.0, GRID_COUNT_X, GRID_COUNT_Y, 0.0, -1, 1);
}

static void DrawTexturedRectangle(float x, float y, float width, float height) {
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   
	glBegin(GL_QUADS);
	
	// lower left
	glTexCoord2f(0, 0);
	glVertex3f(x, y, 0.0);    
	// lower right
	glTexCoord2f(1, 0);
	glVertex3f(x + width, y, 0.0);    
	// upper right
	glTexCoord2f(1, 1);
	glVertex3f(x + width, y + height, 0.0);    
	// upper left
	glTexCoord2f(0, 1);
	glVertex3f(x, y + height, 0.0);
	
	glEnd();	
}

void GameRenderAndUpdate(float windowWidth, float windowHeight, UserInput *userInput) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLint)windowWidth, (GLint)windowHeight);

	DrawTexturedRectangle(1, 1, 2, 2);
	DrawTexturedRectangle(3, 1, 2, 2);
	DrawTexturedRectangle(3, 3, 2, 2);
}
