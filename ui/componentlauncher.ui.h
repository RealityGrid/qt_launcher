/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qfiledialog.h"
#include "qprogressbar.h"

#include "Gridifier.h"

#include <iostream>
using namespace std;
#include <qmessagebox.h>

Gridifier mGridifier;

void ComponentLauncher::init(){
    // set the appropriate default pages correctly
    // page(0) is component selection (plus input file & run time)
    setAppropriate(page(1), false); // Select sim GSH
    setAppropriate(page(2), false); // Edit input file
    setAppropriate(page(3), true); // Select target machine
    setAppropriate(page(4), false); // Select visualization type
    setAppropriate(page(5), true); // Select container
    setAppropriate(page(6), true); // Enter job meta-data
    setAppropriate(page(7), false); // Enter tag for new Chk tree
    
    // hide the horizontal header in the gshTagTable
    gshTagTable->verticalHeader()->hide();
    gshTagTable->setLeftMargin(0);
    gshTagTable->setColumnWidth(0, 250);
    gshTagTable->setColumnWidth(1, 50);
    
    // turn off the multicast address input box by default
    mcastAddrLineEdit->setEnabled(false);

}

void ComponentLauncher::componentSelectedSlot()
{
    Application *chosenApp = &(mConfig->applicationList[componentComboBox->currentItem()]);

    // The pages of the launching wizard are as follows:
    // Page 0: type of app to launch + input file + max runtime - page is
    // always used but en/disable the input-file selection as appropriate
    textLabelInputFile->setEnabled(chosenApp->mHasInputFile);
    simInputLineEdit->setEnabled(chosenApp->mHasInputFile);
    simInputPushButton->setEnabled(chosenApp->mHasInputFile);

    // Page 1: GSH of data source (other component)
    setAppropriate(page(1), (chosenApp->mNumInputs > 0));
    // Page 2: Edit input file (only when restarting?)
    setAppropriate(page(2),
         ((mConfig->migration || mConfig->restart) && (chosenApp->mIsRestartable) && (chosenApp->mHasInputFile)));
    // Page 3: Select input file
	  //setAppropriate(page(4),
	  //     (!(mConfig->migration || mConfig->restart) && (chosenApp->mHasInputFile)));
    // Page 3: Select target machine (inc. num px, pipes, vizserver & multicast)
	  setAppropriate(page(3), true);
    // Page 4: Select type of viz (iso, cut etc.)
	  setAppropriate(page(4), chosenApp->mIsViz);
    // Page 5: Select container for SGS - always used
    // Page 6: Job description (used to be just a tag) - always used
    // Page 7: Tag for new checkpoint tree
    /*
	  if (chosenApp->mIsRestartable && mConfig->newTree){
	    setAppropriate(page(7), true);
	  }
	  else{
	    setAppropriate(page(7), false);
    }
    */
    setAppropriate(page(7), (chosenApp->mIsRestartable && mConfig->newTree));
}



void ComponentLauncher::sgsSoftwarePackageChanged( const QString & )
{
    
}

void ComponentLauncher::sgsOrgChanged( const QString & )
{
    
}

void ComponentLauncher::sgsTagEntered()
{
    Application *chosenApp = &(mConfig->applicationList[componentComboBox->currentItem()]);

    // Next and final page allows user to enter text to describe the
    // checkpoint tree they are about to create.  This is only
    // applicable if the job is restartable and the user hasn't
    // selected a checkpoint (in an existing tree) to start from.
    if (!(mConfig->newTree && chosenApp->mIsRestartable)){
	    setNextEnabled(currentPage(), false);
	    setFinishEnabled(currentPage(), true);
    }
}


void ComponentLauncher::treeTagEntered()
{
    if (mConfig->newTree){
	    setFinishEnabled(currentPage(), true);
    }
}


