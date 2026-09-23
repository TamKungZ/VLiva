// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 TamKungZ_

#pragma once

#include "vliva/capture/shared_frame_protocol.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace vliva::capture {

class SharedFrameConsumer {
public:
    SharedFrameConsumer();
    ~SharedFrameConsumer();

    bool open(const std::string& sessionName);
    void close();

    bool isOpen() const;
    const std::string& sessionName() const;

    bool readLatest(std::vector<unsigned char>& rgbaOut,
                    uint32_t& width,
                    uint32_t& height,
                    uint64_t& frameId);

private:
    void resetState();
    const unsigned char* bufferAt(uint32_t index) const;

    int m_fd;
    void* m_mapped;
    size_t m_mappedSize;

    std::string m_sessionName;
    const SharedFrameHeader* m_header;

    uint32_t m_maxWidth;
    uint32_t m_maxHeight;
    uint32_t m_stride;
};

} // namespace vliva::capture
