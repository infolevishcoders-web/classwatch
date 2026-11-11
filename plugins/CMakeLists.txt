file(GLOB plugins RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

# Plugins not shipped in ClassWatch: they are not needed in a classroom
# environment and increase the attack surface.
#   ldap      - enterprise directory integration, unused in schools
#   webapi    - unauthenticated HTTP control API, removed for security
#   testing   - developer-only test plugin
set(CLASSWATCH_EXCLUDED_PLUGINS ldap webapi testing)

foreach(plugin ${plugins})
	if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${plugin})
		if(NOT plugin IN_LIST CLASSWATCH_EXCLUDED_PLUGINS)
			add_subdirectory(${plugin})
		endif()
	endif()
endforeach()
