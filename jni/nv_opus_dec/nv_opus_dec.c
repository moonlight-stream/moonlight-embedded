#include <stdlib.h>
#include <opus.h>
#include "nv_opus_dec.h"

OpusDecoder* decoder;

#ifdef _WIN32
#pragma comment(lib, "opus.lib")
#pragma comment(lib, "celt.lib")
#pragma comment(lib, "silk_common.lib")
#pragma comment(lib, "silk_float.lib")
#endif


// This function must be called before
// any other decoding functions
int nv_opus_init(void) {
	int err;
	decoder = opus_decoder_create(
		nv_opus_get_sample_rate(),
		nv_opus_get_channel_count(),
		&err);
	return err;
}

// This function must be called after
// decoding is finished
void nv_opus_destroy(void) {
	if (decoder != NULL) {
		opus_decoder_destroy(decoder);
	}
}

// The Opus stream is stereo
int nv_opus_get_channel_count(void) {
	return 2;
}

// This number assumes 2 channels at 48 KHz
int nv_opus_get_max_out_bytes(void) {
	return 1024*nv_opus_get_channel_count();
}

// The Opus stream is 48 KHz
int nv_opus_get_sample_rate(void) {
	return 48000;
}

// outpcmdata must be 11520*2 bytes in length
// packets must be decoded in order
// a packet loss must call this function with NULL indata and 0 inlen
// returns the number of decoded samples
int nv_opus_decode(unsigned char* indata, int inlen, unsigned char* outpcmdata) {
	int err;

	// Decoding to 16-bit PCM with FEC off
	// Maximum length assuming 48KHz sample rate
	err = opus_decode(decoder, indata, inlen,
		(opus_int16*) outpcmdata, 512, 0);

	if (err>0)
		err = err * 2;
		
	return err;
}
