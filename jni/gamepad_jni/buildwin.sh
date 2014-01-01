rm gamepad_jni.dll gamepad_jni64.dll
/C/MinGW/bin/gcc -m32 -shared -Wall -I"/C/Program Files (x86)/Java/jdk1.7.0_45/include" -I"/C/Program Files (x86)/Java/jdk1.7.0_45/include/win32" *.c -L./win32 -lstem_gamepad -lxinput9_1_0 -Wl,--no-undefined -Wl,--kill-at -o gamepad_jni.dll
/C/MinGW-w64/bin/gcc -m64 -shared -Wall -I"/C/Program Files (x86)/Java/jdk1.7.0_45/include" -I"/C/Program Files (x86)/Java/jdk1.7.0_45/include/win32" *.c -L./win64 -lstem_gamepad -lxinput9_1_0 -Wl,--no-undefined -Wl,--kill-at -o gamepad_jni64.dll
