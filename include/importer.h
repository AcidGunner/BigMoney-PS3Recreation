#ifndef IMPORTER_H
#define IMPORTER_H

#include <pngdec/pngdec.h>
#include <sysutil/msg.h>

extern char global_name[];
extern char global_version[];

extern bool show_logo;
extern bool playing_bgm;

extern float RENDER_SIZE;
extern int RENDERING_PIXELS;
extern float RENDERING_SIZE;

extern int screen_w;
extern int screen_h;

extern int offset_x;
extern int offset_y;
extern int ui_offset_y;

extern int game_loaded;
extern int current_bgm;
extern int start_music;
extern int game_init;

typedef struct
{
    short *pcm;
    int pcm_size;
    int freq;
    int stereo;
    int voice;
} SoundEffect;

SoundEffect sfx_buy;

#define BGM_LIST 200
#define MUS_DIR "/dev_hdd0/game/BIGMONEY0/USRDIR/music/"

#define TOTAL_SHEETS 30
pngData spritesheet[TOTAL_SHEETS];
u32 sprite_off[TOTAL_SHEETS];

const char *bgm[BGM_LIST];
int bgm_start[BGM_LIST];
int bgm_end[BGM_LIST];

int LoadSFX();
void init_sfx();
void init_bgm();

int RandRange(int min, int max);
int max(int min, int max);

void PlayBGM(int id, int inf);
void UpdateBGM(void);
void PlaySFX(SoundEffect *sfx);
void StopSFX(SoundEffect *sfx);

void Load_PNG();

void UpdateFPS();
extern int fps;

void DemoCompleteCallback(msgButton button, void *usrData);
void EmptyDialogCallback(msgButton button, void *usrData);
void Initialize();

float lerp(int x, int y, float amount);
float flerp(float x, float y, float amount);

void DebugLog(const char *format, ...);

#endif