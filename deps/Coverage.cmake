# ==============================================================================
# Coverage
# ==============================================================================
# gcov-based coverage via --coverage, reported by gcovr.
# Works with GCC and Clang on Linux and macOS out of the box.
# Windows requires GCC (e.g. MinGW via scoop install gcc) or a Clang build
# that includes compiler-rt (clang_rt.profile).
#
# Requires: gcovr on PATH (install via: uv tool install gcovr)
# Usage:    cmake -DENABLE_COVERAGE=ON ...  then  ninja coverage
# Report:   build/coverage/index.html
# ==============================================================================

option(ENABLE_COVERAGE "Build with coverage instrumentation" OFF)

if (ENABLE_COVERAGE)
    if (NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "ENABLE_COVERAGE requires GCC or Clang")
    endif()

    find_program(GCOVR_EXE gcovr REQUIRED)

    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        # gcovr needs a single-token gcov executable. Wrap llvm-cov gcov in a
        # script placed in the build dir (guaranteed no spaces in path).
        # AppleClang ships llvm-cov inside Xcode, reached only via xcrun.
        if (WIN32)
            find_program(LLVM_COV_EXE llvm-cov REQUIRED)
            set(GCOV_EXECUTABLE "${CMAKE_BINARY_DIR}/llvm-gcov.bat")
            file(WRITE "${GCOV_EXECUTABLE}" "@echo off\r\n\"${LLVM_COV_EXE}\" gcov %*\r\n")
        elseif (CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
            find_program(XCRUN_EXE xcrun REQUIRED)
            set(GCOV_EXECUTABLE "${CMAKE_BINARY_DIR}/llvm-gcov.sh")
            file(WRITE "${GCOV_EXECUTABLE}" "#!/bin/sh\nexec xcrun llvm-cov gcov \"$@\"\n")
            file(CHMOD "${GCOV_EXECUTABLE}"
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
        else()
            find_program(LLVM_COV_EXE llvm-cov REQUIRED)
            set(GCOV_EXECUTABLE "${CMAKE_BINARY_DIR}/llvm-gcov.sh")
            file(WRITE "${GCOV_EXECUTABLE}" "#!/bin/sh\nexec \"${LLVM_COV_EXE}\" gcov \"$@\"\n")
            file(CHMOD "${GCOV_EXECUTABLE}"
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
        endif()
        message(STATUS "Coverage: enabled (gcovr: ${GCOVR_EXE}, gcov: llvm-cov gcov)")
    else()
        set(GCOV_EXECUTABLE "gcov")
        message(STATUS "Coverage: enabled (gcovr: ${GCOVR_EXE})")
    endif()

    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()
