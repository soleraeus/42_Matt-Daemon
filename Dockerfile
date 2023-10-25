FROM debian:bullseye

RUN apt update; apt full-upgrade -y; apt install g++ make tar perl -y; apt autoremove -y; apt autoclean -y;

COPY . /Matt_daemon

WORKDIR /Matt_daemon/libs

RUN	(rm -rf openssl-3.0.11; tar -xvf openssl-3.0.11.tar.gz;)

WORKDIR /Matt_daemon/libs/openssl-3.0.11

RUN ./config no-idea no-camellia no-seed no-bf no-cast no-des no-rc2 no-rc4 no-rc5 no-md2 no-md4 no-mdc2 no-dsa no-dh no-ec no-ecdsa no-ecdh no-sock no-ssl3 no-err no-engine; make depend; make build_generated libcrypto.a


WORKDIR /Matt_daemon
RUN make

CMD ["./Matt_daemon"]
