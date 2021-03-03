FROM alpine:latest

LABEL build_version="PurritoBin v0.4.2dev"

RUN apk update
RUN apk add libgcc libstdc++ libssl1.1 libcrypto1.1
RUN apk add gcc g++ git make musl-dev openssl-dev

ENV CC=gcc
ENV CXX=g++
ENV CFLAGS="-flto -O3"
ENV CXXFLAGS="-flto -O3"

WORKDIR /purritobin/build

RUN git clone https://github.com/uNetworking/uSockets \
 && wget https://raw.githubusercontent.com/gentoo/guru/dev/net-libs/usockets/files/usockets-0.7.1-Makefile.patch \
 && cd uSockets \
 && git apply < ../usockets-0.7.1-Makefile.patch \
 && make WITH_OPENSSL=1 \
 && make install \
 && cd ..

RUN git clone https://github.com/uNetworking/uWebSockets \
 && cd uWebSockets \
 && cp -r src /usr/include/uWebSockets \
 && cd ..

RUN git clone https://github.com/PurritoBin/PurritoBin \
 && cd PurritoBin \
 && make PREFIX="/usr" install

RUN mkdir -p /var/www/purritobin

RUN apk del gcc g++ git make musl-dev openssl-dev
RUN rm -rf /purritobin

CMD ["purrito", "-d", "http://localhost/"]