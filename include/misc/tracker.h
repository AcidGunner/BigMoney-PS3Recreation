#ifndef TRACKER_H
#define TRACKER_H

int Tracker_Init();
int Tracker_Play(const char *filename, int loop);
void Tracker_Update();
void Tracker_Stop();
void Tracker_Shutdown();
int Tracker_IsPlaying();
int Tracker_GotoPattern(int pattern);

#endif