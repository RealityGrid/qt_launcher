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

  QString getSGSFactories(const QString &topLevelRegistry, const QString &desiredContainer);
  void getSGSies(const QString &topLevelRegistry, QTable *gshTagTable);
  QString makeSGSFactory(const QString &container, const QString &topLevelRegistry);
  QString makeSimSGS(const QString &factory, const QString &tag, const QString &topLevelRegistry, const QString &checkPointGSH, const QString &inputFileName, const QString &optionalChkPtTag);
  QString makeVizSGS(const QString &factory, const QString &tag, const QString &topLevelRegistry, const QString &simGSH);
  QString checkPointAndStop(const QString &sgsGSH);

  void makeReGScriptConfig(const QString &filename, const LauncherConfig &config);
  void launchSimScript(const QString &scriptConfigFileName, int timeToRun, const QString &checkPointDataFile = NULL);
  void launchVizScript(const QString &scriptConfigFileName);

  void copyCheckPointFiles(const QString &host);
  void gsiFtp(const QString &file, const QString &destination);

  void setApplication(QApplication *);

private:
  QProcess *getSGSFactoriesProcess;
  QProcess *getSGSiesProcess;
  QProcess *makeSGSFactoryProcess;
  QProcess *makeSimSGSProcess;
  QProcess *makeVizSGSProcess;
  QProcess *launchSimScriptProcess;
  QProcess *launchVizScriptProcess;
  QProcess *jobStatusProcess;

  // Keep a reference to the sgs/tag listview
  QTable *gshTagTable;

  QApplication *mApplication;
  
public slots:
  QString getSGSFactoriesProcessEnded(const QString &desiredContainer);
  void getSGSiesProcessEnded();
  QString makeSGSFactoryProcessEnded();
  QString makeSimSGSProcessEnded();
  QString makeVizSGSProcessEnded();
  void gsiFtpStdoutSlot();
  void gsiFtpStderrSlot();
  
};


#endif