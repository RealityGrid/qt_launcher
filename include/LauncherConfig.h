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

    Initial version by: M Riding, 29.09.2003
    
---------------------------------------------------------------------------*/

#ifndef _LAUNCHERCONFIG_H_
#define _LAUNCHERCONFIG_H_

#define notused -1

#define isosurface 0
#define volumeRender 1
#define hedgehog 2
#define cutPlane 3


#include "qstring.h"
#include "qstringlist.h"
#include "qvaluelist.h"
#include "jobmetadata.h"

#include <iostream>
using namespace std;

// ************************************** //
// Be aware that only some of the values  //
// of this class will get saved out /     //
// read back in on a writeConfig or       //
// readConfig call. I'll highlight which  //
// when I get a moment!                   //
// ************************************** //

class Container {
  public:
    QString mContainer;
    int mPort;

    Container(){};
    Container(const QString aContainer, const int aPort){
      mContainer = aContainer;
      mPort = aPort;
    }
    ~Container(){}
};

class Application {

  public:
    QString mAppName;
    int     mNumInputs;
    bool    mHasInputFile;
    bool    mIsRestartable;
    bool    mIsViz;
    
    Application(){};
    Application(const QString aName, const int aNumInputs, const bool aHasFile,
                const bool aIsRestartable, const bool aIsViz){
      mAppName = aName;
      mNumInputs = aNumInputs;
      mHasInputFile = aHasFile;
      mIsRestartable = aIsRestartable;
      mIsViz = aIsViz;
    }
    ~Application(){}
};

class Machine {

  public:
    QString mName;
    QString mJobManager;
    QString mOS;
    
    Machine(){};
    Machine(const QString aName, const QString aJobManager, const QString aOS){
      mName = aName;
      mJobManager = aJobManager;
      mOS = aOS;
      //cout << "Machine: name=" << aName << " jobmgr=" << aJobManager << " os="<< aOS << endl;
    }
    ~Machine(){}
};

class LauncherConfig {
  public:

  // Member variables
  
    // Grid Service Handles for the various components
    QString topLevelRegistryGSH;
    QString registryOfFactoriesGSH;
    QString SGSFactoryGSH;
    QString SGSGSH;

    QString currentCheckpointGSH;
    QString simulationGSH;
    QString visualizationGSH;
    
    bool    vizServer;
    bool    multicast;
    QString multicastAddress;
    bool    migration;
    bool    restart;
    bool    newTree;
    int     vizType;
    QString mInputFileName;

    //QString mTargetMachine;
    Machine *mTargetMachine;
    QString treeTag;
    int     mNumberProcessors;
    int     mNumberPipes;
    int     mTimeToRun;
    
    JobMetaData *mJobData;

    QValueList<Container> containerList;
    //QStringList machineList;
    //QStringList vizMachineList;
    QString selectedContainer;
    int containerPortNum;
    QString globusLocation;
    QValueList<Application> applicationList;
    Application *mAppToLaunch;
    QValueList<Machine> machineList;
    QValueList<Machine> vizMachineList;

    // Methods

    LauncherConfig();
    LauncherConfig(QString file);
    ~LauncherConfig();

    void writeConfig(QString file);
    void readConfig(QString file);
    QString toXML();

};

#endif
