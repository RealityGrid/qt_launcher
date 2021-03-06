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

# Get the names of the various files from the specified namd input file
# and also get the path to the namd input file so that we can reconstruct
# the full paths to each of the necessary files.

INPUT_FILE=`awk '/^ *inputname/ {print $2}' $SIM_INFILE`
GEOM_FILE=`awk '/^ *geometry/ {print $2}' $SIM_INFILE`
CELLS_FILE="cells.hybrid"
TMP_PATH=`echo $SIM_INFILE |  awk -F/ '{for(i=1;i<NF;i++){printf("%s/",$i)}}'`

echo "TMP_PATH = $TMP_PATH"
echo "GEOM_FILE = $GEOM_FILE"
echo "CELLS_FILE = $CELLS_FILE"
echo "INPUT_FILE = $INPUT_FILE"

# Fourthly: Export these variables for use in child scripts

REG_TMP_FILE=$REG_SCRATCH_DIRECTORY/reg_sim_remote.$$
export  REG_TMP_FILE

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

if [ $SIM_HOSTNAME != "localhost" ]
then
  case $ReG_LAUNCH in
      globus|cog)
      $GLOBUS_BIN_PATH/grid-proxy-info -exists
       if [ $? -ne "0" ]
       then
         echo "ERROR: No grid proxy, please invoke grid-proxy-init"
         exit
       fi
      ;;
  esac
fi

# Setup the script for running the hydro

echo "#!/bin/sh" > $REG_TMP_FILE
echo ". \$HOME/RealityGrid/etc/reg-user-env.sh" >>$REG_TMP_FILE
echo "REG_WORKING_DIR=\$HOME/RealityGrid/scratch" >> $REG_TMP_FILE
echo "export REG_WORKING_DIR" >> $REG_TMP_FILE
echo "SSH=$SSH" >> $REG_TMP_FILE
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

# Check that input file has arrived 
echo "if [ ! -e \$HOME/RealityGrid/scratch/.reg.input-file.$$ ]" >> $REG_TMP_FILE
echo "then" >> $REG_TMP_FILE
echo "  echo \"Input file not found - exiting\"" >> $REG_TMP_FILE
echo "  exit" >> $REG_TMP_FILE
echo "fi" >> $REG_TMP_FILE
echo "mv -f \$HOME/RealityGrid/scratch/.reg.input-file.$$ ." >> $REG_TMP_FILE
if [ "$CHECKPOINT_GSH" == "" ]
    then 
    echo "mv -f \$HOME/RealityGrid/scratch/${GEOM_FILE}.$$ ./${GEOM_FILE}" >> $REG_TMP_FILE
    echo "mv -f \$HOME/RealityGrid/scratch/${INPUT_FILE}.$$ ./${INPUT_FILE}" >> $REG_TMP_FILE
    echo "mv -f \$HOME/RealityGrid/scratch/${CELLS_FILE}.$$ ./${CELLS_FILE}" >> $REG_TMP_FILE
fi

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
echo "REG_PASSPHRASE=$REG_PASSPHRASE" >> $REG_TMP_FILE
echo "export REG_PASSPHRASE" >> $REG_TMP_FILE
echo "REG_SGS_ADDRESS=$REG_SGS_ADDRESS" >> $REG_TMP_FILE
echo "export REG_SGS_ADDRESS" >> $REG_TMP_FILE
echo "echo \"Starting hydro job...\"" >> $REG_TMP_FILE
echo "\$HOME/RealityGrid/bin/hydro .reg.input-file.$$ > \$HOME/$SIM_STD_ERR_FILE 2>&1" >> $REG_TMP_FILE

echo "Transferring simulation input file..."

case $SIM_HOSTNAME in
     localhost)
       cp -f $SIM_INFILE $HOME/RealityGrid/scratch/.reg.input-file.$$
       cp -f ${TMP_PATH}$GEOM_FILE $HOME/RealityGrid/scratch/${GEOM_FILE}.$$
       cp -f ${TMP_PATH}$INPUT_FILE $HOME/RealityGrid/scratch/${INPUT_FILE}.$$
       cp -f ${TMP_PATH}$CELLS_FILE $HOME/RealityGrid/scratch/${CELLS_FILE}.$$
       ;;
     *)
       case $ReG_LAUNCH in
          ssh)
           scp $SIM_INFILE $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/.reg.input-file.$$
           scp ${TMP_PATH}$GEOM_FILE $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${GEOM_FILE}.$$
           scp ${TMP_PATH}$INPUT_FILE $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${INPUT_FILE}.$$
           scp ${TMP_PATH}$CELLS_FILE $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${CELLS_FILE}.$$
           ;;
          *)
           $GLOBUS_BIN_PATH/globus-url-copy file:///$SIM_INFILE gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/.reg.input-file.$$
           $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}$GEOM_FILE gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/${GEOM_FILE}.$$
           $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}$INPUT_FILE gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/${INPUT_FILE}.$$
           $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}$CELLS_FILE gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/${CELLS_FILE}.$$
	   ;;
       esac
       ;;
esac

if [ $? -gt "0" ]
then
  echo "ERROR with transferring input file"
  exit
fi

echo "Starting simulation..."

./reg_globusrun 

echo "...done."
echo "-----------------"
