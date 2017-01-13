
#pragma once
#include <string>

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

std::string PlatformGetExeDir();
void PlatformDBG(const char *msg);


