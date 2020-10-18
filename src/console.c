#include "console.h"
#include "gl_bind.h"
#include "import.h"

HWND wnd_handle;
HDC wnd_context;
HGLRC gl_context;

bool is_open = false;

u32 key_mod_flags = 0;
const u32 window_style = (WS_OVERLAPPEDWINDOW | WS_SIZEBOX | WS_VISIBLE);
i32 cell_scale = 2;
bool resolution_is_dirty = false;

Console console;
Cell* cells = NULL;
u32 cell_num = 0;

void render_init();
void render_update_resolution();
void render_draw();

// Structs for handing events
// WM_KEYDOWN & WM_KEYUP
typedef struct
{
	// Repeat count for this key
	u32 repeat		: 16;
	// Hardware scancode of key
	u32 scancode	: 8;
	// Was extended (CTRL/ALT/etc)
	u32 extended	: 1;
	// Dont touch!
	u32 reserved	: 4;
	// Context code, always 0 for WM_KEYDOWN
	u32 context		: 1;
	// Previous key state (1 if key was down, 0 if it was up)
	u32 previous	: 1;
	// Always 0 
	u32 transition	: 1;
} Win_Key_Params;

// WM_MOUSEMOVE
typedef struct
{
	u16 x;
	u16 y;
} Win_MouseMove_Params;

// WM_SIZE
typedef struct
{
	u16 width;
	u16 height;
} Win_Size_Params;

/* CLIENT TO WINDOW SIZE CONVERSIONS AHHH */
Point client_to_wnd_delta()
{
	RECT cli_rect;
	cli_rect.left = 0;
	cli_rect.top= 0;
	cli_rect.right = 0;
	cli_rect.bottom = 0;

	AdjustWindowRect(&cli_rect, window_style, false);
	return point(cli_rect.right - cli_rect.left, cli_rect.bottom - cli_rect.top);
}

void client_to_wnd_size(u32* width, u32* height)
{
	Point delta = client_to_wnd_delta();

	*width += delta.x;
	*height += delta.y;
}

void wnd_to_client_size(u32* width, u32* height)
{
	Point delta = client_to_wnd_delta();

	*width -= delta.x;
	*height -= delta.y;
}

/* WND PROC */
LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_SIZING:
		{
			RECT* size_rect = (RECT*)lparam;
			u32 unit_width = CELL_WIDTH * cell_scale;
			u32 unit_height = CELL_HEIGHT * cell_scale;

			u32 width = size_rect->right - size_rect->left;
			u32 height = size_rect->bottom - size_rect->top;

			// Convert to client-space and constrain to cells
			wnd_to_client_size(&width, &height);
			width = (width / unit_width) * unit_width;
			height = (height / unit_height) * unit_height;

			// Convert back to window-space
			client_to_wnd_size(&width, &height);

			// Re-set the size rect, based on which corner we're dragging...
			switch(wparam)
			{
				case WMSZ_RIGHT:
					size_rect->right = size_rect->left + width;
					break;
				case WMSZ_BOTTOMRIGHT:
					size_rect->right = size_rect->left + width;
					size_rect->bottom = size_rect->top + height;
					break;
				case WMSZ_BOTTOM:
					size_rect->bottom = size_rect->top + height;
					break;
				case WMSZ_BOTTOMLEFT:
					size_rect->left = size_rect->right - width;
					size_rect->bottom = size_rect->top + height;
					break;
				case WMSZ_LEFT:
					size_rect->left = size_rect->right - width;
					break;
				case WMSZ_TOPLEFT:
					size_rect->left = size_rect->right - width;
					size_rect->top = size_rect->bottom - height;
					break;
				case WMSZ_TOP:
					size_rect->top = size_rect->bottom - height;
					break;
				case WMSZ_TOPRIGHT:
					size_rect->right = size_rect->left + width;
					size_rect->top = size_rect->bottom - height;
					break;
			}
			break;
		}

		case WM_SIZE:
		{
			resolution_is_dirty = true;
			Win_Size_Params* size = (Win_Size_Params*)&lparam;

			console.cols = size->width / (CELL_WIDTH * cell_scale);
			console.rows = size->height / (CELL_HEIGHT * cell_scale);

			glViewport(0, 0, size->width, size->height);
			break;
		}

		case WM_CLOSE:
		{
			is_open = false;
			break;
		}

		/* KEY STUFF */
		/*
		case WM_KEYDOWN:
		{
			Win_Key_Params* key = (Win_Key_Params*)&lparam;
			Key_Event key_ev;
			key_ev.scancode = key->scancode;
			key_ev.keychar = wparam;
			key_ev.press = true;
			key_ev.repeat = key->previous;

			game_key_ev(key_ev);
			break;
		}

		case WM_KEYUP:
		{
			Win_Key_Params* key = (Win_Key_Params*)&lparam;
			Key_Event key_ev;
			key_ev.scancode = key->scancode;
			key_ev.keychar = wparam;
			key_ev.press = false;
			key_ev.repeat = false;

			game_key_ev(key_ev);
			break;
		}
		*/
	}

	return DefWindowProc(wnd, msg, wparam, lparam);
}

