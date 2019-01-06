# Useful CMake utility functions
# Last Updated: 06/01/2019
# Copyright (c) 2015-19 David Avedissian (git@dga.me.uk)
cmake_minimum_required(VERSION 3.1)

# Create a vcproj userfile which correctly runs the binary in a specified working directory when debugging
function(create_vcproj_userfile TARGETNAME)
    cmake_policy(SET CMP0053 NEW)
    if(MSVC)
        set(WORKINGDIR ${ARGV1})
        set(VCPROJ_TEMPLATE "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                            "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">"
                            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">"
                            "<LocalDebuggerCommand>$(TargetPath)</LocalDebuggerCommand>"
                            "<LocalDebuggerWorkingDirectory>@CMAKE_CURRENT_SOURCE_DIR@/${WORKINGDIR}</LocalDebuggerWorkingDirectory>"
                            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor></PropertyGroup>"
                            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">"
                            "<LocalDebuggerCommand>$(TargetPath)</LocalDebuggerCommand>"
                            "<LocalDebuggerWorkingDirectory>@CMAKE_CURRENT_SOURCE_DIR@/${WORKINGDIR}</LocalDebuggerWorkingDirectory>"
                            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor></PropertyGroup>"
                            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='RelWithDebInfo|Win32'\">"
                            "<LocalDebuggerCommand>$(TargetPath)</LocalDebuggerCommand>"
                            "<LocalDebuggerWorkingDirectory>@CMAKE_CURRENT_SOURCE_DIR@/${WORKINGDIR}</LocalDebuggerWorkingDirectory>"
                            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor></PropertyGroup>"
                            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='MinSizeRel|Win32'\">"
                            "<LocalDebuggerCommand>$(TargetPath)</LocalDebuggerCommand>"
                            "<LocalDebuggerWorkingDirectory>@CMAKE_CURRENT_SOURCE_DIR@/${WORKINGDIR}</LocalDebuggerWorkingDirectory>"
                            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor></PropertyGroup>"
                            "</Project>")
        string(REPLACE ";" "\n" VCPROJ_TEMPLATE_STR "${VCPROJ_TEMPLATE}")
        string(CONFIGURE ${VCPROJ_TEMPLATE_STR} VCPROJ_TEMPLATE_STR_OUT @ONLY)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user ${VCPROJ_TEMPLATE_STR_OUT})
    endif()
endfunction()

# Mirror the directory structure in virtual directory based projects
function(mirror_physical_directories)
    foreach(FILE ${ARGN})
        get_filename_component(PARENT_DIR "${FILE}" PATH)
        string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endfunction()

# Set the output location explicitly
macro(set_output_dir TARGET OUTDIR)
    set_target_properties(${TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}
        LIBRARY_OUTPUT_DIRECTORY ${OUTDIR}
        ARCHIVE_OUTPUT_DIRECTORY ${OUTDIR})
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set_target_properties(${TARGET} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTDIR}
            LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTDIR}
            ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTDIR})
    endforeach()
endmacro()

# Enable C++11
macro(enable_cpp11 TARGET)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    if(UNIX)
        util_enable_libcpp(${TARGET})
    endif()
endmacro()

# Enable C++14
macro(enable_cpp14 TARGET)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 14)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    if(UNIX)
        util_enable_libcpp(${TARGET})
    endif()
endmacro()

# Enable C++17
macro(enable_cpp17 TARGET)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    if(UNIX)
        util_enable_libcpp(${TARGET})
    endif()
endmacro()

# If on macOS, enable the libc++ stdlib instead of the default one for more implementations of C++11/14/17
macro(util_enable_libcpp TARGET)
    if(APPLE)
        target_compile_options(${TARGET} PUBLIC -stdlib=libc++)
        if(XCODE)
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
        endif()
    endif()
endmacro()


# Macro to enable maximum warnings
macro(enable_maximum_warnings TARGET)
    if(MSVC)
        add_definitions(-D_SCL_SECURE_NO_WARNINGS)
        target_compile_options(${TARGET} PUBLIC /W4)
    elseif(UNIX)
        target_compile_options(${TARGET} PUBLIC -Wall -Wextra -pedantic -Wuninitialized -Wfloat-equal
                                                -Woverloaded-virtual -Wno-unused-parameter)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET} PUBLIC -Wno-nested-anon-types -Wno-gnu-anonymous-struct)
        endif()
    endif()
endmacro()
