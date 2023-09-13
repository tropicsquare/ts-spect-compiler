FROM ubuntu:22.04 AS base

# SHELL [ "/bin/bash", "-c" ]

RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive TZ="Europe/Prague" \
    apt-get install -yqq --no-install-recommends \
    tzdata \
    ca-certificates \
    make \
    g++ \
    xsltproc \
    python3-pip \
    cmake && \
    rm -rf /var/lib/apt/lists/* && \
    update-ca-certificates

RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir Jinja2

LABEL org.opencontainers.image.source https://github.com/tropicsquare/ts-spect-compiler
LABEL org.opencontainers.image.description Tropic Square SPECT compiler environment