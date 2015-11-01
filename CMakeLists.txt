# --- Script Setup

cmake_minimum_required (VERSION 2.8)

# Use Debug build by default.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Select build type. Options are: 'Debug', 'Release', 'RelWithDebInfo' and 'MinSizeRel'."
		FORCE
	)
	message(STATUS "Set CMAKE_BUILD_TYPE to default '${CMAKE_BUILD_TYPE}'.")
endif(NOT CMAKE_BUILD_TYPE)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# Disable in-source builds and modifications
# to the source tree.
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

enable_testing()

project(llvm-abi C CXX)

# Add path for custom modules
set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

# --- Project Properties

set(LLVMABI_VERSION_MAJOR 1)
set(LLVMABI_VERSION_MINOR 0)
set(LLVMABI_VERSION_PATCH 0)
set(LLVMABI_VERSION_BUILD 0)
set(LLVMABI_VERSION ${LLVMABI_VERSION_MAJOR}.${LLVMABI_VERSION_MINOR}.${LLVMABI_VERSION_PATCH}.${LLVMABI_VERSION_BUILD})

message(STATUS "Building llvm-abi version ${LLVMABI_VERSION} using build type '${CMAKE_BUILD_TYPE}'.")
message(STATUS "    Source directory is '${PROJECT_SOURCE_DIR}'.")
message(STATUS "    Build directory is '${PROJECT_BINARY_DIR}'.")
message(STATUS "    Install prefix is '${CMAKE_INSTALL_PREFIX}'.")

# --- Compiler Flags

add_definitions(
	-D__STDC_LIMIT_MACROS
	-D__STDC_CONSTANT_MACROS
)

# Enable most warnings.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wshadow -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings")

# Use C++11.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Add version as preprocessor defines.
add_definitions(
	"-DLLVMABI_VERSION=${LLVMABI_VERSION}"
	"-DLLVMABI_VERSION_MAJOR=${LLVMABI_VERSION_MAJOR}"
	"-DLLVMABI_VERSION_MINOR=${LLVMABI_VERSION_MINOR}"
	"-DLLVMABI_VERSION_PATCH=${LLVMABI_VERSION_PATCH}"
	"-DLLVMABI_VERSION_BUILD=${LLVMABI_VERSION_BUILD}"
)

# --- User config

set(LLVM_ROOT_DIR "" CACHE STRING "Set LLVM root directory.")

if(NOT "${LLVM_ROOT_DIR}" STREQUAL "")
	message(STATUS "Using LLVM_ROOT_DIR=${LLVM_ROOT_DIR}")
else(NOT "${LLVM_ROOT_DIR}" STREQUAL "")
	message(STATUS "LLVM_ROOT_DIR wasn't specified (you can use this to search for LLVM in a particular path).")
endif(NOT "${LLVM_ROOT_DIR}" STREQUAL "")

# --- Dependencies

# Find LLVM (for code generation).
set(LLVM_REQ_COMPONENTS all-targets bitreader bitwriter core irreader jit linker mcjit nativecodegen native ipo option)
find_package(LLVM REQUIRED ${LLVM_REQ_COMPONENTS})

# Check LLVM version.
set(LLVM_VERSION_SIMPLE_STRING "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}")

if("${LLVM_VERSION_SIMPLE_STRING}" VERSION_LESS "3.3")
	message(FATAL_ERROR "LLVM version ${LLVM_VERSION_SIMPLE_STRING} is not supported (first supported version is 3.3).")
elseif("${LLVM_VERSION_SIMPLE_STRING}" VERSION_GREATER "3.7")
	message(WARNING "LLVM version ${LLVM_VERSION_SIMPLE_STRING} is not officially supported for this release (last supported version is 3.7).")
endif()

math(EXPR LLVMABI_LLVM_VERSION ${LLVM_VERSION_MAJOR}*100+${LLVM_VERSION_MINOR})
message(STATUS "Using LLVM ${LLVM_VERSION_SIMPLE_STRING} (version integer is ${LLVMABI_LLVM_VERSION}).")

add_definitions(
	-DLLVMABI_LLVM_VERSION=${LLVMABI_LLVM_VERSION}
)

set(LLVMABI_LLVM_INCLUDE_DIRS "${LLVM_INCLUDE_DIRS}" CACHE STRING "Set LLVM include directories.")
set(LLVMABI_LLVM_LIBRARY_DIRS "${LLVM_LIBRARY_DIRS}" CACHE STRING "Set LLVM library directories.")

message(STATUS "    LLVM binary directory: ${LLVM_BINARY_DIR}")
message(STATUS "        llc path: ${LLVM_LLC_EXECUTABLE}")
message(STATUS "        llvm-dis path: ${LLVM_DIS_EXECUTABLE}")
message(STATUS "        llvm-link path: ${LLVM_LINK_EXECUTABLE}")
message(STATUS "        llvm-nm path: ${LLVM_NM_EXECUTABLE}")
message(STATUS "        opt path: ${LLVM_OPT_EXECUTABLE}")

message(STATUS "    LLVM include directories: ${LLVMABI_LLVM_INCLUDE_DIRS}")
message(STATUS "        (if this is wrong, specify LLVMABI_LLVM_INCLUDE_DIRS)")
include_directories(SYSTEM ${LLVMABI_LLVM_INCLUDE_DIRS})

message(STATUS "    LLVM library directories: ${LLVMABI_LLVM_LIBRARY_DIRS}")
message(STATUS "        (if this is wrong, specify LLVMABI_LLVM_LIBRARY_DIRS)")
link_directories(${LLVMABI_LLVM_LIBRARY_DIRS})

# --- Subdirectories

include_directories(include)

add_subdirectory(lib)
add_subdirectory(test)

set(LLVMABI_PUBLIC_HEADERS
	include/llvm-abi/ABI.hpp
	include/llvm-abi/ABITypeInfo.hpp
	include/llvm-abi/ArgInfo.hpp
	include/llvm-abi/ArgumentIRMapping.hpp
	include/llvm-abi/Builder.hpp
	include/llvm-abi/Callee.hpp
	include/llvm-abi/Caller.hpp
	include/llvm-abi/CallingConvention.hpp
	include/llvm-abi/DataSize.hpp
	include/llvm-abi/FunctionEncoder.hpp
	include/llvm-abi/FunctionIRMapping.hpp
	include/llvm-abi/FunctionType.hpp
	include/llvm-abi/Type.hpp
	include/llvm-abi/TypeBuilder.hpp
	include/llvm-abi/TypedValue.hpp
	include/llvm-abi/TypePromoter.hpp
)

install(FILES ${LLVMABI_PUBLIC_HEADERS}
	DESTINATION "include/llvm-abi"
)

set(LLVMABI_X86_PUBLIC_HEADERS
	include/llvm-abi/x86/ArgClass.hpp
	include/llvm-abi/x86/Classification.hpp
	include/llvm-abi/x86/Classifier.hpp
	include/llvm-abi/x86/CPUFeatures.hpp
	include/llvm-abi/x86/CPUKind.hpp
	include/llvm-abi/x86/Win64ABI.hpp
	include/llvm-abi/x86/X86_32ABI.hpp
	include/llvm-abi/x86/X86_32ABITypeInfo.hpp
	include/llvm-abi/x86/X86_32Classifier.hpp
	include/llvm-abi/x86/X86_64ABI.hpp
	include/llvm-abi/x86/X86_64ABITypeInfo.hpp
)

install(FILES ${LLVMABI_X86_PUBLIC_HEADERS}
	DESTINATION "include/llvm-abi/x86"
)
