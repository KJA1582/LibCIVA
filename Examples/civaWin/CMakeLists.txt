# CMakeList.txt : CMake project for civaWin, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD  17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project ("civaWin")

add_executable(civaWin
	"src/civaWin.cpp"
	"src/lateralAutopilot.cpp"
	"src/varManager.cpp"
	"src/verticalAutopilot.cpp"
)

target_include_directories(civaWin PRIVATE
	"../../libciva/include"
	"$ENV{MSFS2024_SDK}/SimConnect SDK/include"
	"$ENV{MSFS2024_SDK}/WASM/include"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_link_directories(civaWin PRIVATE
		"../../libciva/out/x64/Release"
	)
else()
	target_link_directories(civaWin PRIVATE
		"../../libciva/out/x64/Debug"
	)
endif()
target_link_directories(civaWin PRIVATE
	"$ENV{MSFS_SDK}/SimConnect SDK/lib"
)

target_link_libraries(civaWin PRIVATE libciva SimConnect.lib)

target_compile_definitions(civaWin PRIVATE __INTELLISENSE__  CIVA_WIN WORK_DIR="./" _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

target_compile_options(civaWin PRIVATE /W4)