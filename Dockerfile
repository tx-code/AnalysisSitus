FROM ubuntu:latest

###############################################################################
#
# - OpenCascade:    this is the geometric modeling kernel which provides the
#                   essential services such as B-rep modeling & shape
#                   interrogation, data exchange, shape healing, etc.
#                   https://github.com/Open-Cascade-SAS/OCCT
#
# - Analysis Situs: the open-source CAD platform providing the feature recognition
#                   services, data model, VTK-based visualization services for
#                   CAD models, and GUI/scripting prototyping framework.
#                   https://gitlab.com/ssv/AnalysisSitus
#
# - Eigen:          linear algebra, vectors, matrices.
#                   https://eigen.tuxfamily.org/index.php?title=Main_Page
#
# - Rapidjson:      output to JSON and export to glTF (Analysis Situs).
#                   https://rapidjson.org
#
# Ex. to build:
# > docker build --pull --rm -f "Dockerfile" -t AS:latest "." --no-cache
###############################################################################

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update

# Build tools.
RUN apt-get -y install build-essential git cmake

# 3-rd parties for OCCT
RUN apt-get -y install tcl tcl-dev tk tk-dev libfreeimage-dev
RUN apt-get -y install libxmu-dev libxi-dev
RUN apt-get -y install libosmesa6-dev

# Xvfb provides an X server that can run on machines with no
# display hardware and no physical input devices. It emulates a
# dumb framebuffer using virtual memory.
RUN apt-get -y install xvfb

# Extra 3-rd parties for Analysis Situs
RUN apt-get -y install libeigen3-dev rapidjson-dev

# OpenCascade
RUN git clone https://github.com/Open-Cascade-SAS/OCCT.git opencascade
WORKDIR /opencascade
RUN git checkout V7_4_0 -b AS
RUN mkdir -p build
WORKDIR /opencascade/build
RUN cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_RPATH="" \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DUSE_FREEIMAGE=ON \
  -DUSE_FFMPEG=OFF \
  -DUSE_VTK=OFF \
  -DUSE_TBB=OFF
RUN make
RUN make install

# Copy sources of AS
COPY cmake          /as/cmake
COPY src            /as/src
COPY data           /as/data
COPY CMakeLists.txt /as

WORKDIR /as/
RUN mkdir -p build
WORKDIR /as/build

# Analysis Situs
RUN cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_INSTALL_RPATH="" \
  -DINSTALL_DIR=/usr/bin/analysissitus \
  -DDISTRIBUTION_TYPE=Algo \
  -D3RDPARTY_DIR=/usr/lib \
  -D3RDPARTY_OCCT_INCLUDE_DIR=/usr/include/opencascade \
  -D3RDPARTY_OCCT_LIBRARY_DIR=/usr/lib \
  -D3RDPARTY_EIGEN_DIR=/usr/include/eigen3/ \
  -DUSE_MOBIUS=off \
  -DUSE_INSTANT_MESHES=off \
  -DUSE_RAPIDJSON=on \
  -DUSE_NETGEN=off \
  -DUSE_THREADING=off \
  -D3RDPARTY_DIR=/usr
RUN make
RUN make install