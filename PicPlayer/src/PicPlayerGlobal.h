#ifndef PICPLAYER_GLOBAL_H
#define PICPLAYER_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif
    #ifdef _WIN32
        #ifdef PICPLAYER_EXPORTS
            #define PICPLAYER_API __declspec(dllexport)
        #else
            #define PICPLAYER_API __declspec(dllimport)
        #endif
        #define PICPLAYER_CALL __stdcall
    #else
        #define PICPLAYER_API __attribute__((visibility("default")))
        #define PICPLAYER_CALL
    #endif
#ifdef __cplusplus
}
#endif
#endif // PICPLAYER_GLOBAL_H
