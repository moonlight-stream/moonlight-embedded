int nv_opus_init(unsigned int channelcount, unsigned int samplerate);
void nv_opus_destroy(void);
int nv_opus_decode(unsigned char* indata, int inlen, int framelen, short* outpcmdata);
