add_library(MojoCore INTERFACE)
target_sources(MojoCore INTERFACE
	pencil.cpp
	$<TARGET_OBJECTS:MojoProto>)
target_link_libraries(MojoCore INTERFACE
	CONAN_PKG::glm
	CONAN_PKG::capnproto
	CONAN_PKG::zmqpp
	MojoProto
)
target_include_directories(MojoCore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mojo scene.cpp server.cpp mojo.cpp)
target_link_libraries(mojo PRIVATE MojoCore)
add_custom_command(TARGET mojo
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/mojo.exe" "${PROJECT_SOURCE_DIR}/maya/bin/"
)
add_library(MojoConnector INTERFACE)
target_sources(MojoConnector INTERFACE connector.cpp)
target_link_libraries(MojoConnector INTERFACE MojoCore)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(SWIG_USE_SWIG_DEPENDENCIES True)

find_package(Maya ${MAYA_VERSION} EXACT REQUIRED COMPONENTS python)
add_subdirectory(mayaRT)
add_subdirectory(mayaCT)

add_subdirectory(blenderCT)

add_executable(DummyConnector dummy.cpp)
target_link_libraries(DummyConnector PRIVATE MojoConnector)
