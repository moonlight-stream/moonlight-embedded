rm *.o libnv_opus_dec.so
gcc -I include -I /usr/lib/jvm/java-7-openjdk-amd64/include/ -I ./inc -fPIC -c *.c
gcc -shared -Wl,-soname,libnv_opus_dec.so -Wl,--no-undefined -o libnv_opus_dec.so *.o -L. -lopus
rm *.o

