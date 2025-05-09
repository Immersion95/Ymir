include(CheckSymbolExists)

# Universal Binaries on MacOS will already list all the target architectures
if (CMAKE_OSX_ARCHITECTURES)
    set(ARCHITECTURES "${CMAKE_OSX_ARCHITECTURES}")
    return()
endif ()

# Test if the target C/C++ compiler has the symbol defined
# Allows for architecture to be defined by the compiler rather than the
# just the architecture of the host-machine to allow for cross-compilation
function(detect_arch_symbol symbol arch)
    if (NOT DEFINED ARCHITECTURES)
        set(CMAKE_REQUIRED_QUIET YES)
        check_symbol_exists("${symbol}" "" symbol_found_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        if (symbol_found_${arch})
            set(ARCHITECTURES "${arch}" PARENT_SCOPE)
        endif ()

        unset(symbol_found_${arch} CACHE)
    endif ()
endfunction ()

# arm64
detect_arch_symbol("__aarch64__" arm64) # Clang/GCC
detect_arch_symbol("__ARM_ARCH_ISA_A64" arm64) # Clang/GCC
detect_arch_symbol("__arm64__ " arm64) # Clang/GCC
detect_arch_symbol("__ARM64__" arm64) # Clang/GCC
detect_arch_symbol("__arm64" arm64) # Clang/GCC
detect_arch_symbol("__ARM_ARCH" arm64) # MSVC
detect_arch_symbol("_M_ARM64" arm64) # MSVC

# x86_64
detect_arch_symbol("__amd64" x86_64) # Clang/GCC
detect_arch_symbol("__x86_64__" x86_64) # Clang/GCC
detect_arch_symbol("__x86_64" x86_64) # Clang/GCC
detect_arch_symbol("_M_AMD64" x86_64) # MSVC
detect_arch_symbol("_M_X64" x86_64) # MSVC