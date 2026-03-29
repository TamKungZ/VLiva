#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VLIVA_PLUGIN_API_VERSION 1u

// Host callbacks exposed to plugins (C ABI).
typedef struct VlivaHostApi {
    void (*log)(const char* message);
} VlivaHostApi;

// Plugin surface expected by host (C ABI).
typedef struct VlivaPluginApi {
    uint32_t api_version;
    const char* (*name)(void);
    void (*on_load)(const VlivaHostApi* host);
    void (*on_unload)(void);
    void (*on_frame)(double time_seconds, float delta_seconds);
} VlivaPluginApi;

// Required exported symbol from plugin shared object.
typedef const VlivaPluginApi* (*VlivaCreatePluginFn)(void);

#ifdef __cplusplus
}
#endif
