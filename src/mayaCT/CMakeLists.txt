set_source_files_properties(maya.i PROPERTIES CPLUSPLUS ON)
swig_add_library(MojoMaya TYPE SHARED LANGUAGE python SOURCES
	mayaConnector.h
	mayaConnector.cpp
	../pencil.h
	../connector.h
	maya.i
)

target_link_libraries(MojoMaya PRIVATE
	CONAN_PKG::boost
	MojoMayaRT
	MojoConnector
	Maya::Maya)
target_include_directories(MojoMaya PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET MojoMaya
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/_MojoMaya.pyd" "${PROJECT_SOURCE_DIR}/maya/lib/windows-${MAYA_VERSION}/"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/MojoMaya.py" "${PROJECT_SOURCE_DIR}/maya/lib/windows-${MAYA_VERSION}/"
)
