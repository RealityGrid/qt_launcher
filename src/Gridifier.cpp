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

/** @file Gridifier.cpp
    @brief Implementation of wrappers for calling OGSA and remote-launching
    scripts. */

#include "Gridifier.h"
#include "Utility.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "qapplication.h"
#include "qfile.h"
#include "qmessagebox.h"

using namespace std;

/** Constructors
 */
Gridifier::Gridifier(){
  // Seed the random number generator
  srand(time(NULL));
  mApplication = qApp;
  // Set a default value in case something goes wrong
  mScriptsDir = QString(QString(getenv("HOME")) + 
			"/RealityGrid/reg_qt_launcher/scripts");
}

Gridifier::~Gridifier(){
}


void Gridifier::setApplication(QApplication *aApplication){
  mApplication = aApplication;
}

void Gridifier::setScriptsDirectory(const QString &dir){
  mScriptsDir = dir;
}


/** The following static methods wrap around QProcess objects to
 *  run the reg_perl_launcher scripts. Ultimately replace with
 *  code that uses an API to perform the same function programatically.
 */
QString Gridifier::getSGSFactories(const QString &topLevelRegistry, const QString &desiredContainer,
				   const QString &className){
  QString result;

  QProcess *getSGSFactoriesProcess = new QProcess(QString("./get_" + className + "_factories.pl"));
  getSGSFactoriesProcess->setWorkingDirectory(mScriptsDir);
  getSGSFactoriesProcess->addArgument(topLevelRegistry);

  getSGSFactoriesProcess->start();

  // At this point we need to wait for the process to complete.
  // Clearly this isn't ideal - hence the desire to replace this with API calls.
  // An alternative is to send the result back via a QT event....
  // for the time being stick with this slightly naff approach
  while (getSGSFactoriesProcess->isRunning()){
    // don't sit in an exhaustive loop - waste of electricity :)
    usleep(10000);
    // and keep the gui updated
    mApplication->processEvents();
  }

  // this functionality is left in a seperate slot since it could be better
  // to allow the standard qt events to handle it, rather than sitting in the
  // above loop
  //result = getSGSFactoriesProcessEnded(desiredContainer);

  QString allFactories = getSGSFactoriesProcess->readStdout();

  if (allFactories.length() == 0){
    // then no SGS Factories exist - so create one - actually do this elsewhere
    result = "";
  }
  else {
    QStringList factories = QStringList::split("\n", allFactories);

    int numFactories = factories.size();

    // prune the list - remove any factories not on the desired container
    for (int i=0; i<numFactories; i++){
      if (!factories[i].contains(desiredContainer)){
        factories.erase(factories.at(i));
        numFactories = factories.size();
        // undo the increment since we've removed an element
        i--;
      }
    }

    // if there's no factories left - return the blank string
    if (numFactories <= 0)
      result = "";

    // choose a factory at random
    int randomNum = rand();
    randomNum = (int)((randomNum / (float)RAND_MAX) * numFactories);
    QString randomFactory = factories[randomNum];

    result = randomFactory;
  }
  
  return result;
}


void Gridifier::getSGSies(const QString &topLevelRegistry, QTable *aGSHTagTable){
  QStringList result;

  mGSHTagTable = aGSHTagTable;
  if (mGSHTagTable == NULL)
    return;

  for (int i=mGSHTagTable->numRows(); i>0; i--){
    mGSHTagTable->removeRow(0);
  }
  mGSHTagTable->insertRows(0, 1);
  mGSHTagTable->setText(0, 0, "Searching for Running Jobs");

  getSGSiesProcess = new QProcess(QString("./get_sgsies.pl"));
  getSGSiesProcess->setWorkingDirectory(mScriptsDir);
  getSGSiesProcess->addArgument(topLevelRegistry);
  getSGSiesProcess->start();

  connect(getSGSiesProcess, SIGNAL(processExited()), this, 
    SLOT(getSGSiesProcessEnded()));

  return;
}

