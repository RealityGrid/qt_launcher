/*----------------------------------------------------------------------------
    Application Class for QT launcher GUI.
    Implementation

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

---------------------------------------------------------------------------*/

/** @file LauncherConfig.cpp
 *  @brief Holds all of the data describing the job to launch 
 *  @author Mark Riding 
 *  @author Andrew Porter */

/** Important note - libxml was used for this class, but QT does provide
  * its own set of XML classes. I just didn't realise that when I began..*/

/* MORE IMPORTANTLY - I'll convert all this to QT xml support later.... */

#include <qdom.h>
#include "LauncherConfig.h"
#include "textviewdialog.h"
#include <iostream>
#include "libxml/parser.h"
#include "qmdcodec.h"
#include <qfile.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qtextedit.h>
#include "ReG_Steer_types.h"
#include "ReG_Steer_Utils.h"

using namespace std;

/** Constructor */
LauncherConfig::LauncherConfig(){

  migration       = false;
  restart         = false;
  newTree         = false;
  mTimeToRun      = 30;
  mJobData        = new JobMetaData;
  mIsCoupledModel = false;

  Wipe_security_info(&registrySecurity);

  // Hacky way of ensuring we know what default.conf should be
  // in case user hasn't got it in the right place
  // - this include file is generated at build time and assigns
  // the contents of default.conf to mConfigFileContent.
#include "ConfigFileContent.h"

}

/** Constructor using configuration file (not used?) */
LauncherConfig::LauncherConfig(QString file){
  readConfig(file);
}

/** Destructor */
LauncherConfig::~LauncherConfig(){
  delete mJobData;
}

/** Method loads a configuration xml file, and parses it to
 *  determine the stored configuration values.
 *  NOTE - NEEDS RESET SUPPORT - WIPE VARIABLES
 */
