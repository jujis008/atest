#include "stdafx.h"
#include "wrapper.h"
#include "com_successfactors_saf_framework_video_service_CaptureImage.h"

//extern wrapper* global_hook = NULL;
JNIEXPORT void JNICALL Java_com_successfactors_saf_framework_video_service_CaptureImage_start(JNIEnv *env, jobject obj, jstring file_base, jstring exe){
	const char* cfile_base = JStr2CStrs(env, file_base);
	const char* browser = JStr2CStrs(env, exe);
	
	/*if (!global_hook) {
		global_hook = new wrapper(cfile_base, browser);
	}
	global_hook->init();
	global_hook->start();*/
}

JNIEXPORT void JNICALL Java_com_successfactors_saf_framework_video_service_CaptureImage_stop(JNIEnv *, jobject) {
	//global_hook->stop();
}

char* JStr2CStrs(JNIEnv* env, jstring jstr) {
	char* ret = NULL;
	jclass clzzStr = env->FindClass("java/lang/String");
	jstring strEncode = env->NewStringUTF("utf-8");
	jmethodID methodId = env->GetMethodID(clzzStr, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray byteArray = (jbyteArray)env->CallObjectMethod(jstr, methodId, strEncode);
	jsize len = env->GetArrayLength(byteArray);
	jbyte* bytes = env->GetByteArrayElements(byteArray, JNI_FALSE);
	if (len > 0) {
		ret = (char*)malloc(len + 1);
		memcpy(ret, byteArray, len);
		ret[len] = 0;
	}
	env->ReleaseByteArrayElements(byteArray, bytes, 0);
	return ret;
}