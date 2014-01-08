rm *.o libnv_omx_dec.so
gcc -I /opt/vc/src/hello_pi/libs/ilclient -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -I /opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I ./inc -fPIC -L. -c *.c
gcc -shared -Wl,-soname,libnv_omx_dec.so -Wl,--no-undefined -o libnv_omx_dec.so *.o -L. -L /opt/vc/lib/ -lbcm_host -lopenmaxil -lvcos -lvchiq_arm -lpthread -lrt -L /opt/vc/src/hello_pi/libs/ilclient -lilclient
rm *.o

