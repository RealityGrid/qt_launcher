/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qdom.h"
#include "qfiledialog.h"
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "qpopupmenu.h"
#include "qprocess.h"
#include "qstatusbar.h"

#include "componentlauncher.h"
#include "RunningJobsDialog.h"
#include "GlobalParamConstructionForm.h"
#include "textviewdialog.h"
#include "CheckPointTreeItem.h"
#include "Gridifier.h"
#include "LauncherConfig.h"
#include "JobStatusThread.h"
#include "ProgressBarThread.h"
#include "ReG_Steer_Steerside_WSRF.h"

// Reuse this from reg_qt_steerer
#include "chkptvariableform.h"
#include "soapH.h"

/** @file reglauncher.ui.h
    @brief Main class for the GUI side of launcher
*/

using namespace std;

QProcess           *proxyStatus = NULL;
/** Variables to hold the configuration of the first
    and, optionally, second components to be launched */
LauncherConfig      config, config2;
CheckPointTree     *cpt = NULL;
CheckPointTreeItem *rightMouseCheckPointTreeItem = NULL;
int checkPointTreeListViewPreviousSelection = -1;

/** Reads the launcher configuration file "launcher.conf"
  * and tweaks the way the checkpoint tree is displayed
  */
void RegLauncher::init(){
  QString homeDir = QString(getenv("HOME"));
  config.readConfig(homeDir + "/.realitygrid/launcher.conf");
  config2.readConfig(homeDir + "/.realitygrid/launcher.conf");

  checkPointTreeListView->setRootIsDecorated(true);

  QDir lDir(config.mScriptsDirectory);
  if (!lDir.isReadable()){
    QMessageBox::critical(this, "Configuration error",
			  "Cannot read from the scripts directory\n("
			  +config.mScriptsDirectory+")\n specified "
			  "in ~/.realitygrid/launcher.conf - launching will "
			  "NOT work.",  QMessageBox::Ok, 0, 0);
  }

  lDir.setPath(config.mScratchDirectory);
  if (!lDir.isReadable()){
    QMessageBox::critical(this, "Configuration error",
			  "Cannot read from the scratch directory\n("
			  +config.mScratchDirectory+")\n specified "
			  "in ~/.realitygrid/launcher.conf - launching will "
			  "NOT work.",  QMessageBox::Ok, 0, 0);
  }

  QFile lFile(config.mSteererBinaryLocation);
  if(!lFile.exists()){
    QMessageBox::warning( NULL, "Configuration error",
			  "Steerer binary is not in the location \n("
			  +config.mSteererBinaryLocation+")\n specified in "
			  "~/.realitygrid/launcher.conf.  Firing-up the\n"
			  "steering client from within the Launcher will not "
			  "be possible\n\n",
			  QMessageBox::Ok, 0, 0 );
  }

  gridifier.setScriptsDirectory(config.mScriptsDirectory);

  // Read in security configuration
  config.readSecurityConfig(homeDir + "/.realitygrid/security.conf");
  config2.readSecurityConfig(homeDir + "/.realitygrid/security.conf");

  if(config.topLevelRegistryGSH.startsWith("https://")){
    config.registrySecurity.use_ssl = REG_TRUE;
    config2.registrySecurity.use_ssl = REG_TRUE;
  }
  else{
    config.registrySecurity.use_ssl = REG_FALSE;
    config2.registrySecurity.use_ssl = REG_FALSE;
  }

  gridifier.getContainerList(&config);
}

/** Store a pointer to the object representing the type of application
    that is being launched */
void RegLauncher::setApplication(QApplication *aApplication){
  mApplication = aApplication;
  gridifier.setApplication(mApplication);
}


void RegLauncher::fileNew()
{ 

}

/** File->Open slot
 *  give a file dialog and try and load in the resulting xml configuration file
 */
void RegLauncher::fileOpen()
{
  QString s = QFileDialog::getOpenFileName(QDir::homeDirPath(), "Launcher Config file (*.xml *.conf)", this, "open file dialog" "Choose a file" );
  if (s!=NULL)
    config.readConfig(s);
}

/** File->Save slot
 *  give a file dialog and try to save the configuration in an xml file
 */
void RegLauncher::fileSave()
{
  QString s = QFileDialog::getSaveFileName(QDir::homeDirPath(), "Launcher Config file (*.xml *.conf)", this, "save file dialog" "Choose a file" );
  if (s!=NULL)
    config.writeConfig(s);
}

void RegLauncher::fileSaveAs()
{

}

void RegLauncher::fileExit()
{
    close();
}

void RegLauncher::migrateSimSlot()
{
  // for a migration:
  //  * get a list of active jobs
  //  * allow the user to select one
  //  * force that job to take a checkpoint
  //  * keep a reference to that checkpoint
  //  * stop the job
  //  * allow the user to select a new machine
  //  * copy the data and input files to that new machine
  //  * start the new job
  
  // Check that user has a valid proxy
  this->proxyInfo();

  RunningJobsDialog rjd;

  QString selectedGSH;
  rjd.setConfig(&config);
  // Read the note at the RunningJobsDialog::setResultString implementation
  rjd.setResultString(&selectedGSH);
  if (!rjd.exec()){
    return;
  }

  // we should now have access to the sgs of the running job,
  // so force it to take a checkpoint and halt
  consoleOutSlot("Taking a checkpoint, and stopping the job...");
  QString checkPointGSH = gridifier.checkPointAndStop(selectedGSH);
  cout << checkPointGSH << endl;

  checkPointGSH = checkPointGSH.stripWhiteSpace();
  config.currentCheckpointGSH = checkPointGSH;

  QString inputFileText = getInputFileFromCheckPoint(checkPointGSH);

  if (inputFileText.length() != 0){
    QString checkPointDataText = getDataFileFromCheckPoint(checkPointGSH);

    // Get the ID string of the app. that created this checkpoint
    QString appName;
    QString checkUIDString;
    QStringList fileNames;
    parseChkPtMetaData(checkPointDataText, appName, checkUIDString, fileNames);

    if(appName.length()==0 ){
      consoleOutSlot("Failed to get name of application from checkpoint metadata");
      return;
    }
    else if(checkUIDString.length()==0){
    	consoleOutSlot("Failed to get UID of checkpoint from metadata");
     	return;
    }
    cout << "ARPDBG: appName = " << appName << endl;
    
    // Work out which of the applications is being migrated
    config.mAppToLaunch = NULL;
    for(unsigned int i=0; i<config.applicationList.count(); i++){
      // ARPDBG
      // IMPORTANT - this will require more care if we want to match up
      // version numbers rather than crudely checking the app name
      if(appName.contains(config.applicationList[i].mAppName, FALSE) == 1){
        config.mAppToLaunch = &(config.applicationList[i]);
        break;
      }
    }
    if(!config.mAppToLaunch){

    	consoleOutSlot("Failed to match name of application used to create checkpoint"
                     "with those listed in default.conf");
     	return;
    }

    cout << "ARPDBG: we are MIGRATING "<< config.mAppToLaunch->mAppName << endl;
    if(appName.contains("lbe3d", FALSE) == 1){
      patchLb3dInputFileText(inputFileText, checkUIDString);
    }
    else if(appName.contains("namd", FALSE) == 1){
      patchNamdInputFileText(inputFileText, checkUIDString, fileNames);
    }

    // cache it
    // we need to copy the selected/edited input file to the target machine
    QString inputFileName = config.mScratchDirectory+"/ReG_tmp_input_file";

    config.mInputFileName = inputFileName;

    // and so we can launch the migration version of the component
    // launcher in order to find out where we're going to move the
    // job to

    ComponentLauncher *migrater = new ComponentLauncher();
    config.migration = true;
    config.restart = false;
    config.newTree = false;
    migrater->setConfig(&config);
    
    // and insert it into the wizard's text edit box
    migrater->setInputFileTextEdit(inputFileText);

    // Run the wizard - it will automatically populate the active
    // jobs list
    bool wizardOk = migrater->exec();

    if (!wizardOk)
      return;

    migrater->getInputFileTextEditText(&inputFileText);

    QFile inputFile( inputFileName );
    if ( inputFile.open( IO_WriteOnly ) ) {
      QTextStream stream( &inputFile );
      stream << inputFileText;
      inputFile.close();
    }
    
    // now launch!
    commonLaunchCode();    
  }
  
}

