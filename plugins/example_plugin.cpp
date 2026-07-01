#include "vliva/plugins/plugin_api.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

const VlivaHostApi* g_host = nullptr;
double g_lastSendTime = 0.0;
bool g_enabled = true;
float g_gain = 1.0f;
constexpr double kSendIntervalSeconds = 1.0 / 60.0;

const char* pluginId()
{
    return "com.vliva.example_udp_bridge";
}

const char* pluginName()
{
    return "Example UDP Bridge Plugin";
}

const char* pluginDescription()
{
    return "Example external plugin that reads tracking data, exposes settings UI, and sends a simple UDP text payload.";
}

const char* settingsSchemaJson()
{
    return R"json({
      "controls": [
        {
          "type": "text",
          "label": "About",
          "text": "This plugin demonstrates VLiva plugin settings. Values are saved in bin/config/com.vliva.example_udp_bridge.toml."
        },
        {
          "type": "checkbox",
          "key": "enabled",
          "label": "Enable UDP bridge",
          "default": "true"
        },
        {
          "type": "slider",
          "key": "gain",
          "label": "Output gain",
          "description": "Multiplier applied to outgoing sample values.",
          "min": 0,
          "max": 3,
          "step": 0.05,
          "default": "1.0"
        },
        {
          "type": "textbox",
          "key": "note",
          "label": "Developer note",
          "placeholder": "Any text saved for this plugin",
          "default": ""
        },
        {
          "type": "button",
          "action": "send_test",
          "label": "Send test packet"
        }
      ]
    })json";
}

void pluginLog(const char* message)
{
    if (g_host && g_host->log && message)
    {
        g_host->log(message);
    }
}

bool isTrue(const char* value)
{
    return value && (std::strcmp(value, "1") == 0 || std::strcmp(value, "true") == 0 || std::strcmp(value, "True") == 0);
}

void readSettings()
{
    if (!g_host || !g_host->plugin_config_read)
    {
        return;
    }

    char value[128] = {0};
    if (g_host->plugin_config_read(pluginId(), "enabled", value, sizeof(value)))
    {
        g_enabled = isTrue(value);
    }
    if (g_host->plugin_config_read(pluginId(), "gain", value, sizeof(value)))
    {
        const float parsed = std::strtof(value, nullptr);
        if (parsed >= 0.0f && parsed <= 3.0f)
        {
            g_gain = parsed;
        }
    }
}

void onLoad(const VlivaHostApi* host)
{
    g_host = host;
    g_lastSendTime = 0.0;
    readSettings();

    int udpConfigured = 0;
    if (g_host && g_host->udp_configure)
    {
        const char* envHost = std::getenv("VLIVA_PLUGIN_UDP_HOST");
        const char* envPort = std::getenv("VLIVA_PLUGIN_UDP_PORT");

        const char* targetHost = (envHost && envHost[0] != '\0') ? envHost : "127.0.0.1";
        int targetPort = 11573;
        if (envPort && envPort[0] != '\0')
        {
            const int parsed = std::atoi(envPort);
            if (parsed > 0 && parsed <= 65535)
            {
                targetPort = parsed;
            }
        }

        udpConfigured = g_host->udp_configure(targetHost, targetPort, 1);
    }

    if (udpConfigured)
    {
        pluginLog("Example UDP Bridge Plugin loaded (configured host UDP sender)");
    }
    else
    {
        pluginLog("Example UDP Bridge Plugin loaded");
    }
}

void onUnload()
{
    pluginLog("Example UDP Bridge Plugin unloaded");
    g_host = nullptr;
}

void onSettingChanged(const char* key, const char* value)
{
    if (!key || !value)
    {
        return;
    }
    if (std::strcmp(key, "enabled") == 0)
    {
        g_enabled = isTrue(value);
    }
    else if (std::strcmp(key, "gain") == 0)
    {
        const float parsed = std::strtof(value, nullptr);
        if (parsed >= 0.0f && parsed <= 3.0f)
        {
            g_gain = parsed;
        }
    }
}

void onAction(const char* actionId)
{
    if (!g_host || !g_host->udp_send || !actionId)
    {
        return;
    }
    if (std::strcmp(actionId, "send_test") == 0)
    {
        g_host->udp_send("VLIVA1_TEST");
        pluginLog("Example UDP Bridge Plugin sent a test packet");
    }
}

void onFrame(double timeSeconds, float /*deltaSeconds*/)
{
    if (!g_enabled || !g_host || !g_host->get_tracking_frame || !g_host->udp_send)
    {
        return;
    }

    if ((timeSeconds - g_lastSendTime) < kSendIntervalSeconds)
    {
        return;
    }

    VlivaTrackingFrame frame{};
    frame.struct_size = sizeof(VlivaTrackingFrame);
    if (!g_host->get_tracking_frame(&frame) || !frame.has_valid_parameters)
    {
        return;
    }

    // Minimal text protocol example.
    // You can replace this formatter with VBridger/Inochi2D-specific payloads.
    char payload[512] = {0};
    const int written = std::snprintf(
        payload,
        sizeof(payload),
        "VLIVA1 %d %.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f",
        frame.has_face,
        frame.angle_x * g_gain,
        frame.angle_y * g_gain,
        frame.angle_z * g_gain,
        frame.eye_x * g_gain,
        frame.eye_y * g_gain,
        frame.eye_l_open * g_gain,
        frame.eye_r_open * g_gain,
        frame.mouth_open_y * g_gain,
        frame.mouth_form * g_gain,
        frame.jaw_open * g_gain);

    if (written <= 0 || written >= static_cast<int>(sizeof(payload)))
    {
        return;
    }

    if (g_host->udp_send(payload))
    {
        g_lastSendTime = timeSeconds;
    }
}

} // namespace

extern "C" const VlivaPluginApi* create_plugin(void)
{
    static VlivaPluginApi api = {
        VLIVA_PLUGIN_API_VERSION,
        &pluginId,
        &pluginName,
        &pluginDescription,
        &onLoad,
        &onUnload,
        &onFrame,
        &settingsSchemaJson,
        &onSettingChanged,
        &onAction
    };
    return &api;
}
