#include "vliva/plugin_api.h"

#include <cstdio>
#include <cstdlib>

namespace {

const VlivaHostApi* g_host = nullptr;
double g_lastSendTime = 0.0;
constexpr double kSendIntervalSeconds = 1.0 / 60.0;

const char* pluginName()
{
    return "Example UDP Bridge Plugin";
}

void pluginLog(const char* message)
{
    if (g_host && g_host->log && message)
    {
        g_host->log(message);
    }
}

void onLoad(const VlivaHostApi* host)
{
    g_host = host;
    g_lastSendTime = 0.0;

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

void onFrame(double timeSeconds, float /*deltaSeconds*/)
{
    if (!g_host || !g_host->get_tracking_frame || !g_host->udp_send)
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
        frame.angle_x,
        frame.angle_y,
        frame.angle_z,
        frame.eye_x,
        frame.eye_y,
        frame.eye_l_open,
        frame.eye_r_open,
        frame.mouth_open_y,
        frame.mouth_form,
        frame.jaw_open);

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
        &pluginName,
        &onLoad,
        &onUnload,
        &onFrame
    };
    return &api;
}
