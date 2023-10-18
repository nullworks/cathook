/*
 * imrenderer.hpp
 *
 *  Created on: Mar 09, 2019
 *      Author: UNKN0WN
 */

#pragma once

/*
 *  THIS IS MOSTLY JUST A WRAPPER FOR IMGUI!
 *  if you want to draw any imgui windows with this, do it between renderStart and renderEnd
 *  YOU CANNOT DO IT IN PAINT/PAINTTRAVERSE, buffers do not support imgui windows, only raw draw commands,
 *  so only draw in SwapWindow
 */

#include <string>
#include "colors.hpp"

class SDL_Window;
class ImFontAtlas;
class ImFont;

namespace im_renderer
{
struct font
{
    font(std::string path, int fontsize, bool outline = false);
    std::string path;
    int size;
    int new_size;
    bool outline       = false;
    bool needs_rebuild = true;
    operator ImFont *();
    void stringSize(std::string string, float *x, float *y);
    void changeSize(int new_font_size);
    void rebuild();
    ImFontAtlas *font_atlas{ nullptr };
    std::unordered_map<int, ImFont *> size_map;
};

class Texture
{
public:
    explicit Texture(std::string path);
    Texture(unsigned int id, int h, int w);
    ~Texture();
    void load();
    unsigned int get();
    int getWidth()
    {
        return width;
    }
    int getHeight()
    {
        return height;
    }

private:
    unsigned char *data{ nullptr };
    unsigned int texture_id{ 0 };
    std::string path{ "" };
    int height{ 0 };
    int width{ 0 };
};

void init();
void bufferBegin();
void renderStart();
void renderEnd();

namespace draw
{
void line(float x1, float y1, float x2, float y2, rgba_t color, float thickness);
void string(int x, int y, rgba_t color, const char *text, font &font);
void rectangle(float x, float y, float w, float h, rgba_t color);
void triangle(float x, float y, float x2, float y2, float x3, float y3, rgba_t color);
void rectangleOutlined(float x, float y, float w, float h, rgba_t color, float thickness);
void rectangleTextured(float x, float y, float w, float h, rgba_t color, Texture &texture, float tx, float ty, float tw, float th, float angle);
void circle(float x, float y, float radius, rgba_t color, float thickness, int steps);
} // namespace draw

} // namespace im_renderer
