
#include "gl_lite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "faction_platform.h"
#include "faction.h"
#include "imgui/imgui.h"

#include <math.h>

//
// ImGui
//

static GLuint g_FontTexture = 0;

static void ImGuiRenderDrawLists(ImDrawData *draw_data) {
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

static void ImGuiStartFrame(Game *game, Input *input) {
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

	io.DeltaTime = game->deltaT;
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(img);
}

Game *FactionInit(int windowPixelWidth, int windowPixelHeight) {
	Game *gameState = new Game;
	memset(gameState, 0, sizeof(Game));

	ImGuiInit();

	glGenTextures(textureID_Count, gameState->textures);
	CreateGlTextureFromImage("../resources/cobble.png", gameState->textures[textureID_Cobble]);
	CreateGlTextureFromImage("../resources/wood.png", gameState->textures[textureID_Wood]);
	CreateGlTextureFromImage("../resources/pumpkin.png", gameState->textures[textureID_Pumpkin]);

	glOrtho(0.0, windowPixelWidth, windowPixelHeight, 0.0, -1, 1);

	return gameState;
}

void FactionShutdown(Game *game) {
	ImGuiShutdown();
}

static void DrawTexturedRectangle(Game *gameState, int textureID, float x, float y, float width, float height) {
	glBindTexture(GL_TEXTURE_2D, gameState->textures[textureID]);

	glEnable(GL_TEXTURE_2D);
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

static bool IsKeyDown(Input *input, int keyID) {
	return input->keysDown[input->keyMap[keyID]];
}

void GameRenderAndUpdate(Game *game, Input *input) {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLint)input->windowWidth, (GLint)input->windowHeight);


	if (IsKeyDown(input, keyCode_W)) {
		game->playerPos.y -= 200 * game->deltaT;
	}
	if (IsKeyDown(input, keyCode_S)) {
		game->playerPos.y += 200 * game->deltaT;
	}

	if (IsKeyDown(input, keyCode_A)) {
		game->playerPos.x -= 200 * game->deltaT;
	}
	if (IsKeyDown(input, keyCode_D)) {
		game->playerPos.x += 200 * game->deltaT;
	}

	#define WORLD_WIDTH 30
	#define WORLD_HEIGHT 14
	static int world[WORLD_HEIGHT][WORLD_WIDTH] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1},
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
	float cameraWidth = cameraTileWidth * tileSize;
	float cameraHeight = cameraTileHeight * tileSize;

	game->cameraPos.x = Max(game->playerPos.x - (cameraWidth / 2), 0);
	game->cameraPos.y = Max(game->playerPos.y - (cameraHeight / 2), 0);

	// clamp the camera so it always fits in the world
	MaxVec2(game->cameraPos, Vec2(0, 0));
	// Vec2 cameraBottomRightPos = game->cameraPos + Vec2(cameraWidth, cameraHeight);
	Vec2 worldSize = Vec2(WORLD_WIDTH, WORLD_HEIGHT) * tileSize;
	if (game->cameraPos.x + cameraWidth > worldSize.x) {
		game->cameraPos.x = worldSize.x - cameraWidth;
	}
	if (game->cameraPos.y + cameraHeight > worldSize.y) {
		game->cameraPos.y = worldSize.y - cameraHeight;
	}

	int tileNumX = (int)(game->cameraPos.x / tileSize);
	int tileNumY = (int)(game->cameraPos.y / tileSize);
	float offsetX = fmodf(game->cameraPos.x, tileSize);
	float offsetY = fmodf(game->cameraPos.y, tileSize);

	glEnable(GL_SCISSOR_TEST);
	int scissorHeight = (int)(cameraTileHeight * tileSize);
	glScissor(0, (int)(input->windowHeight - scissorHeight), (GLsizei)(cameraTileWidth * tileSize), (GLsizei)(scissorHeight));
	for (int y = 0; y < cameraTileHeight + 2; ++y) {
		for (int x = 0; x < cameraTileWidth + 2; ++x) {
			int tileX = x + tileNumX;
			int tileY = y + tileNumY;
			if (tileX >= 0 && tileX < WORLD_WIDTH &&
			    tileY >= 0 && tileY < WORLD_HEIGHT) {
				float xPos = x * tileSize - offsetX;
				float yPos = y * tileSize - offsetY;
				DrawTexturedRectangle(game, world[tileY][tileX], xPos, yPos, tileSize, tileSize);							
			}
		}
	}
	glScissor(0, 0, (GLsizei)input->windowWidth, (GLsizei)input->windowHeight);

	Vec2 worldPos = game->cameraPos * -1;
	DrawTexturedRectangle(game, textureID_Pumpkin, worldPos.x + game->playerPos.x, worldPos.y + game->playerPos.y, tileSize, tileSize);

	ImGuiStartFrame(game, input);
	ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
	ImGui::Text("CamOffsetX: %.2f", offsetX);
	ImGui::Text("CamOffsetY: %.2f", offsetY);
	ImGui::Text("CamTileX: %d", tileNumX);
	ImGui::Text("CamTileY: %d", tileNumX);
	ImGui::Text("PlayerPosX: %.2f", game->playerPos.x);
	ImGui::Text("PlayerPosY: %.2f", game->playerPos.y);
	ImGuiEndFrame();
}
