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
        bool noTrampoline{};
        bool recomputeCRC{};
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

    uint16_t Jump_Opcodes[]{ 0x22,   0x3c,   0x7d,   0x140,  0x14b,  0x25f,  0x4b9,  0x4fa,  0x541,  0x5ac,  0x652,
                             0x6c3,  0x6f9,  0x840,  0x89f,  0x945,  0xa95,  0xb1c,  0xb3f,  0xba0,  0xbc9,  0xc3f,
                             0xc51,  0xc5f,  0xdbf,  0xde4,  0xdee,  0xe51,  0xeb9,  0xf26,  0xf82,  0xfd9,  0x10a9,
                             0x11c8, 0x1222, 0x1259, 0x12a9, 0x1307, 0x131b, 0x1341, 0x1365, 0x1400, 0x1436, 0x1452,
                             0x14dc, 0x1735, 0x17a7, 0x184d, 0x18b2, 0x18b3, 0x1b0a, 0x1c22, 0x1c56, 0x1ca5, 0x1cf3,
                             0x1dc9, 0x1dd2, 0x1e21, 0x1e99, 0x1ebe, 0x1ebf, 0x1ef8, 0x1f22, 0x1f45, 0x201c, 0x2127,
                             0x217b, 0x21bf, 0x21e9, 0x23b9, 0x25a3, 0x25d8, 0x268e, 0x273b, 0x27f0, 0x28ed, 0x299d,
                             0x2b1b, 0x2b92, 0x2bb7, 0x2be6, 0x2bed, 0x2ce2, 0x2d32, 0x2e06, 0x2eea, 0x2f4b, 0x2fd8,
                             0x2fe2, 0x3037, 0x30d3, 0x3122, 0x315b, 0x3179, 0x31f8, 0x3284, 0x32c1, 0x3304, 0x344f,
                             0x34e2, 0x353b, 0x3596, 0x36b4, 0x36bd, 0x37c4, 0x384a, 0x3906, 0x3933, 0x398c, 0x39c1,
                             0x3a48, 0x3a64, 0x3ae0, 0x3b79, 0x3b85, 0x3c54, 0x3df9, 0x3e03, 0x3e5e, 0x3ea1, 0x3ee7 };

    uint16_t GetRandomOpCodes(std::span<uint16_t> opcodes) { return opcodes[rand() % opcodes.size()]; }

    uint32_t ComputeCRC32(void* data, size_t len) {
        byte* b{ (byte*)data };

        crc_cpp::crc32 crc{};

        for (size_t i = 0; i < len; i++) {
            crc.update(b[i]);
        }
        return crc.final();
    }

    size_t FindExportSizeByCrc(byte* bytecode, void* end, uint32_t crc) {
        for (size_t len = 1; bytecode + len <= end; len++) {
            if (ComputeCRC32(bytecode, len) == crc) {
                return len;
            }
        }
        return 0;
    }

    const char* ExtractTmp(uint32_t hash) {
        return utils::va("hash_%x", hash); // todo: use db
    }

    int HandleFFFile(std::filesystem::path in, std::filesystem::path out) {
        std::vector<byte> buffer;

        if (!utils::ReadFile(in, buffer)) {
            LOG_ERROR("Can't read {}", in.string());
            return -1;
        }

        LOG_ERROR("Not yet implemented");

        return -1;
    }

    int HandleGSCFile(std::filesystem::path in, std::filesystem::path out) {
        std::vector<byte> buffer;
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

        struct GscExportData {
            data::gsc::T7GSCExport* ref;
            byte* bc{};
            size_t len{};
        };

        std::unordered_map<NameLocated, std::vector<GscExportData>, NameLocatedHash, NameLocatedEquals> exportsMap{};
        std::unordered_map<NameLocated, std::vector<data::gsc::T7GSCImport*>, NameLocatedHash, NameLocatedEquals>
            importsMap{};
        constexpr size_t TRAMPOLINE_SIZE = 4; // Jmp[2] Delta[2]
        std::vector<uint32_t> trampolineFreeLocations{};
        uint32_t junkLocation{}; // well to redirect all the offset

        for (size_t i = 0; i < header.export_count; i++) {
            data::gsc::T7GSCExport& exp{ exports[i] };

            if (utils::AlignedC<uint16_t>(exp.address) + 4 > scriptLen) { // the minimum is createparam + end (4 bytes)
                LOG_ERROR("Found invalid exports address @{}", i);
                return -1;
            }
            byte* bc{ &header.magic[exp.address] };
            size_t len{ FindExportSizeByCrc(bc, scriptEnd, exp.checksum) };
            if (opt.recomputeCRC && !len) {
                LOG_WARNING("Can't compute crc size for {}::{}", ExtractTmp(exp.name_space), ExtractTmp(exp.name));
            }

            exportsMap[NameLocated{
                           .name_space = exp.name_space,
                           .name = exp.name,
                       }]
                .emplace_back(GscExportData{ .ref = &exp, .bc = bc, .len = len });

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

        auto KillDevByteCodeOp =
            [&header, scriptEnd, &trampolineFreeLocations, &junkLocation](uint32_t& floc, size_t len, size_t delta) {
                if (opt.noDebugKill) {
                    return; // disabled
                }
                byte* bc{ utils::Aligned(&header.magic[floc + delta], len) };

                if (floc < 2 + delta || utils::Aligned<uint16_t>(bc + len) + 2 > scriptEnd) {
                    LOG_ERROR("Invalid dev op size");
                    return;
                }

                // the loc isn't used, so we can use it as a trampoline
                for (size_t i = 0; i < len / TRAMPOLINE_SIZE; i++) {
                    uint32_t rloc { (uint32_t)(&bc[TRAMPOLINE_SIZE * i] - &header.magic[0]) };
                    if (!junkLocation) {
                        junkLocation = rloc;
                    } else if (junkLocation != rloc) {
                        trampolineFreeLocations.emplace_back(rloc);
                    }
                }

                // we leave a small gift in the dev block

                // due to the alignment, we maybe kill (part of) the opcode
                *(uint16_t*)&header.magic[floc - 2] = 0xdead;
                // kill the opcode after the string (from what I know, a dev block can't end with a get string or a
                // function call without dectop)
                *(uint16_t*)utils::Aligned<uint16_t>(&bc[len]) = 0xbeef;

                if (junkLocation) {
                    floc = junkLocation;
                }
            };

        if (header.import_offset + sizeof(data::gsc::T7GSCImport) > scriptLen) {
            LOG_ERROR("Invalid imports size");
            return -1;
        }

        data::gsc::T7GSCImport* imports{ (data::gsc::T7GSCImport*)&header.magic[header.import_offset] };

        for (size_t i = 0; i < header.import_count; i++) {
            data::gsc::T7GSCImport& imp{ *imports };
            uint32_t* offsets{ (uint32_t*)&imports[1] };
            imports = (data::gsc::T7GSCImport*)&offsets[imports->num_address];
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
                    imp.name = 0xa7ee953; // assert
                    imp.param_count = 1;

                    size_t delta = (imp.flags & data::gsc::T7GIF_FUNC_METHOD) != 0 ? 1 : 0;
                    for (size_t j = 0; j < imp.num_address; j++) {
                        KillDevByteCodeOp(offsets[j], 8, delta);
                    }
                }
            }
        }

        uint32_t privateCount{};
        for (auto& [nl, exps] : exportsMap) {
            if (exps.size() != 1) {
                LOG_WARNING("export {}::{} defined multiple times", ExtractTmp(nl.name_space), ExtractTmp(nl.name));
                continue;
            }

            data::gsc::T7GSCExport& exp{ *exps[0].ref };
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

        if (!opt.noDebugKill) {

            if (header.devblock_string_offset + sizeof(data::gsc::T7GSCString) > scriptLen) {
                LOG_ERROR("Invalid strings size");
                return -1;
            }

            data::gsc::T7GSCString* strings{ (data::gsc::T7GSCString*)&header.magic[header.devblock_string_offset] };

            for (size_t i = 0; i < header.devblock_string_count; i++) {
                data::gsc::T7GSCString& str{ *strings };
                uint32_t* offsets{ (uint32_t*)&strings[1] };
                strings = (data::gsc::T7GSCString*)&offsets[strings->num_address];
                if (strings > scriptEnd) {
                    LOG_ERROR("Found invalid strings @{}", i);
                    return -1;
                }

                for (size_t j = 0; j < str.num_address; j++) {
                    KillDevByteCodeOp(offsets[j], 4, 0);
                }

                // kill dev block string information
                std::memset(offsets, 0, sizeof(*offsets) * str.num_address);
                std::memset(&str, 0, sizeof(str));
            }

            header.devblock_string_offset = 0;
            header.devblock_string_count = 0;
        }

        if (!opt.noTrampoline) {
            std::sort(trampolineFreeLocations.begin(), trampolineFreeLocations.end(), [](uint32_t a, uint32_t b) {
                return a < b;
            });

            for (size_t i = 0; i < header.export_count; i++) {
                data::gsc::T7GSCExport& exp{ exports[i] };

                if (trampolineFreeLocations.empty()) {
                    LOG_DEBUG("no enough trampolines");
                    break; // no more locations
                }

                int64_t loc{};
                int64_t dest{ (int64_t)exp.address };

                for (auto it = trampolineFreeLocations.begin(); it != trampolineFreeLocations.end(); it++) {
                    int64_t origin{ (int64_t)*it };
                    int64_t delta{ dest - (origin + 4) }; // a jump delta starts from the end of the op data
                    if (delta > INT16_MAX || delta < INT16_MIN) {
                        continue; // a jump delta is a signed 16 bits number, if we are bigger than that, we can't use this
                                  // trampoline
                    }

                    loc = origin;
                    trampolineFreeLocations.erase(it);
                    break;
                }

                if (!loc) {
                    LOG_DEBUG("no close trampoline for {}::{}", ExtractTmp(exp.name_space), ExtractTmp(exp.name));
                    continue; // no close location for this export
                }

                // if we are not in a dev op, we can use the devblock jump
                uint16_t jmpOpcode{ GetRandomOpCodes(Jump_Opcodes) };
                int16_t delta{ (int16_t)(dest - (loc + 4)) };

                *(uint16_t*)&header.magic[loc] = jmpOpcode;
                *(int16_t*)&header.magic[loc + 2] = delta;
                exp.address = (uint32_t)loc;

                LOG_TRACE(
                    "create trampoline {}::{} at 0x{:x} -> 0x{:x} ({}0x{:x})",
                    ExtractTmp(exp.name_space),
                    ExtractTmp(exp.name),
                    loc,
                    dest,
                    (delta < 0 ? "-" : ""),
                    (delta < 0 ? -delta : delta)
                );
            }
        }

        for (auto& [nl, exps] : exportsMap) {
            for (GscExportData& data : exps) {
                if (!opt.recomputeCRC) {
                    // we can remove the checksum by computing only one byte, it kills cerberus
                    data.ref->checksum = ComputeCRC32(data.bc, 1);
                } else if (data.len) {
                    // we recompute the export checksum, like that cerberus can accept the data
                    data.ref->checksum = ComputeCRC32(data.bc, data.len);
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

    int HandleFile(std::filesystem::path in, std::filesystem::path out) {

        LOG_INFO("Reading {} to {}...", in.string(), out.string());
        std::filesystem::create_directories(out.parent_path());

        if (in.extension() == ".ff") {
            // convert fastfile
            return HandleFFFile(in, out);
        } else if (in.extension() == ".gscc" || in.extension() == ".cscc") {
            // convert gsc
            return HandleGSCFile(in, out);
        } else {
            LOG_ERROR("Invalid extension for file {}, only fastfile and compiled gsc files are accepted", in.string());
            return -1;
        }
    }

} // namespace

int main(int argc, const char* argv[]) {
    srand(time(NULL));
    utils::logs::setbasiclog(true);
    utils::cli_options::CliOptions opts{};

    opts.addOption(&opt.printHelp, "show help", "--help", "", "-h");
    opts.addOption(&opt.printData, "print script header", "--header", "", "-H");
    opts.addOption(&opt.noDebugKill, "no debug data kill", "--no-debug", "", "-d");
    opts.addOption(&opt.noTrampoline, "no trampoline build", "--no-trampoline", "", "-t");
    opts.addOption(&opt.recomputeCRC, "recompute export crc", "--export-crc-recomp", "", "-r");
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
    int r{};

    for (size_t i = 0; i < opts.ParamsCount(); i++) {
        std::vector<std::filesystem::path> paths{};
        std::filesystem::path parent{ opts[i] };
        if (std::filesystem::is_regular_file(parent)) {
            paths.emplace_back(parent.filename());
            parent = std::filesystem::absolute(parent).parent_path();
        } else {
            utils::GetFileRecurseExt(parent, paths, ".gscc\0.cscc\0.ff\0", true);

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