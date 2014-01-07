#include "nv_omx_dec.h"

#include <stdlib.h>
#include <jni.h>

// This function must be called before
// any other decoding functions
JNIEXPORT jint JNICALL
Java_com_limelight_binding_video_OmxDecoder_init(JNIEnv *env, jobject this)
{
	return nv_omx_init();
}

// This function must be called after
// decoding is finished
JNIEXPORT void JNICALL
Java_com_limelight_binding_video_OmxDecoder_stop(JNIEnv *env, jobject this) {
	nv_omx_stop();
}

// This function must be called after
// decoding is finished
JNIEXPORT void JNICALL
Java_com_limelight_binding_video_OmxDecoder_destroy(JNIEnv *env, jobject this) {
	nv_omx_destroy();
}

// packets must be decoded in order
// the input buffer must have proper padding
// returns 0 on success, < 0 on error
JNIEXPORT jint JNICALL
Java_com_limelight_binding_video_OmxDecoder_decode(
	JNIEnv *env, jobject this, // JNI parameters
	jbyteArray indata, jint inoff, jint inlen, jboolean last)
{
	jint ret;
	jbyte* jni_input_data;

	jni_input_data = (*env)->GetByteArrayElements(env, indata, 0);

	ret = nv_omx_decode(&jni_input_data[inoff], inlen, last);

	// The input data isn't changed so it can be safely aborted
	(*env)->ReleaseByteArrayElements(env, indata, jni_input_data, JNI_ABORT);

	return ret;
}
