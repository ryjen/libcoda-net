
FROM ubuntu:14.04

RUN apt-get -y install software-properties-common && add-apt-repository ppa:george-edison55/cmake-3.x && add-apt-repository ppa:ubuntu-toolchain-r/test && apt-get update

RUN apt-get -y install --fix-missing make gcc-5 g++-5 gdb cmake cmake-data valgrind lcov libcurl4-openssl-dev vim

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 60 --slave /usr/bin/g++ g++ /usr/bin/g++-5

ENV BUILD_SRC /usr/src/arg3net

WORKDIR ${BUILD_SRC}

RUN mkdir -p build cmake libs src tests

COPY cmake/ cmake/
COPY src/ src/
COPY tests/ tests/
COPY CMakeLists.txt .

WORKDIR ${BUILD_SRC}/build

RUN cmake -DCMAKE_BUILD_TYPE=Debug -DMEMORY_CHECK=ON -DWITH_CURL=ON ..

RUN make

CMD "make", "test", "ARGS=-V"
