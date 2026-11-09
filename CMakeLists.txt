cmake_minimum_required(VERSION 3.2)
project(SensorDisSys VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get dependencies
include(FetchContent)
FetchContent_Declare(
    glog
    GIT_REPOSITORY  https://github.com/google/glog.git
    GIT_TAG         v0.6.0
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(WITH_GFLAGS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glog)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-28-0
)
FetchContent_MakeAvailable(asio)

FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG        0.8.2
)
FetchContent_MakeAvailable(websocketpp)

add_executable(${PROJECT_NAME} src/main.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASIO_STANDALONE
    _WEBSOCKETPP_CPP11_STL_
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    ${asio_SOURCE_DIR}/asio/include
    ${websocketpp_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glog::glog
    pthread
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion
)

