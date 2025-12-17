cmake_minimum_required(VERSION 3.12)
project(oop_laba7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
        message(FATAL_ERROR "Требуется GCC 7.0 или выше для std::shared_mutex")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0")
        message(FATAL_ERROR "Требуется Clang 5.0 или выше для std::shared_mutex")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.14")
        message(FATAL_ERROR "Требуется MSVC 2017 15.7 или выше для std::shared_mutex")
    endif()
endif()

set(SOURCES
    main.cpp
    bear.cpp
    elf.cpp
    robber.cpp
    game/game.cpp
    factory/factory.cpp
    visitor/visitor.cpp
    observer/observer.cpp
)

add_executable(oop_laba7 ${SOURCES})

if(WIN32)
    target_compile_options(oop_laba7 PRIVATE /EHsc)
    target_compile_definitions(oop_laba7 PRIVATE _WIN32_WINNT=0x0601)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(oop_laba7 pthread)
endif()