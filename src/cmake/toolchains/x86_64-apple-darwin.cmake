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

set(CMAKE_SYSTEM_NAME Darwin CACHE STRING "" FORCE)
set(CMAKE_SYSTEM_PROCESSOR x86_64 CACHE STRING "" FORCE)

set(CMAKE_C_COMPILER_TARGET "x86_64-apple-darwin")
set(CMAKE_CXX_COMPILER_TARGET "x86_64-apple-darwin")
set(CMAKE_ASM_COMPILER_TARGET "x86_64-apple-darwin")

set(CMAKE_PREFIX_PATH "/var/tmp/dist/macosx-toolchain/clang/lib/" CACHE STRING "" FORCE)
set(CMAKE_FRAMEWORK_PATH "/var/tmp/dist/macosx-toolchain/MacOSX10.15.sdk/System/Library/Frameworks" CACHE STRING "" FORCE)
