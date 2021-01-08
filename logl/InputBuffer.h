#pragma once
#include "../loglcore/Math.h"
#include <stdint.h>

class InputBuffer {
public:
	enum KeyCode {
		A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6,
		H = 7, I = 8, J = 9, K = 10, L = 11, M = 12, N = 13,
		O = 14, P = 15, Q = 16, R = 17, S = 18, T = 19, U = 20,
		V = 21, W = 22, X = 23, Y = 24, Z = 25,
		MOUSE_LEFT = 26, MOUSE_RIGHT = 27, MOUSE_MIDDLE = 28,
		ESCAPE = 29,SHIFT = 30,CTRL = 31, KEY_CODE_SIZE = 32
	};

	virtual bool initialize(){
		mousePosx = 0, mousePosy = 0;
		memset(buffer, 0, sizeof(buffer));
		return true;
	}

	virtual void update();

	bool KeyDown(KeyCode code);
	bool KeyHold(KeyCode code);
	bool KeyUp(KeyCode code);

	Game::Vector2 MousePosition();
	void SetMousePosition(int x, int y);
	void LockCursor();
	void HideCursor();
	void ShowCursor();

	//only application could write to input buffer
	void BufferWriteKeyDown(KeyCode code);
	//only application could write to input buffer
	void BufferWriteKeyUp(KeyCode code);
	//only application could write to input buffer
	void BufferWriteMousePosition(uint32_t x, uint32_t y);

	
private:

	uint8_t buffer[KEY_CODE_SIZE];
	uint32_t mousePosx, mousePosy;
	bool cursorLocked = false;
};

inline InputBuffer gInput;