#include "vliva/plugin_api.h"

namespace {

const VlivaHostApi* g_host = nullptr;

const char* pluginName()
{
    return "Example Plugin";
}

void onLoad(const VlivaHostApi* host)
{
    g_host = host;
    if (g_host && g_host->log)
    {
        g_host->log("Example Plugin loaded");
    }
}

void onUnload()
{
    if (g_host && g_host->log)
    {
        g_host->log("Example Plugin unloaded");
    }
}

void onFrame(double /*timeSeconds*/, float /*deltaSeconds*/)
{
    // Keep this no-op as a safe starter plugin.
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
