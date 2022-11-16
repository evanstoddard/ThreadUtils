# Project
project(ThreadUtils)

# Compiler Options
set(CMAKE_CXX_STANDARD 14)

# Output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Packages

# Sources
set(ThreadUtils_SOURCES
	threadpool.cpp
)

# Headers
set(ThreadUtils_HEADERS
	threadpool.h
	runnable.h
)

# Libraries
set(ThreadUtils_LIBS
	pthread
)

# Include Paths

# Targets
add_library(ThreadUtils
	${ThreadUtils_SOURCES}
)

set_target_properties(ThreadUtils PROPERTIES PUBLIC_HEADER
	"${ThreadUtils_HEADERS}"
)

target_link_libraries(ThreadUtils
	${ThreadUtils_LIBS}
)