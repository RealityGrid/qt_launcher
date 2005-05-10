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

/** @file CheckPointTree.cpp
    @brief Implements methods for handling the CheckPointTree
 */

#include "CheckPointTree.h"
#include "CheckPointTreeItem.h"
//#include "soapRealityGridTree.nsmap"
//#include "soapRealityGrid.nsmap"
#include "soapH.h"
#include "qdom.h"

using namespace std;

CheckPointTree::CheckPointTree(QListView *_parent, const QString &_address){
  rootAddress = _address;
  parent = _parent;
  run();
}

CheckPointTree::~CheckPointTree(){
}

void CheckPointTree::run(){
  QStringList t = getActiveTrees();

  for (unsigned int i=0; i<t.count(); i++){
    CheckPointTreeItem *tmp = new CheckPointTreeItem(parent, t[i], this);
  }

}


QStringList CheckPointTree::getActiveTrees(){
  struct soap soap;
  soap_init(&soap);

  //fact__getActiveTreesResponse *out = new fact__getActiveTreesResponse();
  rgtf__getActiveTreesResponse out;

  //  if (soap_call_rgtf__getActiveTrees ( &soap, rootAddress, "", out ))
  if (soap_call_rgtf__getActiveTrees ( &soap, rootAddress, "", &out ))
    soap_print_fault(&soap, stderr);

  QStringList activeTrees;
  //  parse(out->_getActiveTreesReturn);
  parse(out._getActiveTreesReturn);

  soap_end(&soap);
  return activeTrees;
}


void CheckPointTree::getChildNodes(const QString &handle, CheckPointTreeItem *t){
  struct soap soap;
  soap_init(&soap);

  //rgtf__getChildNodesResponse *out = new rgtf__getChildNodesResponse();
  rgt__getChildNodesResponse out;
  //if (soap_call_rgtf__getChildNodes(&soap, handle, "", out))
  if (soap_call_rgt__getChildNodes(&soap, handle, "", &out))
    soap_print_fault(&soap, stderr);

  // and then parse for children and data nodes, fill in list view as appropriate.....

  //parse(QString(out->_getChildNodesReturn), t);
  parse(QString(out._getChildNodesReturn), t);

  parent->triggerUpdate();
  
}


void CheckPointTree::getNodeData(const QString &handle, CheckPointTreeItem *t){
  struct soap soap;
  soap_init(&soap);

  //tree__getCheckPointDataResponse *out = new tree__getCheckPointDataResponse();
  rgt__getCheckPointDataResponse out;
  //if (soap_call_tree__getCheckPointData(&soap, handle, "", out))
  if (soap_call_rgt__getCheckPointData(&soap, handle, "", &out))
    soap_print_fault(&soap, stderr);

  //cout << handle << endl;
  //cout << "************CHKPNTDATA***********" << endl << 
  //out->_getCheckPointDataReturn << endl << 
  //  "******************************" << endl;

  //parse(QString(out->_getCheckPointDataReturn), t);
  parse(QString(out._getCheckPointDataReturn), t);
}



void CheckPointTree::parse(const QString &xmlDocString, CheckPointTreeItem *parentListViewItem){
  
  QString xmlResultsDoc = xmlDocString;

  QDomDocument doc("parsedDoc");
  doc.setContent(xmlResultsDoc);
  QDomElement docElem = doc.documentElement();
  QDomNode topLevelNode = docElem.firstChild();
  
  while( !topLevelNode.isNull() ) {
    CheckPointParamsList paramsList;

    QDomElement e = topLevelNode.toElement(); // try to convert the node to an element.

    if (e.tagName() == "ogsi:entry"){
      // possible variables
      QString memberServiceLocatorHandle = "";
      QString visibleTag = "";
      QString seqNum = "";
      bool isMainNode = false;

      QDomNode secondLevelNode = e.firstChild();
      while (!secondLevelNode.isNull()){
        QDomElement ee = secondLevelNode.toElement();

        if (ee.tagName() == "ogsi:serviceGroupEntryLocator"){
          // ignore this for now
        }
        else if (ee.tagName() == "ogsi:memberServiceLocator"){
          // we need this to get child nodes
          QDomNode locator = ee.firstChild();
          QDomNode handle = locator.firstChild();

          memberServiceLocatorHandle = handle.toElement().text();
        }
        else if (ee.tagName() == "ogsi:content"){
          // we need the "SEQUENCE_NUM" & "TIMESTAMP" values
          QDomNode Checkpoint_node_data = ee.firstChild();
          
          if (Checkpoint_node_data.toElement().tagName() == "Checkpoint_node_data"){

            QDomNode Param_LevelNode = Checkpoint_node_data.firstChild();

            while (!Param_LevelNode.isNull()){
              if (Param_LevelNode.toElement().tagName() != "Param")
                continue;
                
              isMainNode = true;
            
              QDomNode handle = Param_LevelNode.firstChild();
              QDomNode label = handle.nextSibling();
              QDomNode value = label.nextSibling();
                            
              if (label.toElement().text() == "SEQUENCE_NUM"){
                seqNum = value.toElement().text();
              }
              else if (label.toElement().text() == "TIMESTAMP"){
                visibleTag = value.toElement().text();
              }

              CheckPointParams tmp(label.toElement().text().stripWhiteSpace(), handle.toElement().text().stripWhiteSpace(), value.toElement().text().stripWhiteSpace());
              paramsList += tmp;

              Param_LevelNode = Param_LevelNode.nextSibling();
            }
          }
          else { // from: if (Checkpoint_node_data.toElement().tagName() == "Checkpoint_node_data"){
            // Create a label for the top level ActiveTrees level
            visibleTag = ee.text();
          }
          
        } // from:    if (e.tagName() == "ogsi:entry"){
                
        secondLevelNode = secondLevelNode.nextSibling();
      }

      // create a new node for the top of the tree
      // this is true if parentListViewItem is NULL
      CheckPointTreeItem *cpti;
      
      if (parentListViewItem == NULL){
        // this is a special case
        cpti = new CheckPointTreeItem(parent, memberServiceLocatorHandle, this);
        cpti->setText(0, visibleTag);
      }
      else {
        // this is the general case
        // Strategy:
        //  (assumes the first element in the xml is the actual node, not the children
        //   anything else would be folly anyway)
        //  create a single new node at this level,
        //  store child handles on that node, ready for their expansion

        // test to see if we're the top node... (?)
        // at present we check to see if we've meta-data in the node - this should only be
        // true for the queried node, but at present the web-service returns this for children
        // only. This needs to be fixed server side first.
        if (isMainNode){
          cpti = new CheckPointTreeItem(parentListViewItem, memberServiceLocatorHandle, this);
          cpti->setParamsList(paramsList);

          // Need to remove newline characters from the tag string
          int findNewLines = visibleTag.find("\n");
          while (findNewLines>=0){
            usleep(1000);
            int len = visibleTag.length();
            QString left = visibleTag.left(findNewLines);
            QString right = visibleTag.right(len-findNewLines-1);

            visibleTag = left+right;

            findNewLines = visibleTag.find("\n");
          }

          // set the tag text
          cpti->setText(0, visibleTag);
          cpti->setText(1, seqNum);

        }
        
      }
          
    }

    else if (e.tagName() == "Checkpoint_data"){
    }

    topLevelNode = topLevelNode.nextSibling();
  }   

}



