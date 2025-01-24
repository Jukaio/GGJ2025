
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellscalingapi.h>

#pragma comment(lib, "Shcore.lib")

void platform_init()
{
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
}

void platform_destroy()
{

}

void message_box(const char* title, const char* body)
{
	MessageBoxA(NULL, body, title, MB_ICONERROR | MB_OK);
}