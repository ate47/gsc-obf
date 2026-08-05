#include <includes.hpp>
#include <fastfile.hpp>
#include <utils/utils.hpp>

namespace gscobf::fastfile {
    FastfileInfo DecompressFastfile(void* file, size_t fileSize) {
        if (fileSize < sizeof(data::fastfile::XFileBO3)) {
            throw std::runtime_error("Invalid fastfile size");
        }

        uint64_t magic{ *(uint64_t*)file };
        if (magic != data::fastfile::FF_T7_MAGIC) {
            throw std::runtime_error(std::format("Invalid magic 0x{:x}", magic));
        }

        FastfileInfo info{};
        info.header = *(data::fastfile::XFileBO3*)file;

        if (info.header.platform != data::fastfile::XFILE_PC) {
            throw std::runtime_error("Platform not yet supported");
        }
        if (info.header.encrypted) {
            throw std::runtime_error("Encrypted fastfile not yet supported");
        }
        if (info.header.compression >= data::fastfile::XFILE_BDELTA_UNCOMP &&
            info.header.compression <= data::fastfile::XFILE_BDELTA_LZMA) {
            throw std::runtime_error("Delta fastfile"); // wtf
        }

        byte* data{ (byte*)file + sizeof(info.header) };
        byte* dataEnd{ (byte*)file + fileSize };

        while (data < dataEnd) {
            // TODO: read blocks
            info.maxBlockSize = 0; //
            info.out = {};
        }

        return info;
    }
    void CompressFastfile(std::vector<byte>& out, FastfileInfo& info) {
        out.clear();
        // write header
        utils::WriteValue(out, &info.header, sizeof(info.header));

        // TODO: write blocks
    }
} // namespace gscobf::fastfile