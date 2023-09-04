#! docker

ARG AARCH=amd64
FROM ${AARCH}/ubuntu:latest as base

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

RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir Jinja2

# CMAKE
ARG VERSION=3.23.2
ARG APP_PATH=/usr
WORKDIR /downloads
RUN wget -q https://github.com/Kitware/CMake/releases/download/v${VERSION}/cmake-${VERSION}-linux-x86_64.tar.gz
RUN tar -C ${APP_PATH} --strip-components=1 -xzf cmake-${VERSION}-linux-x86_64.tar.gz > /dev/null

COPY ts-spect-fw /spect/ts-spect-fw
COPY ../ /spect/ts-spect-compiler

WORKDIR /spect/ts-spect-compiler
RUN ./build.sh --clean >> /dev/null

ENV PATH="${PATH}:/spect/ts-spect-compiler/build/src/apps/"
ENV TS_REPO_ROOT="/spect/ts-spect-fw"

WORKDIR /spect/ts-spect-fw

LABEL org.opencontainers.image.source https://github.com/tropicsquare/ts-spect-compiler
LABEL org.opencontainers.image.description Tropic Square SPECT compiler
