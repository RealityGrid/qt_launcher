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

#include "componentlauncher.h"
#include "RunningJobsDialog.h"
#include "textviewdialog.h"

#include "CheckPointTreeItem.h"
#include "Gridifier.h"
#include "LauncherConfig.h"
#include "JobStatusThread.h"
#include "ProgressBarThread.h"

// Reuse this from reg_qt_steerer
#include "chkptvariableform.h"

// gSoap
#include "checkPointTreeH.h"


using namespace std;


QProcess *proxyStatus = NULL;
LauncherConfig config;
CheckPointTree *cpt = NULL;
CheckPointTreeItem *rightMouseCheckPointTreeItem = NULL;
int checkPointTreeListViewPreviousSelection = -1;

/** Reads the launcher configuration file "default.conf"
  * and tweaks the way the checkpoint tree is displayed
  */
void RegLauncher::init(){
  config.readConfig("default.conf");
  checkPointTreeListView->setRootIsDecorated(true);

  //statusBar()->hide();
  //statusBar()->setSizeGripEnabled(false);
}

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

void RegLauncher::filePrint()
{

}

void RegLauncher::fileExit()
{
    close();
}

void RegLauncher::helpIndex()
{

}

void RegLauncher::helpContents()
{

}

void RegLauncher::helpAbout()
{

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

    // cache the checkpoint data on disk - we'll use it later
/*    QFile checkPointDataFile(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/checkPointDataCache.xml");
    if ( checkPointDataFile.open(IO_WriteOnly) ){
      QTextStream stream(&checkPointDataFile);
      stream << checkPointDataText;
      checkPointDataFile.close();
    }
*/
    // patch it
    patchInputFileText(inputFileText, checkPointDataText);

    // cache it
    // we need to copy the selected/edited input file to the target machine
    QString inputFileName = QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/ReG_tmp_input_file";

    config.lb3dInputFileName = inputFileName;

    // and so we can launch the migration version of the component
    // launcher in order to find out where we're going to move the
    // job to

    ComponentLauncher *migrater = new ComponentLauncher();
    config.migration = true;
    config.restart = false;
    config.newTree = false;
    migrater->setConfig(&config);
    migrater->setApplication(this);
    
    // and insert it into the wizard's text edit box
    migrater->setInputFileTextEdit(inputFileText);

    // Run the wizard - it will automatically populate the active
    // jobs list
    bool wizardOk = migrater->exec();

    if (!wizardOk)
      return;

cout << "About to get the input file text from gsoap" << endl;

    migrater->getInputFileTextEditText(&inputFileText);

// lockup debug
cout << "Done that" << endl << "About to write the input file to " << inputFileText << endl;

    QFile inputFile( inputFileName );
    if ( inputFile.open( IO_WriteOnly ) ) {
      QTextStream stream( &inputFile );
      stream << inputFileText;
      inputFile.close();
    }

// lockup debug
cout << "Done that - now going over to the common launch code" << endl;
    
    // now launch!
    commonLaunchCode();    
  }
  
}

// Consider sticking these two methods in a seperate soap class
QString RegLauncher::getInputFileFromCheckPoint(const QString &checkPointGSH)
{
  QString result;
  
  struct soap soap;
  soap_init(&soap);
  
  tree__getInputFileResponse *inputFileResponse = new tree__getInputFileResponse();
  if (soap_call_tree__getInputFile(&soap, checkPointGSH, "", inputFileResponse))
    soap_print_fault(&soap, stderr);
  else{
    //if (result == NULL)
    //  result = new QString();
    result = inputFileResponse->_getInputFileReturn;
  }

  return result;
}

QString RegLauncher::getDataFileFromCheckPoint(const QString &checkPointGSH)
{
  QString result;
  
  struct soap soap;
  soap_init(&soap);

  tree__getCheckPointDataResponse *chkptDataResponse = new tree__getCheckPointDataResponse();
  if (soap_call_tree__getCheckPointData(&soap, checkPointGSH, "", chkptDataResponse))
    soap_print_fault(&soap, stderr);
  else{
    //if (result == NULL)
    //  result = new QString();
    result = chkptDataResponse->_getCheckPointDataReturn;
  }

  return result;
}



