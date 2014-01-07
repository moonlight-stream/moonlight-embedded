#include <jni.h>

int nv_omx_init(void);

int nv_omx_decode(unsigned char* indata, int inlen, int last);

void nv_omx_stop(void);
void nv_omx_destroy(void);
