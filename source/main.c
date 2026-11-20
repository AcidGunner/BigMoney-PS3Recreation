#include "rsx_renderer.h"
#include "rsxutil.h"
#define MAX_BUFFERS 2

#define GAME_DIR "/dev_hdd0/game/BIGMONEY0/USRDIR/"

#include <sysutil/sysutil.h>

#include "soundlib/spu_soundlib.h"
#include "soundlib/audioplayer.h"

#include "save.h"
#include "drawing.h"
#include "lang.h"
#include "text.h"
#include "importer.h"
#include "tracker.h"
#include "core.h"

static volatile int exit_requested = 0;
static volatile int xmb_open = 0;

static void sysutil_cb(u64 status, u64 param, void *usrdata)
{
    (void)param; (void)usrdata;
    switch (status)
    {
        case SYSUTIL_EXIT_GAME:  exit_requested = 1; break;
        case SYSUTIL_MENU_OPEN:  xmb_open = 1; break;
        case SYSUTIL_MENU_CLOSE: xmb_open = 0; break;
    }
}

bool old_control_left = false;
bool old_control_right = false;
bool old_control_up = false;
bool old_control_down = false;
bool old_control_leftR = false;
bool old_control_rightR = false;
bool old_control_upR = false;
bool old_control_downR = false;
bool old_control_any = false;

bool LoadImages()
{
	// oh boy this is huge. make sure to really own teh game cuz yes.
	if(!RSX_LoadTextureFromFile(2, GAME_DIR "images/titleback.jpg")) return false;
	if(!RSX_LoadTextureFromFileMasked(3, GAME_DIR "images/mrmoneybags.jpg", GAME_DIR "images/_mrmoneybags.gif")) return false;
	if(!RSX_LoadTextureFromFileMasked(4, GAME_DIR "images/rightbar2.jpg", GAME_DIR "images/_rightbar2.gif")) return false;
	if(!RSX_LoadTextureFromFileMasked(5, GAME_DIR "images/leftbar.jpg", GAME_DIR "images/_leftbar.gif")) return false;
	if(!RSX_LoadTextureFromFileMasked(6, GAME_DIR "images/dollarstrip.gif", GAME_DIR "images/_dollarstrip.gif")) return false;
	
	// after everything checks out
	return true;
}

