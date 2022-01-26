if(MSVC)
	cmake_minimum_required (VERSION 3.21) # 3.21 -> has vs 2022
else()
	cmake_minimum_required (VERSION 3.15)
endif()

set(CMAKE_CXX_STANDARD 20)

project("SFPGenerator" VERSION 0.2.0 LANGUAGES CXX)

if(WIN32)
option(SFPG_RIGHTCLICK ON)
endif()

# place all predefined target projects into a filter
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "Targets")

# SFPGenerator executable
set (SRC_SFPG 
	src/SFPG.cpp
)

add_executable ( ${PROJECT_NAME} 
	${SRC_SFPG}
)

# Right click menu install and uninstall executable
if(${SFPG_RIGHTCLICK})
set (SRC_rightclick
	src/WinRightclickMenu.cpp
)

add_executable(${PROJECT_NAME}_rightclick
	${SRC_rightclick}
)

# Applications needs admin rights in order to edit registry keys
set_target_properties(${PROJECT_NAME}_rightclick PROPERTIES LINK_FLAGS " /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" ")

endif()


target_include_directories(${PROJECT_NAME} PUBLIC "include/" "ipp_src/")

# place files into the correct filters
source_group("src" FILES ${SRC})
source_group("include" FILES ${INCLUDE})

# setup CARG
add_subdirectory("submodules/CARG")

get_property(PROJECT_TARGETS DIRECTORY "${subdir}" PROPERTY BUILDSYSTEM_TARGETS)

foreach(T IN ITEMS ${PROJECT_TARGETS})
	message(${T})

	if(MSVC)
		target_compile_options(${T}  PUBLIC "/ZI")
		target_link_options(${T}  PUBLIC "/INCREMENTAL")
	endif()
	
	
	target_link_libraries(${T} CARG)
	target_include_directories(${T} PUBLIC "${CMAKE_SOURCE_DIR}/submodules/CARG/include")
endforeach()


# setup install

install(TARGETS
	${PROJECT_NAME}
DESTINATION ./)

install(FILES
	install_files/Templates.csv
DESTINATION ./)


if(${SFPG_RIGHTCLICK})
	install(TARGETS
		${PROJECT_NAME}_rightclick
	DESTINATION ./)

	install(FILES
		scripts/complete_uninstall.bat
		scripts/setup_rightclick.bat
		scripts/remove_rightclick.bat
		install_files/QuickTemplates.csv
	DESTINATION ./)
endif()

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-installer")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
include(CPack)
