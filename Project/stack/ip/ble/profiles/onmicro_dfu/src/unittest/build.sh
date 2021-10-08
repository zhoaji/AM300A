rm -rf a.out a.exe
gcc *.c ../onmicro_dfu.c ../sha256.c ../uecc.c ../public_key.c -I.. -I. -Wall -O3 --std=c99 -m32
