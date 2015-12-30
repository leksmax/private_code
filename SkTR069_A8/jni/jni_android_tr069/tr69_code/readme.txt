bevent移植
export PATH=/opt/hisi-linux/x86-arm/gcc-3.4.3-uClibc-0.9.28/bin:$PATH
./configure --prefix=/home/kangzh/test/libevent --host=arm-linux CC=arm-hisi-linux-gcc CXX=arm-hisi-linux-g++
make 
make install