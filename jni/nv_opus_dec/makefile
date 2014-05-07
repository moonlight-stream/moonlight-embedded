CFLAGS   = -I ${JAVA_HOME}/include -I ${JAVA_HOME}/include/linux -I /usr/include/opus -fPIC -L.
LDFLAGS  = -shared -Wl,--no-undefined -lopus -L.

SRCS = nv_opus_dec_jni.c nv_opus_dec.c
TARGET_LIB = libnv_opus_dec.so

OBJS = $(SRCS:.c=.o)

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

all: $(TARGET_LIB)

clean:
	rm -f ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

.PHONY: all clean