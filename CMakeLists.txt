# CMakeList.txt : CMake project for libciva, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("libciva")

# CIVA lib
add_library(libciva "libciva/libciva.cpp" "libciva/libciva.h" "libciva/ins/ins.cpp" "libciva/ins/ins.h" "libciva/varManager/varManager.h"  "libciva/config/config.h" "libciva/config/config.cpp" "libciva/varManager/varManager.cpp" "libciva/geoutils/geoutils.cpp" "libciva/geoutils/geoutils.h" "libciva/ins/insSetGet.cpp" "libciva/ins/insEvent.cpp" "libciva/ins/modes/align.cpp" "libciva/ins/insDisplay.cpp")
set_property(TARGET libciva PROPERTY CXX_STANDARD 14)
target_include_directories(libciva PRIVATE "libciva")
target_compile_definitions(libciva PRIVATE WORK_DIR="./")

# WIN
add_executable(libcivaWin "libcivaWin/libcivaWin.cpp" "libcivaWin/libcivaWin.h" "libcivaWin/winVarManager.cpp" "libcivaWin/winVarManager.h")
set_property(TARGET libcivaWin PROPERTY CXX_STANDARD 14)
target_include_directories(libcivaWin PRIVATE "libciva")
target_link_libraries(libcivaWin PRIVATE libciva)
target_compile_definitions(libcivaWin PRIVATE WORK_DIR="./")
