#ifndef PICMATCHCOMPONENT_GLOBAL_H
#define PICMATCHCOMPONENT_GLOBAL_H

#ifdef _WIN32
    #ifdef PICMATCHCOMPONENT_EXPORTS
        #define PICMATCHCOMPONENT_API __declspec(dllexport)
    #else
        #define PICMATCHCOMPONENT_API __declspec(dllimport)
    #endif
    #define PICMATCHCOMPONENT_CALL __cdecl
#else
    #define PICMATCHCOMPONENT_API __attribute__((visibility("default")))
    #define PICMATCHCOMPONENT_CALL
#endif

#endif // PICMATCHCOMPONENT_GLOBAL_H
