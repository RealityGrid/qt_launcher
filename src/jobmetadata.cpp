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

	QDomDocument *doc = new QDomDocument();
	
  // Get root node
  QDomElement root = doc->createElement( "registryEntry");
  doc->appendChild(root);

  // Type of service
  QDomElement eType = doc->createElement( "serviceType");
  root.appendChild(eType);

  QDomText tServiceType = doc->createTextNode("SGS");
  eType.appendChild(tServiceType);
  
  // Component description
  QDomElement eContent = doc->createElement( "componentContent");
  root.appendChild(eContent);

  // Date/time job launched
  // Create element
  QDomElement eTime = doc->createElement( "componentStartDateTime" );
  // Append element to root node
  eContent.appendChild(eTime);
 
	QDomText tnodeTime = doc->createTextNode(mLaunchTime);
  eTime.appendChild(tnodeTime);

  // Who launched it
  QDomElement ePerson = doc->createElement("componentCreatorName");
  eContent.appendChild(ePerson);

  QDomText tnodePerson = doc->createTextNode(mPersonLaunching);
  ePerson.appendChild(tnodePerson);

  // What organisation they belong to
  QDomElement eOrg = doc->createElement( "componentCreatorGroup" );
  eContent.appendChild(eOrg);

  QDomText tnodeOrg = doc->createTextNode(mOrganisation);
  eOrg.appendChild(tnodeOrg);

  // Which machine it's on
  QDomElement eHost = doc->createElement( "componentHost" );
  eContent.appendChild(eHost);

  QDomText tnodeHost = doc->createTextNode(mMachineName);
  eHost.appendChild(tnodeHost);

  // How many processors
  QDomElement eProc = doc->createElement( "componentNumPx" );
  eContent.appendChild(eProc);

  QDomText tnodeProc = doc->createTextNode(mNumProc);
  eProc.appendChild(tnodeProc);

  // What the software is
  QDomElement eSoftware = doc->createElement("componentSoftwarePackage");
  eContent.appendChild(eSoftware);

  QDomText tnodeSoftware = doc->createTextNode(mSoftwareDescription);
  eSoftware.appendChild(tnodeSoftware);

  // What the job is doing
  QDomElement ePurpose = doc->createElement("componentTaskDescription");
  eContent.appendChild(ePurpose);

  QDomText tnodePurpose = doc->createTextNode(mPurposeOfJob);
  ePurpose.appendChild(tnodePurpose);

  return doc->toString();
}
