#include <sysmodule/sysmodule.h>
#include <ppu-types.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <sys/thread.h>

#include "libraries.h"
#include "save.h"
#include "importer.h"
#include "lang.h"

#include "icon0_bin.h"

SaveData gSave;
sysSaveBufferSettings buffer;

static int saveMode;
static int operationMode;

static sys_ppu_thread_t save_thread = 0;
volatile int save_running = 0;
volatile int save_result = 0;

static sys_ppu_thread_t load_thread = 0;
volatile int load_running = 0;
volatile int load_result = 0;

static struct XMBEvent {
    int drawing;
    int opened;
    int closed;
    int exit;
} xmb;

void SetSaveDefaults()
{
    memset(&gSave, 0, sizeof(gSave));

    gSave.save_level = 0;
}

void savestate_init()
{
	memset(&buffer, 0, sizeof(buffer));
	buffer.maxDirectories = 1;
	buffer.maxFiles = 2;
	buffer.bufferSize = 1024 * 1024;
	buffer.buffer = malloc(buffer.bufferSize);
	
	char debug1[256];
	sprintf(debug1,"buffer = %p\n",buffer.buffer);
    DebugLog(debug1);
}

static u64 load_file_size;

void status_cb(sysSaveCallbackResult *result, sysSaveStatusIn *in, sysSaveStatusOut *out)
{
    static sysSaveSystemFileParam param;
    memset(&param, 0, sizeof(param));

	snprintf(param.title, sizeof(param.title), "%s", global_name);
    snprintf(param.subtitle, sizeof(param.subtitle), "Game Save Data (%s)", global_version);
    //snprintf(param.detail, sizeof(param.detail), "%s\nTotal Minecoins: %d", splash, global_minecoins);
    snprintf(param.detail, sizeof(param.detail), "Beta-Testing!");
	
    if(out)
	{
		out->setParam = &param;
		if (operationMode == MODE_SAVE) out->recreateMode = SYS_SAVE_RECREATE_MODE_DELETE;
		else if (operationMode == MODE_LOAD)
		{
            out->recreateMode = SYS_SAVE_RECREATE_MODE_OVERWRITE_NOT_CORRUPTED;

            int found = 0;
            for (int i = 0; i < in->numFiles; i++)
            {
                if (in->fileList[i].fileType == SYS_SAVE_FILETYPE_STANDARD_FILE)
                {
                    found = 1;
					load_file_size = in->fileList[i].fileSize;
                    if (load_file_size > sizeof(gSave))
                    {
                        DebugLog("LOAD: SAVE.DAT has wrong size\n");
                        result->result = SYS_SAVE_CALLBACK_RESULT_CORRUPTED;
                        return;
                    }

                    DebugLog("LOAD: SAVE.DAT found\n");
                    break;
                }
            }

            if (!found)
            {
                DebugLog("LOAD: SAVE.DAT not found\n");
                result->result = SYS_SAVE_CALLBACK_RESULT_NOT_FOUND;
                return;
            }
		}
	}

    result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
}

void save_file_cb(sysSaveCallbackResult *result, sysSaveFileIn *in, sysSaveFileOut *out)
{
	memset(out, 0, sizeof(*out));
	switch(saveMode)
    {
        case SAVE_ICON:
		{
            out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
            out->fileType      = SYS_SAVE_FILETYPE_CONTENT_ICON0;
            out->buffer     = (void *)icon0_bin;
            out->bufferSize = icon0_bin_size;
            out->size       = icon0_bin_size;

            result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
			result->incrementProgress = 10;
            saveMode = SAVE_DATA;
            break;
		}
        case SAVE_DATA:
		{
			static char filename[] = "SAVE.DAT";
			
            out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
            out->fileType      = SYS_SAVE_FILETYPE_STANDARD_FILE;
            out->filename      = filename;
            out->buffer     = &gSave;
            out->bufferSize = sizeof(gSave);
            out->size       = sizeof(gSave);

			result->incrementProgress = 90;
            result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
            saveMode = SAVE_DONE;
            break;
		}
        case SAVE_DONE:
        {
            result->result = SYS_SAVE_CALLBACK_RESULT_DONE;
            break;
        }
    }
}