void Gridifier::getSGSiesProcessEnded(){
  QStringList result;

  // check we've got a reference to the gshTagTable
  if (mGSHTagTable == NULL)
    return;

  // clear out the table
  for (int i=mGSHTagTable->numRows(); i>0; i--){
    mGSHTagTable->removeRow(0);
  }

  QString processOutput = getSGSiesProcess->readStdout();

  // the output will be in sgs gsh & tag pairs
  // delimit on these and put them in the qstringlist
  result = QStringList::split("\n", processOutput);

  for (unsigned int i=0; i<result.count(); i++){
    int firstSpace = result[i].find(" ");
    QString tSGS = result[i].left(firstSpace);
    QString tag = result[i].right(result[i].length() - firstSpace);
    if (tSGS.startsWith("http://")){
      mGSHTagTable->insertRows(mGSHTagTable->numRows(), 1);
      mGSHTagTable->setText(mGSHTagTable->numRows()-1, 0, tSGS);
      mGSHTagTable->setText(mGSHTagTable->numRows()-1, 1, tag);
    }
  }

  return;
}

void Gridifier::getCoupledParamDefs(const QString &gsh, 
				    QString *aList){
  QStringList result;

  mFileListPtr = aList;
  if (mFileListPtr == NULL)
    return;

  cout << "ARPDBG: getCoupledParamDefs, gsh = " << gsh << endl;
  getParamDefsProcess = new QProcess(QString("./get_component_param_defs.pl"));
  getParamDefsProcess->setWorkingDirectory(mScriptsDir);
  getParamDefsProcess->addArgument(gsh);
  getParamDefsProcess->start();

  while (getParamDefsProcess->isRunning()){
    // don't sit in an exhaustive loop - waste of electricity :)
    usleep(10000);
    // and keep the gui updated
    mApplication->processEvents();
  }

  QString processOutput = getParamDefsProcess->readStdout();

  if(processOutput.startsWith("ERROR")){

    QMessageBox::warning( NULL, "Problem getting Param_defs:",
                    processOutput+"\n\n",
                    QMessageBox::Ok, 0, 0 );

  }

  *mFileListPtr = processOutput;
}

