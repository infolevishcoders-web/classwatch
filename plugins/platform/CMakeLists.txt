if(VEYON_BUILD_WINDOWS)
	add_subdirectory(windows)
endif()

if(VEYON_BUILD_LINUX AND NOT APPLE)
	add_subdirectory(linux)
endif()
