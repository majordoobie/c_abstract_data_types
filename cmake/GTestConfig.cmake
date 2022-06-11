# Enable ability to test
enable_testing()

# Fetch the google test github
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
)
# GMock does not work in C only C++ so just disable
option(INSTALL_GMOCK "Install GMock" OFF)

# Make available then include
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Append the gtest libraries to the target passed in then set sanitation flags
# for detecting memory leaks
function(AddTest target)
    target_compile_options(
            ${target}
            PUBLIC
            -g3
            -fno-omit-frame-pointer
            -fsanitize=address
            -fsanitize=undefined
            -fno-sanitize-recover=all
            -fsanitize=float-divide-by-zero
            -fsanitize=float-cast-overflow
            -fno-sanitize=null
            -fno-sanitize=alignment
    )
    target_link_options(
            ${target}
            PUBLIC
            -g3
            -fno-omit-frame-pointer
            -fsanitize=address
            -fsanitize=undefined
            -fno-sanitize-recover=all
            -fsanitize=float-divide-by-zero
            -fsanitize=float-cast-overflow
            -fno-sanitize=null
            -fno-sanitize=alignment
    )
    target_link_libraries(${target} PRIVATE gtest gtest_main)
    gtest_discover_tests(${target})
endfunction()