void RegLauncher::patchInputFileText(QString &inputFileText, const QString &checkPointDataText)
{
  QString chkUIDString;

  QDomDocument doc("parsedDoc");
  doc.setContent(checkPointDataText);
  QDomElement docElem = doc.documentElement();

  if (docElem.tagName() == "Checkpoint_data"){
    QDomNode actualNode = docElem.firstChild();
    actualNode = actualNode.nextSibling();
    QDomElement e = actualNode.toElement();
    if (e.tagName() == "Chk_UID"){
      chkUIDString = e.text();
    }

  }

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
      inputFileText = inputFileText.left(find__init_cond__) +"init_cond = 7\n"+ inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }

  int find__restore_string__ = inputFileText.find("restore_string");
  if (find__restore_string__ >= 0){
    int nextLine = inputFileText.find("\n", find__restore_string__);
    if (nextLine > find__restore_string__){
      inputFileText = inputFileText.left(find__restore_string__) +"restore_string = '"+chkUIDString+"'\n"+ inputFileText.right(inputFileText.length() - nextLine - 1);
    }
  }
}


/** Slot launches a simulation or visualization component.
 *  User goes through a wizard, filling in data - after
 *  successful completion, launch the actual job.
 */
void RegLauncher::launchSimSlot()
{
  bool restartingFromCheckPoint = checkPointTreeListView->selectedItem()!=NULL;
  config.migration = false;

  // Component launcher will fill in our current config with the user's requests
  ComponentLauncher *componentLauncher = new ComponentLauncher();
  // componentLauncher->setConfig(&config);
  componentLauncher->setApplication(this);
  QString inputFileText = "";

  // Find out if the user has selected a checkpoint, and if so
  // consider that we will be starting from there
  if (restartingFromCheckPoint){
    config.restart = true;

    QString tGSH = ((CheckPointTreeItem*)checkPointTreeListView->selectedItem())->getCheckPointGSH();
    componentLauncher->setCheckPointGSH(tGSH);

    // and then go get the input file associated with the selected checkpoint,
    inputFileText = getInputFileFromCheckPoint(tGSH);

    if (inputFileText.length() != 0){
      QString checkPointDataText = getDataFileFromCheckPoint(tGSH);

      // cache the checkpoint data on disk - we'll use it later
/*      QFile checkPointDataFile(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/checkPointDataCache.xml");
      if ( checkPointDataFile.open(IO_WriteOnly) ){
        QTextStream stream(&checkPointDataFile);
          stream << checkPointDataText;
        checkPointDataFile.close();
      }
*/
      // patch it
      patchInputFileText(inputFileText, checkPointDataText);
        
      // cache it
      // we need to copy the selected/edited input file to the target machine
      QString inputFileName = QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/ReG_tmp_input_file";

      config.lb3dInputFileName = inputFileName;

      //gridifier.gsiFtp(inputFileName, "gsiftp://"+config.simTargetMachine+"/tmp/ReG_launcher_test");

      // and insert it into the wizard's text edit box
      componentLauncher->setInputFileTextEdit(inputFileText);
       
    }

    config.newTree = false;
  } // : if restarting from checkpoint
  else {
    // else we're launching a new tree
    config.restart = false;
    config.newTree = true;
  }

  componentLauncher->setConfig(&config);
  
  // call exec rather than show since the dialog should be modal, program
  // will only continue when the user's finished with the dialog box
  bool wizardOk = componentLauncher->exec();

  if (!wizardOk)
    return;

  // this is lb3d only... do we really ever want to launch miniapp?
  if (config.selectedComponentType == lb3d && restartingFromCheckPoint){
    componentLauncher->getInputFileTextEditText(&inputFileText);
    QFile inputFile( config.lb3dInputFileName );
    if ( inputFile.open( IO_WriteOnly ) ) {
      QTextStream stream( &inputFile );
      stream << inputFileText;
      inputFile.close();
    }
  }


  // now launch!
  commonLaunchCode();
}

