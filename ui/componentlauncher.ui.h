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

bool inputFileEditFlag = false;

void ComponentLauncher::init(){
    // set the appropriate default pages correctly
    // temporarily disable the checkPointGSHLineEdit page
    setAppropriate(page(1), false);
    // setAppropriate(page(1), true);
    setAppropriate(page(2), false);
    setAppropriate(page(3), false);
    setAppropriate(page(4), true);
    setAppropriate(page(5), true);
    setAppropriate(page(6), false);
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
    // now set all other pages in the wizard to be appropriate to the current context
    switch (componentComboBox->currentItem()){
	
    case lb3d:
	// temporarily disable the checkPointGSHLineEdit page
	setAppropriate(page(1), false);
	// setAppropriate(page(1), true);
	setAppropriate(page(2), mConfig->migration);
	setAppropriate(page(3), inputFileEditFlag);
	setAppropriate(page(4), !inputFileEditFlag);
	setAppropriate(page(5), true);
	setAppropriate(page(6), false);
	setAppropriate(page(7), false);
	setAppropriate(page(9), true);
	if (mConfig->newTree)
	    setAppropriate(page(11), true);
	else
	    setAppropriate(page(11), false);
	
	
	break;
	
    case miniapp:
	// temporarily disable the checkPointGSHLineEdit page
	setAppropriate(page(1), false);
	// setAppropriate(page(1), true);
	setAppropriate(page(2), false);
	setAppropriate(page(3), false);
	setAppropriate(page(4), false);
	setAppropriate(page(5), true);
	setAppropriate(page(6), false);
	setAppropriate(page(7), false);
	setAppropriate(page(9), true);
	if (mConfig->newTree)
	    setAppropriate(page(11), true);
	else
	    setAppropriate(page(11), false);
	
	break;
	
    case lb3dviz:
	setAppropriate(page(1), false);
	setAppropriate(page(2), true);
	setAppropriate(page(3), false);      
	setAppropriate(page(4), false);
	setAppropriate(page(5), false);
	setAppropriate(page(6), true);
	setAppropriate(page(7), true);
	setAppropriate(page(9), false);
	setAppropriate(page(11), false);
	
	break;
	
    default:
	setAppropriate(page(1), false);
	setAppropriate(page(2), false);
	setAppropriate(page(3), false);
	setAppropriate(page(4), false);
	setAppropriate(page(5), false);      
	setAppropriate(page(6), false);
	setAppropriate(page(7), false);
	setAppropriate(page(9), true);
	setAppropriate(page(11), false);
	
	break;
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
    if (!mConfig->newTree || componentComboBox->currentItem() == lb3dviz){
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
    //containerListBox->insertStringList((mConfig->containerList));
    int numContainers = mConfig->containerList.size();
    for (int i=0; i<numContainers; i++){
	containerListBox->insertItem(mConfig->containerList[i].mContainer);
    }
    // and the simTargetListBox too
    simTargetListBox->insertStringList((mConfig->machineList));
    // and the vizTargetListBox too
    vizTargetListBox->insertStringList((mConfig->vizMachineList));
    
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
    // Make a check in case the user left the componentSelection combo box on
    // on the default slot, hence didn't create an event for us to pick up
    //if (pageName == title(page(1))){
    //  componentSelectedSlot();
    //}
    cout << string << endl;
    
    if (string == title(page(10))){
    	// Find out who we are - could query our certificate at this stage
    	sgsUserNameLineEdit->setText(QString(getenv("USER")));

      // Get the current date and time
      QDateTime dt = QDateTime::currentDateTime();
      sgsCreationTimeLineEdit->setText(dt.toString());

      // What is it we're launching?
      switch (componentComboBox->currentItem()){

      case lb3d:
        sgsSoftwarePackageLineEdit->setText(QString("lb3d"));
        break;
      case lb3dviz:
        sgsSoftwarePackageLineEdit->setText(QString("Viz for lb3d"));
        break;
      }
    }
}

/** This method overrides the default accept method.
 *  Use it to pick all the selections that the user has made,
 *  and enter them into the LauncherConfig data object.
 */
void ComponentLauncher::accept(){
    
    // Check if it's a Sim or Viz
    int componentType = componentComboBox->currentItem();
    mConfig->selectedComponentType = componentType;
    
    // Sim test
    if (componentType == lb3d || componentType == miniapp){
	    mConfig->simComponentType = componentType;
	
	    // Target machine
			mConfig->simTargetMachine = simTargetListBox->currentText();
	
      // Number processors
	    mConfig->simNumberProcessors = simNumProcLineEdit->text().toInt();
	
	    // Container
	    mConfig->selectedContainer = containerListBox->currentText();
	
	    // sgs Tag
	    mConfig->simTag = tagTextEdit->text();
	
	    // CheckPoint GSH - note that this is entirely optional if we're not migrating
	    if (!mConfig->migration){
	      if (checkPointGSHLineEdit->text().length() == 0)
		      mConfig->currentCheckpointGSH = "";
	      else
		      mConfig->currentCheckpointGSH = checkPointGSHLineEdit->text();
	    }
	
	    // LB3D Input file name
	    if (componentType == lb3d && simInputLineEdit->text().length() != 0){
	      mConfig->lb3dInputFileName = simInputLineEdit->text();
	    }
	
	    // Tree tag
	    if (mConfig->newTree){
	      mConfig->treeTag = treeTagTextEdit->text().stripWhiteSpace();
	    }
	
	    // Time to run
	    mConfig->simTimeToRun = runTimeLineEdit->text().toInt();
	
	    // Container port number
	    mConfig->containerPortNum = containerPortNumLineEdit->text().toInt();
    }
    // Viz test
    else if (componentType == lb3dviz){
	    mConfig->vizComponentType = componentType;
	
	    // Target machine
	    mConfig->vizTargetMachine = vizTargetListBox->currentText();
	
	    // Number processors
	    mConfig->vizNumberProcessors = vizNumProcLineEdit->text().toInt();
	
	    // Number pipes
	    mConfig->vizNumberPipes = vizPipesLineEdit->text().toInt();
	
	    // VizServer
	    mConfig->vizServer = vizServerCheckBox->isChecked();
	
	    // Container
	    mConfig->selectedContainer = containerListBox->currentText();
	
    	// Job meta data
	    mConfig->vizTag = tagTextEdit->text();
	
	    // Visualization type
	    mConfig->vizType = vizTypeComboBox->currentItem();
	
	    // Simulation GSH
	    if (simulationGSHLineEdit->text().length() == 0){
	      // if the user's not entered anything - then use the value from the config
	    }
	    else
	      // otherwise replace the config value with what the user wants
	      // be aware that this should really need to happen
	      mConfig->simulationGSH = simulationGSHLineEdit->text();
	
	    // Multicast
	    mConfig->multicast = mcastCheckBox->isChecked();
	    mConfig->multicastAddress = mcastAddrLineEdit->text();
	
	    // Container port number
	    mConfig->containerPortNum = containerPortNumLineEdit->text().toInt();
	
	    // Time to run
	    mConfig->vizTimeToRun = runTimeLineEdit->text().toInt();
    }
    
    // Deal with the tag
    if (tagTextEdit->length()!=0){
	
	    if (mConfig->simComponentType == lb3d || mConfig->simComponentType == miniapp){
	      mConfig->simTag = tagTextEdit->text();
	    }
	    else if (mConfig->vizComponentType == lb3dviz){
	      mConfig->vizTag = tagTextEdit->text();
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
    QString s = QFileDialog::getOpenFileName(QDir::homeDirPath()+"/realityGrid/reg_qt_launcher", "LB3D input file (*)", this, "open file dialog" "Choose a file" );
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
    
    // if we're starting from a checkpoint tree then allow the user to edit the input file
    inputFileEditFlag = true;
    setAppropriate(page(3), inputFileEditFlag);
    // don't allow them to select their own file
    setAppropriate(page(4), !inputFileEditFlag);
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
    if (vizTargetListBox->currentText() == "tg-master.uc.teragrid.org"){
	// Argonne must use multicast - don't let the user turn it off
	mcastCheckBox->setChecked(true);
	mcastAddrLineEdit->setEnabled(true);
	// don't let the user try a vizserver either
	vizServerCheckBox->setChecked(false);
	vizServerCheckBox->setEnabled(false);
	// and pretend we're setting 9 nodes for the viz
	vizNumProcLineEdit->setText("8");
	vizNumProcLineEdit->setEnabled(false);
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
    else {
	vizServerCheckBox->setEnabled(true);
	setAppropriate(page(9), false);
	vizNumProcLineEdit->setText("1");
	vizNumProcLineEdit->setEnabled(true);
	vizPipesLineEdit->setText("1");
	vizPipesLineEdit->setEnabled(true);
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
