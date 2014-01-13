rm *.o libnv_opus_dec.so
gcc -I include -I $JAVA_HOME/include/ -I $JAVA_HOME/include/linux -I /usr/include/opus -fPIC -c *.c
gcc -shared -Wl,-soname,libnv_opus_dec.so -Wl,--no-undefined -o libnv_opus_dec.so *.o -L. -lopus
rm *.o
