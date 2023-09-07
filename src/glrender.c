/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-06 Created.
 *  2023-09-05 Refactored and supported for Qt6.
 */

#include "suika.h"
#include "glrender.h"

/*
 * Include headers.
 */

/*
 * Windows
 *  - We use OpenGL 3.2
 */
#if defined(WIN)
#include <windows.h>
#include <GL/gl.h>
#include "glhelper.h"
#endif

/*
 * macOS
 *  - We use OpenGL 3.2
 */
#if defined(OSX)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif

/*
 * iOS
 *  - We use OpenGL ES 3.0
 */
#if defined(IOS)
#define GL_SILENCE_DEPRECATION
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

/*
 * Android
 *  - We use OpenGL ES 3.0
 */
#if defined(ANDROID)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * Emscripten
 *  - We use OpenGL ES 3.0
 */
#if defined(EM)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * Linux (excluding Qt)
 *  - We use OpenGL 3.2
 */
#if defined(LINUX) && !defined(USE_QT)
#include <GL/gl.h>
#include "glhelper.h"
#endif

/*
 * Qt
 *  - We use a wrapper for QOpenGLFunctions class
 */
#if defined(USE_QT)
#include <GL/gl.h>
#include "glhelper.h"
#endif

/*
 * Switch
 *  - We use OpenGL ES 3.0
 */
#if defined(SWITCH)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * SDL2
 *  - We simply use GLEW because the SDL2 port is just for porting base.
 */
#if defined(USE_SDL2_OPENGL)
#include <GL/glew.h>
#endif

/*
 * The sole vertex shader that is shared between all fragment shaders.
 */
static GLuint vertex_shader;

/*
 * Fragment shaders.
 */

/* The normal alpha blending. */
static GLuint fragment_shader_normal;

/* The character dimming. (RGB 50%) */
static GLuint fragment_shader_dim;

/* The rule shader. (1-bit universal transition) */
static GLuint fragment_shader_rule;

/* The melt shader. (8-bit universal transition) */
static GLuint fragment_shader_melt;

/*
 * Program per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint program_normal;

/* For the character dimming. (RGB 50%) */
static GLuint program_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint program_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint program_melt;

/*
 * VAO per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint vao_normal;

/* For the character dimming. (RGB 50%) */
static GLuint vao_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint vao_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint vao_melt;

/*
 * VBO per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint vbo_normal;

/* For the character dimming. (RGB 50%) */
static GLuint vbo_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint vbo_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint vbo_melt;

/*
 * IBO per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint ibo_normal;

/* For the character dimming. (RGB 50%) */
static GLuint ibo_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint ibo_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint ibo_melt;

/*
 * The vertex shader source.
 */

/* The source string. */
static const char *vertex_shader_src =
#if !defined(EM)
	"#version 100                 \n"
#endif
	"attribute vec4 a_position;   \n" /* FIXME: vec3? */
	"attribute vec2 a_texCoord;   \n"
	"attribute float a_alpha;     \n"
	"varying vec2 v_texCoord;     \n"
	"varying float v_alpha;       \n"
	"void main()                  \n"
	"{                            \n"
	"  gl_Position = a_position;  \n"
	"  v_texCoord = a_texCoord;   \n"
	"  v_alpha = a_alpha;         \n"
	"}                            \n";

/* This is our vertex format. (6 parameters, XYZUVA...) */
#define V_POS	(0)
#define V_TEX	(3)
#define V_ALPHA	(5)
#define V_ALL	(6)
struct pseudo_vertex_info {
	/* 0 (V_POS) */		float x;
	/* 1 */			float y;
	/* 2 */			float z;
	/* 3 (V_TEX) */		float u;
	/* 4 */			float v;
	/* 5 (V_ALPHA) */	float alpha;
};

/*
 * Fragment shader sources.
 */

/* The normal alpha blending shader. */
static const char *fragment_shader_src_normal =
#if !defined(EM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT)
	"#undef mediump                                      \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"varying float v_alpha;                              \n"
	"uniform sampler2D s_texture;                        \n"
	"void main()                                         \n"
	"{                                                   \n"
	"  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  tex.a = tex.a * v_alpha;                          \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/* The character dimming shader. (RGB 50%) */
