add_library(MojoMayaRT INTERFACE)
target_sources(MojoMayaRT INTERFACE
	pencilNode.cpp
	pencilData.cpp)
target_include_directories(MojoMayaRT INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(MojoMayaPlugin SHARED
	pencilDraw.cpp
	plugin.cpp)
MAYA_PLUGIN(MojoMayaPlugin)
target_link_libraries(MojoMayaPlugin PRIVATE
	CONAN_PKG::glew
	MojoMayaRT
	MojoCore
	Maya::Maya)
set_target_properties(MojoMayaPlugin PROPERTIES OUTPUT_NAME mojo)

add_custom_command(TARGET MojoMayaPlugin
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/mojo.mll" "${PROJECT_SOURCE_DIR}/maya/plug-ins/windows-${MAYA_VERSION}/")
