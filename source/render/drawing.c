#include "drawing.h"
#include "text.h"
#include "save.h"
#include "lang.h"
#include "core.h"

#include "importer.h"
#include "rsx_renderer.h"

#define FADE_NONE 0
#define FADE_IN   1
#define FADE_OUT  2

static int fade_mode = FADE_NONE;
static float fade_alpha = 0.0f;

static GameScreen *fade_variable = NULL;
static GameScreen fade_value;

GameScreen currentScreen = LOGO;

void DrawFade(void)
{
    if(fade_alpha <= 0.0f) return;

    u32 alpha = (u32)fade_alpha;
    u32 color = alpha;
    color = 0x00000000 | alpha;
	
	DrawBackground2D(color, -32);
}

u32 color_from_value(int value)
{
    if (value < 0)  value = 0;
    if (value > 10) value = 10;
    int r, g, b;

    if (value >= 5)
    {
        float t = (float)(value - 5) / 5.0f;
        r = (int)(255.0f - (127.0f * t));
        g = 255;
        b = (int)(64.0f * t);
    }
    else
    {
        float t = (float)value / 5.0f;
        r = 255;
        g = (int)(255.0f * t);
        b = 0;
    }

    return (r << 24) | (g << 16) | (b << 8) | 0xff;
}

void DrawButton(int x, int y, int button, int z)
{
	u32 color = 0xffffffff;
	DrawSprite(0, button, x, y+4, 32, 32, 20, 20, z, color);
}

void drawScene()
{
	u32 color = 0xffffffff;

    static int count_frames = 0;
    // static int anim = 0;
    // static int anim1 = 0;
    // static int anim2 = 0;
    // static int anim3 = 0;
	// static int animc = 0;
	SetFontSize(28, 32);
	
    SetFontColor(0xffffffff, 0x00000000);
	
	if(game_loaded)
	{
		count_frames++;
		
		// anim = (count_frames / 30) % 2;
		// anim1 = (count_frames / 15) % 2;
		// anim2 = (count_frames / 15) % 3;
		// anim3 = (count_frames / 15) % 4;
		
		switch(currentScreen)
		{
			case LOGO:
			{
				SetFontSize(28, 32);
				DrawSprite(1, 0, 0, 0, 640, 480, 260, 260, 0, color);
				DrawFString(0, 4, 4, 0xffffffff, 0, 32, "AcidNT3.1 Productions\nTEMP LOGO\n\nPRESS ANY KEY TO SKIP");
				break;
			}
			case TITLE:
			{
				DrawSprite(2, 0, 0, 0, 640, 480, 640, 480, 1, color);
				DrawSprite(3, 0, 0, 85, 613, 403, 613, 403, 0, color);
				DrawFString(0, 4, 4, 0xffffffff, 0, 32, "%s (%s)", global_name, global_version);
				break;
			}
			case TEST:
			{
				DrawSprite(5, 0, 0, 0, 640, 480, 640, 480, 0, color);
				DrawFString(0, 4, 64, 0xffffffff, 0, 28, "Test of being not dead: %d", count_frames);
				DrawFString(0, 40, 128, 0xffffffff, 0, 28, "Play main theme");
				DrawFString(0, 40, 160, 0xffffffff, 0, 28, "Play Action theme");
				DrawFString(0, 40, 192, 0xffffffff, 0, 28, "Play Strategy theme");
				DrawFString(0, 40, 224, 0xffffffff, 0, 28, "Play Puzzle theme");
				DrawFString(0, 40, 256, 0xffffffff, 0, 28, "Play Game over theme");
				DrawFString(0, 40, 288, 0xffffffff, 0, 28, "Play Action complete theme");
				DrawFString(0, 4, 438, 0xffffffff, 0, 28, "(Loaded from .it file)");
				DrawButton(4, 128, PAD_START, 0);
				DrawButton(4, 160, PAD_CROSS, 0);
				DrawButton(4, 192, PAD_CIRCLE, 0);
				DrawButton(4, 224, PAD_SQUARE, 0);
				DrawButton(4, 256, PAD_TRIANGLE, 0);
				DrawButton(4, 288, PAD_SELECT, 0);
				break;
			}
			default: break;
		}
	}
	else
	{
		SetFontSize(24, 28);
		if(game_init)
		{
			DrawFString(0, 4, 4, 0xffffffff, 0, 32, "Warning! This game uses auto-save feature.\nWhen you see the \"Saving...\" text, it saves your data.\nPlease, do NOT turn off your PlayStation 3 system while\nthe \"Saving...\" text is on-screen.");
			DrawButton(4, 348, PAD_CROSS, 0);
			DrawFString(0, 40, 348, 0xffffffff, 0, 32, "I understand.");
		}
		else DrawFString(0, 0, 0, 0xffffffff, 0, 28, "Loading data...");
	}
	
	SetFontSize(28, 32);
	if(save_running) DrawTextAligned(0, 480, 640, 452, 2, 0xffffffff, 0, 14, 32, "Saving data...");
	
	DrawFade();
}