void ComponentLauncher::setConfig(LauncherConfig *aConfig )
{
    mConfig = aConfig;
    
    // populate the containerListBox
    int numContainers = mConfig->containerList.size();
    for (int i=0; i<numContainers; i++){
	    containerListBox->insertItem(mConfig->containerList[i].mContainer);
    }
    
    // populate the viz's simulation gsh entry box with a good default
    simulationGSHLineEdit->setText(mConfig->simulationGSH);
    
    // populate the gsh tag list view - spawn a process
    // to do this, read the results in from a slot here
    // when we're ready.
    mGridifier.getSGSies(mConfig->topLevelRegistryGSH, gshTagTable);
    
    // Test to see if we're doing a migration
    // Things are a bit different if we're migrating
    if (mConfig->migration || mConfig->restart){
	    setAppropriate(page(0), true); // Choose component (only need runtime)
	    setAppropriate(page(1), false); // Select sim. GSH
	    setAppropriate(page(2), true); // Edit input file
	    setAppropriate(page(3), true); // Select target machine
	    setAppropriate(page(4), false); // Select viz type
	    // force page 0 to be the first that's displayed
	    showPage(page(0));
    }
    
    // Make the user enter a tag if a new tree is being created
    if (mConfig->newTree){
	    setAppropriate(page(7), true);
    }

    // Set up the drop-down list of available apps using the list
    // we got from the configuration file
    QStringList list;
    for ( int i = 0; i < (int)mConfig->applicationList.count(); i++ )
      list += mConfig->applicationList[i].mAppName;

    componentComboBox->insertStringList(list, 0);
}



void ComponentLauncher::vizServerSlot()
{
    
}

void ComponentLauncher::simNumProcessorsSlot()
{
    
}

void ComponentLauncher::simTargetMachineSlot()
{
    
}

void ComponentLauncher::vizNumberPipesSlot()
{
    
}

void ComponentLauncher::vizNumProcessorsSlot()
{
    
}

void ComponentLauncher::vizTargetMachineSlot()
{
    
}

void ComponentLauncher::containerSelectedSlot()
{
    
}

/** Slot is called whenever the user changes the page
 */
void ComponentLauncher::pageSelectedSlot(const QString &string)
{
    if (string == title(page(0))){

    	// If restarting or migrating then we're only using the first page
      // to set the max runtime
      if(mConfig->restart || mConfig->migration){
        componentComboBox->setEnabled(false);
        textLabelInputFile->setEnabled(false);
        simInputLineEdit->setEnabled(false);
        simInputPushButton->setEnabled(false);
      }
    }
    // Automatically generate as much of the meta data for the job
    // as possible
    else if (string == title(page(6))){
    	// Find out who we are - could query our certificate at this stage
    	sgsUserNameLineEdit->setText(QString(getenv("USER")));

      // Get the current date and time
      QDateTime dt = QDateTime::currentDateTime();
      sgsCreationTimeLineEdit->setText(dt.toString(Qt::ISODate));
      
      if(!mConfig->restart && !mConfig->migration){
        Application *chosenApp = &(mConfig->applicationList[componentComboBox->currentItem()]);
        sgsSoftwarePackageLineEdit->setText(chosenApp->mAppName);
      }
      else{
        // If we're restarting then the application to launch is already set
        sgsSoftwarePackageLineEdit->setText(mConfig->mAppToLaunch->mAppName);
      }
    }
    else if(string == title(page(3))){
    
      Application *chosenApp = &(mConfig->applicationList[componentComboBox->currentItem()]);
      if(!chosenApp->mIsViz){
        vizServerCheckBox->setEnabled(false);
        mcastCheckBox->setEnabled(false);
        mcastAddrLineEdit->setEnabled(false);
        vizPipesLineEdit->setEnabled(false);

        // List of available hardware depends on whether app. is a viz.
        // or not
        targetMachineListBox->clear();
        for ( QValueList<Machine>::Iterator it = mConfig->machineList.begin();
                                        it != mConfig->machineList.end(); ++it ){
          targetMachineListBox->insertItem((*it).mName, -1);  // append to list
        }
      }
      else{
        targetMachineListBox->clear();
        for ( QValueList<Machine>::Iterator it = mConfig->vizMachineList.begin();
                                        it != mConfig->vizMachineList.end(); ++it ){
          targetMachineListBox->insertItem((*it).mName, -1); // append to list
        }
      } 
    }
}

