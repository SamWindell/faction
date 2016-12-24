
#include "gl_lite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "faction_platform.h"

struct Image {
	int width = 0;
	int height = 0;
	int bytesPerPx = 0;
	unsigned char *data = NULL;
};

struct Faction {
	Image terrainImg;
};
Faction faction; // global for now

void FactionMain() {
	faction.terrainImg.data = stbi_load("../resources/terrain.png", &faction.terrainImg.width, 
	                                    &faction.terrainImg.height, &faction.terrainImg.bytesPerPx, 0);
	if (faction.terrainImg.data == NULL) {
		PlatformDMG("ERROR loading terrain image\n");
		// stbi_image_free(faction.terrainImg.data);		
	}
}

void RenderFrame() {
    glViewport(0, 0, 800, 600);
    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faction.terrainImg.width, faction.terrainImg.height, 
	             0, GL_BGR, GL_UNSIGNED_BYTE, faction.terrainImg.data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   
}


