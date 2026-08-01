#include <includes.hpp>
#include <data/gsc_data_t7.hpp>
#include <utils/cli_options.hpp>
#include <utils/utils.hpp>
// crc_cpp stuff
#undef small
#include <crc_cpp.h>

namespace {
    struct GscObfOptions {
        bool printHelp{};
        bool printData{};
        bool noDebugKill{};
        const char* logLevel{};
        const char* output{ "output" };
    } opt;

    struct NameLocated {
        uint64_t name_space;
        uint64_t name;
        uint64_t script{};
    };
    struct NameLocatedHash {
        size_t operator()(const NameLocated& k) const {
            return k.name_space ^ std::rotl(k.name, 21) ^ std::rotl(k.script, 42);
        }
    };
    struct NameLocatedEquals {
        bool operator()(const NameLocated& a, const NameLocated& b) const {
            return a.name == b.name && a.name_space == b.name_space && a.script == b.script;
        }
    };

    uint16_t SafeCreateLocalVariables_Opcodes[]{
        0x1d2,  0x299,  0x2f7,  0x336,  0x3fa,  0x45a,  0x49f,  0x5b3,  0x6a7,  0x7b1,  0x80c,  0x83e,  0x958,  0xa9a,
        0xaa1,  0xb17,  0xc67,  0xcd7,  0xcef,  0xe21,  0x1210, 0x1224, 0x1260, 0x12e2, 0x1386, 0x13c3, 0x13ed, 0x15a1,
        0x1654, 0x1897, 0x18a4, 0x195f, 0x1995, 0x1b60, 0x1b6b, 0x1c6d, 0x1e3d, 0x1f65, 0x1f6a, 0x1fa9, 0x209e, 0x213b,
        0x221b, 0x23d6, 0x252d, 0x2634, 0x26eb, 0x272d, 0x27c6, 0x280c, 0x2836, 0x2898, 0x29db, 0x2a67, 0x2b13, 0x2b42,
        0x2c8f, 0x2d02, 0x2d24, 0x2dc0, 0x2de0, 0x2e88, 0x2ebb, 0x2ecf, 0x3029, 0x3196, 0x3227, 0x3243, 0x327c, 0x3308,
        0x339a, 0x33e5, 0x366f, 0x368c, 0x3742, 0x3886, 0x38ab, 0x399f, 0x39c4, 0x3af6, 0x3bb4, 0x3e7e, 0x3ebc
    };

    uint32_t ComputeCRC32(void* data, size_t len) {
        byte* b{ (byte*)data };

        crc_cpp::crc32 crc{};

        for (size_t i = 0; i < len; i++) {
            crc.update(b[i]);
        }
        return crc.final();
    }

    const char* ExtractTmp(uint32_t hash) {
        return utils::va("hash_%x", hash); // todo: use db
    }

