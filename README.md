# Vita port

This is a vita port.

# Build deps

## OpenSSL

https://github.com/xyzz/vita-openssl branch `vita-1_0_2`

```
./Configure no-threads --prefix=$VITASDK/arm-vita-eabi/ vita-cross
make depend
make -j8
make install
```

## curl

https://github.com/xyzz/vita-curl branch `vita`


```
./buildconf
./configure --host=arm-vita-eabi --with-ssl=$VITASDK/arm-vita-eabi/ --disable-shared --disable-ftp --disable-ldap --disable-imap --disable-ipv6 --disable-rtsp --disable-dict --disable-file --disable-gopher --disable-pop3 --disable-smtp --disable-telnet --disable-tftp --enable-https --disable-smb --disable-smbs --prefix=$VITASDK/arm-vita-eabi/
make -j8
make install
```

## expat

(Tested and working commit 25c6393829d03930dbcceb81a298841a700f04dc)

```
git clone git://git.code.sf.net/p/expat/code_git expat
cd expat
git checkout 25c6393829d03930dbcceb81a298841a700f04dc
```

remove line `add_custom_command(TARGET expat PRE_BUILD COMMAND $(MAKE) -C doc xmlwf.1)` in `CMakeLists.txt`

```
cmake .. -DCMAKE_SYSTEM_NAME="Generic" -DCMAKE_C_COMPILER="arm-vita-eabi-gcc" -DBUILD_tools=0 -DBUILD_examples=0 -DBUILD_tests=0 -DBUILD_shared=0 -DCMAKE_INSTALL_PREFIX=$VITASDK/arm-vita-eabi/
make -j8
make install
```

## opus

Patch opus_defines.h: `#define __opus_check_int_ptr(ptr) (ptr) // ((ptr) + ((ptr) - (opus_int32*)(ptr)))`

```
./configure --host=arm-vita-eabi --enable-fixed-point --prefix=$VITASDK/arm-vita-eabi
make -j8
make install
```

# Build Moonlight

```
# if you do git pull, make sure submodules are updated first
git submodule update --init
mkdir build && cd build
cmake ..
make
```
