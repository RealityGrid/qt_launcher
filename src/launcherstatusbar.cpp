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

#include "launcherstatusbar.h"

LauncherStatusBar::LauncherStatusBar(){
}

LauncherStatusBar::LauncherStatusBar( QWidget * parent, const char * name )
: QStatusBar(parent, name){

}

LauncherStatusBar::~LauncherStatusBar(){
}

void LauncherStatusBar::customEvent( QCustomEvent *e )
{
  StatusMessageData *msg = (StatusMessageData *)(e->data());
  if(msg->mTimeout == 0){
    this->message(msg->msgTxt);
  } else {
    this->message(msg->msgTxt, msg->mTimeout);
  }

  // Clean up - I think Qt deletes the event object itself but it can't
  // know about the associated data so we delete that.
  delete msg;
}
