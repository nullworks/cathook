#include "common.hpp"

namespace fonts
{
unsigned long MENU                   = 0;
unsigned long MENU_BIG               = 0;
const std::vector<std::string> fonts = { XORSTR("Tahoma Bold"),  XORSTR("Tahoma"),
                                         XORSTR("TF2 Build"),    XORSTR("Verdana"),
                                         XORSTR("Verdana Bold"), XORSTR("Arial"),
                                         XORSTR("Courier New"),  XORSTR("Ubuntu Mono Bold") };
CatEnum family_enum(fonts);
}

int colorsint::FromHSL(float h, float s, float v)
{
    double hh, p, q, t, ff;
    long i;

    if (s <= 0.0)
    { // < is bogus, just shuts up warnings
        return colorsint::Create(v * 255, v * 255, v * 255, 255);
    }
    hh = h;
    if (hh >= 360.0)
        hh = 0.0;
    hh /= 60.0;
    i  = (long) hh;
    ff = hh - i;
    p  = v * (1.0 - s);
    q  = v * (1.0 - (s * ff));
    t  = v * (1.0 - (s * (1.0 - ff)));

    switch (i)
    {
    case 0:
        return colorsint::Create(v * 255, t * 255, p * 255, 255);
    case 1:
        return colorsint::Create(q * 255, v * 255, p * 255, 255);
    case 2:
        return colorsint::Create(p * 255, v * 255, t * 255, 255);
    case 3:
        return colorsint::Create(p * 255, q * 255, v * 255, 255);
        break;
    case 4:
        return colorsint::Create(t * 255, p * 255, v * 255, 255);
    case 5:
    default:
        return colorsint::Create(v * 255, p * 255, q * 255, 255);
    }
}

int colorsint::RainbowCurrent()
{
    return colorsint::FromHSL(fabs(sin(g_GlobalVars->curtime / 2.0f)) * 360.0f,
                              0.85f, 0.9f);
}

void draw::DrawRect(int x, int y, int w, int h, int color)
{
    g_ISurface->DrawSetColor(*reinterpret_cast<Color *>(&color));
    g_ISurface->DrawFilledRect(x, y, x + w, y + h);
}

void draw::DrawLine(int x, int y, int dx, int dy, int color)
{
    g_ISurface->DrawSetColor(*reinterpret_cast<Color *>(&color));
    g_ISurface->DrawLine(x, y, x + dx, y + dy);
}

void draw::OutlineRect(int x, int y, int w, int h, int color)
{
    g_ISurface->DrawSetColor(*reinterpret_cast<Color *>(&color));
    g_ISurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void draw::GetStringLength(unsigned long font, const char *string, int &length,
                           int &height)
{
    wchar_t buf[512];
    memset(buf, 0, sizeof(wchar_t) * 512);
    mbstowcs(buf, string, strlen(string));
    g_ISurface->GetTextSize(font, buf, length, height);
}

void draw::String(unsigned long font, int x, int y, int color, int shadow,
                  const char *text)
{
    bool newlined;
    int w, h, s, n;
    char ch[512];
    wchar_t string[512];
    size_t len;

    newlined = false;
    len      = strlen(text);
    for (int i = 0; i < len; i++)
    {
        if (text[i] == '\n')
        {
            newlined = true;
            break;
        }
    }
    if (newlined)
    {
        memset(ch, 0, sizeof(char) * 512);
        GetStringLength(font, XORSTR("W"), w, h);
        strncpy(ch, text, 511);
        s = 0;
        n = 0;
        for (int i = 0; i < len; i++)
        {
            if (ch[i] == '\n')
            {
                ch[i] = 0;
                draw::String(font, x, y + n * (h), color, shadow, &ch[0] + s);
                n++;
                s = i + 1;
            }
        }
        draw::String(font, x, y + n * (h), color, shadow, &ch[0] + s);
    }
    else
    {
        memset(string, 0, sizeof(wchar_t) * 512);
        mbstowcs(string, text, 511);
        draw::WString(font, x, y, color, shadow, string);
    }
}

void draw::String(unsigned long font, int x, int y, int color, int shadow,
                  std::string text)
{
    draw::String(font, x, y, color, shadow, text.c_str());
}
CatVar fast_outline(CV_SWITCH, XORSTR("fast_outline"), XORSTR("0"), XORSTR("Fast font outline"),
                    XORSTR("Use only single repaint to increase performance"));
void draw::WString(unsigned long font, int x, int y, int color, int shadow,
                   const wchar_t *text)
{
    unsigned char alpha;
    int black_t;

    if (shadow)
    {
        alpha   = (color >> 24);
        black_t = ((alpha == 255) ? colorsint::black
                                  : colorsint::Create(0, 0, 0, alpha / shadow));
        if (shadow > 0)
        {
            draw::WString(font, x + 1, y + 1, black_t, false, text);
        }
        if (shadow > 1 && !fast_outline)
        {
            draw::WString(font, x - 1, y + 1, black_t, false, text);
            draw::WString(font, x - 1, y - 1, black_t, false, text);
            draw::WString(font, x + 1, y - 1, black_t, false, text);
            draw::WString(font, x + 1, y, black_t, false, text);
            draw::WString(font, x, y + 1, black_t, false, text);
            draw::WString(font, x, y - 1, black_t, false, text);
            draw::WString(font, x - 1, y, black_t, false, text);
        }
    }
    g_ISurface->DrawSetTextPos(x, y);
    g_ISurface->DrawSetTextColor(*reinterpret_cast<Color *>(&color));
    g_ISurface->DrawSetTextFont(font);
    g_ISurface->DrawUnicodeString(text);
}

void draw::FString(unsigned long font, int x, int y, int color, int shadow,
                   const char *text, ...)
{
    va_list list;
    char buffer[2048] = { '\0' };
    va_start(list, text);
    vsprintf(buffer, text, list);
    va_end(list);
    draw::String(font, x, y, color, shadow, buffer);
}

std::pair<int, int> draw::GetStringLength(unsigned long font,
                                          std::string string)
{
    int l, h;
    draw::GetStringLength(font, (char *) string.c_str(), l, h);
    return std::make_pair(l, h);
}
