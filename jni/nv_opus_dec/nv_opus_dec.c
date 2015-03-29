#include <stdlib.h>
#include <opus.h>
#include "nv_opus_dec.h"

#include <stdio.h>

OpusDecoder* decoder;

// This function must be called before
// any other decoding functions
int nv_opus_init(unsigned int channelcount, unsigned int samplerate) {
	int err;
	decoder = opus_decoder_create(samplerate, channelcount, &err);
	return err;
}

// This function must be called after
// decoding is finished
void nv_opus_destroy(void) {
	if (decoder != NULL) {
		opus_decoder_destroy(decoder);
	}
}

// outpcmdata must be 5760*2 shorts in length
// packets must be decoded in order
// a packet loss must call this function with NULL indata and 0 inlen
// returns the number of decoded samples
int nv_opus_decode(unsigned char* indata, int inlen, int framesize, short* outpcmdata) {
	int err;

	// Decoding to 16-bit PCM with FEC off
	// Maximum length assuming 48KHz sample rate
	err = opus_decode(decoder, indata, inlen, outpcmdata, framesize, 0);

	return err;
}
