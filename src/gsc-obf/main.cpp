#include <includes.hpp>
#include <data/gsc_data_t7.hpp>
#include <utils/cli_options.hpp>
#include <utils/utils.hpp>
#include <private_file.hpp>
#include <gsc_obfuscator.hpp>
#include <fastfile.hpp>
#include <version.hpp>

namespace {

    void HandleGscObject(gscobf::options::GscObfOptions& opt, byte* file, size_t fileSize) {
        gscobf::obfuscator::GscObfuscator obf{ opt, file, fileSize };
        obf.RunTasks();
    }

    void HandleFFFile(
        gscobf::options::GscObfOptions& opt, const std::filesystem::path& in, const std::filesystem::path& out
    ) {
        throw std::runtime_error("not yet implemented");
        std::vector<byte> buffer;

        if (!utils::ReadFile(in, buffer)) {
            throw std::runtime_error(std::format("Can't read {}", in.string()));
        }
        gscobf::fastfile::FastfileInfo info{ gscobf::fastfile::DecompressFastfile(buffer.data(), buffer.size()) };

        // todo: apply patches on data

        // reshape the fastfile code
        if (opt.fastfileBuilder) {
            sprintf_s(info.header.builder, "%s", opt.fastfileBuilder);
        }

        if (opt.fastfileCompression) {
            if (!std::strcmp(opt.fastfileCompression, "uncompressed")) {
                info.header.compression = data::fastfile::XFILE_UNCOMPRESSED;
            } else if (!std::strcmp(opt.fastfileCompression, "lz4")) {
                info.header.compression = data::fastfile::XFILE_LZ4;
            } else if (!std::strcmp(opt.fastfileCompression, "zlib")) {
                info.header.compression = data::fastfile::XFILE_ZLIB;
            } else {
                throw std::runtime_error(std::format("Invalid fastfile compression: {}", opt.fastfileCompression));
            }
        }

        gscobf::fastfile::CompressFastfile(buffer, info);

        // write back the file

        if (!utils::WriteFile(out, buffer)) {
            throw std::runtime_error(std::format("Can't write {}", out.string()));
        }
        LOG_DEBUG("Write back {}", out.string());
    }

    void HandleGSCFile(
        gscobf::options::GscObfOptions& opt, const std::filesystem::path& in, const std::filesystem::path& out
    ) {
        std::vector<byte> buffer;

        if (!utils::ReadFile(in, buffer)) {
            throw std::runtime_error(std::format("Can't read {}", in.string()));
        }

        HandleGscObject(opt, buffer.data(), buffer.size());

        // write back the file

        if (!utils::WriteFile(out, buffer)) {
            throw std::runtime_error(std::format("Can't write {}", out.string()));
        }
        LOG_DEBUG("Write back {}", out.string());
    }

    void
    HandleFile(gscobf::options::GscObfOptions& opt, const std::filesystem::path& in, const std::filesystem::path& out) {

        LOG_INFO("Reading {} to {}...", in.string(), out.string());
        std::filesystem::create_directories(out.parent_path());

        if (in.extension() == ".ff") {
            // convert fastfile
            HandleFFFile(opt, in, out);
        } else if (in.extension() == ".gscc" || in.extension() == ".cscc") {
            // convert gsc
            HandleGSCFile(opt, in, out);
        } else {
            throw std::runtime_error(
                std::format(
                    "Invalid extension for file {}, only fastfile and compiled gsc files are accepted",
                    in.string()
                )
            );
        }
    }

} // namespace

int main(int argc, const char* argv[]) {
    srand(time(NULL));
    utils::logs::setbasiclog(true);
    utils::cli_options::CliOptions opts{};

    gscobf::options::GscObfOptions opt;
    opts.addOption(&opt.printHelp, "show help", "--help", "", "-h");
    opts.addOption(&opt.printData, "print script header", "--header", "", "-H");
    opts.addOption(&opt.noDebugKill, "no debug data kill", "--no-debug", "", "-d");
    opts.addOption(&opt.noRemoveLocals, "no remove locals", "--no-locals");
    opts.addOption(&opt.noRemovePrivateExports, "no remove private exports", "--no-private");
    opts.addOption(&opt.noTrampoline, "no trampoline build", "--no-trampoline", "", "-t");
    opts.addOption(&opt.recomputeCRC, "recompute export crc", "--export-crc-recomp", "", "-r");
    opts.addOption(&opt.logLevel, "log level (e,w,i,d,p)", "--logs", " (level)", "-l");
    opts.addOption(&opt.privateFile, "private file", "--private", " (file)", "-p");
    opts.addOption(&opt.fastfileBuilder, "replace fastfile builder name", "--fastfile-builder", " (builder)");
    opts.addOption(
        &opt.fastfileCompression,
        "replace fastfile compression",
        "--fastfile-compression",
        " (compression)"
    );
    opts.addOption(&opt.output, "output dir (default: output)", "--output", " (path)", "-o");

    bool badUsage{ !opts.ComputeOptions(1, argc, argv) || opt.printHelp || opts.NotEnoughParam(1) };

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

    LOG_INFO("GSC Obfuscate {}", gscobf::version::VERSION);

    if (badUsage) {
        LOG_INFO("usage: {} (file)", argv[0]);
        opts.PrintOptions();
        return opt.printHelp ? 0 : -1;
    }

    if (opt.privateFile) {
        if (!opt.privateFileData.ReadFile(opt.privateFile)) {
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
            try {
                HandleFile(opt, inFile, outFile);
            } catch (std::runtime_error& err) {
                LOG_ERROR("Error when handling {}: {}", inFile.string(), err.what());
                r = -1;
            }
        }
    }

    return r;
}