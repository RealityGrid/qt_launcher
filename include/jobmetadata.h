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

#ifndef JOBMETADATA_H
#define JOBMETADATA_H

#include "qstring.h"

/**@file jobmetadata.h
  *Holds the description of the job being launched
  *@author Mark Riding
  */

class JobMetaData {
public: 
	JobMetaData();
	~JobMetaData();

	/** Convert the description to an xml doc */
	QString  toXML(const QString &serviceType);

	/** Who launched it */
	QString  mPersonLaunching;
	/** Which organisation the person launching it belongs to */
	QString  mOrganisation;
	/** The data and time at which the job was launched */
	QString  mLaunchTime;
	/** What software was launched */
	QString  mSoftwareDescription;
	/** What the job is doing */
	QString  mPurposeOfJob;
	/** Which machine the job is running on */
	QString  mMachineName;
	/** How many processors the job is using */
	QString  mNumProc;
};

#endif
