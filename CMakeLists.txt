cmake_minimum_required(VERSION 3.16)

project(komorebi-tray 
    VERSION 1.2
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    src/main.cpp
    src/komorebi-tray.rc
)
if(MSVC) 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
endif()

message("==================")
message("CXX_FLAGS: ${CMAKE_CXX_FLAGS_RELEASE}")
message("LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
message("==================")

add_executable(komorebi-tray ${PROJECT_SOURCES})

target_link_libraries(komorebi-tray
    user32
    shell32
    gdi32
)

set_target_properties(komorebi-tray PROPERTIES
    WIN32_EXECUTABLE TRUE
)
