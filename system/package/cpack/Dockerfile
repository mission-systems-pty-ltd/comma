ARG version=24.04
FROM ubuntu:${version}
ENV DEBIAN_FRONTEND="noninteractive"
ARG version  # Re-declare for use in RUN.
RUN apt-get update \
    && PACKAGES="build-essential cmake devscripts libboost-all-dev" \
    && case "$version" in \
          "20.04"|"22.04") PACKAGES="$PACKAGES libprocps-dev" ;; \
          "24.04") PACKAGES="$PACKAGES libproc2-dev" ;; \
       esac \
    && apt-get install -y --no-install-recommends $PACKAGES \
    && rm -rf /var/lib/apt/lists/*
