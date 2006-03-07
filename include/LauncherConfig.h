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
#include "qdom.h"
#include "ReG_Steer_Common.h"
#include <iostream>

using namespace std;

/** @file LauncherConfig.h
    @brief Defines class holding job description plus Container and Application
    and Machine classes.
*/

// ************************************** //
// Be aware that only some of the values  //
// of this class will get saved out /     //
// read back in on a writeConfig or       //
// readConfig call. I'll highlight which  //
// when I get a moment!                   //
// ************************************** //

/** Holds information on an application which
    we can launch */
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

/** Holds information on a machine on which
    we can launch jobs */
class Machine {

  public:
    QString mName;
    QString mJobManager;
    QString mOS;
    QString mQueue;
    
    Machine(){};
    Machine(const QString aName, const QString aJobManager,
            const QString aOS, const QString aQ){
      mName = aName;
      mJobManager = aJobManager;
      mOS = aOS;
      mQueue = aQ;
    }
    ~Machine(){}
};

/** Holds information on a single steering service (either SGS 
    or SWS) */
class SteeringService {
  public:
    /// The address of this steering service 
    QString mEPR;
    /// Holds authentication information for this service
    struct reg_security_info mSecurity;

    SteeringService(){};
    SteeringService(const QString aEPR,
		    const QString aUsername,
		    const QString aPassword){
    };
    ~SteeringService(){};
};

/** Holds information on the configuration of the launcher - both that
    read from file and that set by the user through the GUI */
class LauncherConfig {
  public:

  // Member variables
  
  /** Grid Service Handle for the top-level registry */
  QString topLevelRegistryGSH;
  /** Struct holding info. needed to authenticate to registry */
  struct reg_security_info registrySecurity;
  /** Grid Service Handle for the checkpoint tree factory */
  QString checkPointTreeFactoryGSH;
  /** Grid Service Handle of the registry for factories */
  QString registryOfFactoriesGSH;
  /** Grid Service Handle of the SGS Factory */
  QString SGSFactoryGSH;
  /** The last SGS created */
  SteeringService SGSGSH;
  /** Holds the GSH of the currently selected checkpoint */
  QString currentCheckpointGSH;
  /** The most recently launched component that is not a 
      visualization */
  SteeringService simulationGSH;
  /** The most recently launched vis. component */
  SteeringService visualizationGSH;
  /** Whether or not we're using vizServer */ 
  bool     vizServer;
  /// Whether or not visualization will be multicast
  bool     multicast;
  /// Address for visualization to multicast to
  QString  multicastAddress;
  /// Whether this is a job migration
  bool     migration;
  /// Whether this is a job restart (from a checkpoint)
  bool     restart;
  /// Whether or not to create a new checkpoint tree for this job
  bool     newTree;
  /// What sort of job this Viz. is (only relevant to lb3d vtk app)
  int      vizType;
  /// Name of the job's top-level input deck
  QString  mInputFileName;
  /// Pointer to description of machine on which to launch
  Machine *mTargetMachine;
  /// Meta-data for new checkpoint tree (if any)
  QString  treeTag;
  /// No. of processes job will use
  int      mNumberProcessors;
  /// No. of graphics pipes viz. job will use (NOT USED)
  int      mNumberPipes;
  /// Max. wallclock time of job in minutes
  int      mTimeToRun;
  /// Pointer to object containing metadata for job being launched
  JobMetaData *mJobData;
  /// List of available containers as obtained from Containers registry
  QStringList             mContainerList;
  /// Full address of container we're going to use in the current launch
  QString                 selectedContainer;
  /// Where globus lives on this machine
  QString                 globusLocation;
  /// List of applications that we can launch
  QValueList<Application> applicationList;
  /// List of machines on which we can launch applications
  QValueList<Machine>     machineList;
  /// List of visualization machines
  QValueList<Machine>     vizMachineList;
  /// Pointer to application that has been chosen for this launch
  Application            *mAppToLaunch;
  /// Whether we are launching a coupled model or not */
  bool    mIsCoupledModel;
  /** The location of the directory holding the perl scripts for doing SOAP
  QString mScriptsDirectory;
  /// The location of our scratch directory
  QString mScratchDirectory;
  /// How to launch remote jobs: one of "globus", "cog" or "ssh"
  QString mLaunchMethod; 
  /** The content of the default default.conf file - used to generate a new
      file if it is missing. */
  QString mConfigFileContent;
  /// Location of the steering client binary
  QString mSteererBinaryLocation;
  /// The password to give the job we are in the process of launching
  QString mServicePassword;

  // Methods

  LauncherConfig();
  LauncherConfig(QString file);
  ~LauncherConfig();

  void writeConfig(QString file);
  /** Reads specified configuration file and stores values */
  void readConfig(QString file);
  /** Use Qt XML support to generate XML document describing the
      job to be launched.*/
  QString toXML();
  /** Method loads a configuration xml file, and parses it to
   *  determine the stored security configuration values. */
  void readSecurityConfig(QString fileName);
  /** Helper method for parsing XML
      @param elem QDomElement from which to get attribute
      @param name The name of the attribute to get
      @return value of the specified attribute */
  QString getElementAttrValue(QDomElement elem, 
			      QString     name);

 protected:
  /** Create a new default.conf if it is missing
      @see mConfigFileContent */
  bool createNewConfigFile();
};

#endif
