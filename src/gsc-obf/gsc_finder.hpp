#pragma once
#include <gsc_finder.hpp>

namespace gscobf::finder {
    struct GscObject {
        const char* name;
        size_t fileSize;
        byte* obj;
    };

    std::vector<GscObject> FindGscInBuffer(byte* buffer, size_t bufferSize);
} // namespace gscobf::finder