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
    scripts. 
    @author Mark Riding
    @author Andrew Porter */

#define WITH_CDATA // ensure that gSoap retains CDATA in xml strings
#include "Gridifier.h"
#include "Utility.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "qapplication.h"
#include "qfile.h"
#include "qmessagebox.h"
#include "qinputdialog.h"

#include <ReG_Steer_Steerside.h>
#include <ReG_Steer_Browser.h>
#include <ReG_Steer_Utils.h>
#include "ReG_Steer_Utils_WSRF.h"
#include "ReG_Steer_Steerside_WSRF.h"
#include "soapH.h"

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
QString Gridifier::getSGSFactories(const QString &topLevelRegistry, 
				   const QString &desiredContainer,
				   const QString &className){
  QString result;

  QProcess *getSGSFactoriesProcess = new QProcess(QString("./get_" + 
							  className + 
							  "_factories.pl"));
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


void Gridifier::getSGSies(LauncherConfig *aConfig, 
			  QTable *aGSHTagTable){

  QStringList              result;
  struct registry_contents content;
  int                      i;

  mGSHTagTable = aGSHTagTable;
  if (mGSHTagTable == NULL)
    return;

  for (int i=mGSHTagTable->numRows(); i>0; i--){
    mGSHTagTable->removeRow(0);
  }
  mGSHTagTable->insertRows(0, 1);
  mGSHTagTable->setText(0, 0, "Searching for Running Jobs");

  // Get the passphrase for the user's key if registry is using
  // SSL
  if( !(aConfig->registrySecurity.passphrase[0]) ){
    bool    ok;
    QString passphrase;
    if(aConfig->registrySecurity.use_ssl){

      passphrase = QInputDialog::getText("RealityGrid Launcher", 
					 "Enter passphrase for X.509 key:", 
					 QLineEdit::Password,
					 QString::null, &ok, NULL );
      if ( !ok ) return; // Cancel if user didn't press OK
    }
    else{
      passphrase = QInputDialog::getText("RealityGrid Launcher", 
					 "Enter passphrase for registry:", 
					 QLineEdit::Password,
					 QString::null, &ok, NULL );
      if ( !ok ) return; // Cancel if user didn't press OK
    }
    strncpy(aConfig->registrySecurity.passphrase,
	    passphrase.ascii(), REG_MAX_STRING_LENGTH);
  }
#ifdef REG_WSRF
  if(Get_registry_entries_secure(aConfig->topLevelRegistryGSH.latin1(), 
				 &(aConfig->registrySecurity),
				 &content) != REG_SUCCESS){
    cout << "Get_registry_entries failed" << endl;
    return;
  }
#else
  if(Get_registry_entries(aConfig->topLevelRegistryGSH.latin1(), 
			  &content) != REG_SUCCESS){
    cout << "Get_registry_entries failed" << endl;
    return;
  }
#endif

  cout << "ARPDBG, got "<< content.numEntries << " entries from registry" 
       << endl;
  if(content.numEntries == 0)return;

  // check we've got a reference to the gshTagTable
  if (mGSHTagTable == NULL){
    cout << "getSGSies: NULL reference to gshTagTable!" << endl;
    return;
  }

  // clear out the table
  for (i=mGSHTagTable->numRows(); i>0; i--){
    mGSHTagTable->removeRow(0);
  }

  for (i=0; i<content.numEntries; i++){
    if(!strcmp(content.entries[i].service_type, "SWS") ||
       !strcmp(content.entries[i].service_type, "SGS")){
      mGSHTagTable->insertRows(mGSHTagTable->numRows(), 1);
      mGSHTagTable->setText(mGSHTagTable->numRows()-1, 0, 
			    QString(content.entries[i].gsh));
      QString tag(content.entries[i].user);
      tag += QString(" ") + QString(content.entries[i].application) +
	QString(" ") + QString(content.entries[i].job_description) +
	QString(" ") + QString(content.entries[i].start_date_time);
      mGSHTagTable->setText(mGSHTagTable->numRows()-1, 1, 
			    tag);
    }
  }

  Delete_registry_table(&content);

  return;
}

//---------------------------------------------------------------
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

//---------------------------------------------------------------
QString Gridifier::makeSGSFactory(const QString &container, 
				  const QString &topLevelRegistry,
				  const QString &className){
  QString result;

  QProcess *makeSGSFactoryProcess = new QProcess(QString("./make_" + 
							 className + 
							 "_factory.pl"));
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

//---------------------------------------------------------------
/* Create an SGS or SWS to associate with a simulation
 */
QString Gridifier::makeSteeringService(const QString &factory,
				       const LauncherConfig &config){
  QString result;

#ifndef REG_WSRF

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

#else // REG_WSRF is defined

  struct reg_job_details   job;
  char                    *chkTree;

  // Initialize job_details struct
  snprintf(job.userName, REG_MAX_STRING_LENGTH,
	   config.mJobData->mPersonLaunching.ascii());
  snprintf(job.group, REG_MAX_STRING_LENGTH, "RSS");
  snprintf(job.software, REG_MAX_STRING_LENGTH, 
	   config.mJobData->mSoftwareDescription.ascii());
  snprintf(job.purpose, REG_MAX_STRING_LENGTH,
	   config.mJobData->mPurposeOfJob.ascii());
  snprintf(job.inputFilename, REG_MAX_STRING_LENGTH,
	   config.mInputFileName.ascii());
  job.lifetimeMinutes = config.mTimeToRun;
  snprintf(job.passphrase, REG_MAX_STRING_LENGTH,
	   config.mServicePassword.ascii());

  // Create a new checkpoint tree if requested
  if(config.treeTag.length()){
    chkTree = Create_checkpoint_tree(config.checkPointTreeFactoryGSH.ascii(), 
				     config.treeTag.ascii());
    if(!chkTree){
      QMessageBox::critical( NULL, "Checkpoint tree error",
                             "Failed to create new checkpoint tree.\n\n",
                             QMessageBox::Ok, 0, 0 );
      return result;
    }
  }
  else{
    chkTree = (char *)(config.currentCheckpointGSH.ascii());
  }
  snprintf(job.checkpointAddress, REG_MAX_STRING_LENGTH, chkTree);

  char *EPR = Create_steering_service(&job,
				      factory.ascii(), 
				      config.topLevelRegistryGSH.ascii(),
                                      &(config.registrySecurity));
  result = QString(EPR);

  struct soap mySoap;
  soap_init(&mySoap);

  // Put contents of input deck (if any) into the RP doc of the SWS
  if( EPR && !(config.mInputFileName.isEmpty()) ){

    QFile *inputFile = new QFile(config.mInputFileName);
    inputFile->open( IO_ReadOnly );
    QByteArray fileData = inputFile->readAll();
    inputFile->close();

    if(Set_resource_property(&mySoap, EPR, job.userName, 
			     job.passphrase, fileData.data() ) != REG_SUCCESS){
      cout << "Gridifier::makeSteeringService: WARNING - failed to store "
	"job input file on SWS." << endl;
    }
  }

  if(EPR && config.mIOProxyPort){

    QString resourceProp("<dataSink><Proxy><address>");
    resourceProp.append(config.mIOProxyAddress);
    resourceProp.append("</address><port>");
    resourceProp.append(QString::number(config.mIOProxyPort, 10));
    resourceProp.append("</port></Proxy></dataSink>");
     
    cout << "Gridifier::makeSteeringService: Calling Set_resource_property "
      "with >>" << resourceProp << "<<" << endl;

    if(Set_resource_property(&mySoap, EPR,
			     config.simulationGSH.mSecurity.userDN,
			     config.simulationGSH.mSecurity.passphrase,
			     (char *)(resourceProp.ascii())) != REG_SUCCESS){
      cout << "Gridifier::makeSteeringService: WARNING - failed to set "
	"details of ioProxy on the SWS" << endl;
    }
  }

  soap_end(&mySoap);
  soap_done(&mySoap);

  if(EPR){
    printf("Address of SWS = %s\n", EPR);
  }
  else{
    printf("FAILED to create SWS :-(\n");
  }

#endif // ndef REG_WSRF

  return result;
}

//---------------------------------------------------------------
/* Create a child MetaSGS or SWS to associate with a simulation
 */
QString Gridifier::makeSteeringService(const QString &factory,
				       const LauncherConfig &config,
				       const QString &parentEPR){
  struct soap mySoap;
  struct sws__AddChildRequest request;
  struct sws__AddChildResponse response;
  struct wsrp__SetResourcePropertiesResponse setRPresponse;
  char   tmpBuf1[256];
  char   tmpBuf2[256];
  QString callBuf;
  QString result;
  QString epr = this->makeSteeringService(factory, config);

  if(epr.isEmpty()) return epr;

  // Tell the child service about its parent
  callBuf = "<parentEPR>" + parentEPR + "</parentEPR>";
  soap_init(&mySoap);

  cout << "makeSteeringService: Calling SetResourceProperties with >>" <<
    callBuf << "<<" << endl;

  if(soap_call_wsrp__SetResourceProperties(&mySoap, epr, "", 
					   (char*)callBuf.ascii(),
					   &setRPresponse) != SOAP_OK){
    cout << "makeSteeringService: SetResourceProperties failed:" << endl;
    soap_print_fault(&mySoap, stderr);
    soap_end(&mySoap);
    soap_done(&mySoap);
    return result;
  }

  // Now tell the parent about this child
  snprintf(tmpBuf1, 256, "%s", epr.ascii());
  snprintf(tmpBuf2, 256, "%s", 
	   config.mJobData->mSoftwareDescription.ascii());
  request.epr = tmpBuf1;
  request.name = tmpBuf2;

  if( soap_call_sws__AddChild(&mySoap, parentEPR, "", 
			      request, &response) != SOAP_OK ){
    cout << "makeSteeringService: AddChild failed:" << endl;
    soap_print_fault(&mySoap, stderr);
  }
  else {
    result = epr;
  }

  soap_end(&mySoap);
  soap_done(&mySoap);
  return result;
}

//---------------------------------------------------------------
QString Gridifier::makeVizSGS(const QString &factory,
                              const LauncherConfig &config){
  QString result;

#ifndef REG_WSRF
  QProcess *makeVizSGSProcess = new QProcess(QString("./make_vis_sgs.pl"));
  makeVizSGSProcess->setWorkingDirectory(mScriptsDir);
  makeVizSGSProcess->addArgument(factory);
  makeVizSGSProcess->addArgument(config.mJobData->toXML(QString("SGS")));
  makeVizSGSProcess->addArgument(config.topLevelRegistryGSH);
  makeVizSGSProcess->addArgument(config.simulationGSH.mEPR);
  makeVizSGSProcess->addArgument(QString::number(config.mTimeToRun));
  makeVizSGSProcess->start();

  while (makeVizSGSProcess->isRunning()){
    usleep(10000);
    mApplication->processEvents();
  }

  if(makeVizSGSProcess->canReadLineStdout()){
    result = QString(makeVizSGSProcess->readStdout());
    cout << "stdout: " << result << endl;
  }
  else{
    cout << "Full line of stdout unavailable" << endl;
  }
  if(makeVizSGSProcess->canReadLineStderr()){
    cout << "stderr: " << QString(makeVizSGSProcess->readStderr()) << endl;
  }
  else{
    cout << "Full line of stderr unavailable" << endl;
  }

  result = QString(makeVizSGSProcess->readStdout()).stripWhiteSpace();

#else // REG_WSRF is defined

  char                                       purpose[1024];
  char                                       proxyAddress[1024];
  int                                        proxyPort;
  char                                       iodef_label[256];
  char                                      *ioTypes;
  struct soap                                mySoap;
  char                                      *EPR;
  struct msg_struct                         *msg;
  xmlDocPtr                                  doc;
  xmlNsPtr                                   ns;
  xmlNodePtr                                 cur;
  struct io_struct                          *ioPtr;
  int                                        i, count;
  struct reg_job_details                     job;

  /* Obtain the IOTypes from the data source */
  soap_init(&mySoap);
  if( Get_resource_property (&mySoap,
			     config.simulationGSH.mEPR.ascii(),
			     config.simulationGSH.mSecurity.userDN,
			     config.simulationGSH.mSecurity.passphrase,
			     "ioTypeDefinitions",
			     &ioTypes) != REG_SUCCESS ){

    cout << "Call to get ioTypeDefinitions ResourceProperty on "<< 
      config.simulationGSH.mEPR << " failed" << endl;
    return result;
  }
  cout << "Got ioTypeDefinitions >>" << ioTypes << "<<" << endl;

  if( !(doc = xmlParseMemory(ioTypes, strlen(ioTypes))) ||
      !(cur = xmlDocGetRootElement(doc)) ){
    fprintf(stderr, "Hit error parsing buffer\n");
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return result;
  }

  ns = xmlSearchNsByHref(doc, cur,
            (const xmlChar *) "http://www.realitygrid.org/xml/steering");

  if ( xmlStrcmp(cur->name, (const xmlChar *) "ioTypeDefinitions") ){
    cout << "ioTypeDefinitions not the root element" << endl;
    return result;
  }
  /* Step down to ReG_steer_message and then to IOType_defs */
  cur = cur->xmlChildrenNode->xmlChildrenNode;

  msg = New_msg_struct();
  msg->msg_type = IO_DEFS;
  msg->io_def = New_io_def_struct();
  parseIOTypeDef(doc, ns, cur, msg->io_def);
  Print_msg(msg);

  if(!(ioPtr = msg->io_def->first_io) ){
    fprintf(stderr, "Got no IOType definitions from data source\n");
    return result;
  }
  i = 0;
  printf("Available IOTypes:\n");
  while(ioPtr){
    if( !xmlStrcmp(ioPtr->direction, (const xmlChar *)"OUT") ){
      printf("  %d: %s\n", i++, (char *)ioPtr->label);
    }
    ioPtr = ioPtr->next;
  }
  count = i-1;
  /*
  printf("Enter IOType to use as data source (0-%d): ", count);
  while(1){
    if(scanf("%d", &i) == 1)break;
  }
  printf("\n");
  */
  // ARPDBG - temporary hardwire to select first output IOType
  //i = 0;

  count = 0; ioPtr = msg->io_def->first_io;
  while(ioPtr){
    if( !xmlStrcmp(ioPtr->direction, (const xmlChar *)"OUT") ){
      //if(count == i){ ARPDBG - temporary hardwire to select first output IOType
	strncpy(iodef_label, (char *)(ioPtr->label), 256);
	break;
	//}ARPDBG
      count++;
    }
    ioPtr = ioPtr->next;
  }

  Delete_msg_struct(&msg);

  // Initialize job_details struct
  snprintf(job.userName, REG_MAX_STRING_LENGTH,
	   config.mJobData->mPersonLaunching.ascii());
  snprintf(job.group, REG_MAX_STRING_LENGTH, "RSS");
  snprintf(job.software, REG_MAX_STRING_LENGTH, 
	   config.mJobData->mSoftwareDescription.ascii());
  snprintf(job.purpose, REG_MAX_STRING_LENGTH,
	   config.mJobData->mPurposeOfJob.ascii());
  snprintf(job.inputFilename, REG_MAX_STRING_LENGTH,
	   config.mInputFileName.ascii());
  job.lifetimeMinutes = config.mTimeToRun;
  snprintf(job.passphrase, REG_MAX_STRING_LENGTH, 
	   config.simulationGSH.mSecurity.passphrase);
  snprintf(job.checkpointAddress, REG_MAX_STRING_LENGTH, 
	   config.currentCheckpointGSH.ascii());

  // Now create SWS for the vis
  if( !(EPR = Create_steering_service(&job,
				      factory.ascii(),
				      config.topLevelRegistryGSH.ascii(),
				      &(config.registrySecurity)) ) ){
    cout << "FAILED to create SWS for " << 
      config.mJobData->mSoftwareDescription << " :-(" << endl;
    soap_end(&mySoap);
    soap_done(&mySoap);
    return result;
  }

  // Finally, set it up with information on the data source

  // Check to see whether or not an ioProxy is being used
  if( Get_resource_property (&mySoap,
			     config.simulationGSH.mEPR.ascii(),
			     config.simulationGSH.mSecurity.userDN,
			     config.simulationGSH.mSecurity.passphrase,
			     "dataSink",
			     &ioTypes) != REG_SUCCESS ){

    cout << "Call to get dataSink ResourceProperty on "<< 
      config.simulationGSH.mEPR << " failed" << endl;
    return result;
  }

  // <sws:dataSink xmlns:sws="http://www.sve.man.ac.uk/SWS">
  //   <Proxy>
  //     <address>methuselah.mvc.mcc.ac.uk</address>
  //     <port>50010</port>
  //   </Proxy>
  // </sws:dataSink>

  cout << "ARPDBG: Got dataSink >>" << ioTypes << "<<" << endl;
  char *ptr, *ptr0, *ptr1;
  proxyPort = 0;
  proxyAddress[0] = '\0';

  if( ptr0 = strstr(ioTypes, "<Proxy>") ){
    // An ioProxy IS being used by the simulation

    ptr1 = NULL;
    ptr = strstr(ptr0, "<address");
    if(ptr)ptr = strstr(ptr, ">");
    if(ptr)ptr++;
    if(ptr)ptr1 = strstr(ptr, "</address>");
    if(ptr1)*ptr1 = '\0';
    if(ptr1)strncpy(proxyAddress, ptr, 1024);
    if(ptr1)*ptr1 = '<';

    ptr = strstr(ptr0, "<port");
    if(ptr)ptr = strstr(ptr, ">");
    if(ptr)ptr++;
    if(ptr)ptr1 = strstr(ptr, "</port>");
    if(ptr1)*ptr1 = '\0';
    if(ptr1)proxyPort = atoi(ptr);
    
    int idx = snprintf(purpose, 1024, "<dataSink><Proxy><address>%s</address>"
		       "<port>%d</port></Proxy></dataSink>",
		       proxyAddress, proxyPort);
    snprintf(&(purpose[idx]), 1024, "<dataSource><Proxy><address>%s"
	     "</address><port>%d</port></Proxy><sourceLabel>%s"
	     "</sourceLabel></dataSource>",
	     proxyAddress, proxyPort, iodef_label);
  }
  else{
    snprintf(purpose, 1024, "<dataSource><sourceEPR>%s</sourceEPR>"
	     "<sourceLabel>%s</sourceLabel></dataSource>",
	     config.simulationGSH.mEPR.ascii(), iodef_label);
  }
     
  printf("Calling Set_resource_property with >>%s<<\n",
	 purpose);

  if(Set_resource_property(&mySoap, EPR,
			   config.simulationGSH.mSecurity.userDN,
			   config.simulationGSH.mSecurity.passphrase,
			   purpose) != REG_SUCCESS){
    cout << "Set_resource_property failed:" << endl;
    soap_end(&mySoap);
    soap_done(&mySoap);
    return result;
  }
  result = QString(EPR);

  soap_end(&mySoap);
  soap_done(&mySoap);
#endif // ndef REG_WSRF

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
    fileText += "REG_SGS_ADDRESS="+config.visualizationGSH.mEPR+"\n";
    fileText += "REG_PASSPHRASE="+
      QString(config.visualizationGSH.mSecurity.passphrase)+"\n";
  }
  else{
    fileText += "REG_SGS_ADDRESS="+config.simulationGSH.mEPR+"\n";
    fileText += "REG_PASSPHRASE="+
      QString(config.simulationGSH.mSecurity.passphrase)+"\n";
  }

  fileText += "export HOST_JOB_MGR CONTAINER STEER_STD_OUT_FILE STEER_STD_ERR_FILE SIM_STD_OUT_FILE SIM_STD_ERR_FILE CLIENT_DISPLAY GLOBUS_LOCATION SIM_HOSTNAME SIM_PROCESSORS SIM_INFILE VIZ_TYPE VIZ_PROCESSORS SIM_USER FIREWALL REG_SGS_ADDRESS REG_VIZ_GSH REG_PASSPHRASE\n\n";

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
int Gridifier::launchSimScript(const QString &scriptConfigFileName,
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
  
  if (launchSimScriptProcess->canReadLineStdout()){
    QString out(launchSimScriptProcess->readStdout());
    cout << "Stdout:" << endl << out << endl;
    if(out.contains("ERROR")){
      return REG_FAILURE;
    }
  }
  if (launchSimScriptProcess->canReadLineStderr()){
    QString out(launchSimScriptProcess->readStderr());
    cout << "Stderr:" << endl << out << endl;
    if(out.contains("ERROR")){
      return REG_FAILURE;
    }
  }
  return REG_SUCCESS;
}

/** Method calls appropriate xxx_launch.sh script to
 *  actually launch the visualization job on the target machine
 */
int Gridifier::launchVizScript(const QString &scriptConfigFileName,
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


  if (launchVizScriptProcess->canReadLineStdout()){
    QString out(launchVizScriptProcess->readStdout());
    cout << "Stdout:" << endl << out << endl;
    if(out.contains("ERROR")){
      return REG_FAILURE;
    }
  }
  if (launchVizScriptProcess->canReadLineStderr()){
    QString out(launchVizScriptProcess->readStderr());
    cout << "Stderr:" << endl << out << endl;
    if(out.contains("ERROR")){
      return REG_FAILURE;
    }
  }
  return REG_SUCCESS;
}


/** Special case to launch a viz on Argonne's cluster
 *  this will always render to multicast
 */
void Gridifier::launchArgonneViz(const LauncherConfig &config){
  QProcess *launchArgonneVizProcess = new QProcess(QString("./argonneVis.sh"));
  launchArgonneVizProcess->setWorkingDirectory(mScriptsDir);
  launchArgonneVizProcess->addArgument(config.visualizationGSH.mEPR);
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

  if(setServiceDataProcess->canReadLineStdout()){
    cout << "Stdout:" << endl << setServiceDataProcess->readStdout() << endl;
  }
  if (setServiceDataProcess->canReadLineStderr()){
    cout << "Stderr:" << endl << setServiceDataProcess->readStderr() << endl;
  }
}

/** Cleans up when launch fails
 */
void Gridifier::cleanUp(LauncherConfig *config){
#ifdef REG_WSRF
  if(!(config->simulationGSH.mEPR.isEmpty())){
    Destroy_steering_service((char*)config->simulationGSH.mEPR.ascii(),
			     &(config->simulationGSH.mSecurity)); // ReG lib
  }
  if(!(config->visualizationGSH.mEPR.isEmpty())){
    Destroy_steering_service((char*)config->visualizationGSH.mEPR.ascii(),
			     &(config->visualizationGSH.mSecurity)); // ReG lib
  }
#endif
} 

void Gridifier::cleanUp(SteeringService *service){
#ifdef REG_WSRF
  if(service && !(service->mEPR.isEmpty())){
    Destroy_steering_service((char*)service->mEPR.ascii(),
			     &(service->mSecurity)); // ReG lib
  }
#endif
} 

void Gridifier::getContainerList(LauncherConfig *aConfig){
#ifdef REG_WSRF
  bool                     ok;
  int                      i;
  struct registry_contents content;

  // Get the passphrase for the user's key if registry is using
  // SSL
  if( !(aConfig->registrySecurity.passphrase[0]) ){
    QString passphrase;
    if( aConfig->registrySecurity.use_ssl){

      passphrase = QInputDialog::getText("RealityGrid Launcher", 
					 "Enter passphrase for X.509 key:", 
					 QLineEdit::Password,
					 QString::null, &ok, NULL );
      if ( !ok ) return; // Cancel if user didn't press OK
    }
    else{

      passphrase = QInputDialog::getText("RealityGrid Launcher", 
					 "Enter passphrase for registry:", 
					 QLineEdit::Password,
					 QString::null, &ok, NULL );
      if ( !ok ) return; // Cancel if user didn't press OK
    }
    strncpy(aConfig->registrySecurity.passphrase,
	    passphrase.ascii(), REG_MAX_STRING_LENGTH);
  }
  if(Get_registry_entries_secure(aConfig->topLevelRegistryGSH.ascii(), 
				 &(aConfig->registrySecurity),
				 &content) != REG_SUCCESS){
    cout << "Get_registry_entries_secure failed" << endl;
    return;
  }
  if(content.numEntries == 0)return;

  ok = false;
  for(i=0; i<content.numEntries; i++){
    if(!strcmp(content.entries[i].service_type, "ServiceGroup") &&
       !strcmp(content.entries[i].job_description, "Container registry")){
      ok = true;
      break;
    }
  }
  if(!ok){
    cout << "ERROR, registry of containers not found" << endl;
    return;
  }
  QString containerRegistryEPR = QString(content.entries[i].gsh);
  Delete_registry_table(&content);

  if(Get_registry_entries_secure(containerRegistryEPR.ascii(), 
				 &(aConfig->registrySecurity),
				 &content) != REG_SUCCESS){
    cout << "Get_registry_entries_secure for containers failed" << endl;
    return;
  }
  if(content.numEntries == 0){
    cout << "No containers found in registry" << endl;
    return;
  }
  aConfig->mContainerList.clear();
  for(i=0; i<content.numEntries; i++){
    if(!strcmp(content.entries[i].service_type, "Container")){
      aConfig->mContainerList.append(QString(content.entries[i].gsh));
      cout << "Container "<< i << ": " << aConfig->mContainerList.last() << endl;
    }
  }
  Delete_registry_table(&content);
#endif // def REG_WSRF
} 