void LauncherConfig::readConfig(QString file){
  xmlDocPtr doc;
  xmlNodePtr root;
  xmlNodePtr childOfRoot;
  xmlNodePtr aGSH;
  //Don't read component info at the moment - ARP
  //Not sure we'll ever need to...
  //xmlNodePtr aComponent;
  xmlNodePtr containers;
  xmlNodePtr settings;
  xmlNodePtr targets;
  xmlNodePtr vizTargets;
  xmlNodePtr applications;

  QFile *configFile = new QFile(file);
  if(!(configFile->exists())){
    cout << "Input file " << file << " does not exist :-(" <<endl;

    if(file.find("default.conf") > -1){

      if(!createNewConfigFile()) return;
    }
  }

  // Load it in and parse it with libxml2
  doc = xmlParseFile(file.latin1());

  // Check ok
  if (doc == NULL){
    cout << "Document not parsed successfully" << endl;
    return;
  }

  // Find root node
  root = xmlDocGetRootElement(doc);

  // Check ok
  if (root == NULL) {
	  cout << "empty document" << endl;
    xmlFreeDoc(doc);
    return;
  }

  // Determine validity
  if (xmlStrcmp(root->name, (const xmlChar *) "launcher")) {
		cout << "document of the wrong type, root node != launcher" << endl;
    
		xmlFreeDoc(doc);
		return;
  }

  childOfRoot = root->xmlChildrenNode;
  
  while (childOfRoot != NULL){

    if(!xmlStrcmp(childOfRoot->name, (const xmlChar*)"settings")){
      settings = childOfRoot->xmlChildrenNode;

      // The location of the perl scripts used to do job launching
      // and web-servicey things
      while(settings != NULL){
        if (!xmlStrcmp(settings->name, 
		       (const xmlChar*)"scriptsDirectory")){

          xmlChar *val = xmlGetProp(settings, (const xmlChar*)"value");
	  mScriptsDirectory = QString((const char*)val);
	  xmlFree(val);
	}

	// The location of our 'tmp' directory
        if (!xmlStrcmp(settings->name, 
		       (const xmlChar*)"scratchDirectory")){

          xmlChar *val = xmlGetProp(settings, (const xmlChar*)"value");
	  mScratchDirectory = QString((const char*)val);
	  // Use this value to set an environment variable so our
	  // various perl scripts know where scratch is
	  setenv("REG_SCRATCH_DIRECTORY", mScratchDirectory.ascii(), 1);
	  xmlFree(val);
	}

	// How we're going to launch remote jobs - via globus, cog
	// or ssh.
        if (!xmlStrcmp(settings->name, 
		       (const xmlChar*)"launchMethod")){
          xmlChar *val = xmlGetProp(settings, (const xmlChar*)"value");
	  mLaunchMethod = QString((const char*)val);
	  setenv("ReG_LAUNCH", mLaunchMethod.ascii(), 1);
	  // Also set env. variable that is passed over to remote machine
	  // so that launching script knows whether or not it has been run
	  // by ssh (may need to submit into queuing system).
	  if(mLaunchMethod.find("ssh") > -1){
	    setenv("SSH", "1", 1);
	  }
	  else{
	    setenv("SSH", "0", 1);
	  }
	  xmlFree(val);
	}

	// The location of the steering client binary (so we can launch it
	// when the user clicks 'steer')
        if (!xmlStrcmp(settings->name, 
		       (const xmlChar*)"steerClientBinary")){
          xmlChar *val = xmlGetProp(settings, (const xmlChar*)"value");
	  mSteererBinaryLocation = QString((const char*)val);
	  xmlFree(val);
	}

	settings = settings->next;
      }
    }

    // Retrieve the GSH information
    if ((!xmlStrcmp(childOfRoot->name, (const xmlChar *)"GSHs"))){
      
      aGSH = childOfRoot->xmlChildrenNode;

      while (aGSH != NULL){
        // test to see which element we've got, 
	// and fill in the appropriate class member variable

        // Top Level Registry
        if ((!xmlStrcmp(aGSH->name, (const xmlChar *)"topLevelRegistry"))) {
          xmlChar *key = xmlGetProp(aGSH, (xmlChar*)"value");
          topLevelRegistryGSH = QString((const char*)key);
	  xmlFree(key);
        }

	// RealityGridTreeFactory address - OGSI
        if ((!xmlStrcmp(aGSH->name, 
			(const xmlChar *)"checkPointTreeFactory"))) {
          xmlChar *key = xmlGetProp(aGSH, (xmlChar*)"value");
          checkPointTreeFactoryGSH = QString((const char*)key);
	  xmlFree(key);
        }

	// CheckPointTree address - WSRF
        if ((!xmlStrcmp(aGSH->name, 
			(const xmlChar *)"checkPointTree"))) {
          xmlChar *key = xmlGetProp(aGSH, (xmlChar*)"value");
          checkPointTreeEPR = QString((const char*)key);
	  xmlFree(key);
        }

        // Registry of Factories
        if ((!xmlStrcmp(aGSH->name, (const xmlChar *)"registryOfFactories"))) {
          xmlChar *key = xmlGetProp(aGSH, (xmlChar*)"value");
          registryOfFactoriesGSH = QString((const char*)key);
	  xmlFree(key);
        }

        // SGS Factory
        if ((!xmlStrcmp(aGSH->name, (const xmlChar *)"SGSFactory"))) {
          xmlChar *key = xmlGetProp(aGSH, (xmlChar*)"value");
          SGSFactoryGSH = QString((const char*)key);
	  xmlFree(key);
        }

        // SGS
        if ((!xmlStrcmp(aGSH->name, (const xmlChar *)"SGS"))) {
          xmlChar *key = xmlGetProp(aGSH, (xmlChar*)"value");
          SGSGSH.mEPR = QString((const char*)key);
	  xmlFree(key);
        }

        aGSH = aGSH->next;
      }
      
    }

    // Retrieve the container information
    if ((!xmlStrcmp(childOfRoot->name, (const xmlChar*)"containers"))){
      containers = childOfRoot->xmlChildrenNode;

      cout << "ARPDBG Containers from launcher.conf:" << endl;
      while (containers != NULL){
        
        if (!xmlStrcmp(containers->name, (const xmlChar*)"container")){
          xmlChar *containerName = xmlNodeListGetString(doc, containers->xmlChildrenNode, 1);
          xmlChar *port = xmlGetProp(containers, (const xmlChar*)"port");
	  mContainerList += "http://"+QString((char*)containerName)+":"+
	    QString((char*)port) + "/";
	  cout << "  ARPDBG Container: " << mContainerList.last() << endl;
          xmlFree(containerName);
          xmlFree(port);
        }
        
        containers = containers->next;
      }
    }

    // Retrieve the sim target machine information
    if ((!xmlStrcmp(childOfRoot->name, (const xmlChar*)"targets"))){
      targets = childOfRoot->xmlChildrenNode;

      while (targets != NULL){

        if (!xmlStrcmp(targets->name, (const xmlChar*)"machine")){
          xmlChar *machineName = xmlGetProp(targets, (const xmlChar*)"name");
          xmlChar *machineOS = xmlGetProp(targets, (const xmlChar*)"os");
          xmlChar *machineJobManager = xmlGetProp(targets, (const xmlChar*)"jobmanager");
          xmlChar *machineQueue = xmlGetProp(targets, (const xmlChar*)"queue");
          
          if(!machineQueue){

            QMessageBox::warning( NULL, "Error parsing configuration file",
                    "No queue associated with machine "+QString((char*)machineName)+": you have "
                    "an old default.conf file.\nAssuming queue=\"none\" for this machine.\n\n",
                    QMessageBox::Ok, 0, 0 );
                    machineQueue = (xmlChar*)xmlMalloc(8);
                    sprintf((char *)machineQueue, "none");
          }
 
          Machine tMachine((const char*)machineName,
                           (const char*)machineJobManager,
                           (const char*)machineOS,
                           (const char*)machineQueue);
          machineList += tMachine;

          xmlFree(machineName);
          xmlFree(machineOS);
          xmlFree(machineJobManager);
          xmlFree(machineQueue);
        }

        targets = targets->next;
      }
    }

    // Retrieve the viz target machine information
    if ((!xmlStrcmp(childOfRoot->name, (const xmlChar*)"vizTargets"))){
      vizTargets = childOfRoot->xmlChildrenNode;

      while (vizTargets != NULL){

        if (!xmlStrcmp(vizTargets->name, (const xmlChar*)"machine")){
          xmlChar *vizMachineName = xmlGetProp(vizTargets, (const xmlChar*)"name");
          xmlChar *vizMachineOS = xmlGetProp(vizTargets, (const xmlChar*)"os");
          xmlChar *vizMachineJobManager = xmlGetProp(vizTargets, (const xmlChar*)"jobmanager");
          xmlChar *vizMachineQueue = xmlGetProp(vizTargets, (const xmlChar*)"queue");
          
          Machine tMachine((const char*)vizMachineName,
                           (const char*)vizMachineJobManager,
                           (const char*)vizMachineOS,
                           (const char*)vizMachineQueue);
                           
          vizMachineList += tMachine;
          
          xmlFree(vizMachineName);
          xmlFree(vizMachineOS);
          xmlFree(vizMachineJobManager);
          xmlFree(vizMachineQueue);
        }

        vizTargets = vizTargets->next;
      }
    }    
    
    if(!xmlStrcmp(childOfRoot->name, (const xmlChar*)"applications")){

      applications = childOfRoot->xmlChildrenNode;
      bool lHasFile, lCanRestart, lIsViz;
      
      while(applications != NULL){

        if(!xmlStrcmp(applications->name, (const xmlChar*)"application")){
          xmlChar *appName = xmlGetProp(applications, (const xmlChar*)"name");
          xmlChar *appNumInputs = xmlGetProp(applications, (const xmlChar*)"inputs");
          xmlChar *appHasFile = xmlGetProp(applications, (const xmlChar*)"hasInputFile");
          xmlChar *appRestartable = xmlGetProp(applications, (const xmlChar*)"restartable");
          xmlChar *appIsViz = xmlGetProp(applications, (const xmlChar*)"isViz");
          
          if(!xmlStrcmp(appHasFile, (const xmlChar*)"yes") ||
             !xmlStrcmp(appHasFile, (const xmlChar*)"Yes")){
            lHasFile = true;
          }
          else{
            lHasFile = false;
          }

          if(!xmlStrcmp(appRestartable, (const xmlChar*)"yes") ||
             !xmlStrcmp(appRestartable, (const xmlChar*)"Yes")){
            lCanRestart = true;
          }
          else{
            lCanRestart = false;
          }

          if(!xmlStrcmp(appIsViz, (const xmlChar*)"yes") ||
             !xmlStrcmp(appIsViz, (const xmlChar*)"Yes")){
            lIsViz = true;
          }
          else{
            lIsViz = false;
          }
          
          Application tApplication((const char*)appName,
                                   QString((const char*)appNumInputs).toInt(),
                                   lHasFile, lCanRestart, lIsViz);
          applicationList += tApplication;

          xmlFree(appName);
          xmlFree(appNumInputs);
          xmlFree(appHasFile);
          xmlFree(appRestartable);
          xmlFree(appIsViz);
        }

        applications = applications->next;
      }
    }

    childOfRoot = childOfRoot->next;
  }


  // Get globus location from environment instead of from conf file because
  // that's more consistent with other applications
  globusLocation = QString(getenv("GLOBUS_LOCATION"));
  if(globusLocation.isEmpty()){

    QMessageBox::warning( NULL, "Configuration error",
                  "Failed to get GLOBUS_LOCATION from environment.\n"
                  "Launching via Globus will not be possible.\n",
                  QMessageBox::Ok, 0, 0 );
  }
}

