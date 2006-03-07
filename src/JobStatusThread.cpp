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
    
---------------------------------------------------------------------------*/

#include "JobStatusThread.h"
#include "qprocess.h"
#include <iostream>
#include <ReG_Steer_Steerside.h>
#include "ReG_Steer_Common.h"
#include "ReG_Steer_XML.h"
#include <ReG_Steer_Steerside_WSRF.h>

/** @file JobStatusThread.cpp
    @brief Implementation of class for monitoring inital status of job.
    @author Mark Riding
    @author Andrew Porter
  */

using namespace std;

JobStatusThread::JobStatusThread(QApplication *aApp, QObject *aMainWindow, 
				 const SteeringService *aService, 
				 const QString &scriptsDir)
   : done(false)
{
  mMainWin = aMainWindow;
  mApp = aApp;
  mScriptsDir = scriptsDir;
  mService.mEPR = aService->mEPR;
  strncpy(mService.mSecurity.userDN, aService->mSecurity.userDN,
	  REG_MAX_STRING_LENGTH);
  strncpy(mService.mSecurity.passphrase, aService->mSecurity.passphrase,
	  REG_MAX_STRING_LENGTH);

  int index = mService.mEPR.find("/service", 0, true);
  mNameSpace = mService.mEPR.left(index);
  index = mNameSpace.findRev("/", index, true);
  mNameSpace = mNameSpace.right(mNameSpace.length() - index - 1);

  // Set the lifespan of the thread to 15 seconds.
  // This isn't currently used - some jobs could sit
  // in a queue for hours on end.
  lifespan = 15000000;

  soap_init2(&mSoap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
}

JobStatusThread::~JobStatusThread()
{
  soap_end(&mSoap);
}

void JobStatusThread::run(){
  while (!done){
    usleep(500000);
    getJobStatus();
  }
}

void JobStatusThread::getJobStatus(){

#ifndef REG_WSRF
  struct sgs__findServiceDataResponse out;
  QString arg("<ogsi:queryByServiceDataNames names=\"SGS:Application_status\"/>");
  if(soap_call_sgs__findServiceData(&mSoap, mService.mEPR.ascii(), "", 
				    (xsd__string)(arg.ascii()),
				    &out)){
    soap_print_fault(&mSoap, stderr);
    done = true;
    return;
  }
  QString results(out._findServiceDataReturn);
#else
  char *rpOut;
  char *rpName = "applicationStatus";
  cout << "ARPDBG, calling Get_resource_property..." << endl;
  Get_resource_property (&mSoap,
			 mService.mEPR.ascii(),
			 mService.mSecurity.userDN,
			 mService.mSecurity.passphrase,
			 rpName, &rpOut);
  cout << "ARPDBG, ...done calling Get_resource_property" << endl;
  QString results(rpOut);
#endif // ndef REG_WSRF

  // Generate an event to send to the status bar (have to do it this way
  // because the gui thread must be the one to do the update)
  QCustomEvent *aUpdateEvent = new QCustomEvent(QEvent::User+1);

  // need to think about having a timeout function
  if (results.find("NOT_STARTED")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Queued", 0);
    aUpdateEvent->setData(aData);
    cout << "posting not yet started notification" << endl;
    mApp->postEvent(mMainWin, aUpdateEvent);
    
  } else if (results.find("RUNNING")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Running", 5000);
    aUpdateEvent->setData(aData);
    cout << "posting RUNNING notification" << endl;
    mApp->postEvent(mMainWin, aUpdateEvent);
    done = true;
    
  } else if (results.find("STOPPING")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Stopping", 0);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mMainWin, aUpdateEvent);
    
  } else if (results.find("STOPPED")>=0){
    StatusMessageData *aData = new StatusMessageData("Job has Stopped", 5000);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mMainWin, aUpdateEvent);
    done = true;
    
  } else if (results.find("PAUSED")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Paused", 0);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mMainWin, aUpdateEvent);
        
  } else {
    StatusMessageData *aData = new StatusMessageData("No such job", 5000);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mMainWin, aUpdateEvent);
    done = true;
  }
  
}


void JobStatusThread::timeout(){
}


