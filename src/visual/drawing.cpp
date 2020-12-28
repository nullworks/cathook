/*
 * drawing.cpp
 *
 *  Created on: Mar 10, 2019
 *      Author: Lighty
 */
/*
 * drawing.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */
#include "common.hpp"
#if ENABLE_IMGUI_DRAWING
#include "imgui/imrenderer.hpp"
#elif ENABLE_GLEZ_DRAWING
#include <glez/draw.hpp>
#include <glez/glez.hpp>
#elif ENABLE_ENGINE_DRAWING
#include "picopng.hpp"
#endif
#include "menu/GuiInterface.hpp"
#include <SDL2/SDL_video.h>
#include <SDLHooks.hpp>
#include "soundcache.hpp"

// String -> Wstring
#include <codecvt>
#include <locale>

#if EXTERNAL_DRAWING
#include "xoverlay.h"
#endif

std::array<std::string, 32> side_strings;
std::array<std::string, 32> center_strings;
std::array<rgba_t, 32> side_strings_colors{ colors::empty };
std::array<rgba_t, 32> center_strings_colors{ colors::empty };
size_t side_strings_count{ 0 };
size_t center_strings_count{ 0 };
static settings::Int esp_font_size{ "visual.font_size.esp", "13" };
static settings::Int center_font_size{ "visual.font_size.center_size", "14" };

void InitStrings()
{
    ResetStrings();
}

void ResetStrings()
{
    side_strings_count   = 0;
    center_strings_count = 0;
}

void AddSideString(const std::string &string, const rgba_t &color)
{
    side_strings[side_strings_count]        = string;
    side_strings_colors[side_strings_count] = color;
    ++side_strings_count;
}

void DrawStrings()
{
    int y{ 8 };

    for (size_t i = 0; i < side_strings_count; ++i)
    {
        draw::String(8, y, side_strings_colors[i], side_strings[i].c_str(), *fonts::center_screen);
        y += fonts::center_screen->size + 1;
    }
    y = draw::height / 2;
    for (size_t i = 0; i < center_strings_count; ++i)
    {
        float sx, sy;
        fonts::menu->stringSize(center_strings[i], &sx, &sy);
        draw::String((draw::width - sx) / 2, y, center_strings_colors[i], center_strings[i].c_str(), *fonts::center_screen);
        y += fonts::center_screen->size + 1;
    }
}

void AddCenterString(const std::string &string, const rgba_t &color)
{
    center_strings[center_strings_count]        = string;
    center_strings_colors[center_strings_count] = color;
    ++center_strings_count;
}

std::string ShrinkString(std::string data, int max_x, fonts::font &font)
{
    int padding = 5;
    float x, y;
    font.stringSize("..", &x, &y);
    int dotdot_with = x;

    if (padding + dotdot_with > max_x)
        return std::string();

    if (!data.empty())
    {
        font.stringSize(data, &x, &y);
        if (x + padding > max_x)
        {
            while (true)
            {
                font.stringSize(data, &x, &y);
                if (x + padding + dotdot_with > max_x)
                {
                    data = data.substr(0, data.size() - 1);
                }
                else
                    break;
            }
            data.append("..");
            return data;
        }
    }
    return data;
}

int draw::width  = 0;
int draw::height = 0;
float draw::fov  = 90.0f;

namespace fonts
{
#if ENABLE_ENGINE_DRAWING
font::operator unsigned int()
{
    if (!size_map[size])
        Init();
    return size_map[size];
}
void font::Init()
{
    size += 3;
    static std::string filename;
    filename.append("ab");
    size_map[size] = g_ISurface->CreateFont();
    auto flag      = vgui::ISurface::FONTFLAG_ANTIALIAS;
    g_ISurface->SetFontGlyphSet(size_map[size], filename.c_str(), size, 500, 0, 0, flag);
    g_ISurface->AddCustomFontFile(filename.c_str(), path.c_str());
}
void font::stringSize(std::string string, float *x, float *y)
{
    if (!size_map[size])
        Init();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
    std::wstring ws = converter.from_bytes(string.c_str());
    int w, h;
    g_ISurface->GetTextSize(size_map[size], ws.c_str(), w, h);
    if (x)
        *x = w;
    if (y)
        *y = h;
}
void font::changeSize(int new_font_size)
{
    size = new_font_size;
    if (!size_map[size])
        Init();
}
#endif
std::unique_ptr<font> menu{ nullptr };
std::unique_ptr<font> esp{ nullptr };
std::unique_ptr<font> center_screen{ nullptr };
} // namespace fonts

