#pragma once

/**
@file
@brief Ymir library version definitions.
*/

namespace ymir::version {

inline constexpr auto string = Ymir_VERSION; ///< The library version string in the format "<major>.<minor>.<patch>"
inline constexpr auto major = Ymir_VERSION_MAJOR;   ///< The library's major version
inline constexpr auto minor = Ymir_VERSION_MINOR;   ///< The library's minor version
inline constexpr auto patch = Ymir_VERSION_PATCH;   ///< The library's patch version
inline constexpr auto suffix = Ymir_VERSION_SUFFIX; ///< The library's version suffix

} // namespace ymir::version