static const char *fragment_shader_src_dim =
#if !defined(EM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT)
	"#undef mediump                                      \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"uniform sampler2D s_texture;                        \n"
	"void main()                                         \n"
	"{                                                   \n"
	"  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  tex.r = tex.r * 0.5;                              \n"
	"  tex.g = tex.g * 0.5;                              \n"
	"  tex.b = tex.b * 0.5;                              \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/* The rule shader. (1-bit universal transition) */
static const char *fragment_shader_src_rule =
#if !defined(EM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT)
	"#undef mediump                                      \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"varying float v_alpha;                              \n"
	"uniform sampler2D s_texture;                        \n"
	"uniform sampler2D s_rule;                           \n"
	"void main()                                         \n"
	"{                                                   \n"
        "  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  vec4 rule = texture2D(s_rule, v_texCoord);        \n"
	"  tex.a = 1.0 - step(v_alpha, rule.b);              \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/* The melt shader. (8-bit universal transition) */
static const char *fragment_shader_src_melt =
#if !defined(EM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT)
	"#undef mediump                                      \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"varying float v_alpha;                              \n"
	"uniform sampler2D s_texture;                        \n"
	"uniform sampler2D s_rule;                           \n"
	"void main()                                         \n"
	"{                                                   \n"
        "  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  vec4 rule = texture2D(s_rule, v_texCoord);        \n"
	"  tex.a = clamp((1.0 - rule.b) + (v_alpha * 2.0 - 1.0), 0.0, 1.0); \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/*
 * Internal texture management struct.
 *  - We call glGenTextures() when a texture first unlocked
 */
struct texture {
	/* Texture ID. */
	GLuint id;

	/* This shows whether glGenTextures() was called. */
	bool is_initialized;
};

/*
 * The following functions are defined in this file if we don't use Qt.
 * In the case we use Qt, they are defined in openglwidget.cpp because
 * VAO/VBO/IBO are abstracted in a very different way on the framework.
 */
bool setup_vertex_shader(const char **vshader_src, GLuint *vshader);
bool setup_fragment_shader(const char **fshader_src, GLuint vshader,
			   bool use_second_texture, GLuint *fshader,
			   GLuint *prog, GLuint *vao, GLuint *vbo,
			   GLuint *ibo);
void cleanup_vertex_shader(GLuint vshader);
void cleanup_fragment_shader(GLuint fshader, GLuint prog, GLuint vao,
			     GLuint vbo, GLuint ibo);

/*
 * Forward declaration.
 */
static void draw_elements(int dst_left, int dst_top,
			  struct image * RESTRICT src_image,
			  struct image * RESTRICT rule_image,
			  bool is_dim, bool is_melt,
			  int width, int height,
			  int src_left, int src_top,
			  int alpha, int bt);

/*
 * Initialize the Suika2's OpenGL rendering subsystem.
 */
bool init_opengl(void)
{
#ifndef USE_QT
	/* Set a viewport. */
	glViewport(0, 0, conf_window_width, conf_window_height);
#endif

	/* Setup a vertex shader. */
	if (!setup_vertex_shader(&vertex_shader_src, &vertex_shader))
		return false;

	/* Setup the fragment shader for normal alpha blending. */
	if (!setup_fragment_shader(&fragment_shader_src_normal,
				   vertex_shader,
				   false, /* no second texture */
				   &fragment_shader_normal,
				   &program_normal,
				   &vao_normal,
				   &vbo_normal,
				   &ibo_normal))
		return false;

	/* Setup the fragment shader for character dimming (RGB 50%). */
	if (!setup_fragment_shader(&fragment_shader_src_dim,
				   vertex_shader,
				   false, /* no second texture */
				   &fragment_shader_dim,
				   &program_dim,
				   &vao_dim,
				   &vbo_dim,
				   &ibo_dim))
		return false;

	/* Setup the fragment shader for rule (1-bit universal transition). */
	if (!setup_fragment_shader(&fragment_shader_src_rule,
				   vertex_shader,
				   false, /* use second texture */
				   &fragment_shader_rule,
				   &program_rule,
				   &vao_rule,
				   &vbo_rule,
				   &ibo_rule))
		return false;

	/* Setup the fragment shader for melt (8-bit universal transition). */
	if (!setup_fragment_shader(&fragment_shader_src_melt,
				   vertex_shader,
				   true, /* use second texture */
				   &fragment_shader_melt,
				   &program_melt,
				   &vao_melt,
				   &vbo_melt,
				   &ibo_melt))
		return false;

	return true;
}