static InitRoutine font_size([]() {
    esp_font_size.installChangeCallback([](settings::VariableBase<int> &var, int after) {
        if (after > 0 && after < 100)
        {
#if ENABLE_GLEZ_DRAWING
            fonts::esp->unload();
            fonts::esp.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), after));
#else
            fonts::esp->changeSize(after);
#endif
        }
    });
    center_font_size.installChangeCallback([](settings::VariableBase<int> &var, int after) {
        if (after > 0 && after < 100)
        {
#if ENABLE_GLEZ_DRAWING
            fonts::center_screen->unload();
            fonts::center_screen.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), after));
#else
            fonts::center_screen->changeSize(after);
#endif
        }
    });
});
namespace draw
{

unsigned int texture_white = 0;

void Initialize()
{
    if (!draw::width || !draw::height)
    {
        g_IEngine->GetScreenSize(draw::width, draw::height);
    }
#if ENABLE_GLEZ_DRAWING
    glez::preInit();
    fonts::menu.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 10));
    fonts::esp.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 10));
    fonts::center_screen.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 12));
#elif ENABLE_ENGINE_DRAWING
    fonts::menu.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 10, true));
    fonts::esp.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 10, true));
    fonts::center_screen.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 12, true));
#elif ENABLE_IMGUI_DRAWING
    fonts::menu.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 13, true));
    fonts::esp.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 13, true));
    fonts::center_screen.reset(new fonts::font(paths::getDataPath("/menu/Verdana.ttf"), 14, true));
#endif
#if ENABLE_ENGINE_DRAWING
    texture_white                = g_ISurface->CreateNewTextureID();
    unsigned char colorBuffer[4] = { 255, 255, 255, 255 };
    g_ISurface->DrawSetTextureRGBA(texture_white, colorBuffer, 1, 1, false, true);
#endif
}

void String(int x, int y, rgba_t rgba, const char *text, fonts::font &font)
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::draw::string(x, y, rgba, text, font);
#elif ENABLE_ENGINE_DRAWING
    rgba = rgba * 255.0f;
    // Fix text being too low
    y -= 2;
    // Outline magic
    if (font.outline)
    {
        // Save Space
        for (int i = -1; i < 2; i += 2)
        {
            // All needed for one draw

            // X Shift draw
            g_ISurface->DrawSetTextPos(x + i, y);
            g_ISurface->DrawSetTextFont(font);
            g_ISurface->DrawSetTextColor(0, 0, 0, rgba.a);

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
            std::wstring ws = converter.from_bytes(text);
            g_ISurface->DrawPrintText(ws.c_str(), ws.size() + 1);

            // Y Shift draw
            g_ISurface->DrawSetTextPos(x, y + i);
            g_ISurface->DrawSetTextFont(font);
            g_ISurface->DrawSetTextColor(0, 0, 0, rgba.a);

            ws = converter.from_bytes(text);
            g_ISurface->DrawPrintText(ws.c_str(), ws.size() + 1);
        }
    }
    g_ISurface->DrawSetTextPos(x, y);
    g_ISurface->DrawSetTextFont(font);
    g_ISurface->DrawSetTextColor(rgba.r, rgba.g, rgba.b, rgba.a);

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
    std::wstring ws = converter.from_bytes(text);

    g_ISurface->DrawPrintText(ws.c_str(), ws.size() + 1);
#else
    glez::draw::outlined_string(x, y, text, font, rgba, colors::black, nullptr, nullptr);
#endif
}

