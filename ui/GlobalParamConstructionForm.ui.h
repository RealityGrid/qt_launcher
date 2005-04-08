/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

/** @file GlobalParamConstructionForm.ui.h
    @brief Dialog for constructing global params for coupled models */

void GlobalParamConstructionForm::init()
{
  mConfig = NULL;
  mGridifier = NULL;
  paramListTable->setLeftMargin(0);
  globalParamContentListBox->clear();
  pushButton2->setEnabled(false);
}

void GlobalParamConstructionForm::addGlobal_clicked()
{
  unsigned int i;
  unsigned int numSel;

  // Clear out the list box and then put in the entries we
  // already know about
  globalParamContentListBox->clear();

  numSel = selectionList.count();
  // Enable the Create Global button if we have some values
  if (numSel) pushButton2->setEnabled(true);
  for(i=0; i<numSel; i+=2){
    QString aText = paramListTable->text(selectionList[i],
					 selectionList[i+1]);
    globalParamContentListBox->insertItem(aText);
  }

  // Now we add entries from the new user selection
  numSel = paramListTable->numSelections();
  // Enable the Create Global button if we have some values
  if (numSel) pushButton2->setEnabled(true);

  for(i=0; i<numSel; i++){
    QTableSelection aSel = paramListTable->selection(i);

    // Store the row and column of this selection so we have
    // it available to us in createGlobal_clicked()
    selectionList.append(aSel.anchorRow());
    selectionList.append(aSel.anchorCol());

    QString aText = paramListTable->text(aSel.anchorRow(),aSel.anchorCol());
    globalParamContentListBox->insertItem(aText);
  }
  paramListTable->clearSelection();
}


void GlobalParamConstructionForm::createGlobal_clicked()
{
  bool ok;
  QString gName = QInputDialog::getText(
            "Global parameter", "Enter name of global parameter:", 
	    QLineEdit::Normal,
            QString::null, &ok, this );

  if( !(ok && !gName.stripWhiteSpace().isEmpty()) ){
    // user entered nothing or pressed Cancel
    return;
  }

  QString doc("<MSGS:Coupling_config>\n<Global_param_list>\n");
  doc.append("<Global_param name=\"" + gName + "\">\n");
  unsigned int nItems = selectionList.count();
  for(unsigned int i=0; i<nItems; i+=2){

    // Reconstruct the full parameter label (we removed the
    // leading GSH ID to make it more user-friendly)
    QString gsh = paramListTable->text(0, selectionList[i+1]);
    QStringList bits = QStringList::split("/", gsh);
    QString gshID = bits.last() + "/";

    doc.append("    <Child_param id=\""+ gsh +
	       "\" label=\""+ gshID +
               paramListTable->text(selectionList[i],selectionList[i+1])+
	       "\"/>\n");

    // Update the table to replace the selected parameters' labels
    // with the name of the new global
    paramListTable->setText(selectionList[i],selectionList[i+1], gName);
  }

  doc.append("  </Global_param>\n"
	     "</Global_param_list>\n</MSGS:Coupling_config>\n");

  cout << doc << endl;

  if(mGridifier){
    mGridifier->setServiceData(QString("MetaSGS"), mParentGSH, doc);
  }
  else{
    cout << "GlobalParamConstructionForm::createGlobal_clicked: "
      "pointer to gridifier is NULL\n";
  }
  globalParamContentListBox->clear();
  selectionList.clear();
}


void GlobalParamConstructionForm::paramListTable_clicked( int lRow, int lCol, 
							  int lButton, const QPoint &aPt )
{
}


void GlobalParamConstructionForm::globalParamDoneButton_clicked()
{
  done(QDialog::Accepted);
}


void GlobalParamConstructionForm::globalParamCancelButton_clicked()
{
  done(QDialog::Rejected);
}


void GlobalParamConstructionForm::setConfig( LauncherConfig *aConfig )
{
  mConfig = aConfig;
}


