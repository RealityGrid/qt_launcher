/*----------------------------------------------------------------------------
    Application Class for QT launcher GUI.
    Header

    (C)Copyright 2003 The University of Manchester, United Kingdom,
    all rights reserved.

    This software is produced by the Supercomputing, Visualization &
    e-Science Group, Manchester Computing, the Victoria University of
    Manchester as part of the RealityGrid project.

    This software has been tested with care but is not guaranteed for
    any particular purpose. Neither the copyright holder, nor the
    University of Manchester offer any warranties or representations,
    nor do they accept any liabilities with respect to this software.

    This software must not be used for commercial gain without the
    written permission of the authors.
    
    This software must not be redistributed without the written
    permission of the authors.

    Permission is granted to modify this software, provided any
    modifications are made freely available to the original authors.
 
    Supercomputing, Visualization & e-Science Group
    Manchester Computing
    University of Manchester
    Manchester M13 9PL
    
    WWW:    http://www.sve.man.ac.uk  
    email:  sve@man.ac.uk
    Tel:    +44 161 275 6095
    Fax:    +44 161 275 6800    

    Initial version by: M Riding, 29.09.2003
    
---------------------------------------------------------------------------*/

/** @file Gridifier.h
    @brief Class for wrapping scripts for handling OGSA framework and
           remote job launching */

#ifndef _GRIDIFIER_H_
#define _GRIDIFIER_H_

#include "qlistview.h"
#include "qprocess.h"
#include "qtable.h"

#include "LauncherConfig.h"

class Gridifier: public QObject{

Q_OBJECT

public:
  Gridifier();
  ~Gridifier();

  QString getSGSFactories(const QString &topLevelRegistry, 
			  const QString &desiredContainer,
			  const QString &className);
  QString makeSGSFactory(const QString &container, 
			 const QString &topLevelRegistry,
			 const QString &className);
  QString makeSimSGS(const QString &factory, const LauncherConfig &config);
  QString makeVizSGS(const QString &factory, const LauncherConfig &config);
  QString makeMetaSGS(const QString &factory,
		      const LauncherConfig &config,
		      const QString &parentGSH);
  QString checkPointAndStop(const QString &sgsGSH);
  void getSGSies(const QString &topLevelRegistry, QTable *aGSHTagTable);
  
  void makeReGScriptConfig(const QString &filename, const LauncherConfig &config);
  void launchSimScript(const QString &scriptConfigFileName, const LauncherConfig &config);
  void launchVizScript(const QString &scriptConfigFileName, const LauncherConfig &config);
  void launchArgonneViz(const LauncherConfig &config);
  /** Prototype */
  void webServiceJobSubmit(const QString &scriptConfigFileName);

  void copyCheckPointFiles(const QString &host);
  /** Not used */
  void gsiFtp(const QString &file, const QString &destination);

  /** Supply a pointer to the application object for the component
      to be launched */
  void setApplication(QApplication *);
  /** Set the location of the wrapper scripts */
  void setScriptsDirectory(const QString &dir);

private:
  QProcess *getSGSiesProcess;

  /** Keep a reference to the sgs/tag listview */
  QTable *mGSHTagTable;

  QApplication *mApplication;
  /** Location of the various wrapper scripts */
  QString       mScriptsDir;

public slots:
  void getSGSiesProcessEnded();
  void gsiFtpStdoutSlot();
  void gsiFtpStderrSlot();
  
};


#endif