//----------------------------------------------------------------------
/// Consider sticking this method in a separate soap class
QString RegLauncher::getInputFileFromCheckPoint(const QString &checkPointGSH)
{
  QString result;
  
  struct soap soap;
  soap_init(&soap);

#ifdef REG_WSRF
  char    *rp;

  if( Get_resource_property(&soap,
			    checkPointGSH.ascii(),
			    "", "",
			    "inputFileContent", &rp) == REG_SUCCESS){
    result = QString(rp);
    // Strip off first and last xml tags
    int index = result.find("><");
    index++;
    int last = result.findRev("><");
    last++;
    result = result.mid(index, last-index);
  }

#else
  rgt__getInputFileResponse inputFileResponse;

  if (soap_call_rgt__getInputFile(&soap, checkPointGSH, "", NULL,
				  &inputFileResponse))
    soap_print_fault(&soap, stderr);
  else{
    result = inputFileResponse._getInputFileReturn;
  }

#endif

  soap_end(&soap);
  soap_done(&soap);

  return result;
}

//-------------------------------------------------------------------------
/// Consider sticking this method in a separate soap class
QString RegLauncher::getDataFileFromCheckPoint(const QString &checkPointGSH)
{
  QString result;
  char    *rp;
  struct soap soap;
  soap_init(&soap);
  
#ifdef REG_WSRF

  if( Get_resource_property(&soap,
			    checkPointGSH.ascii(),
			    "", "",
			    "checkPointData", &rp) == REG_SUCCESS){
    result = QString(rp);
    //result.replace(QRegExp("(<\/?reg:checkPointData>)(>| *xmlns=\".*\">)"), "");
    result.replace(QRegExp("^.*<Checkpoint_data"), "<Checkpoint_data");
    // . doesn't match a newline in regexp
    result.replace(QRegExp("<\/Checkpoint_data>(\n|.)*$"), "<\/Checkpoint_data>");
  }

#else

  rgt__getCheckPointDataResponse chkptDataResponse;
  if (soap_call_rgt__getCheckPointData(&soap, checkPointGSH, "", 
				       NULL, &chkptDataResponse))
    soap_print_fault(&soap, stderr);
  else{
    result = chkptDataResponse._getCheckPointDataReturn;
  }

#endif // def REG_WSRF

  soap_end(&soap);
  soap_done(&soap);

  return result;
}


