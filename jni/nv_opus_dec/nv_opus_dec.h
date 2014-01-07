int nv_opus_init(void);
void nv_opus_destroy(void);
int nv_opus_get_channel_count(void);
int nv_opus_get_max_out_bytes(void);
int nv_opus_get_sample_rate(void);
int nv_opus_decode(unsigned char* indata, int inlen, unsigned char* outpcmdata);
