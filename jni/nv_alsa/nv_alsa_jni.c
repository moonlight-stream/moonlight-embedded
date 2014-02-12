#include "nv_alsa.h"

#include <stdlib.h>
#include <jni.h>

// This function must be called before
// any other decoding functions
JNIEXPORT jint JNICALL
Java_com_limelight_binding_audio_AlsaAudio_init(JNIEnv *env, jobject this, jint channelCount, jint sampleRate, jbyteArray device)
{
	jint ret;
	jbyte* jni_device;
	
	jni_device = (*env)->GetByteArrayElements(env, device, 0);
	
	ret = nv_alsa_init(channelCount, sampleRate, jni_device);
	
	// The input data isn't changed so it can be safely aborted
	(*env)->ReleaseByteArrayElements(env, device, jni_device, JNI_ABORT);
	
	return ret;
}

JNIEXPORT void JNICALL
Java_com_limelight_binding_audio_AlsaAudio_close(JNIEnv *env, jobject this)
{
	nv_alsa_close();
}

JNIEXPORT jint JNICALL
Java_com_limelight_binding_audio_AlsaAudio_play(
	JNIEnv *env, jobject this, // JNI parameters
	jbyteArray indata, jint inoff, jint inlen)
{
	jint ret;
	jbyte* jni_input_data;

	jni_input_data = (*env)->GetByteArrayElements(env, indata, 0);

	ret = nv_alsa_play(&jni_input_data[inoff], inlen);

	// The input data isn't changed so it can be safely aborted
	(*env)->ReleaseByteArrayElements(env, indata, jni_input_data, JNI_ABORT);
	
	return ret;
}