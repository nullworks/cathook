/*
 * drawgl.cpp
 *
 *  Created on: May 21, 2017
 *      Author: nullifiedcat
 */

#include "drawgl.hpp"
#include "common.h"
#include "atlas.hpp"

extern "C" {
#include <mat4.h>
#include <vertex-buffer.h>
#include <shader.h>
}

namespace drawgl {

using namespace ftgl;

const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

vertex_buffer_t* buffer_lines;
vertex_buffer_t* buffer_triangles_plain;
vertex_buffer_t* buffer_triangles_textured;

GLuint shader_v2fc4f;
GLuint shader_v2ft2fc4f;

mat4 model, view, projection;

bool ready_state = false;

void Initialize() {
	buffer_lines = vertex_buffer_new("vertex:2f,color:4f");
	buffer_triangles_plain = vertex_buffer_new("vertex:2f,color:4f");
	buffer_triangles_textured = vertex_buffer_new("vertex:2f,tex_coord:2f,color:4f");

	mat4_set_identity(&projection);
	mat4_set_identity(&view);
	mat4_set_identity(&model);
	mat4_set_orthographic(&projection, 0, draw::width, draw::height, 0, -1, 1);
	//view.m11 = -1;

	shader_v2fc4f = shader_load(DATA_PATH "/shaders/v2f-c4f.vert", DATA_PATH "/shaders/v2f-c4f.frag");
	glUseProgram(shader_v2fc4f);
	{
		glUniformMatrix4fv(glGetUniformLocation(shader_v2fc4f, "model"), 1, 0, model.data);
		glUniformMatrix4fv(glGetUniformLocation(shader_v2fc4f, "view"), 1, 0, view.data);
		glUniformMatrix4fv(glGetUniformLocation(shader_v2fc4f, "projection"), 1, 0, projection.data);
	}

	shader_v2ft2fc4f = shader_load(DATA_PATH "/shaders/v2f-t2f-c4f.vert", DATA_PATH "/shaders/v2f-t2f-c4f.frag");
	glUseProgram(shader_v2ft2fc4f);
	{
		glUniform1i(glGetUniformLocation(shader_v2ft2fc4f, "texture"), 0);
		glUniformMatrix4fv(glGetUniformLocation(shader_v2ft2fc4f, "model"), 1, 0, model.data);
		glUniformMatrix4fv(glGetUniformLocation(shader_v2ft2fc4f, "view"), 1, 0, view.data);
		glUniformMatrix4fv(glGetUniformLocation(shader_v2ft2fc4f, "projection"), 1, 0, projection.data);
	}

	glUseProgram(0);
	// Texture atlas
	textures::Init();

	// Do not fucking ask. Without this, it crashes.
	drawgl::Rect(0, 0, 0, 0);
	drawgl::FilledRect(0, 0, 0, 0);
	drawgl::TexturedRect(0, 0, 0, 0, 0, 0, 0, 0);
	ready_state = true;
	Render();
}

void FilledRect(float x, float y, float w, float h, const float* rgba) {
	GLuint idx = buffer_triangles_plain->vertices->size;
	//
	// 3 - 2
	// | / |
	// 0 - 1
	//
	GLuint indices[] = { idx, idx + 1, idx + 2, idx, idx + 2, idx + 3 };
	vertex_v2c4_t vertices[] = {
			{ vec2{ x, y }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + w, y }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + w, y + h }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x, y + h }, *reinterpret_cast<const vec4*>(rgba) }
	};
	vertex_buffer_push_back_indices(buffer_triangles_plain, indices, 6);
	vertex_buffer_push_back_vertices(buffer_triangles_plain, vertices, 4);
}

void Line(float x, float y, float dx, float dy, const float* rgba) {
	GLuint idx = buffer_lines->vertices->size;
	GLuint indices[] = { idx, idx + 1 };
	vertex_v2c4_t vertices[] = {
			{ vec2{ x, y }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + dx, y + dy }, *reinterpret_cast<const vec4*>(rgba) }
	};
	vertex_buffer_push_back_indices(buffer_lines, indices, 2);
	vertex_buffer_push_back_vertices(buffer_lines, vertices, 2);
}

void Rect(float x, float y, float w, float h, const float* rgba) {
	GLuint idx = buffer_lines->vertices->size;
	GLuint indices[] = { idx, idx + 1, idx + 1, idx + 2, idx + 2, idx + 3, idx + 3, idx };
	vertex_v2c4_t vertices[] = {
			{ vec2{ x + 0.5f, y + 0.5f }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + w, y + 0.5f }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + w, y + h - 0.375f }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + 0.5f, y + h - 0.375f }, *reinterpret_cast<const vec4*>(rgba) }
	};
	vertex_buffer_push_back_indices(buffer_lines, indices, 8);
	vertex_buffer_push_back_vertices(buffer_lines, vertices, 4);
}

void TexturedRect(float x, float y, float w, float h, float u, float v, float u2, float v2, const float* rgba) {
	GLuint idx = buffer_triangles_textured->vertices->size;
	GLuint indices[] = { idx, idx + 1, idx + 2, idx, idx + 2, idx + 3 };
	vertex_v2t2c4_t vertices[] = {
			{ vec2{ x, y }, vec2{ u, v2 }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + w, y }, vec2{ u2, v2 }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x + w, y + h }, vec2{ u2, v }, *reinterpret_cast<const vec4*>(rgba) },
			{ vec2{ x, y + h }, vec2{ u, v }, *reinterpret_cast<const vec4*>(rgba) }
	};
	vertex_buffer_push_back_indices(buffer_triangles_textured, indices, 6);
	vertex_buffer_push_back_vertices(buffer_triangles_textured, vertices, 4);
}

void Refresh() {
	vertex_buffer_clear(buffer_triangles_plain);
	vertex_buffer_clear(buffer_triangles_textured);
	vertex_buffer_clear(buffer_lines);
}

void PreRender() {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_EDGE_FLAG_ARRAY);
	glDisableClientState(GL_FOG_COORD_ARRAY);
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_INDEX_ARRAY);
}

void PostRender() {
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopClientAttrib();
	glPopAttrib();
}

void Render() {
	glUseProgram(shader_v2fc4f);
	vertex_buffer_render(buffer_triangles_plain, GL_TRIANGLES);
	vertex_buffer_render(buffer_lines, GL_LINES);
	glUseProgram(shader_v2ft2fc4f);
	glBindTexture(GL_TEXTURE_2D, textures::texture);
	vertex_buffer_render(buffer_triangles_textured, GL_TRIANGLES);
}

}
