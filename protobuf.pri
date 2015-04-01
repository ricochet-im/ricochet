# Qt qmake integration with Google Protocol Buffers compiler protoc
#
# To compile protocol buffers with qt qmake, specify PROTOS variable and
# include this file
#
# Based on:
#   https://vilimpoc.org/blog/2013/06/09/using-google-protocol-buffers-with-qmake/

PROTOC = protoc

unix {
    PKG_CONFIG = $$pkgConfigExecutable()

    !contains(QT_CONFIG, no-pkg-config) {
        CONFIG += link_pkgconfig
        PKGCONFIG += protobuf
    } else {
        # Some SDK builds (e.g. OS X 5.4.1) are no-pkg-config, so try to hack the linker flags in.
        QMAKE_LFLAGS += $$system($$PKG_CONFIG --libs protobuf)
    }

    gcc|clang {
        # Add -isystem for protobuf includes to suppress some loud compiler warnings in their headers
        PROTOBUF_CFLAGS = $$system($$PKG_CONFIG --cflags protobuf)
        PROTOBUF_CFLAGS ~= s/^(?!-I).*//g
        PROTOBUF_CFLAGS ~= s/^-I(.*)/-isystem \\1/g
        QMAKE_CXXFLAGS += $$PROTOBUF_CFLAGS
    }
}

win32 {
    isEmpty(PROTOBUFDIR):error(You must pass PROTOBUFDIR=path/to/protobuf to qmake on this platform)
    INCLUDEPATH += $${PROTOBUFDIR}/include
    LIBS += -L$${PROTOBUFDIR}/lib -lprotobuf
    PROTOC = $${PROTOBUFDIR}/bin/protoc.exe
}

protobuf_decl.name = protobuf headers
protobuf_decl.input = PROTOS
protobuf_decl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
protobuf_decl.commands = $$PROTOC --cpp_out=${QMAKE_FILE_IN_PATH} --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
protobuf_decl.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf sources
protobuf_impl.input = PROTOS
protobuf_impl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\n)
protobuf_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl
