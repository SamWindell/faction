
#include "gl_lite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "faction_platform.h"
#include "faction.h"
#include "imgui/imgui.h"

#include <math.h>
#include <time.h>

#define USE_IMGUI

//
// ImGui
//

static GLuint g_FontTexture = 0;

static void ImGuiRenderDrawLists(ImDrawData *draw_data) {
#ifdef USE_IMGUI
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO &io = ImGui::GetIO();
	int fb_width = (int) (io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int) (io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0) {
		return;
	}
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// We are using the OpenGL fixed pipeline to make the example code simpler to read!
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_viewport[4];
	glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4];
	glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);
	// glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei) fb_width, (GLsizei) fb_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Render command lists
	#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *) 0)->ELEMENT))
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		const ImDrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;
		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void *) ((char *) vtx_buffer + OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void *) ((char *) vtx_buffer + OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void *) ((char *) vtx_buffer + OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback) {
				pcmd->UserCallback(cmd_list, pcmd);
			} else {
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t) pcmd->TextureId);
				glScissor((int) pcmd->ClipRect.x, (int) (fb_height - pcmd->ClipRect.w), (int) (pcmd->ClipRect.z - pcmd->ClipRect.x),
						  (int) (pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei) pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
							   idx_buffer);
			}
			idx_buffer += pcmd->ElemCount;
		}
	}
	#undef OFFSETOF

	// Restore modified state
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, (GLuint) last_texture);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glViewport(last_viewport[0], last_viewport[1], (GLsizei) last_viewport[2], (GLsizei) last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei) last_scissor_box[2], (GLsizei) last_scissor_box[3]);
#endif
}

static bool ImGuiCreateDeviceObjects() {
	// Build texture atlas
	ImGuiIO &io = ImGui::GetIO();
	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); // Load as RGBA 32-bits (75% of the memory is wasted, but default font is
															// so small) because it is more likely to be compatible with user's
															// existing shaders. If your ImTextureId represent a higher-level concept
															// than just a GL texture id, consider calling GetTexDataAsAlpha8() instead
															// to save on GPU memory.

	// Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &g_FontTexture);
	glBindTexture(GL_TEXTURE_2D, g_FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *) (intptr_t) g_FontTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return true;
}

