//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "Slate/SmartPointers.h"
#include "Window.h"

#include <GLFW/glfw3.h>
#include <tuple>
#include <unordered_map>

namespace Slate {
	// god bless you chatgpitty
	enum class KeyCode : uint16_t {
		// Number row (top row)
		Grave         = GLFW_KEY_GRAVE_ACCENT,  // `
		Num1          = GLFW_KEY_1,
		Num2          = GLFW_KEY_2,
		Num3          = GLFW_KEY_3,
		Num4          = GLFW_KEY_4,
		Num5          = GLFW_KEY_5,
		Num6          = GLFW_KEY_6,
		Num7          = GLFW_KEY_7,
		Num8          = GLFW_KEY_8,
		Num9          = GLFW_KEY_9,
		Num0          = GLFW_KEY_0,
		Minus         = GLFW_KEY_MINUS,         // -
		Equal         = GLFW_KEY_EQUAL,         // =
		Backspace     = GLFW_KEY_BACKSPACE,

		// Tab row (QWERTY)
		Tab           = GLFW_KEY_TAB,
		Q             = GLFW_KEY_Q,
		W             = GLFW_KEY_W,
		E             = GLFW_KEY_E,
		R             = GLFW_KEY_R,
		T             = GLFW_KEY_T,
		Y             = GLFW_KEY_Y,
		U             = GLFW_KEY_U,
		I             = GLFW_KEY_I,
		O             = GLFW_KEY_O,
		P             = GLFW_KEY_P,
		LeftBracket   = GLFW_KEY_LEFT_BRACKET,  // [
		RightBracket  = GLFW_KEY_RIGHT_BRACKET, // ]
		Backslash     = GLFW_KEY_BACKSLASH,     // \

		// Home row (ASDF)
		CapsLock      = GLFW_KEY_CAPS_LOCK,
		A             = GLFW_KEY_A,
		S             = GLFW_KEY_S,
		D             = GLFW_KEY_D,
		F             = GLFW_KEY_F,
		G             = GLFW_KEY_G,
		H             = GLFW_KEY_H,
		J             = GLFW_KEY_J,
		K             = GLFW_KEY_K,
		L             = GLFW_KEY_L,
		Semicolon     = GLFW_KEY_SEMICOLON,     // ;
		Apostrophe    = GLFW_KEY_APOSTROPHE,    // '
		Enter         = GLFW_KEY_ENTER,

		// Bottom row (ZXCV)
		LeftShift     = GLFW_KEY_LEFT_SHIFT,
		Z             = GLFW_KEY_Z,
		X             = GLFW_KEY_X,
		C             = GLFW_KEY_C,
		V             = GLFW_KEY_V,
		B             = GLFW_KEY_B,
		N             = GLFW_KEY_N,
		M             = GLFW_KEY_M,
		Comma         = GLFW_KEY_COMMA,         // ,
		Period        = GLFW_KEY_PERIOD,        // .
		Slash         = GLFW_KEY_SLASH,         // /
		RightShift    = GLFW_KEY_RIGHT_SHIFT,

		// Bottom-most row
		LeftControl   = GLFW_KEY_LEFT_CONTROL,
		LeftAlt       = GLFW_KEY_LEFT_ALT,
		LeftSuper     = GLFW_KEY_LEFT_SUPER,    // Windows / Command (macOS) key
		Space         = GLFW_KEY_SPACE,
		RightSuper    = GLFW_KEY_RIGHT_SUPER,
		RightAlt      = GLFW_KEY_RIGHT_ALT,
		Menu          = GLFW_KEY_MENU,
		RightControl  = GLFW_KEY_RIGHT_CONTROL,

		// Function keys
		Escape        = GLFW_KEY_ESCAPE,
		F1            = GLFW_KEY_F1,
		F2            = GLFW_KEY_F2,
		F3            = GLFW_KEY_F3,
		F4            = GLFW_KEY_F4,
		F5            = GLFW_KEY_F5,
		F6            = GLFW_KEY_F6,
		F7            = GLFW_KEY_F7,
		F8            = GLFW_KEY_F8,
		F9            = GLFW_KEY_F9,
		F10           = GLFW_KEY_F10,
		F11           = GLFW_KEY_F11,
		F12           = GLFW_KEY_F12,

