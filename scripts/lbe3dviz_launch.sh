#!/bin/sh
#----------------------------------------------------------------------
#  A shell script to launch the ReG steering system using COG kit,
#  globus or ssh.
#
#  (C) Copyright 2002, 2004, University of Manchester, United Kingdom,
#  all rights reserved.
#
#  This software is produced by the Supercomputing, Visualization and
#  e-Science Group, Manchester Computing, University of Manchester
#  as part of the RealityGrid project (http://www.realitygrid.org),
#  funded by the EPSRC under grants GR/R67699/01 and GR/R67699/02.
#
#  LICENCE TERMS
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  THIS MATERIAL IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. THE ENTIRE RISK AS TO THE QUALITY
#  AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE
#  DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
#  CORRECTION.
#
#  Author(s)......: <R. L. Pinning>
#  Initial version: <10.03.2003>
#  Modified:        <27.04.2004>
#
#------------------------------------------------------------------------

echo $1
. $1

case $ReG_LAUNCH in
     cog)
       GLOBUS_BIN_PATH=$COG_INSTALL_PATH/bin
      ;;
     globus)
       GLOBUS_BIN_PATH=$GLOBUS_LOCATION/bin
      ;;
     *)
       echo "Using ssh/scp for launching only...check public-key is correctly installed"
      ;;
esac

# Ascertain whether we have a valid grid-proxy 

case $ReG_LAUNCH in
  cog|globus)
  $GLOBUS_LOCATION/bin/grid-proxy-info -exists
  if [ $? -ne "0" ]
  then
     echo "No grid proxy, please invoke grid-proxy-init"
     exit
  fi
  ;;
esac

# Setup xhost access to allow steerer display

xhost + $VIZ_HOSTNAME

# Setup some variables

REG_TMP_FILE=/tmp/reg_viz_remote.$$
#REG_SGS_ADDRESS=$REG_VIS_GSH
#export REG_SGS_ADDRESS VIZ_HOSTNAME REG_TMP_FILE
export VIZ_HOSTNAME REG_TMP_FILE

#  Start visualisation 

   echo "#!/bin/sh" > $REG_TMP_FILE
   echo ". \$HOME/RealityGrid/etc/reg-user-env.sh" >> $REG_TMP_FILE
   echo "SSH=$SSH" >> $REG_TMP_FILE
   echo "export SSH" >> $REG_TMP_FILE
   echo "VIZ_STD_ERR_FILE=$VIZ_STD_ERR_FILE" >> $REG_TMP_FILE
   echo "export VIZ_STD_ERR_FILE" >> $REG_TMP_FILE
   echo "VIZ_STD_OUT_FILE=$VIZ_STD_OUT_FILE" >> $REG_TMP_FILE
   echo "export VIZ_STD_OUT_FILE" >> $REG_TMP_FILE
   echo "if [ ! -d \$HOME/RealityGrid/scratch/ReG_workdir_viz$$ ]" >> $REG_TMP_FILE
   echo "then" >> $REG_TMP_FILE
   echo "   mkdir \$HOME/RealityGrid/scratch/ReG_workdir_viz$$" >> $REG_TMP_FILE
   echo "fi" >> $REG_TMP_FILE
   echo "REG_WORKING_DIR=\$HOME/RealityGrid/scratch/ReG_workdir_viz$$" >> $REG_TMP_FILE
   echo "export REG_WORKING_DIR" >> $REG_TMP_FILE
   echo "REG_STEER_DIRECTORY=\$REG_WORKING_DIR" >> $REG_TMP_FILE
   echo "export REG_STEER_DIRECTORY" >> $REG_TMP_FILE
   echo "cd \$REG_WORKING_DIR" >> $REG_TMP_FILE
   echo "echo \"Starting script\"" >> $REG_TMP_FILE
   echo "DISPLAY=${CLIENT_DISPLAY}" >> $REG_TMP_FILE
   echo "export DISPLAY" >> $REG_TMP_FILE
   echo "UC_PROCESSORS=$VIZ_PROCESSORS" >> $REG_TMP_FILE
   echo "export UC_PROCESSORS" >> $REG_TMP_FILE
   echo "VIZ_TYPE=$VIZ_TYPE" >> $REG_TMP_FILE
   echo "export VIZ_TYPE" >> $REG_TMP_FILE
   echo "echo \$PATH" >> $REG_TMP_FILE
   if [ $SIM_HOSTNAME = viking-i00.viking.lesc.doc.ic.ac.uk ]
   then
      echo "REG_CONNECTOR_HOSTNAME=$SIM_NODE_HOSTNAME" >> $REG_TMP_FILE
   else
      echo "REG_CONNECTOR_HOSTNAME=$SIM_HOSTNAME" >> $REG_TMP_FILE
   fi
   echo "export REG_CONNECTOR_HOSTNAME" >> $REG_TMP_FILE
   echo "REG_SGS_ADDRESS=$REG_SGS_ADDRESS" >> $REG_TMP_FILE
   echo "export REG_SGS_ADDRESS" >> $REG_TMP_FILE
if [ $MULTICAST_ADDRESS ]
then 
   echo "\$HOME/RealityGrid/ReG-vol-viewer-sockets/vis_l2g $VIZ_TYPE $MULTICAST_ADDRESS" >> $REG_TMP_FILE
else
   echo "\$HOME/RealityGrid/ReG-vol-viewer-sockets/vis_l2g $VIZ_TYPE" >> $REG_TMP_FILE
fi

# Build RSL

echo "&(executable=\$(GLOBUSRUN_GASS_URL)/$REG_TMP_FILE)(stdout=$VIZ_STD_OUT_FILE)(stderr=$VIZ_STD_ERR_FILE)" > /tmp/viz_stage.rsl

$HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun $VIZ_HOSTNAME jobmanager-fork /tmp/viz_stage.rsl $VIZ_USER

if [ $? -gt "0" ]
then
   echo "Error with starting viz helper"
fi
