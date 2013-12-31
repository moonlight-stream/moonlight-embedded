rm *.o libgamepad_jni.dylib
gcc -I /Library/Java/JavaVirtualMachines/jdk1.7.0_45.jdk/Contents/Home/include/ -I /Library/Java/JavaVirtualMachines/jdk1.7.0_45.jdk/Contents/Home/include/darwin -fPIC -c *.c
gcc -shared -o libgamepad_jni.dylib *.o -L./osx -lstem_gamepad -lpthread -framework IOKit -framework CoreFoundation
rm *.o

