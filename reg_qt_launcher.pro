HEADERS += include/LauncherConfig.h include/Gridifier.h include/Utility.h include/CheckPointTree.h include/CheckPointTreeItem.h include/chkptvariableform.h include/JobStatusThread.h
SOURCES	+= src/main.cpp src/LauncherConfig.cpp src/Gridifier.cpp src/Utility.cpp src/CheckPointTree.cpp src/CheckPointTreeItem.cpp src/chkptvariableform.cpp src/JobStatusThread.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= ui/reglauncher.ui ui/componentlauncher.ui ui/RunningJobsDialog.ui
TEMPLATE	= app
CONFIG	+= qt warn_on release thread
DBFILE	= reg_qt_launcher.db
LANGUAGE	= C++
INCLUDEPATH = include /usr/include/libxml2 ${HOME}/RealityGrid/reg_gsoap_build ${REG_STEER_HOME}/include
LIBS += -lxml2 -L${HOME}/RealityGrid/reg_gsoap_build -lREGgsoap -L${HOME}/RealityGrid/reg_steer_lib/lib32 -lReG_Steer
