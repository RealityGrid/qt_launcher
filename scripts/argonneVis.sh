#!/bin/sh

# This script assumes that you want the windows to appear back on the machine you ran it from
# $1 is the maximum amount of time you want in minutes

DISP=$HOSTNAME:0.0
echo $DISP

if [ $# -ne 4 ]; then
    echo "Usage: argonneVis.sh <vis SGS> <time to run> <mcast address> <vis_type: vis|iso|cut>"
    REG_SGS_ADDRESS=
    MAXTIME=
    REG_MCAST_ADDRESS=
    VIS_TYPE=
else
    REG_SGS_ADDRESS=$1
    MAXTIME='(maxtime = '$2')'
    REG_MCAST_ADDRESS=$3
    VIS_TYPE=$4
fi

echo $REG_SGS_ADDRESS
echo $MAXTIME 

# Make things work with nasty hacks!
#export X509_USER_PROXY=/tmp/x509up_u6818
xhost +

# Run the job, depending on which type of visualization we want

if [ $VIS_TYPE = "vol" ]; then
globusrun -o -r "tg-master.uc.teragrid.org/jobmanager-pbs_gcc" "&(count=1)(host_count="5:activemural")${MAXTIME}(executable=/home/zzcgurh/RealityGrid/ReG-vis-scripts/globus_spawn4.sh)(arguments=/home/zzcgurh/RealityGrid/ReG-vis-scripts/online_vol4.conf)(environment = (DISPLAY ${DISP}) (REG_SGS_ADDRESS ${REG_SGS_ADDRESS}) (REG_MCAST_ADDRESS ${REG_MCAST_ADDRESS}))"
else

if [ $VIS_TYPE = "iso" ]; then
globusrun -o -r "tg-master.uc.teragrid.org/jobmanager-pbs_gcc" "&(count=1)(host_count="5:activemural")${MAXTIME}(executable=/home/zzcgurh/RealityGrid/ReG-vis-scripts/globus_spawn4.sh)(arguments=/home/zzcgurh/RealityGrid/ReG-vis-scripts/online_iso4.conf)(environment = (DISPLAY ${DISP}) (REG_SGS_ADDRESS ${REG_SGS_ADDRESS}) (REG_MCAST_ADDRESS ${REG_MCAST_ADDRESS}))"
else

globusrun -o -r "tg-master.uc.teragrid.org/jobmanager-pbs_gcc" "&(count=1)(host_count="5:activemural")${MAXTIME}(executable=/home/zzcgurh/RealityGrid/ReG-vis-scripts/globus_spawn4.sh)(arguments=/home/zzcgurh/RealityGrid/ReG-vis-scripts/online_cut4.conf)(environment = (DISPLAY ${DISP}) (REG_SGS_ADDRESS ${REG_SGS_ADDRESS}) (REG_MCAST_ADDRESS ${REG_MCAST_ADDRESS}))"

fi

fi

#globusrun -o -r "tg-master.uc.teragrid.org/jobmanager-pbs_gcc" "&(count=1)(host_count="10:activemural")(${MAXTIME})(executable=/home/zzcgurh/RealityGrid/ReG-vis-scripts/globus_spawn.sh)(arguments=/home/zzcgurh/RealityGrid/ReG-vis-scripts/online_iso.conf)(environment = (DISPLAY ${DISP}) (REG_SGS_ADDRESS http://tg-login1.ncsa.teragrid.org:50122/SGS/service/10431217101031320096042))"

#globusrun -o -r "tg-master.uc.teragrid.org/jobmanager-pbs_gcc" "&(count=1)(host_count="10:activemural")${MAXTIME}(executable=/home/zzcgurh/RealityGrid/ReG-vis-scripts/globus_spawn.sh)(arguments=/home/zzcgurh/RealityGrid/ReG-vis-scripts/lb3d_cut.conf)(environment = (DISPLAY ${DISP}))"
