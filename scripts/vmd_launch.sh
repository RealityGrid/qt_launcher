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

# Instead source the GUI generated configuration file
. $1

# Get the time to run
TIME_TO_RUN=$2

# Optionally get the checkpoint data file name from the command line args
if [ $# -eq 3 ]
then
CHECKPOINT_GSH=$3
else
CHECKPOINT_GSH=""
fi

echo "Checkpoint GSH = $CHECKPOINT_GSH"

# Ascertain whether we have a valid grid-proxy 

if [ $SSH -eq 0 ]
then
echo "Using Globus"
   $GLOBUS_LOCATION/bin/grid-proxy-info -e
   if [ $? -ne "0" ]
     then
       echo "No grid proxy, please invoke grid-proxy-init"
       exit
   fi
else
echo "Using SSH for launching only...check public-key is correctly installed"
fi

REG_STEER_HOME=$HOME/RealityGrid/reg_steer_lib
export REG_STEER_HOME REG_SGS_ADDRESS

# Setup the script for running the vmd wrapper

echo "#!/bin/sh" > /tmp/reg_sim_remote.$$
echo ". \$HOME/RealityGrid/etc/reg-user-env.sh" >>/tmp/reg_sim_remote.$$
echo "REG_WORKING_DIR=\$HOME/RealityGrid/scratch" >> /tmp/reg_sim_remote.$$
echo "export REG_WORKING_DIR" >> /tmp/reg_sim_remote.$$
echo "SSH=\$SSH" >> /tmp/reg_sim_remote.$$
echo "export SSH" >> /tmp/reg_sim_remote.$$
echo "REG_STEER_DIRECTORY=\$REG_WORKING_DIR" >> /tmp/reg_sim_remote.$$
echo "export REG_STEER_DIRECTORY" >> /tmp/reg_sim_remote.$$
echo "echo \"Working directory is \$REG_WORKING_DIR\"" >> /tmp/reg_sim_remote.$$
echo "echo \"Steering directory is \$REG_STEER_DIRECTORY\"" >> /tmp/reg_sim_remote.$$
echo "if [ ! -d \$REG_WORKING_DIR ]" >> /tmp/reg_sim_remote.$$
echo "then" >> /tmp/reg_sim_remote.$$
echo "  mkdir \$REG_WORKING_DIR" >> /tmp/reg_sim_remote.$$
echo "fi" >> /tmp/reg_sim_remote.$$
echo "cd \$REG_WORKING_DIR" >> /tmp/reg_sim_remote.$$
echo "if [ ! -e \$HOME/RealityGrid/scratch/.reg.input-file.$$ ]" >> /tmp/reg_sim_remote.$$
echo "then" >> /tmp/reg_sim_remote.$$
echo "  echo \"Input file not found - exiting\"" >> /tmp/reg_sim_remote.$$
echo "  exit" >> /tmp/reg_sim_remote.$$
echo "fi" >> /tmp/reg_sim_remote.$$
echo "mv -f \$HOME/RealityGrid/scratch/.reg.input-file.$$ ." >> /tmp/reg_sim_remote.$$
echo "chmod a+w .reg.input-file.$$" >> /tmp/reg_sim_remote.$$
echo "UC_PROCESSORS=$SIM_PROCESSORS" >> /tmp/reg_sim_remote.$$
echo "export UC_PROCESSORS" >> /tmp/reg_sim_remote.$$
echo "TIME_TO_RUN=$TIME_TO_RUN" >> /tmp/reg_sim_remote.$$
echo "export TIME_TO_RUN" >> /tmp/reg_sim_remote.$$
echo "GS_INFILE=.reg.input-file.$$" >> /tmp/reg_sim_remote.$$
echo "export GS_INFILE" >> /tmp/reg_sim_remote.$$
echo "SIM_STD_ERR_FILE=$SIM_STD_ERR_FILE" >> /tmp/reg_sim_remote.$$
echo "export SIM_STD_ERR_FILE" >> /tmp/reg_sim_remote.$$
echo "SIM_STD_OUT_FILE=$SIM_STD_OUT_FILE" >> /tmp/reg_sim_remote.$$
echo "export SIM_STD_OUT_FILE" >> /tmp/reg_sim_remote.$$
echo "REG_SGS_ADDRESS=$REG_SGS_ADDRESS" >> /tmp/reg_sim_remote.$$
echo "export REG_SGS_ADDRESS" >> /tmp/reg_sim_remote.$$

# Really hacky bit to shift GLOBUS_TCP_PORT_RANGE if running on
# local host.  This prevents sockets from self-connecting which
# results in the connection back from vmd to namd failing.
if [ "$SIM_HOSTNAME" == "localhost" ]
then
  echo "echo $GLOBUS_TCP_PORT_RANGE" >> /tmp/reg_sim_remote.$$
  # Tell bash to treat these variables as int's, not strings
  echo "declare -i PORT_MIN" >> /tmp/reg_sim_remote.$$
  echo "declare -i PORT_MAX" >> /tmp/reg_sim_remote.$$
  echo "PORT_MIN=\`echo \$GLOBUS_TCP_PORT_RANGE | awk -F, '{print \$1}'\`" >> /tmp/reg_sim_remote.$$
  echo "PORT_MAX=\`echo \$GLOBUS_TCP_PORT_RANGE | awk -F, '{print \$2}'\`" >> /tmp/reg_sim_remote.$$
  echo "PORT_MIN=\${PORT_MIN}+20" >> /tmp/reg_sim_remote.$$
  echo "PORT_MAX=\${PORT_MAX}+20" >> /tmp/reg_sim_remote.$$
  echo "GLOBUS_TCP_PORT_RANGE=\${PORT_MIN}\",\"\${PORT_MAX}" >> /tmp/reg_sim_remote.$$
  echo "GLOBUS_TCP_PORT_RANGE=\$GLOBUS_TCP_PORT_RANGE" >> /tmp/reg_sim_remote.$$
  echo "export GLOBUS_TCP_PORT_RANGE" >> /tmp/reg_sim_remote.$$
fi

echo "echo \"Starting vmd job...\"" >> /tmp/reg_sim_remote.$$
echo "\$HOME/RealityGrid/bin/start_vmd .reg.input-file.$$" >> /tmp/reg_sim_remote.$$

#  Transfer the input file to target machine

echo "Transferring simulation input file..."


if [ $CHECKPOINT_GSH ]
then 
  #  Transfer the checkpoint data files to target machine
  #   do this by calling Mark McKeown's rgcpc script
  echo "Calling Mark McKeown's rgcpc script on Bezier"
  globus-job-run bezier.man.ac.uk/jobmanager-fork /home/bezier1/globus/bin/rg-cp -vb -p 10 -tcp-bs 16777216 -t gsiftp://$SIM_HOSTNAME/~/RealityGrid/scratch -g $CHECKPOINT_GSH
  echo "Done calling script"
fi

if [ $SSH -eq 0 ]
then
  case $SIM_HOSTNAME in
      iam764.psc.edu)
          # Pittsburgh has a special ftp server - put input file in home directory there....
	  globus-url-copy file:$SIM_INFILE gsiftp://panhead.psc.edu/\~/.reg.input-file.$$
          # ...and then copy it over to LeMieux
	  globus-job-run iam764.psc.edu /usr/psc/bin/far get .reg.input-file.$$ \~/RealityGrid/scratch/.
          ;;
      localhost)
	  cp -f $SIM_INFILE $HOME/RealityGrid/scratch/.reg.input-file.$$
	  ;;
      *)
	  globus-url-copy file:$SIM_INFILE gsiftp://$SIM_HOSTNAME/\~/RealityGrid/scratch/.reg.input-file.$$
	  ;;
  esac
