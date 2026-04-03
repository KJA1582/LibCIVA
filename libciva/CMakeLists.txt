# CMakeList.txt : CMake project for libciva, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project ("libciva")

option(SHARED "Shared" OFF)

set(header
  "include/config/config.h"
	"include/ins/ins.h"
	"include/insContainer/insContainer.h"
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
)

if(SHARED)
	message("Building shared")
	add_library(libciva SHARED ${sources} ${headers})

	target_compile_definitions(libciva PRIVATE SHARED)
else()
	add_library(libciva ${sources} ${headers})
endif()

target_include_directories(libciva PRIVATE "include")

target_compile_definitions(libciva PRIVATE WORK_DIR="./")

if(MSVC)
	target_compile_definitions(libciva PRIVATE __INTELLISENSE__)

	target_compile_options(libciva PRIVATE /W4)
else()
	target_compile_options(libciva PRIVATE -Wall -Wextra -pedantic)
endif()