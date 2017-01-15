
#pragma once
#include "faction_platform.h"
#include "faction_types.h"

enum TextureID {
	textureID_Cobble,
	textureID_Wood,
	textureID_Pumpkin,
	textureID_PumpkinBody,
	textureID_Melon,
	textureID_Character,
	textureID_Count,
};

#define PLAYER_WIDTH 1.5f
#define PLAYER_HEIGHT 1.5f
#define PLAYER_SPEED_MS 5.0f

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

// struct Tile {
// 	TextureID textureID;
// 	Vec2 size;
// 	bool isSolid;
// };

// typedef int EntityFlags;
// enum EnitityFlagsEnum {
// 	enitityFlags_None = 0,
// 	enitityFlags_HasCollision = 1 << 0,
// 	enitityFlags_IsCreature = 1 << 1,
// 	enitityFlags_IsLandscape = 1 << 2,
// };

// struct Entity {
// 	TextureID textureID;
// 	Rect bounds;

// 	EntityFlags flags;
// 	Rect collisionR;
// 	float animPos;
// };

// struct TextureInfo {
// 	int widthPx;
// 	int heightPx;
// 	bool isAnimation;
// 	bool isVertical;
// 	int numFrames;
// };

struct AnimatedSprite {
	int textureID;
	int numFrames; // assumed to be vertical
	float animPos;
};

struct Projectile {
	bool isActive;
	Vec2 start;
	Vec2 end;
	float pos;
	float gradient;
	float yIntercept;
};

struct Game {
	GLuint textureIDs[textureID_Count];
	// TextureInfo textures[textureID_Count];

	Projectile projectiles[50];

	float tileSizePx;
	float tileSizeMetres;

	// Entity player;
	AnimatedSprite playerSprite;

	bool debug_TileOutlines;

	Vec2 playerPos;
	Vec2 cameraPos; 
};

Game *FactionInit(float windowWidth, float windowHeight);
void FactionShutdown(Game *game);
void GameRenderAndUpdate(Game *game, Input *input);
