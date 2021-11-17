# based off https://github.com/cpp-best-practices/cpp_starter_project/blob/main/cmake/Sanitizers.cmake

function (clang_setup_sanitizers target)
    option (ENABLE_COVERAGE "Enable GCC/Clang coverage reporting" OFF)
    option (ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option (ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option (ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" OFF)
    option (ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)

    if (ENABLE_COVERAGE)
        target_compile_options (${target} INTERFACE --coverage -O0 -g)
        target_link_libraries (${target} INTERFACE --coverage)
    endif ()

    set (SANITIZERS "")

    if (ENABLE_SANITIZER_ADDRESS)
        list (APPEND SANITIZERS "address")
    endif ()

    if (ENABLE_SANITIZER_LEAK)
        list (APPEND SANITIZERS "leak")
    endif ()

    if (ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
        list (APPEND SANITIZERS "undefined")
    endif ()

    if (ENABLE_SANITIZER_THREAD)
        if ("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
            message (WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled")
        else ()
            list (APPEND SANITIZERS "thread")
        endif ()
    endif ()

    # create the sanitizer flags in the form -fsanitize=<san1>,<san2>,...<sanN>
    list (
        JOIN
        SANITIZERS
        ","
        SAN_FLAGS)
    if (NOT
        ${SAN_FLAGS}
        STREQUAL
        "")
        target_compile_options (${target} INTERFACE -fsanitize=${SAN_FLAGS})
        target_link_options (${target} INTERFACE -fsanitize=${SAN_FLAGS})
    endif ()
endfunction ()
