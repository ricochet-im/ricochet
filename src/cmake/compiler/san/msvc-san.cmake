function(msvc_setup_sanitizers target)
    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)

    set(SANITIZERS "")

    if(ENABLE_SANITIZER_ADDRESS)
        list(APPEND SANITIZERS "address")
    endif()

    # create the sanitizer flags in the form /fsanitize=<san1>,<san2>,...<sanN>
    list(JOIN SANITIZERS "," SAN_FLAGS)
    if(NOT ${SAN_FLAGS} STREQUAL "")
        target_compile_options(${target} INTERFACE /fsanitize=${SAN_FLAGS})
        target_link_options(${target} INTERFACE /fsanitize=${SAN_FLAGS})
    endif()
endfunction()