#ifndef USE_QT

/*
 * Setup a vertex shader.
 */
bool
setup_vertex_shader(
	const char **vshader_src,	/* IN: A vertex shader source string. */
	GLuint *vshader)		/* OUT: A vertex shader ID. */
{
	char buf[1024];
	GLint compiled;
	int len;

	*vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(*vshader, 1, vshader_src, NULL);
	glCompileShader(*vshader);

	/* Check errors. */
	glGetShaderiv(*vshader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		log_info("Vertex shader compile error");
		glGetShaderInfoLog(*vshader, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	return true;
}

/*
 * Cleanup a vertex shader.
 */
void cleanup_vertex_shader(GLuint vshader)
{
	glDeleteShader(vshader);
}

/*
 * Setup a fragment shader, a program, a VAO, a VBO and an IBO.
 */
bool
setup_fragment_shader(
	const char **fshader_src,	/* IN: A fragment shader source string. */
	GLuint vshader,			/* IN: A vertex shader ID. */
	bool use_second_texture,	/* IN: Whether to use a second texture. */
	GLuint *fshader,		/* OUT: A fragment shader ID. */
	GLuint *prog,			/* OUT: A program ID. */
	GLuint *vao,			/* OUT: A VAO ID. */
	GLuint *vbo,			/* OUT: A VBO ID. */
	GLuint *ibo)			/* OUT: An IBO ID. */
{
	char err_msg[1024];
	GLint pos_loc, tex_loc, alpha_loc, sampler_loc, rule_loc;
	GLint is_succeeded;
	int err_len;

	const GLushort indices[] = {0, 1, 2, 3};

	/* Create a fragment shader. */
	*fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(*fshader, 1, fshader_src, NULL);
	glCompileShader(*fshader);

	/* Check errors. */
	glGetShaderiv(*fshader, GL_COMPILE_STATUS, &is_succeeded);
	if (!is_succeeded) {
		log_info("Fragment shader compile error");
		glGetShaderInfoLog(*fshader, sizeof(err_msg), &err_len, &err_msg[0]);
		log_info("%s", err_msg);
		return false;
	}

	/* Create a program. */
	*prog = glCreateProgram();
	glAttachShader(*prog, vshader);
	glAttachShader(*prog, *fshader);
	glLinkProgram(*prog);
	glGetProgramiv(*prog, GL_LINK_STATUS, &is_succeeded);
	if (!is_succeeded) {
		log_info("Program link error\n");
		glGetProgramInfoLog(*prog, sizeof(err_msg), &err_len, &err_msg[0]);
		log_info("%s", err_msg);
		return false;
	}
	glUseProgram(*prog);

	/* Create a VAO. */
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	/* Create a VBO. */
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	/* Set the vertex attibute for "a_position" (V_POS) in the vertex shader. */
	pos_loc = glGetAttribLocation(*prog, "a_position");
	glVertexAttribPointer((GLuint)pos_loc,
			      3,	/* (x, y, z) */
			      GL_FLOAT,
			      GL_FALSE,
			      V_ALL * sizeof(GLfloat),
			      V_POS);
	glEnableVertexAttribArray((GLuint)pos_loc);

	/* Set the vertex attibute for "a_texCoord" (V_TEX) in the vertex shader. */
	tex_loc = glGetAttribLocation(*prog, "a_texCoord");
	glVertexAttribPointer((GLuint)tex_loc,
			      2,	/* (u, v) */
			      GL_FLOAT,
			      GL_FALSE,
			      V_ALL * sizeof(GLfloat),
			      (const GLvoid *)(V_TEX * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)tex_loc);

	/* Set the vertex attibute for "a_alpha" (V_ALPHA) in the vertex shader. */
	alpha_loc = glGetAttribLocation(*prog, "a_alpha");
	glVertexAttribPointer((GLuint)alpha_loc,
			      1,	/* (alpha) */
			      GL_FLOAT,
			      GL_FALSE,
			      V_ALL * sizeof(GLfloat),
			      (const GLvoid *)(V_ALPHA * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)alpha_loc);

	/* Setup "s_texture" in a fragment shader. */
	sampler_loc = glGetUniformLocation(*prog, "s_texture");
	glUniform1i(sampler_loc, 0);

	/* Setup "s_rule" in a fragment shader if we use a second texture. */
	if (use_second_texture) {
		rule_loc = glGetUniformLocation(*prog, "s_rule");
		glUniform1i(rule_loc, 1);
	}

	/* Create an IBO. */
	glGenBuffers(1, ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STATIC_DRAW);

	return true;
}

void
cleanup_fragment_shader(
	GLuint fshader,
	GLuint prog,
	GLuint vao,
	GLuint vbo,
	GLuint ibo)
{
	GLuint a[1];

	/* Delete a fragment shader. */
	glDeleteShader(fshader);

	/* Delete a program. */
	glDeleteProgram(prog);

	/* Delete a VAO. */
	a[0] = vao;
	glDeleteVertexArrays(1, (const GLuint *)&a);

	/* Delete a VBO. */
	a[0] = vbo;
	glDeleteBuffers(1, (const GLuint *)&a);

	/* Delete an IBO. */
	a[0] = ibo;
	glDeleteBuffers(1, (const GLuint *)&a);
}

#endif	/* ifndef USE_QT */

/*
 * Cleanup the Suika2's OpenGL rendering subsystem.
 *  - Note: On Emscripten, this will never be called
 */
void cleanup_opengl(void)
{
	cleanup_fragment_shader(fragment_shader_normal,
				program_normal,
				vao_normal,
				vbo_normal,
				ibo_normal);
	cleanup_fragment_shader(fragment_shader_dim,
				program_dim,
				vao_dim,
				vbo_dim,
				ibo_dim);
	cleanup_fragment_shader(fragment_shader_rule,
				program_rule,
				vao_rule,
				vbo_rule,
				ibo_rule);
	cleanup_fragment_shader(fragment_shader_melt,
				program_melt,
				vao_melt,
				vbo_melt,
				ibo_melt);
	cleanup_vertex_shader(vertex_shader);
}

/*
 * Start a frame rendering.
 */
void opengl_start_rendering(void)
{
#ifndef USE_QT
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#else
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

/*
 * End a frame rendering.
 */
void opengl_end_rendering(void)
{
	glFlush();
}

/*
 * Texture manipulation:
 *  - "Texture" here is a GPU backend of an image
 *  - Suika2 abstracts modifications of textures by "lock/unlock" operations
 *  - However, OpenGL doesn't have a mechanism to lock pixels
 *  - Thus, we need to call glTexImage2D() to update entire texture for every "unlock" operations
 */

/*
 * Lock a texture.
 *  - This will just allocate memory for a texture management struct
 *  - We just use pixels of a frontend image for modification
 */
bool
opengl_lock_texture(
	int width,			/* IN: Image width */
	int height,			/* IN: Image height */
	pixel_t *pixels,		/* IN: Image pixels */
	pixel_t **locked_pixels,	/* OUT: Pixel pointer to modify image */
	void **texture)			/* OUT: Texture object */
{
	struct texture *tex;

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);

	assert(*locked_pixels == NULL);

	/*
	 * If a texture object for the image that uses "pixels" is not created yet.
	 * (In other words, the image was created by create_image(), but still
	 *  not drawn or cleared. This lazy initialization achieves a bit
	 *  optimization.)
	 */
	if (*texture == NULL) {
		/* Allocate memory for a texture struct. */
		tex = malloc(sizeof(struct texture));
		if (tex == NULL) {
			log_memory();
			return false;
		}

		tex->is_initialized = false;
		*texture = tex;
	}

	/*
	 * For image updates until unlock, we'll just use the area
	 * that "pixels" points to.
	 */
	*locked_pixels = pixels;

	return true;
}

/*
 * Unlock a texture.
 *  - This function uploads the contents that "pixels" points to,
 *    from CPU memory to GPU memory
 */
void
opengl_unlock_texture(
	int width,			/* IN: Image width */
	int height,			/* IN: Image height */
	pixel_t *pixels,		/* IN: Image pixels */
	pixel_t **locked_pixels,	/* IN/OUT: Pixel pointer to modify image (to be NULL) */
	void **texture)			/* IN: Texture object */
{
	struct texture *tex;

	UNUSED_PARAMETER(pixels);

	assert(*locked_pixels != NULL);

	tex = (struct texture *)*texture;

	/* If this is the first unlock. */
	if (!tex->is_initialized) {
		glGenTextures(1, &tex->id);
		tex->is_initialized = true;
	}

	/* Create or update an OpenGL texture. */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, tex->id);
#ifdef EM
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, *locked_pixels);
	glActiveTexture(GL_TEXTURE0);

	/* Set NULL to "locked_pixels" to show it is not active. */
	*locked_pixels = NULL;
}