void GlobalParamConstructionForm::setParentGSH( const QString &aGSH )
{
  QString      configFileList;
  QStringList  result;
  int          i;
  QStringList::Iterator it;

  mParentGSH = aGSH;

  if(!mConfig){
    cout << "WARNING: no pointer to config in "
      "GlobalParamConstructionForm::setParentGSH" << endl;
    return;
  }

  if(!mGridifier){
    cout << "WARNING: no pointer to gridifier in "
      "GlobalParamConstructionForm::setParentGSH" << endl;
    return;
  }

  // Get param defs from service...
  // ...the output will be a space-delimited list of filenames
  if(!mParentGSH.isEmpty()){
    mGridifier->setScriptsDirectory(mConfig->mScriptsDirectory);
    mGridifier->getCoupledParamDefs(mParentGSH, &configFileList);
  }
  else{
    cout << "Have no GSH for parent service" << endl;
    return;
  }
  result = QStringList::split(" ", configFileList);

  // clear out the table
  for (i=(paramListTable->numRows()-1); i>-1; i--){
    paramListTable->removeRow(i);
  }
  for (i=(paramListTable->numCols()-1); i>-1; i--){
    paramListTable->removeColumn(i);
  }

  int count = 0;
  // Count how many valid file names we have (& thus how
  // many columns we need)
  for (it = result.begin(); it != result.end(); ++it){
    if((*it).stripWhiteSpace().isEmpty())continue;
    ++count;
  }

  // Insert the required number of columns in the table
  paramListTable->insertColumns(0, count);
  // Make a hidden row to hold GSHs
  paramListTable->insertRows(0,1);
  paramListTable->hideRow(0);

  count = 0;
  // Loop over each file
  for (it = result.begin(); it != result.end(); ++it){

    if((*it).stripWhiteSpace().isEmpty())continue;

    QString name = mConfig->mScriptsDirectory + "/" + *it;
    QFile file(name);

    if(!file.open(IO_ReadOnly)){
      cout << "Failed to open: " << name << endl;
      continue;
    }

    QDomDocument doc("Param_defs");
    if(!doc.setContent(&file, false)){
      cout << "Failed to parse file from: " << name << endl;;
      continue;
    }
    QString appName = *it;
    // Get the application name by removing the stuff that is appended
    // to it in order to form the filename
    int index = appName.find(QString("_metadata"));
    appName.truncate(index);

    // Set the column header to be the name of the application
    paramListTable->horizontalHeader()->setLabel(count, appName );

    // Pull out the GSH of the service that has these param_defs
    QDomNodeList list = doc.elementsByTagName(QString("Param_defs"));
    if(!list.item(0).isElement()){
      cout << "ARPDBG: not a QDomElement :-(" << endl;
      return;
    }
    QString gsh = list.item(0).toElement().attribute(QString("GSH"));

    list = doc.elementsByTagName(QString("Label"));
    // '-1' allows for first, hidden, row of GSH values
    if( (int)(list.count()) > ((int)(paramListTable->numRows())-1)){
      paramListTable->insertRows(paramListTable->numRows(),
		       	 (list.count() + 1 - paramListTable->numRows()));
    }

    for(i=0; i<(int)(list.count()); i++){
      QString label = list.item(i).firstChild().nodeValue();
      // Remove the 'GSH_ID/' bit from the beginning of each label
      // to make it more user friendly.
      // The '\'s have to appear twice in C++ code??
      label.remove(QRegExp("^\\d*\\/"));

      // Insert the label into the table - '+1' allows for initial
      // (hidden) row holding the GSH
      paramListTable->setText(i+1, count, label.stripWhiteSpace());
    }

    // Adjust the column width BEFORE putting the (hidden) gsh string
    // into the top row of this column
    paramListTable->adjustColumn(count);
    paramListTable->setText(0, count, gsh);

    file.close();
    ++count;
  }
}


void GlobalParamConstructionForm::setGridifier( Gridifier *aGrid )
{
  mGridifier = aGrid;
}
