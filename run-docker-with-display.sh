#!/bin/bash

############################################################
# Help                                                     #
############################################################
Help()
{
   # Display Help
   echo "This script can be used to extend 'sudo docker run' commands such that the created container allows passing any visual output to the host display."
   echo
   echo "Usage: $0 <docker run arguments>"
   echo
   echo "Example usage: $0 -it analysis-situs:latest"
   echo
}


############################################################
# Main program                                             #
############################################################
# If no arguments are provided, print the help and exit.
if [ $# -eq 0 ]; then
  Help
  exit 1
fi

# Otherwise perform a `sudo docker run` command with the given arguments
xhost +local:docker
XSOCK=/tmp/.X11-unix
XAUTH=/tmp/.docker.xauth
xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -
sudo docker run -e DISPLAY=$DISPLAY -v $XSOCK:$XSOCK -v $XAUTH:$XAUTH -e XAUTHORITY=$XAUTH --privileged $@
xhost -local:docker
