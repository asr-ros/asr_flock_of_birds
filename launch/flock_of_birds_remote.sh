#!/bin/bash
export ROS_MASTER_URI="http://$HOSTNAME:11311"
export ROSLAUNCH_SSH_UNKNOWN=1
roslaunch asr_flock_of_birds flock_of_birds_remote.launch
