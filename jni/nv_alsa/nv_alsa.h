#include <jni.h>

int nv_alsa_init(unsigned int channelCount, unsigned int sampleRate);
int nv_alsa_play(unsigned char* indata, int inlen);
void nv_alsa_close(void);
