#include "InputBuffer.h"
#include <Windows.h>
#include <WinUser.h>
#include "logl.h"

enum KeyState {
	NONE = 0, DOWN = 1, HOLD = 2, UP = 3
};

void InputBuffer::BufferWriteKeyDown(InputBuffer::KeyCode code) {
	if (buffer[code] != HOLD)buffer[code] = DOWN;
}

void InputBuffer::BufferWriteKeyUp(InputBuffer::KeyCode code) {
	buffer[code] = UP;
}

void InputBuffer::BufferWriteMousePosition(uint32_t x, uint32_t y) {
	mousePosx = x, mousePosy = y;
}

void InputBuffer::update() {
	for (int i = 0; i != KEY_CODE_SIZE; i++) {
		switch (buffer[i]) {
		case DOWN:
			buffer[i] = HOLD;
			break;
		case UP:
			buffer[i] = NONE;
			break;
		}
	}
	if (cursorLocked) {
		int cwidth = GetWinWidth() / 2;
		int cheight = GetWinHeight() / 2;

		SetPhysicalCursorPos(cwidth, cheight);
	}
}

bool InputBuffer::KeyDown(InputBuffer::KeyCode code) { return buffer[code] == DOWN; }
bool InputBuffer::KeyUp(InputBuffer::KeyCode code) { return buffer[code] == UP; }
bool InputBuffer::KeyHold(InputBuffer::KeyCode code) {
	return buffer[code] == HOLD;
}

Game::Vector2 InputBuffer::MousePosition() {
	return Game::Vector2(mousePosx, mousePosy);
}

void InputBuffer::SetMousePosition(int x,int y) {
	SetPhysicalCursorPos(x, y);
}

void InputBuffer::HideCursor() {
	::ShowCursor(false);
}

void InputBuffer::ShowCursor() {
	::ShowCursor(true);
}

void InputBuffer::LockCursor() {
	cursorLocked = true;
}