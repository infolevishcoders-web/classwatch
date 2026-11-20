include(BuildVeyonPlugin)

build_veyon_plugin(textmessage
	NAME TextMessage
	SOURCES
	TextMessageFeaturePlugin.cpp
	TextMessageDialog.cpp
	TextMessageDialog.ui
	StickyMessageWidget.cpp
	TextMessageFeaturePlugin.h
	TextMessageDialog.h
	StickyMessageWidget.h
	textmessage.qrc
	)
