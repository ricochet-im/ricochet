# Ricochet Refresh - https://ricochetrefresh.net/
# Copyright (C) 2021, Blueprint For Free Speech <ricochet@blueprintforfreespeech.net>
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
# 
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following disclaimer
#      in the documentation and/or other materials provided with the
#      distribution.
# 
#    * Neither the names of the copyright owners nor the names of its
#      contributors may be used to endorse or promote products derived from
#      this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# QT doesn't play very nicely with static builds and cmake
# One of the issues is that QT will not add the plugin libraries at link time,
# so it has to be done manually

# Usage:
#  Will automatically find and link plugin libraries when using a static build
#  of QT. Expects STATIC_QT_ROOT_DIR to be the root path of the static build of
#  the QT SDK.
#  Assumes the QML files for the target are located in a subdirectory of
#  CMAKE_CURRENT_SOURCE_DIR
# Example:
#  target_generate_static_qml_plugins(ricochet-refresh)
function (target_generate_static_qml_plugins target)
    # Find all the plugins required using qmlimportscanner
    execute_process(
        COMMAND "${STATIC_QT_ROOT_DIR}/bin/qmlimportscanner" -rootPath "${CMAKE_CURRENT_SOURCE_DIR}" -importPath "${STATIC_QT_ROOT_DIR}/qml"
        OUTPUT_VARIABLE QML_IMPORT_JSON
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    
    # convert the output into a list by matching everything between {}
    string(REGEX MATCHALL "\\{[^\\}]+}" QML_IMPORT_LIST ${QML_IMPORT_JSON})
        
    # qmlimportscanner outputs in JSON, we want to extract all the "classname"
    # and "plugin" values
    set(PLUGIN_CLASSES "")
    set(PLUGIN_NAMES "")
    foreach (LINE ${QML_IMPORT_LIST})
        # try extract the class name
        string(REGEX MATCH "\"classname\"\\: \"([a-zA-Z0-9]*)\"" PLUGIN_CLASS ${LINE})
        if (CMAKE_MATCH_1 AND NOT CMAKE_MATCH_1 STREQUAL "")
            set(PLUGIN_CLASS ${CMAKE_MATCH_1})
            list(APPEND PLUGIN_CLASSES ${CMAKE_MATCH_1})

            # try extract the plugin name
            string(REGEX MATCH "\"plugin\"\\: \"([a-zA-Z0-9_]*)\"" PLUGIN_NAME ${LINE})
            if (CMAKE_MATCH_1 AND NOT CMAKE_MATCH_1 STREQUAL "")
                set(PLUGIN_NAME ${CMAKE_MATCH_1})
                list(APPEND PLUGIN_NAMES ${CMAKE_MATCH_1})

                # try extract the path hints
                string(REGEX MATCH "\"path\"\\: \"([^\"]*)\"" PLUGIN_PATH ${LINE})
                if (CMAKE_MATCH_1 AND NOT CMAKE_MATCH_1 STREQUAL "")
                    set(${PLUGIN_NAME}_PATH ${CMAKE_MATCH_1})
                endif ()
            endif ()
        endif ()
    endforeach()

    list(REMOVE_DUPLICATES PLUGIN_CLASSES)
    list(REMOVE_DUPLICATES PLUGIN_NAMES)

    # write the cpp file
    set(STATIC_QML_PLUGIN_SRC_FILE ${CMAKE_CURRENT_BINARY_DIR}/${target}-qml_plugin_imports.cpp)
    file(WRITE ${STATIC_QML_PLUGIN_SRC_FILE}
        "#include <QtPlugin>\n")
    foreach (CLASSNAME ${PLUGIN_CLASSES})
        file(APPEND ${STATIC_QML_PLUGIN_SRC_FILE}
            "Q_IMPORT_PLUGIN(${CLASSNAME})\n")
    endforeach ()
    target_sources(${target} PRIVATE ${STATIC_QML_PLUGIN_SRC_FILE})

    # add the libraries to the link line
    foreach (PLUGIN_NAME ${PLUGIN_NAMES})
        # TODO: this works for now, but it might break if we ever use plugins
        # that qmlimportscanner doesn't provide a path for
        find_library(${PLUGIN_NAME}_lib ${PLUGIN_NAME} HINTS ${${PLUGIN_NAME}_PATH})
        if (${${PLUGIN_NAME}_lib} STREQUAL "${PLUGIN_NAME}_lib-NOTFOUND")
            message(FATAL_ERROR "Could not find plugin library for plugin ${PLUGIN_NAME}. Tried ${${PLUGIN_NAME}_PATH} (${${PLUGIN_NAME}_lib})")
        endif ()

        target_link_libraries(${target} PRIVATE ${${PLUGIN_NAME}_lib})
    endforeach ()
endfunction ()

# Same usage as above, but for QT plugins
function (target_generate_static_qt_plugins target)
    # Get *all* plugin group names from QT_DIR/lib/cmake
    file(GLOB PLUGINGROUPS
        LIST_DIRECTORIES ON
        RELATIVE "${STATIC_QT_ROOT_DIR}/lib/cmake"
        "${STATIC_QT_ROOT_DIR}/lib/cmake/*")

    # On each of those groups, if a plugin is needed then QT should set
    # ${PLUGIN_GROUP}_DIR and ${PLUGIN_GROUP}_PLUGINS
    set(PLUGINS "")
    foreach (PLUGINGROUP ${PLUGINGROUPS})
        if (NOT ("${${PLUGINGROUP}_DIR}" STREQUAL "") AND NOT ("${${PLUGINGROUP}_PLUGINS}" STREQUAL ""))
            # Get all the plugins for this group
            foreach (PLUGIN_LIB ${${PLUGINGROUP}_PLUGINS})
                # Remove the leading "Qt<VERSION>::"
                string(REGEX REPLACE "Qt.::(.*)$" "\\1" PLUGIN ${PLUGIN_LIB})
                
                target_link_libraries(${target} PRIVATE ${PLUGIN_LIB})
                list(APPEND PLUGINS ${PLUGIN})
            endforeach ()
        endif ()
    endforeach ()

    # Write out the import file
    set(STATIC_QT_PLUGIN_SRC_FILE ${CMAKE_CURRENT_BINARY_DIR}/${target}-qt_plugin_imports.cpp)
    file(WRITE ${STATIC_QT_PLUGIN_SRC_FILE}
        "#include <QtPlugin>\n")
    foreach (CLASSNAME ${PLUGINS})
        file(APPEND ${STATIC_QT_PLUGIN_SRC_FILE}
            "Q_IMPORT_PLUGIN(${CLASSNAME})\n")
    endforeach ()
    target_sources(${target} PRIVATE ${STATIC_QT_PLUGIN_SRC_FILE})
endfunction ()