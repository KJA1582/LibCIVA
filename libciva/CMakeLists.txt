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

option(SHARED "Shared" OFF)

set(header
  "include/config/config.h"
	"include/ins/ins.h"
	"include/logger/logger.h"
	"include/types/types.h"
	"include/varManager/varManager.h"
)
set(sources
	"src/config/config.cpp"
	"src/ins/modes/align.cpp"
	"src/ins/ins.cpp"
	"src/ins/insDisplay.cpp"
	"src/ins/insEvent.cpp"
	"src/ins/insSetGet.cpp"
	"src/ins/insUpdates.cpp"
	"src/logger/logger.cpp"
	"src/types/types.cpp"
	"src/varManager/varManager.cpp"
)

if(SHARED)
	message("Building shared")
	add_library(libciva SHARED ${sources} ${headers})
else()
	add_library(libciva ${sources} ${headers})
endif()
set_property(TARGET libciva PROPERTY CXX_STANDARD 14)
target_compile_definitions(libciva PRIVATE WORK_DIR="./")
target_include_directories(libciva PRIVATE "include")
