/*
 * ftrender.cpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#include "ftrender.hpp"

extern "C" {
#include <freetype-gl.h>
#include <vertex-buffer.h>
#include <mat4.h>
#include <shader.h>
}

#include "drawing.h"
#include "helpers.h"
#include "logging.h"

using namespace ftgl;

typedef struct {
    float x, y, z;
    float s, t;
    rgba_t rgba;
} vertex_t;

vertex_buffer_t* buffer;
texture_atlas_t* atlas;
vec2 pen;
GLuint shader;
mat4 model, view, projection;

void FTGL_PreInit() {
	pen.x = pen.y = 0;
	atlas = texture_atlas_new(1024, 1024, 1);
	buffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
	mat4_set_identity(&projection);
	mat4_set_identity(&view);
	mat4_set_identity(&model);
	mat4_set_orthographic(&projection, 0, draw::width, 0, draw::height, -1, 1);
	fonts::ftgl_ESP = ftgl::texture_font_new_from_file(atlas, 14, DATA_PATH "/fonts/opensans-bold.ttf");
	logging::Info("Pre-Init done %d %d", draw::width, draw::height);
}

void FTGL_Init() {
	logging::Info("Init glew..");
	glewInit();
	logging::Info("Done...");
	glGenTextures(1, &atlas->id);
	logging::Info("Loading shaders...");
	shader = shader_load(DATA_PATH "/shaders/v3f-t2f-c4f.vert", DATA_PATH "/shaders/v3f-t2f-c4f.frag");
	logging::Info("Done init");
}

void add_text(vertex_buffer_t * buffer, texture_font_t * font,
              const std::string& text, vec2 pen, const rgba_t& fg_color_1, const rgba_t& fg_color_2, int *size_x = nullptr, int *size_y = nullptr)
{
	vec2 v_size { pen.x, 0 };
    size_t i;
    for( i = 0; i < text.length(); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, &text.at(i) );
        float kerning = 0.0f;
        if( i > 0)
        {
            kerning = texture_glyph_get_kerning( glyph, &text.at(i - 1) );
        }
        pen.x += kerning;

        /* Actual glyph */
        float x0  = ( pen.x + glyph->offset_x );
        float y0  = (int)( pen.y + glyph->offset_y );
        float x1  = ( x0 + glyph->width );
        float y1  = (int)( y0 - glyph->height );
        float s0 = glyph->s0;
        float t0 = glyph->t0;
        float s1 = glyph->s1;
        float t1 = glyph->t1;
        GLuint index = buffer->vertices->size;
        GLuint indices[] = {index, index+1, index+2,
                            index, index+2, index+3};
        vertex_t vertices[] = {
            { (int)x0,y0,0,  s0,t0,  fg_color_1 },
            { (int)x0,y1,0,  s0,t1,  fg_color_2 },
            { (int)x1,y1,0,  s1,t1,  fg_color_2 },
            { (int)x1,y0,0,  s1,t0,  fg_color_1 } };
        vertex_buffer_push_back_indices( buffer, indices, 6 );
        vertex_buffer_push_back_vertices( buffer, vertices, 4 );
        pen.x += glyph->advance_x;
        if (glyph->height > v_size.y) v_size.y = glyph->height;
    }
    v_size.x = pen.x - v_size.x;
    if (size_x) *size_x = v_size.x;
    if (size_y) *size_y = v_size.y;
}

void FTGL_Draw(const std::string& text, int x, int y, ftgl::texture_font_t* font, const rgba_t& clr, int *size_x, int *size_y) {
	rgba_t black = { 0.0, 0.0, 0.0, 1.0 };
	black.a = clr.a;
	font->rendermode = RENDER_OUTLINE_POSITIVE;
	font->outline_thickness = 1.0f;
	add_text(buffer, font, text, vec2{ x, draw::height - y - font->height }, black, black);
	font->rendermode = RENDER_NORMAL;
	font->outline_thickness = 0.0f;
	add_text(buffer, font, text, vec2{ x, draw::height - y - font->height }, clr, clr, size_x, size_y);
}

void FTGL_NewFrame() {
	vertex_buffer_clear(buffer);
}

void FTGL_ChangeFont(texture_font_t** font, const char* newfont) {
	texture_atlas_clear(atlas);
	texture_font_t* replacement = texture_font_new_from_file(atlas, 14, strfmt(DATA_PATH "/fonts/%s.ttf", newfont));
	if (replacement) {
		texture_font_delete(*font);
		*font = replacement;
	}
}

void FTGL_Render() {
	static GLint loc_texture, loc_model, loc_view, loc_projection;
	static bool loc_init { false };

	if (buffer->indices->size == 0) {
		return;
	}

	glUseProgram(shader);
	if (!loc_init) {
		loc_texture = glGetUniformLocation(shader, "texture");
		loc_model = glGetUniformLocation(shader, "model");
		loc_view = glGetUniformLocation(shader, "view");
		loc_projection = glGetUniformLocation(shader, "projection");
		loc_init = true;
	}
	glBindTexture(GL_TEXTURE_2D, atlas->id);
	if (atlas->dirty) {
		logging::Info("[DEBUG] Atlas updated.");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, atlas->data);
		atlas->dirty = false;
	}
	glUniform1i(loc_texture, 0);
	glUniformMatrix4fv(loc_model, 1, 0, model.data);
	glUniformMatrix4fv(loc_view, 1, 0, view.data);
	glUniformMatrix4fv(loc_projection, 1, 0, projection.data);
	vertex_buffer_render(buffer, GL_TRIANGLES);
}

void FTGL_StringLength(const std::string& text, ftgl::texture_font_t* font, int *size_x, int *size_y) {
	vec2 pen { 0, 0 };
	for (size_t i = 0; i < text.length(); ++i) {
		texture_glyph_t *glyph = texture_font_get_glyph(font, &text.at(i));
		float kerning = 0.0f;
		if (i) {
			kerning = texture_glyph_get_kerning(glyph, &text.at(i - 1));
		}
		pen.x += kerning;

		/* Actual glyph */
		if (pen.y < glyph->height) pen.y = glyph->height;
		pen.x += glyph->advance_x;
	}
	if (size_x) *size_x = pen.x;
	if (size_y) *size_y = pen.y;
}
