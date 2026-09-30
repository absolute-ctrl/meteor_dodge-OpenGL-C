#include "renderer.h"
#include "config.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <stddef.h>
#include <stdio.h>

static const char *VERTEX_SRC =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aUV;\n"
    "layout(location = 2) in vec4 aColor;\n"
    "uniform vec2 uScreen;\n"
    "out vec2 vUV;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    vec2 ndc = aPos / uScreen * 2.0 - 1.0;\n"
    "    gl_Position = vec4(ndc, 0.0, 1.0);\n"
    "    vUV = aUV;\n"
    "    vColor = aColor;\n"
    "}\n";

static const char *FRAGMENT_SRC =
    "#version 330 core\n"
    "in vec2 vUV;\n"
    "in vec4 vColor;\n"
    "uniform sampler2D uTex;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = texture(uTex, vUV) * vColor;\n"
    "}\n";

typedef struct {
    float x, y;
    float u, v;
    float r, g, b, a;
} Vertex;

#define MAX_VERTICES 12000

static Vertex       batch[MAX_VERTICES];
static int          batch_count;
static unsigned int current_tex; 

static unsigned int program, vao, vbo;
static Texture      white;
static float        cr = 1, cg = 1, cb = 1, ca = 1;

static unsigned int compile_shader(unsigned int type, const char *src)
{
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, sizeof log, NULL, log);
        fprintf(stderr, "Erro ao compilar shader:\n%s\n", log);
    }
    return s;
}

static void flush(void)
{
    if (batch_count == 0)
        return;

    glBindTexture(GL_TEXTURE_2D, current_tex);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)(batch_count * sizeof(Vertex)), batch);
    glDrawArrays(GL_TRIANGLES, 0, batch_count);
    batch_count = 0;
}

static void reserve(int n, unsigned int tex)
{
    if (tex != current_tex || batch_count + n > MAX_VERTICES) {
        flush();
        current_tex = tex;
    }
}

static void push(float x, float y, float u, float v)
{
    Vertex *vt = &batch[batch_count++];
    vt->x = x;  vt->y = y;
    vt->u = u;  vt->v = v;
    vt->r = cr; vt->g = cg; vt->b = cb; vt->a = ca;
}

int renderer_init(void)
{
    unsigned int vs = compile_shader(GL_VERTEX_SHADER, VERTEX_SRC);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SRC);

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    int ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(program, sizeof log, NULL, log);
        fprintf(stderr, "Erro ao ligar o programa de shader:\n%s\n", log);
        return 0;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof batch, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, x));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, u));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, r));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    unsigned char px[4] = {255, 255, 255, 255};
    glGenTextures(1, &white.id);
    glBindTexture(GL_TEXTURE_2D, white.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    white.w = white.h = 1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "uTex"), 0);
    glUniform2f(glGetUniformLocation(program, "uScreen"),
                (float)WIDTH, (float)HEIGHT);
    return 1;
}

void renderer_shutdown(void)
{
    texture_free(&white);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
}

void renderer_begin(int framebuffer_w, int framebuffer_h,
                    float r, float g, float b)
{
    glViewport(0, 0, framebuffer_w, framebuffer_h);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    batch_count = 0;
    current_tex = white.id;
}

void renderer_end(void)
{
    flush();
}

void renderer_color(float r, float g, float b, float a)
{
    cr = r; cg = g; cb = b; ca = a;
}

void renderer_triangle(float x1, float y1, float x2, float y2,
                       float x3, float y3)
{
    reserve(3, white.id);
    push(x1, y1, 0, 0);
    push(x2, y2, 0, 0);
    push(x3, y3, 0, 0);
}

void renderer_rect(float x, float y, float w, float h)
{
    renderer_triangle(x, y, x + w, y, x + w, y + h);
    renderer_triangle(x, y, x + w, y + h, x, y + h);
}

void renderer_polygon(const float *xy, int n)
{
    for (int i = 1; i + 1 < n; i++)
        renderer_triangle(xy[0], xy[1],
                          xy[2 * i], xy[2 * i + 1],
                          xy[2 * i + 2], xy[2 * i + 3]);
}

void renderer_sprite(const Texture *tex, float x, float y, float w, float h)
{
    if (!tex || !tex->id)
        return;

    reserve(6, tex->id);
    push(x,     y,     0, 1);
    push(x + w, y,     1, 1);
    push(x + w, y + h, 1, 0);
    push(x,     y,     0, 1);
    push(x + w, y + h, 1, 0);
    push(x,     y + h, 0, 0);
}

int texture_load(Texture *tex, const char *path)
{
    int n;
    unsigned char *data = stbi_load(path, &tex->w, &tex->h, &n, 4);
    if (!data) {
        fprintf(stderr, "Nao consegui carregar '%s': %s\n",
                path, stbi_failure_reason());
        tex->id = 0;
        return 0;
    }

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
                 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    return 1;
}

void texture_free(Texture *tex)
{
    if (tex->id)
        glDeleteTextures(1, &tex->id);
    tex->id = 0;
}
