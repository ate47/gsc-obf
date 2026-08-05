#pragma once
namespace gscobf::version {
    // Do not write in this file, it is updated by cmake

    constexpr unsigned int DEV_VERSION_ID = 0xFFFFFFFF;
    constexpr unsigned int BUILD_VERSION_ID = 0x0001000;

    constexpr const char* VERSION = "0.1.0";
    constexpr const wchar_t* VERSIONW = L"0.1.0";

    // Disable updater
    constexpr unsigned int VERSION_ID = DEV_VERSION_ID;

} // namespace gscobf::version
