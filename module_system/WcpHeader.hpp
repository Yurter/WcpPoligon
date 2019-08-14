#pragma once

#ifdef WCP_MAIN
#define WCP_DLL_EXPORT __declspec(dllexport)
#else
#define WCP_DLL_EXPORT __declspec(dllimport)
#endif
