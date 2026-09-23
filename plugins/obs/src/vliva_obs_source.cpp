// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 TamKungZ_

#include <obs-module.h>

#include "vliva/capture/shared_frame_consumer.hpp"

#include <graphics/graphics.h>
#include <util/platform.h>

#include <cstring>
#include <string>
#include <vector>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("vliva_obs_source", "en-US")

MODULE_EXPORT const char* obs_module_description(void)
{
    return "VLiva shared-memory source";
}
namespace {

struct VlivaObsSource {
    obs_source_t* source = nullptr;
    gs_texture_t* texture = nullptr;

    vliva::capture::SharedFrameConsumer consumer;
    std::vector<unsigned char> frameBuffer;

    std::string sessionName = "vliva_obs_main";
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t lastFrameId = 0;

    float reconnectTimer = 0.0f;
};

const char* vlivaObsSourceName(void*)
{
    return "VLiva Capture";
}

void vlivaObsReconnect(VlivaObsSource* ctx)
{
    if (!ctx) {
        return;
    }

    ctx->consumer.close();
    ctx->consumer.open(ctx->sessionName);
    ctx->reconnectTimer = 0.0f;
}

void vlivaObsEnsureTexture(VlivaObsSource* ctx, uint32_t width, uint32_t height)
{
    if (!ctx) {
        return;
    }

    if (ctx->texture && ctx->width == width && ctx->height == height) {
        return;
    }

    obs_enter_graphics();
    if (ctx->texture) {
        gs_texture_destroy(ctx->texture);
        ctx->texture = nullptr;
    }
    ctx->texture = gs_texture_create(width, height, GS_RGBA, 1, nullptr, GS_DYNAMIC);
    obs_leave_graphics();

    ctx->width = width;
    ctx->height = height;
}

void* vlivaObsCreate(obs_data_t* settings, obs_source_t* source)
{
    auto* ctx = new VlivaObsSource();
    ctx->source = source;

    const char* rawSession = obs_data_get_string(settings, "session_name");
    if (rawSession && rawSession[0] != '\0') {
        ctx->sessionName = rawSession;
    }

    vlivaObsReconnect(ctx);
    return ctx;
}

void vlivaObsDestroy(void* data)
{
    auto* ctx = static_cast<VlivaObsSource*>(data);
    if (!ctx) {
        return;
    }

    obs_enter_graphics();
    if (ctx->texture) {
        gs_texture_destroy(ctx->texture);
        ctx->texture = nullptr;
    }
    obs_leave_graphics();

    delete ctx;
}

void vlivaObsUpdate(void* data, obs_data_t* settings)
{
    auto* ctx = static_cast<VlivaObsSource*>(data);
    if (!ctx) {
        return;
    }

    const char* rawSession = obs_data_get_string(settings, "session_name");
    std::string newSession = (rawSession && rawSession[0] != '\0') ? std::string(rawSession) : std::string("vliva_obs_main");
    if (newSession != ctx->sessionName) {
        ctx->sessionName = newSession;
        ctx->lastFrameId = 0;
        vlivaObsReconnect(ctx);
    }
}

void vlivaObsDefaults(obs_data_t* settings)
{
    obs_data_set_default_string(settings, "session_name", "vliva_obs_main");
}

obs_properties_t* vlivaObsProperties(void*)
{
    obs_properties_t* props = obs_properties_create();
    obs_properties_add_text(props, "session_name", "Shared Memory Session", OBS_TEXT_DEFAULT);
    return props;
}

void vlivaObsTick(void* data, float seconds)
{
    auto* ctx = static_cast<VlivaObsSource*>(data);
    if (!ctx) {
        return;
    }

    ctx->reconnectTimer += seconds;
    if (!ctx->consumer.isOpen()) {
        if (ctx->reconnectTimer >= 1.0f) {
            vlivaObsReconnect(ctx);
        }
        return;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t frameId = 0;
    if (!ctx->consumer.readLatest(ctx->frameBuffer, width, height, frameId)) {
        // Producer may have rotated shared memory while this consumer still
        // maps an old region. Periodically force reconnect to rebind.
        if (ctx->reconnectTimer >= 1.0f) {
            vlivaObsReconnect(ctx);
        }
        return;
    }

    if (frameId == 0 || width == 0 || height == 0) {
        if (ctx->reconnectTimer >= 1.0f) {
            vlivaObsReconnect(ctx);
        }
        return;
    }

    if (frameId == ctx->lastFrameId) {
        // No fresh frame for a while likely means stale mapping.
        if (ctx->reconnectTimer >= 1.0f) {
            vlivaObsReconnect(ctx);
        }
        return;
    }

    ctx->lastFrameId = frameId;
    ctx->reconnectTimer = 0.0f;

    vlivaObsEnsureTexture(ctx, width, height);
    if (!ctx->texture) {
        return;
    }

    obs_enter_graphics();
    gs_texture_set_image(ctx->texture, ctx->frameBuffer.data(), width * 4u, false);
    obs_leave_graphics();
}

void vlivaObsRender(void* data, gs_effect_t*)
{
    auto* ctx = static_cast<VlivaObsSource*>(data);
    if (!ctx || !ctx->texture) {
        return;
    }

    // OBS may already have an active effect in this callback path.
    // Use the helper draw path to avoid nested gs_effect_loop warnings.
    obs_source_draw(ctx->texture, 0, 0, 0, 0, false);
}

uint32_t vlivaObsWidth(void* data)
{
    auto* ctx = static_cast<VlivaObsSource*>(data);
    return ctx ? ctx->width : 0;
}

uint32_t vlivaObsHeight(void* data)
{
    auto* ctx = static_cast<VlivaObsSource*>(data);
    return ctx ? ctx->height : 0;
}

} // namespace

bool obs_module_load(void)
{
    obs_source_info sourceInfo{};
    sourceInfo.id = "vliva_capture_source";
    sourceInfo.type = OBS_SOURCE_TYPE_INPUT;
    sourceInfo.output_flags = OBS_SOURCE_VIDEO;
    sourceInfo.get_name = vlivaObsSourceName;
    sourceInfo.create = vlivaObsCreate;
    sourceInfo.destroy = vlivaObsDestroy;
    sourceInfo.update = vlivaObsUpdate;
    sourceInfo.get_defaults = vlivaObsDefaults;
    sourceInfo.get_properties = vlivaObsProperties;
    sourceInfo.video_tick = vlivaObsTick;
    sourceInfo.video_render = vlivaObsRender;
    sourceInfo.get_width = vlivaObsWidth;
    sourceInfo.get_height = vlivaObsHeight;

    obs_register_source(&sourceInfo);
    blog(LOG_INFO, "VLiva OBS source plugin loaded");
    return true;
}
