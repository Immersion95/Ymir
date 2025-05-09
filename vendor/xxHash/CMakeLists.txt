add_library(xxHash
	xxHash/xxhash.c
	xxHash/xxhash.h
	xxHash/xxh3.h
)

if ("x86_64" IN_LIST ARCHITECTURES)
    set(XXHSUM_DISPATCH ON)
	target_sources(xxHash PRIVATE
		xxHash/xxh_x86dispatch.c
		xxHash/xxh_x86dispatch.h
	)
	target_compile_definitions(xxHash PRIVATE XXHSUM_DISPATCH=1)
endif ()

add_library(xxHash::xxHash ALIAS xxHash)
target_include_directories(xxHash
    PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xxHash>"
    PRIVATE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xxHash>"
)
target_compile_features(xxHash PUBLIC cxx_std_20)

## Apply performance options
if ("x86_64" IN_LIST ARCHITECTURES)
	if (Ymir_AVX2)
		if (MSVC)
			target_compile_options(xxHash PUBLIC "/arch:AVX2")
		else ()
			target_compile_options(xxHash PUBLIC "-mavx2")
			target_compile_options(xxHash PUBLIC "-mfma")
			target_compile_options(xxHash PUBLIC "-mbmi")
		endif ()
	endif ()
endif ()
