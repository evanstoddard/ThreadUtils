# Create target to run Doxygen
add_custom_target(docs
	COMMAND doxygen ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/docs
)