//---------------------------------------------------------------------
void RegLauncher::patchLb3dInputFileText(QString &inputFileText,
                                         QString &chkUIDString)
{
  // remove the leading 'check*'
  if (chkUIDString.startsWith("check*")){
    chkUIDString = chkUIDString.right(chkUIDString.length() - 6);
  }
  else {
    // make sure that there is a leading 'check*'
    // this should never happen - print a very visible warning if it does
    consoleOutSlot("WARNING - Chk_UID string from the CheckPointData didn't start with 'check*'");
  }

  // perform the patching
  int find__init_cond__ = inputFileText.find("init_cond");
  if (find__init_cond__ >= 0){
    int nextLine = inputFileText.find("\n",find__init_cond__);
    if (nextLine > find__init_cond__){
      inputFileText = inputFileText.left(find__init_cond__) +
	"init_cond = 7\n"+ inputFileText.right(inputFileText.length() 
					       - nextLine - 1);
    }
  }

  int find__restore_string__ = inputFileText.find("restore_string");
  if (find__restore_string__ >= 0){
    int nextLine = inputFileText.find("\n", find__restore_string__);
    if (nextLine > find__restore_string__){
      inputFileText = inputFileText.left(find__restore_string__) +
	"restore_string = '"+chkUIDString+"'\n"+ 
	inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }
}

//-----------------------------------------------------------------
/** Method to patch NAMD input file such that it is suitable for
    performing a restart from the specified checkpoint
 */
void RegLauncher::patchNamdInputFileText(QString &inputFileText,
                                         QString &chkUIDString,
                                         QStringList &fileNames)
{ 
  // Generate the names of the five restart files
  QString coordName = QString("coordinates");
          coordName += "           ";
  QString velName = QString("velocities");
          velName += "           ";
  QString extSysName = QString("extendedSystem");
          extSysName += "           ";
  // structure file
  QString structName = QString("structure");
          structName += "           ";
  // parameters file
  QString paramName = QString("parameters");
          paramName += "           ";
  // fep input file (is a pdb)
  QString fepName = QString("fepFile");
          fepName += "           ";
  bool hasFepFile = FALSE;
          
  QStringList::iterator file = fileNames.begin();
  while(file != fileNames.end()){

    if((*file).endsWith(".coor")){

      // 'section' splits string according to specified separator,
      //  '-1' asks for the right-most portion
      coordName +=  (*file).section('/', -1) + "\n";
      //cout << "ARPDBG coor: " << coordName << endl;
    }
    else if((*file).endsWith(".vel")){
    
      velName += (*file).section('/', -1) + "\n";
      //cout << "ARPDBG vel: " << velName << endl;
    }
    else if((*file).endsWith(".xsc")){

      extSysName += (*file).section('/', -1) + "\n";
      //cout << "ARPDBG xsc: " << extSysName << endl;
    }
    else if((*file).endsWith(".psf")){

      structName += (*file).section('/', -1) + "\n";
      //cout << "ARPDBG struct: " << structName << endl;
    }
    else if((*file).endsWith(".inp")){

      paramName += (*file).section('/', -1) + "\n";
      //cout << "ARPDBG param: " << paramName << endl;
    }
    else if((*file).endsWith(".pdb")){

      fepName += (*file).section('/',-1) + "\n";
      hasFepFile = TRUE;
    }
    
    ++file;
  }
  
  int nextLine = 0;
  int index = inputFileText.find("\ncoordinates");
  if(index > 0){
    // Replace exisiting line
    nextLine = inputFileText.find("\n", ++index);
    if (nextLine > index){
      inputFileText = inputFileText.left(index) + coordName + 
                      inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }
  else{
    // Insert new line
    index = inputFileText.find("\ncoordinates");
    if(index > 0){
      nextLine = inputFileText.find("\n", ++index);
      inputFileText = inputFileText.left(nextLine) + "\n" + 
                      coordName + inputFileText.right(nextLine);
    }
  }

  index = inputFileText.find("\nvelocities");
  if(index > 0){
    // Replace exisiting line
    nextLine = inputFileText.find("\n", ++index);
    if (nextLine > index){
      inputFileText = inputFileText.left(index) + velName + 
                      inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }
  else{
    // Insert new line
    index = inputFileText.find("\ncoordinates");
    if(index > 0){
      nextLine = inputFileText.find("\n", ++index);
      inputFileText = inputFileText.left(nextLine) + "\n" + 
                      velName + inputFileText.right(nextLine);
    }
  }

  index = inputFileText.find("\nextendedSystem");
  if(index > 0){
    // Replace exisiting line
    nextLine = inputFileText.find("\n", ++index);
    if (nextLine > index){
      inputFileText = inputFileText.left(index) + 
                      extSysName + inputFileText.right(inputFileText.length() 
                      - nextLine - 1);
    }
  }
  else{
    // Insert new line
    index = inputFileText.find("\ncoordinates");
    if(index > 0){
      nextLine = inputFileText.find("\n", ++index);
      inputFileText = inputFileText.left(nextLine) + "\n" + 
                      extSysName + inputFileText.right(nextLine);
    }
  }

  // We're specifying velocities so we musn't try and specify a temperature
  index = inputFileText.find("\ntemperature");
  if( index != -1 ){

    inputFileText = inputFileText.left(index+1) + "#" + 
                    inputFileText.right(inputFileText.length() - index - 1);
  }

  // We're specifying the basis vectors in a file so comment out if in input deck
  index = inputFileText.find("\ncellBasisVector1");
  if((index != -1) && !(inputFileText.at(index-1) == '#')){

    inputFileText = inputFileText.left(index+1) + "#" + 
                    inputFileText.right(inputFileText.length() - index - 1);
  }
  index = inputFileText.find("\ncellBasisVector2", ++index);
  if((index != -1) && !(inputFileText.at(index-1) == '#')){

    inputFileText = inputFileText.left(index+1) + "#" + 
                    inputFileText.right(inputFileText.length() - index - 1);
  }
  index = inputFileText.find("\ncellBasisVector3", ++index);
  if((index != -1) && !(inputFileText.at(index-1) == '#')){

    inputFileText = inputFileText.left(index+1) + "#" + 
                    inputFileText.right(inputFileText.length() - index - 1);
  }

  // structure file
  index = inputFileText.find("\nstructure");
  if(index > 0){
    // Replace exisiting line
    nextLine = inputFileText.find("\n", ++index);
    if (nextLine > index){
      inputFileText = inputFileText.left(index) + structName + 
                  inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }
  else{
    // Insert new line
    index = inputFileText.find("\ncoordinates");
    if(index > 0){
      nextLine = inputFileText.find("\n", ++index);
      inputFileText = inputFileText.left(nextLine) + "\n" + structName + 
                      inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }

  // parameters file
  index = inputFileText.find("\nparameters");
  if(index > 0){
    // Replace exisiting line
    nextLine = inputFileText.find("\n", ++index);
    if (nextLine > index){
      inputFileText = inputFileText.left(index) + paramName + 
                    inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }
  else{
    // Insert new line
    index = inputFileText.find("\ncoordinates");
    if(index > 0){
      nextLine = inputFileText.find("\n", ++index);
      inputFileText = inputFileText.left(nextLine) + "\n" + paramName + 
                      inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }

  // fep file
  if(hasFepFile){
    index = inputFileText.find("\nfepFile", 0, FALSE);
    if(index > 0){
      // Replace exisiting line
      nextLine = inputFileText.find("\n", ++index);
      if (nextLine > index){
      	inputFileText = inputFileText.left(index) + fepName + 
                  inputFileText.right(inputFileText.length() - nextLine - 1);
      }
    }
    else{
      // Insert new line
      index = inputFileText.find("\ncoordinates");
      if(index > 0){
      	nextLine = inputFileText.find("\n", ++index);
      	inputFileText = inputFileText.left(nextLine) + "\n" + fepName + 
                 inputFileText.right(inputFileText.length() - nextLine - 1);
      }
    }
  }
  
  cerr << "Modified input file is now: >>" << inputFileText << "<<" << endl;
}

//------------------------------------------------------------------------
/** Slot launches a simulation or visualization component.
 *  User goes through a wizard, filling in data - after
 *  successful completion, launch the actual job.
 */
void RegLauncher::launchSimSlot()
{
  // Check that user has a valid proxy
  this->proxyInfo();
  
  bool restartingFromCheckPoint = checkPointTreeListView->selectedItem()!=NULL;
  config.migration = false;

  // Component launcher will fill in our current config with the user's requests
  ComponentLauncher *componentLauncher = new ComponentLauncher();

  QString inputFileText = "";

  // Find out if the user has selected a checkpoint, and if so
  // consider that we will be starting from there
  if (restartingFromCheckPoint){
    config.restart = true;

    QString tGSH = ((CheckPointTreeItem*)checkPointTreeListView->selectedItem())->getCheckPointGSH();
    config.currentCheckpointGSH = tGSH;

    // and then go get the input file associated with the selected checkpoint,
    inputFileText = getInputFileFromCheckPoint(tGSH);

    if (inputFileText.length() == 0){
    	cerr << "Failed to get job input file from checkpoint " << tGSH << endl;
      return;
    }
    
    QString checkPointDataText = getDataFileFromCheckPoint(tGSH);

    if (checkPointDataText.length() == 0){
    	cerr << "Failed to get checkpoint meta-data from checkpoint " << tGSH << endl;
      return;
    }

    // Get the ID string of the app. that created this checkpoint
    QString appName;
    QString checkUIDString;
    QStringList fileNames;
    parseChkPtMetaData(checkPointDataText, appName, checkUIDString, fileNames);

    if(appName.length()==0 ){

      consoleOutSlot("Failed to get name of application from checkpoint metadata");
      return;
    }
    else if(checkUIDString.length()==0){

    	consoleOutSlot("Failed to get UID of checkpoint from metadata");
     	return;
    }

    // Work out which of the applications is being restarted
    config.mAppToLaunch = NULL;
    for(unsigned int i=0; i<config.applicationList.count(); i++){
      // IMPORTANT - this will require more care if we want to match up
      // version numbers rather than crudely checking the app name
      if(appName.contains(config.applicationList[i].mAppName, FALSE) == 1){
        config.mAppToLaunch = &(config.applicationList[i]);
        break;
      }
    }
    if(!config.mAppToLaunch){

    	consoleOutSlot("Failed to match name of application used to create checkpoint"
                     "with those listed in default.conf");
     	return;
    }
    
    if(appName.contains("lbe3d", FALSE) == 1){
      patchLb3dInputFileText(inputFileText, checkUIDString);
    }
    else if(appName.contains("namd", FALSE) == 1){
      patchNamdInputFileText(inputFileText, checkUIDString, fileNames);
    }

    // cache it
    // we need to copy the selected/edited input file to the target machine
    QString inputFileName = config.mScratchDirectory+"/ReG_tmp_input_file";

    config.mInputFileName = inputFileName;

    // and insert it into the wizard's text edit box
    componentLauncher->setInputFileTextEdit(inputFileText);

    config.newTree = false;
  } // : if restarting from checkpoint
  else {
    // else we're launching a new tree (if this type of app is restartable)
    config.restart = false;
    config.newTree = true;
  }

  componentLauncher->setConfig(&config);
  
  // call exec rather than show since the dialog should be modal, program
  // will only continue when the user's finished with the dialog box
  bool wizardOk = componentLauncher->exec();

  if (!wizardOk)
    return;

  // If we're restarting a job and have had to edit the input file...
  if (config.mAppToLaunch->mHasInputFile && restartingFromCheckPoint){
  
    componentLauncher->getInputFileTextEditText(&inputFileText);
    
    QFile inputFile( config.mInputFileName );
    if ( inputFile.open( IO_WriteOnly ) ) {
      QTextStream stream( &inputFile );
      stream << inputFileText;
      inputFile.close();
    }
  }
  
  if(config.mIsCoupledModel){
    config2.restart = false;
    config2.newTree = true;
    config2.mIsCoupledModel = true;
    config2.mTimeToRun = config.mTimeToRun;
    // Turn off again for future launches
    config.mIsCoupledModel = false;

    ComponentLauncher *componentLauncher2 = new ComponentLauncher();
    componentLauncher2->setConfig(&config2);
    wizardOk = componentLauncher2->exec();

    if (!wizardOk) return;

    coupledModelLaunchCode();
  }
  else{
    // Launch single component
    commonLaunchCode();
  }
}

//--------------------------------------------------------------------
void RegLauncher::commonLaunchCode(){

  QString sgs;
  bool restartingFromCheckpoint = checkPointTreeListView->selectedItem()!=NULL;
  
  if (config.migration)
    restartingFromCheckpoint = true;

  consoleOutSlot("Starting a component...");

#ifndef REG_WSRF
  // First find a factory, and if we don't have one - make one
  QString factory = gridifier.getSGSFactories(config.topLevelRegistryGSH, 
					      config.selectedContainer,
					      QString("sgs"));

  if (factory.length() == 0){
    consoleOutSlot("No factories to be had - I'd better make one");
    QString posFactory = gridifier.makeSGSFactory(config.selectedContainer,
						  config.topLevelRegistryGSH,
						  QString("sgs"));
      
    if (posFactory.startsWith(config.selectedContainer))
      factory = posFactory;
    else{
      consoleOutSlot("Sorry! - couldn't start a factory");
      consoleOutSlot("makeSGSFactory returned: "+posFactory);
      return;
    }     
  }

  consoleOutSlot(QString("SGS Factory is "+factory).stripWhiteSpace());
#else
  QString factory = config.selectedContainer;
  cout << "Using container: "<< factory << endl;
#endif // ndef REG_WSRF

  // Now determine whether we need to configure the SGS with data source(s)
  if (config.mAppToLaunch->mNumInputs == 0){
  
    // No data sources....

    // Create an SGS GSH, and create a checkpoint tree if necessary
    if (config.newTree && !(config.treeTag.isEmpty()) ){
      consoleOutSlot(QString("Making new checkpoint tree with tag "+
			     config.treeTag));
      sgs = gridifier.makeSteeringService(factory, config);
    }
    else{
      consoleOutSlot(QString("NOT making new checkpoint tree"));
      config.treeTag = "";
      sgs = gridifier.makeSteeringService(factory, config);
    }

    // Check that the sgs was created properly, if not die
    if (sgs.length()==0 || !sgs.startsWith("http")){
      consoleOutSlot("Failed to create a steering service - is "
		     "the factory valid?");
      return;
    }
      
    // Copy the value to the config
    config.simulationGSH.mEPR = sgs;
    strncpy(config.simulationGSH.mSecurity.userDN,
	    config.mJobData->mPersonLaunching.ascii(), REG_MAX_STRING_LENGTH);
    strncpy(config.simulationGSH.mSecurity.passphrase, 
	    config.mServicePassword.ascii(), REG_MAX_STRING_LENGTH);
      
    consoleOutSlot(QString("SGS is "+
			   config.simulationGSH.mEPR).stripWhiteSpace());

    // Create xml job description and save to file
    //QFile jobFile(config.mScratchDirectory + "/job.xml");
    //jobFile.open( IO_WriteOnly );

    //QTextStream filestream(&jobFile);
    //filestream << config.toXML();
    //jobFile.close();

    // Launch job using remote web service and the xml job description
    //consoleOutSlot("Submitting job to web service...");
    //gridifier.webServiceJobSubmit(config.mScratchDirectory + "/job.xml");
    //consoleOutSlot("...done job submission");
    //return;//ARPDBG
    
    // Now launch the job
    gridifier.makeReGScriptConfig(config.mScratchDirectory+"/sim.conf", 
				  config);

    // Check to see if we're starting from a checkpoint or not..
    if (restartingFromCheckpoint){
      // and copy the checkpoint files too - this could take a looong time
      // then start the job
      consoleOutSlot("About to copy checkpoint files to target machine. "
		     "This may take some time....");

      ProgressBarThread *test = new ProgressBarThread();
      test->start();
      if(gridifier.launchSimScript(config.mScratchDirectory+"/sim.conf",
				   config) != REG_SUCCESS){
	consoleOutSlot("Error with starting job - see stdout/err for details");
	gridifier.cleanUp(&config);
	return;
      }
      test->kill();
      consoleOutSlot("Done with copying checkpoint files. Job "
		     "should be queued.");
    }
    else {
      if(gridifier.launchSimScript(config.mScratchDirectory+"/sim.conf", 
				   config) != REG_SUCCESS){
	consoleOutSlot("Error with starting job - see stdout/err for details");
	gridifier.cleanUp(&config);
	return;
      }
    }
    
    JobStatusThread *aJobStatusThread = 
                      new JobStatusThread(mApplication, this,
					  &(config.simulationGSH),
					  config.mScriptsDirectory);
    aJobStatusThread->start();
  }
  else{
    // We need to set-up data sources
    sgs = gridifier.makeVizSGS(factory, config);
    cout << "Common launch code, sgs = >>" << sgs << "<<" << endl;
    // Check that the sgs was created properly, if not die
    if (sgs.length()==0 || !sgs.startsWith("http")){
      consoleOutSlot("Failed to create a visualization SGS - is the "
		     "simulation SGS valid and running?");
      consoleOutSlot("Output was:");
      consoleOutSlot(sgs);
      return;
    }

    // Copy the value to the config
    config.visualizationGSH.mEPR = sgs;
    strncpy(config.visualizationGSH.mSecurity.userDN,
	    config.mJobData->mPersonLaunching.ascii(), REG_MAX_STRING_LENGTH);
    strncpy(config.visualizationGSH.mSecurity.passphrase, 
	    config.mServicePassword.ascii(), REG_MAX_STRING_LENGTH);

    consoleOutSlot(QString("Viz SGS is "+
			   config.visualizationGSH.mEPR).stripWhiteSpace());

    // At this point we want to specialise for the Argonne Cluster.
    // Check to see where we are rendering and act accordingly
    if (config.mTargetMachine->mName == "tg-master.uc.teragrid.org"){
      gridifier.launchArgonneViz(config);
    }
    else {
      gridifier.makeReGScriptConfig(config.mScratchDirectory+"/viz.conf", 
				    config);
      if(gridifier.launchVizScript(config.mScratchDirectory+"/viz.conf", 
				   config) != REG_SUCCESS){
	consoleOutSlot("Error with starting job - see stdout/err for details");
	gridifier.cleanUp(&config);
	return;
      }
    }

    JobStatusThread *aJobStatusThread = new JobStatusThread(mApplication, this,
                                                            &(config.visualizationGSH),
							    config.mScriptsDirectory);
    aJobStatusThread->start();
  }
}

//--------------------------------------------------------------------
/** Launch a two-component coupled model */
void RegLauncher::coupledModelLaunchCode(){

  QString parentMetaSGS_GSH, firstChildMetaSGS_GSH, secondChildMetaSGS_GSH;
  QString parentSWS_EPR, firstChildSWS_EPR, secondChildSWS_EPR;
  LauncherConfig tmpConfig;

  consoleOutSlot("Starting two components...");

#ifndef REG_WSRF
  // First find a factory, and if we don't have one - make one
  QString factory = gridifier.getSGSFactories(config.topLevelRegistryGSH, 
					      config.selectedContainer,
					      QString("msgs"));
  if (factory.length() == 0){
    consoleOutSlot("No factories to be had - I'd better make one");
    QString posFactory = gridifier.makeSGSFactory(config.selectedContainer,
			         config.topLevelRegistryGSH,
			         QString("msgs"));
      
    if (posFactory.startsWith(config.selectedContainer))
      factory = posFactory;
        
    else{
      consoleOutSlot("Sorry! - couldn't start a factory");
      return;
    }     
  }

  // Second factory for second MetaSGS
  QString factory2 = gridifier.getSGSFactories(config2.topLevelRegistryGSH, 
					       config2.selectedContainer,
					       QString("msgs"));
  if (factory2.length() == 0){
    consoleOutSlot("No factories to be had - I'd better make one");
    QString posFactory = gridifier.makeSGSFactory(config2.selectedContainer,
						  config2.topLevelRegistryGSH,
						  QString("msgs"));
      
    if (posFactory.startsWith(config2.selectedContainer))
      factory2 = posFactory;
    else{
      consoleOutSlot("Sorry! - couldn't start a factory");
      return;
    }     
  }

  consoleOutSlot(QString("MetaSGS Factories are "+factory+"\n and "+
			 factory2).stripWhiteSpace());
#else // REG_WSRF defined...
  QString factory = config.selectedContainer;
  cout << "Using container 1: "<< factory << endl;
  QString factory2 = config2.selectedContainer;
  cout << "Using container 2: "<< factory << endl;
#endif // ndef REG_WSRF


  // Now create the parent SWS

  ComponentLauncher *componentLauncher = new ComponentLauncher();
  componentLauncher->toggleCollectJobMetaDataOnly(true);
  tmpConfig.mAppToLaunch = config.mAppToLaunch;
  tmpConfig.migration = false;
  tmpConfig.restart = false;
  tmpConfig.newTree = false;
  tmpConfig.topLevelRegistryGSH = config.topLevelRegistryGSH;
  componentLauncher->setConfig(&tmpConfig);
  componentLauncher->showPage(componentLauncher->page(7));
  componentLauncher->exec();

  tmpConfig.treeTag = "";
  tmpConfig.currentCheckpointGSH = config.currentCheckpointGSH;
  tmpConfig.mInputFileName = "";
  tmpConfig.mTimeToRun = config.mTimeToRun;

#ifdef REG_WSRF
  parentSWS_EPR = gridifier.makeSteeringService(factory, tmpConfig);
  // Check that the sws was created properly, if not die
  if (parentSWS_EPR.length()==0 || 
      !parentSWS_EPR.startsWith("http")){
    consoleOutSlot("Failed to create parent SWS - is the container ("+
		   factory+") valid?");
    return;
  }
  else{
    consoleOutSlot("Parent SWS EPR = "+parentSWS_EPR);
  }
#else
  parentMetaSGS_GSH = gridifier.makeMetaSGS(factory, tmpConfig, "");
  // Check that the sgs was created properly, if not die
  if (parentMetaSGS_GSH.length()==0 || 
      !parentMetaSGS_GSH.startsWith("http")){
    consoleOutSlot("Failed to create parent MetaSGS - is the factory ("+
		   factory+") valid?");
    return;
  }
#endif // def REG_WSRF

  // Create the first child
#ifndef REG_WSRF
  firstChildMetaSGS_GSH = gridifier.makeMetaSGS(factory, config, 
						parentMetaSGS_GSH);
  // Check that the sgs was created properly, if not die
  if (firstChildMetaSGS_GSH.length()==0 || 
      !firstChildMetaSGS_GSH.startsWith("http")){
    consoleOutSlot("Failed to create parent MetaSGS - is the factory ("+
		   factory+") valid?");
    gridifier.cleanUp(&SteeringService(parentMetaSGS_GSH,
				      "",
				      ""));
    return;
  }
  // Copy the value to the config
  config.simulationGSH.mEPR = firstChildMetaSGS_GSH;
  consoleOutSlot(QString("1st MetaSGS is "+
			 config.simulationGSH.mEPR).stripWhiteSpace());
#else
  firstChildSWS_EPR = gridifier.makeSteeringService(factory, config,
						    parentSWS_EPR);
  // Check that the sws was created properly, if not die
  if (firstChildSWS_EPR.length()==0 || 
      !firstChildSWS_EPR.startsWith("http")){
    consoleOutSlot("Failed to create first child SWS - is the factory ("+
		   factory+") valid?");
    gridifier.cleanUp(&SteeringService(parentSWS_EPR,
				       "", ""));
    return;
  }
  // Copy the value to the config
  config.simulationGSH.mEPR = firstChildSWS_EPR;
  consoleOutSlot(QString("1st child SWS is "+
			 config.simulationGSH.mEPR).stripWhiteSpace());
#endif // ndef REG_WSRF

  // Create the second child
#ifndef REG_WSRF
  secondChildMetaSGS_GSH = gridifier.makeMetaSGS(factory2, config2, 
						 parentMetaSGS_GSH);
  // Check that the sgs was created properly, if not die
  if (secondChildMetaSGS_GSH.length()==0 || 
      !secondChildMetaSGS_GSH.startsWith("http")){
    consoleOutSlot("Failed to create 2nd child MetaSGS - is the factory ("+
		   factory2+") valid?");
    gridifier.cleanUp(&config);
    gridifier.cleanUp(&SteeringService(parentMetaSGS_GSH,
				      "",""));
    return;
  }
  // Copy the value to the config
  config2.simulationGSH.mEPR = secondChildMetaSGS_GSH;
  consoleOutSlot(QString("2nd MetaSGS is "+
			 config2.simulationGSH.mEPR).stripWhiteSpace());
#else
  secondChildSWS_EPR = gridifier.makeSteeringService(factory2, config2, 
						     parentSWS_EPR);
  // Check that the sws was created properly, if not die
  if (secondChildSWS_EPR.length()==0 || 
      !secondChildSWS_EPR.startsWith("http")){
    consoleOutSlot("Failed to create second child SWS - is the factory ("+
		   factory2+") valid?");
    gridifier.cleanUp(&config);
    gridifier.cleanUp(&SteeringService(parentSWS_EPR,
				      "", ""));
    return;
  }
  // Copy the value to the config
  config2.simulationGSH.mEPR = secondChildSWS_EPR;
  consoleOutSlot(QString("2nd child SWS is "+
			 config2.simulationGSH.mEPR).stripWhiteSpace());
#endif // ndef REG_WSRF

  // Now launch the jobs themselves
  gridifier.makeReGScriptConfig(config.mScratchDirectory+"/sim.conf", config);
  if(gridifier.launchSimScript(config.mScratchDirectory+"/sim.conf", 
			       config) != REG_SUCCESS){
    gridifier.cleanUp(&config);
    gridifier.cleanUp(&config2);
#ifdef REG_WSRF
    gridifier.cleanUp(&SteeringService(parentSWS_EPR,
				      "",""));
#else
    gridifier.cleanUp(&SteeringService(parentMetaSGS_GSH,
				      "", ""));
#endif
    consoleOutSlot("Failed to launch first component - cancelling");
  }
  gridifier.makeReGScriptConfig(config.mScratchDirectory+"/sim.conf", config2);
  if(gridifier.launchSimScript(config.mScratchDirectory+"/sim.conf", 
			       config2) != REG_SUCCESS){
    gridifier.cleanUp(&config);
    gridifier.cleanUp(&config2);
#ifdef REG_WSRF
    gridifier.cleanUp(&SteeringService(parentSWS_EPR,
				      "", ""));
#else
    gridifier.cleanUp(&SteeringService(parentMetaSGS_GSH,
				      "", ""));
#endif
    // ARPDBG - somehow need to withdraw the first component that is 
    // now running on its target machine...
    consoleOutSlot("Failed to launch second component - cancelling");
  }

  // Fix so that launcher correctly points steerer at parent rather than
  // a child if the user presses the 'steer' button
#ifdef REG_WSRF
  config.simulationGSH.mEPR = parentSWS_EPR;
#else
  config.simulationGSH.mEPR = parentMetaSGS_GSH;
#endif

#ifdef REG_WSRF
  JobStatusThread *aJobStatusThread = new JobStatusThread(mApplication, this,
							  &SteeringService(parentSWS_EPR,
							  "", ""),
							  config.mScriptsDirectory);
#else
  JobStatusThread *aJobStatusThread = new JobStatusThread(mApplication, this,
							  &SteeringService(parentMetaSGS_GSH,
							  "",""),
							  config.mScriptsDirectory);
#endif // defined REG_WSRF

  aJobStatusThread->start();
}

/** Searches for checkpoint trees using the root address specified in
    the default.conf configuration file */
void RegLauncher::discoverySlot()
{
  launchButton->setText("Launch");
  
  checkPointTreeListView->clear();
  checkPointTreeListView->clearSelection();
  consoleOutSlot("Searching for CheckPoint Trees");

#ifdef REG_WSRF
  if(config.checkPointTreeEPR.isEmpty()){
    cpt = new CheckPointTree(checkPointTreeListView, 
			     config.checkPointTreeFactoryGSH);
  }
  else{
    cpt = new CheckPointTree(checkPointTreeListView, 
			     config.checkPointTreeEPR);
  }

#else  
  cpt = new CheckPointTree(checkPointTreeListView, 
			   config.checkPointTreeFactoryGSH);
#endif // defined REG_WSRF
}

//----------------------------------------------------------------
QProcess *steerer;
void RegLauncher::steerSlot()
{
  ////////////////// TEST /////////////////////

#ifdef speedTest
  gridifier.gsiFtp("file:///tmp/aBigFile", 
		   "gsiftp://bezier.man.ac.uk/tmp/aGSIFTPSpeedTest");
  if (1) return;
#endif  

  if(config.mSteererBinaryLocation.isEmpty()){

    QMessageBox::warning(NULL, "Configuration error",
			 "Steerer location is not set - default.conf should have\n"
			 "a steerClientBinary entry\n\n",
			 QMessageBox::Ok, 0, 0 );
    return;
  }

  // create an instance of the RealityGrid QT Steerer for the current GSH
  steerer = new QProcess(config.mSteererBinaryLocation);
  if (config.simulationGSH.mEPR.length() != 0){    
    steerer->addArgument(config.simulationGSH.mEPR);
    steerer->setCommunication(QProcess::Stdout|QProcess::Stderr|QProcess::DupStderr);

    steerer->start();
  }
  else {
    steerer->start();
  }
}

//----------------------------------------------------------------
void RegLauncher::readSteerStdoutSlot(){
  // dump this in a file so we don't make the console useless...
  cout << "SLOT: " << QString(steerer->readStdout()) << endl;
}

/** Slot is called when the Proxy Init menu item is selected.
   *  Method creates a QProcess to call the grid-proxy-init cmd.
   */
void RegLauncher::proxyInit()
{
  // First - pop up a pass phrase entry dialog (non threaded - GUI blocking)
  bool ok = FALSE; 

  QString passPhrase = QInputDialog::getText( tr("Enter GRID Pass Phrase"), 
					      tr("Enter Your Pass Phrase"), 
					      QLineEdit::Password, QString::null, 
					      &ok, this);

  // Check to see if the user entered anything and didn't click cancel - return if so
  if (!ok || passPhrase.isEmpty()){
    printf("Proxy not intialised\n");
    return;
  }

  // Launch the grid-proxy-init process, and pass the user pass phrase in on stdin
  QProcess *proxyInit = new QProcess(this);

  proxyInit->addArgument("grid-proxy-init");
  proxyInit->addArgument("-debug");
  proxyInit->addArgument("-pwstdin");

  proxyInit->launch(passPhrase, NULL);

  // Finally perform a check on the newly formed proxy
  connect(proxyInit, SIGNAL(processExited()), this, SLOT(proxyInfo()));
}

/** Slot is called when the Proxy Info menu item is selected.
   *  Method creates a QProcess to call the grid-proxy-info cmd.
   *  Process results are retrieved in the proxyInfoProcessEnded slot.
   */
void RegLauncher::proxyInfo()
{
  proxyStatus = new QProcess(this);
  proxyStatus->addArgument("grid-proxy-info");
  proxyStatus->addArgument("-exists");
  proxyStatus->start();
  
  connect(proxyStatus, SIGNAL(processExited()), this, SLOT(proxyInfoProcessEnded()));

}

/** Slot is called when the Proxy Destroy menu item is selected.
  *  Method creates a QProcesss to call the grid-proxy-destroy cmd.
  *  Process results are not currently retrieved and checked.
  */
void RegLauncher::proxyDestroy()
{
  QProcess *gridProxyDestroy = new QProcess(this);
  gridProxyDestroy->addArgument("grid-proxy-destroy");
  gridProxyDestroy->start();
}

/** Slot is called when the proxy info process terminates.
  *  Process status inidcates success or otherwise of the grid-proxy-info cmd.
  */
void RegLauncher::proxyInfoProcessEnded()
{
  if (proxyStatus == NULL)
      return;

  // Only bother checking if we're actually using globus for
  // launching
  QString launchMethod = QString(getenv("ReG_LAUNCH"));
  if(launchMethod.contains("ssh") > 0)return;

  if (0 == proxyStatus->exitStatus()){
    consoleOutSlot("Proxy Initialised");
  }
  else{
    consoleOutSlot("Proxy error or proxy does not exist");
    
    QMessageBox::warning( NULL, "Configuration error",
                    "Proxy error or proxy does not exist - is GLOBUS_LOCATION\n"
                    "set and do you have a valid proxy?\n\n",
                    QMessageBox::Ok, 0, 0 );
  }
}

//----------------------------------------------------------------
void RegLauncher::consoleOutSlot(const QString &text)
{
  textOutTextEdit->insertParagraph(text, -1);
}

//----------------------------------------------------------------
void RegLauncher::checkPointListViewExpanded(QListViewItem *item)
{
  // For each item that's been expanded - call it's getChildData method.
  // This in turn calls the CheckPointTree thread, which retrieves the
  // actual data without blocking.
  ((CheckPointTreeItem*)item)->getChildData();
}


void RegLauncher::checkPointListViewClickedSlot( QListViewItem *selectedItem )
{
  if (selectedItem == NULL){
    checkPointTreeListViewPreviousSelection = -1;

    checkPointTreeListView->clearSelection();

    // Be nice and tell the user what's going to happen if they click the launch button
    launchButton->setText("Launch");
    config.currentCheckpointGSH = "";
   
    return;
  }

  // if the current selection is already highlighted - deselect it
  if (checkPointTreeListView->itemPos(selectedItem) == checkPointTreeListViewPreviousSelection){
    // toggle the status
    if (launchButton->text() == "Launch"){
      launchButton->setText("Restart");
      config.currentCheckpointGSH = ((CheckPointTreeItem*)selectedItem)->getCheckPointGSH();
    }
    else {
      checkPointTreeListView->clearSelection();

      // Be nice and tell the user what's going to happen if they click the launch button
      launchButton->setText("Launch");
      config.currentCheckpointGSH = "";
    }
  }
  else {
    checkPointTreeListViewPreviousSelection = checkPointTreeListView->itemPos(selectedItem);

    // Be nice and tell the user what's going to happen if they click the launch button
    launchButton->setText("Restart");
    config.currentCheckpointGSH = ((CheckPointTreeItem*)selectedItem)->getCheckPointGSH();
  }
  
}

//----------------------------------------------------------------
void RegLauncher::contextMenuRequestedSlot( QListViewItem *listViewItem,
                                            const QPoint &pnt, int column)
{
  // First of all check to see if the mouse was actually over a item
  if (listViewItem == NULL)
    return;

  // Cast the listViewItem to a CheckPointTreeItem
  rightMouseCheckPointTreeItem = (CheckPointTreeItem*)listViewItem;
    
  QPopupMenu popupMenu;
  popupMenu.insertItem(QString("View Parameters"), 0);
  popupMenu.insertItem(QString("View GSH"), 1);
  popupMenu.insertItem(QString("View Input File"), 2);
  popupMenu.insertItem(QString("View CheckPoint Data"), 3);
  if(rightMouseCheckPointTreeItem->isRootNode()){
    popupMenu.insertItem(QString("Delete tree"), 4);
  }
  else{
    popupMenu.insertItem(QString("Delete node"), 4);
  }
  connect(&popupMenu, SIGNAL(activated(int)), this, 
	  SLOT(contextMenuItemSelectedSlot(int)));
  popupMenu.exec(pnt);
}

//----------------------------------------------------------------
void RegLauncher::contextMenuItemSelectedSlot(int itemId)
{

  // See if we've selected to 'View Parameters'
  if (itemId == 0){
    // Since we're reusing the class from the steerer
    Output_log_struct tmp;

    // Get a copy of the parameters that we want
    if (rightMouseCheckPointTreeItem != NULL){
      CheckPointParamsList cpParamList = 
	rightMouseCheckPointTreeItem->getParamsList();

      tmp.num_param = (int)cpParamList.size();
      for (unsigned int i=0; i<cpParamList.size(); i++){
	printf("%d: %s = %s\n", i, cpParamList[i].mLabel.ascii(),
	       cpParamList[i].mValue.ascii());
        strcpy(tmp.param_labels[i], cpParamList[i].mLabel);
        strcpy(tmp.param_values[i], cpParamList[i].mValue);
      }
    }

    ChkPtVariableForm *aChkPtVariableForm = new ChkPtVariableForm(&tmp, 
								  this, 
								  "testDialog");
    aChkPtVariableForm->show();
  }

  // or if we've selected to 'View GSH'
  else if (itemId == 1){

    if (rightMouseCheckPointTreeItem != NULL)
      QInputDialog::getText("GSH", "Checkpoint GSH", 
			    QLineEdit::Normal, 
			    rightMouseCheckPointTreeItem->getCheckPointGSH(), 
			    NULL, this);
  }
  // or if we've selected to 'View Input File'
  else if (itemId == 2){

    if (rightMouseCheckPointTreeItem != NULL){
      // TODO - subclass QDialog to get a better pop-up window for this
      QString inputFileText = getInputFileFromCheckPoint(rightMouseCheckPointTreeItem->getCheckPointGSH());

      TextViewDialog *textViewDialog = new TextViewDialog();
      textViewDialog->mTextEdit->setText(inputFileText);
      textViewDialog->mTextEdit->setReadOnly(TRUE);
      textViewDialog->show();
    }
  }
  // or if we've selected to 'View CheckPoint Data'
  else if (itemId == 3){
    if (rightMouseCheckPointTreeItem != NULL){
      // TODO - subclass QDialog to get a better pop-up window for this
      QString dataFileText = getDataFileFromCheckPoint(rightMouseCheckPointTreeItem->getCheckPointGSH());

      TextViewDialog *textViewDialog = new TextViewDialog();
      textViewDialog->mTextEdit->setText(dataFileText);
      textViewDialog->mTextEdit->setReadOnly(TRUE);
      textViewDialog->show();
    }
  }
  // or if we've chosen to delete the node
  else if(itemId == 4){
    rightMouseCheckPointTreeItem->destroy();

    // attempt to refresh the display
    if( !(rightMouseCheckPointTreeItem->isRootNode()) ){
      CheckPointTreeItem *aParent = rightMouseCheckPointTreeItem->getParent();
      if (aParent) aParent->getChildData();
    }
  }
}

/** Parse the checkpoint meta-data obtained from a node in the checkpoint
   tree in order to get the name of the application and the UID of
   the checkpoint
 */
void RegLauncher::parseChkPtMetaData( const QString &chkMetaData,
                                      QString &appName, QString &chkUID,
                                      QStringList &fileNames )
{
  unsigned int i;
  
  QDomDocument doc("Checkpoint meta-data");
  doc.setContent(chkMetaData);

  QDomNodeList nodes = doc.elementsByTagName(QString("Checkpoint_data"));
  if(nodes.count() >= 1){

    if(nodes.item(0).isElement()){
      appName = nodes.item(0).toElement().attribute("application");

      // Extract the UID of the checkpoint from the metadata
      QDomNodeList childNodes =
           nodes.item(0).toElement().elementsByTagName(QString("Chk_UID"));

      if(childNodes.count() > 0){

        if(childNodes.item(0).isElement()){

          chkUID = childNodes.item(0).toElement().text();
        }
      }

      childNodes = nodes.item(0).toElement().elementsByTagName(QString("Files"));

      QDomNode fileNode;
      for(i=0; i<childNodes.count(); i++){

      	if(childNodes.item(i).isElement()){
          childNodes = childNodes.item(i).toElement().elementsByTagName(QString("file"));
     
          // Extract all of the filenames that are listed
          for(i=0; i<childNodes.count(); i++){

          	if(childNodes.item(i).isElement()){
		  fileNames.append(childNodes.item(i).toElement().text());
		}
          }
          break;
        }
      }
    }
  }
}

//----------------------------------------------------------------
void RegLauncher::customEvent( QCustomEvent *e )
{
  if(e->type() != (QEvent::User+1))return;
  
  StatusMessageData *msg = (StatusMessageData *)(e->data());
  if(msg->mTimeout == 0){
    statusBar()->message(msg->msgTxt);
  } else {
    statusBar()->message(msg->msgTxt, msg->mTimeout);
  }

  // Clean up - I think Qt deletes the event object itself but it can't
  // know about the associated data so we delete that.
  delete msg;
}

//----------------------------------------------------------------
void RegLauncher::coupledModelCreateGlobalParamSlot()
{
  QString selectedGSH;
  RunningJobsDialog rjd;

  rjd.setConfig(&config);
  // Read the note at the RunningJobsDialog::setResultString implementation
  rjd.setResultString(&selectedGSH);
  if (rjd.exec() == QDialog::Rejected){
    return;
  }
  cout << "result string = " << selectedGSH << endl;
  GlobalParamConstructionForm globalDlg;
  globalDlg.setConfig(&config);
  globalDlg.setGridifier(&gridifier);
  globalDlg.setParentGSH(selectedGSH);
  globalDlg.exec();
}

