# ==============================================================================
# IDE Integration
# ==============================================================================
# Groups dependency targets into folders for Visual Studio / Xcode.
# Has no effect on the build.
# ==============================================================================

function(set_target_folder target folder)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES FOLDER ${folder})
    endif()
endfunction()

if (CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR MATCHES "Xcode")
    set_target_folder(unity deps)
    set_target_folder(cmock deps)
endif()
