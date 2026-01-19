set(RTYPE_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wunused
)

function(rtype_enable_warnings TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${TARGET} PRIVATE ${RTYPE_WARNING_FLAGS})
    endif()
endfunction()
