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

/** @file CheckPointTreeItem.h
    @author Mark Riding
    @author Andrew Porter
    @brief Application Class for QT launcher GUI.
*/

#ifndef _CHECKPOINTTREEITEM_H_
#define _CHECKPOINTTREEITEM_H_

#include "qlistview.h"
#include "qstring.h"

/** CheckPointTreeItem class
 *
 *  This class inherits QListViewItem, and so instances can be placed
 *  directly into a QListView. In our case, this is the checkpoint tree
 *  widget on the main launcher window.
 *
 *  We want the checkpoint tree to be self producing - in that we create
 *  a single instance of the top level node, and the tree goes away,
 *  calling Mark McKeown's web service via gSoap to build itself,
 *  populating the QListView as it goes. Of course, this will take several
 *  successive calls to the gSoap library, and hence will take time.
 *
 *  Therefore, whilst the CheckPointTree can be created directly, it will
 *  be best practice to use a builder object that inherits QThread, so
 *  that the GUI doesn't block when the tree is being built.
 *
 *  We'll call this builder CheckPointTree - hopefully not too confusing..
 */

#include "CheckPointTree.h"
#include "qvaluelist.h"

class CheckPointParams {
  public:
    QString mLabel;
    QString mHandle;
    QString mValue;

    CheckPointParams(){
      mLabel = "";
      mHandle = "";
      mValue = "";
    }
    CheckPointParams(const QString &pLabel, 
		     const QString &pHandle, 
		     const QString &pValue){
      mLabel = pLabel;
      mHandle = pHandle;
      mValue = pValue;
    }
    ~CheckPointParams(){}
};

typedef QValueList<CheckPointParams> CheckPointParamsList;

class CheckPointTreeItem: public QListViewItem {
  private:
    CheckPointTree *creatorThread;

    int             regSeqNum;
    QString         timestamp;
    QStringList     checkPointData;
    QString         tagForInput;
    QString         checkPointGSH;

    /// Pointer to parent of this node
    CheckPointTreeItem  *parent;
    CheckPointTreeItem  *children;
    /// The no. of children this node has
    int                  numChildren;
    /// Whether or not this node is the root of a checkpoint tree
    bool                 mIsRootNode;
    /// The parameters associated with this node
    CheckPointParamsList mParamsList;
    
  public:
    /// Use this constructor at the very top level only: this is
    /// not a full checkpoint tree node, but a place holder for the
    /// whole tree itself
    CheckPointTreeItem(QListView *parent, 
		       const QString &checkPointTreeHandle, 
		       CheckPointTree *creator);
    /// Standard constructor
    CheckPointTreeItem(CheckPointTreeItem *parent, 
		       const QString &checkPointTreeHandle, 
		       CheckPointTree *creator);
    
    ~CheckPointTreeItem();

    void                 getChildData();
    QString              getCheckPointGSH();
    void                 setParamsList(const CheckPointParamsList *pParamsList);
    CheckPointParamsList getParamsList();
    /// Method to remove a single node from within a tree - calls the node's
    /// destroy method which updates the node's children with its logged
    /// steering commands and the identity of its parent, moves them into
    /// the ServiceGroup of its parent and finally, deletes it.
    void                 destroy();
    CheckPointTreeItem  *getParent();
    bool                 isRootNode();
};

#endif

