HEADERS += include/LauncherConfig.h include/Gridifier.h include/Utility.h include/CheckPointTree.h include/CheckPointTreeItem.h include/chkptvariableform.h include/JobStatusThread.h include/ProgressBarThread.h include/jobmetadata.h include/qmdcodec.h
SOURCES	+= src/main.cpp src/LauncherConfig.cpp src/Gridifier.cpp src/Utility.cpp src/CheckPointTree.cpp src/CheckPointTreeItem.cpp src/chkptvariableform.cpp src/JobStatusThread.cpp src/ProgressBarThread.cpp src/jobmetadata.cpp gsoap_generated_code/checkPointTreeC.cpp gsoap_generated_code/checkPointTreeClient.cpp gsoap_generated_code/stdsoap2.cpp src/qmdcodec.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= ui/reglauncher.ui ui/componentlauncher.ui ui/RunningJobsDialog.ui ui/textviewdialog.ui
TEMPLATE	= app
CONFIG	+= qt warn_on release thread opengl x11
DBFILE	= reg_qt_launcher.db
LANGUAGE	= C++
INCLUDEPATH = include gsoap_generated_code /usr/include/libxml2  ${REG_STEER_HOME}/include
LIBS += -lxml2 -L${REG_STEER_HOME}/lib32 -lReG_Steer
