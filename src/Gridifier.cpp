/*----------------------------------------------------------------------------
    Application Class for QT launcher GUI.
    Implementation

    (C)Copyright 2003 The University of Manchester, United Kingdom,
    all rights reserved.

    This software is produced by the Supercomputing, Visualization &
    e-Science Group, Manchester Computing, the Victoria University of
    Manchester as part of the RealityGrid project

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

#include "Gridifier.h"
#include "Utility.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "qapplication.h"
#include "qfile.h"

using namespace std;

/** Constructors
 */
Gridifier::Gridifier(){
  // Seed the random number generator
  srand(time(NULL));
}

Gridifier::~Gridifier(){
}


void Gridifier::setApplication(QApplication *aApplication){
  mApplication = aApplication;
}


/** The following static methods wrap around QProcess objects to
 *  run the reg_perl_launcher scripts. Ultimately replace with
 *  code that uses an API to perform the same function programatically.
 */
QString Gridifier::getSGSFactories(const QString &topLevelRegistry, const QString &desiredContainer){
  QString result;

  getSGSFactoriesProcess = new QProcess(QString("./get_sgs_factories.pl"));
  getSGSFactoriesProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts/"));
  //QString(QDir::homeDirPath()+"/RealityGrid/reg_perl_launcher/get_sgs_factories.pl"));
  getSGSFactoriesProcess->addArgument(topLevelRegistry);

  //connect(getSGSFactoriesProcess, SIGNAL(processExited()), this, SLOT(getSGSFactoriesProcessEnded()));

  getSGSFactoriesProcess->start();

  // At this point we need to wait for the process to complete.
  // Clearly this isn't ideal - hence the desire to replace this with API calls.
  // An alternative is to send the result back via a QT event....
  // for the time being stick with this slightly naff approach
  while (getSGSFactoriesProcess->isRunning()){
    // don't sit in an exhaustive loop - waste of electricity :)
    usleep(10000);
    mApplication->processEvents();
  }

  // this functionality is left in a seperate slot since it could be better
  // to allow the standard qt events to handle it, rather than sitting in the
  // above loop
  result = getSGSFactoriesProcessEnded(desiredContainer);
  
  return result;
}

void Gridifier::getSGSies(const QString &topLevelRegistry, QTable *_gshTagTable){
  QStringList result;

  gshTagTable = _gshTagTable;
  if (gshTagTable == NULL)
    return;

  for (int i=gshTagTable->numRows(); i>0; i--){
    gshTagTable->removeRow(0);
  }
  gshTagTable->insertRows(0, 1);
  gshTagTable->setText(0, 0, "Searching for Running Jobs");

  getSGSiesProcess = new QProcess(QString("./get_sgsies.pl"));
  getSGSiesProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  getSGSiesProcess->addArgument(topLevelRegistry);
  getSGSiesProcess->start();

  connect(getSGSiesProcess, SIGNAL(processExited()), this, SLOT(getSGSiesProcessEnded()));

  return;
}

