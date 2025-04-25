cmake_minimum_required(VERSION 3.28)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "Build for 10.14")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


project(clap-thread-demo VERSION 1.0.0.0 LANGUAGES C CXX)
set(PRODUCT_NAME "Clap Thread Demo")


option(USE_SANITIZER "Build and link with ASAN" FALSE)
option(COPY_AFTER_BUILD "Will copy after build" TRUE)
option(BUILD_SINGLE_ONLY "Only build the one plugin - no seven sines out" FALSE)

include(cmake/compile-options.cmake)

set(CLAP_WRAPPER_DOWNLOAD_DEPENDENCIES TRUE CACHE BOOL "Get em")
if (APPLE)
    # BaconPaul has jack kinda installed
    set(RTAUDIO_API_JACK FALSE CACHE BOOL "Not on apple")
endif()

add_subdirectory(libs/clap)
add_subdirectory(libs/clap-helpers)
add_subdirectory(libs/clap-wrapper)

## New version

add_library(${PROJECT_NAME}-impl STATIC
        src/ctd-clap.cpp
        src/ctd-clap-entry-impl.cpp
)
target_include_directories(${PROJECT_NAME}-impl PUBLIC src)
target_compile_definitions(${PROJECT_NAME}-impl PRIVATE
        PRODUCT_NAME="${PRODUCT_NAME}"
)

target_link_libraries(${PROJECT_NAME}-impl PUBLIC
        clap
)
target_link_libraries(${PROJECT_NAME}-impl PRIVATE
        clap-helpers clap-wrapper-extensions
)

make_clapfirst_plugins(
        TARGET_NAME ${PROJECT_NAME}
        IMPL_TARGET ${PROJECT_NAME}-impl

        OUTPUT_NAME "${PRODUCT_NAME}"

        ENTRY_SOURCE src/ctd-clap-entry.cpp

        BUNDLE_IDENTIFER "org.baconpaul.clap-thread-demo"
        BUNDLE_VERSION ${PROJECT_VERSION}

        COPY_AFTER_BUILD ${COPY_AFTER_BUILD}

        PLUGIN_FORMATS CLAP VST3 AUV2

        ASSET_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_assets
)




