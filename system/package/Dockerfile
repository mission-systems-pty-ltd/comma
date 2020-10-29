ARG version
FROM ubuntu:${version}
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt update \
    && apt install software-properties-common --yes \
    && apt install --yes ansible git build-essential cmake cmake-curses-gui python3-pip
RUN apt install --yes python3-stdeb

# && apt-add-repository --yes --update ppa:ansible/ansible \
