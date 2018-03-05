
FROM ryjen/cpp-coveralls

ARG CMAKE_DEFINES

RUN apt-get update

RUN apt-get -y install libssl-dev libcurl4-openssl-dev liburiparser-dev

ENV BUILD_SRC /usr/src

COPY . ${BUILD_SRC}

RUN mkdir -p ${BUILD_SRC}/build

WORKDIR ${BUILD_SRC}/build

RUN cmake ${CMAKE_DEFINES} ..

RUN make

CMD "make", "test", "ARGS=-V"
