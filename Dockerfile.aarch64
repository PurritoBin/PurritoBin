FROM alpine:3.12

ARG  US_TAG="5440dbac79bd76444175b76ee95dfcade12a6aac"
ARG  UWS_TAG="2fb38d5de92795998f9137ae30193aa65128b68b"
ARG  P_TAG="master"

ARG  VERSION="latest"

LABEL org.opencontainers.image.authors="Aisha Tammy <purrito@bsd.ac>"
LABEL org.opencontainers.image.description="ultra fast, minimalistic, encrypted command line paste-bin"
LABEL org.opencontainers.image.licenses="ISC"
LABEL org.opencontainers.image.source="https://github.com/PurritoBin/PurritoBin"
LABEL org.opencontainers.image.title="PurritoBin v${VERSION}"
LABEL org.opencontainers.image.version="${VERSION}"
LABEL org.opencontainers.image.revision="0"

ENV CC=gcc
ENV CXX=g++
ENV CFLAGS="-flto -O3"
ENV CXXFLAGS="-flto -O3"

ENV DOMAIN="http://localhost:42069/"
ENV MAXPASTESIZE="65536"
ENV SLUGSIZE="7"

ENV TLS=NO
ENV SERVERNAME="http://localhost/"
ENV PUBLICKEY="/etc/purritobin/public.crt"
ENV PRIVATEKEY="/etc/purritobin/private.crt"

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
 && git checkout "${P_TAG}" \
 && make PREFIX="/usr" install \
 && install -m755 docker/purritobin_wrapper /usr/bin \
 && cd .. \
 && mkdir -p /var/www/purritobin /etc/purritobin /var/db/purritobin.mdb \
 && apk del gcc g++ git make musl-dev openssl-dev \
 && cd / \
 && rm -rf /purritobin

CMD /usr/bin/purritobin_wrapper