		// Navigation
		Insert        = GLFW_KEY_INSERT,
		Delete        = GLFW_KEY_DELETE,
		Home          = GLFW_KEY_HOME,
		End           = GLFW_KEY_END,
		PageUp        = GLFW_KEY_PAGE_UP,
		PageDown      = GLFW_KEY_PAGE_DOWN,

		// Arrow keys
		ArrowUp       = GLFW_KEY_UP,
		ArrowDown     = GLFW_KEY_DOWN,
		ArrowLeft     = GLFW_KEY_LEFT,
		ArrowRight    = GLFW_KEY_RIGHT,

		// Numpad
		NumLock       = GLFW_KEY_NUM_LOCK,
		NumpadDivide  = GLFW_KEY_KP_DIVIDE,
		NumpadMultiply= GLFW_KEY_KP_MULTIPLY,
		NumpadSubtract= GLFW_KEY_KP_SUBTRACT,
		NumpadAdd     = GLFW_KEY_KP_ADD,
		NumpadEnter   = GLFW_KEY_KP_ENTER,
		Numpad1       = GLFW_KEY_KP_1,
		Numpad2       = GLFW_KEY_KP_2,
		Numpad3       = GLFW_KEY_KP_3,
		Numpad4       = GLFW_KEY_KP_4,
		Numpad5       = GLFW_KEY_KP_5,
		Numpad6       = GLFW_KEY_KP_6,
		Numpad7       = GLFW_KEY_KP_7,
		Numpad8       = GLFW_KEY_KP_8,
		Numpad9       = GLFW_KEY_KP_9,
		Numpad0       = GLFW_KEY_KP_0,
		NumpadDecimal = GLFW_KEY_KP_DECIMAL
	};
	enum class MouseButtonCode : unsigned char {
		ButtonLeft     = GLFW_MOUSE_BUTTON_LEFT,
		ButtonRight    = GLFW_MOUSE_BUTTON_RIGHT,
		ButtonMiddle   = GLFW_MOUSE_BUTTON_MIDDLE,

		Button4        = GLFW_MOUSE_BUTTON_4,
		Button5        = GLFW_MOUSE_BUTTON_5,
		Button6        = GLFW_MOUSE_BUTTON_6,
		Button7        = GLFW_MOUSE_BUTTON_7,
		Button8        = GLFW_MOUSE_BUTTON_8,

		// Optional: alias for indexing
		Button0 = GLFW_MOUSE_BUTTON_1, // Same as Left
		Button1 = GLFW_MOUSE_BUTTON_2, // Same as Right
		Button2 = GLFW_MOUSE_BUTTON_3  // Same as Middle
	};

	enum class InputState : unsigned short {
		Up,
		Down,
		Held,
	};
	enum class MouseShape {
		CROSSHAIR,
		CENTER
	};
	enum class InputMode {
		CURSOR_DISABLED,
		CURSOR_NORMAL
	};

	class InputHandler final {
	public:
		explicit InputHandler(Window& window) : _windowInjection(window) {}
		~InputHandler() = default;
	public:
		bool IsKeyPressed(KeyCode key); // repeat
		bool IsKeyJustClicked(KeyCode key); // single frame
		bool IsKeyJustReleased(KeyCode key); // single frame

		bool IsMouseButtonPressed(MouseButtonCode button); // repeat
		bool IsMouseButtonJustClicked(MouseButtonCode button); // single frame
		bool IsMouseButtonJustReleased(MouseButtonCode button); // single frame

		std::pair<float, float> GetMousePosition() const;
		std::pair<float, float> GetMouseDeltaPosition() const;

		InputMode GetInputMode() const;
		void SetInputMode(InputMode new_mode);
	private:
		// dependency
		Window& _windowInjection;

		std::unordered_map<InputState, KeyCode> _keyStates;
		std::unordered_map<InputState, MouseButtonCode> _mouseButtonStates;
		KeyCode _lastHeldKey = {};
		std::pair<float, float> _lastFrameMousePos = {0, 0};
	};
}




