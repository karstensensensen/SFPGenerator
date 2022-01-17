if(MSVC)
	cmake_minimum_required (VERSION 3.21) # 3.21 -> has vs 2022
else()
	cmake_minimum_required (VERSION 3.0.0)
endif()
set(CMAKE_CXX_STANDARD 20)

project(Straight Forward Project Generator CXX)

# place all predefined target projects into a filter
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "Targets")

set (SRC
	src/SFPG.cpp
)

set (INCLUDE
	
)

# Add source to this project's executable.
add_executable ( ${PROJECT_NAME} 
	${INCLUDE} ${SRC}
)

if(MSVC)
  target_compile_options(${PROJECT_NAME}  PUBLIC "/ZI")
  target_link_options(${PROJECT_NAME}  PUBLIC "/INCREMENTAL")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} "include/" "ipp_src/")

# place files into the correct filters
source_group("src" FILES ${SRC})
source_group("include" FILES ${INCLUDE})
