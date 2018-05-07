# - Find LLVM headers and libraries.
# This module locates LLVM and adapts the llvm-config output for use with
# CMake.
#
# A given list of COMPONENTS is passed to llvm-config.
#
# The following variables are defined:
#  LLVM_FOUND		   - true if LLVM was found
#  LLVM_BINARY_DIR	  - Directory containing LLVM tools binaries.
#  LLVM_CXXFLAGS		- C++ compiler flags for files that include LLVM headers.
#  LLVM_HOST_TARGET	 - Target triple used to configure LLVM.
#  LLVM_INCLUDE_DIRS	- Directory containing LLVM include files.
#  LLVM_LDFLAGS		 - Linker flags to add when linking against LLVM
#						 (includes -LLLVM_LIBRARY_DIRS).
#  LLVM_LIBRARIES	   - Full paths to the library files to link against.
#  LLVM_LIBRARY_DIRS	- Directory containing LLVM libraries.
#  LLVM_ROOT_DIR		- The root directory of the LLVM installation.
#						 llvm-config is searched for in ${LLVM_ROOT_DIR}/bin.
#  LLVM_VERSION_MAJOR   - Major version of LLVM.
#  LLVM_VERSION_MINOR   - Minor version of LLVM.
#  LLVM_VERSION_STRING  - Full LLVM version string (e.g. 2.9).
# 
# These tools are also found:
#  LLVM_DIS_EXECUTABLE  - Path of 'llvm-dis' executable.
#  LLVM_LINK_EXECUTABLE - Path of 'llvm-link' executable.
#  LLVM_LLC_EXECUTABLE  - Path of 'llc' executable.
#  LLVM_NM_EXECUTABLE   - Path of 'llvm-nm' executable.
#  LLVM_OPT_EXECUTABLE  - Path of 'opt' executable.
set(llvm_config_names
	llvm-config-6.0
	llvm-config
)

find_program(LLVM_CONFIG
	NAMES ${llvm_config_names}
	PATHS ${LLVM_ROOT_DIR}/bin NO_DEFAULT_PATH
	DOC "Path to llvm-config tool.")
find_program(LLVM_CONFIG NAMES ${llvm_config_names})

if (NOT LLVM_CONFIG)
	message(FATAL_ERROR "Could not find llvm-config. Try manually setting LLVM_CONFIG to the llvm-config executable of the installation to use.")
endif()

macro(llvm_set var flag)
	if(LLVM_FIND_QUIETLY)
		set(_quiet_arg ERROR_QUIET)
	endif()
	execute_process(
		COMMAND ${LLVM_CONFIG} --${flag}
		OUTPUT_VARIABLE LLVM_${var}
		OUTPUT_STRIP_TRAILING_WHITESPACE
		${_quiet_arg}
	)
	if(${ARGV2})
		file(TO_CMAKE_PATH "${LLVM_${var}}" LLVM_${var})
	endif()
endmacro()
macro(llvm_set_libs var flag)
	if(LLVM_FIND_QUIETLY)
		set(_quiet_arg ERROR_QUIET)
	endif()
	execute_process(
		COMMAND ${LLVM_CONFIG} --${flag} ${LLVM_FIND_COMPONENTS}
		OUTPUT_VARIABLE tmplibs
		OUTPUT_STRIP_TRAILING_WHITESPACE
		${_quiet_arg}
	)
	file(TO_CMAKE_PATH "${tmplibs}" tmplibs)
	string(REGEX MATCHALL "${pattern}[^ ]+" LLVM_${var} ${tmplibs})
endmacro()

llvm_set(VERSION_STRING version)
string(REGEX REPLACE "([0-9]+).*" "\\1" LLVM_VERSION_MAJOR "${LLVM_VERSION_STRING}" )
string(REGEX REPLACE "[0-9]+\\.([0-9]+).*[A-Za-z]*" "\\1" LLVM_VERSION_MINOR "${LLVM_VERSION_STRING}" )

llvm_set(CXXFLAGS cxxflags)

# On CMake builds of LLVM, the output of llvm-config --cxxflags does not
# include -fno-rtti, leading to linker errors. Be sure to add it.
if(NOT ${LLVM_CXXFLAGS} MATCHES "-fno-rtti")
	set(LLVM_CXXFLAGS "${LLVM_CXXFLAGS} -fno-rtti")
endif()

llvm_set(HOST_TARGET host-target)
llvm_set(INCLUDE_DIRS includedir true)
llvm_set(OBJ_ROOT obj-root true)
llvm_set(ROOT_DIR prefix true)

# Also use LLVM_OBJ_ROOT here to support custom builds of LLVM.
list(APPEND LLVM_INCLUDE_DIRS "${LLVM_OBJ_ROOT}/include")

llvm_set(LDFLAGS ldflags)

# System library dependencies (e.g. "-lz") are accessed using the separate
# "--system-libs" flag.
llvm_set(SYSTEM_LIBS system-libs)
string(REPLACE "\n" " " LLVM_LDFLAGS "${LLVM_LDFLAGS} ${LLVM_SYSTEM_LIBS}")

llvm_set(LIBRARY_DIRS libdir true)
llvm_set_libs(LIBRARIES libs)

llvm_set(BINARY_DIR bindir true)

# Use the default CMake facilities for handling QUIET/REQUIRED.
include(FindPackageHandleStandardArgs)

set(LLVM_BINARIES_LIST "")

find_program(LLVM_DIS_EXECUTABLE
	NAMES llvm-dis
	PATHS ${LLVM_BINARY_DIR}
	DOC "Path to llvm-dis executable"
	NO_DEFAULT_PATH
)

list(APPEND LLVM_BINARIES_LIST "${LLVM_DIS_EXECUTABLE}")

find_program(LLVM_LINK_EXECUTABLE
	NAMES llvm-link
	PATHS ${LLVM_BINARY_DIR}
	DOC "Path to llvm-link executable"
	NO_DEFAULT_PATH
)

list(APPEND LLVM_BINARIES_LIST "${LLVM_LINK_EXECUTABLE}")

find_program(LLVM_LLC_EXECUTABLE
	NAMES llc
	PATHS ${LLVM_BINARY_DIR}
	DOC "Path to llc executable"
	NO_DEFAULT_PATH
)

list(APPEND LLVM_BINARIES_LIST "${LLVM_LLC_EXECUTABLE}")

find_program(LLVM_NM_EXECUTABLE
	NAMES llvm-nm
	PATHS ${LLVM_BINARY_DIR}
	DOC "Path to llvm-nm executable"
	NO_DEFAULT_PATH
)

list(APPEND LLVM_BINARIES_LIST "${LLVM_NM_EXECUTABLE}")

find_program(LLVM_OPT_EXECUTABLE
	NAMES opt
	PATHS ${LLVM_BINARY_DIR}
	DOC "Path to opt executable"
	NO_DEFAULT_PATH
)

list(APPEND LLVM_BINARIES_LIST "${LLVM_OPT_EXECUTABLE}")

find_package_handle_standard_args(LLVM
	REQUIRED_VARS LLVM_ROOT_DIR LLVM_HOST_TARGET LLVM_BINARIES_LIST
	VERSION_VAR LLVM_VERSION_STRING
)
