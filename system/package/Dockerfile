ARG version
FROM ubuntu:${version}
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt update \
    && apt install --yes apt-utils \
    && apt install --yes software-properties-common \
    && apt install --yes ansible git build-essential cmake cmake-curses-gui python3-pip
RUN apt install --yes python3-stdeb
RUN apt install --yes python3-all
RUN apt install --yes dh-python

# && apt-add-repository --yes --update ppa:ansible/ansible \
