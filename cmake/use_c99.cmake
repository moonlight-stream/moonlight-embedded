# compatibility wrapper for CMAKE_C_STANDARD feature, for cmake versions older than 3.1
# copied from http://stackoverflow.com/a/30564223
macro(use_c99 target)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
    endif ()
  else ()
    set_property(TARGET target PROPERTY C_STANDARD 99)
  endif ()
endmacro(use_c99)
