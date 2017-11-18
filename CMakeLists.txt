cmake_minimum_required(VERSION 3.0)
project(iniparser)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/iniparser.cpp include/iniparser.h)
add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} toolboxpp)

install(
	TARGETS ${PROJECT_NAME}
	DESTINATION lib${LIB_SUFFIX}
)

install(
	FILES ${PUBLIC_HEADERS}
	DESTINATION include/${PROJECT}
)