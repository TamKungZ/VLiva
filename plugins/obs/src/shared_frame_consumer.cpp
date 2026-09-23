// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 TamKungZ_

#include "vliva/capture/shared_frame_consumer.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace vliva::capture {

SharedFrameConsumer::SharedFrameConsumer()
    : m_fd(-1)
    , m_mapped(nullptr)
    , m_mappedSize(0)
    , m_sessionName()
    , m_header(nullptr)
    , m_maxWidth(0)
    , m_maxHeight(0)
    , m_stride(0)
{
}

SharedFrameConsumer::~SharedFrameConsumer()
{
    close();
}

bool SharedFrameConsumer::open(const std::string& sessionName)
{
    close();

    m_sessionName = normalizeSharedMemoryName(sessionName);
    m_fd = ::shm_open(m_sessionName.c_str(), O_RDONLY, 0666);
    if (m_fd < 0) {
        resetState();
        return false;
    }

    struct stat st {};
    if (::fstat(m_fd, &st) != 0 || st.st_size <= 0) {
        close();
        return false;
    }

    m_mappedSize = static_cast<size_t>(st.st_size);
    void* mapped = ::mmap(nullptr, m_mappedSize, PROT_READ, MAP_SHARED, m_fd, 0);
    if (mapped == MAP_FAILED) {
        close();
        return false;
    }

    m_mapped = mapped;
    m_header = static_cast<const SharedFrameHeader*>(m_mapped);

    if (m_header->magic != kSharedFrameMagic ||
        m_header->version != kSharedFrameVersion ||
        m_header->format != kSharedFrameFormatRgba8) {
        std::cerr << "[SharedFrameConsumer] Invalid shared frame header for " << m_sessionName << std::endl;
        close();
        return false;
    }

    m_maxWidth = m_header->maxWidth;
    m_maxHeight = m_header->maxHeight;
    m_stride = m_header->stride;

    const size_t minimumSize = calculateSharedRegionSize(m_stride, m_maxHeight);
    if (m_mappedSize < minimumSize || m_maxWidth == 0 || m_maxHeight == 0 || m_stride < (m_maxWidth * 4u)) {
        std::cerr << "[SharedFrameConsumer] Shared frame mapping is too small or invalid for " << m_sessionName << std::endl;
        close();
        return false;
    }

    return true;
}

void SharedFrameConsumer::close()
{
    if (m_mapped && m_mapped != MAP_FAILED) {
        ::munmap(m_mapped, m_mappedSize);
        m_mapped = nullptr;
    }

    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }

    resetState();
}

bool SharedFrameConsumer::isOpen() const
{
    return (m_fd >= 0) && (m_mapped != nullptr) && (m_header != nullptr);
}

const std::string& SharedFrameConsumer::sessionName() const
{
    return m_sessionName;
}

bool SharedFrameConsumer::readLatest(std::vector<unsigned char>& rgbaOut,
                                     uint32_t& width,
                                     uint32_t& height,
                                     uint64_t& frameId)
{
    if (!isOpen()) {
        return false;
    }

    const uint32_t ready = __atomic_load_n(&m_header->ready, __ATOMIC_ACQUIRE);
    if (ready == 0) {
        return false;
    }

    for (int attempt = 0; attempt < 2; ++attempt) {
        const uint32_t activeA = __atomic_load_n(&m_header->activeIndex, __ATOMIC_ACQUIRE) & 1u;
        const uint64_t frameA = __atomic_load_n(&m_header->frameId, __ATOMIC_ACQUIRE);
        const uint32_t w = __atomic_load_n(&m_header->width, __ATOMIC_RELAXED);
        const uint32_t h = __atomic_load_n(&m_header->height, __ATOMIC_RELAXED);

        if (frameA == 0 || w == 0 || h == 0 || w > m_maxWidth || h > m_maxHeight) {
            return false;
        }

        const size_t rowBytes = static_cast<size_t>(w) * 4u;
        const size_t outBytes = rowBytes * static_cast<size_t>(h);
        rgbaOut.resize(outBytes);

        const unsigned char* src = bufferAt(activeA);
        if (!src) {
            return false;
        }

        for (uint32_t row = 0; row < h; ++row) {
            const unsigned char* srcLine = src + (static_cast<size_t>(row) * m_stride);
            unsigned char* dstLine = rgbaOut.data() + (rowBytes * static_cast<size_t>(row));
            std::memcpy(dstLine, srcLine, rowBytes);
        }

        const uint32_t activeB = __atomic_load_n(&m_header->activeIndex, __ATOMIC_ACQUIRE) & 1u;
        const uint64_t frameB = __atomic_load_n(&m_header->frameId, __ATOMIC_ACQUIRE);
        if (activeA == activeB && frameA == frameB) {
            width = w;
            height = h;
            frameId = frameB;
            return true;
        }
    }

    return false;
}

void SharedFrameConsumer::resetState()
{
    m_mapped = nullptr;
    m_mappedSize = 0;
    m_header = nullptr;
    m_maxWidth = 0;
    m_maxHeight = 0;
    m_stride = 0;
}

const unsigned char* SharedFrameConsumer::bufferAt(uint32_t index) const
{
    if (!m_mapped || m_mapped == MAP_FAILED) {
        return nullptr;
    }

    const unsigned char* base = static_cast<const unsigned char*>(m_mapped) + sizeof(SharedFrameHeader);
    const size_t frameBytes = static_cast<size_t>(m_stride) * static_cast<size_t>(m_maxHeight);
    return base + (static_cast<size_t>(index & 1u) * frameBytes);
}

} // namespace vliva::capture
