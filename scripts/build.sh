# openvino预编译库的路径
LITE_DIR=/home/pi/wsy/Paddle-Lite/build.lite.armlinux.armv7hf.gcc/inference_lite_lib.armlinux.armv7hf/cxx
# gflags预编译库的路径
GFLAGS_DIR=/home/pi/wsy/gflags/build

# opencv预编译库的路径, 如果使用自带预编译版本可不修改
OPENCV_DIR=$INTEL_OPENVINO_DIR/opencv
# 下载自带预编译版本
#sh $(pwd)/scripts/bootstrap.sh

rm -rf build
mkdir -p build
cd build
cmake .. \
    -DOPENCV_DIR=${OPENCV_DIR} \
    -DGFLAGS_DIR=${GFLAGS_DIR} \
    -DLITE_DIR=${LITE_DIR} \
    -DCMAKE_CXX_FLAGS="-march=armv7-a"  
make
