include(BuildVeyonApplication)

build_veyon_application(classwatch-worker
	NAME Worker
	SOURCES
	src/FeatureWorkerManagerConnection.cpp
	src/FeatureWorkerManagerConnection.h
	src/main.cpp
	src/VeyonWorker.cpp
	src/VeyonWorker.h
	)
