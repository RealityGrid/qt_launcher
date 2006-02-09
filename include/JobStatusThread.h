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
#define WITH_CDATA // ensure that gSoap retains CDATA in xml strings
#include "soapH.h"

/** @file JobStatusThread.h
    @brief Class for monitoring inital status of job.
  */

class JobStatusThread: public QThread {
  public:
    /** Constructor method
        @param aApp Ptr to main QApplication object
        @param aMainWindow Ptr to launcher's main window
        @param aGSH Address of service to poll
        @param aUsername Username to use when accessing service
        @param aPasswd   Password to use when accessing service
        @param scriptsDir Location of the scripts directory */
    JobStatusThread(QApplication *aApp, QObject *aMainWindow,
                    const QString &aGSH, const QString &aUsername,
		    const QString &aPasswd, const QString &scriptsDir);
    ~JobStatusThread();

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
    /** Location of the scripts directory where the Perl scripts live */
    QString       mScriptsDir;
    /** Namespace (SGS or MetaSGS) of service to talk to */
    QString       mNameSpace;
    /** Password used to access secured SWS (i.e. WSRF only) */
    QString mPassword;
    /** Username used to access secured SWS (i.e. WSRF only) */
    QString mUsername;
    /** Used to signal when thread should exit */
    bool done;
    /** Lifespan of thread in microseconds - NOT currently used */
    long lifespan;
    /** Common soap struct for calls to service via gSoap */
    struct soap mSoap;

    void getJobStatus();

  public slots:
    void timeout();
};

#endif