void console_open(const char* title, u32 x, u32 y, u32 cols, u32 rows)
{
	static bool class_was_registered = false;
	static const LPCSTR class_name = "WindowClass";

	// Init opengl!
	init_opengl();

	// Get module instance
	HINSTANCE instance = GetModuleHandle(NULL);

	// Register window class if we haven't
	if (!class_was_registered)
	{
		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = wnd_proc;
		wc.hInstance = instance;
		wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
		wc.lpszClassName = class_name;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.style = CS_OWNDC;

		RegisterClass(&wc);
		class_was_registered = true;
	}

	// Calculate window-size so that the client has the specified size
	u32 wnd_width = cols * CELL_WIDTH * cell_scale;
	u32 wnd_height = rows * CELL_HEIGHT * cell_scale;
	client_to_wnd_size(&wnd_width, &wnd_height);

	console.cols = cols;
	console.rows = rows;

	// Open window!
	wnd_handle = CreateWindow(
		class_name,
		title,
		window_style,
		x, y, wnd_width, wnd_height,
		0, 0,
		instance,
		0
	);

	/** Create GL context **/
	HDC device_context = GetDC(wnd_handle);

	// Fetch pixel format
	i32 pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
		WGL_ACCELERATION_ARB,		WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,			WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,			32,
		WGL_DEPTH_BITS_ARB,			24,
		WGL_STENCIL_BITS_ARB,		8,
		WGL_SAMPLE_BUFFERS_ARB,		1,
		WGL_SAMPLES_ARB,			0,
		0
	};

	i32 pixel_format;
	UINT num_formats;

	wglChoosePixelFormatARB(device_context, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);

	// Set that format
	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(device_context, pixel_format, sizeof(pfd), &pfd);
	SetPixelFormat(device_context, pixel_format, &pfd);

	// Initialize 3.3 context
	i32 context_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB,		3,
		WGL_CONTEXT_MINOR_VERSION_ARB,		3,
		WGL_CONTEXT_PROFILE_MASK_ARB,		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	HGLRC gl_context = wglCreateContextAttribsARB(device_context, 0, context_attribs);

	wglMakeCurrent(device_context, gl_context);

	// Disable VSYNC, bro
	wglSwapIntervalEXT(0);

	wnd_context = device_context;
	gl_context = gl_context;

	// Init rendering
	render_init();
	render_update_resolution();

	is_open = true;
}

bool console_is_open()
{
	return is_open;
}

void console_update()
{
	render_draw();

	SwapBuffers(wnd_context);
	Sleep(1);

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (resolution_is_dirty)
	{
		render_update_resolution();
		resolution_is_dirty = false;
	}
}

inline Cell* cell_get(Point at)
{
	if (at.x < 0 || at.x >= (i32)console.cols ||
		at.y < 0 || at.y >= (i32)console.rows)
		return NULL;

	return &cells[at.x + at.y * console.cols];
}

void console_fill(i32 glyph, i32 fg_color, i32 bg_color)
{
	for(u32 i=0; i<console.cols * console.rows; ++i)
	{
		if (glyph >= 0)
			cells[i].glyph = glyph;
		if (fg_color >= 0)
			cells[i].fg_color = fg_color;
		if (bg_color >= 0)
			cells[i].bg_color = bg_color;
	}
}

