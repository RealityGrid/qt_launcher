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

#  Source the GUI generated configuration file
. $1

# Get the time to run
TIME_TO_RUN=$2
export TIME_TO_RUN

# Optionally get the checkpoint data file name from the command line args
if [ $# -eq 3 ]
then
CHECKPOINT_GSH=$3
else
CHECKPOINT_GSH=""
fi
export CHECKPOINT_GSH
echo "Checkpoint GSH = $CHECKPOINT_GSH"

# Thirdly: Setup REG_STEER_HOME for library location

REG_STEER_HOME=$HOME/RealityGrid/reg_steer_lib
export REG_STEER_HOME

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

if [ $SIM_HOSTNAME != "localhost" ]
then
  case $ReG_LAUNCH in
      globus|cog)
      $GLOBUS_BIN_PATH/grid-proxy-info -exists
       if [ $? -ne "0" ]
       then
         echo "No grid proxy, please invoke grid-proxy-init"
         exit
       fi
      ;;
  esac
fi

if [ $SIM_HOSTNAME != "localhost" ]
then
   xhost + $SIM_HOSTNAME
fi

# Setup the script for launching the lb3dviz

REG_TMP_FILE=$REG_SCRATCH_DIRECTORY/reg_viz_remote.$$
export REG_TMP_FILE

REG_TMP_FILE_ONLY=reg_sim_remote.$$
export  REG_TMP_FILE_ONLY

echo "#!/bin/sh" > $REG_TMP_FILE
echo ". \$HOME/RealityGrid/etc/reg-user-env.sh" >> $REG_TMP_FILE
echo "SIM_STD_ERR_FILE=$SIM_STD_ERR_FILE" >> $REG_TMP_FILE
echo "export SIM_STD_ERR_FILE" >> $REG_TMP_FILE
echo "SIM_STD_OUT_FILE=$SIM_STD_OUT_FILE" >> $REG_TMP_FILE
echo "export SIM_STD_OUT_FILE" >> $REG_TMP_FILE
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
echo "REG_SGS_ADDRESS=$REG_SGS_ADDRESS" >> $REG_TMP_FILE
echo "export REG_SGS_ADDRESS" >> $REG_TMP_FILE
if [ $MULTICAST_ADDRESS ]
then 
   echo "\$HOME/RealityGrid/ReG-vol-viewer-sockets/vis_l2g $VIZ_TYPE $MULTICAST_ADDRESS" >> $REG_TMP_FILE
else
   echo "\$HOME/RealityGrid/ReG-vol-viewer-sockets/vis_l2g $VIZ_TYPE" >> $REG_TMP_FILE
fi

# Start the simulation using the script created above

echo "Starting simulation..."

$HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun 

echo "...done."
echo "-----------------"
