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
  RunningJobsDialog rjd;
  QString selectedGSH;
  QString configFileList;

  // Now user has selected parent GSH, get its param defs...
  gridifier.getCoupledParamDefs(selectedGSH, &configFileList);

  cout << "result string is: " << configFileList << endl;
}

void GlobalParamConstructionForm::addGlobal_clicked()
{

}


void GlobalParamConstructionForm::createGlobal_clicked()
{

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
  mParentGSH = aGSH;
}
