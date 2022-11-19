# Project
project(ex_buffered_threadpool)

# Compiler Options
set(CMAKE_CXX_STANDARD 14)

# Output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/buffered_threadpool)

# Packages

# Sources
set(${PROJECT_NAME}_SOURCES
	main.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
)

# Libraries
set(${PROJECT_NAME}_LIBS
	pthread
)

# Include Paths
include_directories(
	${CMAKE_SOURCE_DIR}/src
)

# Targets
add_executable(${PROJECT_NAME}
	${${PROJECT_NAME}_SOURCES}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
	${${PROJECT_NAME}_LIBS}
)


