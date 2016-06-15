#include "stdafx.h"
#include "jni.h"
#include "wrapper.h"
#include "com_natives_test_Test.h"

extern wrapper* global_hooks = NULL;

JNIEXPORT void JNICALL Java_com_natives_test_Test_start (JNIEnv * env, jobject obj, jstring file_base, jstring exe) {
	const char* cfile_base = JStr2CStr(env, file_base);
	const char* browser = JStr2CStr(env, exe);
	
	if (!global_hooks)
		global_hooks = new wrapper(cfile_base,browser);
	global_hooks->init();
	global_hooks->start();
	MessageBox(NULL, cfile_base, "TETSTESTE", MB_OK);
}

JNIEXPORT void JNICALL Java_com_natives_test_Test_stop(JNIEnv* env, jobject obj) {
	global_hooks->stop();
}

char* JStr2CStr(JNIEnv* env, jstring jstr) {
	char* ret = NULL;
	jclass clzzStr = env->FindClass("java/lang/String");
	jstring strEncode = env->NewStringUTF("UTF-8");
	jmethodID methodId = env->GetMethodID(clzzStr, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray byteArray = (jbyteArray)env->CallObjectMethod(jstr, methodId, strEncode);
	jsize len = env->GetArrayLength(byteArray);
	jbyte* bytes = env->GetByteArrayElements(byteArray, JNI_FALSE);
	if (len > 0) {
		ret = (char*)malloc(len + 1);
		memcpy(ret, bytes, len);
		ret[len] = 0;
	}
	env->ReleaseByteArrayElements(byteArray, bytes, 0);
	return ret;
}