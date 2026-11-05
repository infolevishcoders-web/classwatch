include(BuildVeyonApplication)

build_veyon_application(classwatch-service
	NAME Service
	SOURCES
	src/main.cpp)

if(VEYON_BUILD_LINUX)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/classwatch.service.in ${CMAKE_CURRENT_BINARY_DIR}/classwatch.service @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/classwatch.service DESTINATION ${SYSTEMD_SERVICE_INSTALL_DIR})
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/io.levish.classwatch-service.conf DESTINATION "${CMAKE_INSTALL_DATADIR}/dbus-1/system.d")
endif()
