FROM secretchase/buildbase:ubuntu18.04_gcc7.5_cmake3.12_boost1.72_cpp17

ADD mlnserver /usr/src/app/mlnserver

RUN mkdir build && cd build && git clone -b master --single-branch https://github.com/chaseYLC/MLN.git \
&& cd MLN \
&& cmake -DCMAKE_BUILD_TYPE=Debug CMakeLists.txt \
&& make -j$(nproc)

WORKDIR /usr/src/app/mlnserver


RUN cp /build/MLN/mlnserver/bin/mlnserver /usr/src/app/mlnserver/mlnserver

#CMD ["/bin/bash"]
CMD ["./mlnserver"]