void RegLauncher::commonLaunchCode(){
// lockup debug
cout << "Common Launch Code" << endl;
  bool restartingFromCheckpoint = checkPointTreeListView->selectedItem()!=NULL;
  if (config.migration)
    restartingFromCheckpoint = true;

  // First up determine if we're starting a sim or a viz
  if (config.selectedComponentType == lb3d || config.selectedComponentType == miniapp){
    // It's a sim
    consoleOutSlot("Starting a simulation component");

    // First find a factory, and if we don't have one - make one
    QString factory = gridifier.getSGSFactories(config.topLevelRegistryGSH, config.selectedContainer);

    // need to make sure we can force a particular container to be used
    // this doesn't happen at the moment - we can specifiy a container
    // and the system will choose a different one entirely
    
    if (factory.length() == 0){
      QString posFactory = gridifier.makeSGSFactory("http://"+config.selectedContainer+":"+QString::number(config.containerPortNum)+"/", config.topLevelRegistryGSH);
      
      if (posFactory.startsWith("http://"+config.selectedContainer+":"+QString::number(config.containerPortNum)+"/"))
        factory = posFactory;
        
      else{
        consoleOutSlot("Sorry! - couldn't start a factory");
        return;
      }
        
    }

    consoleOutSlot(QString("SGS Factory is "+factory).stripWhiteSpace());

    QString jobDescriptionTxt = config.mJobData->toXML();

    // Now use the factory to create an SGS
    QString sgs;
    
    // Create an SGS GSH, and create a checkpoint tree if necessary
    if (config.newTree) {
      //sgs = gridifier.makeSimSGS(factory, config.simTag, config.topLevelRegistryGSH, config.currentCheckpointGSH, config.lb3dInputFileName, config.treeTag);
      sgs = gridifier.makeSimSGS(factory, jobDescriptionTxt, config.topLevelRegistryGSH, config.currentCheckpointGSH, config.lb3dInputFileName, config.treeTag);
    }
    else{
      //sgs = gridifier.makeSimSGS(factory, config.simTag, config.topLevelRegistryGSH, config.currentCheckpointGSH, config.lb3dInputFileName, "");
      sgs = gridifier.makeSimSGS(factory, jobDescriptionTxt, config.topLevelRegistryGSH, config.currentCheckpointGSH, config.lb3dInputFileName, "");
    }
      
    // Check that the sgs was created properly, if not die
    if (sgs.length()==0 || !sgs.startsWith("http://")){
      consoleOutSlot("Failed to create a simulation SGS - is the factory valid?");
      return;
    }
      
    // Copy the value to the config
    config.simulationGSH = sgs;

    consoleOutSlot(QString("SGS is "+config.simulationGSH).stripWhiteSpace());

    // Now launch the job

    gridifier.makeReGScriptConfig(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/sim.conf", config);

    // Check to see if we're starting from a checkpoint or not..
    if (restartingFromCheckpoint){
      // and copy the checkpoint files too - this could take a looong time
      // then start the job
      consoleOutSlot("About to copy checkpoint files to target machine. This may take some time....");

      ProgressBarThread *test = new ProgressBarThread();
      test->start();
      gridifier.launchSimScript(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/sim.conf", config.simTimeToRun, config.currentCheckpointGSH);
      //QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/checkPointDataCache.xml");
      test->kill();
      consoleOutSlot("Done with copying checkpoint files. Job should be queued.");
    }
    else {
      gridifier.launchSimScript(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/sim.conf", config.simTimeToRun);
    }

    JobStatusThread *aJobStatusThread = new JobStatusThread(statusBar(), config.simulationGSH);
    aJobStatusThread->start();

  }
  else if (config.selectedComponentType == lb3dviz){
    // It's a viz
    consoleOutSlot("Starting a visualization component");

    // First find a factory, and if we don't have one - make one
    QString factory = gridifier.getSGSFactories(config.topLevelRegistryGSH, config.selectedContainer);

    if (factory.length() == 0){
      consoleOutSlot("There's no factories to be had - I'd better make one");
      QString posFactory = gridifier.makeSGSFactory("http://"+config.selectedContainer+":"+QString::number(config.containerPortNum)+"/", config.topLevelRegistryGSH);

      if (posFactory.startsWith("http://"+config.selectedContainer+":"+QString::number(config.containerPortNum)+"/"))
        factory = posFactory;

      else{
        consoleOutSlot("Sorry! - couldn't start a factory");
        return;
      }
    }

    consoleOutSlot(QString("SGS Factory is "+factory).stripWhiteSpace());

    // Now use the factory to create an SGS
    QString sgs;

    // Remember to fill in the config.simulationGSH during the wizard stage - or we'll have problems!
    // for the time being though - if we make sure we create a simulation before creating a visualization
    // then we'll be ok for testing purposes
    sgs = gridifier.makeVizSGS(factory, config.vizTag, config.topLevelRegistryGSH, config.simulationGSH);

    // Check that the sgs was created properly, if not die
    if (sgs.length()==0 || !sgs.startsWith("http://")){
      consoleOutSlot("Failed to create a visualization SGS - is the simulation SGS valid and running?");
      return;
    }

    // Copy the value to the config
    config.visualizationGSH = sgs;

    consoleOutSlot(QString("Viz SGS is "+config.visualizationGSH).stripWhiteSpace());

    // At this point we want to specialise for the Argonne Cluster.
    // Check to see where we are rendering and act accordingly
    if (config.vizTargetMachine == "tg-master.uc.teragrid.org"){
      gridifier.launchArgonneViz(config);
    }
    else {
      gridifier.makeReGScriptConfig(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/viz.conf", config);

      gridifier.launchVizScript(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/tmp/viz.conf");
    }
    
  }
  

}

void RegLauncher::discoverySlot()
{
  launchButton->setText("Launch");
  
  checkPointTreeListView->clear();
  checkPointTreeListView->clearSelection();
  consoleOutSlot("Searching for CheckPoint Trees");
  
  cpt = new CheckPointTree(checkPointTreeListView);
  //cpt->start();
}

QProcess *steerer;
void RegLauncher::steerSlot()
{
  ////////////////// TEST /////////////////////

#ifdef speedTest
  gridifier.gsiFtp("file:///tmp/aBigFile", "gsiftp://bezier.man.ac.uk/tmp/aGSIFTPSpeedTest");
  if (1)
    return;
#endif  
  // create an instance of the RealityGrid QT Steerer for the current GSH
  // bear in mind that his requires that the Steerer environment variables
  // have already been set
  //QProcess *steerer = new QProcess(QString("/home/zzcgumr/realityGrid/reg_qt_steerer/steerer"));
  steerer = new QProcess(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts/steerer_wrapper"));
  if (config.simulationGSH.length() != 0){    
    steerer->addArgument(config.simulationGSH);
    steerer->setCommunication(QProcess::Stdout|QProcess::Stderr|QProcess::DupStderr);

    cout << getenv("REG_STEER_HOME") << endl;
    cout << steerer->arguments().join(" ") << endl;
    //connect(steerer, SIGNAL(readyReadStdout()), this, SLOT(readSteerStdoutSlot()));
    
    steerer->start();

    // Unfortunately the following code appears not to work.
    // seemingly the QProcess->start environment variable passing
    // technique doesn't work - many posts to the QT forums alluding
    // to this ....
    /*
    QStringList env;
    env += QString("REG_SGS_ADDRESS="+config.simulationGSH);

    cout << env.join(" ");
    cout << steerer->arguments().join(" ");
    
    steerer->start(&env);
    */
  }
  else {
    steerer->start();
  }

//    system(QString("/home/zzcgumr/realityGrid/reg_qt_steerer/steerer "+config.simulationGSH).latin1());
}

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

  QString passPhrase = QInputDialog::getText( tr("Enter GRID Pass Phrase"), tr("Enter Your Pass Phrase"), QLineEdit::Password, QString::null, &ok, this);

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
  if (0 == proxyStatus->exitStatus()){
    consoleOutSlot("Proxy Initialised");
  }
  else{
    consoleOutSlot("Proxy error or proxy does not exist");
  }
}


void RegLauncher::consoleOutSlot(const QString &text)
{
//  textOutListBox->insertItem(text);
//  textOutListBox->setBottomItem(textOutListBox->count()-1);

  textOutTextEdit->insertParagraph(text, -1);
}

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
    
    return;
  }

  // if the current selection is already highlighted - deselect it
  if (checkPointTreeListView->itemPos(selectedItem) == checkPointTreeListViewPreviousSelection){
    // toggle the status
    if (launchButton->text() == "Launch"){
      launchButton->setText("Restart");
    }
    else {
      checkPointTreeListView->clearSelection();

      // Be nice and tell the user what's going to happen if they click the launch button
      launchButton->setText("Launch");
    }
  }
  else {
    checkPointTreeListViewPreviousSelection = checkPointTreeListView->itemPos(selectedItem);
    config.currentCheckpointGSH = ((CheckPointTreeItem*)selectedItem)->getCheckPointGSH();

    // Be nice and tell the user what's going to happen if they click the launch button
    launchButton->setText("Restart");
  }
  
}



void RegLauncher::contextMenuRequestedSlot( QListViewItem *listViewItem, const QPoint &pnt, int column)
{
  // First of all check to see if the mouse was actually over a item
  if (listViewItem == NULL)
    return;

  // Cast the listViewItem to a CheckPointTreeItem
  //CheckPointTreeItem *checkPointTreeItem = (CheckPointTreeItem*)listViewItem;
  rightMouseCheckPointTreeItem = (CheckPointTreeItem*)listViewItem;
    
  QPopupMenu popupMenu;
  popupMenu.insertItem(QString("View Parameters"), 0);
  popupMenu.insertItem(QString("View GSH"), 1);
  popupMenu.insertItem(QString("View Input File"), 2);
  popupMenu.insertItem(QString("View CheckPoint Data"), 3);
  connect(&popupMenu, SIGNAL(activated(int)), this, SLOT(contextMenuItemSelectedSlot(int)));
  popupMenu.exec(pnt);
}


void RegLauncher::contextMenuItemSelectedSlot(int itemId)
{

  // See if we've selected to 'View Parameters'
  if (itemId == 0){
    // Since we're reusing the class from the steerer
    Output_log_struct tmp;

    // Get a copy of the parameters that we want
    if (rightMouseCheckPointTreeItem != NULL){
      CheckPointParamsList cpParamList = rightMouseCheckPointTreeItem->getParamsList();

      tmp.num_param = (int)cpParamList.size();
      for (unsigned int i=0; i<cpParamList.size(); i++){
        strcpy(tmp.param_labels[i], cpParamList[i].mLabel);
        strcpy(tmp.param_values[i], cpParamList[i].mValue);
      }
    }

    ChkPtVariableForm *aChkPtVariableForm = new ChkPtVariableForm(&tmp, this, "testDialog");
    aChkPtVariableForm->show();
  }

  // or if we've selected to 'View GSH'
  else if (itemId == 1){
    if (rightMouseCheckPointTreeItem != NULL)
      QInputDialog::getText("GSH", "Checkpoint GSH", QLineEdit::Normal, rightMouseCheckPointTreeItem->getCheckPointGSH(), NULL, this);
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
  
}


/*
THIS WON'T WORK WITHOUT NAMESPACES BECAUSE OF THE WAY GSOAP WORKS
void RegLauncher::getInputFileFromSGSGSH(const QString &sgsGSH, QString *result)
{
  struct soap soap;
  soap_init(&soap);

  tree__findServiceDataResponse *serviceDataResponse = new tree__findServiceDataResponse();
  if (soap_call_tree__findServiceData(&soap, sgsGSH, "", "", serviceDataResponse))
    soap_print_fault(&soap, stderr);
  else{
    if (result == NULL)
      result = new QString();
    *result = serviceDataResponse->_findServiceDataReturn;
  }
}
*/

//void RegLauncher::getJobStatus(const QString &pSGSGSH){
  // This code essentially cribbed from Andy Porter's steering lib
/*
  struct tns__findServiceDataResponse findServiceDataResponse;
  struct msg_struct *msg = NULL;
  struct soap soap;
  char queryBuf[REG_MAX_STRING_LENGTH];
  char *pChar = NULL;
  char *pChar1 = NULL;

  findServiceDataResponse._result = NULL;
  sprintf(queryBuf, "<ogsi:queryByServiceDataNames names=\"SGS:Application_status\"/>");

  if(soap_call_tns__findServiceData(&soap, pSGSGSH, "", queryBuf, &findServiceDataResponse )){

    fprintf(stderr, "Get_service_data: findServiceData failed:\n");
    soap_print_fault(&soap, stderr);

    return;
  }
*/
//}