/** Method uses libxml2 to create an xml encoding of the current configuration
 *  and save the file to disk
 *  NOT UP TO DATE AND NOT CURRENTLY USED
 */
void LauncherConfig::writeConfig(QString file){
  xmlDocPtr doc;
  xmlNodePtr root;
  xmlNodePtr SGSs;
  xmlNodePtr simComponent;
  xmlNodePtr vizComponent;
  xmlNodePtr containers;
  xmlNodePtr machines;
  xmlNodePtr vizMachines;

  xmlNodePtr topLevelRegistry;
  xmlNodePtr registryOfFactories;
  xmlNodePtr SGSFactory;
  xmlNodePtr SGS;

  char buff[32];

  // Build the doc
  doc = xmlNewDoc((const xmlChar*)"1.0");
  root = xmlNewDocNode(doc, NULL, (const xmlChar*)"launcher", NULL);
  SGSs = xmlNewTextChild(root, NULL, (const xmlChar*)"SGSs", NULL);
  
  topLevelRegistry = xmlNewTextChild(SGSs, NULL, (const xmlChar*)"topLevelRegistry", NULL);
  xmlNewProp(topLevelRegistry, (const xmlChar*)"value", (const xmlChar*)topLevelRegistryGSH.latin1());
  
  registryOfFactories = xmlNewTextChild(SGSs, NULL, (const xmlChar*)"registryOfFactories", NULL);
  xmlNewProp(registryOfFactories, (const xmlChar*)"value", (const xmlChar*)registryOfFactoriesGSH.latin1());
    
  SGSFactory = xmlNewTextChild(SGSs, NULL, (const xmlChar*)"SGSFactory", NULL);
  xmlNewProp(SGSFactory, (const xmlChar*)"value", (const xmlChar*)SGSFactoryGSH.latin1());
    
  SGS = xmlNewTextChild(SGSs, NULL, (const xmlChar*)"SGS", NULL);
  xmlNewProp(SGS, (const xmlChar*)"value", (const xmlChar*)SGSGSH.mEPR.ascii());

// Don't bother with component info for now - ARPDBG
//  if (simComponentType == lb3d){
//    simComponent = xmlNewTextChild(root, NULL, (const xmlChar*)"component", NULL);
//    xmlNewProp(simComponent, (const xmlChar*)"type", (const xmlChar*)"lb3d");
//    xmlNewProp(simComponent, (const xmlChar*)"targetMachineName", (const xmlChar*)simTargetMachine.latin1());
//    sprintf(buff, "%d", simNumberProcessors);
//    xmlNewProp(simComponent, (const xmlChar*)"numberProcessors", (const xmlChar*)buff);
//    xmlNewProp(simComponent, (const xmlChar*)"tag", (const xmlChar*)simTag.latin1());
//  }
//  else if (simComponentType == miniapp){
//    simComponent = xmlNewTextChild(root, NULL, (const xmlChar*)"component", NULL);
//    xmlNewProp(simComponent, (const xmlChar*)"type", (const xmlChar*)"miniapp");
//    xmlNewProp(simComponent, (const xmlChar*)"targetMachineName", (const xmlChar*)simTargetMachine.latin1());
//    sprintf(buff, "%d", simNumberProcessors);
//    xmlNewProp(simComponent, (const xmlChar*)"numberProcessors", (const xmlChar*)buff);
//    xmlNewProp(simComponent, (const xmlChar*)"tag", (const xmlChar*)simTag.latin1());
//  }
//
//  if (vizComponentType == lb3dviz){
//    vizComponent = xmlNewTextChild(root, NULL, (const xmlChar*)"component", NULL);
//    xmlNewProp(vizComponent, (const xmlChar*)"type", (const xmlChar*)"lb3dviz");
//    xmlNewProp(vizComponent, (const xmlChar*)"targetMachineName", (const xmlChar*)vizTargetMachine.latin1());
//    sprintf(buff, "%d", vizNumberProcessors);
//    xmlNewProp(vizComponent, (const xmlChar*)"numberProcessors", (const xmlChar*)buff);
//    sprintf(buff, "%d", vizNumberPipes);
//    xmlNewProp(vizComponent, (const xmlChar*)"numberPipes", (const xmlChar*)buff);
//    if (vizServer)
//      xmlNewProp(vizComponent, (const xmlChar*)"vizServer", (const xmlChar*)"yes");
//    else
//      xmlNewProp(vizComponent, (const xmlChar*)"vizServer", (const xmlChar*)"no");
//    xmlNewProp(vizComponent, (const xmlChar*)"tag", (const xmlChar*)vizTag.latin1());
//    sprintf(buff, "%d", vizType);
//    xmlNewProp(vizComponent, (const xmlChar*)"type", (const xmlChar*)buff);
//  }

  containers = xmlNewTextChild(root, NULL, (const xmlChar*)"containers", NULL);
  for ( QStringList::Iterator it = mContainerList.begin(); it != mContainerList.end(); ++it ) {
      xmlNodePtr t = xmlNewTextChild(containers, NULL, (const xmlChar*)"container", (const xmlChar*)(*it).latin1());
      //sprintf(buff, "%d", (*it).mPort);
      //xmlNewProp(t, (const xmlChar*)"port", (const xmlChar*)buff);
  }

  machines = xmlNewTextChild(root, NULL, (const xmlChar*)"targets", NULL);
  for ( QValueList<Machine>::Iterator it = machineList.begin(); it != machineList.end(); ++it ) {
      xmlNodePtr t = xmlNewTextChild(machines, NULL, (const xmlChar*)"machine", NULL);
      xmlNewProp(t, (const xmlChar*)"name", (const xmlChar*)(*it).mName.latin1());
      xmlNewProp(t, (const xmlChar*)"jobmanager", (const xmlChar*)(*it).mJobManager.latin1());
      xmlNewProp(t, (const xmlChar*)"os", (const xmlChar*)(*it).mOS.latin1());
  }

  vizMachines = xmlNewTextChild(root, NULL, (const xmlChar*)"vizTargets", NULL);
  for ( QValueList<Machine>::Iterator it = vizMachineList.begin(); it != vizMachineList.end(); ++it ) {
      xmlNodePtr t = xmlNewTextChild(vizMachines, NULL, (const xmlChar*)"machine", NULL);
      xmlNewProp(t, (const xmlChar*)"name", (const xmlChar*)(*it).mName.latin1());
      xmlNewProp(t, (const xmlChar*)"jobmanager", (const xmlChar*)(*it).mJobManager.latin1());
      xmlNewProp(t, (const xmlChar*)"os", (const xmlChar*)(*it).mOS.latin1());
  }

  xmlDocSetRootElement(doc, root);

  // And save it!
  xmlSaveFile(file.latin1(), doc);
}

