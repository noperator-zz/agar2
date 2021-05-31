#pragma once
#include <stdint.h>
#include <random>


//union color_t
//{
//	static constexpr uint32_t NUM_COLOR = 2;
//	static color_t ColorMap[NUM_COLOR];
//
//	color_t()
//		: Color(0) {}
//
//	color_t(uint32_t Color_)
//		: Color(Color_) {}
//
//	color_t(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
//		: r(r_), g(g_), b(b_), a(a_) {}
//
//	static color_t RandomColor()
//	{
//		return ColorMap[rand() % NUM_COLOR];
//	}
//
//	uint32_t Color;
//	struct
//	{
//		uint8_t r;
//		uint8_t g;
//		uint8_t b;
//		uint8_t a;
//	};
//};
