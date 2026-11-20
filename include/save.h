#ifndef SAVE_H
#define SAVE_H

#include <sysutil/save.h>

#define SAVE_DIR "BIGMONEY"

typedef struct
{
	int save_level;
} SaveData;

extern SaveData gSave;
extern sysSaveBufferSettings buffer;

void SetSaveDefaults();

extern volatile int save_running;

enum
{
    SAVE_ICON,
    SAVE_DATA,
    SAVE_DONE
};

enum
{
    MODE_SAVE,
    MODE_LOAD
};

void savestate_init();
void status_cb(sysSaveCallbackResult *result, sysSaveStatusIn *in, sysSaveStatusOut *out);
void save_file_cb(sysSaveCallbackResult *result, sysSaveFileIn *in, sysSaveFileOut *out);
void load_file_cb(sysSaveCallbackResult *result, sysSaveFileIn *in, sysSaveFileOut *out);

int StartSave(void);
int StartLoad(void);

#endif