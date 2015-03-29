#include "nv_opus_dec.h"

#include <stdlib.h>
#include <jni.h>

#include <stdio.h>

// This function must be called before
// any other decoding functions
JNIEXPORT jint JNICALL
Java_com_limelight_binding_audio_OpusDecoder_init(JNIEnv *env, jobject this, jint channelcount, jint samplerate) {
	return nv_opus_init(channelcount, samplerate);
}

// This function must be called after
// decoding is finished
JNIEXPORT void JNICALL
Java_com_limelight_binding_audio_OpusDecoder_destroy(JNIEnv *env, jobject this) {
	nv_opus_destroy();
}

// outpcmdata must be 5760*2 shorts in length
// packets must be decoded in order
// a packet loss must call this function with NULL indata and 0 inlen
// returns the number of decoded samples
JNIEXPORT jint JNICALL
Java_com_limelight_binding_audio_OpusDecoder_decode(
	JNIEnv *env, jobject this, // JNI parameters
	jbyteArray indata, jint inoff, jint inlen, jint framesize, // Input parameters
	jbyteArray outpcmdata) // Output parameter
{
	jint ret;
	jbyte* jni_input_data;
	jbyte* jni_pcm_data;

	jni_pcm_data = (*env)->GetByteArrayElements(env, outpcmdata, 0);
	if (indata != NULL) {
		jni_input_data = (*env)->GetByteArrayElements(env, indata, 0);
		
		ret = nv_opus_decode(&jni_input_data[inoff], inlen, framesize, (jshort*) jni_pcm_data);

		// The input data isn't changed so it can be safely aborted
		(*env)->ReleaseByteArrayElements(env, indata, jni_input_data, JNI_ABORT);
	} else {
		ret = nv_opus_decode(NULL, 0, framesize, (jshort*) jni_pcm_data);
	}

	(*env)->ReleaseByteArrayElements(env, outpcmdata, jni_pcm_data, 0);

	return ret;
}
