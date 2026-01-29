# CMakeList.txt : CMake project for libciva, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("libciva")

add_library(libciva "ins/ins.cpp" "config/config.cpp" "varManager/varManager.cpp" "types/types.cpp"  "ins/insSetGet.cpp" "ins/insEvent.cpp" "ins/modes/align.cpp" "ins/insDisplay.cpp" "logger/logger.cpp")
set_property(TARGET libciva PROPERTY CXX_STANDARD 14)
target_compile_definitions(libciva PRIVATE WORK_DIR="./")
