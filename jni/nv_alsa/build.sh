rm *.o libnv_alsa.so
gcc -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -I /opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I ./inc -fPIC -L. -c *.c
gcc -shared -Wl,-soname,libnv_alsa.so -Wl,--no-undefined -o libnv_alsa.so *.o -L. -lasound
rm *.o
