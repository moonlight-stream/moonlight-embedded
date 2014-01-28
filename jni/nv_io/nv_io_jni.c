#include <fcntl.h>
#include <jni.h>

JNIEXPORT jboolean JNICALL
Java_com_limelight_input_IO_ioctl(
	JNIEnv *env, jobject this, // JNI parameters
	jstring filename, jbyteArray buffer, jint request)
{
	const char* jni_filename = (*env)->GetStringUTFChars(env, filename, NULL);
	unsigned char* jni_buffer = (*env)->GetByteArrayElements(env, buffer, NULL);

	jboolean retval;
	int fd;
	if ((fd = open(jni_filename, O_RDONLY)) < 0) {
		retval = 0;
	} else {
		ioctl(fd, request, jni_buffer);
		close(fd);
		retval = 1;
	}

	(*env)->ReleaseByteArrayElements(env, buffer, jni_buffer, 0);
	return retval;
}