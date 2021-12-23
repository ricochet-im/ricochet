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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/compiler/warnings)
function (setup_compiler_warnings target)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
    if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        include(clang-warnings)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        include(gcc-warnings)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        include(msvc-warnings)
    else ()
        message(WARNING "No compiler specific warning settings detected for compiler ${CMAKE_CXX_COMPILER_ID}")
    endif ()

    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CXX_WARNINGS}>
                                             $<$<COMPILE_LANGUAGE:C>:${C_WARNINGS}>)
endfunction ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/compiler/san)
function (setup_compiler_sanitizers target)
    if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        include(clang-san)
        clang_setup_sanitizers(${target})
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        include(gcc-san)
        gcc_setup_sanitizers(${target})
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        include(msvc-san)
        msvc_setup_sanitizers(${target})
    else ()
        message(WARNING "No compiler specific sanitizer settings detected for compiler ${CMAKE_CXX_COMPILER_ID}")
    endif ()
endfunction ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/compiler/static)
function (setup_compiler_static_flags target)
    set(TEGO_STATIC_BUILD OFF CACHE BOOL "Enable compiler specific static build flags")
    if (TEGO_STATIC_BUILD)
        if (MINGW)
            include(mingw-static)
            mingw_setup_static_build(${target})
        elseif (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            include(clang-static)
            clang_setup_static_build(${target})
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            include(gcc-static)
            gcc_setup_static_build(${target})
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            include(msvc-static)
            msvc_setup_static_build(${target})
        else ()
            message(WARNING "No compiler specific static build settings detected for compiler ${CMAKE_CXX_COMPILER_ID}")
        endif ()
    endif ()
endfunction ()

function (setup_compiler target)
    setup_compiler_warnings(${target})
    setup_compiler_sanitizers(${target})
    setup_compiler_static_flags(${target})
endfunction ()