// x2_offset and y2_offset are an OFFSET, meaning you need to pass coordinate 2 - coordinate 1 for it to work, x2_offset is aded to x1
void Line(float x1, float y1, float x2_offset, float y2_offset, rgba_t color, float thickness)
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::draw::line(x1, y1, x2_offset, y2_offset, color, thickness);
#elif ENABLE_ENGINE_DRAWING
    color = color * 255.0f;
    g_ISurface->DrawSetColor(color.r, color.g, color.b, color.a);
    if (thickness > 1.0f)
    {
        g_ISurface->DrawSetTexture(texture_white);
        // Dirty
        x1 += 0.5f;
        y1 += 0.5f;

        float length = sqrtf(x2_offset * x2_offset + y2_offset * y2_offset);
        x2_offset *= (length - 1.0f) / length;
        y2_offset *= (length - 1.0f) / length;

        float nx = x2_offset;
        float ny = y2_offset;

        float ex = x1 + x2_offset;
        float ey = y1 + y2_offset;

        if (length <= 1.0f)
            return;

        nx /= length;
        ny /= length;

        float th = thickness;

        nx *= th * 0.5f;
        ny *= th * 0.5f;

        float px = ny;
        float py = -nx;

        vgui::Vertex_t vertices[4];

        vertices[2].m_Position = { float(x1) - nx + px, float(y1) - ny + py };
        vertices[1].m_Position = { float(x1) - nx - px, float(y1) - ny - py };
        vertices[3].m_Position = { ex + nx + px, ey + ny + py };
        vertices[0].m_Position = { ex + nx - px, ey + ny - py };

        g_ISurface->DrawTexturedPolygon(4, vertices);
    }
    else
    {
        g_ISurface->DrawLine(x1, y1, x1 + x2_offset, y1 + y2_offset);
    }
#elif ENABLE_GLEZ_DRAWING
    glez::draw::line(x1, y1, x2_offset, y2_offset, color, thickness);
#endif
}

void Rectangle(float x, float y, float w, float h, rgba_t color)
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::draw::rectangle(x, y, w, h, color);
#elif ENABLE_ENGINE_DRAWING
    color = color * 255.0f;
    g_ISurface->DrawSetTexture(texture_white);
    g_ISurface->DrawSetColor(color.r, color.g, color.b, color.a);

    vgui::Vertex_t vertices[4];
    vertices[0].m_Position = { x, y };
    vertices[1].m_Position = { x, y + h };
    vertices[2].m_Position = { x + w, y + h };
    vertices[3].m_Position = { x + w, y };

    g_ISurface->DrawTexturedPolygon(4, vertices);
#elif ENABLE_GLEZ_DRAWING
    glez::draw::rect(x, y, w, h, color);
#endif
}

void Triangle(float x, float y, float x2, float y2, float x3, float y3, rgba_t color)
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::draw::triangle(x, y, x2, y2, x3, y3, color);
#elif ENABLE_ENGINE_DRAWING
    color = color * 255.0f;
    g_ISurface->DrawSetTexture(texture_white);
    g_ISurface->DrawSetColor(color.r, color.g, color.b, color.a);

    vgui::Vertex_t vertices[3];
    vertices[0].m_Position = { x, y };
    vertices[1].m_Position = { x2, y2 };
    vertices[1].m_Position = { x3, y3 };

    g_ISurface->DrawTexturedPolygon(3, vertices);
#elif ENABLE_GLEZ_DRAWING
    glez::draw::triangle(x, y, x2, y2, x3, y3, color);
#endif
}

void Circle(float x, float y, float radius, rgba_t color, float thickness, int steps)
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::draw::circle(x, y, radius, color, thickness, steps);
#else
    float px = 0;
    float py = 0;
    for (int i = 0; i <= steps; i++)
    {
        float ang = 2 * float(M_PI) * (float(i) / steps);
        if (!i)
            ang = 2 * float(M_PI);
        if (i)
            draw::Line(px, py, x - px + radius * cos(ang), y - py + radius * sin(ang), color, thickness);
        px = x + radius * cos(ang);
        py = y + radius * sin(ang);
    }
#endif
}

void RectangleOutlined(float x, float y, float w, float h, rgba_t color, float thickness)
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::draw::rectangleOutlined(x, y, w, h, color, thickness);
#else
    Rectangle(x, y, w, 1, color);
    Rectangle(x, y, 1, h, color);
    Rectangle(x + w - 1, y, 1, h, color);
    Rectangle(x, y + h - 1, w, 1, color);
