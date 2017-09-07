//
// Created by zyh on 2017/9/7.
//
#include <jni.h>

#ifndef ANALYSE_NDK_THREADS_THREADS_H
#define ANALYSE_NDK_THREADS_THREADS_H
#ifdef __cplusplus
extern "C" {
#endif

void native_init(JNIEnv *env, jobject obj);

void native_free(JNIEnv *env, jobject obj);

void native_worker(JNIEnv *env, jobject obj, jint id, jint iterations);

#ifdef __cplusplus
}
#endif
#endif //ANALYSE_NDK_THREADS_THREADS_H
