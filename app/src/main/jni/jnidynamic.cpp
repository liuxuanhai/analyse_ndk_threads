//
// Created by zyh on 2017/9/7.
//
#include "threads.h"
#include "jnilog.h"

static JNINativeMethod gMethods[] = {
    {
        "nativeInit",
        "()V",
        (void*)native_init
    },
    {
        "nativeFree",
        "()V",
        (void*)native_free
    },
    {
        "nativeWorker",
        "(II)V",
        (void*)native_worker
    },
    {
        "logFromJni",
        "()Ljava/lang/String;",
        (void *)native_log
    }
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved){
    JNIEnv *env = NULL;
    jint result = JNI_FALSE;

    //获取env指针
    if(jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK){
        return result;
    }

    if(NULL == env){
        return result;
    }

    //获取类引用，写类的全路径（包+类名），FindClass等JNI函数将在后面详解
    jclass clazz = env->FindClass("com/year17/analyse_ndk_threads/MainActivity");
    if(clazz == NULL){
        return result;
    }

    //注册方法
    if(env->RegisterNatives(clazz, gMethods, sizeof(gMethods)/sizeof(gMethods[0]))<0){
        return result;
    }

    //成功
    result = JNI_VERSION_1_6;
    return result;
}
