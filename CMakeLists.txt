cmake_minimum_required(VERSION 3.20)

project("GAS3" C CXX ASM)

include(cmake/st-project.cmake)

add_executable(${PROJECT_NAME})
add_st_target_properties(${PROJECT_NAME})

include_directories(Drivers)
set(CMAKE_CXX_STANDARD 17)

target_sources(
    ${PROJECT_NAME} PRIVATE
    "Drivers\\BSP\\OLED\\oled.c"
    "Drivers\\BSP\\SGP30\\sgp30.c"
    "Drivers\\SYSTEM\\delay\\delay.c"
    "Drivers\\SYSTEM\\sys\\sys.c"
)

