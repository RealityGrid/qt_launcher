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

    Initial version by: M Riding, 29.09.2003
    
---------------------------------------------------------------------------*/

/* Important note - libxml was used for this class, but QT does provide */
/* its own set of XML classes. I just didn't realise that when I began..*/

/* MORE IMPORTANTLY - I'll convert all this to QT xml support later.... */

#include "LauncherConfig.h"
#include <iostream>
#include "libxml/parser.h"
#include "qmdcodec.h"
#include <qdom.h>
#include <qfile.h>

using namespace std;

LauncherConfig::LauncherConfig(){

  migration  = false;
  restart    = false;
  newTree    = false;
  mTimeToRun = 30;

  mJobData = new JobMetaData;
}

LauncherConfig::LauncherConfig(QString file){
  readConfig(file);
}

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
  xmlNodePtr targets;
  xmlNodePtr vizTargets;
  xmlNodePtr applications;

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

  // Grab the SGSs
  childOfRoot = root->xmlChildrenNode;
  
  while (childOfRoot != NULL){
    // Retrieve the GSH information
    if ((!xmlStrcmp(childOfRoot->name, (const xmlChar *)"GSHs"))){
      
      aGSH = childOfRoot->xmlChildrenNode;

      while (aGSH != NULL){
        // test to see which element we've got, and fill in the appropriate class member variable

        // Top Level Registry
        if ((!xmlStrcmp(aGSH->name, (const xmlChar *)"topLevelRegistry"))) {
          xmlChar *key = xmlGetProp(aGSH, (xmlChar*)"value");
          topLevelRegistryGSH = QString((const char*)key);
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
          SGSGSH = QString((const char*)key);
		      xmlFree(key);
        }

        aGSH = aGSH->next;
      }
      
    }

    // Retrieve the target machine information
//    if ((!xmlStrcmp(childOfRoot->name, (const xmlChar *)"component"))){
//
//      // for code visibility purposes
//      aComponent = childOfRoot;
//
//      xmlChar *key = xmlGetProp(aComponent, (xmlChar*)"type");
//
//      if (!xmlStrcmp(key, (const xmlChar*)"lb3d")){
//        simComponentType = lb3d;
//
//        xmlChar *target = xmlGetProp(aComponent, (xmlChar*)"targetMachineName");
//        simTargetMachine = QString((const char*)target);
//        xmlFree(target);
//
//        xmlChar *processors = xmlGetProp(aComponent, (xmlChar*)"numberProcessors");
//        simNumberProcessors = atoi((const char*)processors);
//        xmlFree(processors);
//
//        xmlChar *tag = xmlGetProp(aComponent, (xmlChar*)"tag");
//        simTag = QString((const char*)tag);
//        xmlFree(tag);
//      }
//      if (!xmlStrcmp(key, (const xmlChar*)"lb3dviz")){
//        vizComponentType = lb3dviz;
//
//        xmlChar *target = xmlGetProp(aComponent, (xmlChar*)"targetMachineName");
//        vizTargetMachine = QString((const char*)target);
//        xmlFree(target);
//
//        xmlChar *processors = xmlGetProp(aComponent, (xmlChar*)"numberProcessors");
//        vizNumberProcessors = atoi((const char*)processors);
//        xmlFree(processors);
//
//        xmlChar *pipes = xmlGetProp(aComponent, (xmlChar*)"numberPipes");
//        vizNumberPipes = atoi((const char*)pipes);
//        xmlFree(pipes);
//
//        xmlChar *vizServerChoice = xmlGetProp(aComponent, (xmlChar*)"vizServer");
//        if (!xmlStrcmp(vizServerChoice, (const xmlChar*)"yes")){
//         vizServer = true;
//         // if we're using vizServer then there can only be one graphics pipe - enforce this
//         vizNumberPipes = 1;
//        }
//        else {
//         vizServer = false;
//        }
//        xmlFree(vizServerChoice);
//
//        xmlChar *vizTypeStr = xmlGetProp(aComponent, (xmlChar*)"type");
//        vizType = atoi((const char*)vizTypeStr);
//        xmlFree(vizTypeStr);
//
//        xmlChar *tag = xmlGetProp(aComponent, (xmlChar*)"tag");
//        vizTag = QString((const char*)tag);
//        xmlFree(tag);
//      }
//      if (!xmlStrcmp(key, (const xmlChar*)"miniapp")){
//        simComponentType = miniapp;
//
//        xmlChar *target = xmlGetProp(aComponent, (xmlChar*)"targetMachineName");
//        simTargetMachine = QString((const char*)target);
//        xmlFree(target);
//
//        xmlChar *processors = xmlGetProp(aComponent, (xmlChar*)"numberProcessors");
//        simNumberProcessors = atoi((const char*)processors);
//        xmlFree(processors);
//
//        xmlChar *tag = xmlGetProp(aComponent, (xmlChar*)"tag");
//        simTag = QString((const char*)tag);
//        xmlFree(tag);
//      }
//
//      xmlFree(key);
//    }

    // Retrieve the container information
    if ((!xmlStrcmp(childOfRoot->name, (const xmlChar*)"containers"))){
      containers = childOfRoot->xmlChildrenNode;

      while (containers != NULL){
        
        if (!xmlStrcmp(containers->name, (const xmlChar*)"container")){
          xmlChar *containerName = xmlNodeListGetString(doc, containers->xmlChildrenNode, 1);
          xmlChar *port = xmlGetProp(containers, (const xmlChar*)"port");
          Container tContainer((const char*)containerName, QString((const char*)port).toInt());
          containerList += tContainer;
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

    if (!xmlStrcmp(childOfRoot->name, (const xmlChar*)"globus")){
      globusLocation = (const char*)xmlGetProp(childOfRoot, (const xmlChar*)"location");
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
}

/** Method uses libxml2 to create an xml encoding of the current configuration
 *  and save the file to disk
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
  xmlNewProp(SGS, (const xmlChar*)"value", (const xmlChar*)SGSGSH.latin1());

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
  for ( QValueList<Container>::Iterator it = containerList.begin(); it != containerList.end(); ++it ) {
      xmlNodePtr t = xmlNewTextChild(containers, NULL, (const xmlChar*)"container", (const xmlChar*)((*it).mContainer).latin1());
      sprintf(buff, "%d", (*it).mPort);
      xmlNewProp(t, (const xmlChar*)"port", (const xmlChar*)buff);
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

/**
 * Use Qt XML support to generate XML document describing the
 * job to be launched.
 */
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
    tSGS = doc->createTextNode(simulationGSH);
    outputFile.append(simulationGSH.right(simulationGSH.length() - simulationGSH.findRev('/') - 1));
  }
  else{
    tSGS = doc->createTextNode(visualizationGSH);
    outputFile.append(visualizationGSH.right(visualizationGSH.length() - visualizationGSH.findRev('/') - 1));
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


