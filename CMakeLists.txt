cmake_minimum_required(VERSION 3.10)
message(STATUS "Cmake version ${CMAKE_VERSION}")

project(testnetFaucet)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# CURL::libcurl 
find_package(CURL REQUIRED COMPONENTS HTTP HTTPS SSL)
if(CMAKE_VERSION VERSION_LESS 3.12 AND NOT TARGET CURL::libcurl)
    add_library(libcurl UNKNOWN IMPORTED GLOBAL)
    set_target_properties(libcurl PROPERTIES
      IMPORTED_LOCATION ${CURL_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
    add_library(CURL_libcurl INTERFACE)
    target_link_libraries(CURL_libcurl INTERFACE libcurl)
    add_library(CURL::libcurl ALIAS CURL_libcurl)
endif()

file(GLOB cpr_sources ${conf_depends} external/cpr/cpr/*.cpp)

add_library(cpr STATIC EXCLUDE_FROM_ALL ${cpr_sources})
target_link_libraries(cpr PUBLIC CURL::libcurl)
target_include_directories(cpr PUBLIC external/cpr/include)
target_compile_definitions(cpr PUBLIC CPR_CURL_NOSIGNAL)
add_library(cpr::cpr ALIAS cpr)

add_subdirectory(external/nlhomann)

add_executable(transfer main.cpp)

target_include_directories(transfer PUBLIC external/nlhomann/include)
target_link_libraries(transfer PUBLIC cpr::cpr nlohmann_json)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)