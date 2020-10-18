#pragma once
#include "color.h"

#define CELL_WIDTH 6
#define CELL_HEIGHT 9

#define GLPH_BLOCK (0x8 * 18)

typedef struct
{
	i32 glyph;
	i32 fg_color;
	i32 bg_color;
} Cell;

typedef struct
{
	u32 cols;
	u32 rows;
} Console;
extern Console console;

void console_open(const char* title, u32 x, u32 y, u32 cols, u32 rows);

bool console_is_open();
void console_update();

void console_fill(i32 glyph, i32 fg_color, i32 bg_color);
void console_set(Point at, i32 glyph, i32 fg_color, i32 bg_color);
void console_rect(Rect rect, i32 glyph, i32 fg_color, i32 bg_color);
void console_write(Point at, const char* str, i32 fg_color, i32 bg_color);