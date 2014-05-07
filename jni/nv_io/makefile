CFLAGS   = -I ${JAVA_HOME}/include -I ${JAVA_HOME}/include/linux -fPIC -L.
LDFLAGS  = -shared -Wl,--no-undefined -L.

SRCS = nv_io_jni.c
TARGET_LIB = libnv_io.so

OBJS = $(SRCS:.c=.o)

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

all: $(TARGET_LIB)

clean:
	rm -f ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

.PHONY: all clean