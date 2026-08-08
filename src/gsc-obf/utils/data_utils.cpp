#include <includes.hpp>
#include <utils/data_utils.hpp>

namespace utils::data {

    std::string AsHex(const void* buff, size_t size) {
        std::stringstream ss{};

        byte* ptr{ (byte*)buff };
        for (size_t i = 0; i < size; i++) {
            if ((i & 7) == 0 && i)
                ss << " ";
            ss << std::setw(2) << std::hex << std::setfill('0') << (int)ptr[i];
        }

        return ss.str();
    }

    std::string AsHexArray(const void* buff, size_t size) {
        std::stringstream ss{};

        byte* ptr{ (byte*)buff };
        for (size_t i = 0; i < size; i++) {
            if (i)
                ss << " ";
            ss << std::setw(2) << std::hex << std::setfill('0') << (int)ptr[i];
        }

        return ss.str();
    }
    std::mt19937& RandomMachine() {
        static struct {
            std::random_device rd;
            std::mt19937 gen{ rd() };
        } v{};
        return v.gen;
    }

    size_t Rand(size_t max) {
        if (!max) {
            return 0;
        }
        std::uniform_int_distribution<size_t> distrib{ 0, max - 1 };
        return distrib(RandomMachine());
    }

    void FillRandomBuffer(void* _buff, size_t size) {
        byte* buff{ (byte*)_buff };
        std::mt19937& gen{ RandomMachine() };
        std::uniform_int_distribution distrib{ 0, 0xFF };
        for (size_t i = 0; i < size; i++) {
            buff[i] = (byte)distrib(gen);
        }
    }

    bool IsNulled(const void* buff, size_t size) {
        switch (size) {
        case 1:
            return !*(byte*)buff;
        case 2:
            return !*(uint16_t*)buff;
        case 3:
            return !*(uint16_t*)buff && !((byte*)buff)[2];
        case 4:
            return !*(uint32_t*)buff;
        case 5:
            return !*(uint32_t*)buff && !((byte*)buff)[4];
        case 6:
            return !*(uint32_t*)buff && !((uint16_t*)buff)[2];
        case 8:
            return !*(uint64_t*)buff;
        case 0x10:
            return !(((uint64_t*)buff)[0] || ((uint64_t*)buff)[1]);
        default: {
            byte tmp[0x1000]{};
            const byte* p{ (byte*)buff };
            const byte* end{ &p[size] };

            while (p != end) {
                size_t tr{ std::min<size_t>(sizeof(tmp), end - p) };
                if (std::memcmp(tmp, p, tr)) {
                    return false;
                }
                p += tr;
            }

            return true;
        }
        }
    }

} // namespace utils::data