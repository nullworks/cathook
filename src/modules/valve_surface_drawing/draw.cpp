
/*
 *
 *	This is the draw handler for valve surface module.
 *
 *
 *
 */
#include "../../util/stringhelpers.hpp"

#include "draw.hpp"

//
namespace modules { namespace valvesurface {
	
// Var for surface to hold
vgui::ISurface* g_ISurface = nullptr;
	
void Line(int x, int y, int w, int h, rgba_t color) {
	int tmp = convert(color);
	g_ISurface->DrawSetColor(*reinterpret_cast<Color*>(&tmp));
	g_ISurface->DrawLine(x, y, x + w, y + h);
}

void Rect(int x, int y, int w, int h, rgba_t color) {
	int tmp = convert(color);
	g_ISurface->DrawSetColor(*reinterpret_cast<Color*>(&tmp));
	g_ISurface->DrawOutlinedRect(x, y, x + w, y + h);
}
	
void RectFilled(int x, int y, int w, int h, rgba_t color) {
	int tmp = convert(color);
	g_ISurface->DrawSetColor(*reinterpret_cast<Color*>(&tmp));
	g_ISurface->DrawFilledRect(x, y, x + w, y + h);
}
	
	
// Font system	
unsigned long opensans[32];// On int, generate these. 32 are for all the sizes as remaking them is kinda intensive, it at least i think it is.
unsigned long verasans[32];// I think the 32 per font is really big on memory... Welp, I want to die!
unsigned long unispace[32];
unsigned long tf2build[32];
	
static bool font_init = false;
void FontInit()	{
	// One loop per font
	for (int i = 0; i <= 31; i++) {
		opensans[i] = g_ISurface->CreateFont();
		g_ISurface->SetFontGlyphSet(opensans[i], "Open Sans", i + 1, 0, 0, 0, 0);
	}
	for (int i = 0; i <= 31; i++) {
		opensans[i] = g_ISurface->CreateFont();
		g_ISurface->SetFontGlyphSet(verasans[i], "Bitstream Vera Sans Mono", i + 1, 0, 0, 0, 0);
	}
	for (int i = 0; i <= 31; i++) {
		opensans[i] = g_ISurface->CreateFont();
		g_ISurface->SetFontGlyphSet(opensans[i], "Unispace", i + 1, 0, 0, 0, 0);
	}
	for (int i = 0; i <= 31; i++) {
		opensans[i] = g_ISurface->CreateFont();
		g_ISurface->SetFontGlyphSet(opensans[i], "TF2 Build", i + 1, 0, 0, 0, 0);
	}
	font_init = true;
}

// Retrieves one of the fonts generated and saved into
unsigned long GetSurfaceFont(int font, int size) {
	if (!font_init) FontInit(); // Init fonts if we havent
	while (size > 31) {size--;} // Clamping as we dont have that many font sizes
	switch(font) {
	case 0: 	
		return 0;
	case 1:	
		return opensans[size];
	case 2:
		return verasans[size];
	case 3:
		return unispace[size];
	case 4:
		return tf2build[size];
	}
	return 0;
}
	
// Get string height and length in pixels
void StringLength(const char* string, EFont font, int size, int& length, int& height){
	wchar_t buf[512];
	memset(buf, 0, sizeof(wchar_t) * 512);
	mbstowcs(buf, string, strlen(string));
	g_ISurface->GetTextSize(GetSurfaceFont(font, size), buf, length, height);	
}
	
// String drawing system
// Main string draw
void String3(const char* text, int x, int y, EFont font, int size, rgba_t color) {
	int tmp = convert(color);
	
	g_ISurface->DrawSetTextPos(x, y);
	g_ISurface->DrawSetTextColor(*reinterpret_cast<Color*>(&tmp));
	g_ISurface->DrawSetTextFont(GetSurfaceFont(font, size)); // I need to do something about this!!!
	// Conversion from char
	wchar_t string[512];
	memset(string, 0, sizeof(wchar_t) * 512);
	mbstowcs(string, text, 511);
	g_ISurface->DrawUnicodeString(string);
}
// Shadow processor
void String2(const char* text, int x, int y, EFont font, int size, rgba_t color) {
	rgba_t shadow = rgba_t(0, 0, 0, 40);
	
	String3(text, x, y - 1, font, size, shadow);
	String3(text, x - 1, y, font, size, shadow);
	String3(text, x - 1, y - 1, font, size, shadow);
	
	String3(text, x, y, font, size, color);	
	
}
// String input, fixes newlines
void String1(const char* text, int x, int y, EFont font, int size, rgba_t color) {
	
	// Check if newlined
	bool newlined = false;
	size_t len = strlen(text);
	for (int i = 0; i < len; i++) {
		if (text[i] == '\n') {
			newlined = true; break;
		}
	}
	// Correct for newlines?
	if (newlined) {
		char ch[512];
		memset(ch, 0, sizeof(char) * 512);
		int w, h;
		StringLength("W", font, size, w, h);
		strncpy(ch, text, 511);
		int s = 0;
		int n = 0;
		for (int i = 0; i < len; i++) {
			if (ch[i] == '\n') {
				ch[i] = 0;
				String2(&ch[0] + s, x, y + n * (h), font, size, color);
				n++;
				s = i + 1;
			}
		}
		String2(&ch[0] + s, x, y + n * (h), font, size, color);
	} else {
		String2(text, x, y, font, size, color);
	}
}
	


	
}}