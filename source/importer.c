#include "importer.h"
#include "assets.h"
#include "libraries.h"
#include "tracker.h"
#include "soundlib/spu_soundlib.h"
#include "soundlib/audioplayer.h"
#include "spu_soundmodule.bin.h"

char global_name[] = "Big Money";
char global_version[] = "Proof of Concept";

bool doLogs = true;

float RENDER_SIZE;
int RENDERING_PIXELS = 16;
float RENDERING_SIZE = 2.0f;

int screen_w = 848 / 2;
int screen_h = 512 / 2;

int offset_x;
int offset_y;
int ui_offset_y = 66;

bool playing_bgm = false;
bool show_warning = false;
bool show_logo = false;

int game_loaded = 0;
int start_music = 0;
int game_init = 0;
int current_bgm = 0;

int fps = 0;
static int frames = 0;
static u64 last_time = 0;

float lerp(int x, int y, float amount)
{
	return x + amount * (y - x);
}

float flerp(float x, float y, float amount)
{
	return x + amount * (y - x);
}

void UpdateFPS()
{
    u64 now = sysGetSystemTime();
    frames++;

    if(now - last_time >= 1000000ULL)
    {
        fps = frames;
        frames = 0;
        last_time = now;
    }
}

int sex = 2147483647;

void init_bgm()
{
	for(int z = 0; z < BGM_LIST; z++)
	{
		bgm[z] = "";
		bgm_start[z] = 0;
		bgm_end[z] = sex;
	}
	
	bgm[0] = MUS_DIR "title.mp3";
	bgm_start[0] = 18400;
	bgm_end[0] = 132794;
	
}

int LoadSFX(SoundEffect *sfx, const void *data, int size)
{
    sfx->pcm_size = size * 20;
	sfx->pcm = memalign(32, sfx->pcm_size);

    if(!sfx->pcm) return 0;
    if(DecodeAudio((void *)data,size,sfx->pcm,&sfx->pcm_size,&sfx->freq,&sfx->stereo) != 0)
    {
        free(sfx->pcm);
        sfx->pcm = NULL;
        return 0;
    }

    return 1;
}

void init_sfx()
{
	//LoadSFX(&sfx_buy, cash_register_bin, cash_register_bin_size);
}

int RandRange(int min, int max)
{
	int rndom;
	rndom = rand() % (max-min+1) + min;
	return rndom;
}

int max(int min, int max)
{
	int biggest;
	biggest = (min > max) ? min : max;
	
	return biggest;
}

void PlayBGM(int id, int inf)
{
	StopAudio();
	
	FILE *fp = fopen(bgm[id], "rb");
	
	if(!fp)
    {
        DebugLog("Error! No music found.\n");
        return;
    }
	
    PlayAudiofd(fp, 0, inf ? AUDIO_INFINITE_TIME : AUDIO_ONE_TIME);
    current_bgm = id;
	
	DebugLog("FILE: %s\n", bgm[id]);
	DebugLog("START: %d  ---  ", bgm_start[id]);
	DebugLog("END: %d\n", bgm_end[id]);
}

void UpdateBGM(void)
{
	s32 temp_t = GetTimeAudio();
	int tempzz = 50;
	if(temp_t >= bgm_end[current_bgm]-tempzz) SetTimeAudio(bgm_start[current_bgm]-tempzz);
}

void PlaySFX(SoundEffect *sfx)
{
	int voice = SND_GetFirstUnusedVoice();
    if (voice < 1) return;
	
    SND_SetVoice(voice, sfx->stereo ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT,
        sfx->freq, 0, sfx->pcm, sfx->pcm_size, 255, 255, NULL);
	
	sfx->voice = voice;
}

void StopSFX(SoundEffect *sfx)
{
    if(sfx->voice >= 1)
    {
        SND_StopVoice(sfx->voice);
        sfx->voice = -1;
    }
}

void Load_PNG()
{
	pngLoadFromBuffer(buttons_bin, buttons_bin_size, &spritesheet[0]);
	pngLoadFromBuffer(logo_bin, logo_bin_size, &spritesheet[1]);
}

void DemoCompleteCallback(msgButton button, void *usrData)
{
    sysProcessExit(0);
}

void EmptyDialogCallback(msgButton button, void *usrData)
{
    // ehhehe fuck this shit
}

void Initialize()
{
	DebugLog("Game initialization...\n");
	u32 spu;
	sysSpuImage spu_image;
	sysSpuInitialize(1, 5);
	sysSpuRawCreate(&spu, NULL);
	sysSpuImageImport(&spu_image, spu_soundmodule_bin, 0);
	sysSpuRawImageLoad(spu, &spu_image);
	//SND_Init(spu);
	Tracker_Init();
}

void DebugLog(const char *format, ...)
{
	char buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
	
	printf(buffer);
	
	if(doLogs)
	{
		FILE *fp = fopen("/dev_hdd0/game/BIGMONEY0/USRDIR/biglog.txt", "ab");
		if(fp)
		{
			fprintf(fp, buffer);
			fclose(fp);
		}
	}
}