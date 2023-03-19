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


# INSTALL QT5:
RUN \
    set -eux && \
    apt-get update && \
    apt-get -y upgrade && \
    apt-get -y install \
        aptitude apt-rdepends bash ccache clang clang-tidy cmake cppcheck curl doxygen diffstat gawk gdb git gnupg gperf iputils-ping \
        libboost-all-dev libfcgi-dev libgfortran5 libgl1-mesa-dev libjemalloc-dev libjemalloc2 libmlpack-dev libtbb-dev libyaml-cpp-dev \
        linux-tools-generic nano nasm ninja-build openjdk-11-jdk openssh-server openssl pkg-config python3 qt5-default spawn-fcgi \
        sudo tini unzip valgrind wget zip texinfo gcc-multilib chrpath socat cpio xz-utils debianutils libegl1-mesa \
        patch perl tar rsync bc libelf-dev libssl-dev libsdl1.2-dev xterm mesa-common-dev whois software-properties-common \
        libx11-xcb-dev libxcb-dri3-dev libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-render-util0-dev \
        libxcb-render0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-util-dev libxcb-xfixes0-dev libxcb-xinerama0-dev libxcb-xkb-dev xorg-dev \
        libconfuse-dev libnl-3-dev libnl-route-3-dev libncurses-dev dh-autoreconf freeglut3 freeglut3-dev libglfw3-dev \
        apt-transport-https g++ graphviz xdot golang-go && \
    curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > /etc/apt/trusted.gpg.d/bazel.gpg && \
    echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list && \
    apt-get update && \
    apt-get -y install bazel && \
    bazel --version && \
    wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.0-1_all.deb && \
    sudo dpkg -i cuda-keyring_1.0-1_all.deb && \
    wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin && \
    mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600 && \
    apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/7fa2af80.pub && \
    add-apt-repository "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/ /" && \
    apt-get update && \
    apt-get -y install cuda && \
    apt-get -y autoremove && \
    apt-get -y autoclean && \
    apt-get -y clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    exit 0
# Install python pip
RUN \
    set -eux && \
    python3 --version && \
    curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py && \
    python3 get-pip.py && \
    rm get-pip.py && \
    python3 -m pip install -U pip && \
    pip3 --version && \
    pip3 install --upgrade pip setuptools wheel && \
    pip3 --version && \
    exit 0
# Install python pip packages
RUN \
    set -eux && \
    pip3 --version && \
    pip3 install --upgrade pip setuptools wheel && \
    pip3 --version && \
    pip3 install --upgrade autoenv autopep8 cmake-format clang-format conan conan_package_tools meson && \
    pip3 install --upgrade cppclean flawfinder lizard pygments pybind11 GitPython pexpect subunit Jinja2 pylint CLinters && \
    pip3 install --upgrade ipython jupyter matplotlib nose numba numpy pandas pymc3 PyWavelets requests scikit-learn scipy seaborn sympy quandl textblob nltk yfinance && \
    exit 0
RUN pip3 install --upgrade PyPortfolioOpt
RUN pip3 install --upgrade dlib -vvv
RUN pip3 install --upgrade frida frida-tools
RUN pip3 install --upgrade vaex
#RUN pip3 install --upgrade --ignore-installed cltk
# Install FB Prophet
# https://github.com/facebook/prophet/blob/master/python/requirements.txt
RUN \
    pip3 install --upgrade Cython cmdstanpy==0.9.68 pystan~=2.19.1.1 numpy pandas matplotlib LunarCalendar convertdate holidays setuptools-git python-dateutil tqdm && \
    pip3 install --upgrade fbprophet && \
    exit 0
# Install conan
RUN \
    set -eux && \
    conan profile new default --detect  && \
    conan profile update settings.compiler.libcxx=libstdc++11 default && \
    conan remote list && \
    conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan && \
    exit 0
# Install buildifier
Run go get github.com/bazelbuild/buildtools/buildifier
# Setup ssh
RUN \
    set -eux && \
    mkdir -p /var/run/sshd && \
    mkdir -p /root/.ssh && \
    sed -ri 's/^#?PermitRootLogin\s+.*/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -ri 's/UsePAM yes/#UsePAM yes/g' /etc/ssh/sshd_config && \
    groupadd -g 1000 myuser && \
    useradd --system --no-log-init --create-home --home-dir /home/myuser --gid myuser --groups sudo --uid 1000 --shell /bin/bash myuser && \
    echo 'root:root' | chpasswd && \
    echo 'myuser:myuser' | chpasswd && \
    ssh-keygen -A && \
    exit 0
RUN sudo apt-get update -y
RUN sudo apt-get install -y qttools5-dev-tools
ENV IGNORE_CC_MISMATCH=1
ENV PATH=$PATH:/usr/local/cuda/bin
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/lib64
ENTRYPOINT ["/usr/bin/tini", "--"]
CMD ["/usr/sbin/sshd", "-D", "-e"]
RUN rm /bin/sh && ln -s /bin/bash /bin/sh
ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8
RUN sed -i 's/ universe/ universe multiverse/' /etc/apt/sources.list
RUN apt update &&                  \
    apt upgrade -y &&              \
    apt dist-upgrade -y &&         \
    apt install -y                 \
        xvfb                       \
        flex                       \
        dh-make                    \
        debhelper                  \
        checkinstall               \
        fuse                       \
        bison                      \
        libxcursor-dev             \
        libxcomposite-dev          \
        libxcb1-dev                \
        libx11-dev                 \
        libudev-dev                \
        qtbase5-private-dev      &&\
    apt clean


# Installing QT5
RUN apt-get install libqt5x11extras5-dev qt5-default -y
RUN sudo apt install mesa-utils -y


COPY install_vtk.sh /home


COPY VTK-8.2.0-install /home/vtk-install
COPY tcltk-86-64 /home/tcl-install
COPY freetype-2.5.5-vc14-64 /home/freetype-install


# Analysis Situs
RUN cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DCMAKE_INSTALL_RPATH="" \
  -DINSTALL_DIR=/usr/local/bin/analysissitus \
  -DDISTRIBUTION_TYPE=Complete \
  -D3RDPARTY_DIR=/usr/lib \
  -D3RDPARTY_OCCT_INCLUDE_DIR=/usr/include/opencascade \
  -D3RDPARTY_OCCT_LIBRARY_DIR=/usr/lib \
  -D3RDPARTY_EIGEN_DIR=/usr/include/eigen3/ \
  -D3RDPARTY_QT_DIR_NAME=/usr/lib/qt5/ \
  -D3RDPARTY_tcl_DIR=/usr \
  -D3RDPARTY_vtk_DIR=/usr \
  -D3RDPARTY_freetype_DIR=/usr \
  -DUSE_MOBIUS=off \
  -DUSE_INSTANT_MESHES=off \
  -DUSE_RAPIDJSON=off \
  -DUSE_NETGEN=off \
  -DUSE_THREADING=off \
  -D3RDPARTY_DIR=/usr/lib
RUN make
RUN make install
