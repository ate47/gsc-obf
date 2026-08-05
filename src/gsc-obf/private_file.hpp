#pragma once

namespace gscobf::private_file {
    class PrivateFile {
        std::unordered_map<std::string, std::string> scripts{};
        std::unordered_map<std::string, std::string> strings{};

      public:
        PrivateFile() {};

        bool ReadFile(const char* file);
    };
} // namespace gscobf::private_file