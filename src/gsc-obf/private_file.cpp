#include <includes.hpp>
#include <private_file.hpp>
#include <rapidcsv.h>

namespace gscobf::private_file {

    bool PrivateFile::ReadFile(const char* file) {
        rapidcsv::Document doc{ file };

        if (doc.GetColumnCount() != 2) {
            LOG_ERROR("Invalid private file: should a two column CSV!");
            return false;
        }

        for (size_t i = 0; i < doc.GetRowCount(); i++) {
            std::string type{ doc.GetCell<std::string>(0, i) };
            std::string val{ doc.GetCell<std::string>(1, i) };

            if (type == "string") {
                std::string str{ std::format("$${:x}", i) };
                if (str.size() > val.size()) {
                    LOG_WARNING("String '{}' is too small", val);
                    continue;
                }

                this->strings[val] = std::move(str);
                continue;
            }

            if (type == "script") {
                if (!(val.ends_with(".gsc") || val.ends_with(".gsh") || val.ends_with(".csc"))) {
                    LOG_WARNING("Missing or invalid script extension for {}", val);
                    continue;
                }
                std::string str{ std::format("oscr/{:x}{}", i, val.substr(val.size() - 4, 4)) };

                if (str.size() > val.size()) {
                    LOG_WARNING("Script '{}' is too small", val);
                    continue;
                }

                if (str.size() < val.size()) {
                    // add padding, it is required because we don't want to resize the asset name string in the xblock.
                    // for example
                    // in .. scripts/zm/zm_test_utils.gsc
                    // out . oscr/123xxxxxxxxxxxxxxxx.gsc
                    // same size, but one is obfuscated, maybe we should find how to not have the same size
                    // we can't use the gsc file to hide that because it is aligned
                    size_t start{ str.size() - 4 };
                    str.resize(val.size());
                    char* p{ str.data() };
                    // copy extension
                    std::memmove(&p[val.size() - 4], &p[start], 4);

                    for (size_t i = start; i < str.size() - 4; i++) {
                        p[i] = 'x';
                    }
                }
                this->scripts[val] = std::move(str);
                continue;
            }

            LOG_WARNING("Invalid private type '{}' for key '{}'", type, val);
        }

        return true;
    }
} // namespace gscobf::private_file