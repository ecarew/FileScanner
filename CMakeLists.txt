# Use the variable PROJECT_NAME for changing the target name
cmake_minimum_required(VERSION 3.30)
include(FindPkgConfig)
project(ScanFiles LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(Boost_NO_SYSTEM_PATHS ON)
set(BOOST_ROOT "/opt/homebrew/opt/boost")
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
add_compile_options(-g -O0)


find_package(Boost REQUIRED COMPONENTS filesystem date_time program_options )

pkg_check_modules(sqlite3 REQUIRED IMPORTED_TARGET sqlite3)
# Set our project name
project(${PROJECT_NAME})

# Use all the *.cpp files we found under this folder for the project
file(GLOB SRCS "*.cpp" "*.c")

# Define the executable
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} sqlite3 Boost::filesystem Boost::date_time Boost::program_options)
