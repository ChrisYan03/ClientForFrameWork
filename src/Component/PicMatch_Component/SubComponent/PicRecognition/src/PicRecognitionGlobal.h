// e:\ClientForFrameWork\src\PicRecognition\src\PicRecognitionGlobal.h
#ifndef PICRECOGNITION_GLOBAL_H
#define PICRECOGNITION_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #ifdef PICRECOGNITION_EXPORTS
        #define PICRECOGNITION_API __declspec(dllexport)
    #else
        #define PICRECOGNITION_API __declspec(dllimport)
    #endif
    #define PICRECOGNITION_CALL __stdcall
#else
    #define PICRECOGNITION_API __attribute__((visibility("default")))
    #define PICRECOGNITION_CALL
#endif

#ifdef __cplusplus
}
#endif

#endif // PICRECOGNITION_GLOBAL_H