/** This method overrides the default accept method.
 *  Use it to pick all the selections that the user has made,
 *  and enter them into the LauncherConfig data object.
 */
void ComponentLauncher::accept(){

    // Store ptr to chosen application
    if(!mConfig->restart && !mConfig->migration){
      // If this is a restart or migration then the app to launch is already set
      mConfig->mAppToLaunch = &(mConfig->applicationList[componentComboBox->currentItem()]);
    }
    // Target machine
    //mConfig->mTargetMachine = targetMachineListBox->currentText();
    if(mConfig->mAppToLaunch->mIsViz){
      mConfig->mTargetMachine = &(mConfig->vizMachineList[targetMachineListBox->currentItem()]);
    }
    else{
      mConfig->mTargetMachine = &(mConfig->machineList[targetMachineListBox->currentItem()]);
    }
    
    // Number processors
    mConfig->mNumberProcessors = numProcLineEdit->text().toInt();
    // Container
    mConfig->selectedContainer = containerListBox->currentText();

    // Input file name
    if (mConfig->mAppToLaunch->mHasInputFile && simInputLineEdit->text().length() != 0){
      mConfig->mInputFileName = simInputLineEdit->text();
    }

    // Tree tag
    if (mConfig->newTree){
      mConfig->treeTag = treeTagTextEdit->text().stripWhiteSpace();
    }

    // Time to run
    mConfig->mTimeToRun = runTimeLineEdit->text().toInt();

    // Container port number
    mConfig->containerPortNum = containerPortNumLineEdit->text().toInt();

    if(mConfig->mAppToLaunch->mIsViz){
      // Number pipes
      mConfig->mNumberPipes = vizPipesLineEdit->text().toInt();

      // VizServer
      mConfig->vizServer = vizServerCheckBox->isChecked();
	  
      // Multicast
      mConfig->multicast = mcastCheckBox->isChecked();
      mConfig->multicastAddress = mcastAddrLineEdit->text();

      // Visualization type
      mConfig->vizType = vizTypeComboBox->currentItem();
    }

    // GSH of data source
    if(mConfig->mAppToLaunch->mNumInputs > 0){

      if (simulationGSHLineEdit->text().length() == 0){
        // if the user's not entered anything - then use the value from the config
      }
      else{
        // otherwise replace the config value with what the user wants
        // be aware that this should really need to happen
        mConfig->simulationGSH = simulationGSHLineEdit->text();
      }
    }
        
    // Store meta-data about this job
    mConfig->mJobData->mPersonLaunching = sgsUserNameLineEdit->text();
    mConfig->mJobData->mOrganisation = sgsOrganisationLineEdit->text();
    mConfig->mJobData->mLaunchTime = sgsCreationTimeLineEdit->text();
    mConfig->mJobData->mSoftwareDescription = sgsSoftwarePackageLineEdit->text();
    mConfig->mJobData->mPurposeOfJob = tagTextEdit->text();

    done(1);
}


/** Bring up a file input selection dialog to choose the input for an lb3d
 *  simulation. Put the result into the accompanying line edit box
 */
void ComponentLauncher::simInputPushButton_clicked()
{
    QString s = QFileDialog::getOpenFileName(QDir::homeDirPath()+"/realityGrid/reg_qt_launcher", "Input file (*)", this, "open file dialog" "Choose a file" );
    simInputLineEdit->setText(s);
}

/** When the user double clicks on an item in the list view, promote it to the
 *  GSH line edit.
 */
void ComponentLauncher::gshTagSelectedSlot( int row, int col, int button, const QPoint & mousePos )
{
    // Stick the result in the Line edit
    simulationGSHLineEdit->setText(gshTagTable->text(row,0));
}

/** if the user's selected an item from the checkpoint tree, then fill
 *  it in in the wizard.
 */
