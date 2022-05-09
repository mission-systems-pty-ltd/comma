ARG version
FROM ubuntu:${version}
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update \
    && apt-get install --yes apt-utils \
    && apt-get install --yes software-properties-common \
    && apt-get install --yes ansible git build-essential cmake cmake-curses-gui python3-pip devscripts \
    && apt-get install --yes python3-stdeb python3-all dh-python
    && apt-get install --yes libboost-all-dev  libzmq3-dev libprocps-dev libc-dev socat recode
# && apt-add-repository --yes --update ppa:ansible/ansible \
