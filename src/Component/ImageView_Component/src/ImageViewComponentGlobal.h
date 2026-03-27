#ifndef IMAGEVIEWCOMPONENTGLOBAL_H
#define IMAGEVIEWCOMPONENTGLOBAL_H

#if defined(_WIN32) || defined(_WIN64)
#  ifdef IMAGEVIEWCOMPONENT_EXPORTS
#    define IMAGEVIEWCOMPONENT_API __declspec(dllexport)
#  else
#    define IMAGEVIEWCOMPONENT_API __declspec(dllimport)
#  endif
#  define IMAGEVIEWCOMPONENT_CALL __cdecl
#else
#  define IMAGEVIEWCOMPONENT_API __attribute__((visibility("default")))
#  define IMAGEVIEWCOMPONENT_CALL
#endif

#endif // IMAGEVIEWCOMPONENTGLOBAL_H
