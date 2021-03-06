/*----------------------------------------------------------------------------
    Application Class for QT launcher GUI.
    Header

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

/** @file CheckPointTree.h
    @brief Header file for class for handling the CheckPointTree
    @author Mark Riding
 */

#ifndef _CHECKPOINTTREE_H_
#define _CHECKPOINTTREE_H_

#include "qlistview.h"
#include "qstringlist.h"
#include "qthread.h"


class CheckPointTreeItem;

/** Class containing methods for querying a checkpoint tree */
class CheckPointTree{
  public:
    CheckPointTree(QListView *parent, const QString &address);
    ~CheckPointTree();

    void getChildNodes(const QString &handle, CheckPointTreeItem *);
    void getNodeData(const QString &handle, CheckPointTreeItem *);

  protected:
    void run();

  private:
    QString    rootAddress;
    QListView *parent;
  
    void getActiveTrees();

    void parse(const QString &xmlDocString, 
	       CheckPointTreeItem *parentListViewItem = NULL);
};


// Have a class which holds the results of a WSDL web service query....



#endif
