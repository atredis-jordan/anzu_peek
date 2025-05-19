#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>

#define LIB_PATH "./anzu.dll"

//CAREFUL! IT WILL DELETE EVERYTHING IN THE DATA PATH FOLDER!
#define DATA_PATH "D:\\Projects\\re\\anzu\\data\\"
#define LOG_NAME "harn.log"

#define TRACKMANIA_APPKEY   "com.nadeo.trackmania.windows"
#define TRACKMANIA_APPID    "999273fabbb04829eb1945a2"


#define CACHE_CAPACITY  0x8000000

void* (*Anzu_InternalDebugging)(int32_t code, void* ctx) = NULL;
void (*Anzu_SetGDPRConsent)(char consent, const char* somestr) = NULL;
void (*Anzu_ApplicationActive)(char is_active) = NULL;
void (*Anzu_SetLogLevel)(int32_t level) = NULL;

typedef void(*log_cb_f)(void* ctx, uint64_t level, const char* line);
void (*Anzu_RegisterLogCallback)(log_cb_f cb, void* ctx) = NULL;

typedef void(*msg_cb_f)(void* ctx, const char* msg);
void (*Anzu_RegisterMessageCallback)(msg_cb_f cb, void* ctx) = NULL;

typedef uint64_t(*unk_cb_f)();
void (*Anzu_RegisterTextureUpdateCallback)(unk_cb_f cb, void* ctx) = NULL;
void (*Anzu_RegisterTextureInitCallback)(unk_cb_f cb, void* ctx) = NULL;
void (*Anzu_RegisterTextureImpressionCallback)(unk_cb_f cb, void* ctx) = NULL;
void (*Anzu_RegisterTexturePlaybackCompleteCallback)(unk_cb_f cb, void* ctx) = NULL;

uint64_t(*Anzu_Initialize)(const char* app_key, const char* app_id, char unk_3);

//TODO Anzu_RegisterMessageEventCallback (Not used in game?)
//TODO Anzu_RegisterNetworkCallback(cb, ctx) (Not used in game?)

#define GETPROC(name) do { \
        name = GetProcAddress(lib, #name); \
        if (name == NULL) { \
            fprintf(stderr, "Unable to get " #name " from library\n"); \
            return NULL; \
        } \
    } while (0)

HMODULE init_library()
{
    HMODULE lib = LoadLibraryA(LIB_PATH);

    if (lib == NULL) {
        fprintf(stderr, "Unable to open library at %s: gle 0x%x\n", LIB_PATH, GetLastError());
        return NULL;
    }

    GETPROC(Anzu_InternalDebugging);
    
    GETPROC(Anzu_SetGDPRConsent);
    GETPROC(Anzu_ApplicationActive);
    GETPROC(Anzu_SetLogLevel);

    GETPROC(Anzu_RegisterLogCallback);
    GETPROC(Anzu_RegisterMessageCallback);
    GETPROC(Anzu_RegisterTextureUpdateCallback);
    GETPROC(Anzu_RegisterTextureInitCallback);
    GETPROC(Anzu_RegisterTextureImpressionCallback);
    GETPROC(Anzu_RegisterTexturePlaybackCompleteCallback);

    GETPROC(Anzu_Initialize);

    return lib;
}

void log_cb(void* ctx, uint64_t level, const char* line)
{
    fprintf(stdout, "Log CB: %s\n", line);
    fflush(stdout);

}

uint64_t msg_cb(void* ctx, const char* msg)
{
    fprintf(stdout, "Msg CB: %s\n", msg);
    fflush(stdout);
    //TODO
    return 0;
}

uint64_t tex_upd_cb()
{
    fprintf(stdout, "Texture Update CB\n");
    fflush(stdout);
    //TODO
    return 0;
}

uint64_t tex_init_cb()
{
    fprintf(stdout, "Texture Init CB\n");
    fflush(stdout);
    //TODO
    return 0;
}

uint64_t tex_imp_cb()
{
    fprintf(stdout, "Texture Impression CB\n");
    fflush(stdout);
    //TODO
    return 0;
}

uint64_t tex_pb_comp_cb()
{
    fprintf(stdout, "Texture Playback Complete CB\n");
    fflush(stdout);
    //TODO
    return 0;
}

uint64_t unk_5b31_cb()
{
    fprintf(stdout, "Request CB\n");
    fflush(stdout);
    //TODO
    return 0;
}

uint64_t unk_5b04_cb()
{
    fprintf(stdout, "Unk 5b04 CB\n");
    fflush(stdout);
    //TODO
    return 0;
}

void anzu_setup() {
    Anzu_InternalDebugging(0xc0de5b11, (void*)CACHE_CAPACITY);

    Anzu_SetGDPRConsent(1, NULL);

    // set up logging
    Anzu_InternalDebugging(0xc0de5b10, DATA_PATH);
    Anzu_InternalDebugging(0xc0de5b05, LOG_NAME);
    Anzu_SetLogLevel(0);

    Anzu_ApplicationActive(1);

    // whats it do?
    Anzu_InternalDebugging(0xc0de5b03, (void*)1);

    Anzu_RegisterLogCallback(log_cb, NULL);
    Anzu_RegisterMessageCallback(msg_cb, NULL);
    Anzu_RegisterTextureUpdateCallback(tex_upd_cb, NULL);
    Anzu_RegisterTextureInitCallback(tex_init_cb, NULL);
    Anzu_RegisterTextureImpressionCallback(tex_imp_cb, NULL);
    Anzu_RegisterTexturePlaybackCompleteCallback(tex_pb_comp_cb, NULL);

    // some kind of download/request callback?
    Anzu_InternalDebugging(0xc0de5b31, unk_5b31_cb);

    //TODO, what do? Very small function in game just drefing an int ptr
    Anzu_InternalDebugging(0xc0de5b04, unk_5b04_cb);

    //TODO, what do?
    Anzu_InternalDebugging(0xc0de5aff, (void*)1);
    Anzu_InternalDebugging(0xc0de5b00, (void*)1);

    // set supported/prefered formats?
    Anzu_InternalDebugging(0xc0de5b16, "image/x-dds");
    Anzu_InternalDebugging(0xc0de5b26, "video/webm");
    Anzu_InternalDebugging(0xc0de5b16, "dds");
    Anzu_InternalDebugging(0xc0de5b26, "webm");

    // idk what the 0x60 is about? It seems left over from a pointer? not sure where used.
    //TODO getting a null dref in Initialize
    uint64_t init_res = Anzu_Initialize(TRACKMANIA_APPKEY, TRACKMANIA_APPID, 0x60);

    fprintf(stdout, "Initialize result: %llx\n", init_res);

    Anzu_ApplicationActive(1);
    return;
}

int main(int argc, char** argv)
{
	HMODULE library = init_library();
    if (library == NULL) {
        return -1;
    }

    fprintf(stdout, "PID = %d\n(enter to continue)\n", GetCurrentProcessId());
    getchar();

    anzu_setup();

    //TODO requests failing sometimes? Why?

    // create a texture
    //TODO

    // wait for callbacks

    fprintf(stdout, "Waiting for interaction\n(enter to exit)\n");
    getchar();

    //TODO close down

    //TODO bad free on dll unload?

    return 0;
}