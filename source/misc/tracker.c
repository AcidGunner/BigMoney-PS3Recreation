#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mikmod.h>
#include "tracker.h"
#include "importer.h"

static MODULE *tracker_module = NULL;
static int tracker_initialized = 0;

int Tracker_Init()
{
    if (tracker_initialized) return 1;

    DebugLog("TRACKER: Initializing MikMod...\n");

    MikMod_RegisterDriver(&drv_psl1ght);
    MikMod_RegisterAllLoaders();

    md_mixfreq = 44100;
    md_mode = DMODE_16BITS | DMODE_STEREO | DMODE_SOFT_MUSIC | DMODE_INTERP;

    if (MikMod_Init(NULL))
    {
        DebugLog("TRACKER: Init failed: %s\n", MikMod_strerror(MikMod_errno));
        return 0;
    }

    tracker_initialized = 1;
    DebugLog("TRACKER: Initialized successfully.\n");

    return 1;
}

int Tracker_Play(const char *filename, int loop)
{
    if (!tracker_initialized)
    {
        DebugLog("TRACKER: Not initialized!\n");
        return 0;
    }

    Tracker_Stop();

    DebugLog("TRACKER: Loading %s\n", filename);
    tracker_module = Player_Load((CHAR *)filename, 64, 0);

    if (!tracker_module)
    {
        DebugLog("TRACKER: Failed to load module: %s\n", MikMod_strerror(MikMod_errno));
        return 0;
    }

    tracker_module->loop = loop ? 1 : 0;
    Player_Start(tracker_module);

    DebugLog("TRACKER: Playback started.\n");

    return 1;
}

void Tracker_Update()
{
    if (!tracker_initialized) return;
    MikMod_Update();
}

void Tracker_Stop()
{
    if (!tracker_module) return;

    Player_Stop();
    Player_Free(tracker_module);
    tracker_module = NULL;

    DebugLog("TRACKER: Stopped.\n");
}

int Tracker_IsPlaying()
{
    return tracker_module != NULL && Player_Active();
}

void Tracker_Shutdown()
{
    if (!tracker_initialized) return;

    Tracker_Stop();
    MikMod_Exit();
    tracker_initialized = 0;

    DebugLog("TRACKER: Shutdown.\n");
}

int Tracker_GotoPattern(int pattern)
{
    MODULE *mod = Player_GetModule();

    if (!mod || pattern < 0 || pattern >= mod->numpat) return 0;
    for (int i = 0; i < mod->numpos; i++)
    {
        if (mod->positions[i] == pattern)
        {
            Player_SetPosition(i);
            return 1;
        }
    }

    return 0;
}