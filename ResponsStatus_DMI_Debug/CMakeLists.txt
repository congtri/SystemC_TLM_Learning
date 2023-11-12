cmake_minimum_required(VERSION 3.10)
project(tlm2_getting_started_2)

# Set the compiler and flags
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Include SystemC headers
include_directories(/usr/local/systemc-2.3.4/include)

# Set the source files
set(SOURCES
    tlm2_getting_started_2cpp.cpp         # Your main program source file
)

# Create the executable
add_executable(tlm2_getting_started_2cpp ${SOURCES})

# Link SystemC library
target_link_libraries(tlm2_getting_started_2cpp /usr/local/systemc-2.3.4/lib/libsystemc.dylib)
