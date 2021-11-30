#pragma once

#ifndef INPUT_CONTROL_H
#define INPUT_CONTROL_H
#include <iostream>

enum class InputState {
	NORMAL,
	EMPTY,
	INVALID
};

template <typename T>
class InputControler {
public:
	InputControler(std::istream& is) : is(is), state(InputState::NORMAL) {}
	InputState GetSate() {
		return state;
	}
	void Normalize() {
		state = InputState::NORMAL;
	}
	InputControler& operator>>(T& value) {
		if (state == InputState::NORMAL) {
			is.clear();
			if (is.peek() == '\n')
				state = InputState::EMPTY;
			else if (!(is >> value) || is.peek() != '\n' && is.peek() != EOF)
				state = InputState::INVALID;
			is.clear();
			is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		return *this;
	}

private:
	std::istream& is;
	InputState state;
};

#endif // !INPUT_CONTROL_H