QString Gridifier::makeSGSFactory(const QString &container, const QString &topLevelRegistry){
  QString result;
  
  makeSGSFactoryProcess = new QProcess(QString("./make_sgs_factory.pl"));
  makeSGSFactoryProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  makeSGSFactoryProcess->addArgument(container);
  makeSGSFactoryProcess->addArgument(topLevelRegistry);

  makeSGSFactoryProcess->start();

  while (makeSGSFactoryProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  // this functionality is left in a seperate slot since it could be better
  // to allow the standard qt events to handle it, rather than sitting in the
  // above loop
  result = makeSGSFactoryProcessEnded().stripWhiteSpace();

  return result;
}

QString Gridifier::makeSimSGS(const QString &factory, const QString &tag, const QString &topLevelRegistry, const QString &checkPointGSH, const QString &inputFileName, const QString &optionalChkPtTag){
  QString result;
  
  makeSimSGSProcess = new QProcess(QString("./make_sgs.pl"));
  makeSimSGSProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  makeSimSGSProcess->addArgument(factory);
  // need to make certain that the tag is handled correctly if it contains spaces
  makeSimSGSProcess->addArgument(tag);
  makeSimSGSProcess->addArgument(topLevelRegistry);
  makeSimSGSProcess->addArgument(checkPointGSH);
  makeSimSGSProcess->addArgument(inputFileName);
  if (optionalChkPtTag.length() > 0)
    makeSimSGSProcess->addArgument(optionalChkPtTag);
cout << makeSimSGSProcess->arguments().join(" ") << endl;
  makeSimSGSProcess->start();

  while(makeSimSGSProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  // this functionality is left in a seperate slot since it could be better
  // to allow the standard qt events to handle it, rather than sitting in the
  // above loop
  result = makeSimSGSProcessEnded().stripWhiteSpace();

  return result;
}

QString Gridifier::makeVizSGS(const QString &factory, const QString &tag, const QString &topLevelRegistry, const QString &simSGS){
  QString result;

  makeVizSGSProcess = new QProcess(QString("./make_vis_sgs.pl"));
  makeVizSGSProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  makeVizSGSProcess->addArgument(factory);
  makeVizSGSProcess->addArgument(tag);
  makeVizSGSProcess->addArgument(topLevelRegistry);
  makeVizSGSProcess->addArgument(simSGS);

cout << makeVizSGSProcess->arguments().join(" ") << endl;
  
  makeVizSGSProcess->start();

  while (makeVizSGSProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  result = makeVizSGSProcessEnded().stripWhiteSpace();

  return result;
}

/** Slots to receive process finished events
 */
QString Gridifier::getSGSFactoriesProcessEnded(const QString &desiredContainer){
  QString results = getSGSFactoriesProcess->readStdout();
 
  if (results.length() == 0){
    // then no SGS Factories exist - so create one - actually do this elsewhere
    
  }
  else {
    QStringList factories = QStringList::split("\n", results);

    int numFactories = factories.size();

    // prune the list - remove any factories not on the desired container
    for (int i=0; i<numFactories; i++){
      if (!factories[i].contains(desiredContainer)){
        factories.erase(factories.at(i));
        numFactories--;
      }
    }

    // if there's no factories left - return the blank string
    if (numFactories <= 0)
      return "";
    
    // choose a factory at random
    int randomNum = rand();
    randomNum = (int)((randomNum / (float)RAND_MAX) * numFactories);
    QString randomFactory = factories[randomNum];

    return randomFactory;
  }

  return "";
}


void Gridifier::getSGSiesProcessEnded(){
  QStringList result;

  // check we've got a reference to the gshTagTable
  if (gshTagTable == NULL)
    return;

  // clear out the table
  for (int i=gshTagTable->numRows(); i>0; i--){
    gshTagTable->removeRow(0);
  }
  
  QString processOutput = getSGSiesProcess->readStdout();

  // the output will be in sgs gsh & tag pairs
  // delimit on these and put them in the qstringlist
  result = QStringList::split("\n", processOutput);

  for (unsigned int i=0; i<result.count(); i++){
    QStringList temp = QStringList::split(" ", result[i]);
    if (temp.count() == 2){
      gshTagTable->insertRows(gshTagTable->numRows(), 1);
      gshTagTable->setText(gshTagTable->numRows()-1, 0, temp[0]);
      gshTagTable->setText(gshTagTable->numRows()-1, 1, temp[1]);
    }
  }

  
  return;// result;
}


QString Gridifier::makeSGSFactoryProcessEnded(){
  QString result = makeSGSFactoryProcess->readStdout();
  
  return result;  
}

QString Gridifier::makeSimSGSProcessEnded(){
  QString result = makeSimSGSProcess->readStdout();

  cout << makeSimSGSProcess->readStdout() << endl;
  cout << makeSimSGSProcess->readStderr() << endl;

  return result;
}

QString Gridifier::makeVizSGSProcessEnded(){
  QString result = makeVizSGSProcess->readStdout();

// Debugging going on here
QFile logFile(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/log");
if ( logFile.open(IO_WriteOnly) ){
  QTextStream stream(&logFile);
  stream << makeVizSGSProcess->arguments().join(" ") << endl;
  stream << result << endl;
  stream << endl << QString(makeVizSGSProcess->readStderr()) << endl;
  logFile.close();
}
  
  return result;
}


void Gridifier::makeReGScriptConfig(const QString & filename, const LauncherConfig &config){
  QFile file(filename);

  file.open(IO_WriteOnly);

  QStringList fileText;
  fileText += "#!/bin/sh\n\n";
  fileText += "OGSI\n\n";
  fileText += "CONTAINER="+config.selectedContainer+"\n";
  if (config.globusLocation.length() > 0)
    fileText += "GLOBUS_LOCATION="+config.globusLocation+"\n";
  else
    fileText += "# Couldn't find a globus location in the default.conf\n# Going with the default environment if it is set\n";
  fileText += "SIM_HOSTNAME="+config.simTargetMachine+"\n";
  fileText += "SIM_PROCESSORS="+QString::number(config.simNumberProcessors)+"\n";
  fileText += "SIM_INFILE="+config.lb3dInputFileName+"\n\n";
  fileText += QString("SIM_USER=")+getenv("USER")+"\n";
  fileText += "STEER_ANSWER=yes\n";
  fileText += "FIREWALL=no\n";
  fileText += "VIZ_ANSWER=no\n";
  fileText += "CLIENT_DISPLAY="+Utility::getHostName()+":0.0\n";
  fileText += "VIZ_HOSTNAME="+config.vizTargetMachine+"\n\n";
  fileText += "SIM_STD_OUT_FILE=RealityGrid/scratch/ReG-sim-stdout.$$.txt\n";
  fileText += "SIM_STD_ERR_FILE=RealityGrid/scratch/ReG-sim-stderr.$$.txt\n";
  fileText += "VIZ_STD_OUT_FILE=RealityGrid/scratch/ReG-viz-stdout.$$.txt\n";
  fileText += "VIZ_STD_ERR_FILE=RealityGrid/scratch/ReG-viz-stderr.$$.txt\n";
  fileText += "STEER_STD_OUT_FILE=RealityGrid/scratch/ReG-steer-stdout.$$.txt\n";
  fileText += "STEER_STD_ERR_FILE=RealityGrid/scratch/ReG-steer-stderr.$$.txt\n\n";
  
  
  QString vizTypeStr;
  if (config.vizType == isosurface){
    if (config.vizServer)
      vizTypeStr = "viz_iso";
    else
      vizTypeStr = "x_iso";
  }
  else if (config.vizType == volumeRender){
    if (config.vizServer)
      vizTypeStr = "viz_vol";
    else
      vizTypeStr = "x_vol";
  }
  if (config.vizType == hedgehog){
    if (config.vizServer)
      vizTypeStr = "viz_hog";
    else
      vizTypeStr = "x_hog";
  }
  else if (config.vizType == cutPlane){
    if (config.vizServer)
      vizTypeStr = "viz_cut";
    else
      vizTypeStr = "x_cut";
  }
  fileText += "VIZ_TYPE="+vizTypeStr+"\n";

  fileText += "VIZ_PROCESSORS="+QString::number(config.vizNumberProcessors)+"\n";
  fileText += "REG_SGS_ADDRESS="+config.simulationGSH+"\n";
  fileText += "REG_VIS_GSH="+config.visualizationGSH+"\n";

  fileText += "export OGSI CONTAINER VIZ_STD_OUT_FILE VIZ_STD_ERR_FILE STEER_STD_OUT_FILE STEER_STD_ERR_FILE SIM_STD_OUT_FILE SIM_STD_ERR_FILE CLIENT_DISPLAY GLOBUS_LOCATION SIM_HOSTNAME SIM_PROCESSORS SIM_INFILE VIZ_ANSWER VIZ_HOSTNAME VIZ_TYPE VIZ_PROCESSORS STEER_ANSWER SIM_USER FIREWALL REG_SGS_ADDRESS REG_VIZ_GSH\n\n";

  
  // Then convert to a string and write it out to the file

  QString fileTextSingleQString = fileText.join("");
  const char* fileTextSingleCharString = fileTextSingleQString.latin1();
  int buffLength = fileTextSingleQString.length();
  
  file.writeBlock(fileTextSingleCharString, buffLength);

  file.flush();

  file.close();
}


/** Method calls Robin's ReG-L2-Sim-QTL script to
 *  actually launch the job on the target machine
 */
void Gridifier::launchSimScript(const QString &scriptConfigFileName, int timeToRun, const QString &checkPointDataFile){
  launchSimScriptProcess = new QProcess(QString("./ReG-L2-Sim-QTL"));
  launchSimScriptProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  launchSimScriptProcess->addArgument(scriptConfigFileName);
  launchSimScriptProcess->addArgument(QString::number(timeToRun));
  if (checkPointDataFile != NULL)
    launchSimScriptProcess->addArgument(checkPointDataFile);

  cout << launchSimScriptProcess->arguments().join(" ") << endl;
            
  launchSimScriptProcess->start();

  while (launchSimScriptProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

// Debugging going on here
QFile logFile(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/log");
if ( logFile.open(IO_WriteOnly) ){
  QTextStream stream(&logFile);
  stream << launchSimScriptProcess->arguments().join(" ") << endl;
  stream << QString(launchSimScriptProcess->readStdout()) << endl;
  stream << endl << QString(launchSimScriptProcess->readStderr()) << endl;
  logFile.close();
}
  
  cout << "Stdout:" << endl << launchSimScriptProcess->readStdout() << endl;
  cout << "Stderr:" << endl << launchSimScriptProcess->readStderr() << endl;
}

/** Method calls Robin's ReG-L2-Viz-QTL script to
 *  actually launch the job on the target machine
 */
void Gridifier::launchVizScript(const QString &scriptConfigFileName){
  launchVizScriptProcess = new QProcess(QString("./ReG-L2-Viz-QTL"));
  launchVizScriptProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  launchVizScriptProcess->addArgument(scriptConfigFileName);

  launchVizScriptProcess->start();

  while (launchVizScriptProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

}



/** Method calls Mark McKeown's rgcpc perl script, which
 *  copies the relevant files over to the relevant remote
 *  machine. We don't need to pass a parameter since we
 *  can use the pre-retrieved CheckPointData cache file.
 */
//////// NOT USED //////////
void Gridifier::copyCheckPointFiles(const QString &host){
  QProcess *rgcpcProcess = new QProcess(QString("./rgcpc.pl"));
  rgcpcProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  rgcpcProcess->addArgument("-t");
  rgcpcProcess->addArgument(host);
  rgcpcProcess->addArgument("-f");
  rgcpcProcess->addArgument("checkPointDataCache.xml");
  rgcpcProcess->start();

  // and then do something clever to give the users an update status on the file transfer progress
}

/** Method simply calls globus-url-copy to transfer a file
 *  to a remote machine. We don't check for the existance
 *  of a valid proxy.
 */

 // NOT USED!?
QProcess *gsiFtpProcess;
void Gridifier::gsiFtp(const QString &aFile, const QString &aDestination){
  QString file = aFile;
  QString destination = aDestination;

  // do some sanity checking on the input paramaters - we might not
  // get a 'file://' tag for the input file
  if (!file.startsWith("file://")){
    // ought to test to see if the current path is in the file string already
    file = "file://"+file;
  }

  gsiFtpProcess = new QProcess(QString("globus-url-copy"));
  gsiFtpProcess->addArgument("-vb");
  gsiFtpProcess->addArgument(file);
  gsiFtpProcess->addArgument(destination);
  gsiFtpProcess->start();

  connect(gsiFtpProcess, SIGNAL(readyReadStdout()), this, SLOT(gsiFtpStdoutSlot()));
  connect(gsiFtpProcess, SIGNAL(readyReadStderr()), this, SLOT(gsiFtpStderrSlot()));
}

void Gridifier::gsiFtpStdoutSlot()
{
  cout << "Stdout:" << endl << gsiFtpProcess->readStdout() << endl;
}

void Gridifier::gsiFtpStderrSlot()
{
  cout << "Stderr:" << endl << gsiFtpProcess->readStderr() << endl;
}


QString Gridifier::checkPointAndStop(const QString &sgsGSH){
  QProcess *checkPointAndStopProcess = new QProcess(QString("./checkpoint_and_stop.pl"));
  checkPointAndStopProcess->setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  checkPointAndStopProcess->addArgument(sgsGSH);
  checkPointAndStopProcess->start();

  while (checkPointAndStopProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  QString result = checkPointAndStopProcess->readStdout();

  return result;
}




