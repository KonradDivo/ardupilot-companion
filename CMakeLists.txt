cmake_minimum_required(VERSION 3.16)
project(DroneCompanionControl VERSION 1.0.0 LANGUAGES CXX)

# Exiger le standard C++17 minimum
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Recherche de la bibliothèque industrielle MAVSDK
find_package(MAVSDK REQUIRED)

# Définition de l'exécutable
add_executable(offboard_control src/offboard_control.cpp)

# Liaison avec les modules MAVSDK requis
target_link_libraries(offboard_control
    PRIVATE
        mavsdk::mavsdk
        mavsdk::plugins_telemetry
        mavsdk::plugins_mavlink_passthrough
)

# Options de compilation pour un code propre (Warnings as Errors)
target_compile_options(offboard_control PRIVATE -Wall -Wextra -Werror)
