FROM docker.swagger.io/swaggerapi/swagger-ui:v5.32.11 AS swagger-ui-assets

FROM ubuntu:24.04
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential ca-certificates clang cmake curl git libpq-dev ninja-build \
    postgresql-client python3-venv \
    && rm -rf /var/lib/apt/lists/* \
    && python3 -m venv /opt/conan-venv \
    && /opt/conan-venv/bin/pip install --no-cache-dir 'conan>=2,<3'
ENV PATH="/opt/conan-venv/bin:${PATH}"
ENV CC=clang CXX=clang++
COPY --from=swagger-ui-assets /usr/share/nginx/html/ /opt/swagger-ui/
WORKDIR /app
COPY conanfile.py ./
RUN conan profile detect --force \
    && conan install . --build=missing -s build_type=Release
COPY . .
RUN cmake -S . -B build/Release \
        -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && cmake --build build/Release --parallel
ENV HTTP_PORT=8080
EXPOSE 8080
CMD ["/app/build/Release/api"]
