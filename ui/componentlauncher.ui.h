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


Gridifier mGridifier;

void ComponentLauncher::init(){
    // set the appropriate default pages correctly
    // temporarily disable the checkPointGSHLineEdit page
    setAppropriate(page(1), false);
    setAppropriate(page(2), false);
    setAppropriate(page(3), false);
    setAppropriate(page(4), true);
    setAppropriate(page(5), false);
    setAppropriate(page(6), true);
    setAppropriate(page(7), false);
    setAppropriate(page(9), true);
    setAppropriate(page(11), false);
    
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
    // Page 0: type of app to launch - always used
    // Page 1: GSH of checkpoint to restart from
 		setAppropriate(page(1), false);
    // Page 2: GSH of data source (other component)
    setAppropriate(page(2), (chosenApp->mNumInputs > 0));
    // Page 3: Edit input file (only when restarting?)
    setAppropriate(page(3),
         ((mConfig->migration || mConfig->restart) && (chosenApp->mIsRestartable) && (chosenApp->mHasInputFile)));
    // Page 4: Select input file
	  setAppropriate(page(4),
	       (!(mConfig->migration || mConfig->restart) && (chosenApp->mHasInputFile)));
    // Page 5: Select target machine (& num px) - USE PAGE 6 INSTEAD NOW
	  setAppropriate(page(5), false);
    // Page 6: Select target viz machine (inc. num px, pipes, vizserver & multicast)
	  setAppropriate(page(6), true);
    // Page 7: Select type of viz (iso, cut etc.)
	  setAppropriate(page(7), chosenApp->mIsViz);
    // Page 8: Select container for SGS - always used
    // Page 9: Max run time - always used
    // Page 10: Job description (used to be just a tag) - always used
    // Page 11: Tag for new checkpoint tree
	  if (chosenApp->mIsRestartable && mConfig->newTree){
	    setAppropriate(page(11), true);
	  }
	  else{
	    setAppropriate(page(11), false);
    }    
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
	    setAppropriate(page(0), false);
	    setAppropriate(page(1), false);
	    setAppropriate(page(2), false);
	    setAppropriate(page(3), true);
	    setAppropriate(page(4), false);
	    // force page 3 to be the first that's displayed
	    showPage(page(3));
    }
    
    // Make the user enter a tag if a new tree is being created
    if (mConfig->newTree){
	    setAppropriate(page(11), true);
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
    // Automatically generate as much of the meta data for the job
    // as possible
    if (string == title(page(10))){
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
    else if(string == title(page(6))){
    
      Application *chosenApp = &(mConfig->applicationList[componentComboBox->currentItem()]);
      if(!chosenApp->mIsViz){
        vizServerCheckBox->setEnabled(false);
        mcastCheckBox->setEnabled(false);
        mcastAddrLineEdit->setEnabled(false);
        vizPipesLineEdit->setEnabled(false);

        // List of available hardware depends on whether app. is a viz.
        // or not
        targetMachineListBox->insertStringList((mConfig->machineList));
      }
      else{
        targetMachineListBox->insertStringList((mConfig->vizMachineList));
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
    mConfig->mTargetMachine = targetMachineListBox->currentText();
    // Number processors
    mConfig->mNumberProcessors = numProcLineEdit->text().toInt();
 	  // Container
	  mConfig->selectedContainer = containerListBox->currentText();

    // CheckPoint GSH - note that this is entirely optional if we're not migrating
	  if (!mConfig->migration){
	    if (checkPointGSHLineEdit->text().length() == 0)
		    mConfig->currentCheckpointGSH = "";
	    else
		    mConfig->currentCheckpointGSH = checkPointGSHLineEdit->text();
	  }

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
void ComponentLauncher::simInputButtonPushedSlot()
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
    checkPointGSHLineEdit->setText(checkPointGSH);
    
    // if we're starting from a checkpoint tree then allow the user to
    // edit the input file.  This may be overridden if the user has
    // chosen to launch an app that's not restartable (i.e. wasn't
    // used to generate the checkpoint tree in the first place).
    setAppropriate(page(3), true);
    // don't allow them to select their own file
    setAppropriate(page(4), false);
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
	    // check the multicast toggle, turn on the time to run page
	    mcastCheckBox->setChecked(true);
	    multicastToggleSlot();
	
	    setAppropriate(page(9), true);
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
