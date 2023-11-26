FROM ubuntu:22.04

WORKDIR /app

ADD . /app/src

RUN apt-get update \
    && apt-get install -y apt-utils

RUN export DEBIAN_FRONTEND=noninteractive \
    && export DEBCONF_NONINTERACTIVE_SEEN=true \
    && echo tzdata tzdata/Areas select Europe > preseed.txt \
    && echo tzdata tzdata/Zones/Europe select Moscow >> preseed.txt \
    && debconf-set-selections preseed.txt \
    && ln -fs /usr/share/zoneinfo/Europe/Moscow /etc/localtime \
    && apt-get install -y tzdata \
    && dpkg-reconfigure --frontend noninteractive tzdata

RUN apt-get install -y \
	debhelper g++ gcc cmake libavformat-dev libavcodec-dev libavdevice-dev \
	libswresample-dev libavutil-dev libswscale-dev libssl-dev libnss3-dev libsrtp2-dev \
	build-essential libboost-dev libboost-system-dev libboost-filesystem-dev \
	libopencv-dev libfreetype-dev libharfbuzz-dev libx11-dev libwebrtc-audio-processing-dev \
	libvncserver-dev pkg-config devscripts

RUN mkdir -p build \
	&& cd build \
	&& cmake ../src \
	&& cmake --build .

