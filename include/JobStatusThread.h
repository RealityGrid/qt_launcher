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

#ifndef __JOBSTATUSTHREAD_H__
#define __JOBSTATUSTHREAD_H__

#include "qthread.h"
#include "qapplication.h"
#include "StatusMessageData.h"

/** @file JobStatusThread.h
    @brief Class for monitoring inital status of job.
  */

class JobStatusThread: public QThread {
  public:
    JobStatusThread(QApplication *aApp, QObject *aMainWindow,
                    const QString &aGSH);
    
  protected:
    /** Starts the thread */
    virtual void run();

  private:
    /** Pointer to main window so we can post GUI updates */
    QObject      *mMainWin;
    /** Pointer to the application object for which we are getting status */
    QApplication *mApp;
    /** GSH of the service to poll for status */
    QString       mGSH;
    /** Namespace (SGS or MetaSGS) of service to talk to */
    QString       mNameSpace;
    bool done;
    long age;
    long lifespan;
    
    void getJobStatus();

  public slots:
    void timeout();
};

#endif



