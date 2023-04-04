
unix {

	isEmpty(PREFIX) {
		PREFIX = /usr
	}
	BINDIR = $$PREFIX/lib/qt5/plugins/platforminputcontexts

	INSTALLS += target
	target.path = $$BINDIR
}