    int HandleFile(std::filesystem::path in, std::filesystem::path out) {
        std::vector<byte> buffer;

        LOG_INFO("Reading {} to {}...", in.string(), out.string());

        void* script;
        size_t scriptLen;

        if (!utils::ReadFileAlign(in, buffer, script, scriptLen)) {
            LOG_ERROR("Can't read {}", in.string());
            return -1;
        }

        void* scriptEnd{ (byte*)script + scriptLen };

        if (scriptLen < sizeof(data::gsc::T7GSCOBJ)) {
            LOG_ERROR("Can't read {}: Invalid size", in.string());
            return -1;
        }

        uint64_t magic{ *(uint64_t*)script };

        if (magic != data::gsc::VMI_T7_1C) {
            LOG_ERROR("Can't read {}: Invalid magic 0x{:x}", in.string(), magic);
            return -1;
        }

        data::gsc::T7GSCOBJ& header{ *(data::gsc::T7GSCOBJ*)script };

        if (opt.printData) {
            LOG_INFO("source_crc 0x{:x}", (uint32_t)header.source_crc);
            LOG_INFO("include_offset 0x{:x}", (uint32_t)header.include_offset);
            LOG_INFO("animtree_offset 0x{:x}", (uint32_t)header.animtree_offset);
            LOG_INFO("cseg_offset 0x{:x}", (uint32_t)header.cseg_offset);
            LOG_INFO("string_offset 0x{:x}", (uint32_t)header.string_offset);
            LOG_INFO("devblock_string_offset 0x{:x}", (uint32_t)header.devblock_string_offset);
            LOG_INFO("export_offset 0x{:x}", (uint32_t)header.export_offset);
            LOG_INFO("import_offset 0x{:x}", (uint32_t)header.import_offset);
            LOG_INFO("fixup_offsets 0x{:x}", (uint32_t)header.fixup_offsets);
            LOG_INFO("profile_offset 0x{:x}", (uint32_t)header.profile_offset);
            LOG_INFO("cseg_size 0x{:x}", (uint32_t)header.cseg_size);
            LOG_INFO("name_offset 0x{:x}", (uint32_t)header.name_offset);
            LOG_INFO("string_count 0x{:x}", (uint32_t)header.string_count);
            LOG_INFO("export_count 0x{:x}", (uint32_t)header.export_count);
            LOG_INFO("import_count 0x{:x}", (uint32_t)header.import_count);
            LOG_INFO("fixup_count 0x{:x}", (uint32_t)header.fixup_count);
            LOG_INFO("profile_count 0x{:x}", (uint32_t)header.profile_count);
            LOG_INFO("devblock_string_count 0x{:x}", (uint32_t)header.devblock_string_count);
            LOG_INFO("include_count 0x{:x}", (uint32_t)header.include_count);
            LOG_INFO("animtree_count 0x{:x}", (uint32_t)header.animtree_count);
            LOG_INFO("flags 0x{:x}", (uint32_t)header.flags);
        }

        if (header.export_offset + sizeof(data::gsc::T7GSCExport) * header.export_count > scriptLen) {
            LOG_ERROR("Invalid exports size");
            return -1;
        }

        data::gsc::T7GSCExport* exports{ (data::gsc::T7GSCExport*)&header.magic[header.export_offset] };

        std::unordered_map<NameLocated, std::vector<data::gsc::T7GSCExport*>, NameLocatedHash, NameLocatedEquals>
            exportsMap{};
        std::unordered_map<NameLocated, std::vector<data::gsc::T7GSCImport*>, NameLocatedHash, NameLocatedEquals>
            importsMap{};

        for (size_t i = 0; i < header.export_count; i++) {
            data::gsc::T7GSCExport& exp{ exports[i] };

            exportsMap[NameLocated{
                           .name_space = exp.name_space,
                           .name = exp.name,
                       }]
                .emplace_back(&exp);

            if (utils::AlignedC<uint16_t>(exp.address) + 4 > scriptLen) { // the minimum is createparam + end (4 bytes)
                LOG_ERROR("Found invalid exports address @{}", i);
                return -1;
            }
            byte* bc{ &header.magic[exp.address] };

            // we can remove the checksum by computing only one byte, it kills cerberus
            exp.checksum = ComputeCRC32(bc, 1);

            bc = utils::Aligned<uint16_t>(bc);
            uint16_t createParams{ *(uint16_t*)bc };
            if (std::find(
                    std::begin(SafeCreateLocalVariables_Opcodes),
                    std::end(SafeCreateLocalVariables_Opcodes),
                    createParams
                ) != std::end(SafeCreateLocalVariables_Opcodes)) {
                // we have variables, we can remove their names
                byte count{ bc[2] };

                if (count) {
                    bc = utils::Aligned<uint32_t>(bc + 3);
                    if (bc + count * sizeof(uint32_t) * 2 > scriptEnd) {
                        LOG_ERROR("Found invalid exports bytecode @{}", i);
                        return -1;
                    }

                    for (size_t j = 0; j < count; j++) {
                        uint32_t* varName{ (uint32_t*)bc };
                        bc += 4;
                        byte flags{ *bc };
                        bc += 4; // 3 padding bytes

                        if ((flags & 2) == 0) {          // !varargs
                            *(uint32_t*)varName = 1 + j; // var_1, var_2, etc.
                        }
                    }
                }
            }
        }
        LOG_TRACE("Patched {} exports", header.export_count);

        if (header.import_offset + sizeof(data::gsc::T7GSCImport) > scriptLen) {
            LOG_ERROR("Invalid imports size");
            return -1;
        }

        data::gsc::T7GSCImport* imports{ (data::gsc::T7GSCImport*)&header.magic[header.import_offset] };

        for (size_t i = 0; i < header.import_count; i++) {
            data::gsc::T7GSCImport& imp{ *imports };
            imports = (data::gsc::T7GSCImport*)((uint32_t*)&imports[1] + imports->num_address);
            if (imports > scriptEnd) {
                LOG_ERROR("Found invalid imports @{}", i);
                return -1;
            }

            importsMap[NameLocated{
                           .name_space = imp.import_namespace,
                           .name = imp.name,
                       }]
                .emplace_back(&imp);

            if (!opt.noDebugKill) {
                if ((imp.flags & data::gsc::T7GIF_DEV_CALL) != 0) {
                    // kill dev block call information
                    imp.name = 0xdead;
                    imp.import_namespace = 0xdead;
                }
            }
        }

        uint32_t privateCount{};
        for (auto& [nl, exps] : exportsMap) {
            if (exps.size() != 1) {
                LOG_WARNING("export {}::{} defined multiple times", ExtractTmp(nl.name_space), ExtractTmp(nl.name));
                continue;
            }

            data::gsc::T7GSCExport& exp{ *exps[0] };
            if ((exp.flags & data::gsc::T7GEF_PRIVATE) != 0) {
                // private export, we can remove its name

                exp.name = ++privateCount;

                auto iit{ importsMap.find(nl) };
                if (iit == importsMap.end()) {
                    continue;
                }

                for (data::gsc::T7GSCImport* imp : iit->second) {
                    imp->name = exp.name;
                }
            }
        }

        // write back the file

        if (!utils::WriteFile(out, script, scriptLen)) {
            LOG_ERROR("Can't write {}", out.string());
            return -1;
        }
        LOG_DEBUG("Write back {}", out.string());

        return 0;
    }

} // namespace

