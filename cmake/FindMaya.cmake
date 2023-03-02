set(MAYA_VERSION ${Maya_FIND_VERSION})

#platform-dependent compile definitions
set(MAYA_TARGET_TYPE LIBRARY)
if(WIN32)
	set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL")
    set(MAYA_INSTALL_BASE_DEFAULT "C:/PROGRA~1/Autodesk")
    set(MAYA_PLUGIN_EXTENSION ".mll")
    set(MAYA_TARGET_TYPE RUNTIME)
elseif(UNIX)
    set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL;LINUX")
    set(MAYA_INSTALL_BASE_DEFAULT "/usr/autodesk")
    set(MAYA_PLUGIN_EXTENSION ".so")
endif()

#search for include and lib location 
set(MAYA_LOCATION ${MAYA_INSTALL_BASE_DEFAULT}/maya${MAYA_VERSION})
find_path(MAYA_INCLUDE_DIR maya/MFn.h
    PATHS
        ${MAYA_LOCATION}
        $ENV{MAYA_LOCATION}
    PATH_SUFFIXES
        "include/"
)
find_library(MAYA_LIBRARY
    NAMES
        OpenMaya
    PATHS
        ${MAYA_LOCATION}
        $ENV{MAYA_LOCATION}
    PATH_SUFFIXES
        "lib/"
    NO_DEFAULT_PATH
)
set(MAYA_INCLUDE_DIRS "${MAYA_INCLUDE_DIR}")
set(MAYA_LIBRARIES "${MAYA_LIBRARY}")

#setup libraries list
set(_MAYA_LIBRARIES OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI Foundation clew)
if(UNIX)
    list(APPEND _MAYA_LIBRARIES Shared)
    if (MAYA_VERSION LESS 2019)
        list(APPEND _MAYA_LIBRARIES OpenMayalib)
    endif()
endif()
foreach(MAYA_LIB ${Maya_FIND_COMPONENTS})
    if (${MAYA_LIB} MATCHES "python")
        if (MAYA_VERSION GREATER_EQUAL 2022)
			set(MAYAPY_MAJOR_VERSION 3)
			if (MAYA_VERSION EQUAL 2022)
				set(MAYAPY_MINOR_VERSION 7)
			elseif(MAYA_VERSION EQUAL 2023)
				set(MAYAPY_MINOR_VERSION 9)
			else()
				message(FATAL_ERROR "unsupported maya version: ${MAYA_VERSION}")
			endif()
            list(APPEND MAYA_INCLUDE_DIRS ${MAYA_INCLUDE_DIR}/Python${MAYAPY_MAJOR_VERSION}${MAYAPY_MINOR_VERSION}/python)
        else()
            list(APPEND MAYA_INCLUDE_DIRS ${MAYA_INCLUDE_DIR}/python2.7)
			set(MAYAPY_MAJOR_VERSION 2)
			set(MAYAPY_MINOR_VERSION 7)
        endif()
		if (UNIX)
			list(APPEND _MAYA_LIBRARIES "python${MAYAPY_MAJOR_VERSION}.${MAYAPY_MINOR_VERSION}")
		else()
			list(APPEND _MAYA_LIBRARIES "python${MAYAPY_MAJOR_VERSION}${MAYAPY_MINOR_VERSION}")
		endif()
    else()
		list(APPEND _MAYA_LIBRARIES ${MAYA_LIB})
    endif()
endforeach()

if(NOT TARGET Maya::Maya)
    add_library(Maya::Maya UNKNOWN IMPORTED)
    set_target_properties(Maya::Maya PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "${MAYA_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${MAYA_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${MAYA_LIBRARY}")
endif()

#find libraries
foreach(MAYA_LIB ${_MAYA_LIBRARIES})
    find_library(MAYA_${MAYA_LIB}_LIBRARY
        NAMES
            ${MAYA_LIB}
        PATHS
            ${MAYA_LOCATION}
            $ENV{MAYA_LOCATION}
        PATH_SUFFIXES
            "lib/"
        NO_DEFAULT_PATH)    
    mark_as_advanced(MAYA_${MAYA_LIB}_LIBRARY)
    if(MAYA_${MAYA_LIB}_LIBRARY)
        add_library(Maya::${MAYA_LIB} UNKNOWN IMPORTED)
        set_target_properties(Maya::${MAYA_LIB} PROPERTIES
            IMPORTED_LOCATION "${MAYA_${MAYA_LIB}_LIBRARY}")
        set_property(TARGET Maya::Maya APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Maya::${MAYA_LIB})
        list(APPEND MAYA_LIBRARIES "${MAYA_${MAYA_LIB}_LIBRARY}")
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Maya REQUIRED_VARS MAYA_INCLUDE_DIRS MAYA_LIBRARIES)
mark_as_advanced(MAYA_INCLUDE_DIRS MAYA_LIBRARIES MAYA_INCLUDE_DIR)

function(MAYA_PLUGIN _target)
    if(WIN32)
        set_target_properties(${_target} PROPERTIES
            LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin")
    endif()
    set_target_properties(${_target} PROPERTIES
        PREFIX ""
        SUFFIX ${MAYA_PLUGIN_EXTENSION})
endfunction()
