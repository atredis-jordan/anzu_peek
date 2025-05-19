#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>


#define DATA_PATH "C:\ProgramData\Trackmania\Anzu"
#define LOG_NAME "log.log"

// TODO use an env var for this one, otherwise put it somewhere everyone will have?
#define LOG_FILE_PATH "D:\\Projects\\re\\anzu\\anzu.log"
#define ORIGINAL_PATH ".\\anzu_original.dll"
// data path is already being set by the game
// careful, this folder gets cleared on startup
#define DATA_PATH "C:\ProgramData\Trackmania\Anzu"
#define LOG_NAME "log.log"

FILE* g_logf = NULL;

#include "declarations.h"


// 0xc0de5b10 - ctx = pointer to utf8 zstr to C:\ProgramData\Trackmania\Anzu (db in there)

__declspec(dllexport) FARPROC Anzu_InternalDebugging(int32_t code, void* ctx) {
    FARPROC ret = NULL;

    //TODO intercept callback ones
    if (0xc0de5b31 == code) {
        //TODO
    }

    if (0xc0de5b04 == code) {
        //TODO
    }

    ret = (FARPROC)real_Anzu_InternalDebugging(code, ctx);

    fprintf(g_logf, "@ Anzu_InternalDebugging(%x, %p) => %p\n", code, ctx, ret);

    return ret;
}

__declspec(dllexport) void Anzu__Texture_SetVisibilityScore(int32_t index, float farg1, float farg2, float farg3) {
    // RE makes it look like they are just using singles

    fprintf(g_logf, "@ Anzu__Texture_SetVisibilityScore(%x, %f %f %f)\n", index, farg1, farg2, farg3);
    fflush(g_logf);

    real_Anzu_InternalDebugging(index, farg1, farg2, farg3);
}

__declspec(dllexport) void Anzu_SetLogLevel(int32_t level) {
    static BOOL firstcall = TRUE;

    fprintf(g_logf, "@ Anzu_SetLogLevel(%x) (forcing 0 and injecting debug setup)\n", level);
    real_Anzu_SetLogLevel(0);

    if (firstcall) {
        firstcall = FALSE;
        // this appends a log name to the already set data path
        // so we now get logging going!
        fprintf(g_logf, "DEBUG: Force registering log file:" LOG_NAME "\n");
        real_Anzu_InternalDebugging(0xc0de5b05, LOG_NAME);
    }
}

__declspec(dllexport) void Anzu_ApplicationActive(int32_t value) {
    fprintf(g_logf, "@ Anzu_ApplicationActive(%x)\n", value);
    real_Anzu_ApplicationActive(value);
}

void (*g_msg_cb)(void*, const char*) = NULL;
void* g_msg_ctx = NULL;

void fake_msg_cb(void* ctx, const char* msg) {
    fprintf(g_logf, "@ Message Callback: %s\n", msg);
    g_msg_cb(g_msg_ctx, msg);
}

__declspec(dllexport) void Anzu_RegisterMessageCallback(void* cb, void* ctx) {

    fprintf(g_logf, "@ Anzu_RegisterMessageCallback(%p, %p) (redirecting)\n", cb, ctx);

    g_msg_cb = cb;
    g_msg_ctx = ctx;

    real_Anzu_RegisterMessageCallback(fake_msg_cb, NULL);
}

void (*g_log_cb)(void*, uint64_t, const char*) = NULL;
void* g_log_ctx = NULL;

void fake_log_cb(void* ctx, uint64_t level, const char* msg) {
    fprintf(g_logf, "@ Log CB %lld: %s\n", level, msg);
    g_log_cb(g_msg_ctx, level, msg);
}

int64_t fake_net_cb(void* ctx, int32_t lockstatus, uint8_t is_precb) {
    // is a pre and post callback
    fprintf(g_logf, "@ Net CB %d %d\n", lockstatus, is_precb);
    return 0;
}

__declspec(dllexport) void Anzu_RegisterLogCallback(void* cb, void* ctx) {

    fprintf(g_logf, "@ Anzu_RegisterMessageCallback(%p, %p) (redirecting)\n", cb, ctx);

    g_log_cb = cb;
    g_log_ctx = ctx;

    real_Anzu_RegisterLogCallback(fake_log_cb, NULL);

    // also init our own network callback for debug
    fprintf(g_logf, "DEBUG: Force registering net callback\n");
    real_Anzu_RegisterNetworkCallback(fake_net_cb, NULL);
}


//TODO
/*
__declspec(dllexport) void Anzu_RegisterTextureUpdateCallback(void* cb, void* ctx) {

    fprintf(g_logf, "@ Anzu_RegisterMessageCallback(%p, %p) (redirecting)\n", cb, ctx);

    g_tex_upd_cb = cb;
    g_tex_upd_ctx = ctx;

    real_Anzu_RegisterTextureUpdateCallback(fake_tex_upd_cb, NULL);
}

__declspec(dllexport) void Anzu_RegisterTextureInitCallback(void* cb, void* ctx) {

    fprintf(g_logf, "@ Anzu_RegisterMessageCallback(%p, %p) (redirecting)\n", cb, ctx);

    g_tex_ini_cb = cb;
    g_tex_ini_ctx = ctx;

    real_Anzu_RegisterTextureInitCallback(fake_tex_ini_cb, NULL);
}

__declspec(dllexport) void Anzu_RegisterTextureImpressionCallback(void* cb, void* ctx) {

    fprintf(g_logf, "@ Anzu_RegisterMessageCallback(%p, %p) (redirecting)\n", cb, ctx);

    g_tex_imp_cb = cb;
    g_tex_imp_ctx = ctx;

    real_Anzu_RegisterImpressionCallback(fake_tex_imp_cb, NULL);
}

__declspec(dllexport) void Anzu_RegisterTexturePlaybackCompleteCallback(void* cb, void* ctx) {

    fprintf(g_logf, "@ Anzu_RegisterMessageCallback(%p, %p) (redirecting)\n", cb, ctx);

    g_tex_play_cmpl_cb = cb;
    g_tex_play_cmpl_ctx = ctx;

    real_Anzu_RegisterTexturePlaybackCompleteCallback(fake_tex_play_cmpl_cb, NULL);
}
*/

void init_original() {
    errno_t err;

    err = fopen_s(&g_logf, LOG_FILE_PATH, "a");
    
    if (err) {
        __fastfail(FAST_FAIL_INVALID_FAST_FAIL_CODE);
    }

    fprintf(g_logf, "Interception DLL Loaded\n");

    HMODULE original = LoadLibraryA(ORIGINAL_PATH);

    if (original == NULL) {
        fprintf(g_logf, "Unable to open original dll at %s: gle 0x%x\n", ORIGINAL_PATH, GetLastError());
        __fastfail(FAST_FAIL_PATCH_CALLBACK_FAILED);
    }

#include "fn_inits.h"

    fprintf(g_logf, "Init complete\n");
}

void log_proc(char* fn_name) {
    fprintf(g_logf, "@ %s\n", fn_name);
    fflush(g_logf);
    return;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        init_original();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

