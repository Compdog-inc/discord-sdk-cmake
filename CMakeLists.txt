cmake_minimum_required(VERSION 3.20)

project(
    discordsdk
    VERSION 1.2.0
    LANGUAGES CXX
)

option(DISCORDSDK_BUILD_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})

set(DISCORDSDK_DOWNLOAD_URL "" CACHE STRING "URL to download the Discord SDK zip from")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FetchContent)

add_library(discordsdk STATIC
    src/discord.cpp
)

add_library(discordsdk::discordsdk ALIAS discordsdk)

target_compile_features(discordsdk PUBLIC cxx_std_20)

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_external")
set(SDK_DIR "${EXTERNAL_DIR}/discord_social_sdk")
set(SDK_ZIP "${CMAKE_CURRENT_BINARY_DIR}/discord_social_sdk.zip")
set(SDK_LIB_NAME "discord_partner_sdk")

if(NOT EXISTS "${SDK_DIR}" AND NOT DISCORDSDK_DOWNLOAD_URL STREQUAL "")
    message(STATUS "Downloading Discord SDK from ${DISCORDSDK_DOWNLOAD_URL}...")

    file(MAKE_DIRECTORY "${EXTERNAL_DIR}")

    file(DOWNLOAD
        "${DISCORDSDK_DOWNLOAD_URL}"
        "${SDK_ZIP}"
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
    )

    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_ERROR)

    if(NOT DOWNLOAD_ERROR EQUAL 0)
        list(GET DOWNLOAD_STATUS 1 DOWNLOAD_MESSAGE)
        message(FATAL_ERROR
            "Failed to download Discord SDK: ${DOWNLOAD_MESSAGE}"
        )
    endif()

    message(STATUS "Extracting Discord SDK...")

    file(ARCHIVE_EXTRACT
        INPUT "${SDK_ZIP}"
        DESTINATION "${EXTERNAL_DIR}"
    )

    file(REMOVE "${SDK_ZIP}")

    message(STATUS "Discord SDK installed")
else()
    message(STATUS "Found Discord SDK in ${SDK_DIR}")
endif()

target_include_directories(discordsdk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${SDK_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(WIN32)
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(SDK_LIB_PATH "${SDK_DIR}/lib/$<CONFIG>/arm64/${SDK_LIB_NAME}.lib")
    else()
        set(SDK_LIB_PATH "${SDK_DIR}/lib/$<CONFIG>/${SDK_LIB_NAME}.lib")
    endif()

    target_link_libraries(discordsdk PUBLIC "${SDK_LIB_PATH}" delayimp)
    target_link_options(discordsdk INTERFACE "/DELAYLOAD:${SDK_LIB_NAME}.dll")

elseif(APPLE)
    set(SDK_DYLIB_PATH "${SDK_DIR}/lib/$<CONFIG>/lib${SDK_LIB_NAME}.dylib")
    target_link_libraries(discordsdk PUBLIC "-Wl,-weak_library,${SDK_DYLIB_PATH}")

elseif(UNIX AND NOT APPLE)
    enable_language(ASM)

    FetchContent_Declare(
        implib
        GIT_REPOSITORY https://github.com/yugr/Implib.so.git
        GIT_TAG master
        GIT_SHALLOW 1
    )

    FetchContent_GetProperties(implib)
    if(NOT implib_POPULATED)
        FetchContent_Populate(implib)
    endif()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(SDK_SO_PATH "${SDK_DIR}/lib/${CMAKE_BUILD_TYPE}/lib${SDK_LIB_NAME}.so")
    
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lib${SDK_LIB_NAME}.tramp.S
               ${CMAKE_CURRENT_BINARY_DIR}/lib${SDK_LIB_NAME}.init.c
        COMMAND ${Python3_EXECUTABLE} ${implib_SOURCE_DIR}/implib-gen.py --dlopen-callback=linux_dlopen_callback -q ${SDK_SO_PATH}
        DEPENDS ${implib_SOURCE_DIR}/implib-gen.py ${SDK_SO_PATH}
    )

    target_sources(discordsdk PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}/lib${SDK_LIB_NAME}.tramp.S
        ${CMAKE_CURRENT_BINARY_DIR}/lib${SDK_LIB_NAME}.init.c
    )
    target_link_libraries(discordsdk PUBLIC dl)
endif()

install(
    TARGETS discordsdk
    EXPORT discordsdkTargets
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT discordsdkTargets
    NAMESPACE discordsdk::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/discordsdk
)

configure_package_config_file(
    cmake/discordsdkConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/discordsdkConfig.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/discordsdk
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/discordsdkConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/discordsdkConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/discordsdkConfigVersion.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/discordsdk
)

if(DISCORDSDK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()