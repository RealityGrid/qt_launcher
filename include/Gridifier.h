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

/** @file Gridifier.h
    @brief Class for wrapping scripts for handling OGSA framework and
           remote job launching */

#ifndef _GRIDIFIER_H_
#define _GRIDIFIER_H_

#include "qlistview.h"
#include "qprocess.h"
#include "qtable.h"

#include "LauncherConfig.h"

/// This class contains methods that either wrap perl scripts or make
/// calls using gSoap to interact with the OGSA framework and for
/// remote job launching.  Perl scripts are used for the (older) OGSI
/// stuff whilst gSoap bindings are used for interacting with the newer 
/// WSRF framework.
/// @author Mark Riding
/// @author Andrew Porter
class Gridifier: public QObject{

Q_OBJECT

public:
  Gridifier();
  ~Gridifier();

  /** Cleans up when launch fails */
  void cleanUp(LauncherConfig *config);
  /** Overloaded version that takes object describing service to Destroy 
      @param service Pointer to description of service to destroy */
  void Gridifier::cleanUp(SteeringService *service);
  /** Get list of available SGS factories */
  QString getSGSFactories(const QString &topLevelRegistry, 
			  const QString &desiredContainer,
			  const QString &className);
  /** Create an SGS factory in the specified container */
  QString makeSGSFactory(const QString &container, 
			 const QString &topLevelRegistry,
			 const QString &className);
  /** Create a SWS using the specified factory */
  QString makeSteeringService(const QString &factory, 
			      const LauncherConfig &config);
  /** Overloaded version for coupled models - create SWS and set up
      as child of specified parent service */
  QString makeSteeringService(const QString &factory, 
			      const LauncherConfig &config,
			      const QString &parentEPR);
  /** Creates an SGS for a visualization job using specified factory */
  QString makeVizSGS(const QString &factory, const LauncherConfig &config);
  /** OBSOLETE - creates a MetaSGS for use in a coupled model */
  QString makeMetaSGS(const QString &factory,
		      const LauncherConfig &config,
		      const QString &parentGSH);
  /** Instruct the specified simulation to take a checkpoint and then stop */
  QString checkPointAndStop(const QString &sgsGSH);
  /** Query the specified registry for details of available SGSs.  Details
      are then inserted in the GSHTagTable. 
      @param aConfig Pointer to LauncherConfig object holding info on location of registry and certificates etc.
      @param aGSHTagTable Pointer to table to update with results */
  void getSGSies(LauncherConfig *aConfig, 
		 QTable *aGSHTagTable);
  /** Query the specified (parent) service for details of its parameters
      so that 'global' parameters can be constructed for the coupled model */
  void getCoupledParamDefs(const QString &gsh, 
			   QString *aList);
  /** Make the configuration script that is sent to the remote host
      in order to set-up env before job launch */
  void makeReGScriptConfig(const QString &filename, 
			   const LauncherConfig &config);
  /** Run the appropriate <app-name>_launch.sh script to copy
      input files to remote machine and start job.
      @returns REG_SUCCESS if all OK, REG_FAILURE otherwise */
  int launchSimScript(const QString &scriptConfigFileName, 
		      const LauncherConfig &config);
  /** Run the appropriate <app-name>_launch.sh script to copy
      input files to remote machine and start vis. job.
      @returns REG_SUCCESS if all OK, REG_FAILURE otherwise */
  int launchVizScript(const QString &scriptConfigFileName, 
		      const LauncherConfig &config);
  /** Launch a vizualization job on the Argonne cluster */
  void launchArgonneViz(const LauncherConfig &config);
  /** Prototype function for using a web service to submit jobs */
  void webServiceJobSubmit(const QString &scriptConfigFileName);

  void copyCheckPointFiles(const QString &host);
  /** Not used */
  void gsiFtp(const QString &file, const QString &destination);

  /** Supply a pointer to the application object for the component
      to be launched */
  void setApplication(QApplication *);
  /** Set the location of the wrapper scripts */
  void setScriptsDirectory(const QString &dir);
  /** Set the service data of the specified service.  This routine
      adds the <ogsi:setByServiceDataNames> tags to the argument. */
  void setServiceData(const QString &nameSpace,
		      const QString &gsh,
		      const QString &sdeText);

private:
  QProcess *getSGSiesProcess;
  QProcess *getParamDefsProcess;

  /** Keep a reference to the sgs/tag listview */
  QTable *mGSHTagTable;

  QApplication *mApplication;
  /** Location of the various wrapper scripts */
  QString       mScriptsDir;

  /** Ptr to QString to hold list of files generated by perl 
      script which interogates parent MetaSGS for param defs */
  QString      *mFileListPtr;

public slots:
  void gsiFtpStdoutSlot();
  void gsiFtpStderrSlot();
  
};


#endif