int main(int argc, const char* argv[])
{
	padInfo padinfo;
	int ip;
	static padData oldpad;
	padData paddata;
	
	srand(time(NULL));
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sysutil_cb, NULL);
	if(RSX_Init() !=0)
	{
		DebugLog("RSX: Init failed!");
		return -1;
	}
	
	u16 screen_width, screen_height;
	rsxBuffer buffers[MAX_BUFFERS];
	if (!getResolution(&screen_width, &screen_height))
	{
		DebugLog("Failed to get display resolution!");
		return -1;
	}

	DebugLog("Display: %dx%d\n", screen_width, screen_height);
	for (int i = 0; i < MAX_BUFFERS; i++) {
		if (!makeBuffer(&buffers[i], screen_width, screen_height, i))
		{
			DebugLog("Failed to create framebuffer %d!\n", i);
			return -1;
		}
	}
	
	if (RSX_LoadShaders() != 0)
	{
		DebugLog("RSX: shader loading failed!");
		return -1;
	}
	
	if (!RSX_CreateSpriteBuffers())
	{
		DebugLog("RSX: sprite buffer creation failed!");
		return -1;
	}
	
	sysModuleLoad(SYSMODULE_PNGDEC);
	sysModuleLoad(SYSMODULE_JPGDEC);
	
	savestate_init();
	Initialize();
	
	init_bgm();
	init_sfx();
	
	ioPadInit(7);
	Load_PNG();
	
	if(!LoadImages())
	{
		msgDialogOpen2(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON,
			"Error!\n\"images\" folder not found, or incomplete.", DemoCompleteCallback, NULL, NULL);
	}
	
    RSX_UploadSpritesheets();
	Text_Init();
	
	LangInit();
	
	StartLoad();
	
	for(int i = 0; i < MAX_BUFFERS; i++) RSX_TestDraw(&buffers[i]);
	if(!flip(RSX_GetContext(), MAX_BUFFERS - 1)) DebugLog("RSX: initial flip failed!\n");
	int currentBuffer = 0;
	
	depth_pitch = screen_width * 2;
	depth_buffer = (u32*)rsxMemalign(64, depth_pitch * screen_height);
	if (!depth_buffer)
	{
		DebugLog("RSX: depth buffer allocation failed!");
		return -1;
	}

	if (rsxAddressToOffset(depth_buffer, &depth_offset) != 0)
	{
		DebugLog("RSX: depth buffer offset failed!");
		return -1;
	}

    while(1)
    {
		sysUtilCheckCallback();
		
		if (exit_requested) break;
		
		UpdateFPS();
		
		UpdateFade(1.0f / 60.0f);
		
		if(game_loaded)
		{
			ioPadGetInfo(&padinfo);
			for(ip = 0; ip < MAX_PADS; ip++)
			{
				if(padinfo.status[ip])
				{
					ioPadGetData(ip, &paddata);
					
					bool control_left = paddata.BTN_LEFT || paddata.ANA_L_H < 80;
					bool control_right = paddata.BTN_RIGHT || paddata.ANA_L_H > 176;

					bool control_up = paddata.BTN_UP || paddata.ANA_L_V < 80;
					bool control_down = paddata.BTN_DOWN || paddata.ANA_L_V > 176;
					
					bool control_leftR = paddata.ANA_R_H < 80;
					bool control_rightR = paddata.ANA_R_H > 176;
					bool control_upR = paddata.ANA_R_V < 80;
					bool control_downR = paddata.ANA_R_V > 176;
					
					bool control_any = paddata.BTN_CROSS || paddata.BTN_CIRCLE || paddata.BTN_TRIANGLE || paddata.BTN_SQUARE || paddata.BTN_START || paddata.BTN_SELECT || paddata.BTN_L1 || paddata.BTN_L2 || paddata.BTN_R1 || paddata.BTN_R2 || paddata.BTN_L3 || paddata.BTN_R3;
					
					switch(currentScreen)
					{
						case LOGO:
						{
							static int video_frame = 0;
							static int video_framez = 0;
							//static int f = 0;
							
							video_framez++;
							video_frame = video_framez / 2;
							
							if(video_frame>=10 && control_any && !old_control_any) video_frame = 9999;
							
							if(video_frame>120)
							{
								currentScreen = TITLE;
								start_music = 1;
								video_frame = 0;
							}
							break;
						}
						case TITLE:
						{
							if(start_music)
							{
								start_music = 0;
								Tracker_Play("/dev_hdd0/game/BIGMONEY0/USRDIR/music/QualityTime_BigMoney.it", 1);
								Tracker_GotoPattern(70);
							}
							
							if(paddata.BTN_START && !oldpad.BTN_START) currentScreen = TEST;
							break;
						}
						case TEST:
						{
							if(paddata.BTN_CROSS && !oldpad.BTN_CROSS) Tracker_GotoPattern(24);
							if(paddata.BTN_CIRCLE && !oldpad.BTN_CIRCLE) Tracker_GotoPattern(38);
							if(paddata.BTN_SQUARE && !oldpad.BTN_SQUARE) Tracker_GotoPattern(0);
							if(paddata.BTN_TRIANGLE && !oldpad.BTN_TRIANGLE) Tracker_GotoPattern(55);
							if(paddata.BTN_START && !oldpad.BTN_START) Tracker_GotoPattern(70);
							if(paddata.BTN_SELECT && !oldpad.BTN_SELECT) Tracker_GotoPattern(35);
							
							//FadeInOut(&currentScreen, CREDITS);
							break;
						}
						default: break;
					}
					
					old_control_left = control_left;
					old_control_right = control_right;
					old_control_up = control_up;
					old_control_down = control_down;
					old_control_leftR = control_leftR;
					old_control_rightR = control_rightR;
					old_control_upR = control_upR;
					old_control_downR = control_downR;
					old_control_any = control_any;
					oldpad = paddata;
				}
			}
		}
		
		if(game_init && !game_loaded)
		{
			ioPadGetInfo(&padinfo);
			for(ip = 0; ip < MAX_PADS; ip++)
			{
				if(padinfo.status[ip])
				{
					ioPadGetData(ip, &paddata);
					if(paddata.BTN_CROSS && !oldpad.BTN_CROSS)
					{
						DebugLog("LOGOSHOW\n");
						show_logo=false;
						//PlayBGM(198, 1);
						game_loaded = 1;
					}
					oldpad = paddata;
				}
			}
		}
		
		//if(playing_bgm) UpdateBGM();
		Tracker_Update();
		
		waitFlip();
		
		RSX_BeginFrame();
		
		RSX_SetRenderTarget(&buffers[currentBuffer]);
		RSX_SetupViewport();
		RSX_ClearScreen();

		drawScene();
		RSX_FlushDraws();
		RSX_EndFrameMark();
		
		if (!flip(RSX_GetContext(), buffers[currentBuffer].id)) DebugLog("RSX: test flip failed!\n");
		currentBuffer = (currentBuffer + 1) % MAX_BUFFERS;
    }

	rsxFinish(RSX_GetContext(), 1);
	StopAudio();
	ioPadEnd();
	sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
    return 0;
}