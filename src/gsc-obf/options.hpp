#pragma once
#include <private_file.hpp>

namespace gscobf::options {
    struct GscObfOptions {
        gscobf::private_file::PrivateFile privateFileData{};
        bool printHelp{};
        bool printData{};
        bool noRemoveLocals{};
        bool noRemovePrivateExports{};
        bool noDebugKill{};
        bool noTrampoline{};
        bool recomputeCRC{};
        size_t seed{};
        const char* logLevel{};
        const char* privateFile{};
        const char* output{ "output" };
        const char* fastfileBuilder{};
        const char* fastfileCompression{};
    };
} // namespace gscobf::options