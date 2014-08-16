CFLAGS   = -fPIC -I ${JAVA_HOME}/include -I ${JAVA_HOME}/include/linux -I /opt/fsl/include/
LDFLAGS  = -shared -Wl,--no-undefined -L. -L /opt/fsl/lib/ -lvpu

SRCS = nv_imx_dec.c nv_imx_dec_jni.c
TARGET_LIB = libnv_imx_dec.so

OBJS = $(SRCS:.c=.o)

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

all: $(TARGET_LIB)

clean:
	rm -f ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

.PHONY: all clean