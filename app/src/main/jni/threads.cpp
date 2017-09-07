//
// Created by zyh on 2017/9/7.
//

#include <stdio.h>
#include <unistd.h>
#include "threads.h"

//方法ID被缓存
static jmethodID gOnNativeMessage = NULL;

void native_init(JNIEnv *env, jobject obj){
    //如果方法ID没被缓存
    if(NULL == gOnNativeMessage){
        //从对象中获取
        jclass clazz = env->GetObjectClass(obj);

        //获取回调方法ID
        gOnNativeMessage = env->GetMethodID(clazz,"onNativeMessage","(Ljava/lang/Sring;)V");

        //如果方法没有找到
        if(NULL == gOnNativeMessage){
            //获取异常
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");

            //抛出异常
            env->ThrowNew(exceptionClazz,"Unable to find method");
        }
    }
}

void native_free(JNIEnv *env, jobject obj){

}

void native_worker(JNIEnv *env, jobject obj, jint id, jint iterations){
    //循环给定的迭代数
    for(jint i=0;i<iterations;i++){
        //准备消息
        char message[26];
        sprintf(message,"Worker %d: Iteration %d", id, i);

        //来自C字符串的消息
        jstring messageString = env->NewStringUTF(message);

        //调用原生消息方法
        env->CallVoidMethod(obj,gOnNativeMessage, messageString);

        //检查是否产生异常
        if(NULL != env->ExceptionOccurred())
            break;

        //睡眠一秒
        sleep(1);
    }
}
