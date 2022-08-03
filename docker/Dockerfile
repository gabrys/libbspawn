# We need MinGW version 9 (GCC 11) which isn't available in Debian yet

FROM archlinux

# General cache buster:

RUN true 2022-07-29

# Install system packages:

RUN pacman --noconfirm -Sy gcc git make mingw-w64-gcc vim wget which zip

WORKDIR /build/

# docker run --volume $PWD:/build/ -it bspawn x86_64-w64-mingw32-gcc -o spawn.exe -I cosmopolitan spawn.c -l ws2_32
