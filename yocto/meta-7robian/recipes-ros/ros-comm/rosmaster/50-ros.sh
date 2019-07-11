export ROS_DISTRO=indigo
export ROS_ROOT=/opt/ros/$ROS_DISTRO
export ROS_PACKAGE_PATH=$ROS_ROOT/share
export PATH=$PATH:$ROS_ROOT/bin
export LD_LIBRARY_PATH=$ROS_ROOT/lib
export PYTHONPATH=$ROS_ROOT/lib/python2.7/site-packages
export ROS_MASTER_URI=http://localhost:11311
# this is required at runtime by ros!
export CMAKE_PREFIX_PATH=/opt/ros/indigo

