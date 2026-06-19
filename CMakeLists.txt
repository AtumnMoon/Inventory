cmake_minimum_required(VERSION 4.0.0)
project(inventory_manager VERSION 1.0.0 LANGUAGES CXX)

# Project Files
set(
    PROJECT_SOURCES
    src/inventory.cpp
    src/utils.cpp
    src/log.cpp
    src/main.cpp
)

# Create the executable with the specified C++ Version
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Enable error checking and warning as errors
target_compile_options(${PROJECT_NAME}
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:${COMMON_WARNING_FLAGS} -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)