void ComponentLauncher::setCheckPointGSH(const QString &checkPointGSH)
{
    //checkPointGSHLineEdit->setText(checkPointGSH);
    
    // if we're starting from a checkpoint tree then allow the user to
    // edit the input file.  This may be overridden if the user has
    // chosen to launch an app that's not restartable (i.e. wasn't
    // used to generate the checkpoint tree in the first place).
    setAppropriate(page(2), true);
    // don't allow them to select their own file
    // that's now done on p.0 anyway
    //setAppropriate(page(4), false);
}

/** if the user's restarting from a checkpoint, then grab the input file,
 *  tweak it, and allow them to add their own changes
 */
void ComponentLauncher::setInputFileTextEdit(const QString &text){
    inputFileTextEdit->setText(text);  
}

void ComponentLauncher::getInputFileTextEditText(QString *returnPtr){
    *returnPtr = inputFileTextEdit->text();
}


// ????????????????????
// not used - remove //
void ComponentLauncher::setApplication(RegLauncher *aRegLauncher){
    mRegLauncher = aRegLauncher;
}


/** When the user selects Multicast - turn the address input
 *  line edit on. Otherwise have it greyed out.
 */
void ComponentLauncher::multicastToggleSlot()
{
    if (targetMachineListBox->currentText() == "tg-master.uc.teragrid.org"){
	    // Argonne must use multicast - don't let the user turn it off
	    mcastCheckBox->setChecked(true);
	    mcastAddrLineEdit->setEnabled(true);
	    // don't let the user try a vizserver either
	    vizServerCheckBox->setChecked(false);
	    vizServerCheckBox->setEnabled(false);
	    // and pretend we're setting 9 nodes for the viz
	    numProcLineEdit->setText("8");
	    numProcLineEdit->setEnabled(false);
	    vizPipesLineEdit->setText("4");
	    vizPipesLineEdit->setEnabled(false);
	    return;
    }
    
    if (mcastCheckBox->isChecked())
   	  mcastAddrLineEdit->setEnabled(true);
    else
	    mcastAddrLineEdit->setEnabled(false);
}



/** We need to check to see if we've selected the Argonne cluster.
 *  It always needs a multicast address, and must have a time to run.
 */
void ComponentLauncher::vizTargetSelectedSlot( QListBoxItem *selectedMachine )
{
    if (selectedMachine != NULL & selectedMachine->text() == "tg-master.uc.teragrid.org"){
	    // check the multicast toggle
	    mcastCheckBox->setChecked(true);
	    multicastToggleSlot();
    }
}

/** Method is called when the user clicks on a container.
 *  It's merely a nicety that fills in the correct port num.
 */
void ComponentLauncher::containerListBoxSelectedSlot( QListBoxItem *selectedContainer )
{
    if (selectedContainer != NULL){
	    int containerIndex = containerListBox->index(selectedContainer);
	    int selectedPortNum = mConfig->containerList[containerIndex].mPort;
	    containerPortNumLineEdit->setText(QString::number(selectedPortNum));
    }
}


void ComponentLauncher::sgsCreationTimeChanged( const QString & )
{
    
}


void ComponentLauncher::sgsUserNameChanged( const QString & )
{
    
}


void ComponentLauncher::next()
{
  // If this is the first page then make sure the user has selected an
  // input file if one is required
  if( mConfig->applicationList[componentComboBox->currentItem()].mHasInputFile &&
      (this->indexOf(this->currentPage())) == 0){

    if(!mConfig->restart && !mConfig->migration){

      if(simInputLineEdit->text().length() == 0){

        QMessageBox::warning(this, "No input file",
                             "Chosen component requires that an input file be specified\n",
                             QMessageBox::Ok,
                             QMessageBox::NoButton,
                             QMessageBox::NoButton );
        return;
      }
      else if( !(QFile::exists(simInputLineEdit->text())) ){

        QMessageBox::warning(this, "Missing input file",
                             "The specified input file ("+simInputLineEdit->text()+") cannot be found\n",
                             QMessageBox::Ok,
                             QMessageBox::NoButton,
                             QMessageBox::NoButton );
        return;
      }
    }
  }
  QWizard::next();
}

