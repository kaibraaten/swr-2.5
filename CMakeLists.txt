cmake_minimum_required(VERSION 3.21)
project(swr2)

include_directories(src libs/swr_support)
link_directories(libs/swr_support)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(swr2
  src/act_comm.c
  src/act_info.c
  src/act_move.c
  src/act_obj.c
  src/act_wiz.c
  src/boards.c
  src/build.c
  src/clans.c
  src/colony.c
  src/comm.c
  src/const.c
  src/copyover.c
  src/db.c
  src/fight.c
  src/handler.c
  src/imc.c
  src/interp.c
  src/magic.c
  src/makeobjs.c
  src/misc.c
  src/morespace.c
  src/mud_comm.c
  src/mud_prog.c
  src/nanny.c
  src/planets.c
  src/player.c
  src/reset.c
  src/save.c
  src/shops.c
  src/skillpackage.c
  src/skills.c
  src/space.c
  src/special.c
  src/swskills.c
  src/tables.c
  src/telopt.c
  src/track.c
  src/update.c
  src/vector3_aux.c
  src/vote.c)

#target_compile_features(swr2 PRIVATE cxx_std_20)
target_compile_options(swr2
  PRIVATE $<$<C_COMPILER_ID:GNU>:-DSWR2_USE_DLSYM -DSWR2_USE_IMC  -Werror -Wall -Wformat-security -Wshadow -Winline -Wpointer-arith -Wcast-align -Wredundant-decls -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -Wno-sign-compare -Wno-write-strings -Wno-format-truncation -Wno-format-overflow -Wno-stringop-overflow>
  PRIVATE $<$<C_COMPILER_ID:MSVC>:/D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE /wd4244 /wd4996 /wd6262 /wd26451>
  PRIVATE $<$<CXX_COMPILER_ID:IntelLLVM>:-Rno-debug-disables-optimization -Wno-deprecated-declarations -DSWR2_USE_IMC -DSWR2_USE_DLSYM>
  )
target_link_libraries(swr2 dl swr_support z)

add_subdirectory(libs/swr_support)
