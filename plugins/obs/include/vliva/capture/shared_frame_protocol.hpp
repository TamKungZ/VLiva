// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 TamKungZ_

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace vliva::capture {

constexpr uint32_t kSharedFrameMagic = 0x564C5641u;   // "VLVA"
constexpr uint32_t kSharedFrameVersion = 1u;
constexpr uint32_t kSharedFrameFormatRgba8 = 1u;
constexpr const char* kDefaultSharedSessionName = "/vliva_obs_main";

struct SharedFrameHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t maxWidth;
    uint32_t maxHeight;
    uint32_t stride;
    uint32_t format;
    uint32_t activeIndex;
    uint32_t ready;
    uint64_t frameId;
    uint64_t timestampNs;
    uint32_t width;
    uint32_t height;
    uint32_t reserved[8];
};

inline size_t calculateSharedRegionSize(uint32_t stride, uint32_t maxHeight)
{
    const size_t singleFrameBytes = static_cast<size_t>(stride) * static_cast<size_t>(maxHeight);
    return sizeof(SharedFrameHeader) + (singleFrameBytes * 2u);
}

inline std::string normalizeSharedMemoryName(const std::string& rawName)
{
    if (rawName.empty()) {
        return std::string(kDefaultSharedSessionName);
    }
    if (!rawName.empty() && rawName.front() == '/') {
        return rawName;
    }
    return std::string("/") + rawName;
}

} // namespace vliva::capture
