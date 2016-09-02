cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER "arm-vita-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-vita-eabi-g++")

project(moonlight)
set(TITLE_ID "XYZZ00002")

set(CMAKE_C_FLAGS "-Wl,-q -O3 -g -std=c99 -Dntohs=__builtin_bswap16 -Dhtons=__builtin_bswap16 -Dntohl=__builtin_bswap32 -Dhtonl=__builtin_bswap32 -DENET_DEBUG=1")

include_directories(
	third_party/moonlight-common-c/src/
	libgamestream/
	third_party/libuuid/src/
	third_party/h264bitstream/
	third_party/enet/include/
)

add_executable(${PROJECT_NAME}.elf
	src/config.c
	src/input/mapping.c
	src/connection.c
	src/global.c
	src/loop.c
	src/main.c
	src/platform.c

	src/audio/vita.c
	src/video/vita.c
	src/input/vita.c
	src/graphics.c
	src/font.c

	libgamestream/client.c
	libgamestream/http.c
	libgamestream/mkcert.c
	libgamestream/sps.c
	libgamestream/xml.c

	third_party/libuuid/src/clear.c
	third_party/libuuid/src/compare.c
	third_party/libuuid/src/copy.c
	third_party/libuuid/src/gen_uuid.c
	third_party/libuuid/src/isnull.c
	third_party/libuuid/src/pack.c
	third_party/libuuid/src/parse.c
	third_party/libuuid/src/unpack.c
	third_party/libuuid/src/unparse.c
	third_party/libuuid/src/uuid_time.c

	third_party/h264bitstream/h264_nal.c
	third_party/h264bitstream/h264_sei.c
	third_party/h264bitstream/h264_stream.c

	third_party/moonlight-common-c/src/AudioStream.c
	third_party/moonlight-common-c/src/ByteBuffer.c
	third_party/moonlight-common-c/src/Connection.c
	third_party/moonlight-common-c/src/ControlStream.c
	third_party/moonlight-common-c/src/FakeCallbacks.c
	third_party/moonlight-common-c/src/InputStream.c
	third_party/moonlight-common-c/src/LinkedBlockingQueue.c
	third_party/moonlight-common-c/src/Misc.c
	third_party/moonlight-common-c/src/Platform.c
	third_party/moonlight-common-c/src/PlatformSockets.c
	third_party/moonlight-common-c/src/RtpReorderQueue.c
	third_party/moonlight-common-c/src/RtspConnection.c
	third_party/moonlight-common-c/src/RtspParser.c
	third_party/moonlight-common-c/src/SdpGenerator.c
	third_party/moonlight-common-c/src/VideoDepacketizer.c
	third_party/moonlight-common-c/src/VideoStream.c

	third_party/enet/callbacks.c
	third_party/enet/compress.c
	third_party/enet/host.c
	third_party/enet/list.c
	third_party/enet/packet.c
	third_party/enet/peer.c
	third_party/enet/protocol.c
	third_party/enet/vita.c
)

target_link_libraries(${PROJECT_NAME}.elf
	-lcrypto
	-lexpat
	-lcurl
	-lz
	-lssl
	-lopus

	-lSceDisplay_stub
	-lSceCtrl_stub
	-lSceTouch_stub
	-lSceReg_stub
	-lSceNet_stub
	-lSceSysmodule_stub
	-lSceNetCtl_stub
	-lSceVideodec_stub
	-lSceCommonDialog_stub
	-lSceAudio_stub
)

add_custom_target(${PROJECT_NAME}.vpk ALL
	COMMAND vita-elf-create ${PROJECT_NAME}.elf ${PROJECT_NAME}.velf
	COMMAND vita-make-fself -s ${PROJECT_NAME}.velf eboot.bin
	COMMAND vita-mksfoex -s TITLE_ID=${TITLE_ID} "${PROJECT_NAME}" param.sfo
	COMMAND vita-pack-vpk -s param.sfo -b eboot.bin ${PROJECT_NAME}.vpk
)
add_dependencies(${PROJECT_NAME}.vpk ${PROJECT_NAME}.elf)
