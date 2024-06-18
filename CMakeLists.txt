cmake_minimum_required(VERSION 3.28)
project(AudioRecorder)

set(CMAKE_CXX_STANDARD 17)

# Add subdirectories
add_subdirectory(libs/json)
add_subdirectory(ui)

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
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        include
        libs/json/include
        ${PULSEAUDIO_INCLUDE_DIRS}
)

# Link against required libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
        nlohmann_json::nlohmann_json
        uv_a
        ${USOCKETS_LIB_PATH}
        pthread
        ${PULSEAUDIO_LIBRARIES}
        z
        m
        dl
)

# Add dependency on ui target
add_dependencies(${PROJECT_NAME} ui)
