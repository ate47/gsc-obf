#pragma once
#include <data/gsc_data_t7.hpp>
#include <utils/utils.hpp>
#include <options.hpp>

namespace gscobf::obfuscator {

    constexpr size_t TRAMPOLINE_SIZE = 4; // Jmp[2] Delta[2]

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

    template<typename Value>
    using NameLocatedMap = std::unordered_map<NameLocated, Value, NameLocatedHash, NameLocatedEquals>;

    struct GscExportData {
        data::gsc::T7GSCExport* ref;
        byte* bc{};
        size_t len{};
    };

    class GscObfuscator {
        gscobf::options::GscObfOptions& opt;
        byte* script;
        size_t scriptLen;
        void* scriptEnd;
        data::gsc::T7GSCOBJ& header;

        NameLocatedMap<std::vector<GscExportData>> exportsMap{};
        NameLocatedMap<std::vector<data::gsc::T7GSCImport*>> importsMap{};
        std::vector<uint32_t> trampolineFreeLocations{};
        uint32_t junkLocation{}; // well to redirect all the offset
      public:
        GscObfuscator(gscobf::options::GscObfOptions& opt, byte* script, size_t scriptLen);

        // init
        void ReadTables();
        void RemoveLocalVariables();
        void KillDevImports();
        void KillDevStrings();
        void KillPrivateExports();
        void ApplyPrivateStrings();
        void ApplyPrivateScripts();
        void CreateTrampolines();
        void ComputeChecksums();
        void RunTasks();

        // utils
        void PrintHeader();
        void ValidateInScript(const void* ptr, const char* descr);
        void ValidateStringInScript(const char* ptr, const char* descr);
        void KillDevByteCodeOp(uint32_t& floc, size_t len, size_t delta);

        // align a location relative to the header (used to avoid aligning the script in memory)
        byte* Aligned(byte* loc, size_t size) { return &script[utils::Aligned<size_t>(loc - script, size)]; }
        template<typename T>
        byte* Aligned(byte* loc) {
            return Aligned(loc, sizeof(T));
        }
    };
} // namespace gscobf::obfuscator