/*
 * Destroy a texture.
 */
void opengl_destroy_texture(void *texture)
{
	struct texture *tex;

	/* Destroy if a texture struct is allocated. */
	if (texture != NULL) {
		tex = (struct texture *)texture;

		/* Delete an OpenGL texture if it exists. */
		if (tex->is_initialized)
			glDeleteTextures(1, &tex->id);

		/* Free a texture struct. */
		free(tex);
	}
}

/*
 * 画面にイメージをレンダリングする
 */
void opengl_render_image(int dst_left, int dst_top,
			 struct image * RESTRICT src_image, int width,
			 int height, int src_left, int src_top, int alpha,
			 int bt)
{
	UNUSED_PARAMETER(bt);

	/* 描画の必要があるか判定する */
	if (alpha == 0 || width == 0 || height == 0)
		return;	/* 描画の必要がない */
	if (!clip_by_source(get_image_width(src_image),
			   get_image_height(src_image),
			   &width, &height, &dst_left, &dst_top, &src_left,
			   &src_top))
		return;	/* 描画範囲外 */
	if (!clip_by_dest(conf_window_width, conf_window_height, &width,
			 &height, &dst_left, &dst_top, &src_left, &src_top))
		return;	/* 描画範囲外 */

	draw_elements(dst_left, dst_top, src_image, NULL, false, false,
		      width, height, src_left, src_top, alpha, bt);
}