void console_set(Point at, i32 glyph, i32 fg_color, i32 bg_color)
{
	Cell* cell = cell_get(at);
	if (!cell)
		return;

	if (glyph >= 0)
		cell->glyph = glyph;
	if (fg_color >= 0)
		cell->fg_color = fg_color;
	if (bg_color >= 0)
		cell->bg_color = bg_color;
}

void console_rect(Rect rect, i32 glyph, i32 fg_color, i32 bg_color)
{
	for(i32 y=rect.min.y; y<=rect.max.y; ++y)
		for(i32 x=rect.min.x; x<=rect.max.x; ++x)
			console_set(point(x, y), glyph, fg_color, bg_color);
}

void console_write(Point at, const char* str, i32 fg_color, i32 bg_color)
{
	while(*str)
	{
		console_set(at, *str, fg_color, bg_color);
		str++;
		at.x++;
	}
}

/* RENDERING */
GLuint cell_vbo;
GLuint program;

GLint u_ConsoleSize;

void render_init()
{
	GLuint vao;
	GLuint quad_vbo;

	GLuint font_texture;
	GLuint color_texture;

	// Setup vertex objects
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* CELL QUAD VBO */
	float quad_data[] = {
		0.f, 0.f,
		1.f, 0.f,
		0.f, 1.f,

		1.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
	};

	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

	/* CELL DATA VBO */
	glGenBuffers(1, &cell_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cell_vbo);

	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(2, 1, GL_INT, 3 * sizeof(i32), 0);
	glVertexAttribIPointer(3, 2, GL_INT, 3 * sizeof(i32), (void*)(1 * sizeof(i32)));
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);

	// Setup the shaders
	u32 vert_len;
	u32 frag_len;
	char* vert_src = file_read_all("res\\tiles.vert", &vert_len);
	char* frag_src = file_read_all("res\\tiles.frag", &frag_len);

	GLuint vert_shdr = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shdr, 1, &vert_src, &vert_len);
	glCompileShader(vert_shdr);

	GLuint frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shdr, 1, &frag_src, &frag_len);
	glCompileShader(frag_shdr);

	program = glCreateProgram();
	glAttachShader(program, vert_shdr);
	glAttachShader(program, frag_shdr);
	glLinkProgram(program);
	glUseProgram(program);

	char INFO_BUFFER[256];
	glGetProgramInfoLog(program, 256, NULL, INFO_BUFFER);
	log_write(INFO_BUFFER);

	// Load the font
	Tga_File font_tga;
	tga_load(&font_tga, "res/font.tga");

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &font_texture);
	glBindTexture(GL_TEXTURE_2D, font_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font_tga.width, font_tga.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, font_tga.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	Tga_File color_tga;
	tga_load(&color_tga, "res/colors.tga");

	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_tga.width, color_tga.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, color_tga.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Set cell uniforms
	u_ConsoleSize = glGetUniformLocation(program, "u_ConsoleSize");
	GLint u_CellSize = glGetUniformLocation(program, "u_CellSize");
	GLint u_TilesetSize = glGetUniformLocation(program, "u_TilesetSize");
	GLint u_ColorMapSize = glGetUniformLocation(program, "u_ColorMapSize");
	GLint u_ColorSampler = glGetUniformLocation(program, "u_ColorSampler");
	glUniform2i(u_CellSize, CELL_WIDTH, CELL_HEIGHT);
	glUniform2i(u_ConsoleSize, console.cols, console.rows);
	glUniform2i(u_TilesetSize, font_tga.width, font_tga.height);
	glUniform2i(u_ColorMapSize, color_tga.width, color_tga.height);
	glUniform1i(u_ColorSampler, 1);

	tga_free(&font_tga);
	tga_free(&color_tga);
}

void render_update_resolution()
{
	glUniform2i(u_ConsoleSize, console.cols, console.rows);

	if (cell_num < console.cols * console.rows)
	{
		if (cells)
			free(cells);

		cell_num = console.cols * console.rows;
		cells = malloc(sizeof(Cell) * cell_num);
	}

	memzero(cells, sizeof(Cell) * cell_num);
}


void render_draw()
{
	// Update the cell vao
	u32 cells_size = sizeof(Cell) * console.cols * console.rows;

	glBindBuffer(GL_ARRAY_BUFFER, cell_vbo);
	glBufferData(GL_ARRAY_BUFFER, cells_size, cells, GL_STREAM_DRAW);

	glUseProgram(program);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, console.cols * console.rows);
}