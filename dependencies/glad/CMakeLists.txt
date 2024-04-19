cmake_minimum_required(VERSION 3.13)

add_library(glad "")

target_sources(glad
	PUBLIC
		include/glad/glad.h
	PRIVATE
		include/KHR/khrplatform.h
		src/glad.c
)

target_include_directories(glad
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(glad PROPERTIES FOLDER "Dependencies")
