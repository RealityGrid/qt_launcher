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

#include "jobmetadata.h"
#include <qdom.h>

#include <iostream>

using std::cout;
using std::endl;

JobMetaData::JobMetaData(){
}
JobMetaData::~JobMetaData(){
}


QString JobMetaData::toXML(){

	QDomDocument doc( "job" );
	
  // Get root node
  QDomElement root = doc.createElement( "componentContent");
  doc.appendChild(root);

  // Date/time job launched
  // Create element
  QDomElement eTime = doc.createElement( "componentStartDateTime" );
  // Append element to root node
  root.appendChild(eTime);
 
	QDomText tnodeTime = doc.createTextNode(mLaunchTime);
  eTime.appendChild(tnodeTime);

  // Who launched it
  QDomElement ePerson = doc.createElement("componentCreatorName");
  root.appendChild(ePerson);

  QDomText tnodePerson = doc.createTextNode(mPersonLaunching);
  ePerson.appendChild(tnodePerson);

  // What organisation they belong to
  QDomElement eOrg = doc.createElement( "componentCreatorGroup" );
  root.appendChild(eOrg);

  QDomText tnodeOrg = doc.createTextNode(mOrganisation);
  eOrg.appendChild(tnodeOrg);

  // What the software is
  QDomElement eSoftware = doc.createElement("componentSoftwarePackage");
  root.appendChild(eSoftware);

  QDomText tnodeSoftware = doc.createTextNode(mSoftwareDescription);
  eSoftware.appendChild(tnodeSoftware);

  // What the job is doing
  QDomElement ePurpose = doc.createElement("componentTaskDescription");
  root.appendChild(ePurpose);

  QDomText tnodePurpose = doc.createTextNode(mPurposeOfJob);
  ePurpose.appendChild(tnodePurpose);

  return doc.toString();
}