void MyDrawFormatString(float x, float y, const char* format, ...)
{
    char utf8_buffer[1024];
    char cp1251_buffer[1024];
	
    va_list args;
    va_start(args, format);
    vsnprintf(utf8_buffer, sizeof(utf8_buffer), format, args);
    va_end(args);
	
    utf8_to_cp1251(utf8_buffer, cp1251_buffer, sizeof(cp1251_buffer));
    DrawString(x, y, cp1251_buffer);
}

void DrawFString(int wrap, float x, float y, u32 textColor, int BW, int size, const char *fmt, ...)
{
    char buffer[1024];
	u32 shadowColor = 0x000000ff;
	
	if(BW) shadowColor = 0xffffffff;
	
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    SetFontColor(shadowColor, 0x00000000);
	
	if(wrap==0) MyDrawFormatString(x+2, y+2, buffer);
	else DrawWrappedText(x+2, y+2, size, wrap, buffer);
	
    SetFontColor(textColor, 0x00000000);
	
	if(wrap==0) MyDrawFormatString(x, y, buffer);
	else DrawWrappedText(x, y, size, wrap, buffer);
}

int GetFontWidth(const char *text, int fz)
{
    return (int)(strlen(text) * fz * 0.55f);
}

void DrawTextAligned(int wrap, float x1, float x2, float y, int align, u32 tc, int bw, int fsize, int size, const char *text, ...)
{
    char bufferz[1024];
    va_list args;
    va_start(args, text);
    vsnprintf(bufferz, sizeof(bufferz), text, args);
    va_end(args);
	
    int width = GetFontWidth(bufferz, fsize);
    float x;

    if(align == 0) x = x1;
    else if(align == 1) x = x1 + ((x2 - x1) - width) / 2.0f;
    else x = x2 - width;

    //DrawFormatString(x, y, "%s", text);
	DrawFString(wrap, x, y, tc, bw, size, bufferz);
}

void FadeInOut(GameScreen *variable, GameScreen value)
{
    fade_variable = variable;
    fade_value = value;
	
	if(*fade_variable == value) return;

    fade_alpha = 0.0f;
    fade_mode = FADE_OUT;
}

void UpdateFade(float dt)
{
    if(fade_mode == FADE_OUT)
    {
        fade_alpha += 255.0f * dt / 0.5f;

        if(fade_alpha >= 255.0f)
        {
            fade_alpha = 255.0f;
            fade_mode = FADE_IN;

            if(fade_variable)
            {
                *fade_variable = fade_value;
                fade_variable = NULL;
            }
        }
    }
    else if(fade_mode == FADE_IN)
    {
        fade_alpha -= 255.0f * dt / 0.5f;

        if(fade_alpha <= 0.0f)
        {
            fade_alpha = 0.0f;
            fade_mode = FADE_NONE;
        }
    }
}