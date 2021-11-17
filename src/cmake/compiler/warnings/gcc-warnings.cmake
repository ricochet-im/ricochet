# based off https://github.com/cpp-best-practices/cpp_starter_project/blob/main/cmake/CompilerWarnings.cmake

include (clang-warnings)

# GCC accepts the same set of warnings as clang, and then some
set (
    CXX_WARNINGS
    ${CXX_WARNINGS}
    -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
    -Wduplicated-cond # warn if if / else chain has duplicated conditions
    -Wduplicated-branches # warn if if / else branches have duplicated code
    -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
)
