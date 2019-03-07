cmake_minimum_required(VERSION 3.0)
project(ncmdump)

set(CMAKE_CXX_STANDARD 14)
file(GLOB SOURCES lib/*.h lib/*.cpp)

# will add option to select target type and language binding
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
# add_library(${PROJECT_NAME} SHARED ${SOURCES})
include_directories(lib)
if (UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(TAGLIB REQUIRED taglib)
    target_link_libraries(${PROJECT_NAME} ${TAGLIB_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${TAGLIB_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PUBLIC ${TAGLIB_CFLAGS_OTHER})
else (WIN32)
    message(WARNING "No automation on other platforms, you must handle dependencies yourself")
    target_link_libraries(${PROJECT_NAME} taglib)
endif ()


