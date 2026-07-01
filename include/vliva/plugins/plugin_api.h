#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VLIVA_PLUGIN_API_VERSION 2u
#define VLIVA_TRACKING_FRAME_VERSION 1u
#define VLIVA_TRACKING_MODEL_NAME_CAPACITY 128u
#define VLIVA_TRACKING_MODE_NAME_CAPACITY 64u

// Snapshot of tracking/model parameters exposed by host to plugins.
// Plugins can read this in on_frame() and emit their own protocol (UDP/OSC/etc).
typedef struct VlivaTrackingFrame {
    uint32_t struct_size;
    uint32_t frame_version;
    double time_seconds;
    int32_t has_face;
    int32_t has_valid_parameters;
    float angle_x;
    float angle_y;
    float angle_z;
    float body_angle_x;
    float body_angle_y;
    float body_angle_z;
    float eye_x;
    float eye_y;
    float eye_l_open;
    float eye_r_open;
    float mouth_open_y;
    float mouth_form;
    float jaw_open;
    char model_name[VLIVA_TRACKING_MODEL_NAME_CAPACITY];
    char tracking_mode[VLIVA_TRACKING_MODE_NAME_CAPACITY];
} VlivaTrackingFrame;

// Host callbacks exposed to plugins (C ABI).
typedef struct VlivaHostApi {
    // Optional logger (safe to call anytime from plugin callbacks).
    void (*log)(const char* message);

    // Query latest tracking frame from host. Returns 1 on success, 0 when
    // tracking data is unavailable.
    int (*get_tracking_frame)(VlivaTrackingFrame* out_frame);

    // Configure host UDP endpoint + enabled state. Returns 1 on success.
    int (*udp_configure)(const char* host, int port, int enabled);

    // Send a UDP payload using host UDP sender. Returns 1 on success.
    int (*udp_send)(const char* payload);

    // Directory for per-plugin config files. Steam builds resolve this to
    // bin/config next to the executable; development builds use build/dev/config.
    const char* (*plugin_config_dir)(void);

    // Read/write a key from bin/config/<plugin_id>.toml. Values are stored as
    // strings so plugins can decide how to parse them.
    int (*plugin_config_read)(const char* plugin_id, const char* key, char* out_value, uint32_t out_value_capacity);
    int (*plugin_config_write)(const char* plugin_id, const char* key, const char* value);
} VlivaHostApi;

// Plugin surface expected by host (C ABI).
typedef struct VlivaPluginApi {
    uint32_t api_version;
    const char* (*id)(void);
    const char* (*name)(void);
    const char* (*description)(void);
    void (*on_load)(const VlivaHostApi* host);
    void (*on_unload)(void);
    void (*on_frame)(double time_seconds, float delta_seconds);
    const char* (*settings_schema_json)(void);
    void (*on_setting_changed)(const char* key, const char* value);
    void (*on_action)(const char* action_id);
} VlivaPluginApi;

// Required exported symbol from plugin shared object.
typedef const VlivaPluginApi* (*VlivaCreatePluginFn)(void);

#ifdef __cplusplus
}
#endif
