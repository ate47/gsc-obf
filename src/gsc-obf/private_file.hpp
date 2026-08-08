#pragma once

namespace gscobf::private_file {

    /*
     * generate encoded string from a number, an encoded string is a small string
     * representation of a number, can be used to create small and unique string
     */
    void EncodeVal(size_t val, char* buff, size_t buffSize);
    // generate thread tmp a encoded string from a number
    const char* EncodeValTmp(size_t val);

    class PrivateFile {
        // private file uid, used to differentiate the generated strings
        char pfuid[7]{};
        std::unordered_map<std::string, std::string> scripts{};
        std::unordered_map<std::string, std::string> strings{};

      public:
        PrivateFile();
        bool ReadFile(const char* file);
        void RenamedString(char* str);
    };
} // namespace gscobf::private_file