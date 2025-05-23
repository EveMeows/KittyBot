FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update -y && apt upgrade -y && apt install -y \
  build-essential git ninja-build gcc g++ cmake curl unzip zip ca-certificates bison flex autoconf pkg-config linux-libc-dev \
  && rm -rf /var/lib/apt/lists/*

# Setup VCPKG
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg
RUN /opt/vcpkg/bootstrap-vcpkg.sh

ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH=$VCPKG_ROOT:$PATH

# Build and run project.
WORKDIR /app
COPY . .

RUN mkdir build
WORKDIR /app/build

RUN cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
RUN cmake --build . --parallel

FROM ubuntu:24.04 AS runtime

RUN apt update -y && apt install -y \
  ca-certificates \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Get wait-for
RUN curl https://raw.githubusercontent.com/eficode/wait-for/v2.2.3/wait-for -o /app/wait-for.sh
RUN chmod +x /app/wait-for.sh

COPY --from=builder /app/build /app/build
CMD ["/app/wait-for.sh", "postgres:5432", "--", "/app/build/KittyBot"]
