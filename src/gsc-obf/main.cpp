#include <includes.hpp>
#include <data/gsc_data_t7.hpp>
#include <utils/cli_options.hpp>
#include <utils/utils.hpp>

namespace {
    struct GscObfOptions {
        bool printHelp{};
        bool printData{};
        const char* logLevel{};
        const char* output{ "output" };
    } opt;

    int HandleFile(std::filesystem::path in, std::filesystem::path out) {
        std::vector<byte> buffer;

        LOG_INFO("Reading {} to {}...", in.string(), out.string());

        void* script;
        size_t scriptLen;

        if (!utils::ReadFileAlign(in, buffer, script, scriptLen)) {
            LOG_ERROR("Can't read {}", in.string());
            return -1;
        }

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

        // 

        return 0;
    }

} // namespace

int main(int argc, const char* argv[]) {
    utils::logs::setbasiclog(true);
    utils::cli_options::CliOptions opts{};

    opts.addOption(&opt.printHelp, "show help", "--help", "", "-h");
    opts.addOption(&opt.printData, "print script header", "--header", "", "-H");
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