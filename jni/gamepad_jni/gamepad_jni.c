#include <stdlib.h>
#include <jni.h>

#include "Gamepad.h"

static JavaVM *jvm;
static jclass nativeGamepadClass;
static jmethodID jdeviceAttached;
static jmethodID jdeviceRemoved;
static jmethodID jbuttonDown;
static jmethodID jbuttonUp;
static jmethodID jaxisMove;

static JNIEnv *getThreadEnv(void) {
	JNIEnv *env;
	(*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6);
	return env;
}

static void invokeJavaCallback(jmethodID method, ...) {
	JNIEnv *env = getThreadEnv();
	va_list args;
	va_start(args, method);
	(*env)->CallStaticVoidMethodV(env, nativeGamepadClass, method, args);
	va_end(args);
}

static void deviceAttachCallback(struct Gamepad_device * device, void * context) {
	invokeJavaCallback(jdeviceAttached, device->deviceID, device->numButtons, device->numAxes);
}

static void deviceRemoveCallback(struct Gamepad_device * device, void * context) {
	invokeJavaCallback(jdeviceRemoved, device->deviceID);
}

static void buttonDownCallback(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context) {
	invokeJavaCallback(jbuttonDown, device->deviceID, buttonID);
}

static void buttonUpCallback(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context) {
	invokeJavaCallback(jbuttonUp, device->deviceID, buttonID);
}

static void axisMoveCallback(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue, double timestamp, void * context) {
	invokeJavaCallback(jaxisMove, device->deviceID, axisID, value, lastValue);
}

// This function must be called first
JNIEXPORT void JNICALL
Java_com_limelight_input_gamepad_NativeGamepad_init(JNIEnv *env, jobject this) {
	Gamepad_deviceAttachFunc(deviceAttachCallback, NULL);
	Gamepad_deviceRemoveFunc(deviceRemoveCallback, NULL);
	Gamepad_buttonDownFunc(buttonDownCallback, NULL);
	Gamepad_buttonUpFunc(buttonUpCallback, NULL);
	Gamepad_axisMoveFunc(axisMoveCallback, NULL);
		
	Gamepad_init();
}

// This function must be called last
JNIEXPORT void JNICALL
Java_com_limelight_input_gamepad_NativeGamepad_shutdown(JNIEnv *env, jobject this) {
	Gamepad_shutdown();
	
	// Remove the class reference
	(*env)->DeleteGlobalRef(env, nativeGamepadClass);
}

// This returns the number of connected devices
JNIEXPORT jint JNICALL
Java_com_limelight_input_gamepad_NativeGamepad_numDevices(JNIEnv *env, jobject this) {
	return Gamepad_numDevices();
}

// This triggers device detection
JNIEXPORT void JNICALL
Java_com_limelight_input_gamepad_NativeGamepad_detectDevices(JNIEnv *env, jobject this) {
	Gamepad_detectDevices();
}

// This polls for events and calls the appropriate callbacks
JNIEXPORT jint JNICALL
Java_com_limelight_input_gamepad_NativeGamepad_processEvents(JNIEnv *env, jobject this) {
	Gamepad_processEvents();
}

// This is called when the library is first loaded
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *pjvm, void *reserved) {
	JNIEnv *env;
	
	// This has to be saved before getThreadEnv() can be called
	jvm = pjvm;
	
	env = getThreadEnv();
	
	// We use a global reference to keep the class loaded
	nativeGamepadClass = (*env)->NewGlobalRef(env, (*env)->FindClass(env, "com/limelight/input/gamepad/NativeGamepad"));
	
	// These method IDs are only valid as long as the NativeGamepad class is loaded
	jdeviceAttached = (*env)->GetStaticMethodID(env, nativeGamepadClass, "deviceAttachCallback", "(III)V");
	jdeviceRemoved = (*env)->GetStaticMethodID(env, nativeGamepadClass, "deviceRemoveCallback", "(I)V");
	jbuttonDown = (*env)->GetStaticMethodID(env, nativeGamepadClass, "buttonDownCallback", "(II)V");
	jbuttonUp = (*env)->GetStaticMethodID(env, nativeGamepadClass, "buttonUpCallback", "(II)V");
	jaxisMove = (*env)->GetStaticMethodID(env, nativeGamepadClass, "axisMovedCallback", "(IIFF)V");
	
	return JNI_VERSION_1_6;
}
