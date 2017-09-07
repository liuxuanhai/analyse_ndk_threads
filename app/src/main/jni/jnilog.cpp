/**
 * Created by zyh on 2017/9/1.
 */
#include "mylog.h"
#include "jnilog.h"

jstring native_log(JNIEnv *env, jclass clz){
    MY_LOG_VERBOSE("The stringFromJNI is called");
    MY_LOG_DEBUG("env=%p thiz=%p", env, clz);
    MY_LOG_ASSERT(0!=env, "JNIEnv cannot be NULL.");
    MY_LOG_INFO("Returning a new string.");

    return env->NewStringUTF("native_log");
}

