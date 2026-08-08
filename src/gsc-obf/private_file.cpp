#include <includes.hpp>
#include <private_file.hpp>
#include <rapidcsv.h>
#include <utils/data_utils.hpp>

namespace gscobf::private_file {
    void EncodeVal(size_t val, char* buff, size_t buffSize) {
        constexpr const char PRIVATE_FILE_UID_DICT[] = "abcdefghijklmnopqrstuvwxyz0123456789_.";
        constexpr size_t n = (sizeof(PRIVATE_FILE_UID_DICT) - 1);

        if (!buffSize) {
            throw std::runtime_error("PrivateDigit invalid buff");
        }
        while (val) {
            if (buffSize == 1) {
                break;
            }
            *(buff++) = PRIVATE_FILE_UID_DICT[val % n];
            val /= n;
            buffSize--;
        }
        *buff = 0;
    }

    const char* EncodeValTmp(size_t val) {
        thread_local size_t idx{};
        thread_local char tmp[16][16]{};
        auto& v{ tmp[(idx + 1) % ACTS_ARRAYSIZE(tmp)] };
        EncodeVal(val, v, sizeof(v));
        return v;
    }

    PrivateFile::PrivateFile() {}

    bool PrivateFile::ReadFile(const char* file) {
        rapidcsv::Document doc{ file };

        size_t uidVal{ utils::data::Rand(UINT32_MAX) };
        EncodeVal(uidVal, pfuid, sizeof(pfuid));
        LOG_DEBUG("using private file UID '{}' (0x{:x})", pfuid, uidVal);

        if (doc.GetColumnCount() != 2) {
            LOG_ERROR("Invalid private file: should a two column CSV!");
            return false;
        }

        for (size_t i = 0; i < doc.GetRowCount(); i++) {
            std::string type{ doc.GetCell<std::string>(0, i) };
            std::string val{ doc.GetCell<std::string>(1, i) };

            if (type == "string") {
                std::string str{ std::format("{}{}", pfuid, EncodeValTmp(i)) };
                if (str.size() > val.size()) {
                    LOG_WARNING("String '{}' is too small", val);
                    continue;
                }

                LOG_TRACE("add str '{}'->'{}'", val, str);
                this->strings[val] = std::move(str);
                continue;
            }

            if (type == "script") {
                if (!(val.ends_with(".gsc") || val.ends_with(".gsh") || val.ends_with(".csc"))) {
                    LOG_WARNING("Missing or invalid script extension for {}", val);
                    continue;
                }
                std::string str{ std::format("{}/{}{}", pfuid, EncodeValTmp(i), val.substr(val.size() - 4, 4)) };

                if (str.size() > val.size()) {
                    LOG_WARNING("Script '{}' is too small", val);
                    continue;
                }

                if (str.size() < val.size()) {
                    // add padding, it is required because we don't want to resize the asset name string in the xblock.
                    // for example
                    // in .. scripts/zm/zm_test_utils.gsc
                    // out . PFUID/123xxxxxxxxxxxxxxx.gsc
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
    void PrivateFile::RenamedString(char* str) {
        auto it{ strings.find(str) };
        if (it == strings.end()) {
            return; // not known
        }

        // replace the value
        std::memcpy(str, it->second.data(), it->second.size() + 1);
    }
} // namespace gscobf::private_file