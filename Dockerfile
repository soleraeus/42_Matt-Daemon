FROM debian:bullseye

RUN apt update; apt full-upgrade -y; apt install g++ make -y; apt autoremove -y; apt autoclean -y;

COPY . /Matt_daemon

WORKDIR /Matt_daemon

RUN make

CMD ["./Matt_daemon"]
