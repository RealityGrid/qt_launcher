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
  paramListTable->setLeftMargin(0);
  globalParamContentListBox->clear();
}

void GlobalParamConstructionForm::addGlobal_clicked()
{
  int numSel = paramListTable->numSelections();
  cout << "We have " << numSel << " selections" <<endl;

  globalParamContentListBox->clear();

  for(int i=0; i<numSel; i++){
    QTableSelection aSel = paramListTable->selection(i);
    QString aText = paramListTable->text(aSel.anchorRow(),aSel.anchorCol());
    globalParamContentListBox->insertItem(aText);
  }
}


void GlobalParamConstructionForm::createGlobal_clicked()
{
  bool ok;
  QString gName = QInputDialog::getText(
            "Global parameter", "Enter name of global parameter:", QLineEdit::Normal,
            QString::null, &ok, this );

  if( !(ok && !gName.isEmpty()) ){
    // user entered nothing or pressed Cancel
    return;
  }
  /*
  QString doc("<MSGS:Coupling_config>\n<Global_param_list>\n");
  doc.append("<Global_param name=\"" + gName + "\">\n")
  unsigned int nRows = globalParamContentListBox->count();
  for(unsigned int i=0; i<nRows; i++){

    doc.append("    <Child_param id=\"$param_set[$i]\" "
	       "label=\"" +  globalParamContentListBox->text(i) + "\"/>\n");
  }

  doc.append("  </Global_param>\n"
	     "</Global_param_list>\n</MSGS:Coupling_config>\n");
  */
  globalParamContentListBox->clear();
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
  QString     configFileList;
  QStringList result;
  int         i;
  
  mParentGSH = aGSH;

  if(!mConfig){
    cout << "WARNING: no pointer to config in "
      "GlobalParamConstructionForm::setParentGSH" << endl;
    return;
  }

  // Get param defs from service...
  // ...the output will be a space-delimited list of 
  // filenames
  if(!mParentGSH.isEmpty()){
    gridifier.getCoupledParamDefs(mParentGSH, &configFileList);
  }
  else{
    cout << "Have no GSH for parent service" << endl;
    return;
  }

  // clear out the table
  for (int i=paramListTable->numRows(); i>0; i--){
    paramListTable->removeRow(i);
  }
  for (int i=paramListTable->numCols(); i>0; i--){
    paramListTable->removeColumn(i);
  }

  result = QStringList::split(" ", configFileList);

  // Insert the required number of columns in the table
  paramListTable->insertColumns(0, result.count());
  // Make a hidden row to hold GSHs
  paramListTable->insertRows(1,0);
  //paramListTable->hideRow(0);

  int count = 0;
  // Loop over each file
  for (QStringList::Iterator it = result.begin(); it != result.end(); ++it){

    if((*it).isEmpty())continue;

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
    int index = appName.find(QString("_metadata"));
    appName.truncate(appName.length() - index);

    // Set the column header to be the name of the application
    paramListTable->horizontalHeader()->setLabel(count, appName );

    QDomNodeList list = doc.elementsByTagName(QString("Param_defs"));
    if(!list.item(0).isElement()){
      cout << "ARPDBG: not a QDomElement :-(" << endl;
      return;
    }
    QString gsh = list.item(0).toElement().attribute(QString("GSH"));
    paramListTable->setText(0, count, gsh);

    list = doc.elementsByTagName(QString("Label"));

    // '-1' allows for first, hidden, row of GSH values
    if( (list.count()-1) > paramListTable->numRows()){
      paramListTable->insertRows(paramListTable->numRows(),
				 (list.count() - 1 - paramListTable->numRows()));
    }

    for(i=1; i<list.count(); i++){
      QString label = list.item(i).firstChild().nodeValue();
      cout << "Param label = " << label << endl;
      paramListTable->setText(i, count, label);
    }

    file.close();
    ++count;
  }
}
