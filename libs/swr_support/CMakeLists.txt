cmake_minimum_required(VERSION 3.10)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(swr_support SHARED
  bet.c
  buffer.c
  file_io.c
  hashstr.c
  misc.c
  os_amiga.c
  os_unix.c
  os_win32.c
  sha256.c
  string_handling.c
  time_fun.c
  vector3.c
)
target_compile_options(swr_support
  PRIVATE $<$<C_COMPILER_ID:GNU>:-DSWR2_USE_DLSYM -Werror -Wall -Wformat-security -Wshadow -Winline -Wpointer-arith -Wcast-align -Wredundant-decls -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -Wno-sign-compare -Wno-write-strings -Wno-format-truncation>
  PRIVATE $<$<C_COMPILER_ID:MSVC>:/D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE /wd4996 /wd6262 /wd26451 /wd4267 /wd4244 /wd4018 /we4805 /we4305>
  PRIVATE $<$<CXX_COMPILER_ID:IntelLLVM>:-Rno-debug-disables-optimization -Wno-deprecated-declarations>
  )
target_link_libraries(swr_support m)