else
  scp $PWD/$SIM_INFILE $SIM_USER@$SIM_HOSTNAME:RealityGrid/scratch/.reg.input-file.$$
fi

if [ $? -gt "0" ]
then
  echo "Error with transferring input file"
fi

echo ""

# Start the simulation using the script created above

echo "Starting simulation..."

# SIM_NODE_HOSTNAME is needed for running on a linux cluster
# viking has a unique script on the machine itself configured to put the job
# into SGE queues
if [ $SSH -eq 0 ]
then
  case $SIM_HOSTNAME in
       viking-i00.viking.lesc.doc.ic.ac.uk)
          SIM_NODE_HOSTNAME=viking000.viking.lesc.doc.ic.ac.uk
          globus-job-run $SIM_HOSTNAME -stdout $SIM_STD_OUT_FILE.initial -stderr $SIM_STD_ERR_FILE.initial -s /tmp/reg_sim_remote.$$ &  
          ;;
       green.cfs.ac.uk)
#          globus-job-run wren.cfs.ac.uk/jobmanager-lsf-green -x "(jobtype=single)(maxWallTime=${TIME_TO_RUN})(queue=testq)" -stdout $SIM_STD_OUT_FILE -stderr $SIM_STD_ERR_FILE -np $SIM_PROCESSORS -s /tmp/reg_sim_remote.$$ &
          globus-job-run wren.cfs.ac.uk/jobmanager-lsf-green -x "(jobtype=single)(maxWallTime=${TIME_TO_RUN})" -stdout $SIM_STD_OUT_FILE -stderr $SIM_STD_ERR_FILE -np $SIM_PROCESSORS -s /tmp/reg_sim_remote.$$ &
          ;;
       fermat.cfs.ac.uk)
          globus-job-run wren.cfs.ac.uk/jobmanager-lsf-fermat -x "(jobtype=single)(maxWallTime=${TIME_TO_RUN})" -stdout $SIM_STD_OUT_FILE -stderr $SIM_STD_ERR_FILE -np $SIM_PROCESSORS -s /tmp/reg_sim_remote.$$ &
          ;;
       wren.cfs.ac.uk)
          globus-job-run wren.cfs.ac.uk/jobmanager-lsf -x "(jobtype=single)(maxWallTime=${TIME_TO_RUN})" -stdout $SIM_STD_OUT_FILE -stderr $SIM_STD_ERR_FILE -np $SIM_PROCESSORS -s /tmp/reg_sim_remote.$$ &
          ;;
       localhost)
	  chmod a+x /tmp/reg_sim_remote.$$
          /tmp/reg_sim_remote.$$ &> ${HOME}/${SIM_STD_ERR_FILE} &	  
	  ;;
       *)
          globus-job-run $SIM_HOSTNAME/jobmanager-fork -x '(jobtype=single)' -stdout $SIM_STD_OUT_FILE -stderr $SIM_STD_ERR_FILE -np $SIM_PROCESSORS -s /tmp/reg_sim_remote.$$ &
          ;;
  esac
else
  case $SIM_HOSTNAME in
       viking-i00.viking.lesc.doc.ic.ac.uk)
          SIM_NODE_HOSTNAME=viking000.viking.lesc.doc.ic.ac.uk
          chmod a+x /tmp/reg_sim_remote.$$
          scp /tmp/reg_sim_remote.$$ $SIM_USER@$SIM_HOSTNAME:/tmp/reg_sim_remote.$$
          ssh -f $SIM_USER@$SIM_HOSTNAME /tmp/reg_sim_remote.$$   
          ;;
       localhost)
	  chmod a+x /tmp/reg_sim_remote.$$
          /tmp/reg_sim_remote.$$ &> ${HOME}/${SIM_STD_ERR_FILE} &	  
	  ;;
       *)
          chmod a+x /tmp/reg_sim_remote.$$
          scp /tmp/reg_sim_remote.$$ $SIM_USER@$SIM_HOSTNAME:/tmp/reg_sim_remote.$$
          ssh -f $SIM_USER@$SIM_HOSTNAME /tmp/reg_sim_remote.$$   
          ;;
  esac
fi

if [ $? -gt "0" ]
then
  echo "Error with starting simulation"
  exit
fi

echo ""
