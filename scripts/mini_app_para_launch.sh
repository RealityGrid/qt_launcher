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


# Instead source the GUI generated configuration file
. $1

# Firstly: Get the time to run

TIME_TO_RUN=$2
export TIME_TO_RUN

# Secondly: Optionally get the checkpoint data file name from the command line args

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

# Fourthly: Export these variables for use in child scripts

REG_TMP_FILE=$REG_SCRATCH_DIRECTORY/reg_sim_remote.$$
export REG_TMP_FILE 

REG_TMP_FILE_ONLY=reg_sim_remote.$$
export  REG_TMP_FILE_ONLY

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
    globus|cog)
    $GLOBUS_BIN_PATH/grid-proxy-info -exists
     if [ $? -ne "0" ]
     then
       echo "No grid proxy, please invoke grid-proxy-init"
       exit
     fi
    ;;
esac

# Setup the script for running the mini_app_para wrapper

echo "#!/bin/sh" > $REG_TMP_FILE
echo ". \$HOME/RealityGrid/etc/reg-user-env.sh" >>$REG_TMP_FILE
echo "REG_WORKING_DIR=\$HOME/RealityGrid/scratch" >> $REG_TMP_FILE
echo "export REG_WORKING_DIR" >> $REG_TMP_FILE
echo "SSH=\$SSH" >> $REG_TMP_FILE
echo "export SSH" >> $REG_TMP_FILE
echo "REG_STEER_DIRECTORY=\$REG_WORKING_DIR" >> $REG_TMP_FILE
echo "export REG_STEER_DIRECTORY" >> $REG_TMP_FILE
echo "echo \"Working directory is \$REG_WORKING_DIR\"" >> $REG_TMP_FILE
echo "echo \"Steering directory is \$REG_STEER_DIRECTORY\"" >> $REG_TMP_FILE
echo "if [ ! -d \$REG_WORKING_DIR ]" >> $REG_TMP_FILE
echo "then" >> $REG_TMP_FILE
echo "  mkdir \$REG_WORKING_DIR" >> $REG_TMP_FILE
echo "fi" >> $REG_TMP_FILE
echo "cd \$REG_WORKING_DIR" >> $REG_TMP_FILE
echo "UC_PROCESSORS=$SIM_PROCESSORS" >> $REG_TMP_FILE
echo "export UC_PROCESSORS" >> $REG_TMP_FILE
echo "TIME_TO_RUN=$TIME_TO_RUN" >> $REG_TMP_FILE
echo "export TIME_TO_RUN" >> $REG_TMP_FILE
echo "GS_INFILE=.reg.input-file.$$" >> $REG_TMP_FILE
echo "export GS_INFILE" >> $REG_TMP_FILE
echo "SIM_STD_ERR_FILE=$SIM_STD_ERR_FILE" >> $REG_TMP_FILE
echo "export SIM_STD_ERR_FILE" >> $REG_TMP_FILE
echo "SIM_STD_OUT_FILE=$SIM_STD_OUT_FILE" >> $REG_TMP_FILE
echo "export SIM_STD_OUT_FILE" >> $REG_TMP_FILE
echo "REG_SGS_ADDRESS=$REG_SGS_ADDRESS" >> $REG_TMP_FILE
echo "export REG_SGS_ADDRESS" >> $REG_TMP_FILE
echo "echo \"Starting mini_app_para job...\"" >> $REG_TMP_FILE
echo "\$HOME/RealityGrid/bin/start_mini_app_para" >> $REG_TMP_FILE

echo "Starting simulation..."

$HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun 

echo "...done."
echo "-----------------"