void load_file_cb(sysSaveCallbackResult *result, sysSaveFileIn *in, sysSaveFileOut *out)
{
	DebugLog("LOAD FILE CALLBACK ENTER\n");
    static char filename[] = "SAVE.DAT";
	
	memset(out, 0, sizeof(*out));

    if(in->previousOperationResultSize == 0)
    {
        DebugLog("LOAD CALLBACK: requesting SAVE.DAT\n");

        out->fileOperation = SYS_SAVE_FILE_OPERATION_READ;
        out->fileType = SYS_SAVE_FILETYPE_STANDARD_FILE;
        out->filename = filename;

        out->buffer = &gSave;
        out->bufferSize = sizeof(gSave);
        out->size = load_file_size;

        result->incrementProgress = 100;
        result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
    }
    else
    {
        char debug[256];

        sprintf(debug,
            "LOAD CALLBACK: read=%d expected=%d\n",
            (int)in->previousOperationResultSize,
            (int)load_file_size);

        DebugLog(debug);

        if(in->previousOperationResultSize == load_file_size)
        {
            DebugLog("LOAD CALLBACK: SAVE.DAT read successfully\n");
            result->result = SYS_SAVE_CALLBACK_RESULT_DONE;
        }
        else
        {
            DebugLog("LOAD CALLBACK: SAVE.DAT read failed\n");
            result->result = SYS_SAVE_CALLBACK_RESULT_CORRUPTED;
        }
    }
}

static void SaveThread(void *arg)
{
	s32 ret;
	
	ret = sysSaveAutoSave2(SYS_SAVE_CURRENT_VERSION,SAVE_DIR,
		SYS_SAVE_ERROR_DIALOG_SHOW,&buffer,status_cb,save_file_cb,0,NULL);

    save_result = ret;
    save_running = 0;
    save_thread = 0;
	
	char debug[256];
	sprintf(debug,"Save finished: %d\n",ret);
    DebugLog(debug);
	xmb.opened = 0;
	xmb.closed = 1;
	xmb.drawing = 0;

    sysThreadExit(0);
}

int StartSave(void)
{
    if(save_running) return -1;

    gSave.save_level = 666;
	
	saveMode = SAVE_ICON;
	operationMode = MODE_SAVE;
	
    save_running = 1;
	xmb.opened = 1;
	xmb.closed = 0;
	xmb.drawing = 1;

    s32 ret = sysThreadCreate(&save_thread, SaveThread, NULL, 1500, 0x4000, 0, "SaveThread");

    if(ret != 0)
    {
		char debug[256];
		sprintf(debug, "SAVE: thread creation failed (%08X)\n" ,ret);
		DebugLog(debug);
        save_running = 0;
        save_thread = 0;
        return ret;
    }

    return 0;
}

static void LoadThread(void *arg)
{
	SetSaveDefaults();
	
    s32 ret;
    DebugLog("LOAD: starting...\n");

    ret = sysSaveAutoLoad2(SYS_SAVE_CURRENT_VERSION, SAVE_DIR, SYS_SAVE_ERROR_DIALOG_SHOW,
		&buffer, status_cb, load_file_cb, 0, NULL);

    load_result = ret;
    load_running = 0;
    load_thread = 0;
	
	char debug1[256];
	sprintf(debug1,"LOAD: finished (%08X)\n",ret);
    DebugLog(debug1);
	
	//LEVEL = gSave.save_level;
	
	game_init = 1;
	
	xmb.opened = 0;
	xmb.closed = 1;
	xmb.drawing = 0;
	
    sysThreadExit(0);
}

int StartLoad(void)
{
    if(load_running) return -1;
	
	operationMode = MODE_LOAD;
	
    load_running = 1;
	xmb.opened = 1;
	xmb.closed = 0;
	xmb.drawing = 1;
    s32 ret = sysThreadCreate(&load_thread, LoadThread, NULL, 1500, 0x4000, 0, "LoadThread");

    if(ret != 0)
    {
		char debug1[256];
		sprintf(debug1,"LOAD: thread creation failed (%08X)\n",ret);
		DebugLog(debug1);

        load_running = 0;
        load_thread = 0;

        return ret;
    }

    return 0;
}