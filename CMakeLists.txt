cmake_minimum_required(VERSION 3.18)
project(Recorder)

set(CMAKE_CXX_STANDARD 17)

# Add subdirectories
add_subdirectory(ui)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(UV_A NAMES uv_a uv PATHS /usr/lib/x86_64-linux-gnu /lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /lib/aarch64-linux-gnu)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so")

# Specify the path to uSockets.a
set(USOCKETS_LIB_PATH /usr/local/lib/uSockets.a)

# Find PulseAudio package
find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse)

# Add AddressSanitizer flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -fsanitize=address -fno-omit-frame-pointer")

# Add the executable
add_executable(${PROJECT_NAME}
        src/main.cpp
        include/miniaudio.h
        src/recording/baseRecorder.h
        src/recording/audio/audioRecorder.cpp
        src/recording/audio/audioRecorder.h
        src/dashboard/dashboard.cpp
        src/dashboard/dashboard.h
        src/dashboard/cpu/CpuInfo.h
        src/recording/recorders.h
        src/dashboard/app.h
        include/sha512.h
        src/dashboard/auth/auth.h
        src/dashboard/auth/auth.cpp
        src/dashboard/auth/auth.h
        src/dashboard/auth/auth.cpp
        src/dashboard/auth/auth.h
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${PULSEAUDIO_INCLUDE_DIRS}
)

# Link against required libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
        ${UV_A}
        ${USOCKETS_LIB_PATH}
        pthread
        ${PULSEAUDIO_LIBRARIES}
        z
        m
        dl
)

# Add dependency on ui target
add_dependencies(${PROJECT_NAME} ui)

file(GLOB MISC_FILES misc/*)
file(COPY ${MISC_FILES} DESTINATION ${CMAKE_BINARY_DIR})
