# the only sanitizer that clang supports that GCC doesn't is memorysan, which we don't support anyway
include(clang-san)

function(gcc_setup_sanitizers target)
    clang_setup_sanitizers(${target})
endfunction()
