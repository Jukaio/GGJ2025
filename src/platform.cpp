#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellscalingapi.h>

#pragma comment(lib, "Shcore.lib")

void message_box(const char* title, const char* body)
{
	MessageBoxA(NULL, body, title, MB_ICONERROR | MB_OK);
}

#endif