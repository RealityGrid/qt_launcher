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

#include "ProgressBarThread.h"
#include "unistd.h"

#include <iostream>
using namespace std;


ProgressBarThread::ProgressBarThread():
depressed(false)
{
}
ProgressBarThread::~ProgressBarThread(){
}


void ProgressBarThread::run(){
  int limit = 100;
  bool commitSuicide = false;

  mProgressBar.setTotalSteps(limit);
  mProgressBar.setPercentageVisible(false);
  mProgressBar.show();
  
  while (!commitSuicide){
        
    for(int i=0; i<limit; i++){
      mProgressBar.setProgress(i);
      usleep(10000);
      if (depressed){
        break; // then commit suicide
      }
    }

    for(int i=limit; i>0; i--){
      mProgressBar.setProgress(i);
      usleep(10000);
      if (depressed){
        break; // then commit suicide
      }
    }  

    if (depressed)
      commitSuicide = true;
  }

  mProgressBar.close();
  terminate();

}

void ProgressBarThread::kill(){
  depressed = true;
}


