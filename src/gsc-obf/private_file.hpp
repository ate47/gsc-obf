#pragma once

namespace gscobf::private_file {

    class PrivateFile {
        // private file uid, used to differentiate the generated strings
        size_t uidVal{};
        std::unordered_map<std::string, std::string> scripts{};
        std::unordered_map<std::string, std::string> strings{};

      public:
        PrivateFile();
        void EncodeBuffer(char* k, size_t start, size_t end, size_t val);
        void EncodeVal(size_t val, char* buff, size_t buffSize);
        const char* EncodeValTmp(size_t val);

        bool ReadFile(const char* file);
        void RenamedString(char* str);
        void RenamedScript(char* str);
        void RenamedScriptExt(char* str, bool client);
    };
} // namespace gscobf::private_file