/*
 * 画面にイメージを暗くレンダリングする
 */
void opengl_render_image_dim(int dst_left, int dst_top,
			     struct image * RESTRICT src_image, int width,
			     int height, int src_left, int src_top)
{
	/* 描画の必要があるか判定する */
	if (width == 0 || height == 0)
		return;	/* 描画の必要がない */
	if (!clip_by_source(get_image_width(src_image),
			   get_image_height(src_image),
			   &width, &height, &dst_left, &dst_top, &src_left,
			   &src_top))
		return;	/* 描画範囲外 */
	if (!clip_by_dest(conf_window_width, conf_window_height, &width,
			 &height, &dst_left, &dst_top, &src_left, &src_top))
		return;	/* 描画範囲外 */

	draw_elements(dst_left, dst_top, src_image, NULL, true, false,
		      width, height, src_left, src_top, 255, BLEND_FAST);
}

/*
 * 画面にイメージをルール付きでレンダリングする
 */
void opengl_render_image_rule(struct image * RESTRICT src_image,
			      struct image * RESTRICT rule_image,
			      int threshold)
{
	draw_elements(0, 0, src_image, rule_image, false, false,
		      conf_window_width, conf_window_height,
		      0, 0, threshold, BLEND_FAST);

}

/*
 * 画面にイメージをルール付き(メルト)でレンダリングする
 */
