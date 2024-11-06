# Use the variable PROJECT_NAME for changing the target name
set(PROJECT_NAME "ScanFiles")
set(CMAKE_CXX_STANDARD 20)
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_NO_BOOST_CMAKE ON)
set(BOOST_ROOT "/opt/homebrew/opt/boost")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
set(SQLite3_INCLUDE_DIR /opt/homebrew/Cellar/sqlite/3.47.0/include)
include_directories(${BOOST_INCLUDE_DIRS} ${SQLite3_INCLUDE_DIR})
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
add_compile_options(-g -O0)


find_package(Boost REQUIRED COMPONENTS filesystem date_time system program_options )

set(SQLite3_LIBRARY_DIR /opt/homebrew/Cellar/sqlite/3.47.0/lib)
set(SQLite3_LIBRARY sqlite3.dll)
find_package(SQLite3 REQUIRED)

# Set our project name
project(${PROJECT_NAME})

# Use all the *.cpp files we found under this folder for the project
file(GLOB SRCS "*.cpp" "*.c")

# Define the executable
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} SQLite3 Boost::filesystem Boost::date_time Boost::program_options)
