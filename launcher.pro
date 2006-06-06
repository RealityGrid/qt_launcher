TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release thread opengl x11

# Check that ReG steering env. is setup
STEER_HOME = $$(REG_STEER_HOME)
isEmpty( STEER_HOME ){
  error("REG_STEER_HOME environment variable not set")
}

LIBS	+= -lxml2 -L${REG_STEER_HOME}/lib32
# We check for existance of ReG_Steer_Utils library as indication
# of whether we have version 1.2 or version 2.0+ of steering
# library
exists( $$(REG_STEER_HOME)/lib32/libReG_Steer_Utils* ){
  message("ReG_Steer_Utils library found")
  DEFINES     += WITH_OPENSSL
  LIBS        += -lReG_Steer_Utils -lssl -lcrypto
}
LIBS += -lReG_Steer

INCLUDEPATH += include /usr/include/libxml2 ${REG_STEER_HOME}/include

# The $$() notation ensures that the environment variable
# is expanded and used in the qmake expression
!exists( $$(HOME)/RealityGrid/etc/launcher.conf ){
  message("launcher.conf isn't already installed")
  CONF_FILES = conf/launcher.conf
}
!exists( $$(HOME)/RealityGrid/etc/security.conf ){
  message("security.conf isn't already installed")
  isEmpty(CONF_FILES){
    CONF_FILES = conf/security.conf
  }
  else{
    CONF_FILES += conf/security.conf
  }
}
!isEmpty( CONF_FILES ){
  message("Creating install target for config file(s)")
  config_files.path = ${HOME}/RealityGrid/etc
  config_files.files = $$join(CONF_FILES, " ", " ")
  INSTALLS += config_files
}

HEADERS	+= include/LauncherConfig.h \
	include/Gridifier.h \
	include/Utility.h \
	include/CheckPointTree.h \
	include/CheckPointTreeItem.h \
	include/chkptvariableform.h \
	include/JobStatusThread.h \
	include/ProgressBarThread.h \
	include/jobmetadata.h \
	include/qmdcodec.h

SOURCES	+= src/main.cpp \
	src/LauncherConfig.cpp \
	src/Gridifier.cpp \
	src/Utility.cpp \
	src/CheckPointTree.cpp \
	src/CheckPointTreeItem.cpp \
	src/chkptvariableform.cpp \
	src/JobStatusThread.cpp \
	src/ProgressBarThread.cpp \
	src/jobmetadata.cpp \
	src/qmdcodec.cpp

FORMS	= ui/reglauncher.ui \
	ui/componentlauncher.ui \
	ui/RunningJobsDialog.ui \
	ui/textviewdialog.ui \
	ui/GlobalParamConstructionForm.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
