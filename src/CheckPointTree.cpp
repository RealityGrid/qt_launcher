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

#include "CheckPointTree.h"

#include "CheckPointTree.h"
#include "CheckPointTreeItem.h"
#include "soapRealityGridTree.nsmap"
#include "qdom.h"

using namespace std;

CheckPointTree::CheckPointTree(QListView *_parent) : rootAddress("http://vermont.mvc.mcc.ac.uk:50000/Session/RealityGridTree/factory"){
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

  fact__getActiveTreesResponse *out = new fact__getActiveTreesResponse();

  if (soap_call_fact__getActiveTrees ( &soap, rootAddress, "", out ))
    soap_print_fault(&soap, stderr);

//  cout << out->_getActiveTreesReturn << endl;
/*
  // At the moment the data returned from Mark McKeown's web service isn't
  // a valid XML document, since it doesn't have an single enclosing tag.
  // To get round this for now, pre and append temporay tags to the string.
  QString xmlResultDoc = QString("<foo>")+out->_getActiveTreesReturn+"</foo>";
  // There's another instance of it being not well formed - the <Checkpoint_node_data>
  // tag is never closed.... it should be before the next </ogsi:content>

  QStringList activeTrees;

  QDomDocument doc("ActiveTrees");
  doc.setContent(xmlResultDoc);

  QDomElement docElem = doc.documentElement();

  QDomNode n = docElem.firstChild();

  // Go through and grab all the active trees

  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.

    if( !e.isNull() && e.tagName() == "ogsi:entry") {

      QDomNode nn = e.firstChild();

      if (!nn.isNull()){
        QDomElement ee = nn.nextSibling().toElement();

        if (!ee.isNull() && ee.tagName() == "ogsi:memberServiceLocator") {

          nn = ee.firstChild();

          if (!nn.isNull()){
            ee = nn.toElement();

            if (!ee.isNull() && ee.tagName() == "ogsi:locator"){

              nn = ee.firstChild();

              if (!nn.isNull()){
                ee = nn.toElement();

                if (!ee.isNull() && ee.tagName() == "ogsi:handle"){
                  //cout << ee.text() << endl;
                  activeTrees += ee.text();
                }
              }

            }
          }

        }

      }
    }
    n = n.nextSibling();
  }
*/
  QStringList activeTrees;
  parse(out->_getActiveTreesReturn);

  return activeTrees;
}


void CheckPointTree::getChildNodes(const QString &handle, CheckPointTreeItem *t){
  struct soap soap;
  soap_init(&soap);

  tree__getChildNodesResponse *out = new tree__getChildNodesResponse();
  if (soap_call_tree__getChildNodes(&soap, handle, "", out))
    soap_print_fault(&soap, stderr);

//  cout << out->_getChildNodesReturn << endl;

  // and then parse for children and data nodes, fill in list view as appropriate.....

  parse(QString(out->_getChildNodesReturn), t);

  parent->triggerUpdate();
  
}


void CheckPointTree::getNodeData(const QString &handle, CheckPointTreeItem *t){
  struct soap soap;
  soap_init(&soap);

  tree__getCheckPointDataResponse *out = new tree__getCheckPointDataResponse();
  if (soap_call_tree__getCheckPointData(&soap, handle, "", out))
    soap_print_fault(&soap, stderr);

  cout << handle << endl;
  cout << "************CHKPNTDATA***********" << endl << out->_getCheckPointDataReturn << endl << "******************************" << endl;

  parse(QString(out->_getCheckPointDataReturn), t);
}



void CheckPointTree::parse(const QString &xmlDocString, CheckPointTreeItem *parentListViewItem){
/*  // First off deal with the invalid documents we're receiving from
  // Mark McKeown's web service on vermant
  QString xmlResultsDoc = QString("<foo>") + xmlDocString + QString("</foo>");
  int testForOddTag = xmlResultsDoc.find("<Checkpoint_node_data>");
  while (testForOddTag >= 0){
    // then we need to add a closing tag
    int insertAt = xmlResultsDoc.find("</ogsi:content>", testForOddTag);
    int closingTagLocation = xmlResultsDoc.find("</Checkpoint_node_data>");
    if (insertAt>closingTagLocation){
      xmlResultsDoc = xmlResultsDoc.left(insertAt) + "</Checkpoint_node_data>" + xmlResultsDoc.right(xmlResultsDoc.length()-insertAt);

      testForOddTag = xmlResultsDoc.find("<Checkpoint_node_data>", insertAt);
    }
    else
      testForOddTag = closingTagLocation;
  }
*/
  
  QString xmlResultsDoc = xmlDocString;

  QDomDocument doc("parsedDoc");
  doc.setContent(xmlResultsDoc);
  QDomElement docElem = doc.documentElement();
  QDomNode topLevelNode = docElem.firstChild();
  
  while( !topLevelNode.isNull() ) {
    CheckPointParamsList paramsList;

    QDomElement e = topLevelNode.toElement(); // try to convert the node to an element.
//    cout << e.tagName() << endl;

    if (e.tagName() == "ogsi:entry"){
      // possible variables
      QString memberServiceLocatorHandle = "";
      QString visibleTag = "";
      QString seqNum = "";
      bool isMainNode = false;

      QDomNode secondLevelNode = e.firstChild();
      while (!secondLevelNode.isNull()){
        QDomElement ee = secondLevelNode.toElement();
//        cout << " " << ee.tagName() << endl;

        if (ee.tagName() == "ogsi:serviceGroupEntryLocator"){
          // ignore this for now
        }
        else if (ee.tagName() == "ogsi:memberServiceLocator"){
          // we need this to get child nodes
          QDomNode locator = ee.firstChild();
          QDomNode handle = locator.firstChild();
//          cout << "  " << handle.toElement().text() << endl;

          memberServiceLocatorHandle = handle.toElement().text();
        }
        else if (ee.tagName() == "ogsi:content"){
          // we need the "REG_SEQ_NUM" & "TIMESTAMP" values
          QDomNode Checkpoint_node_data = ee.firstChild();
          
          if (Checkpoint_node_data.toElement().tagName() == "Checkpoint_node_data"){
//            cout << "  " << Checkpoint_node_data.toElement().tagName() << endl;
            QDomNode Param_LevelNode = Checkpoint_node_data.firstChild();

            while (!Param_LevelNode.isNull()){
              if (Param_LevelNode.toElement().tagName() != "Param")
                continue;
                
              isMainNode = true;
            
//              cout << "   " << Param_LevelNode.toElement().tagName() << endl;
              QDomNode handle = Param_LevelNode.firstChild();
              QDomNode label = handle.nextSibling();
              QDomNode value = label.nextSibling();
              
//              if (label.toElement().text() == "REG_SEQ_NUM" || label.toElement().text() == "TIMESTAMP"){
//                cout << "    handle:"<< handle.toElement().text() << endl;
//                cout << "    label:"<< label.toElement().text() << endl;
//                cout << "    value:"<< value.toElement().text() << endl;
//              }
              
              if (label.toElement().text() == "REG_SEQ_NUM"){
                seqNum = value.toElement().text();
              }
              else if (label.toElement().text() == "TIMESTAMP"){
                visibleTag = value.toElement().text();
              }

              CheckPointParams tmp(label.toElement().text().stripWhiteSpace(), handle.toElement().text().stripWhiteSpace(), value.toElement().text().stripWhiteSpace());
//              cout << label.toElement().text().stripWhiteSpace() << " " << handle.toElement().text().stripWhiteSpace() << " " << value.toElement().text().stripWhiteSpace() << endl;
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



