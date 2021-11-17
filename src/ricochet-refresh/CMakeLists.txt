cmake_minimum_required (VERSION 3.14)

project (ricochet-refresh LANGUAGES CXX)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (
    QT
    NAMES
    Qt6
    Qt5
    COMPONENTS Core
               Gui
               Network
               Quick
               Widgets
    REQUIRED)
find_package (
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core
               Gui
               Network
               Quick
               Widgets
    REQUIRED)

if (APPLE)
    find_package (
        Qt${QT_VERSION_MAJOR}
        COMPONENTS MacExtras
        REQUIRED)
endif ()

find_package (fmt REQUIRED)

qt_add_resources (
    RICOCHET_QML_RES
    translation/embedded.qrc
    sounds/sounds.qrc
    icons/icons.qrc
    ui/qml.qrc)

if (APPLE)
    add_executable (ricochet-refresh MACOSX_BUNDLE)
    set_target_properties (ricochet-refresh PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
elseif (WIN32) # Despite the naming, this also checks for win64 builds
    add_executable (ricochet-refresh WIN32)
elseif (UNIX)
    # Linux or Cygwin (TODO: i'm not too familiar with cygwin, but i think we should build as if we're building for
    # linux?)
    add_executable (ricochet-refresh)
endif ()

target_sources (ricochet-refresh PRIVATE main.cpp ${RICOCHET_QML_RES})
target_precompile_headers (ricochet-refresh PRIVATE precomp.hpp)

include (lto)
include (compiler_opts)
# enables compiler specific warnings/sanitizers if requested
setup_compiler (ricochet-refresh)

target_compile_features (ricochet-refresh PRIVATE cxx_std_20)

target_link_libraries (ricochet-refresh PUBLIC tego tego_ui)

target_link_libraries (ricochet-refresh PRIVATE fmt::fmt)

target_link_libraries (
    ricochet-refresh
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Qml
            Qt${QT_VERSION_MAJOR}::Quick)
if (APPLE)
    target_link_libraries (ricochet-refresh PRIVATE Qt${QT_VERSION_MAJOR}::MacExtras)
endif ()

if ("${CMAKE_BUILD_TYPE}" MATCHES "Rel.*" OR "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    target_compile_definitions (ricochet-refresh PRIVATE QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT)
endif ()
