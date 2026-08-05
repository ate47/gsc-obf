#pragma once
#include <data/fastfile_data_t7.hpp>

namespace gscobf::fastfile {
    struct FastfileInfo {
        size_t maxBlockSize;
        data::fastfile::XFileBO3 header;
        std::vector<byte> out;
    };

    FastfileInfo DecompressFastfile(void* file, size_t fileSize);
    void CompressFastfile(std::vector<byte>& out, FastfileInfo& info);
}