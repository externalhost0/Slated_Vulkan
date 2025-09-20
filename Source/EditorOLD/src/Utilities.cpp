//
// Created by Hayden Rivas on 2/7/25.
//
#if defined(SLATE_OS_MACOS)
#include <ApplicationServices/ApplicationServices.h>
#elif defined(SLATE_OS_WINDOWS)
#include <windows.h>
#endif

#include "Utilities.h"

namespace Slate::Utilities {

#if defined(SLATE_OS_MACOS)
	std::array<int, 4> ColorPickScreenFuncMac() {
		CGEventRef event = CGEventCreate(nullptr);
		CGPoint cursor = CGEventGetLocation(event);
		CFRelease(event);

		// capture 1x1 extent
		CGImageRef image = CGDisplayCreateImageForRect(CGMainDisplayID(), CGRectMake(cursor.x, cursor.y, 1, 1));
		assert(image);

		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		unsigned char pixelData[4];

		CGContextRef context = CGBitmapContextCreate(pixelData, 1, 1, 8, 4, colorSpace, kCGImageAlphaPremultipliedLast);

		// draw onto
		CGContextDrawImage(context, CGRectMake(0, 0, 1, 1), image);

		// our return meshData
		int red = pixelData[0];
		int green = pixelData[1];
		int blue = pixelData[2];
		int alpha = pixelData[3];

		// clean
		CGContextRelease(context);
		CGColorSpaceRelease(colorSpace);
		CGImageRelease(image);

		return {red, green, blue, alpha};
	}
#endif
#if defined(SLATE_OS_WINDOWS)
	std::array<int, 4> ColorPickScreenFuncWindows() {
		// Get the cursor position
		POINT cursorPos;
		if (!GetCursorPos(&cursorPos)) {
			std::cerr << "Failed to get cursor position!" << std::endl;
			return;
		}

		// Get the device context of the screen
		HDC hdcScreen = GetDC(NULL);
		if (!hdcScreen) {
			std::cerr << "Failed to get screen DC!" << std::endl;
			return;
		}

		// Get the color at the cursor position
		COLORREF color = GetPixel(hdcScreen, cursorPos.x, cursorPos.y);
		if (color == CLR_INVALID) {
			std::cerr << "Failed to retrieve pixel color!" << std::endl;
		} else {
			int red = GetRValue(color);
			int green = GetGValue(color);
			int blue = GetBValue(color);
			std::cout << "Color at (" << cursorPos.x << ", " << cursorPos.y << "): "
					  << "R=" << red << " G=" << green << " B=" << blue << std::endl;
		}

		// Release the device context
		ReleaseDC(NULL, hdcScreen)
	}
#endif
	std::array<int, 4> ColorPickScreenFunc() {
		#if defined(SLATE_OS_MACOS)
			return ColorPickScreenFuncMac();
		#elif defined(SLATE_OS_WINDOWS)
			return ColorPickScreenFuncWindows();
		#else
			return {1.0, 1.0, 1.0, 1.0};
		#endif
	}

}