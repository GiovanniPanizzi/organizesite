cmake_minimum_required(VERSION 3.10)
project(organizesite VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# gumbo-parser files
file(GLOB GUMBO_SOURCES
	vendor/gumbo-parser/src/*.c
)

add_library(gumbo STATIC ${GUMBO_SOURCES})

target_include_directories(gumbo PUBLIC
	vendor/gumbo-parser/src
)

# app
file(GLOB APP_SOURCES
	main.cpp
	src/*.cpp
)

# create executable
add_executable(organizesite ${APP_SOURCES})

target_include_directories(organizesite PRIVATE
	include
)

# link gumbo as a static library
target_link_libraries(organizesite PRIVATE gumbo)
