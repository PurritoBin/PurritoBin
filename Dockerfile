FROM alpine:latest

LABEL build_version="PurritoBin v0.4.2dev"

ENV CC=gcc
ENV CXX=g++
ENV CFLAGS="-flto -O3"
ENV CXXFLAGS="-flto -O3"

WORKDIR /purritobin

RUN apk update \
 && apk add libgcc libstdc++ libssl1.1 libcrypto1.1 \
 && apk add gcc g++ git make musl-dev openssl-dev \
 && git clone https://github.com/uNetworking/uSockets \
 && wget https://raw.githubusercontent.com/gentoo/guru/dev/net-libs/usockets/files/usockets-0.7.1-Makefile.patch \
 && cd uSockets \
 && git apply < ../usockets-0.7.1-Makefile.patch \
 && make WITH_OPENSSL=1 \
 && make install \
 && cd .. \
 && git clone https://github.com/uNetworking/uWebSockets \
 && cd uWebSockets \
 && cp -r src /usr/include/uWebSockets \
 && cd .. \
 && git clone https://github.com/PurritoBin/PurritoBin \
 && cd PurritoBin \
 && make PREFIX="/usr" install \
 && mkdir -p /var/www/purritobin \
 && cp frontend/paste.html /var/www/purritobin \
 && apk del gcc g++ git make musl-dev openssl-dev \
 && cd / \
 && rm -rf /purritobin

CMD ["purrito", "-d", "http://localhost/", "-t"]
