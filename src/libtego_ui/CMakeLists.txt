cmake_minimum_required(VERSION 3.14)

project(tego_ui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    REQUIRED)
find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core
               Gui
               Network
               Quick
               Widgets
    REQUIRED)

if (APPLE)
    find_package(
        Qt${QT_VERSION_MAJOR}
        COMPONENTS MacExtras
        REQUIRED)
endif ()

find_package(fmt REQUIRED)

add_library(
    tego_ui STATIC
    ui/LanguagesModel.h
    ui/ContactsModel.h
    ui/Clipboard.cpp
    ui/Clipboard.h
    ui/ContactsModel.cpp
    ui/LanguagesModel.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
    utils/Useful.h
    utils/Settings.cpp
    utils/Settings.h
    libtego_callbacks.cpp
    shims/UserIdentity.h
    shims/ContactsManager.cpp
    shims/TorCommand.h
    shims/UserIdentity.cpp
    shims/ContactUser.h
    shims/ContactsManager.h
    shims/OutgoingContactRequest.h
    shims/ContactIDValidator.h
    shims/TorControl.h
    shims/TorManager.h
    shims/TorCommand.cpp
    shims/TorControl.cpp
    shims/IncomingContactRequest.h
    shims/OutgoingContactRequest.cpp
    shims/ConversationModel.cpp
    shims/ContactIDValidator.cpp
    shims/IncomingContactRequest.cpp
    shims/ContactUser.cpp
    shims/ConversationModel.h
    shims/TorManager.cpp
    libtego_callbacks.hpp)
target_precompile_headers(tego_ui PRIVATE precomp.hpp)

include(lto)
include(compiler_opts)
# enables compiler specific warnings/sanitizers if requested
setup_compiler(tego_ui)

target_compile_features(tego_ui PRIVATE cxx_std_20)

# Since ricochet-refresh includes libtego_callbacks.hpp as a system header file, we export the include directory twice,
# once as local, once as system TODO: perhaps there's a cleaner way to go about this
target_include_directories(tego_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tego_ui SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tego_ui PUBLIC tego)

target_link_libraries(tego PRIVATE fmt::fmt)

# QT
target_link_libraries(
    tego_ui
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Qml
            Qt${QT_VERSION_MAJOR}::Quick)
if (APPLE)
    target_link_libraries(tego PRIVATE Qt${QT_VERSION_MAJOR}::MacExtras)
endif ()

if ("${CMAKE_BUILD_TYPE}" MATCHES "Rel.*" OR "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    target_compile_definitions(tego PRIVATE QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT)
endif ()
