# FROM python:3.8.10-slim-buster

FROM ubuntu:20.04 AS base

SHELL [ "/bin/bash", "-c" ]

RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive TZ="Europe/Prague" \
    apt-get install -yqq --no-install-recommends \
    tzdata \
    ca-certificates \
    xz-utils \
    wget \
    vim \
    make \
    gcc \
    g++ \
    xsltproc \
    curl \
    python3-pip \
    python3-distutils && \
    rm -rf /var/lib/apt/lists/* && \
    update-ca-certificates

# CMAKE
ARG VERSION=3.23.2
ARG APP_PATH=/usr
WORKDIR /downloads
RUN wget -q https://github.com/Kitware/CMake/releases/download/v${VERSION}/cmake-${VERSION}-linux-x86_64.tar.gz
RUN tar -C ${APP_PATH} --strip-components=1 -xzf cmake-${VERSION}-linux-x86_64.tar.gz > /dev/null

COPY ts-python-env /usr/ts-python-env
COPY ts-spect-compiler /spect/ts-spect-compiler
COPY ts-spect-fw /spect/ts-spect-fw

WORKDIR /spect/ts-spect-compiler
RUN ./build.sh --clean >> /dev/null


WORKDIR /spect

# FROM ubuntu:20.04 AS final
FROM alpine AS final

# SHELL [ "/bin/bash", "-c" ]

COPY --from=base /spect /spect
# ENV PATH="${PATH}:/spect/ts-spect-compiler/build/src/apps/"
# ENV TS_REPO_ROOT="/spect/ts-spect-fw"

COPY --from=base /usr /usr

# WORKDIR /usr/ts-python-env
# RUN apt-get update -qq && apt-get install -yqq ca-certificates && update-ca-certificates
# RUN source ./install_virtual_environment.sh lab && echo "source $(poetry env info --path)/bin/activate" >> /root/.profile && echo "source $(poetry env info --path)/bin/activate" >> /root/.bashrc


# WORKDIR /spect/ts-spect-fw

LABEL org.opencontainers.image.source https://github.com/tropicsquare/ts-spect-compiler
LABEL org.opencontainers.image.description Tropic Square SPECT compiler
