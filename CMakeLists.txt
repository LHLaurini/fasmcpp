cmake_minimum_required(VERSION 3.0)
project("fasmcpp")

if (MSVC)
	string(FIND ${CMAKE_GENERATOR} Win64 WIN64)
	if (${WIN64} GREATER -1)
		set(ARCH "x64")
	else()
		set(ARCH "x86")
	endif()
endif()

find_program(FASM_PATH fasm REQUIRED)

if (NOT EXISTS ${FASM_PATH})
	message(SEND_ERROR "Flat assembler not found!")
endif()

if (WIN32)
	if (ARCH STREQUAL x64)
		set(WIN64_LOW_MEMORY_FIXED_ADDRESS 0x00010000 CACHE STRING "This is a fixed address which must reside in 32-bit accessible memory where the Win64 FASM binary is loaded.")
		set(FASM_SOURCE fasmcpp.win64.asm)
		set(FASM_OUTPUT fasmcpp.win64.bin)
		set(FASM_BINARY fasmcpp.win64.bin)
		set(FASM_DEFINITIONS -DWIN64_LOW_MEMORY_FIXED_ADDRESS=${WIN64_LOW_MEMORY_FIXED_ADDRESS})
		set(FASM_FILES ${FASM_SOURCE} fasmcpp.win64.cpp fasmcpp.win64.h)
		set(CPP_DEFINITIONS PRIVATE WIN64_LOW_MEMORY_FIXED_ADDRESS=${WIN64_LOW_MEMORY_FIXED_ADDRESS})
		set(CPP_LINK_LIBARIES)
	elseif(ARCH STREQUAL x86)
		set(FASM_SOURCE fasmcpp.win32.asm)
		set(FASM_OUTPUT fasmcpp.win32.obj)
		set(FASM_BINARY fasmcpp.win32.obj.lib)
		set(FASM_DEFINITIONS)
		set(FASM_FILES ${FASM_SOURCE} fasmcpp.win32.cpp fasmcpp.win32.h)
		set(CPP_DEFINITIONS)
		set(CPP_LINK_LIBARIES ${CMAKE_CURRENT_BINARY_DIR}/${FASM_BINARY})
		configure_file(fasmcpp.win32.objfix.asm.in "${CMAKE_CURRENT_BINARY_DIR}/fasmcpp.win32.objfix.asm" @ONLY)
	endif()
endif()

get_filename_component(FASM_DIR "${FASM_PATH}" DIRECTORY)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FASM_OUTPUT}
                   COMMAND ${CMAKE_COMMAND} -E env "INCLUDE=${FASM_DIR}/source" "${FASM_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/${FASM_SOURCE}" "${CMAKE_CURRENT_BINARY_DIR}/${FASM_OUTPUT}" ${FASM_DEFINITIONS}
                   DEPENDS ${FASM_SOURCE})
if (WIN32 AND ARCH STREQUAL x86)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FASM_BINARY}
	                   COMMAND "${FASM_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/fasmcpp.win32.objfix.asm" "${CMAKE_CURRENT_BINARY_DIR}/${FASM_BINARY}"
	                   DEPENDS fasmcpp.win32.objfix.asm.in ${CMAKE_CURRENT_BINARY_DIR}/${FASM_OUTPUT})
endif()
add_custom_target(fasm ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${FASM_BINARY})

add_library(fasmcpp fasmcpp.cpp fasmcpp.h
                    fasmIO.cpp fasmIO.h
                    fasmFunctions.cpp fasmFunctions.h
                    ${FASM_FILES})

if (CPP_DEFINITIONS)
	target_compile_definitions(fasmcpp ${CPP_DEFINITIONS})
endif()

if (CPP_LINK_LIBARIES)
	target_link_libraries(fasmcpp ${CPP_LINK_LIBARIES})
endif()

add_dependencies(fasmcpp fasm)

add_executable(Example example.cpp)
target_link_libraries(Example fasmcpp)

install(TARGETS fasmcpp DESTINATION lib)
install(FILES fasmcpp.h fasmIO.h DESTINATION include)

set(NAME "FASMCPP")
set(INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fasmcpp${CMAKE_STATIC_LIBRARY_SUFFIX})

if (WIN32 AND ARCH STREQUAL x64)
	set(BINARY ${CMAKE_INSTALL_PREFIX}/redist/${FASM_BINARY})
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${FASM_BINARY}" DESTINATION redist)
else()
	list(APPEND LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/${FASM_BINARY})
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${FASM_BINARY}" DESTINATION lib)
endif()

configure_file(fasmcppConfig.cmake.in ${CMAKE_BINARY_DIR}/fasmcppConfig.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/fasmcppConfig.cmake DESTINATION .)
