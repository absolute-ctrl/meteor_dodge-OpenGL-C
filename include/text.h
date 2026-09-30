#ifndef TEXT_H
#define TEXT_H

float text_width(const char *str, float s);
void  text_draw(const char *str, float x, float y, float s,
                float r, float g, float b);
void  text_draw_centered(const char *str, float y, float s,
                         float r, float g, float b);

#endif
