
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
};

void FactionMain();
void GameRenderAndUpdate(float windowWidth, float windowSize, UserInput *userInput);
