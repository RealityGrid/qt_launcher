/*----------------------------------------------------------------------------
    Application Class for QT launcher GUI.
    Implementation

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

    Initial version by: M Riding, 29.09.2003
    
---------------------------------------------------------------------------*/

#include "CheckPointTreeItem.h"


CheckPointTreeItem::CheckPointTreeItem(QListView *parent, const QString &checkPointTreeHandle, CheckPointTree *creator)
: QListViewItem(parent) {

  // set a reference to the creating thread, so we can call its
  // method later when we come to expand the tree
  creatorThread = creator;

  parent = NULL;
  children = NULL;
  numChildren = 0;

  // then set the text and make expandable
  //setText(0, checkPointTreeHandle);
  setExpandable(true);

  checkPointGSH = checkPointTreeHandle;
}


/** Constructor for a  tree item member.
 *
 *  We want each constructor to then go away and check for the gsi-ftp files....
 */
CheckPointTreeItem::CheckPointTreeItem(CheckPointTreeItem *_parent, const QString &checkPointTreeHandle, CheckPointTree *creator)
: QListViewItem(_parent) {
  // set a reference to the creating thread, so we can call its
  // method later when we come to expand the tree
  creatorThread = creator;

  parent = _parent;
  children = NULL;
  numChildren = 0;

  // then set the text and make expandable
  //setText(0, checkPointTreeHandle);
  setExpandable(true);

  checkPointGSH = checkPointTreeHandle;
}




CheckPointTreeItem::~CheckPointTreeItem(){
}


void CheckPointTreeItem::getChildData(){

  // Clear everything below
  CheckPointTreeItem *child = (CheckPointTreeItem*)firstChild();
  while (child != NULL){
    delete child;
    child = (CheckPointTreeItem*)firstChild();
  }

  if (creatorThread != NULL){
    // find the current node's data
    //creatorThread->getNodeData(checkPointGSH, this);
  
    // and find any children
    creatorThread->getChildNodes(checkPointGSH, this);
  }
}


QString CheckPointTreeItem::getCheckPointGSH(){
  return checkPointGSH;
}

void CheckPointTreeItem::setParamsList(const CheckPointParamsList &pParamsList){
  mParamsList = pParamsList;
}

CheckPointParamsList CheckPointTreeItem::getParamsList(){
  return mParamsList;
}


