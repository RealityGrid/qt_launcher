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
echo "Checkpoint GSH = $CHECKPOINT_GSH"

# Thirdly: Setup REG_STEER_HOME for library location

REG_STEER_HOME=$HOME/RealityGrid/reg_steer_lib
export REG_STEER_HOME REG_SGS_ADDRESS

# Get the names of the various files from the specified namd input file
# and also get the path to the namd input file so that we can reconstruct
# the full paths to each of the necessary files.

COORD_FILE=`awk '/coordinates/ {print $2}' $SIM_INFILE`
STRUCT_FILE=`awk '/structure/ {print $2}' $SIM_INFILE`
PARAM_FILE=`awk '/parameters/ {print $2}' $SIM_INFILE`
VECT_FILE=`awk '/extendedSystem/ {print $2}' $SIM_INFILE`
TMP_PATH=`echo $SIM_INFILE |  awk -F/ '{for(i=1;i<NF;i++){printf("%s/",$i)}}'`

# Fourthly: Export these variables for use in child scripts

REG_TMP_FILE=/tmp/reg_sim_remote.$$
export CHECKPOINT_GSH SIM_HOSTNAME SIM_STD_ERR_FILE SIM_STD_OUT_FILE SIM_PROCESSORS 

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

# Setup the script for running the lbe3d wrapper

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
echo "if [ ! -e \$HOME/RealityGrid/scratch/.reg.input-file.$$ ]" >> $REG_TMP_FILE
echo "then" >> $REG_TMP_FILE
echo "  echo \"Input file not found - exiting\"" >> $REG_TMP_FILE
echo "  exit" >> $REG_TMP_FILE
echo "fi" >> $REG_TMP_FILE
echo "mv -f \$HOME/RealityGrid/scratch/.reg.input-file.$$ ." >> $REG_TMP_FILE
echo "mv -f \$HOME/RealityGrid/scratch/${COORD_FILE}.$$ ./${COORD_FILE}" >> $REG_TMP_FILE
echo "mv -f \$HOME/RealityGrid/scratch/${STRUCT_FILE}.$$ ./${STRUCT_FILE}" >> $REG_TMP_FILE
echo "mv -f \$HOME/RealityGrid/scratch/${PARAM_FILE}.$$ ./${PARAM_FILE}" >> $REG_TMP_FILE
echo "mv -f \$HOME/RealityGrid/scratch/${VECT_FILE}.$$ ./${VECT_FILE}" >> $REG_TMP_FILE
echo "chmod a+w .reg.input-file.$$" >> $REG_TMP_FILE
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
echo "echo \"Starting mpi job...\"" >> $REG_TMP_FILE
echo "\$HOME/RealityGrid/bin/start_namd \$GS_INFILE" >> $REG_TMP_FILE

echo "Transferring simulation input file..."

if [ $CHECKPOINT_GSH ]
then 
# Build RSL
  echo "&(executable="/home/bezier1/globus/bin/rg-cp")(arguments="-vb -p 10 -tcp-bs 16777216 -t gsiftp://$SIM_HOSTNAME/~/RealityGrid/scratch -g $CHECKPOINT_GSH")" > /tmp/rgcp.rsl
  echo "Calling MM's rgcpc script on Bezier..."
  $HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun bezier.man.ac.uk jobmanager-fork /tmp/rgcp.rsl 
fi
case $SIM_HOSTNAME in
      localhost)
          cp -f $SIM_INFILE $HOME/RealityGrid/scratch/.reg.input-file.$$
          cp -f ${TMP_PATH}${COORD_FILE} $HOME/RealityGrid/scratch/${COORD_FILE}.$$
          cp -f ${TMP_PATH}${STRUCT_FILE} $HOME/RealityGrid/scratch/${STRUCT_FILE}.$$
          cp -f ${TMP_PATH}${PARAM_FILE} $HOME/RealityGrid/scratch/${PARAM_FILE}.$$
          cp -f ${TMP_PATH}${VECT_FILE} $HOME/RealityGrid/scratch/${VECT_FILE}.$$
          ;;
      *)
          case $ReG_LAUNCH in
             ssh)
               scp $SIM_INFILE $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/.reg.input-file.$$
               scp ${TMP_PATH}${COORD_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${COORD_FILE}.$$
               scp ${TMP_PATH}${STRUCT_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${STRUCT_FILE}.$$
               scp ${TMP_PATH}${PARAM_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${PARAM_FILE}.$$
               scp ${TMP_PATH}${VECT_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${VECT_FILE}.$$
               ;;
             *)
               $GLOBUS_BIN_PATH/globus-url-copy file:///$SIM_INFILE gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/.reg.input-file.$$
               $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${COORD_FILE} gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/${COORD_FILE}.$$
               $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${STRUCT_FILE} gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/${STRUCT_FILE}.$$
               $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${PARAM_FILE} gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/${PARAM_FILE}.$$
               $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${VECT_FILE} gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/${VECT_FILE}.$$
	       ;;
          esac
          ;;
esac

if [ $? -gt "0" ]
then
  echo "Error with transferring input file"
fi

echo "Starting simulation..."

# Build the RSL file

echo "&(executable=\$(GLOBUSRUN_GASS_URL)/$REG_TMP_FILE)(jobtype=single)(maxWallTime=$TIME_TO_RUN)(stdout=$SIM_STD_OUT_FILE)(stderr=$SIM_STD_ERR_FILE)(count=$SIM_PROCESSORS)" > /tmp/sim_stage.rsl

case $SIM_HOSTNAME in
       green.cfs.ac.uk)
        $HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun wren.cfs.ac.uk jobmanager-lsf-green /tmp/sim_stage.rsl $SIM_USER
          ;;
       fermat.cfs.ac.uk)
        $HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun wren.cfs.ac.uk jobmanager-lsf-fermat /tmp/sim_stage.rsl $SIM_USER
          ;;
       wren.cfs.ac.uk)
        $HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun wren.cfs.ac.uk jobmanager-lsf /tmp/sim_stage.rsl $SIM_USER
          ;;
       localhost)
          chmod a+x $REG_TMP_FILE
          $REG_TMP_FILE &> ${HOME}/${SIM_STD_ERR_FILE} &
          ;;
       *)
        $HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun $SIM_HOSTNAME jobmanager-fork /tmp/sim_stage.rsl $SIM_USER
          ;;
esac

if [ $? -gt "0" ]
then
  echo "Error with starting simulation"
  exit
fi