static void ImGuiInvalidateDeviceObjects() {
	if (g_FontTexture) {
		glDeleteTextures(1, &g_FontTexture);
		ImGui::GetIO().Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

static void ImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.RenderDrawListsFn = ImGuiRenderDrawLists;
}

static void ImGuiStartFrame(Input *input) {
	if (!g_FontTexture) {
		ImGuiCreateDeviceObjects();
	}

	ImGuiIO &io = ImGui::GetIO();

	io.DisplaySize = ImVec2(input->windowWidth, input->windowHeight);
	io.DisplayFramebufferScale = ImVec2(1, 1);
	io.MousePos = ImVec2(input->cursorPos.x, input->cursorPos.y);

	io.MouseDown[0] = (input->mouseState[0]);
	io.MouseDown[1] = (input->mouseState[1]);
	io.MouseDown[2] = (input->mouseState[2]);

	io.KeyCtrl = false;
	io.KeyShift = false;
	io.KeyAlt = false;
	io.KeySuper = false;

	io.DeltaTime = input->deltaT;
	io.MouseWheel = 0;

	ImGui::NewFrame();
}

static void ImGuiEndFrame() {
	ImGui::Render();
}

static void ImGuiShutdown() {
	ImGuiInvalidateDeviceObjects();
	ImGui::Shutdown();
}

//
// Faction
//

static void CreateGlTextureFromImage(const char *fileName, GLuint glTextureID) {
	int width, height, comp = 0;
	unsigned char* img = 0;
	glBindTexture(GL_TEXTURE_2D, glTextureID);
	img = stbi_load(fileName, &width, &height, &comp, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	stbi_image_free(img);
}

Game *FactionInit(float windowPixelWidth, float windowPixelHeight) {
	Game *game = new Game;
	memset(game, 0, sizeof(Game));

	ImGuiInit();

	glGenTextures(textureID_Count, game->textureIDs);
	CreateGlTextureFromImage("../resources/cobble.png", game->textureIDs[textureID_Cobble]);
	CreateGlTextureFromImage("../resources/wood.png", game->textureIDs[textureID_Wood]);
	CreateGlTextureFromImage("../resources/pumpkin.png", game->textureIDs[textureID_Pumpkin]);
	CreateGlTextureFromImage("../resources/pumpkin_body.png", game->textureIDs[textureID_PumpkinBody]);
	CreateGlTextureFromImage("../resources/melon.png", game->textureIDs[textureID_Melon]);
	CreateGlTextureFromImage("../resources/character.png", game->textureIDs[textureID_Character]);

	game->playerSprite.numFrames = 2;
	game->playerSprite.textureID = textureID_Character;

	game->playerPos = {3, 3};
	game->tileSizePx = 80;
	game->tileSizeMetres = 1.2f;

	// game->player.bounds.pos = {3, 3};
	// game->player.flags = enitityFlags_IsCreature | enitityFlags_HasCollision;
	// game->player.textureID = textureID_Pumpkin;

	return game;
}


void FactionShutdown(Game *game) {
	ImGuiShutdown();
}

static void DrawTexturedRectangle(Game *game, int textureID, float x, float y, float width, float height) {
	glBindTexture(GL_TEXTURE_2D, game->textureIDs[textureID]);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	// glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   
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
	glDisable(GL_TEXTURE_2D);
}

static void DrawTexturedRectangleAnim(Game *game, int textureID, float x, float y, float width, float height, float textureYStart, float textureYEnd) {
	glBindTexture(GL_TEXTURE_2D, game->textureIDs[textureID]);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	// glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   
	glBegin(GL_QUADS);
	
	// upper left
	glTexCoord2f(0, textureYEnd);
	glVertex3f(x, y + height, 0.0);
	// upper right
	glTexCoord2f(1, textureYEnd);
	glVertex3f(x + width, y + height, 0.0);    
	// lower right
	glTexCoord2f(1, textureYStart);
	glVertex3f(x + width, y, 0.0);    
	// lower left
	glTexCoord2f(0, textureYStart);
	glVertex3f(x, y, 0.0);    
	
	glEnd();	
	glDisable(GL_TEXTURE_2D);
}

void DrawAnimatedSprite(Game *game, AnimatedSprite *sprite, float x, float y, float w, float h) {
	float textureFrameSize = 1.0f / sprite->numFrames;
	float textureStart = textureFrameSize * (int)sprite->animPos;
	float textureEnd = textureStart + textureFrameSize;	

	DrawTexturedRectangleAnim(game, sprite->textureID, x, y, w, h, textureStart, textureEnd);
}

static void DrawBlobMan(Game *game, int headTextureID, int bodyTextureID, float x, float y, float sectionW, float sectionH) {
	DrawTexturedRectangle(game, bodyTextureID, x, y, sectionW, sectionH);
	DrawTexturedRectangle(game, textureID_Pumpkin, x, y - sectionH, sectionW, sectionH);
}

static bool IsKeyDown(Input *input, int keyID) {
	return input->keysDown[input->keyMap[keyID]];
}

static float MetresToPixels(Game *game, float value) {
	return value * (game->tileSizePx / game->tileSizeMetres);
}

static float PixelsToMetres(Game *game, float value) {
	return (value / game->tileSizePx) * game->tileSizeMetres;
}

static bool IsPointEmpty(Game *game, Vec2 pos) {
	int tileX = (int)(pos.x / game->tileSizeMetres);
	int tileY = (int)(pos.y / game->tileSizeMetres);
	return world[tileY][tileX] == 0;
}

static void IncrementSpriteAnim(Input *input, AnimatedSprite *sprite, float timeInSecondsForWholeAnim) {
	float timePerFrame = timeInSecondsForWholeAnim / sprite->numFrames;
	float amountToInc = (1.0f / timePerFrame) * input->deltaT;
	sprite->animPos += amountToInc;
	if (sprite->animPos > sprite->numFrames) {
		sprite->animPos = 0;
	}
}

static bool IsHorizLineEmpty(Game *game, float x1, float x2, float y) {
	float testPoint = x1;
	int tileY = (int)(y / game->tileSizeMetres); 
	while (testPoint < x2) {
		int tileX = (int)(x1 / game->tileSizeMetres);
		if (world[tileY][tileX] == 1) {
			return false;
		}
		testPoint += game->tileSizeMetres;
	}
	return true;
}

static bool IsVertLineEmpty(Game *game, float y1, float y2, float x) {
float testPoint = y1;
	int tileX = (int)(x / game->tileSizeMetres); 
	while (testPoint < y2) {
		int tileY = (int)(y1 / game->tileSizeMetres);
		if (world[tileY][tileX] == 1) {
			return false;
		}
		testPoint += game->tileSizeMetres;
	}
	return true;
}

// type = 0 for horiz, 1 for vert
static void CheckWallCollision(int type, Vec2 *outRemainingVec, float *minT, float testWall, Vec2 startingPos, Vec2 deltaPos, float min, float max) {
	int otherType = abs(type - 1);
	if (deltaPos.e[otherType] != 0) {
		float intersectionT = (testWall - startingPos.e[otherType]) / deltaPos.e[otherType];
		Vec2 intersectionPoint = startingPos + intersectionT * deltaPos;
		if (intersectionT < *minT && intersectionT >= 0) {
			if (intersectionPoint.e[type] < max && intersectionPoint.e[type] >= min) {
				*minT = intersectionT;

				// work out the remaining velocity parallel to the wall we have just hit
				float remainingT = 1 - intersectionT;
				Vec2 testVec = {};
				testVec.e[type] = 1; 
				Vec2 result = {};
				float dot = DotProduct(testVec, deltaPos);
				if (dot > 0) { // down or right
					result = remainingT * testVec;
				} else if (dot == 0) { // perpendicular, do nothing
					result = {0, 0};
				} else { // up or left
					result = remainingT * testVec * -1;
				}
				*outRemainingVec = result;
			}
		}
	}
}

void GameRenderAndUpdate(Game *game, Input *input) {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGuiStartFrame(input);
	auto imguiDrawList = ImGui::GetWindowDrawList();
	imguiDrawList->PushClipRectFullScreen();

	glViewport(0, 0, (GLint)input->windowWidth, (GLint)input->windowHeight);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, input->windowWidth, input->windowHeight, 0.0, -1, 1);	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	bool playerMoved = false;
	// > Handle Input
	{ 
		Vec2 deltaPlayerPos = {};
		bool playerMovedX = false;
		bool playerMovedY = false;
		if (IsKeyDown(input, keyCode_W)) {
			deltaPlayerPos.y -= 1;
			playerMovedY = true;
		}
		if (IsKeyDown(input, keyCode_S)) {
			deltaPlayerPos.y += 1;
			playerMovedY = true;
		}

		if (IsKeyDown(input, keyCode_D)) {
			deltaPlayerPos.x += 1;
			playerMovedX = true;
		}
		if (IsKeyDown(input, keyCode_A)) {
			deltaPlayerPos.x -= 1;
			playerMovedX = true;
		}

		playerMoved = playerMovedX || playerMovedY;
		if (playerMovedX && playerMovedY) {
			deltaPlayerPos = Normalise(deltaPlayerPos);
		}
		
		if (playerMoved) {
			Vec2 oldPlayerPos = game->playerPos;
			Vec2 newPlayerPos = game->playerPos + deltaPlayerPos * (PLAYER_SPEED_MS * input->deltaT);
			Vec2 scaledDeltaPlayerPos = deltaPlayerPos * (PLAYER_SPEED_MS * input->deltaT);

			int oldPlayerPosTileX = (int)(oldPlayerPos.x / game->tileSizeMetres);
			int oldPlayerPosTileY = (int)(oldPlayerPos.y / game->tileSizeMetres);
			int newPlayerPosTileX = (int)(newPlayerPos.x / game->tileSizeMetres);
			int newPlayerPosTileY = (int)(newPlayerPos.y / game->tileSizeMetres);

			// loop through all the tile between the players old position and the players new position
			// usually only going to be like 1 tile, unless the player is going really fast
			int startingTileX = Min(oldPlayerPosTileX, newPlayerPosTileX);
			int startingTileY = Min(oldPlayerPosTileY, newPlayerPosTileY);
			int endingTileX = Max(oldPlayerPosTileX, newPlayerPosTileX) + 1;
			int endingTileY = Max(oldPlayerPosTileY, newPlayerPosTileY) + 1;

			float minT = 1;
			Vec2 remainingVelocity = {};
			for (int yTile = startingTileY; yTile < endingTileY; ++yTile) {
				for (int xTile = startingTileX; xTile < endingTileX; ++xTile) {
					if (world[yTile][xTile] == 1) { // there is something in the tile we want to move into

						Vec2 collisionMin = {xTile * game->tileSizeMetres, yTile * game->tileSizeMetres};
						Vec2 collisionMax = {collisionMin.x + game->tileSizeMetres, collisionMin.y + game->tileSizeMetres};

						CheckWallCollision(1, &remainingVelocity, &minT, collisionMin.x, oldPlayerPos, deltaPlayerPos, 
						                   collisionMin.y, collisionMax.y);
						CheckWallCollision(1, &remainingVelocity, &minT, collisionMax.x, oldPlayerPos, deltaPlayerPos, 
						                   collisionMin.y, collisionMax.y);

						CheckWallCollision(0, &remainingVelocity, &minT, collisionMin.y, oldPlayerPos, deltaPlayerPos, 
						                   collisionMin.x, collisionMax.x);
						CheckWallCollision(0, &remainingVelocity, &minT, collisionMax.y, oldPlayerPos, deltaPlayerPos, 
						                   collisionMin.x, collisionMax.x);

					}
				}
			}

			if (minT != 1) {
				float remainingT = 1 - minT;
				newPlayerPos = oldPlayerPos + minT * (scaledDeltaPlayerPos);
				newPlayerPos += remainingVelocity * (PLAYER_SPEED_MS * input->deltaT);
			}
			game->playerPos = newPlayerPos;

			// if (world[newPlayerPosTileX][newPlayerPosTileY] == 1) { // there is something in the tile we want to move into

			// 	// collision box of the tile (just the size of a tile at the moment)
			// 	Vec2 topLeft = {newPlayerPosTileX * game->tileSizeMetres, newPlayerPosTileY * game->tileSizeMetres};
			// 	Vec2 topRight = {topLeft.x + game->tileSizeMetres, topLeft.y};
			// 	Vec2 bottomRight = {topRight.x, topLeft.y + game->tileSizeMetres};
			// 	Vec2 bottomLeft = {topLeft.x, bottomRight.y};

			// 	Vec2 newPlayerCollisionBoxMin = {newPlayerPos.x - PLAYER_WIDTH / 2, newPlayerPos.y - PLAYER_HEIGHT / 2};
			// 	Vec2 newPlayerCollisionBoxMax = {newPlayerPos.x + PLAYER_WIDTH / 2, newPlayerPos.y};

			// }

			// game->playerPos += deltaPlayerPos * (PLAYER_SPEED_MS * input->deltaT);
		}

		// if (yChange != 0 || xChange != 0) {
		// 	Vec2 newPlayerPos = {game->playerPos.x + xChange, game->playerPos.y + yChange};

		// 	Vec2 newPlayerCollisionBoxMin = {newPlayerPos.x - PLAYER_WIDTH / 2, newPlayerPos.y - PLAYER_HEIGHT / 2};
		// 	Vec2 newPlayerCollisionBoxMax = {newPlayerPos.x + PLAYER_WIDTH / 2, newPlayerPos.y};

		// 	if (IsHorizLineEmpty(game, newPlayerCollisionBoxMin.x, newPlayerCollisionBoxMax.x, newPlayerCollisionBoxMin.y) &&
		// 	    IsHorizLineEmpty(game, newPlayerCollisionBoxMin.x, newPlayerCollisionBoxMax.x, newPlayerCollisionBoxMax.y)) {
		// 		game->playerPos.x = newPlayerPos.x;
		// 		playerMoved = true;
		// 	}
		// 	if (IsVertLineEmpty(game, newPlayerCollisionBoxMin.y, newPlayerCollisionBoxMax.y, newPlayerCollisionBoxMin.x) &&
		// 	    IsVertLineEmpty(game, newPlayerCollisionBoxMin.y, newPlayerCollisionBoxMax.y, newPlayerCollisionBoxMax.x)) {
		// 		game->playerPos.y = newPlayerPos.y;
		// 		playerMoved = true;			
		// 	}
		// }

		if (input->scroll) {
			game->tileSizePx += input->scroll;
		}

		// if (newPos != game->playerPos) {
		// 	playerMoved = true;
		// 	Vec2 newPlayerLeft = newPos;
		// 	Vec2 newPlayerRight = newPos;
		// 	newPlayerLeft.x -= PLAYER_WIDTH / 2;
		// 	newPlayerRight.x += PLAYER_WIDTH / 2;

		// 	Vec2 newPlayerTopLeft = newPlayerLeft;
		// 	Vec2 newPlayerTopRight = newPlayerRight;
		// 	newPlayerTopLeft.y -= PLAYER_HEIGHT / 2;
		// 	newPlayerTopRight.y -= PLAYER_HEIGHT / 2;

		// 	if (IsPointEmpty(game, newPlayerRight) && IsPointEmpty(game, newPlayerLeft) &&
		// 	    IsPointEmpty(game, newPlayerTopRight) && IsPointEmpty(game, newPlayerTopLeft)) {
		// 		game->playerPos = newPos;
		// 	}
		// }
	}

	// camera
	float windowTileWidth = input->windowWidth / game->tileSizePx;
	float windowTileHeight = input->windowHeight / game->tileSizePx;
	int cameraVisibleTilesX = (int)ceilf(windowTileWidth);
	int cameraVisibleTilesY = (int)ceilf(windowTileHeight);

	float cameraWidthMetres = windowTileWidth * game->tileSizeMetres;
	float cameraHeightMetres = windowTileHeight * game->tileSizeMetres;

	game->cameraPos.x = Max(game->playerPos.x - (cameraWidthMetres / 2), 0);
	game->cameraPos.y = Max(game->playerPos.y - (cameraHeightMetres / 2), 0);
	MaxVec2(game->cameraPos, Vec2(0, 0));
	Vec2 worldSize = Vec2(WORLD_WIDTH, WORLD_HEIGHT) * game->tileSizeMetres;
	if (game->cameraPos.x + cameraWidthMetres > worldSize.x) {
		game->cameraPos.x = worldSize.x - cameraWidthMetres;
	}
	if (game->cameraPos.y + cameraHeightMetres > worldSize.y) {
		game->cameraPos.y = worldSize.y - cameraHeightMetres;
	}

	float cursorInWorldX = PixelsToMetres(game, input->cursorPos.x) + game->cameraPos.x;
	float cursorInWorldY = PixelsToMetres(game, input->cursorPos.y) + game->cameraPos.y;

	// > Create Projectiles
	if (input->mouseState[0]) {
		Projectile *proj = NULL;
		for (int i = 0; i < ArraySize(game->projectiles); ++i) {
			if (!game->projectiles[i].isActive) {
				proj = &game->projectiles[i];
			}
		}
		if (proj) {
			proj->isActive = true;
			proj->pos.x = game->playerPos.x;
			proj->pos.y = game->playerPos.y - PLAYER_HEIGHT * 0.5f;
			float changeInY = cursorInWorldY - (game->playerPos.y - PLAYER_HEIGHT * 0.5f);
			float changeInX = cursorInWorldX - game->playerPos.x;
			proj->angle = atanf(changeInY / changeInX);
			if (changeInX < 0) {
				proj->angle += M_PI;
			}
			srand((unsigned int)proj);
			auto randOffset = rand() % 100;
			proj->speedMS = 20.0f + (randOffset / 40.0f); // 20 metres per second ish
		}
	}

	// > Draw Tiles
	int tileNumX = (int)(game->cameraPos.x / game->tileSizeMetres);
	int tileNumY = (int)(game->cameraPos.y / game->tileSizeMetres);
	float offsetX = fmodf(game->cameraPos.x, game->tileSizeMetres);
	float offsetY = fmodf(game->cameraPos.y, game->tileSizeMetres);
	for (int y = 0; y < cameraVisibleTilesY + 2; ++y) {
		for (int x = 0; x < cameraVisibleTilesX + 2; ++x) {
			int tileX = x + tileNumX;
			int tileY = y + tileNumY;
			if (tileX >= 0 && tileX < WORLD_WIDTH &&
			    tileY >= 0 && tileY < WORLD_HEIGHT) {
				float xPosPx = MetresToPixels(game, x * game->tileSizeMetres - offsetX);
				float yPosPx = MetresToPixels(game, y * game->tileSizeMetres - offsetY);
				DrawTexturedRectangle(game, world[tileY][tileX], xPosPx, yPosPx, game->tileSizePx, game->tileSizePx);
				if (game->debug_TileOutlines) {
					// imguiDrawList->AddRect(ImVec2(xPosPx, yPosPx), ImVec2(xPosPx + game->tileSizePx, yPosPx + game->tileSizePx),
										   // 0xff0000ff);
				}
			}
		}
	}

	Vec2 worldPos = game->cameraPos * -1;

	// > Draw Projectiles
	for (int i = 0; i < ArraySize(game->projectiles); ++i) {
		if (game->projectiles[i].isActive) {

			float hypotDistance = game->projectiles[i].speedMS * input->deltaT;
			Vec2 offset;
			offset.y = sinf(game->projectiles[i].angle) * hypotDistance;
			offset.x = cosf(game->projectiles[i].angle) * hypotDistance;
			Vec2 newPos = game->projectiles[i].pos + offset;

			float worldWidthMetres = WORLD_WIDTH * game->tileSizeMetres;
			float worldHeightMetres = WORLD_HEIGHT * game->tileSizeMetres;

			if (!IsPointEmpty(game, newPos) ||
			    (newPos.x < 0 || newPos.x > worldWidthMetres ||
				newPos.y < 0 || newPos.y > worldHeightMetres)) {
				game->projectiles[i].isActive = false;				
			} else {
				game->projectiles[i].pos = newPos;
				float projSizePx = MetresToPixels(game, 0.2f);
				float posX = MetresToPixels(game, newPos.x + worldPos.x);
				float posY = MetresToPixels(game, newPos.y + worldPos.y);
				DrawTexturedRectangle(game, textureID_Melon, posX, posY, projSizePx, projSizePx);
			}
		}
	}

	// > Draw Player
	float playerPosPixelsX = MetresToPixels(game, worldPos.x + game->playerPos.x - PLAYER_WIDTH / 2);
	float playerPosPixelsY = MetresToPixels(game, worldPos.y + game->playerPos.y - PLAYER_HEIGHT);

	// auto startPoint = ImVec2(MetresToPixels(game, worldPos.x + game->playerPos.x), 
	//                          MetresToPixels(game, worldPos.y + game->playerPos.y));
	// imguiDrawList->AddRect(startPoint, ImVec2(startPoint.x + 5, startPoint.y + 5), 0xff0000ff);

	if (playerMoved) {
		IncrementSpriteAnim(input, &game->playerSprite, 0.2f);
	}
	DrawAnimatedSprite(game, &game->playerSprite, playerPosPixelsX, playerPosPixelsY, 
	                   MetresToPixels(game, PLAYER_WIDTH), MetresToPixels(game, PLAYER_HEIGHT));

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	imguiDrawList->PopClipRect();
	if (ImGui::Button("Tile Outlines")) game->debug_TileOutlines = !game->debug_TileOutlines;
	ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
	ImGui::Text("Camera: (%.2f, %.2f)", game->cameraPos.x, game->cameraPos.y);
	ImGui::Text("CursorPosM: (%.2f, %.2f)", cursorInWorldX, cursorInWorldY);
	ImGui::Text("PlayerPos: (%.2f, %.2f)", game->playerPos.x, game->playerPos.y);
	ImGui::Text("TilePX: %.2f", game->tileSizePx);
	ImGuiEndFrame();
}

