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

# ARPDBG - for testing at home
#SIM_USER=zzcguap
#export SIM_USER
#...ARPDBGEND

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

# Get the names of the various files from the specified input file
# and also get the path to this file so that we can reconstruct
# the full paths to each of the necessary files.

IN_FILE=`awk 'BEGIN{FS="\""};/^in_file/ {print $2}' $SIM_INFILE`
IO_FILE=`awk 'BEGIN{FS="\""};/^io_file/ {print $2}' $SIM_INFILE`
MOVIE_FILE=`awk 'BEGIN{FS="\""};/^imovie_file/ {print $2}' $SIM_INFILE`
POT_FILE=`awk 'BEGIN{FS="\""};/^ipot_file/ {print $2}' $SIM_INFILE`
CONS_FILE=`awk 'BEGIN{FS="\""};/^icons_file/ {print $2}' $SIM_INFILE`
CART_FILE=`awk 'BEGIN{FS="\""};/^incart_file/ {print $2}' $SIM_INFILE`
CHK_FILE=`awk 'BEGIN{FS="\""};/^icheck_file/ {print $2}' $SIM_INFILE`

TMP_PATH=`echo $SIM_INFILE |  awk -F/ '{for(i=1;i<NF;i++){printf("%s/",$i)}}'`

echo "TMP_PATH = $TMP_PATH"
echo "IN_FILE = $IN_FILE"
echo "IO_FILE = $IO_FILE"
echo "MOVIE_FILE = $MOVIE_FILE"
echo "POT_FILE = $POT_FILE"
echo "CONS_FILE = $CONS_FILE"
echo "CART_FILE = $CART_FILE"
echo "CHK_FILE = $CHK_FILE"

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

# Setup the script for running the namd wrapper

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
echo "  mkdir \$REG_WORKING_DIR" $ReG_LAUNCH>> $REG_TMP_FILE
echo "fi" >> $REG_TMP_FILE
echo "cd \$REG_WORKING_DIR" >> $REG_TMP_FILE

# Cope with fact that lemieux is not the frontend for file-transfer at PSC
GRIDFTP_HOSTNAME=$SIM_HOSTNAME

if [ "$SIM_HOSTNAME" == "lemieux.psc.edu" ]
then
  GRIDFTP_HOSTNAME="knucklehead.psc.edu"

elif [ "$SIM_HOSTNAME" == "fermat.cfs.ac.uk"  ]
then
  GRIDFTP_HOSTNAME="wren.cfs.ac.uk"

elif [ "$SIM_HOSTNAME" == "green.cfs.ac.uk"  ]
then
  GRIDFTP_HOSTNAME="wren.cfs.ac.uk"
fi

# On lemieux, the compute node can't even see the disk to which gridftp
# writes the files...
if [ "$SIM_HOSTNAME" == "lemieux.psc.edu" ]
then

  echo "/usr/psc/bin/far get RealityGrid/scratch/.reg.input-file.$$ ." >> $REG_TMP_FILE
  echo "/usr/psc/bin/far get RealityGrid/scratch/${IN_FILE}.$$ ." >> $REG_TMP_FILE
  echo "/usr/psc/bin/far get RealityGrid/scratch/${IO_FILE}.$$ ." >> $REG_TMP_FILE
  echo "/usr/psc/bin/far get RealityGrid/scratch/${MOVIE_FILE}.$$ ." >> $REG_TMP_FILE
  echo "/usr/psc/bin/far get RealityGrid/scratch/${POT_FILE}.$$ ." >> $REG_TMP_FILE
  echo "/usr/psc/bin/far get RealityGrid/scratch/${CONS_FILE}.$$ ." >> $REG_TMP_FILE
  echo "/usr/psc/bin/far get RealityGrid/scratch/${CART_FILE}.$$ ." >> $REG_TMP_FILE
  echo "/usr/psc/bin/far get RealityGrid/scratch/${CHK_FILE}.$$ ." >> $REG_TMP_FILE

