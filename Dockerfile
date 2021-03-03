FROM alpine:3.12

ARG  US_TAG="5440dbac79bd76444175b76ee95dfcade12a6aac"
ARG  UWS_TAG="b7202e71eea5195a35260f69efc3278b863a50ba"

ARG  VERSION="0.4.3"

LABEL org.opencontainers.image.authors="Aisha Tammy <purrito@bsd.ac>"
LABEL org.opencontainers.image.description="fast and minimalistic command line pastebin"
LABEL org.opencontainers.image.licenses="ISC"
LABEL org.opencontainers.image.source="https://github.com/PurritoBin/PurritoBin"
LABEL org.opencontainers.image.title="PurritoBin v${VERSION}"
LABEL org.opencontainers.image.version="${VERSION}"

ENV CC=gcc
ENV CXX=g++
ENV CFLAGS="-flto -O3"
ENV CXXFLAGS="-flto -O3"

ENV DOMAIN="http://localhost/"
ENV MAXPASTESIZE="65536"
ENV SLUGSIZE="7"

ENV SSL=NO
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
 && make PREFIX="/usr" install \
 && install -m755 docker/purritobin_wrapper /usr/bin \
 && mkdir -p /var/www/purritobin /etc/purritobin \
 && apk del gcc g++ git make musl-dev openssl-dev \
 && cd / \
 && rm -rf /purritobin

CMD /usr/bin/purritobin_wrapper
