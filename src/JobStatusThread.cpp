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

#include "JobStatusThread.h"

#include "qprocess.h"

JobStatusThread::JobStatusThread(QApplication *aApp, LauncherStatusBar *aStatusBar, const QString &aGSH)
: done(false)
{
  mainWindowStatusBar = aStatusBar;
  mGSH = aGSH;
  mApp = aApp;
  
  // Set the lifespan of the thread to 15 seconds.
  // This isn't currently used - some jobs could sit
  // in a queue for hours on end.
  lifespan = 15000000;

}

void JobStatusThread::run(){
  while (!done){
    usleep(500000);
    getJobStatus();
  }
}

void JobStatusThread::getJobStatus(){
  // Unfortunately this needs to be done by calling a script, rather than using gsoap directly from c++
  //
  // The reason for this is that the reg_steer_lib uses gsoap from c++, as does the library for the
  // checkpoint tree discovery. The issue is that we then have multiply defined functions and the linker
  // bails. There are three possible solutions:
  //  * combine the reg_steer_lib with the checkpoint tree discovery lib
  //      no good, since this would interfere with the encapsulation of both libs
  //  * use c++ namespaces for the checkpoint tree lib
  //      potentially the best way forward - but a bit of a botch also
  //  * just wrap around a script
  //      quickest way to progress - hence chosen - the deadline is fast approaching.

  // It's better that we run this process in stackspace rather than heapspace
  // That way QT doesn't have to worry about garbage collecting it
  QProcess jobStatusProcess(QString("./jobStatus.pl"));
  jobStatusProcess.setWorkingDirectory(QString(QDir::homeDirPath()+"/RealityGrid/reg_qt_launcher/scripts"));
  jobStatusProcess.addArgument(mGSH);

  jobStatusProcess.start();

  // We're in a thread - so no big worries about sitting waiting for
  // this process to finish
  while (jobStatusProcess.isRunning()){
    usleep(50000);
  }

  // Get the output, and simply grep for the desired results
  QString results = jobStatusProcess.readStdout();

  // Generate an event to send to the status bar (have to do it this way
  // because the gui thread must be the one to do the update)
  QCustomEvent *aUpdateEvent = new QCustomEvent(QEvent::User+1);

  // need to think about having a timeout function
  if (results.find("NOT_STARTED")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Queued", 0);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mainWindowStatusBar, aUpdateEvent);
    
  } else if (results.find("RUNNING")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Running", 2500);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mainWindowStatusBar, aUpdateEvent);
    done = true;
    
  } else if (results.find("STOPPING")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Stopping", 0);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mainWindowStatusBar, aUpdateEvent);
    
  } else if (results.find("STOPPED")>=0){
    StatusMessageData *aData = new StatusMessageData("Job has Stopped", 2000);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mainWindowStatusBar, aUpdateEvent);
    done = true;
    
  } else if (results.find("PAUSED")>=0){
    StatusMessageData *aData = new StatusMessageData("Job is Paused", 0);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mainWindowStatusBar, aUpdateEvent);
    
  } else {
    StatusMessageData *aData = new StatusMessageData("No such job", 5000);
    aUpdateEvent->setData(aData);
    mApp->postEvent(mainWindowStatusBar, aUpdateEvent);
    done = true;
  }
  
}


void JobStatusThread::timeout(){
}


