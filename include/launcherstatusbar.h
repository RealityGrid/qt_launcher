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

    Initial version by: A Porter, 29.09.2003
    
---------------------------------------------------------------------------*/

#ifndef LAUNCHERSTATUSBAR_H
#define LAUNCHERSTATUSBAR_H

#include <qstatusbar.h>

/**Subclassed status bar to receive custom events from jobstatusthread.
  *@author Andrew Porter
  */
class StatusMessageData {
public:
  StatusMessageData(const QString msg, const int timeout){

    msgTxt = msg;
    mTimeout = timeout;
  }
  
  QString msgTxt;
  int     mTimeout;
};

class LauncherStatusBar : public QStatusBar  {
public:
	LauncherStatusBar();
  LauncherStatusBar( QWidget * parent = 0, const char * name = 0 );
	~LauncherStatusBar();

	void customEvent ( QCustomEvent *e );
};

#endif