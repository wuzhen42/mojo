set(BLENDER_VERSION 33)
find_package(Python 3.10 EXACT REQUIRED COMPONENTS Development)

set_source_files_properties(blender.i PROPERTIES CPLUSPLUS ON)
swig_add_library(MojoBlender TYPE SHARED LANGUAGE python SOURCES
	../pencil.h
	../connector.h
	blender.i
)

target_link_libraries(MojoBlender PRIVATE MojoConnector ${Python_LIBRARIES})
target_include_directories(MojoBlender PRIVATE
	${Python_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET MojoBlender
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/_MojoBlender.pyd" "${PROJECT_SOURCE_DIR}/blender/lib/windows-py310/"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/MojoBlender.py" "${PROJECT_SOURCE_DIR}/blender/lib/windows-py310/"
)
