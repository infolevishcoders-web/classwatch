include(WindowsBuildHelpers)

file(GLOB veyoncore_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui
	${CMAKE_CURRENT_SOURCE_DIR}/src/Configuration/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Configuration/*.h
	)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/builddata.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/builddata.qrc)
set(core_RESOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/core.qrc
	${CMAKE_CURRENT_BINARY_DIR}/builddata.qrc
	)

add_library(classwatch-core SHARED ${veyoncore_SOURCES} ${core_RESOURCES} ${libvncclient_SOURCES})

target_compile_definitions(classwatch-core PRIVATE XK_KOREAN veyon_core_EXPORTS)

set_default_target_properties(classwatch-core)

target_compile_options(classwatch-core PRIVATE -Wno-parentheses)

target_include_directories(classwatch-core PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}/src
	)

target_link_libraries(classwatch-core PUBLIC
	Qt${QT_MAJOR_VERSION}::Concurrent
	Qt${QT_MAJOR_VERSION}::Gui
	Qt${QT_MAJOR_VERSION}::Network
	Qt${QT_MAJOR_VERSION}::Widgets
	qca-qt${QT_MAJOR_VERSION}
	)

if(WITH_QT6)
	# required by qca-qt6
	target_link_libraries(classwatch-core PUBLIC Qt6::Core5Compat)
endif()

if(WITH_TESTS)
	target_link_libraries(classwatch-core PUBLIC Qt${QT_MAJOR_VERSION}::Test)
endif()

target_link_libraries(classwatch-core PUBLIC OpenSSL::SSL)

if(WITH_BUNDLED_LIBVNC)
	target_link_libraries(classwatch-core PUBLIC vncclient)
else()
	target_link_libraries(classwatch-core PUBLIC LibVNC::LibVNCClient)
endif()

if(VEYON_BUILD_WINDOWS)
	# add Windows Socket library required by libvncclient
	target_link_libraries(classwatch-core PRIVATE -lws2_32)
	set_target_properties(classwatch-core PROPERTIES PREFIX "")
	add_windows_resources(classwatch-core
		NAME ClassWatchCore
		DESCRIPTION "ClassWatch Core Library")
	if(NOT WITH_CORE_ONLY)
		install(TARGETS classwatch-core RUNTIME DESTINATION ${VEYON_LIB_DIR})
	endif()
else()
	if(NOT WITH_CORE_ONLY)
		install(TARGETS classwatch-core LIBRARY DESTINATION ${VEYON_LIB_DIR})
	endif()
endif()

if(VEYON_BUILD_WINDOWS)
	target_compile_definitions(classwatch-core PUBLIC _WIN32_WINNT=0x0602)
endif()

if(WITH_PCH)
	set(PCH ${CMAKE_CURRENT_SOURCE_DIR}/src/PrecompiledHeader.h)
	target_precompile_headers(classwatch-core PRIVATE ${PCH})

	add_pch_target(classwatch-application-pch ${PCH})
	set_target_properties(classwatch-application-pch PROPERTIES COMPILE_OPTIONS "${CMAKE_COMPILE_OPTIONS_PIE}")
	set_target_properties(classwatch-application-pch PROPERTIES LINK_OPTIONS "${CMAKE_LINK_OPTIONS_PIE}")

	add_pch_target(classwatch-library-pch ${PCH})
	set_target_properties(classwatch-library-pch PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
