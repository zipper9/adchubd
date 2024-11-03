cmake_minimum_required(VERSION 3.10)
project(lua)

set(SOURCES
lapi.c
lauxlib.c
lbaselib.c
lcode.c
ldblib.c
ldebug.c
ldo.c
ldump.c
lfunc.c
lgc.c
linit.c
liolib.c
llex.c
lmathlib.c
lmem.c
loadlib.c
lobject.c
lopcodes.c
loslib.c
lparser.c
lstate.c
lstring.c
lstrlib.c
ltable.c
ltablib.c
ltm.c
lundump.c
lvm.c
lzio.c
)

add_library(lua STATIC ${SOURCES})

if(WIN32)
  message(STATUS "Platform: Windows")
elseif(APPLE)
  message(STATUS "Platform: MacOS")
  add_compile_definitions(LUA_USE_MACOS)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  message(STATUS "Platform: Linux")
  add_compile_definitions(LUA_USE_LINUX)
else()
  message(STATUS "Platform: POSIX")
  add_compile_definitions(LUA_USE_POSIX)
endif()
