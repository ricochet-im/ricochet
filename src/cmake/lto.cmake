# Enables LTO if available and requested (disable with windows targets when statically linking openssl and QT, see issue
# #104)

option(ENABLE_LTO "Enable Link Time Optimization (LTO)" ON)
if (ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT have_ipo OUTPUT err)
    if (have_ipo)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else ()
        message(SEND_ERROR "IPO requested, but not supported: ${err}")
    endif ()
endif ()
