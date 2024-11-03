cmake_minimum_required(VERSION 3.10)
project(adchubd)

find_package(Boost REQUIRED)
find_package(OpenSSL)

add_subdirectory(lua)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if(APPLE)
  add_compile_definitions(_DARWIN_C_SOURCE)
endif()

set(SOURCES
adchpp/AdcCommand.cpp
adchpp/adchpp.cpp
adchpp/BloomManager.cpp
adchpp/Bot.cpp
adchpp/Buffer.cpp
adchpp/Client.cpp
adchpp/ClientManager.cpp
adchpp/common.cpp
adchpp/Core.cpp
adchpp/Encoder.cpp
adchpp/Entity.cpp
adchpp/File.cpp
adchpp/HashBloom.cpp
adchpp/Hub.cpp
adchpp/LogManager.cpp
adchpp/LuaEngine.cpp
adchpp/LuaScript.cpp
adchpp/ManagedSocket.cpp
adchpp/PluginManager.cpp
adchpp/ScriptManager.cpp
adchpp/SimpleXML.cpp
adchpp/SocketManager.cpp
adchpp/Text.cpp
adchpp/Thread.cpp
adchpp/TigerHash.cpp
adchpp/Util.cpp
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

find_package(Threads)
if(CMAKE_USE_PTHREADS_INIT)
  add_compile_definitions(HAVE_PTHREAD)
endif()

if(OPENSSL_FOUND)
  add_compile_definitions(HAVE_OPENSSL)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENSSL_LIBRARIES})
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} lua)
