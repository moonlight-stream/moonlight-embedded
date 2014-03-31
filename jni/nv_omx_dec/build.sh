rm *.o libnv_omx_dec.so
gcc -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -I /opt/vc/include -I /opt/vc/include/interface/vchiq -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I ./inc -fPIC -L. -c *.c
gcc -I /opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -fPIC -L. -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi -c inc/*.c
gcc -shared -Wl,-soname,libnv_omx_dec.so -Wl,--no-undefined -o libnv_omx_dec.so *.o -L. -L /opt/vc/lib/ -lbcm_host -lopenmaxil -lvcos -lvchiq_arm -lpthread -lrt
rm *.o

