include(BuildVeyonApplication)

file(GLOB master_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)

set(master_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/master.qrc)
if(VEYON_DEBUG)
	set(master_RESOURCES ${master_RESOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/examples.qrc)
endif()

build_veyon_application(classwatch-master
	NAME Master
	WINDOWS_ICON data/classwatch-master.ico
	SOURCES
	${master_SOURCES}
	${master_RESOURCES})

if(VEYON_BUILD_LINUX)
	xdg_install(${CMAKE_CURRENT_BINARY_DIR}/data/classwatch-master.desktop ${CMAKE_CURRENT_SOURCE_DIR}/data/classwatch-master.xpm ${CMAKE_CURRENT_SOURCE_DIR}/data/classwatch-master.png ${CMAKE_CURRENT_SOURCE_DIR}/data/classwatch-master.svg)
endif()
