#include <jni.h>

int nv_imx_init(void);

int nv_imx_decode(unsigned char* indata, int inlen, int last);

void nv_imx_destroy(void);
