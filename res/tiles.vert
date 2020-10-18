#version 330 core

uniform ivec2 u_ConsoleSize;
uniform ivec2 u_CellSize;
uniform ivec2 u_TilesetSize;

layout(location = 0) in vec2 a_Quad;
layout(location = 1) in ivec2 a_Offset;
layout(location = 2) in int a_GlyphIndex;
layout(location = 3) in ivec2 a_ColorIndex;

out vec2 f_UV;
flat out ivec2 f_ColorIndex;

void main()
{
	vec2 offset = vec2(gl_InstanceID % u_ConsoleSize.x, gl_InstanceID / u_ConsoleSize.x);
	vec2 position = a_Quad + offset;
	position /= u_ConsoleSize;
	position.y = 1.0 - position.y;
	position = position * 2.0 - 1.0;

	gl_Position = vec4(position, 0.0, 1.0);

	int tileset_cols = int(u_TilesetSize.x / (u_CellSize.x + 1.0));
	ivec2 tile = ivec2(a_GlyphIndex % tileset_cols, a_GlyphIndex / tileset_cols);
	vec2 uv = a_Quad;
	uv.x = (float(u_CellSize.x) / u_TilesetSize.x) * (tile.x + uv.x) + (1.0 / u_TilesetSize.x) * tile.x;
	uv.y = (float(u_CellSize.y) / u_TilesetSize.y) * (tile.y + uv.y) + (1.0 / u_TilesetSize.y) * tile.y;

	f_UV = uv;
	f_ColorIndex = a_ColorIndex;
}
