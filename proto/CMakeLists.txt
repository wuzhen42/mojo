add_custom_command(OUTPUT mojo.capnp.c++
	COMMAND capnp compile --output=c++ --src-prefix=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/mojo.capnp -I${CONAN_INCLUDE_DIRS_CAPNPROTO}
	COMMAND ${CMAKE_COMMAND} -E copy mojo.capnp.c++ mojo.capnp.h ${CMAKE_CURRENT_BINARY_DIR}
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/mojo.capnp
	WORKING_DIRECTORY ${CONAN_BIN_DIRS_CAPNPROTO}
	VERBATIM
)
add_library(MojoProto OBJECT mojo.capnp.c++)
target_include_directories(MojoProto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(MojoProto PUBLIC CONAN_PKG::capnproto)
