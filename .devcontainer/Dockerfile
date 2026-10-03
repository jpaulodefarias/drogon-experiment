FROM docker.swagger.io/swaggerapi/swagger-ui:v5.32.11 AS swagger-ui-assets

FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        clang \
        clang-format \
        clang-tidy \
        libclang-rt-18-dev \
        cmake \
        curl \
        gdb \
        git \
        libfreetype6 \
        libpq-dev \
        libxext6 \
        libxi6 \
        libxrender1 \
        libxtst6 \
        ninja-build \
        pkg-config \
        procps \
        postgresql-client \
        python3-venv \
        unzip \
    && rm -rf /var/lib/apt/lists/* \
    && python3 -m venv /opt/conan-venv \
    && /opt/conan-venv/bin/pip install --no-cache-dir 'conan>=2,<3' \
    && mkdir -p /home/ubuntu/.conan2 \
    && chown ubuntu:ubuntu /home/ubuntu/.conan2

ENV PATH="/opt/conan-venv/bin:${PATH}"

COPY --from=swagger-ui-assets /usr/share/nginx/html/ /opt/swagger-ui/

USER ubuntu