int main(int argc, const char* argv[]) {
    utils::logs::setbasiclog(true);
    utils::cli_options::CliOptions opts{};

    opts.addOption(&opt.printHelp, "show help", "--help", "", "-h");
    opts.addOption(&opt.printData, "print script header", "--header", "", "-H");
    opts.addOption(&opt.noDebugKill, "no debug data kill", "--no-debug", "", "-d");
    opts.addOption(&opt.logLevel, "log level (e,w,i,d,p)", "--logs", " (level)", "-l");
    opts.addOption(&opt.logLevel, "log level (e,w,i,d,p)", "--logs", " (level)", "-l");
    opts.addOption(&opt.output, "output dir (default: output)", "--output", " (path)", "-o");

    if (!opts.ComputeOptions(1, argc, argv) || opt.printHelp || opts.NotEnoughParam(1)) {
        LOG_INFO("usage: {} (file)", argv[0]);
        opts.PrintOptions();
        return opt.printHelp ? 0 : -1;
    }

    if (opt.logLevel) {
        utils::logs::setbasiclog(false);
        if (opt.logLevel[0] && opt.logLevel[1]) {
            LOG_ERROR("Invalid log level: {}", opt.logLevel);
            return -1;
        }
        switch (*opt.logLevel) {
        case 'e':
        case 'E':
            utils::logs::setlevel(utils::logs::LVL_ERROR);
            break;
        case 'w':
        case 'W':
            utils::logs::setlevel(utils::logs::LVL_WARNING);
            break;
        case 'i':
        case 'I':
            utils::logs::setlevel(utils::logs::LVL_INFO);
            break;
        case 'd':
        case 'D':
            utils::logs::setlevel(utils::logs::LVL_DEBUG);
            break;
        case 't':
        case 'T':
            utils::logs::setlevel(utils::logs::LVL_TRACE);
            break;
        case 'p':
        case 'P':
            utils::logs::setlevel(utils::logs::LVL_TRACE_PATH);
            break;
        default:
            LOG_ERROR("Invalid log level: {}", opt.logLevel);
            return -1;
        }
    }

    std::filesystem::path outDir{ opt.output };
    std::filesystem::create_directories(outDir);
    int r{};

    for (size_t i = 0; i < opts.ParamsCount(); i++) {
        std::vector<std::filesystem::path> paths{};
        std::filesystem::path parent{ opts[i] };
        if (std::filesystem::is_regular_file(parent)) {
            paths.emplace_back(parent.filename());
            parent = std::filesystem::absolute(parent).parent_path();
        } else {
            utils::GetFileRecurseExt(parent, paths, ".gscc\0.cscc\0", true);

            if (paths.empty()) {
                LOG_WARNING("Can't find compiled gsc files (.gscc/.cscc) in {}", opts[i]);
                continue;
            }
        }

        for (const std::filesystem::path& path : paths) {
            const std::filesystem::path inFile{ parent / path };
            std::filesystem::path outFile{ outDir / path };
            int hr{ HandleFile(inFile, outFile) };
            if (hr) {
                r = hr;
            }
        }
    }

    return r;
}