rm *.o libnv_io.so
gcc -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -fPIC -L. -c *.c
gcc -shared -Wl,-soname,libnv_io.so -Wl,--no-undefined -o libnv_io.so *.o -L.
rm *.o
