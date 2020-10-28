ARG version
FROM ubuntu:${version}
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt update \
    && apt install software-properties-common --yes \
    && apt install --yes ansible git build-essential cmake cmake-curses-gui

# && apt-add-repository --yes --update ppa:ansible/ansible \
