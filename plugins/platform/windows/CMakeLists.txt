include(BuildVeyonPlugin)

set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)

build_veyon_plugin(windows-platform
	NAME WindowsPlatform
	SOURCES
	WindowsPlatformPlugin.cpp
	WindowsPlatformConfigurationPage.h
	WindowsPlatformConfigurationPage.cpp
	WindowsPlatformConfigurationPage.ui
	WindowsCoreFunctions.cpp
	WindowsDeviceFunctions.cpp
	WindowsFilesystemFunctions.cpp
	WindowsInputDeviceFunctions.cpp
	${CMAKE_SOURCE_DIR}/3rdparty/interception/library/interception.c
	WindowsKeyboardShortcutTrapper.cpp
	WindowsNetworkFunctions.cpp
	WindowsServerProcess.cpp
	WindowsServiceControl.cpp
	WindowsServiceCore.cpp
	WindowsServiceFunctions.cpp
	WindowsSessionFunctions.cpp
	WindowsUserFunctions.cpp
	WtsSessionManager.cpp
	DesktopInputController.cpp
	SasEventListener.cpp
	XEventLog.cpp
	${ultravnc_DIR}/winvnc/winvnc/vnckeymap.cpp
	${ultravnc_DIR}/addon/ms-logon/authSSP/authSSP.cpp
	${ultravnc_DIR}/addon/ms-logon/authSSP/GenClientServerContext.cpp

	WindowsPlatformConfiguration.h
	WindowsPlatformPlugin.h
	WindowsCoreFunctions.h
	WindowsDeviceFunctions.h
	WindowsFilesystemFunctions.h
	WindowsInputDeviceFunctions.h
	WindowsKeyboardShortcutTrapper.h
	WindowsNetworkFunctions.h
	WindowsServerProcess.h
	WindowsServiceControl.h
	WindowsServiceCore.h
	WindowsServiceFunctions.h
	WindowsSessionFunctions.h
	WindowsSmartObjects.h
	WindowsUserFunctions.h
	WtsSessionManager.h
	DesktopInputController.h
	SasEventListener.h
	../common/LogonHelper.h
	../common/LogonHelper.cpp
	../common/PersistentLogonCredentials.h
	../common/PersistentLogonCredentials.cpp
	../common/PlatformSessionManager.h
	../common/PlatformSessionManager.cpp
	../common/ServiceDataManager.h
	../common/ServiceDataManager.cpp
	XEventLog.h
	windows.qrc
	)

target_include_directories(windows-platform PRIVATE
	../common
	${CMAKE_SOURCE_DIR}/3rdparty/interception/library
	${ultravnc_DIR}/addon/ms-logon/authSSP
	${ultravnc_DIR}/winvnc/winvnc
	${ultravnc_DIR}
	)

target_link_libraries(windows-platform PRIVATE -lws2_32 -lwtsapi32 -lnetapi32 -luserenv -limm32 -lntdll -liphlpapi -lsetupapi -lsecur32)
target_link_libraries(windows-platform PRIVATE Qt6::GuiPrivate)
target_compile_definitions(windows-platform PRIVATE ULTRAVNC_VEYON_SUPPORT INTERCEPTION_STATIC
	GWL_USERDATA=GWLP_USERDATA GWL_WNDPROC=GWLP_WNDPROC GWL_HINSTANCE=GWLP_HINSTANCE
	GWL_STYLE=GWLP_STYLE GWL_EXSTYLE=GWLP_EXSTYLE DWL_MSGRESULT=DWLP_MSGRESULT)
if(VEYON_BUILD_WIN64)
	target_compile_definitions(windows-platform PRIVATE _X64)
endif()
target_compile_options(windows-platform PRIVATE
	"-Wno-unknown-pragmas;-flarge-source-files;-Wno-misleading-indentation;-Wno-unused-variable;-Wno-unused-label;-Wno-unused-but-set-variable;-Wno-error=unused-but-set-variable;-Wno-error=unused-variable;-Wno-error=unused-label")
set_source_files_properties(WindowsNetworkFunctions.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE SKIP_PRECOMPILE_HEADERS TRUE)
set_target_properties(windows-platform PROPERTIES UNITY_BUILD OFF)