#endif
}

void RectangleTextured(float x, float y, float w, float h, rgba_t color, Texture &texture, float tx, float ty, float tw, float th, float angle)
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::draw::rectangleTextured(x, y, w, h, color, texture, tx, ty, tw, th, angle);
#elif ENABLE_ENGINE_DRAWING
    color = color * 255.0f;
    vgui::Vertex_t vertices[4];
    g_ISurface->DrawSetColor(color.r, color.g, color.b, color.a);
    g_ISurface->DrawSetTexture(texture.get());

    float tex_width = texture.getWidth();
    float tex_height = texture.getHeight();

    Vector2D scr_top_left = { x, y };
    Vector2D scr_top_right = { x + w, y };
    Vector2D scr_bottom_right = { x + w, y + h };
    Vector2D scr_botton_left = { x, y + w };

    if (angle != 0.0f)
    {
        float cx = x + float(w) / 2.0f;
        float cy = y + float(h) / 2.0f;

        auto f = [&](Vector2D &v) {
            v.x = cx + cosf(angle) * (v.x - cx) - sinf(angle) * (v.y - cy);
            v.y = cy + sinf(angle) * (v.x - cx) + cosf(angle) * (v.y - cy);
        };
        f(scr_top_left);
        f(scr_top_right);
        f(scr_bottom_right);
        f(scr_bottom_right);
    }

    Vector2D tex_top_left = { tx / tex_width, ty / tex_height };
    Vector2D tex_top_right = { (tx + tw) / tex_width, ty / tex_height };
    Vector2D tex_bottom_right = { (tx + tw) / tex_width, (ty + th) / tex_height };
    Vector2D tex_botton_left = { tx / tex_width, (ty + th) / tex_height };
    // logging::Info("%f,%f %f,%f", tex_top_left.x, tex_top_left.y, tex_top_right.x, tex_top_right.y);

    vertices[0].Init(scr_top_left, tex_top_left);
    vertices[1].Init(scr_top_right, tex_top_right);
    vertices[2].Init(scr_bottom_right, tex_bottom_right);
    vertices[3].Init(scr_botton_left, tex_botton_left);

    g_ISurface->DrawTexturedPolygon(4, vertices);
#elif ENABLE_GLEZ_DRAWING
    glez::draw::rect_textured(x, y, w, h, color, texture, tx, ty, tw, th, angle);
#endif
}

bool EntityCenterToScreen(CachedEntity *entity, Vector &out)
{
    Vector world, min, max;
    bool succ;

    if (CE_INVALID(entity))
        return false;
    RAW_ENT(entity)->GetRenderBounds(min, max);
    world = RAW_ENT(entity)->GetAbsOrigin();

    // Dormant
    if (RAW_ENT(entity)->IsDormant())
    {
        if (entity->m_vecDormantOrigin())
            world = *entity->m_vecDormantOrigin();
        else
            return false;
    }
    world.z += (min.z + max.z) / 2;
    succ = draw::WorldToScreen(world, out);
    return succ;
}

VMatrix wts{};

void UpdateWTS()
{
    CViewSetup gay;
    VMatrix _, __, ___;
    g_IBaseClient->GetPlayerView(gay);
    g_IVRenderView->GetMatricesForView(gay, &_, &__, &wts, &___);
}

