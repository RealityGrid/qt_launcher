/*----------------------------------------------------------------------------
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
---------------------------------------------------------------------------*/

/** @file
    @author Mark Riding
    @author Andrew Porter
    @brief Application Class for QT launcher GUI.
*/

#include "CheckPointTreeItem.h"
#include "soapH.h"
#include <qregexp.h>

using namespace std;

//----------------------------------------------------------------
CheckPointTreeItem::CheckPointTreeItem(QListView *parent, 
				       const QString &checkPointTreeHandle, 
				       CheckPointTree *creator)
: QListViewItem(parent) {

  // set a reference to the creating thread, so we can call its
  // method later when we come to expand the tree
  creatorThread = creator;
  mIsRootNode = true;
  parent = NULL;
  children = NULL;
  numChildren = 0;

  // make expandable
  setExpandable(true);

  checkPointGSH = checkPointTreeHandle;
  cout << "CheckPointTreeItem(): ROOT node, handle = " << checkPointGSH << endl;
  cout << "Parent = " << parent << endl;
  cout << "this is "<< this << endl;
}


/** Constructor for a  tree item member.
 *
 */
CheckPointTreeItem::CheckPointTreeItem(CheckPointTreeItem *_parent, 
				       const QString &checkPointTreeHandle, 
				       CheckPointTree *creator)
: QListViewItem(_parent) {
  // set a reference to the creating thread, so we can call its
  // method later when we come to expand the tree
  creatorThread = creator;
  mIsRootNode = false;
  parent = _parent;
  children = NULL;
  numChildren = 0;

  // make expandable
  setExpandable(true);

  checkPointGSH = checkPointTreeHandle;
}


//----------------------------------------------------------------
CheckPointTreeItem::~CheckPointTreeItem(){
}

//----------------------------------------------------------------
void CheckPointTreeItem::getChildData(){

  // Clear everything below
  CheckPointTreeItem *child = (CheckPointTreeItem*)firstChild();
  while (child != NULL){
    delete child;
    child = (CheckPointTreeItem*)firstChild();
  }

  if (creatorThread != NULL){
    // and find any children
    creatorThread->getChildNodes(checkPointGSH, this);
  }
}

//----------------------------------------------------------------
QString CheckPointTreeItem::getCheckPointGSH(){
  return checkPointGSH;
}

//----------------------------------------------------------------
void CheckPointTreeItem::setParamsList(const CheckPointParamsList *pParamsList){
  mParamsList = *pParamsList;
}

//----------------------------------------------------------------
CheckPointParamsList CheckPointTreeItem::getParamsList(){
  return mParamsList;
}

//----------------------------------------------------------------
void CheckPointTreeItem::destroy(){

  struct soap soap;

  cout << "CheckPointTreeItem::destroy: gsh = " << checkPointGSH << endl;

  soap_init(&soap);

  if(!mIsRootNode){
    cptn__destroyResponse out;

    if (soap_call_cptn__Destroy(&soap, checkPointGSH, "", NULL, &out)){
      cout << "CheckPointTreeItem::destroy - failed:" << endl;
      soap_print_fault(&soap, stderr);
    }
    else{
      cout << "CheckPointTreeItem::destroy - destroyed node with GSH = " <<
	checkPointGSH << endl;
    }
  }
  else{
    cpt__destroyResponse out;

    cout << "CheckPointTreeItem::destroy - this is a ROOT node" << endl;
    QString epr = checkPointGSH;
    epr.replace(QRegExp("Node"), "");
    cout << "arpdbg: epr for call to destroy is " << epr << endl;
    if (soap_call_cpt__Destroy(&soap, epr, "", NULL, &out)){
      cout << "CheckPointTreeItem::destroy - failed:" << endl;
      soap_print_fault(&soap, stderr);
    }
    else{
      cout << "CheckPointTreeItem::destroy - destroyed tree with root GSH = " <<
	checkPointGSH << endl;
    }
  }

  soap_end(&soap);
  soap_done(&soap);
}

//----------------------------------------------------------------
CheckPointTreeItem  *CheckPointTreeItem::getParent(){
  return parent;
}

//----------------------------------------------------------------
bool CheckPointTreeItem::isRootNode(){

  return mIsRootNode;
}
