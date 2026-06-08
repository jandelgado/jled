# A minimal dockerfile to provide a build environment to compile the JLed
# raspberry pi pico JLed in a docker container. 
FROM ubuntu:20.04

LABEL MAINTAINER "Jan Delgado <jdelgado@gmx.net>"

ARG TZ=Europe/Berlin
ENV DEBIAN_FRONTEND=noninteractive

RUN    echo ${TZ} > /etc/timezone && rm -f /etc/localtime \
    && cat /etc/timezone\
    && apt-get update \
    && apt-get install -y git cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
                          build-essential vim python3 python3-pip

RUN mkdir /pico
WORKDIR /pico

# install SDK
RUN    git clone --depth=1 -b master https://github.com/raspberrypi/pico-sdk.git \
    && cd pico-sdk && git submodule update --init 

ENV PICO_SDK_PATH=/pico/pico-sdk

