
#pragma once
#include "faction_platform.h"

struct Vec2 {
	Vec2() { x = y = 0; }
	Vec2(float _x, float _y) { x = _x; y = _y; }
	float x;
	float y;
};

struct UserInput {
	bool mouseState[3]; // left, right, middle
	Vec2 cursorPos;
	int scroll; // zero if no scroll, else positive or negative offset to show scroll amount

	int keyMap[keyCode_Count]; // the indexes of the key states in the keyDown array
	bool keysDown[512];
};

struct GameState {
	float deltaT;

	Vec2 playerPos;
};

GameState *FactionMain(int windowWidth, int windowHeight);
void GameRenderAndUpdate(GameState *game, float windowWidth, float windowSize, UserInput *userInput);
