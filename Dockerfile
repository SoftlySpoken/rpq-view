FROM ubuntu:20.04 AS builder

LABEL vendor="pkumod"
LABEL description="rpq-view"
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo '$TZ' > /etc/timezone

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
	cmake \
    python3 \
    libtbb-dev \
    pkg-config \
    uuid-dev

RUN export PATH=$PATH:/usr/local/bin
RUN export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
RUN export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
RUN export CPATH=/usr/local/include:$CPATH
RUN ldconfig

RUN mkdir -p /src
RUN mkdir -p /data

WORKDIR /usr/src/

# Copy source code
ADD rpq-view.tar.gz /usr/src/

RUN ln -s /usr/data /usr/src/rpq-view/real_data

RUN cd /usr/src/rpq-view && \
    mkdir lib && \
    tar -xzvf antlr4-cpp-runtime-4.tar.gz && \
    cd antlr4-cpp-runtime-4/ && \
    cmake . && \
    make -j && \
    cp dist/libantlr4-runtime.a ../lib/

RUN cd /usr/src/rpq-view/third_party && \
    tar -xzvf googletest.tar.gz

RUN cd /usr/src/rpq-view/ && \
    cmake -S . -B build && \
    cmake --build build -j

