# Vita port

This is a vita port.

# Build deps

## OpenSSL

https://github.com/d3m3vilurr/vita-openssl branch `vita-1_0_2`

```
./Configure no-threads --prefix=$VITASDK/arm-vita-eabi/ vita-cross
make depend
make -j8
make install
```

## curl

https://github.com/d3m3vilurr/vita-curl branch `vita`


```
./buildconf
cp lib/config-vita.h lib/curl_config.h
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
mkdir build && cd build
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
