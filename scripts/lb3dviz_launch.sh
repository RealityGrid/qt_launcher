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

# Read in config file

#. $PWD/ReG-L2.conf
#. $PWD/ReG-L2-GUI.conf

echo $1
. $1

# Optionally get the multicast address we're to use
#if [ $# -eq 2 ]
#then
#MULTICAST_ADDRESS=$2
#else
#MULTICAST_ADDRESS=""
#fi

# Ascertain whether we have a valid grid-proxy 

if [ $SSH -eq 0 ]
then
  echo "Using Globus to launch"
  $GLOBUS_LOCATION/bin/grid-proxy-info -e
  if [ $? -ne "0" ]
  then
     echo "No grid proxy, please invoke grid-proxy-init"
     exit
  fi
else
  echo "Using SSH to launch"
fi

# Initialize Vizserver if required

if [[ $VIZ_TYPE = "viz_iso" || $VIZ_TYPE = "viz_vol" || $VIZ_TYPE = "viz_cut" ]]
then
   echo "Launching SGI Vizserver client..."
   echo ""
   xhost + $VIZ_HOSTNAME
   echo ""
   vizserver -h $VIZ_HOSTNAME &
fi

# Setup xhost access to allow steerer display

xhost + $SIM_HOSTNAME
xhost + $VIZ_HOSTNAME
xhost + 

#  Start visualisation 

echo "Starting viz"

# Fixed in launcher code instead
#REG_SGS_ADDRESS=$REG_VIS_GSH
export REG_SGS_ADDRESS

   echo "#!/bin/sh" > /tmp/reg_viz_remote.$$
   echo ". \$HOME/RealityGrid/etc/reg-user-env.sh" >> /tmp/reg_viz_remote.$$
   echo "SSH=$SSH" >> /tmp/reg_viz_remote.$$
   echo "export SSH" >> /tmp/reg_viz_remote.$$
   echo "VIZ_STD_ERR_FILE=$VIZ_STD_ERR_FILE" >> /tmp/reg_viz_remote.$$
   echo "export VIZ_STD_ERR_FILE" >> /tmp/reg_viz_remote.$$
   echo "VIZ_STD_OUT_FILE=$VIZ_STD_OUT_FILE" >> /tmp/reg_viz_remote.$$
   echo "export VIZ_STD_OUT_FILE" >> /tmp/reg_viz_remote.$$
   echo "if [ ! -d \$HOME/RealityGrid/scratch/ReG_workdir_viz$$ ]" >> /tmp/reg_viz_remote.$$
   echo "then" >> /tmp/reg_viz_remote.$$
   echo "   mkdir \$HOME/RealityGrid/scratch/ReG_workdir_viz$$" >> /tmp/reg_viz_remote.$$
   echo "fi" >> /tmp/reg_viz_remote.$$
   echo "REG_WORKING_DIR=\$HOME/RealityGrid/scratch/ReG_workdir_viz$$" >> /tmp/reg_viz_remote.$$
   echo "export REG_WORKING_DIR" >> /tmp/reg_viz_remote.$$
   echo "REG_STEER_DIRECTORY=\$REG_WORKING_DIR" >> /tmp/reg_viz_remote.$$
   echo "export REG_STEER_DIRECTORY" >> /tmp/reg_viz_remote.$$
   echo "cd \$REG_WORKING_DIR" >> /tmp/reg_viz_remote.$$
   echo "echo \"Starting script\"" >> /tmp/reg_viz_remote.$$
   echo "DISPLAY=${CLIENT_DISPLAY}" >> /tmp/reg_viz_remote.$$
   echo "export DISPLAY" >> /tmp/reg_viz_remote.$$
   echo "UC_PROCESSORS=$VIZ_PROCESSORS" >> /tmp/reg_viz_remote.$$
   echo "export UC_PROCESSORS" >> /tmp/reg_viz_remote.$$
   echo "VIZ_TYPE=$VIZ_TYPE" >> /tmp/reg_viz_remote.$$
   echo "export VIZ_TYPE" >> /tmp/reg_viz_remote.$$
   echo "echo \$PATH" >> /tmp/reg_viz_remote.$$
   if [ $SIM_HOSTNAME = viking-i00.viking.lesc.doc.ic.ac.uk ]
   then
      echo "REG_CONNECTOR_HOSTNAME=$SIM_NODE_HOSTNAME" >> /tmp/reg_viz_remote.$$
   else
      echo "REG_CONNECTOR_HOSTNAME=$SIM_HOSTNAME" >> /tmp/reg_viz_remote.$$
   fi
   echo "export REG_CONNECTOR_HOSTNAME" >> /tmp/reg_viz_remote.$$
   echo "REG_SGS_ADDRESS=$REG_SGS_ADDRESS" >> /tmp/reg_viz_remote.$$
   echo "export REG_SGS_ADDRESS" >> /tmp/reg_viz_remote.$$
if [ $MULTICAST_ADDRESS ]
then 
   echo "\$HOME/RealityGrid/ReG-vol-viewer-sockets/vis_l2g $VIZ_TYPE $MULTICAST_ADDRESS" >> /tmp/reg_viz_remote.$$
else
   echo "\$HOME/RealityGrid/ReG-vol-viewer-sockets/vis_l2g $VIZ_TYPE" >> /tmp/reg_viz_remote.$$
fi

   # Submit job into lsf batch system on bezier
   #   globus-job-run $VIZ_HOSTNAME/jobmanager-lsf -x "(jobtype=single)" -stderr $VIZ_STD_ERR_FILE -stdout $VIZ_STD_OUT_FILE -np $VIZ_PROCESSORS -s /tmp/reg_viz_remote.$$ &
#   globus-job-run $VIZ_HOSTNAME/jobmanager-fork -stderr $VIZ_STD_ERR_FILE -stdout $VIZ_STD_OUT_FILE -s /tmp/reg_viz_remote.$$ &

if [ $SSH = 0 ]
then
     case $VIZ_HOSTNAME in
       bezier.man.ac.uk)
   globus-job-run $VIZ_HOSTNAME/jobmanager-fork -stderr $VIZ_STD_ERR_FILE -stdout $VIZ_STD_OUT_FILE -s /tmp/reg_viz_remote.$$ &
          ;;
       *)
	  globus-job-run $VIZ_HOSTNAME -stderr $VIZ_STD_ERR_FILE -stdout $VIZ_STD_OUT_FILE -s /tmp/reg_viz_remote.$$ &
	  ;;
     esac
else
    chmod a+x /tmp/reg_viz_remote.$$
    scp /tmp/reg_viz_remote.$$ $VIZ_USER@$VIZ_HOSTNAME:/tmp/reg_viz_remote.$$
    ssh -f $VIZ_USER@$VIZ_HOSTNAME /tmp/reg_viz_remote.$$ 
fi

   if [ $? -gt "0" ]
   then
     echo "Error with starting viz helper"
   fi