else

  # Check that input file has arrived - don't do this for PSC since files
  # arrive on another disk
  echo "if [ ! -e \$HOME/RealityGrid/scratch/.reg.input-file.$$ ]" >> $REG_TMP_FILE
  echo "then" >> $REG_TMP_FILE
  echo "  echo \"Input file not found - exiting\"" >> $REG_TMP_FILE
  echo "  exit" >> $REG_TMP_FILE
  echo "fi" >> $REG_TMP_FILE

  echo "mv -f \$HOME/RealityGrid/scratch/.reg.input-file.$$ ./files.rc" >> $REG_TMP_FILE

  echo "mv -f \$HOME/RealityGrid/scratch/${IN_FILE}.$$ ./${IN_FILE}" >> $REG_TMP_FILE
  echo "mv -f \$HOME/RealityGrid/scratch/${IO_FILE}.$$ ./${IO_FILE}" >> $REG_TMP_FILE
  echo "mv -f \$HOME/RealityGrid/scratch/${MOVIE_FILE}.$$ ./${MOVIE_FILE}" >> $REG_TMP_FILE
  echo "mv -f \$HOME/RealityGrid/scratch/${POT_FILE}.$$ ./${POT_FILE}" >> $REG_TMP_FILE
  echo "mv -f \$HOME/RealityGrid/scratch/${CONS_FILE}.$$ ./${CONS_FILE}" >> $REG_TMP_FILE
  echo "mv -f \$HOME/RealityGrid/scratch/${CART_FILE}.$$ ./${CART_FILE}" >> $REG_TMP_FILE
  echo "mv -f \$HOME/RealityGrid/scratch/${CHK_FILE}.$$ ./${CHK_FILE}" >> $REG_TMP_FILE
fi

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
echo "REG_PASSPHRASE=$REG_PASSPHRASE" >> $REG_TMP_FILE
echo "export REG_PASSPHRASE" >> $REG_TMP_FILE
echo "echo \"Starting job...\"" >> $REG_TMP_FILE
echo "\$HOME/RealityGrid/bin/runctp" >> $REG_TMP_FILE

if [ $CHECKPOINT_GSH ]
then
./rg-cp -vb -p 10 -tcp-bs 16777216 -t gsiftp://$SIM_HOSTNAME/~/RealityGrid/scratch -g $CHECKPOINT_GSH
fi

echo "Transferring simulation input file..."

case $SIM_HOSTNAME in
      localhost)
          cp -f $SIM_INFILE $HOME/RealityGrid/scratch/.reg.input-file.$$
          cp -f ${TMP_PATH}${IN_FILE} $HOME/RealityGrid/scratch/${IN_FILE}.$$
          cp -f ${TMP_PATH}${IO_FILE} $HOME/RealityGrid/scratch/${IO_FILE}.$$
          cp -f ${TMP_PATH}${MOVIE_FILE} $HOME/RealityGrid/scratch/${MOVIE_FILE}.$$
          cp -f ${TMP_PATH}${POT_FILE} $HOME/RealityGrid/scratch/${POT_FILE}.$$
          cp -f ${TMP_PATH}${CONS_FILE} $HOME/RealityGrid/scratch/${CONS_FILE}.$$
          cp -f ${TMP_PATH}${CART_FILE} $HOME/RealityGrid/scratch/${CART_FILE}.$$
          cp -f ${TMP_PATH}${CHK_FILE} $HOME/RealityGrid/scratch/${CHK_FILE}.$$
          ;;
      *)
          case $ReG_LAUNCH in
             ssh)
echo "SIM_INFILE is: " $SIM_INFILE
echo "SIM_USER is: " $SIM_USER
echo "SIM_HOSTNAME is: " $SIM_HOSTNAME
               scp $SIM_INFILE $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/.reg.input-file.$$
               scp ${TMP_PATH}${IN_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${IN_FILE}.$$
               scp ${TMP_PATH}${IO_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${IO_FILE}.$$
               scp ${TMP_PATH}${MOVIE_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${MOVIE_FILE}.$$
               scp ${TMP_PATH}${POT_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${POT_FILE}.$$
               scp ${TMP_PATH}${CONS_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${CONS_FILE}.$$
               scp ${TMP_PATH}${CART_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${CART_FILE}.$$
               scp ${TMP_PATH}${CHK_FILE} $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/${CHK_FILE}.$$
               ;;
             *)
               $GLOBUS_BIN_PATH/globus-url-copy file:///$SIM_INFILE gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/.reg.input-file.$$
	       if [ "$CHECKPOINT_GSH" == "" ]
		   then
		   echo "Copying various input files to target machine"
		   $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${IN_FILE} gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/${IN_FILE}.$$
		   $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${IO_FILE} gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/${IO_FILE}.$$
 		   $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${MOVIE_FILE} gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/${MOVIE_FILE}.$$
		   $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${POT_FILE} gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/${POT_FILE}.$$
		   $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${CONS_FILE} gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/${CONS_FILE}.$$
		   $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${CART_FILE} gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/${CART_FILE}.$$
		   $GLOBUS_BIN_PATH/globus-url-copy file:///${TMP_PATH}${CHK_FILE} gsiftp://$GRIDFTP_HOSTNAME/\~/RealityGrid/scratch/${CHK_FILE}.$$
	       fi
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