//----------------------------------------------------------------------
QString LauncherConfig::toXML(){

  QDomDocument *doc = new QDomDocument();

  // Get root node
  QDomElement root = doc->createElement("LaunchSimulation");
  doc->appendChild(root);

  // Where to launch job
  QDomElement eMachine = doc->createElement("TargetHostname");
  root.appendChild(eMachine);

  QDomText tMachineName = doc->createTextNode(mTargetMachine->mName);
  eMachine.appendChild(tMachineName);

  // What the jobmanager is called
  QDomElement eJobMgr = doc->createElement("TargetHostJobManager");
  root.appendChild(eJobMgr);

  QDomText tJobMgr = doc->createTextNode(mTargetMachine->mJobManager);
  eJobMgr.appendChild(tJobMgr);

  // Wall-clock time of job in minutes
  QDomElement eRunTime = doc->createElement("RunTime");
  root.appendChild(eRunTime);

  QDomText tRunTime = doc->createTextNode(QString::number(mTimeToRun));
  eRunTime.appendChild(tRunTime);

  // Checkpoint GSH
  QDomElement eChkGSH;
  QDomText tChkGSH;
  if(restart || migration){
    eChkGSH = doc->createElement("CheckPointGSH");
    root.appendChild(eChkGSH);

    tChkGSH = doc->createTextNode(currentCheckpointGSH);
    eChkGSH.appendChild(tChkGSH);
  }

  // No. of processors
  QDomElement eNumProc = doc->createElement("NoProcessors");
  root.appendChild(eNumProc);

  QDomText tNumProc = doc->createTextNode(QString::number(mNumberProcessors));
  eNumProc.appendChild(tNumProc);

  // Input file(s) for simulation
  QDomElement eInputFiles = doc->createElement("SimulationInputFiles");
  root.appendChild(eInputFiles);

  QDomElement eFile;
  QDomText tFile;
  if(mAppToLaunch->mHasInputFile){
  
    eFile =  doc->createElement("File");
    eInputFiles.appendChild(eFile);

    // Read input file
    QFile file(mInputFileName);
    file.open( IO_ReadOnly );
    QByteArray fileData = file.readAll();
    file.close();
  
    // Encode input file as base64
    QCString fileDataB64 = QCodecs::base64Encode(fileData, true);
  
    tFile = doc->createTextNode(fileDataB64);
    eFile.appendChild(tFile);
  }

  // Pull out GSH of SGS and use to create job ID too
  QString outputFile("ReGJob.");
  QDomText tSGS;
  if(mAppToLaunch->mNumInputs == 0){
    tSGS = doc->createTextNode(simulationGSH.mEPR);
    outputFile.append(simulationGSH.mEPR.right(simulationGSH.mEPR.length() - 
					       simulationGSH.mEPR.findRev('/') - 1));
  }
  else{
    tSGS = doc->createTextNode(visualizationGSH.mEPR);
    outputFile.append(visualizationGSH.mEPR.right(visualizationGSH.mEPR.length() - 
						  visualizationGSH.mEPR.findRev('/') - 1));
  }

  // Name of file to write job stdout to
  QDomElement eStdOut = doc->createElement("SimulationSTDOUTfile");
  root.appendChild(eStdOut);
  QDomText tStdOut = doc->createTextNode(outputFile+".stdout");
  eStdOut.appendChild(tStdOut);

  // Name of file to write job stderr to
  QDomElement eStdErr = doc->createElement("SimulationSTDERRfile");
  root.appendChild(eStdErr);
  QDomText tStdErr = doc->createTextNode(outputFile+".stderr");
  eStdErr.appendChild(tStdErr);

  // GSH of associated SGS
  QDomElement eSGS = doc->createElement("ReGSGSAddress");
  root.appendChild(eSGS);
  eSGS.appendChild(tSGS);
  
  return doc->toString();
}

