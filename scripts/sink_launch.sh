#!/bin/sh
#set -x
# /*----------------------------------------------------------------------------
#    Script to start RealityGrid-L2 steered application
#
#    (C)Copyright 2003 The University of Manchester, United Kingdom,
#    all rights reserved.
#
#    This software is produced by the Supercomputing, Visualization &
#    e-Science Group, Manchester Computing, the Victoria University of
#    Manchester as part of the RealityGrid project.
#
#    This software has been tested with care but is not guaranteed for
#    any particular purpose. Neither the copyright holder, nor the
#    University of Manchester offer any warranties or representations,
#    nor do they accept any liabilities with respect to this software.
#
#    This software must not be used for commercial gain without the
#    written permission of the authors.
#
#    This software must not be redistributed without the written
#    permission of the authors.
#
#    Permission is granted to modify this software, provided any
#    modifications are made freely available to the original authors.
#
#    Supercomputing, Visualization & e-Science Group
#    Manchester Computing
#    University of Manchester
#    Manchester M13 9PL
#
#    WWW:    http://www.sve.man.ac.uk
#    email:  sve@man.ac.uk
#    Tel:    +44 161 275 6095
#    Fax:    +44 161 275 6800
#
#    Initial version by: R Pinning, 10.03.2003
#
#---------------------------------------------------------------------------*/

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

# Setup the script for running the 'sink' wrapper

REG_TMP_FILE=$REG_SCRATCH_DIRECTORY/reg_sim_remote.$$
export  REG_TMP_FILE

REG_TMP_FILE_ONLY=reg_sim_remote.$$
export  REG_TMP_FILE_ONLY

echo "#!/bin/sh" > $REG_TMP_FILE
echo ". \$HOME/RealityGrid/etc/reg-user-env.sh" >>$REG_TMP_FILE
echo "REG_WORKING_DIR=\$HOME/RealityGrid/scratch" >> $REG_TMP_FILE
echo "export REG_WORKING_DIR" >> $REG_TMP_FILE
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
echo "echo \"Starting job...\"" >> $REG_TMP_FILE
echo "\$HOME/RealityGrid/bin/sink" >> $REG_TMP_FILE

echo ""

# Start the simulation using the script created above

echo "Starting simulation..."

$HOME/RealityGrid/reg_qt_launcher/scripts/reg_globusrun 

echo "...done."
echo "-----------------"
