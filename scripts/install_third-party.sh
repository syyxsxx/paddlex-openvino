# download third-part lib
if [ ! -d "./deps" ]; then
    mkdir deps
fi
if [ ! -d "./deps/gflag" ]; then
    cd deps
    git clone https://github.com/gflags/gflags
    cd gflags
    cmake .
    make -j 8
    cd ..
    cd ..
fi
if [ ! -d "./deps/glog" ]; then
    cd deps
    git clone https://github.com/google/glog
    sudo apt-get install autoconf automake libtool
    cd glog
    ./autogen.sh
    ./configure
    make -j 8
    cd ..
    cd ..
fi
OPENCV_URL=https://paddleseg.bj.bcebos.com/deploy/docker/opencv3gcc4.8.tar.bz2
if [ ! -d "./deps/opencv3gcc4.8" ]; then
    cd deps
    wget -c ${OPENCV_URL}
    tar xvfj opencv3gcc4.8.tar.bz2
    rm -rf opencv3gcc4.8.tar.bz2
    cd ..
fi