QString Gridifier::makeSGSFactory(const QString &container, 
				  const QString &topLevelRegistry,
				  const QString &className){
  QString result;
  
  QProcess *makeSGSFactoryProcess = new QProcess(QString("./make_" + className + "_factory.pl"));
  makeSGSFactoryProcess->setWorkingDirectory(mScriptsDir);
  makeSGSFactoryProcess->addArgument(container);
  makeSGSFactoryProcess->addArgument(topLevelRegistry);

  cout << makeSGSFactoryProcess->arguments().join(" ") << endl;
  
  makeSGSFactoryProcess->start();

  while (makeSGSFactoryProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  // this functionality is left in a seperate slot since it could be better
  // to allow the standard qt events to handle it, rather than sitting in the
  // above loop
  result = QString(makeSGSFactoryProcess->readStdout()).stripWhiteSpace();

  return result;
}

/* Create an SGS to associate with a simulation
 */
QString Gridifier::makeSimSGS(const QString &factory,
                              const LauncherConfig &config){
  QString result;
  
  QProcess *makeSimSGSProcess = new QProcess(QString("./make_sgs.pl"));
  makeSimSGSProcess->setWorkingDirectory(mScriptsDir);
  makeSimSGSProcess->addArgument(factory);
  // the tag is handled correctly if it contains spaces - thanks QT!
  makeSimSGSProcess->addArgument(config.mJobData->toXML(QString("SGS")));
  makeSimSGSProcess->addArgument(config.topLevelRegistryGSH);
  makeSimSGSProcess->addArgument(config.currentCheckpointGSH);
  makeSimSGSProcess->addArgument(config.mInputFileName);
  makeSimSGSProcess->addArgument(QString::number(config.mTimeToRun));
  if (config.treeTag.length() > 0){
    makeSimSGSProcess->addArgument(config.treeTag);
    makeSimSGSProcess->addArgument(config.checkPointTreeFactoryGSH);
  }
  makeSimSGSProcess->start();

  while(makeSimSGSProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  // Grab the sgs and return it
  // Do some error checking here - or in the calling class?
  result = QString(makeSimSGSProcess->readStdout()).stripWhiteSpace();

  return result;
}

QString Gridifier::makeVizSGS(const QString &factory,
                              const LauncherConfig &config){
  QString result;

  QProcess *makeVizSGSProcess = new QProcess(QString("./make_vis_sgs.pl"));
  makeVizSGSProcess->setWorkingDirectory(mScriptsDir);
  makeVizSGSProcess->addArgument(factory);
  makeVizSGSProcess->addArgument(config.mJobData->toXML(QString("SGS")));
  makeVizSGSProcess->addArgument(config.topLevelRegistryGSH);
  makeVizSGSProcess->addArgument(config.simulationGSH);
  makeVizSGSProcess->addArgument(QString::number(config.mTimeToRun));
  makeVizSGSProcess->start();

  while (makeVizSGSProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  result = QString(makeVizSGSProcess->readStdout()).stripWhiteSpace();

  return result;
}

/** Create a MetaSGS */
QString Gridifier::makeMetaSGS(const QString &factory,
			       const LauncherConfig &config,
			       const QString &parentGSH){

  QString result;
  
  QProcess *makeSimSGSProcess = new QProcess(QString("./make_msgs.pl"));
  makeSimSGSProcess->setWorkingDirectory(mScriptsDir);
  makeSimSGSProcess->addArgument(factory);
  // the tag is handled correctly if it contains spaces - thanks QT!
  makeSimSGSProcess->addArgument(config.mJobData->toXML(QString("MetaSGS")));
  makeSimSGSProcess->addArgument(config.topLevelRegistryGSH);
  makeSimSGSProcess->addArgument(config.currentCheckpointGSH);
  makeSimSGSProcess->addArgument(config.mInputFileName);
  makeSimSGSProcess->addArgument(QString::number(config.mTimeToRun));
  makeSimSGSProcess->addArgument(parentGSH);
  if (config.treeTag.length() > 0){
    makeSimSGSProcess->addArgument(config.treeTag);
    makeSimSGSProcess->addArgument(config.checkPointTreeFactoryGSH);
  }
  makeSimSGSProcess->start();

  while(makeSimSGSProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  // Grab the sgs and return it
  // Do some error checking here - or in the calling class?
  result = QString(makeSimSGSProcess->readStdout()).stripWhiteSpace();

  cout << "Stdout:" << endl << result << endl;
  if (makeSimSGSProcess->canReadLineStderr())
    cout << "Stderr:" << endl << makeSimSGSProcess->readStderr() << endl;

  return result;
}

/** Create the script that sets up the job environment */
void Gridifier::makeReGScriptConfig(const QString & filename,
                                    const LauncherConfig &config){
  QFile file(filename);

  file.open(IO_WriteOnly);

  QStringList fileText;
  fileText += "#!/bin/sh\n\n";
  fileText += "CONTAINER="+config.selectedContainer+"\n";
  if (config.globusLocation.length() > 0)
    fileText += "GLOBUS_LOCATION="+config.globusLocation+"\n";
  else
    fileText += "# Couldn't find a globus location in the default.conf\n# Going with the default environment if it is set\n";
  fileText += "SIM_HOSTNAME="+config.mTargetMachine->mName+"\n";
  fileText += "HOST_JOB_MGR="+config.mTargetMachine->mJobManager+"\n";
  fileText += "HOST_QUEUE="+config.mTargetMachine->mQueue+"\n";
  fileText += "export HOST_QUEUE\n";
  fileText += "SIM_PROCESSORS="+QString::number(config.mNumberProcessors)+"\n";
  fileText += "SIM_INFILE="+config.mInputFileName+"\n\n";
  fileText += QString("SIM_USER=")+getenv("USER")+"\n";
  fileText += "FIREWALL=no\n";
  fileText += "CLIENT_DISPLAY="+Utility::getCurrentDisplay()+"\n";
  fileText += "SIM_STD_OUT_FILE=RealityGrid/scratch/ReG-sim-stdout.$$.txt\n";
  fileText += "SIM_STD_ERR_FILE=RealityGrid/scratch/ReG-sim-stderr.$$.txt\n";
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
  fileText += "VIZ_PROCESSORS="+QString::number(config.mNumberProcessors)+"\n";
  
  if(config.mAppToLaunch->mNumInputs > 0){
    fileText += "REG_SGS_ADDRESS="+config.visualizationGSH+"\n";
  }
  else{
    fileText += "REG_SGS_ADDRESS="+config.simulationGSH+"\n";
  }
 
  fileText += "export HOST_JOB_MGR CONTAINER STEER_STD_OUT_FILE STEER_STD_ERR_FILE SIM_STD_OUT_FILE SIM_STD_ERR_FILE CLIENT_DISPLAY GLOBUS_LOCATION SIM_HOSTNAME SIM_PROCESSORS SIM_INFILE VIZ_TYPE VIZ_PROCESSORS SIM_USER FIREWALL REG_SGS_ADDRESS REG_VIZ_GSH\n\n";

  if (config.multicast){
    fileText += "MULTICAST_ADDRESS="+config.multicastAddress+"\n\n";
    fileText += "export MULTICAST_ADDRESS";
  }

  // Then convert to a string and write it out to the file

  QString fileTextSingleQString = fileText.join("");
  const char* fileTextSingleCharString = fileTextSingleQString.latin1();
  int buffLength = fileTextSingleQString.length();
  
  file.writeBlock(fileTextSingleCharString, buffLength);

  file.flush();

  file.close();
  }


/** Method calls <app_name>_launch.sh script to
 *  actually launch the job on the target machine
 */
void Gridifier::launchSimScript(const QString &scriptConfigFileName,
                                const LauncherConfig &config){

  // Construct name of script from name of application
  QProcess *launchSimScriptProcess = new QProcess(QString("./"+config.mAppToLaunch->mAppName+"_launch.sh"));
  launchSimScriptProcess->setWorkingDirectory(mScriptsDir);
  launchSimScriptProcess->addArgument(scriptConfigFileName);
  launchSimScriptProcess->addArgument(QString::number(config.mTimeToRun));
  
  if(config.currentCheckpointGSH.length() != 0)
    launchSimScriptProcess->addArgument(config.currentCheckpointGSH);
    
  launchSimScriptProcess->start();

  while (launchSimScriptProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

// Debugging going on here
/*QFile logFile(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/log");
if ( logFile.open(IO_WriteOnly) ){
  QTextStream stream(&logFile);
  stream << launchSimScriptProcess->arguments().join(" ") << endl;
  stream << QString(launchSimScriptProcess->readStdout()) << endl;
  stream << endl << QString(launchSimScriptProcess->readStderr()) << endl;
  logFile.close();
}*/
  
  cout << "Stdout:" << endl << launchSimScriptProcess->readStdout() << endl;
  if (launchSimScriptProcess->canReadLineStderr())
    cout << "Stderr:" << endl << launchSimScriptProcess->readStderr() << endl;
}

/** Method calls Robin's ReG-L2-Viz-QTL script to
 *  actually launch the job on the target machine
 */
void Gridifier::launchVizScript(const QString &scriptConfigFileName,
                                const LauncherConfig &config){
                                
  // Construct name of script from name of application
  QProcess *launchVizScriptProcess = new QProcess(QString("./"+config.mAppToLaunch->mAppName+"_launch.sh"));
  launchVizScriptProcess->setWorkingDirectory(mScriptsDir);
  launchVizScriptProcess->addArgument(scriptConfigFileName);
  launchVizScriptProcess->addArgument(QString::number(config.mTimeToRun));

  launchVizScriptProcess->start();

  while (launchVizScriptProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }


  cout << "Stdout:" << endl << launchVizScriptProcess->readStdout() << endl;
  if (launchVizScriptProcess->canReadLineStderr())
    cout << "Stderr:" << endl << launchVizScriptProcess->readStderr() << endl;
}


/** Special case to launch a viz on Argonne's cluster
 *  this will always render to multicast
 */
void Gridifier::launchArgonneViz(const LauncherConfig &config){
  QProcess *launchArgonneVizProcess = new QProcess(QString("./argonneVis.sh"));
  launchArgonneVizProcess->setWorkingDirectory(mScriptsDir);
  launchArgonneVizProcess->addArgument(config.visualizationGSH);
  launchArgonneVizProcess->addArgument(QString::number(config.mTimeToRun));
  launchArgonneVizProcess->addArgument(config.multicastAddress);

  QString vizTypeStr = "";
  if (config.vizType == isosurface){
    vizTypeStr = "iso";
  }
  else if (config.vizType == volumeRender){
    vizTypeStr = "vol";
  }
  if (config.vizType == hedgehog){
    vizTypeStr = "hog";
  }
  else if (config.vizType == cutPlane){
    vizTypeStr = "cut";
  }
  launchArgonneVizProcess->addArgument(vizTypeStr);

  cout << launchArgonneVizProcess->arguments().join(" ") << endl;
  
  launchArgonneVizProcess->start();

  while (launchArgonneVizProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  cout << "Stdout:" << endl << launchArgonneVizProcess->readStdout() << endl;
  if (launchArgonneVizProcess->canReadLineStderr())
    cout << "Stderr:" << endl << launchArgonneVizProcess->readStderr() << endl;
}

/** Method calls Mark McKeown's rgcpc perl script, which
 *  copies the relevant files over to the relevant remote
 *  machine. We don't need to pass a parameter since we
 *  can use the pre-retrieved CheckPointData cache file.
 */
//////// NOT USED //////////
void Gridifier::copyCheckPointFiles(const QString &host){
  QProcess *rgcpcProcess = new QProcess(QString("./rgcpc.pl"));
  rgcpcProcess->setWorkingDirectory(mScriptsDir);
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

/** Instructs the simulation associated with the specified SGS
    to take a checkpoint and then stop.  This call blocks until
    job has stopped or a time-out occurs. */
QString Gridifier::checkPointAndStop(const QString &sgsGSH){
  QProcess *checkPointAndStopProcess = new QProcess(QString("./checkpoint_and_stop.pl"));
  checkPointAndStopProcess->setWorkingDirectory(mScriptsDir);
  checkPointAndStopProcess->addArgument(sgsGSH);
  checkPointAndStopProcess->start();

  while (checkPointAndStopProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  QString result = checkPointAndStopProcess->readStdout();

  return result;
}

/**
 * Method calls lb3d_client.pl script to call web service
 * to actually launch the job on the target machine
 */
void Gridifier::webServiceJobSubmit(const QString & scriptConfigFileName){

  // Construct name of script from name of application
  QProcess *launchSimScriptProcess = new QProcess(QString("./lb3d_client.pl"));
  launchSimScriptProcess->setWorkingDirectory(mScriptsDir);
  launchSimScriptProcess->addArgument(scriptConfigFileName);

  launchSimScriptProcess->start();

  while (launchSimScriptProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  cout << "Stdout:" << endl << launchSimScriptProcess->readStdout() << endl;
  if (launchSimScriptProcess->canReadLineStderr())
    cout << "Stderr:" << endl << launchSimScriptProcess->readStderr() << endl;
}

/** Calls setServiceData on the specified service using the
    second argument as the basis for the argument to setServiceData. 
    (The <ogsi:setByServiceDataNames> tags are added in this 
    routine.) */
void Gridifier::setServiceData(const QString &nameSpace,
			       const QString &gsh,
			       const QString &sdeText){

  QString arg("<ogsi:setByServiceDataNames>");
  arg.append(sdeText);
  arg.append("</ogsi:setByServiceDataNames>");

  QProcess *setServiceDataProcess = new QProcess(QString("./setServiceData.pl"));
  setServiceDataProcess->setWorkingDirectory(mScriptsDir);
  setServiceDataProcess->addArgument(nameSpace);
  setServiceDataProcess->addArgument(gsh);
  setServiceDataProcess->addArgument(arg);
  setServiceDataProcess->start();

  while (setServiceDataProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  cout << "Stdout:" << endl << setServiceDataProcess->readStdout() << endl;
  if (setServiceDataProcess->canReadLineStderr())
    cout << "Stderr:" << endl << setServiceDataProcess->readStderr() << endl;
}
