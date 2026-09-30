#ifndef RENDERER_H
#define RENDERER_H

typedef struct {
    unsigned int id;
    int w, h;       
} Texture;

int  renderer_init(void);
void renderer_shutdown(void);


void renderer_begin(int framebuffer_w, int framebuffer_h,
                    float r, float g, float b);
void renderer_end(void);

void renderer_color(float r, float g, float b, float a);

void renderer_triangle(float x1, float y1, float x2, float y2,
                       float x3, float y3);
void renderer_rect(float x, float y, float w, float h);


void renderer_polygon(const float *xy, int n);


void renderer_sprite(const Texture *tex, float x, float y, float w, float h);


int  texture_load(Texture *tex, const char *path);
void texture_free(Texture *tex);

#endif