bool WorldToScreen(const Vector &origin, Vector &screen)
{
    float w;
    screen.z  = 0;
    w         = wts[3][0] * origin[0] + wts[3][1] * origin[1] + wts[3][2] * origin[2] + wts[3][3];
    float odw = 1.0f / w;
    screen.x  = (draw::width / 2) + (0.5 * ((wts[0][0] * origin[0] + wts[0][1] * origin[1] + wts[0][2] * origin[2] + wts[0][3]) * odw) * draw::width + 0.5);
    screen.y  = (draw::height / 2) - (0.5 * ((wts[1][0] * origin[0] + wts[1][1] * origin[1] + wts[1][2] * origin[2] + wts[1][3]) * odw) * draw::height + 0.5);
    if (w > 0.001)
        return true;
    return false;
}
#if ENABLE_ENGINE_DRAWING
bool Texture::load()
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    std::streamsize size = 0;
    if (file.seekg(0, std::ios::end).good())
        size = file.tellg();
    if (file.seekg(0, std::ios::beg).good())
        size -= file.tellg();

    if (size < 1)
        return false;

    unsigned char *buffer = new unsigned char[(size_t) size + 1];
    file.read((char *) buffer, size);
    file.close();
    int error = decodePNG(data, m_width, m_height, buffer, size);

    // if there's an error, display it and return false to indicate failure
    if (error != 0)
    {
        logging::Info("Error loading texture, error code %i\n", error);
        return false;
    }
    texture_id = g_ISurface->CreateNewTextureID(true);
    // g_ISurface->DrawSetTextureRGBA(texture_id, data, m_width, m_height, false, false);
    g_ISurface->DrawSetTextureRGBAEx(texture_id, data, m_width, m_height, ImageFormat::IMAGE_FORMAT_RGBA8888);
    if (!g_ISurface->IsTextureIDValid(texture_id))
        return false;
    init = true;
    return true;
}

Texture::~Texture()
{
    g_ISurface->DeleteTextureByID(texture_id);
}

Texture::Texture(unsigned int id, unsigned int height, unsigned int width) : texture_id{ id }, m_width{ width }, m_height{ height }
{
    init = true;
}

unsigned int Texture::get()
{
    if (texture_id == 0)
    {
        if (!load())
            throw std::runtime_error("Couldn't init texture!");
    }
    return texture_id;
}
#endif

void InitGL()
{
    logging::Info("InitGL: %d, %d", draw::width, draw::height);
#if EXTERNAL_DRAWING
    int status = xoverlay_init();
    if (status < 0)
    {
        logging::Info("ERROR: could not initialize Xoverlay: %d", status);
    }
    else
    {
        logging::Info("Xoverlay initialized");
    }
    xoverlay_show();
    xoverlay_draw_begin();
#if ENABLE_GLEZ_DRAWING
    glez::init(xoverlay_library.width, xoverlay_library.height);
#elif ENABLE_IMGUI_DRAWING
    im_renderer::init();
#endif
    xoverlay_draw_end();
#else
#if ENABLE_IMGUI_DRAWING
    // glewInit();
    im_renderer::init();
#elif ENABLE_GLEZ_DRAWING || ENABLE_IMGUI_DRAWING
    glClearColor(1.0, 0.0, 0.0, 0.5);
    glewExperimental = GL_TRUE;
    glewInit();
    glez::init(draw::width, draw::height);
#endif
#endif

#if ENABLE_GUI
    gui::init();
#endif
}

void BeginGL()
{
#if ENABLE_IMGUI_DRAWING
#if EXTERNAL_DRAWING
    xoverlay_draw_begin();
#endif
    im_renderer::renderStart();
#elif ENABLE_GLEZ_DRAWING
    glColor3f(1, 1, 1);
#if EXTERNAL_DRAWING
    xoverlay_draw_begin();
    {
        PROF_SECTION(draw_begin__SDL_GL_MakeCurrent);
        // SDL_GL_MakeCurrent(sdl_hooks::window, context);
    }
#endif
    {
        glActiveTexture(GL_TEXTURE0);
        PROF_SECTION(draw_begin__glez_begin);
        glez::begin();
        glDisable(GL_FRAMEBUFFER_SRGB);
        PROF_SECTION(DRAWEX_draw_begin);
    }
#endif
}

void EndGL()
{
#if ENABLE_IMGUI_DRAWING
    im_renderer::renderEnd();
#if EXTERNAL_DRAWING
    xoverlay_draw_end();
    SDL_GL_MakeCurrent(sdl_hooks::window, nullptr);
#endif
#elif ENABLE_GLEZ_DRAWING
    PROF_SECTION(DRAWEX_draw_end);
    {
        PROF_SECTION(draw_end__glez_end);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glez::end();
    }
#if EXTERNAL_DRAWING
    xoverlay_draw_end();
    {
        PROF_SECTION(draw_end__SDL_GL_MakeCurrent);
        SDL_GL_MakeCurrent(sdl_hooks::window, nullptr);
    }
#endif
#endif
}
} // namespace draw
