cmake_minimum_required(VERSION 3.16)

project(ricochet-refresh LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (FORCE_QT5)
    find_package(
        QT
        NAMES
        Qt5
        COMPONENTS Core
                Gui
                Network
                Quick
                Widgets
                LinguistTools
        REQUIRED)
else ()
    find_package(
        QT
        NAMES
        Qt6
        Qt5
        COMPONENTS Core
                Gui
                Network
                Quick
                Widgets
                LinguistTools
        REQUIRED)
endif ()

find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core
               Gui
               Network
               Quick
               Widgets
               LinguistTools
    REQUIRED)

# Require Qt >5.15
if (${QT_VERSION_MAJOR} EQUAL 5)
    if (${QT_VERSION_MINOR} LESS 15)
        message(FATAL_ERROR "Qt >5.15 is required to build this project!")
    endif ()
endif ()

if (APPLE)
    find_package(
        Qt${QT_VERSION_MAJOR}
        COMPONENTS MacExtras
        REQUIRED)
endif ()

include(FindOpenSSL)

if (APPLE)
    add_executable(ricochet-refresh MACOSX_BUNDLE)
    set_target_properties(ricochet-refresh PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/Info.plist.in)

    # Set the icon in info.plist
    set(MACOSX_BUNDLE_ICON_FILE "ricochet_refresh.icns")

    # Tell cmake where to look for the actual icons
    set(RICOCHET_ICNS_RES ${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/ricochet_refresh.icns)
    set_source_files_properties(${RICOCHET_ICNS_RES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # And add it to the final app
    target_sources(ricochet-refresh PRIVATE ${RICOCHET_ICNS_RES})
elseif (WIN32) # Despite the naming, this also checks for win64 builds
    add_executable(ricochet-refresh WIN32)

    # Add the icon
    target_sources(ricochet-refresh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/resources.rc)
elseif (UNIX)
    # Linux or Cygwin (TODO: i'm not too familiar with cygwin, but i think we should build as if we're building for
    # linux?)
    add_executable(ricochet-refresh)
endif ()

# Add the translations
qt_add_translation(RICOCHET_QM_RES
    translation/ricochet_cs.ts
    translation/ricochet_ru.ts
    translation/ricochet_ja.ts
    translation/ricochet_tr.ts
    translation/ricochet_zh.ts
    translation/ricochet_uk.ts
    translation/ricochet_sv.ts
    translation/ricochet_sl.ts
    translation/ricochet_es.ts
    translation/ricochet_pt_BR.ts
    translation/ricochet_it.ts
    translation/ricochet_bg.ts
    translation/ricochet_da.ts
    translation/ricochet_et_EE.ts
    translation/ricochet_sq.ts
    translation/ricochet_he.ts
    translation/ricochet_de.ts
    translation/ricochet_fi.ts
    translation/ricochet_nb.ts
    translation/ricochet_it_IT.ts
    translation/ricochet_fil_PH.ts
    translation/ricochet_fr.ts
    translation/ricochet_pt_PT.ts
    translation/ricochet_en.ts
    translation/ricochet_pl.ts
    translation/ricochet_zh_HK.ts
    translation/ricochet_nl_NL.ts
)
# The QM files are generated in the build dir, but the qrc file expects the
# QM files to be relative paths. Work around this by copying the qrc file to
# the build dir
configure_file(translation/embedded.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

qt_add_resources(
    RICOCHET_QML_RES
    ${CMAKE_CURRENT_BINARY_DIR}/embedded.qrc
    sounds/sounds.qrc
    icons/icons.qrc
    ui/qml.qrc)

target_sources(ricochet-refresh PRIVATE main.cpp ${RICOCHET_QML_RES} ${RICOCHET_QM_RES})
target_precompile_headers(ricochet-refresh PRIVATE precomp.hpp)

include(lto)
include(compiler_opts)
# enables compiler specific warnings/sanitizers if requested
setup_compiler(ricochet-refresh)

target_compile_features(ricochet-refresh PRIVATE cxx_std_20)

target_link_libraries(ricochet-refresh PUBLIC tego tego_ui)

if (NOT USE_SUBMODULE_FMT)
    find_package(fmt REQUIRED)
endif ()
target_link_libraries(ricochet-refresh PRIVATE fmt::fmt-header-only)
target_link_libraries(ricochet-refresh PRIVATE OpenSSL::Crypto)

target_link_libraries(
    ricochet-refresh
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Qml
            Qt${QT_VERSION_MAJOR}::Quick)
if (APPLE)
    target_link_libraries(ricochet-refresh PRIVATE Qt${QT_VERSION_MAJOR}::MacExtras)
endif ()

if ("${CMAKE_BUILD_TYPE}" MATCHES "Rel.*" OR "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    target_compile_definitions(ricochet-refresh PRIVATE QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT)
endif ()

# Linux / Cygwin
if (UNIX)
    # Again, not sure if this needs to be UNIX AND NOT WIN32, or if we should
    # install to /bin on Cygwin like it does now
    install(TARGETS ricochet-refresh DESTINATION usr/bin)
endif ()

# Linux
if (UNIX AND NOT WIN32)
    option (RICOCHET_REFRESH_INSTALL_DESKTOP "Install desktop integration files + icons" OFF)
    if (RICOCHET_REFRESH_INSTALL_DESKTOP)
        install(FILES resources/linux/ricochet-refresh.desktop DESTINATION usr/share/applications)
        install(FILES resources/linux/icons/48x48/ricochet-refresh.png DESTINATION usr/share/icons/hicolor/48x48/apps/)
        install(FILES resources/linux/icons/scalable/ricochet-refresh.svg DESTINATION usr/share/icons/hicolor/scalable/apps/)
    endif ()
endif ()

# Move our final binary to a bin dir inside the output dir. This makes it
# easier for integration with ricochet-build
set_target_properties(ricochet-refresh
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ricochet-refresh/"
)
