cmake_minimum_required(VERSION 3.11)
project(adchubd)
set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED)
find_package(OpenSSL)

add_subdirectory(lua)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if(APPLE)
  add_compile_definitions(_DARWIN_C_SOURCE)
endif()

set(SOURCES
baselib/Base32.cpp
baselib/BaseUtil.cpp
baselib/CompatibilityManager.cpp
baselib/debug.cpp
baselib/Exception.cpp
baselib/File.cpp
baselib/FormatUtil.cpp
baselib/ParamExpander.cpp
baselib/Random.cpp
baselib/SimpleXML.cpp
baselib/SimpleXMLReader.cpp
baselib/SysVersion.cpp
baselib/Text.cpp
baselib/Thread.cpp
baselib/TigerHash.cpp
baselib/TimeUtil.cpp
adchpp/AdcCommand.cpp
adchpp/AppPaths.cpp
adchpp/BloomManager.cpp
adchpp/Bot.cpp
adchpp/Buffer.cpp
adchpp/CID.cpp
adchpp/Client.cpp
adchpp/ClientManager.cpp
adchpp/Core.cpp
adchpp/Entity.cpp
adchpp/HashBloom.cpp
adchpp/Hub.cpp
adchpp/LogManager.cpp
adchpp/LuaEngine.cpp
adchpp/LuaScript.cpp
adchpp/ManagedSocket.cpp
adchpp/PluginManager.cpp
adchpp/ScriptManager.cpp
adchpp/SocketManager.cpp
adchpp/Utils.cpp
adchpp/version.cpp
swig/lua_wrap.cxx
adchppd/adchppd.cpp
)

if(WIN32)
  list(APPEND SOURCES adchppd/adchppdw.cpp)
else()
  list(APPEND SOURCES adchppd/adchppdu.cpp)
endif()

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lua)

if(OPENSSL_FOUND)
  add_compile_definitions(HAVE_OPENSSL)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENSSL_LIBRARIES})
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} lua)

if(NOT WIN32)
  find_package(Threads)
  find_package(Iconv)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} ${Iconv_LIBRARIES})
endif()
