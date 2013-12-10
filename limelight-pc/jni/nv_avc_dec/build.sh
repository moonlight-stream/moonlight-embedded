rm *.o libnv_avc_dec.so
gcc -I /usr/lib/jvm/java-7-openjdk-amd64/include/ -I ./inc -fPIC -L. -c *.c
gcc -shared -Wl,-soname,libnv_avc_dec.so -Wl,--no-undefined -o libnv_avc_dec.so *.o -L. -lavcodec -lavfilter -lavformat -lavutil -lswresample -lswscale
rm *.o

