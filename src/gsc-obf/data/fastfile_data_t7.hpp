#pragma once

namespace data::fastfile {
    constexpr uint32_t FF_T7_VERSION = 0x251;
    constexpr uint64_t FF_T7_MAGIC = 0x3030303066664154;

    enum FastFileCompression : byte {
        XFILE_UNCOMPRESSED = 0x0,
        XFILE_ZLIB = 0x1,
        XFILE_ZLIB_HC = 0x2,
        XFILE_LZ4 = 0x3,
        XFILE_LZ4_HC = 0x4,
        XFILE_BDELTA_UNCOMP = 0x5,
        XFILE_BDELTA_ZLIB = 0x6,
        XFILE_BDELTA_LZMA = 0x7,
        XFILE_COMPRESSION_COUNT = 0x8,
    };

    enum FastFilePlatform : byte {
        XFILE_PC = 0x0,
        XFILE_XBOX = 0x1,
        XFILE_PLAYSTATION = 0x2,
        XFILE_DEV = 0x3,
        XFILE_PLATFORM_COUNT = 0x4,
    };

    struct XFileBO3 {
        uint8_t magic[8];
        uint32_t version;
        uint8_t server;
        FastFileCompression compression;
        FastFilePlatform platform;
        uint8_t encrypted;
        uint64_t timestamp;
        uint32_t changelist;
        uint32_t archiveChecksum[4];
        char builder[32];
        uint32_t metaVersion;
        char mergeFastfile[64];
        uint64_t size;
        uint64_t externalSize;
        uint64_t memMappedOffset;
        uint64_t blockSize[10];
        char fastfileName[64];
        uint8_t signature[256];
        uint8_t aesIV[16];
    };
    static_assert(sizeof(XFileBO3) == 0x248);

    struct DBStreamHeader {
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        uint32_t alignedSize;
        uint32_t offset;
    };
    static_assert(sizeof(DBStreamHeader) == 0x10);

} // namespace data::gsc