//---------------------------------------------------------------------------
bool LauncherConfig::createNewConfigFile(){

  QString homePath = QDir::homeDirPath();
  cout << "Home directory = " << homePath << endl;
  QDir testDir = QDir(homePath+"/.realitygrid");
  if(!testDir.exists()){
    if(!testDir.mkdir(homePath+"/.realitygrid")){
      QMessageBox::critical( NULL, "Error with configuration file",
			     "File ~/.realitygrid/launcher.conf does not exist\n"
			     "and I cannot create the ~/.realitygrid directory.\n\n",
			     QMessageBox::Ok, 0, 0 );
      return false;
    }
  }

  QFile configFile(homePath+"/.realitygrid/launcher.conf");
  if(!configFile.open(IO_WriteOnly)){
      QMessageBox::critical( NULL, "Error with configuration file",
			     "File ~/.realitygrid/launcher.conf does not exist\n"
			     "and I cannot create it.\n\n",
			     QMessageBox::Ok, 0, 0 );
      return false;
  }

  QMessageBox::warning( NULL, "New configuration file",
			"The configuration file ~/.realitygrid/launcher.conf was missing\n"
			"so I will create one for you but you'll need to edit it.\n\n",
			QMessageBox::Ok, 0, 0 );

  TextViewDialog *textViewDialog = new TextViewDialog();
  textViewDialog->mTextEdit->setTextFormat(QTextEdit::PlainText);
  textViewDialog->mTextEdit->setText(mConfigFileContent);
  textViewDialog->mTextEdit->setReadOnly(FALSE);
  textViewDialog->exec();
  mConfigFileContent = textViewDialog->mTextEdit->text();

  // Now write a 'default' launcher.conf using the text originally held in 
  // mConfigFileContent (obtained from include/ConfigFileContent.h) that the
  // user has just edited.
  QTextStream stream(&configFile);
  stream << mConfigFileContent;
  configFile.close();

  return true;
}

//---------------------------------------------------------------------------
void LauncherConfig::readSecurityConfig(QString fileName){

  QFile configFile(fileName);

  if(!configFile.exists() || 
     (Get_security_config(fileName.ascii(), &registrySecurity) 
      != REG_SUCCESS) ){
    QMessageBox::critical( NULL, "Error with security configuration file",
			   "File "+fileName+" does not exist\n"
			   "or cannot be parsed.\n\n",
			   QMessageBox::Ok, 0, 0 );
    return;
  }
  return;
}