void opengl_render_image_melt(struct image * RESTRICT src_image,
			      struct image * RESTRICT rule_image,
			      int threshold)
{
	draw_elements(0, 0, src_image, rule_image, false, true,
		      conf_window_width, conf_window_height,
		      0, 0, threshold, BLEND_FAST);

}

/* 画像を描画する */
static void draw_elements(int dst_left, int dst_top,
			  struct image * RESTRICT src_image,
			  struct image * RESTRICT rule_image,
			  bool is_dim, bool is_melt,
			  int width, int height,
			  int src_left, int src_top,
			  int alpha, int bt)
{
	GLfloat pos[24];
	struct texture *tex, *rule;
	float hw, hh, tw, th;

	/* struct textureを取得する */
	tex = get_texture_object(src_image);
	assert(tex != NULL);
	if (rule_image != NULL) {
		rule = get_texture_object(rule_image);
		assert(rule != NULL);
	} else {
		rule = NULL;
	}

	if (bt == BLEND_NONE)
		alpha = 255;

	/* ウィンドウサイズの半分を求める */
	hw = (float)conf_window_width / 2.0f;
	hh = (float)conf_window_height / 2.0f;

	/* テキスチャサイズを求める */
	tw = (float)get_image_width(src_image);
	th = (float)get_image_height(src_image);

	/* 左上 */
	pos[0] = ((float)dst_left - hw) / hw;
	pos[1] = -((float)dst_top - hh) / hh;
	pos[2] = 0.0f;
	pos[3] = (float)src_left / tw;
	pos[4] = (float)src_top / th;
	pos[5] = (float)alpha / 255.0f;

	/* 右上 */
	pos[6] = ((float)dst_left + (float)width - hw) / hw;
	pos[7] = -((float)dst_top - hh) / hh;
	pos[8] = 0.0f;
	pos[9] = (float)(src_left + width) / tw;
	pos[10] = (float)(src_top) / th;
	pos[11] = (float)alpha / 255.0f;

	/* 左下 */
	pos[12] = ((float)dst_left - hw) / hw;
	pos[13] = -((float)dst_top + (float)height - hh) / hh;
	pos[14] = 0.0f;
	pos[15] = (float)src_left / tw;
	pos[16] = (float)(src_top + height) / th;
	pos[17] = (float)alpha / 255.0f;

	/* 右下 */
	pos[18] = ((float)dst_left + (float)width - hw) / hw;
	pos[19] = -((float)dst_top + (float)height - hh) / hh;
	pos[20] = 0.0f;
	pos[21] = (float)(src_left + width) / tw;
	pos[22] = (float)(src_top + height) / th;
	pos[23] = (float)alpha / 255.0f;

	/* シェーダを設定して頂点バッファに書き込む */
	if (rule_image == NULL) {
		if (!is_dim) {
			/* 通常のアルファブレンド */
			glUseProgram(program_normal);
			glBindVertexArray(vao_normal);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_normal);
		} else {
			/* DIMシェーダ */
			glUseProgram(program_dim);
			glBindVertexArray(vao_dim);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_dim);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_dim);
		}
	} else if (!is_melt) {
		/* ルールシェーダ */
		glUseProgram(program_rule);
		glBindVertexArray(vao_rule);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_rule);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_rule);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		/* メルトシェーダ */
		glUseProgram(program_melt);
		glBindVertexArray(vao_melt);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_melt);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_melt);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	/* テクスチャを選択する */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	if (rule_image != NULL) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, rule->id);
	}

	if (bt == BLEND_NONE) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
	} else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	/* 図形を描画する */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
}

#ifdef WIN
/*
 * 全画面表示のときのスクリーンオフセットを指定する
 */
void opengl_set_screen(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
}
#endif
