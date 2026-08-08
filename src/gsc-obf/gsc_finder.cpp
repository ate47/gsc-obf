#include <includes.hpp>
#include <gsc_finder.hpp>
#include <data/gsc_data_t7.hpp>
#include <utils/bytebuffer.hpp>

namespace gscobf::finder {

    constexpr uint64_t FF_DATA_NEXT = 0xFFFFFFFFFFFFFFFF;

    struct ScriptParseTree {
        int64_t name;
        int32_t len;
        int32_t pad;
        int64_t buffer;
    };

    std::vector<GscObject> FindGscInBuffer(byte* buffer, size_t bufferSize) {
        utils::bytebuffer::ByteBuffer decReader{ buffer, bufferSize };
        std::vector<GscObject> results{};

        ScriptParseTree sptSearch{};
        ScriptParseTree sptMask{};
        memset(&sptMask, 0xff, sizeof(sptMask));
        sptMask.len = 0;
        sptSearch.buffer = 0xFFFFFFFFFFFFFFFF;
        sptSearch.name = 0xFFFFFFFFFFFFFFFF;

        size_t off{};
        while (true) {
            if ((off = decReader.FindMasked(&sptSearch, &sptMask, sizeof(sptSearch))) == std::string::npos) {
                break;
            }
            decReader.Goto(off);
            off++;
            ScriptParseTree& spt{ *decReader.ReadPtr<ScriptParseTree>() };

            char* name{ decReader.ReadString() };

            std::string_view sw{ name };
            if (spt.len < 8 ||
                !(sw.ends_with(".gsc") || sw.ends_with(".csc") || sw.ends_with(".gsh") || sw.ends_with(".csh"))) {
                continue; // not a candidate
            }
            size_t fileSize{ (size_t)spt.len + 1 };
            if (!decReader.CanRead(fileSize)) {
                continue; // bad size
            }
            byte* data{ decReader.ReadPtr<byte>(fileSize) };
            uint64_t magic{ *(uint64_t*)data };

            if (magic != data::gsc::VMI_T7_1C) {
                LOG_DEBUG("found invalid magic: 0x{:x}", magic);
                continue; // bad magic
            }

            results.emplace_back(GscObject{ .name = name, .fileSize = fileSize, .obj = data });
        }

        return results;
    }
} // namespace gscobf::finder