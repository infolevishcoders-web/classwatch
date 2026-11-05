include(BuildVeyonApplication)

build_veyon_application(classwatch-client
	NAME ClassWatchClient
	SOURCES
	src/ClassWatchClientApp.cpp
	src/ClassWatchClientApp.h
	src/ClassWatchClientStatus.cpp
	src/ClassWatchClientStatus.h
	src/classwatch-client.cpp
	)
