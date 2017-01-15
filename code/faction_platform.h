
#pragma once
#include "faction_types.h"

enum KeyCodes {
	keyCode_Tab,
	keyCode_LeftArrow,
	keyCode_RightArrow,
	keyCode_UpArrow,
	keyCode_DownArrow,
	keyCode_Enter,
	keyCode_Escape,
	keyCode_A,
	keyCode_D,
	keyCode_S,
	keyCode_W,

	keyCode_Count
};

struct Input {
	Vec2 cursorPos;
	float windowWidth;
	float windowHeight;
	float deltaT;

	int scroll; // zero if no scroll, else positive or negative offset to show scroll amount
	bool mouseState[3]; // left, right, middle

	int keyMap[keyCode_Count]; // the indexes of the key states in the keyDown array
	bool keysDown[512];
};

void PlatformDBG(const char *msg);


