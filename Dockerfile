FROM alpine:latest as builder

RUN apk update && apk add build-base ninja meson git wget bash curl openssl openssl-dev

# Install usockets
RUN git clone https://github.com/uNetworking/uSockets && \
    cd uSockets && \
    git fetch --all --tags && \
    git checkout c2c1bbfa1644f1f6eb7fc9375650f41c5f9b7b06 && \
    curl https://raw.githubusercontent.com/gentoo/guru/dev/net-libs/usockets/files/usockets-0.8.1_p20211023-Makefile.patch | git apply && \
    curl https://raw.githubusercontent.com/gentoo/guru/dev/net-libs/usockets/files/usockets-0.8.1_p20211023-pkg-config.patch | git apply && \
    make WITH_OPENSSL=1 && \
    make install && \
    cd ..

# Install uwebsocket source
RUN git clone https://github.com/uNetworking/uWebSockets && \
    cd uWebSockets && \
    git fetch --all --tags && \
    git checkout $(git describe --tags $(git rev-list --tags --max-count=1)) && \
    cp -r src /usr/include/uWebSockets && \
    cd ..

# Install LMDB
RUN wget https://git.openldap.org/openldap/openldap/-/archive/LMDB_0.9.29/openldap-LMDB_0.9.29.tar.gz && \
    tar xzf openldap-LMDB_0.9.29.tar.gz && \
    cd openldap-LMDB_0.9.29/libraries/liblmdb && \
    make && \
    make prefix="/usr" install

# Instal lmdb++.h
RUN wget https://raw.githubusercontent.com/hoytech/lmdbxx/1.0.0/lmdb%2B%2B.h -O /usr/include/lmdb++.h

WORKDIR /app
COPY . .
RUN mkdir /out

# Install at /out
RUN meson --prefix "/out" build && \
    ninja -C build && \
    ninja -C build install

# Our runner dockerfile
FROM alpine:latest

# Install all the dynamically linked dependencies
RUN apk add wget make gcc openssl openssl-dev curl musl-dev git g++ && \
    wget https://git.openldap.org/openldap/openldap/-/archive/LMDB_0.9.29/openldap-LMDB_0.9.29.tar.gz && \
    tar xzf openldap-LMDB_0.9.29.tar.gz && \
    cd openldap-LMDB_0.9.29/libraries/liblmdb && \
    make && \
    make prefix="/usr" install && \
    cd ../../.. && \
    rm -rf openldap-LMDB_0.9.29.tar.gz openldap-LMDB_0.9.29 && \
    git clone https://github.com/uNetworking/uSockets && \
    cd uSockets && \
    git fetch --all --tags && \
    git checkout c2c1bbfa1644f1f6eb7fc9375650f41c5f9b7b06 && \
    curl https://raw.githubusercontent.com/gentoo/guru/dev/net-libs/usockets/files/usockets-0.8.1_p20211023-Makefile.patch | git apply && \
    curl https://raw.githubusercontent.com/gentoo/guru/dev/net-libs/usockets/files/usockets-0.8.1_p20211023-pkg-config.patch | git apply && \
    make WITH_OPENSSL=1 && \
    make install && \
    cd .. && \
    rm -rf uSockets && \
    apk del openssl-dev make curl wget musl-dev git g++

# Purrito binary from builder container
COPY --from=builder /out/bin/purrito /

CMD ["/purrito", "-d", "localhost/"]
