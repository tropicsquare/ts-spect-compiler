ARG AARCH=amd64
FROM ${AARCH}/ubuntu:22.04 as base

SHELL [ "/bin/bash", "-c" ]

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

# docker build -t spect . -f Dockerfiles/Dockerfile
# docker run --rm -v $(pwd):/app -w /app spect ./build.sh --clean