FROM alpine:3.18.2 AS build_alpine

RUN apk add --no-cache openssl-dev boost-dev cmake build-base
ADD ./src /app/src
ADD ./thirdparty /app/thirdparty
ADD ./test /app/test
ADD ./CMakeLists.txt /app/CMakeLists.txt

RUN mkdir -p /app/build
WORKDIR /app/build
RUN cmake .. && cmake --build /app/build --target soapstab -- -j 4

FROM alpine:3.18.2

WORKDIR /app

RUN export LD_LIBRARY_PATH=/lib
RUN apk add --no-cache libstdc++
RUN apk add --no-cache openssl
RUN apk add --no-cache shadow
RUN apk add --no-cache bash
RUN groupadd -r soapstab
RUN useradd -r -g soapstab soapstab

COPY docker-resources/run.sh /app
RUN chmod +rx /app/run.sh

COPY --from=build_alpine /app/build/soapstab .

USER soapstab

ENTRYPOINT ["/app/run.sh"]
