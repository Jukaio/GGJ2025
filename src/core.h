#pragma once


using i32 = int;
using i64 = long long;
using u32 = unsigned int;
using u64 = unsigned long long;

void platform_init();
void platform_destroy();
void message_box(const char* title, const char* body);

struct AssetRef
{
    u64 offset;
    u64 size;
};

#define ASSERT(condition, msg)                                                  \
    do {                                                                        \
        if (!(condition)) {                                                     \
            char buffer[1024];                                                  \
            snprintf(buffer, sizeof(buffer),                                    \
                     "Assertion failed: (%s)\nFile: %s\nLine: %d\nMessage: ",   \
                     #condition, __FILE__, __LINE__);                           \
            snprintf(buffer + strlen(buffer),                                   \
                     sizeof(buffer) - strlen(buffer),                           \
                     "%s", msg);                                                \
            message_box("Assertion Failed", buffer);                            \
            __debugbreak();                                                     \
        }                                                                       \
    } while (0)

#define ASSERT_FMT(condition, format, ...)                                      \
    do {                                                                        \
        if (!(condition)) {                                                     \
            char buffer[1024];                                                  \
            snprintf(buffer, sizeof(buffer),                                    \
                     "Assertion failed: (%s)\nFile: %s\nLine: %d\nMessage: ",   \
                     #condition, __FILE__, __LINE__);                           \
            snprintf(buffer + strlen(buffer),                                   \
                     sizeof(buffer) - strlen(buffer),                           \
                     format, ##__VA_ARGS__);                                    \
            message_box("Assertion Failed", buffer);                            \
            __debugbreak();                                                     \
        }                                                                       \
    } while (0)